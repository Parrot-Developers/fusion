/**
 * @file io_mon.c
 * @date 16 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <unistd.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <io_src.h>
#include <io_mon.h>

#include "io_utils.h"

#define MONITOR_MAX_SOURCES 10

/**
 * @struct funk_monitor
 * @brief global monitor's context, handles the pool of sources and callbacks
 */
struct io_mon {
	/** file descriptor for monitoring all the sources */
	int epollfd;
	/** sources list for I/O operations */
	rs_node_t *source;
	/** number of sources currently registered */
	unsigned nb_sources;
};

/**
 * Adds a source to the monitor
 * @param monitor Monitor context
 * @param source Monitor's source
 * @return FUNK_ERROR_EXHAUSTION or FUNK_ERROR_PARAM
 */
static int add_source(io_mon_t *mon, io_src_t *src)
{
	int ret = -1;

	if (NULL == src->callback)
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
static int alter_source(int epfd, io_src_t *src, int op)
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
static int remove_source(io_mon_t *mon, io_src_t *src)
{
	int ret;
	rs_node_t *node;
	io_src_t *old_src;

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
static int register_source(io_mon_t *mon, io_src_t *src)
{
	if (NULL == mon || NULL == src)
		return -EINVAL;

	return alter_source(mon->epollfd, src, EPOLL_CTL_ADD);
}

io_mon_t *io_mon_new(void)
{
	io_mon_t *mon = NULL;

	/* allocate resources */
	mon = calloc(1, sizeof(*mon));
	if (NULL == mon)
		return NULL;
	memset(mon, 0, sizeof(*mon));

	mon->epollfd = epoll_create1(EPOLL_CLOEXEC);
	if (-1 == mon->epollfd)
		goto out;

	return mon;
out:
	io_mon_delete(&mon);

	return NULL;
}

int io_mon_add_source(io_mon_t *mon, io_src_t *src)
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

int io_mon_activate_out_source(io_mon_t *mon, io_src_t *src, int active)
{
	if (NULL == mon || NULL == src || !(IO_OUT & src->type))
		return -EINVAL;

	if (active)
		src->active |= IO_OUT;
	else
		src->active &= ~IO_OUT;

	return alter_source(mon->epollfd, src, EPOLL_CTL_MOD);
}

int io_mon_get_fd(io_mon_t *mon)
{
	if (NULL == mon)
		return -EINVAL;

	return mon->epollfd;
}

int io_mon_process_events(io_mon_t *mon)
{
	int n = 0;
	int i = 0;
	struct epoll_event events[MONITOR_MAX_SOURCES];
	int ret = 0;
	io_src_t *src = NULL;

	if (NULL == mon)
		return -EINVAL;

	/* doesn't block, because of the 0 timeout */
	n = epoll_wait(mon->epollfd, events, MONITOR_MAX_SOURCES, 0);
	if (-1 == n)
		return -errno;

	for (i = 0; i < n; i++) {
		src = events[i].data.ptr;
		src->events = events[i].events;
		/*
		 * if during processing, sources are altered, some events may
		 * have become irrelevant and must be filtered out
		 */
		if (!(src->active & src->events) && !(src->active & IO_EPOLL_ERROR_EVENTS))
			continue;

		ret = src->callback(src);
		if (0 != ret)
			perror("src->callback");

		if (0 > ret || (IO_EPOLL_ERROR_EVENTS & events[i].events)) {
			/*
			 * a negative return from the callback says the source
			 * must be removed. The removal is also forced when any
			 * I/O error occurs
			 */
			ret = remove_source(mon, src);
			if (0 != ret)
				return ret;
		}
		ret = 0;
	}

	return 0;
}

void io_mon_delete(io_mon_t **monitor)
{
	io_mon_t *mon;
	io_src_t *src;

	if (NULL == monitor || NULL == *monitor)
		return;
	mon = *monitor;

	while (mon->source) {
		src = to_src(mon->source);
		remove_source(mon, src);
	}

	close(mon->epollfd);
	free(mon);
	*monitor = NULL;
}
