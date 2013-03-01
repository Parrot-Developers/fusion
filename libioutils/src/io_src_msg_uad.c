/**
 * @file io_src_msg_uad.c
 * @date 24 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Source for reading / writing fixed length messages
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <sys/socket.h>

#include "io_mon.h"
#include "io_src_msg_uad.h"

/**
 * @def to_src_msg_uad
 * @brief Convert a source to it's message source container
 */
#define to_src_msg_uad(p) container_of(p, struct io_src_msg_uad, src_msg)

/**
 * Called when the underlying io_src_msg has I/O ready. Performs I/O via
 * recvfrom / sendto
 * @param src Source
 * @param evt Either IO_IN or IO_OUT, not both
 * @return errno compatible value, positive for only a warning, negative if the
 * source must be removed, 0 on success
 */
static int uad_cb(struct io_src_msg *src, enum io_src_event evt)
{
	int ret;
	ssize_t sret;
	struct io_src_msg_uad *uad = to_src_msg_uad(src);

	if (IO_IN != evt && IO_OUT != evt)
		return -EINVAL;

	if (IO_IN == evt) {
		sret = TEMP_FAILURE_RETRY(recvfrom(src->src.fd, src->rcv_buf,
				src->len, 0, NULL, NULL));
		if (-1 == sret)
			return -errno;
	}
	ret = uad->cb(uad, evt);
	if (0 > ret)
		return ret;
	if (IO_OUT == evt) {
		sret = TEMP_FAILURE_RETRY(sendto(src->src.fd, src->send_buf,
				src->len, 0,
				(const struct sockaddr *)&(uad->addr),
				sizeof(uad->addr)));
		if (-1 == sret)
			return -errno;
	}

	return ret;
}

/**
 * Clean callback, called on io_src_clean, automatically on source removal due
 * to error and monitor clean. Resets all the sources fields then calls user
 * io_src_msg_uad_clean_t callback
 * @param src Source to clean, previously cleaned by io_src_clean (i.e. with fd
 * already closed)
 */
static void uad_clean(struct io_src_msg *src)
{
	struct io_src_msg_uad *uad = to_src_msg_uad(src);

	uad->cb = NULL;
	memset(&(uad->addr), 0, sizeof(uad->addr));

	if (uad->clean)
		uad->clean(uad);

	uad->clean = NULL;
	/*
	 * TODO there is no way to properly shutdown() the socket before close
	 * which is mandatory. A change in the API is needed there
	 */
}

int io_src_msg_uad_set_next_message(struct io_src_msg_uad *uad,
		const void *rcv_buf)
{
	if (NULL == uad)
		return -EINVAL;

	return io_src_msg_set_next_message(&(uad->src_msg), rcv_buf);
}

int io_src_msg_uad_get_message(struct io_src_msg_uad *uad, void **msg)
{
	if (NULL == uad || NULL == msg)
		return -EINVAL;

	return io_src_msg_get_message(&(uad->src_msg), msg);
}

int io_src_msg_uad_init(struct io_src_msg_uad *uad, io_src_msg_uad_cb_t *cb,
		io_src_msg_uad_clean_t *clean, void *rcv_buf, unsigned len,
		const char *fmt, ...)
{
	int sockfd;
	va_list args;
	int ret;

	if (NULL == uad || NULL == rcv_buf || 0 == len || NULL == cb ||
			NULL == fmt || '\0' == *fmt)
		return -EINVAL;

	memset(uad, 0, sizeof(*uad));

	sockfd = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (sockfd < 0) {
		ret = -errno;
		goto out;
	}

	uad->addr.sun_path[0] = '\0';
	va_start(args, fmt);
	vsnprintf(uad->addr.sun_path + 1, UNIX_PATH_MAX - 1, fmt, args);
	va_end(args);
	uad->addr.sun_family = AF_UNIX;
	ret = bind(sockfd, (struct sockaddr *) &(uad->addr),
			sizeof(uad->addr));
	if (ret < 0) {
		ret = -errno;
		goto out;
	}

	uad->cb = cb;
	uad->clean = clean;

	/* can fail only on parameters */
	return io_src_msg_init(&(uad->src_msg), sockfd, IO_DUPLEX, uad_cb,
			uad_clean, rcv_buf, len, 0);

out:
	if (-1 != sockfd)
		close(sockfd);

	return ret;
}
