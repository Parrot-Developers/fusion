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

#include <ut_utils.h>
#include <ut_file.h>

#include <io_mon.h>
#include <io_utils.h>

#include "io_platform.h"

#define MONITOR_MAX_SOURCES 10

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

	ret = io_set_non_blocking(src->fd);
	if (0 != ret)
		return ret;

	/* sources can't be present twice */
	if (rs_node_find(&(src->node), mon->source.next))
		return -EEXIST;
	rs_node_push(&(mon->source.next), &(src->node));
	src->node.prev = &mon->source;

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
 * Says if a source still has events pending matching at least one of the
 * events it is registered for and the error events
 * @param src Source
 * @return Non-zero if there is still events to process, 0 otherwise
 */
static bool has_events_pending(struct io_src *src)
{
	return (src->events & (src->active | IO_EPOLL_ERROR_EVENTS)) != 0;
}

/**
 * @fn io_src_match_fd
 * @brief matches an io_src
 * @param __n Node of the source to match
 * @param __d Pointer to the value of the fd field to match the source with
 * @return true iif the source has the given fd value
 */
static RS_NODE_MATCH_MEMBER(io_src, fd, node)

/**
 * Retrieves a source in a monitor, knowing it's file descriptor
 * @param mon Monitor in which to search for the source
 * @param fd File descriptor of the source
 * @return Source if found, NULL if not
 */
static struct io_src *find_source_by_fd(struct io_mon *mon, int fd)
{
	struct rs_node *needle;

	if (NULL == mon || -1 == fd)
		return NULL;

	needle = rs_node_find_match(mon->source.next, io_src_match_fd, &fd);
	if (NULL == needle)
		return NULL;

	return to_src(needle);
}

/**
 * Removes a source from a monitor
 * @param mon Monitor
 * @param src Source to remove
 * @return negative errno value on error, 0 otherwise
 */
static int remove_source(struct io_mon *mon, struct io_src *src)
{
	int ret;
	struct io_src *old_src;
	struct rs_node *node;

	node = rs_node_remove(&mon->source, &(src->node));
	if (NULL == node)
		return -ENOENT;

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
 * Removes a source from a monitor, knowing it's file descriptor
 * @param mon Monitor
 * @param fd File descriptor of the source to remove
 * @return negative errno value on error, 0 otherwise
 */
static int remove_source_by_fd(struct io_mon *mon, int fd)
{
	struct io_src *src;

	if (NULL == mon || -1 == fd)
		return -EINVAL;
	src = find_source_by_fd(mon, fd);
	if (NULL == src)
		return 0;

	return remove_source(mon, src);
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
	/* backup in case the client cb destroys it */
	struct io_src backup_src = *src;

	/*
	 * if during processing, sources are altered, some events may
	 * have become irrelevant and must be filtered out
	 */
	if (!has_events_pending(src))
		return 0;
	src->cb(src);

	if (io_src_has_error(&backup_src))
		remove_source_by_fd(mon, backup_src.fd);

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

		/*
		 * a source can have been removed by a previous source's
		 * callback, in this case, we must skip it
		 */
		if (NULL == rs_node_find(mon->source.next, &src->node))
			continue;

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
	enum io_src_event old_active;
	if (NULL == mon || NULL == src || !(direction & src->type))
		return -EINVAL;

	old_active = src->active;
	if (active)
		src->active |= direction;
	else
		src->active &= ~direction;

	if (old_active == src->active)
		return 0;

	return alter_source(mon->epollfd, src, EPOLL_CTL_MOD);
}

/**
 * Source callback for integrating a libioutils monitor into another one
 * @param src Underlying source of the monitor
 */
static void mon_cb(struct io_src *src)
{
	struct io_mon *mon = ut_container_of(src, struct io_mon, src);

	io_mon_process_events(mon);
}

/**
 * Adds or removes a list of sources from/to a monitor
 * @param mon Monitor
 * @param add_remove action to take on the source
 * @param args NULL-terminated list of arguments
 * @return negative errno value on error, 0 otherwise
 */
static int add_remove_sources(struct io_mon *mon,
		int (*add_remove)(struct io_mon *mon, struct io_src *src),
		va_list args)
{
	struct io_src *src;
	int ret = 0;

	if (NULL == mon)
		return -EINVAL;

	do {
		src = va_arg(args, struct io_src *);
		if (NULL == src)
			break;
		ret = add_remove(mon, src);
		if (0 != ret)
			break;
	} while (1);

	return ret;
}

int io_mon_init(struct io_mon *mon)
{
	if (NULL == mon)
		return -EINVAL;

	memset(mon, 0, sizeof(*mon));
	mon->epollfd = io_epoll_create1(EPOLL_CLOEXEC);
	if (-1 == mon->epollfd)
		return -errno;

	return io_src_init(&mon->src, io_mon_get_fd(mon), IO_IN, mon_cb);
}

int io_mon_get_fd(struct io_mon *mon)
{
	if (NULL == mon)
		return -EINVAL;

	return mon->epollfd;
}

struct io_src *io_mon_get_source(struct io_mon *mon)
{
	if (NULL == mon)
		return NULL;

	return &mon->src;
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
	/*
	 * cast is ok because it can't change the value of type and it's needed
	 * because otherwise, there is a -Wsign-conversion warning
	 */
	src->active = (long)src->type & ~IO_OUT;

	return register_source(mon, src);
}

int io_mon_add_sources(struct io_mon *mon, ...)
{
	int ret;
	va_list args;

	va_start(args, mon);
	ret = add_remove_sources(mon, io_mon_add_source, args);
	va_end(args);

	return ret;
}

int io_mon_remove_source(struct io_mon *mon, struct io_src *src)
{
	if (NULL == mon || NULL == src)
		return -EINVAL;

	return remove_source(mon, src);
}

int io_mon_remove_sources(struct io_mon *mon, ...)
{
	int ret;
	va_list args;

	va_start(args, mon);
	ret = add_remove_sources(mon, io_mon_remove_source, args);
	va_end(args);

	return ret;
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

int io_mon_poll(struct io_mon *mon, int timeout)
{
	ssize_t n = 0;
	struct epoll_event events[MONITOR_MAX_SOURCES];

	if (NULL == mon)
		return -EINVAL;

	/* retrieve events */
	n = io_epoll_wait(mon->epollfd, events, MONITOR_MAX_SOURCES, timeout);
	if (-1 == n)
		return -errno;

	return do_process_events_sets(mon, n, events);
}

int io_mon_process_events(struct io_mon *mon)
{
	return io_mon_poll(mon, 0 /* don't block */);
}

int io_mon_clean(struct io_mon *mon)
{
	struct io_src *src;

	if (NULL == mon)
		return -EINVAL;

	while (mon->source.next) {
		src = to_src(mon->source.next);
		remove_source(mon, src);
	}

	if (-1 != mon->epollfd)
		ut_file_fd_close(&mon->epollfd);
	memset(mon, 0, sizeof(*mon));
	io_src_clean(&mon->src);
	mon->epollfd = -1;

	return 0;
}
