/******************************************************************************
* @file mb_timer.c
*
* @brief mambo timer
*
* Copyright (C) 2011 Parrot S.A.
*
* @author Jean-Baptiste Dubois
* @date May 2011
******************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <errno.h>
#include <poll.h>
#include "mb_log.h"
#include "mb_list.h"
#include "mb_platform.h"
#include "mb_time.h"
#include "mb_fd_loop.h"
#include "mb_timer.h"

/*
 * set a timer timeout in ms
 * */
static int mb_timer_read(struct mb_timer *timer, uint64_t *val)
{
	uint64_t u;
	ssize_t status;
	int ret = 0;

	status = read(timer->mbfd.fd, &u, sizeof(u));
	if (status != sizeof(u)) {
		mb_log_fd_errno("read", timer->mbfd.fd);
		ret = -errno;
	} else if (val) {
		*val = u;
	}
	return ret;
}

static void mb_timer_events(struct mb_fd *mbfd, int events, void *data)
{
	struct mb_timer *timer = (struct mb_timer *)data;
	uint64_t nbexpired = 0;

	/* is it possible for a timer fd ? */
	if (mb_fd_event_error(events)) {
		/* remove source from loop */
		mb_fd_loop_remove(timer->loop, &timer->mbfd);
		mb_fd_reset(&timer->mbfd);
	}

	if (!mb_fd_event_read(events))
		return;

	/* read timer value */
	mb_timer_read(timer, &nbexpired);

	/* invoke timer callback */
	(*timer->cb) (timer, &nbexpired, timer->data);
}

int mb_timer_create(struct mb_timer *timer, struct mb_fd_loop *loop,
		    mb_timer_cb_t cb, void *data)
{
	int fd, ret = 0;

	if (!timer || !loop || !cb) {
		ret = -EINVAL;
		goto error;
	}

	/* create timerfd with CLOCK_MONOTONIC clock*/
	fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (fd == -1) {
		ret = -errno;
		mb_log_errno("timerfd_create");
		goto error;
	}

	mb_fd_init(&timer->mbfd, fd, MB_FD_IN, &mb_timer_events, timer);

	/* add fd timer object in loop */
	ret = mb_fd_loop_add(loop, &timer->mbfd);
	if (ret < 0)
		goto destroy_tfd;

	timer->cb = cb;
	timer->data = data;
	timer->timeout = 0;
	timer->loop = mb_fd_loop_ref(loop);
	return 0;

destroy_tfd:
	close(fd);
error:
	mb_log_error(ret);
	return ret;
}

int mb_timer_destroy(struct mb_timer *timer)
{
	int ret;

	mb_fd_loop_remove(timer->loop, &timer->mbfd);
	ret = close(timer->mbfd.fd);
	mb_fd_reset(&timer->mbfd);
	if (ret == -1) {
		ret = -errno;
		mb_log_fd_errno("close", timer->mbfd.fd);
	}
	mb_fd_loop_unref(timer->loop);
	return ret;
}

int mb_timer_set(struct mb_timer *timer, int timeout)
{
	int ret = 0;
	struct itimerspec nval, oval;

	/* configure one shot */
	nval.it_interval.tv_sec = 0;
	nval.it_interval.tv_nsec = 0;

	if (timeout > 0) {
		nval.it_value.tv_sec = timeout / MSEC_PER_SEC;
		nval.it_value.tv_nsec = (timeout % MSEC_PER_SEC)*NSEC_PER_SEC;
	} else {
		/* clear timer */
		nval.it_value.tv_sec = 0;
		nval.it_value.tv_nsec = 0;
	}
	ret = timerfd_settime(timer->mbfd.fd, 0, &nval, &oval);
	if (ret == -1) {
		ret = -errno;
		mb_log_fd_errno("timerfd_settime", timer->mbfd.fd);
	} else {
		timer->timeout = timeout;
	}
	return ret;
}

int mb_timer_clear(struct mb_timer *timer)
{
	return mb_timer_set(timer, 0);
}
