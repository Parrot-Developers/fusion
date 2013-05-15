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
#include <sys/socket.h>

#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "io_mon.h"
#include "io_src_msg_uad.h"
#include "io_utils.h"

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
 */
static void uad_cb(struct io_src_msg *src, enum io_src_event evt)
{
	struct io_src_msg_uad *uad = to_src_msg_uad(src);

	if (IO_IN != evt && IO_OUT != evt)
		return;

	uad->cb(uad, evt);
}

static int uad_init_args_are_invalid(struct io_src_msg_uad *uad,
		io_src_msg_uad_cb_t *cb, void *rcv_buf, unsigned len,
		const char *fmt)
{
	return NULL == uad || NULL == cb || NULL == rcv_buf || 0 == len ||
			NULL == fmt || '\0' == *fmt;
}

int io_src_msg_uad_set_next_message(struct io_src_msg_uad *uad,
		const void *rcv_buf, unsigned send_buf_size)
{
	if (NULL == uad)
		return -EINVAL;

	return io_src_msg_set_next_message(&(uad->src_msg), rcv_buf,
			send_buf_size);
}

int io_src_msg_uad_get_message(struct io_src_msg_uad *uad, void **msg)
{
	if (NULL == uad || NULL == msg)
		return -EINVAL;

	return io_src_msg_get_message(&(uad->src_msg), msg);
}

int io_src_msg_uad_init(struct io_src_msg_uad *uad, io_src_msg_uad_cb_t *cb,
		void *rcv_buf, unsigned len, const char *fmt, ...)
{
	int sockfd;
	va_list args;
	int ret;

	if (uad_init_args_are_invalid(uad, cb, rcv_buf, len, fmt))
		return -EINVAL;

	memset(uad, 0, sizeof(*uad));

	sockfd = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (sockfd < 0)
		return -errno;

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

	ret = connect(sockfd, (struct sockaddr *) &(uad->addr),
			sizeof(uad->addr));
	if (ret < 0) {
		ret = -errno;
		goto out;
	}

	uad->cb = cb;

	/* can fail only on parameters */
	return io_src_msg_init(&(uad->src_msg), sockfd, IO_DUPLEX, uad_cb,
			rcv_buf, len, 1);

out:
	close(sockfd);

	return ret;
}

void io_src_msg_uad_clean(struct io_src_msg_uad *uad)
{
	if (NULL == uad)
		return;

	uad->cb = NULL;
	memset(&(uad->addr), 0, sizeof(uad->addr));
	shutdown(uad->src_msg.src.fd, SHUT_RDWR);

	io_src_msg_clean((&uad->src_msg));
}

