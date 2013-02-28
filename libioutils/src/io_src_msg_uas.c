/**
 * @file io_src_msg_uas.c
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
#include "io_src_msg_uas.h"

/**
 * @def to_src_msg_uas
 * @brief Convert a source to it's message source container
 */
#define to_src_msg_uas(p) container_of(p, struct io_src_msg_uas, src_msg)

/**
 * Called when the underlying io_src_msg has I/O ready. Performs I/O via
 * recvfrom / sendto
 * @param src Source
 * @param evt Either IO_IN or IO_OUT, not both
 * @return errno compatible value, positive for only a warning, negative if the
 * source must be removed, 0 on success
 */
static int uas_cb(struct io_src_msg *src, enum io_src_event evt)
{
	int ret;
	ssize_t sret;
	struct io_src_msg_uas *msg_uas_src = to_src_msg_uas(src);

	if (IO_IN != evt && IO_OUT != evt)
		return -EINVAL;

	if (IO_IN == evt) {
		sret = TEMP_FAILURE_RETRY(recvfrom(src->src.fd, src->rcv_buf,
				src->len, 0, NULL, NULL));
		if (-1 == sret)
			return -errno;
	}
	ret = msg_uas_src->cb(msg_uas_src, evt);
	if (0 > ret)
		return ret;
	if (IO_OUT == evt) {
		sret = TEMP_FAILURE_RETRY(sendto(src->src.fd, src->send_buf,
				src->len, 0,
				(const struct sockaddr *)&(msg_uas_src->addr),
				sizeof(msg_uas_src->addr)));
		if (-1 == sret)
			return -errno;
	}

	return ret;
}

/**
 * Clean callback, called on io_src_clean, automatically on source removal due
 * to error and monitor clean. Resets all the sources fields then calls user
 * io_src_msg_uas_clean_t callback
 * @param src Source to clean, previously cleaned by io_src_clean (i.e. with fd
 * already closed)
 */
static void uas_clean(struct io_src_msg *src)
{
	struct io_src_msg_uas *uas = to_src_msg_uas(src);

	uas->cb = NULL;
	memset(&(uas->addr), 0, sizeof(uas->addr));

	if (uas->clean)
		uas->clean(uas);

	uas->clean = NULL;
	/*
	 * TODO there is no way to properly shutdown() the socket before close
	 * which is mandatory. A change in the API is needed there
	 */
}

int io_src_msg_uas_set_next_message(struct io_src_msg_uas *uas_src,
		const void *rcv_buf)
{
	if (NULL == uas_src)
		return -EINVAL;

	return io_src_msg_set_next_message(&(uas_src->src_msg), rcv_buf);
}

int io_src_msg_uas_get_message(struct io_src_msg_uas *uas_src, void **msg)
{
	if (NULL == uas_src || NULL == msg)
		return -EINVAL;

	return io_src_msg_get_message(&(uas_src->src_msg), msg);
}

int io_src_msg_uas_init(struct io_src_msg_uas *uas_src, io_src_msg_uas_cb_t *cb,
		io_src_msg_uas_clean_t *clean, void *rcv_buf, unsigned len,
		const char *fmt, ...)
{
	int sockfd;
	va_list args;
	int ret;

	if (NULL == uas_src || NULL == rcv_buf || 0 == len || NULL == cb ||
			NULL == fmt || '\0' == *fmt)
		return -EINVAL;

	memset(uas_src, 0, sizeof(*uas_src));

	sockfd = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (sockfd < 0) {
		ret = -errno;
		goto out;
	}

	uas_src->addr.sun_path[0] = '\0';
	va_start(args, fmt);
	vsnprintf(uas_src->addr.sun_path + 1, UNIX_PATH_MAX - 1, fmt, args);
	va_end(args);
	uas_src->addr.sun_family = AF_UNIX;
	ret = bind(sockfd, (struct sockaddr *) &(uas_src->addr),
			sizeof(uas_src->addr));
	if (ret < 0) {
		ret = -errno;
		goto out;
	}

	uas_src->cb = cb;
	uas_src->clean = clean;

	/* can fail only on parameters */
	return io_src_msg_init(&(uas_src->src_msg), sockfd, IO_DUPLEX, uas_cb,
			uas_clean, rcv_buf, len, 0);

out:
	if (-1 != sockfd)
		close(sockfd);

	return ret;
}
