/**
 * @file io_tmr.c
 *
 * @brief Timer io source, imported and adapted from mambo
 *
 * Copyright (C) 2011 Parrot S.A.
 *
 * @author Jean-Baptiste Dubois
 * @author nicolas.carrier@parrot.com
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

#include <rs_utils.h>

#include "io_platform.h"
#include "io_utils.h"
#include "io_src_tmr.h"

/* useful time ratio value */
#define MSEC_PER_SEC  1000
#define NSEC_PER_MSEC 1000000

/**
 * @def to_tmr_src
 * @brief Convert a source to it's timer source container
 */
#define to_tmr_src(p) rs_container_of(p, struct io_src_tmr, src)

/**
 * Read a value from a timer which has expired
 * @param tmr Time to read the value of
 * @param val In output, value read. Can't be NULL, or you will suffer great
 * pain
 * @return errno compatible negative value on error, 0 on success
 */
static int tmr_read(struct io_src_tmr *tmr, uint64_t *val)
{
	ssize_t sret;
	int ret = 0;

	sret = io_read(tmr->src.fd, val, sizeof(*val));
	if (sret != sizeof(*val))
		ret = -errno;

	return ret;
}

/**
 * io_str calback for io_tmr_src events
 * @param src Underlying io source
 */
static void tmr_cb(struct io_src *src)
{
	struct io_src_tmr *tmr = to_tmr_src(src);
	uint64_t nbexpired = 0;

	if (NULL == src)
		return;

	if (io_src_has_in(src->events)) {
		/* read timer value */
		tmr_read(tmr, &nbexpired);

		/* invoke timer callback */
		tmr->cb(tmr, &nbexpired);
	}

	/* nothing to do on epoll error, source is automatically removed */
}

int io_src_tmr_init(struct io_src_tmr *tmr, io_tmr_cb_t cb)
{
	int fd;

	if (NULL == tmr || NULL == cb)
		return -EINVAL;

	/* create timerfd with CLOCK_MONOTONIC clock*/
	fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (fd == -1)
		return -errno;

	tmr->cb = cb;

	return io_src_init(&tmr->src, fd, IO_IN, &tmr_cb);
}

void io_src_tmr_clean(struct io_src_tmr *tmr)
{
	if (NULL == tmr)
		return;

	tmr->cb = NULL;
	io_close(&tmr->src.fd);

	io_src_clean(&(tmr->src));
}

int io_src_tmr_set(struct io_src_tmr *tmr, int timeout)
{
	int ret = 0;
	struct itimerspec nval = {
			.it_value = { /* disarm */
					.tv_sec = 0,
					.tv_nsec = 0,
			},
			.it_interval = { /* one shot */
					.tv_sec = 0,
					.tv_nsec = 0,
			},
	};

	if (NULL == tmr)
		return -EINVAL;

	if (IO_SRC_TMR_DISARM != timeout) {
		nval.it_value.tv_sec = timeout / MSEC_PER_SEC;
		nval.it_value.tv_nsec = ((long long)timeout % MSEC_PER_SEC) *
				NSEC_PER_MSEC;
		nval.it_interval.tv_sec = timeout / MSEC_PER_SEC;
		nval.it_interval.tv_nsec = ((long long)timeout % MSEC_PER_SEC) *
				NSEC_PER_MSEC;
	} /* else, disarm */

	ret = timerfd_settime(tmr->src.fd, 0, &nval, NULL);
	if (ret == -1)
		ret = -errno;

	return ret;
}
