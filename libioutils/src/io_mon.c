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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <io_src.h>
#include <io_mon.h>

#include "io_platform.h"
#include "io_utils.h"

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

	ret = set_non_blocking(src->fd);
	if (0 != ret)
		return ret;

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
	if (NULL == mon || NULL == src || !(IO_OUT & src->type))
		return -EINVAL;

	if (active)
		src->active |= IO_OUT;
	else
		src->active &= ~IO_OUT;

	return alter_source(mon->epollfd, src, EPOLL_CTL_MOD);
}

int io_mon_process_events(struct io_mon *mon)
{
	int n = 0;
	int i = 0;
	struct epoll_event events[MONITOR_MAX_SOURCES];
	int ret = 0;
	struct io_src *src = NULL;

	if (NULL == mon)
		return -EINVAL;

	n = TEMP_FAILURE_RETRY(epoll_wait(mon->epollfd, events,
			MONITOR_MAX_SOURCES,
			0 /* don't block */
			));
	if (-1 == n)
		return -errno;

	for (i = 0; i < n; i++) {
		src = events[i].data.ptr;
		src->events = events[i].events;
		/*
		 * if during processing, sources are altered, some events may
		 * have become irrelevant and must be filtered out
		 */
		if (!(src->events & (src->active | IO_EPOLL_ERROR_EVENTS)))
			continue;

		ret = src->cb(src);
		if (0 != ret)
			fprintf(stderr, "src->cb : %s\n", strerror(abs(ret)));

		/*
		 * a negative return from the callback says the source must be
		 * removed. The removal is also forced when any I/O error occur
		 */
		if (0 > ret || (io_mon_has_error(events[i].events))) {
			ret = remove_source(mon, src);
			if (0 != ret)
				return ret;

			/*
			 * cleanup cb must be done AFTER unchaining so that the
			 * client can do what he wants of it's context
			 */
			io_src_clean(src);
		}
		ret = 0;
	}

	return 0;
}

int io_mon_clean(struct io_mon *mon)
{
	struct io_src *src;

	if (NULL == mon)
		return -EINVAL;

	while (mon->source) {
		src = to_src(mon->source);
		remove_source(mon, src);
		io_src_clean(src);
	}

	if (-1 != mon->epollfd)
		close(mon->epollfd);
	memset(mon, 0, sizeof(*mon));
	mon->epollfd = -1;

	return 0;
}
