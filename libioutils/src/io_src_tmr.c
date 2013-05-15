/**
 * @file io_tmr.c
 *
 * @brief mambo timer
 *
 * Copyright (C) 2011 Parrot S.A.
 *
 * @author Jean-Baptiste Dubois
 * @date May 2011
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/timerfd.h>

#include <unistd.h>

#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <errno.h>
#include <poll.h>

#include "io_platform.h"
#include "io_src_tmr.h"

/**
 * @def to_src
 * @brief Convert a source to it's signal source container
 */
#define to_src_tmr(p) container_of(p, struct io_timer, src)

/*
 * set a timer timeout in ms
 * */
static int io_timer_read(struct io_timer *timer, uint64_t *val)
{
	uint64_t u;
	ssize_t status;
	int ret = 0;

	status = read(timer->src.fd, &u, sizeof(u));
	if (status != sizeof(u)) {
		ret = -errno;
	} else if (val) {
		*val = u;
	}
	return ret;
}

static void tmr_cb(struct io_src *src)
{
	struct io_timer *timer = to_src_tmr(src);
	uint64_t nbexpired = 0;

	if (io_src_has_in(src->events)) {
		/* read timer value */
		io_timer_read(timer, &nbexpired);

		/* invoke timer callback */
		(*timer->cb)(timer, &nbexpired, timer->data);
	}

	/* is it possible for a timer fd ? */
	if (io_src_has_error(src->events)) {
		/* TODO do something ? */
	}

}

int io_timer_create(struct io_timer *timer, io_timer_cb_t cb, void *data)
{
	int fd;

	if (!timer || !cb)
		return -EINVAL;

	/* create timerfd with CLOCK_MONOTONIC clock*/
	fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (fd == -1)
		return -errno;

	timer->cb = cb;
	timer->data = data;
	timer->timeout = 0;

	return io_src_init(&timer->src, fd, IO_IN, &tmr_cb);
}

int io_timer_destroy(struct io_timer *timer)
{
	int ret = close(timer->src.fd);

	return ret == -1 ? errno : 0;
}

int io_timer_set(struct io_timer *timer, int timeout)
{
	int ret = 0;
	struct itimerspec nval, oval;

	/* configure one shot */
	nval.it_interval.tv_sec = 0;
	nval.it_interval.tv_nsec = 0;

	if (timeout > 0) {
		nval.it_value.tv_sec = timeout / MSEC_PER_SEC;
		nval.it_value.tv_nsec = (timeout % MSEC_PER_SEC) * NSEC_PER_SEC;
	} else {
		/* clear timer */
		nval.it_value.tv_sec = 0;
		nval.it_value.tv_nsec = 0;
	}
	ret = timerfd_settime(timer->src.fd, 0, &nval, &oval);
	if (ret == -1) {
		ret = -errno;
	} else {
		timer->timeout = timeout;
	}
	return ret;
}

int io_timer_clear(struct io_timer *timer)
{
	return io_timer_set(timer, 0);
}
