/**
 * @file io_mon.c
 * @date 16 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <unistd.h>

#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <io_src.h>
#include <io_mon.h>

#include "io_platform.h"
#include "io_utils.h"

#define MONITOR_MAX_SOURCES 10

/**
 * Sets a file descriptor non-blocking
 * @param fd File descriptor
 * @return errno compatible negative value on error, 0 otherwise
 * @see fcntl
 */
static int set_non_blocking(int fd)
{
	int flags;
	int ret;

	flags = fcntl(fd, F_GETFL, 0);
	if (-1 == flags)
		flags = 0;
	if (!(flags & O_NONBLOCK)) {
		ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		if (0 != ret)
			return -errno;
	}

	return 0;
}

/**
 * Adds a source to the monitor
 * @param monitor Monitor context
 * @param source Monitor's source
 * @return FUNK_ERROR_EXHAUSTION or FUNK_ERROR_PARAM
 */
static int add_source(struct io_mon *mon, struct io_src *src)
{
	int ret = -1;

	if (NULL == src->cb)
		return -EINVAL;

	ret = set_non_blocking(src->fd);
	if (0 != ret)
		return ret;

	/* sources can't be present twice */
	if (rs_node_find(&(src->node), mon->source))
		return -EEXIST;
	rs_node_push(&(mon->source), &(src->node));
	mon->nb_sources++;

	return 0;
}

/**
 * Changes the epoll monitoring status of a source
 * @param epfd Epoll file descriptor
 * @param source Source to alter
 * @param op epoll's operator (EPOLL_CTL_ADD, EPOLL_CTL_MOD or EPOLL_CTL_DEL)
 * @return FUNK_ERROR_MONITOR
 */
static int alter_source(int epfd, struct io_src *src, int op)
{
	struct epoll_event event = {
			.events = src->active,
			.data = {
					.ptr = src,
			},
	};
	int ret;

	ret = epoll_ctl(epfd, op, src->fd, &event);
	if (-1 == ret)
		return -errno;

	return 0;
}

/**
 * Removes a source from the monitoring pool and stop monitoring the
 * corresponding fd
 * @param mon Monitor
 * @param src Source to remove
 * @return negative errno compatible value on error, 0 otherwise
 */
static int remove_source(struct io_mon *mon, struct io_src *src)
{
	int ret;
	struct rs_node *node;
	struct io_src *old_src;

	if (&(src->node) == mon->source)
		mon->source = rs_node_next(mon->source);

	node = rs_node_remove(&(src->node), &(src->node));
	if (NULL == node)
		return -EINVAL;
	mon->nb_sources--;

	old_src = to_src(node);
	if (IO_NONE != old_src->active) {
		old_src->active = IO_NONE;
		ret = alter_source(mon->epollfd, old_src, EPOLL_CTL_DEL);
		if (-1 == ret)
			return -errno;
	}


	return 0;
}

/**
 * Registers a source to epoll subsystem. In sources are monitored, out ones
 * aren't, duplex ones are monitored only for in events. All types are forced to
 * monitor errors
 * @param mon Monitor's context
 * @param src Source to register
 * @return negative errno-compatible value on error, 0 otherwise
 */
static int register_source(struct io_mon *mon, struct io_src *src)
{
	if (NULL == mon || NULL == src)
		return -EINVAL;

	return alter_source(mon->epollfd, src, EPOLL_CTL_ADD);
}

/**
 * Says if a source still has events pending mathching at least one of the
 * events it is registered for and the error events
 * @param src Source
 * @return Non-zero if there is still events to process, 0 otherwise
 */
static int has_events_pending(struct io_src *src)
{
	return src->events & (src->active | IO_EPOLL_ERROR_EVENTS);
}

/**
 * Notifies client of I/O events for a source and checks for errors.
 * @param mon Monitor
 * @param src Source
 * @return negative errno-compatible value on error from the client callback, 0
 * otherwise
 */
static int process_event_sets(struct io_mon *mon, struct io_src *src)
{
	/*
	 * if during processing, sources are altered, some events may
	 * have become irrelevant and must be filtered out
	 */
	if (!has_events_pending(src))
		return 0;

	src->cb(src);

	if (io_mon_has_error(src->events))
		/*
		 * TODO notify client that a removal has been performed, if not
		 * initiated by him
		 */
		remove_source(mon, src);

	return 0;
}

/**
 * Notifies client of I/O events sets pending for a source and checks for
 * errors.
 * @param mon Monitor
 * @param n Number of events sets to process
 * @param events List of the events sets to process
 * @return First critical error from a client callback, 0 on success
 */
static int do_process_events_sets(struct io_mon *mon, int n,
		struct epoll_event *events)
{
	int i = 0;
	struct io_src *src = NULL;
	struct epoll_event *event;

	for (i = 0; i < n; i++) {
		event = events + i;
		src = event->data.ptr;
		assert(src != NULL);
		if (NULL == src)
			return -EINVAL;
		src->events = event->events;

		process_event_sets(mon, src);
	}

	return 0;
}

/**
 * Helper for io_mon_activate_out_source and io_mon_activate_in_source
 * implementation
 * @param mon Monitor
 * @param src Source to (de-)activate
 * @param active non-zero if the source must be monitored, 0 otherwise
 * @param direction Direction to alter the monitoring of, either IO_IN or
 * IO_OUT, not both
 *
 * @return negative errno value on error, 0 otherwise
 */
static int activate_source(struct io_mon *mon, struct io_src *src,
		int active, enum io_src_event direction)
{
	if (NULL == mon || NULL == src || !(direction & src->type))
		return -EINVAL;

	if (active)
		src->active |= direction;
	else
		src->active &= ~direction;

	/* TODO do nothing if the source is already active */

	return alter_source(mon->epollfd, src, EPOLL_CTL_MOD);
}

int io_mon_init(struct io_mon *mon)
{
	if (NULL == mon)
		return -EINVAL;

	memset(mon, 0, sizeof(*mon));
	mon->epollfd = io_epoll_create1(EPOLL_CLOEXEC);
	if (-1 == mon->epollfd)
		return -errno;

	return 0;
}

int io_mon_get_fd(struct io_mon *mon)
{
	if (NULL == mon)
		return -EINVAL;

	return mon->epollfd;
}

int io_mon_add_source(struct io_mon *mon, struct io_src *src)
{
	int ret = 0;

	if (NULL == mon || NULL == src)
		return -EINVAL;

	/* add the source to our list */
	ret = add_source(mon, src);
	if (0 != ret)
		return ret;

	/* by default, only IN monitoring is activated */
	src->active = src->type & ~IO_OUT;

	return register_source(mon, src);
}

int io_mon_add_sources(struct io_mon *mon, ...)
{
	struct io_src *src;
	va_list args;
	int ret;

	if (NULL == mon)
		return -EINVAL;

	va_start(args, mon);
	do {
		src = va_arg(args, struct io_src *);
		if (NULL == src)
			break;
		ret = io_mon_add_source(mon, src);
		if (0 != ret)
			return ret;
	} while (1);
	va_end(args);

	return 0;
}

void io_mon_dump_epoll_event(uint32_t events)
{
	fprintf(stderr, "epoll events :\n");

	if (events & EPOLLIN)
		fprintf(stderr, "\tEPOLLIN\n");
	if (events & EPOLLOUT)
		fprintf(stderr, "\tEPOLLOUT\n");
	if (events & EPOLLRDHUP)
		fprintf(stderr, "\tEPOLLRDHUP\n");
	if (events & EPOLLPRI)
		fprintf(stderr, "\tEPOLLPRI\n");
	if (events & EPOLLERR)
		fprintf(stderr, "\tEPOLLERR\n");
	if (events & EPOLLHUP)
		fprintf(stderr, "\tEPOLLHUP\n");
}

int io_mon_activate_out_source(struct io_mon *mon, struct io_src *src,
		int active)
{
	return activate_source(mon, src, active, IO_OUT);
}

int io_mon_activate_in_source(struct io_mon *mon, struct io_src *src,
		int active)
{
	return activate_source(mon, src, active, IO_IN);
}

int io_mon_process_events(struct io_mon *mon)
{
	int n = 0; /* TODO should be a ssize_t hu ? */
	struct epoll_event events[MONITOR_MAX_SOURCES];

	if (NULL == mon)
		return -EINVAL;

	/* retrieve events */
	n = io_epoll_wait(mon->epollfd, events, MONITOR_MAX_SOURCES,
			0 /* don't block */
			);
	if (-1 == n)
		return -errno;

	return do_process_events_sets(mon, n, events);
}

int io_mon_clean(struct io_mon *mon)
{
	struct io_src *src;

	if (NULL == mon)
		return -EINVAL;

	while (mon->source) {
		src = to_src(mon->source);
		remove_source(mon, src);
	}

	if (-1 != mon->epollfd)
		close(mon->epollfd);
	memset(mon, 0, sizeof(*mon));
	mon->epollfd = -1;

	return 0;
}
