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

#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <ut_utils.h>
#include <ut_string.h>
#include <ut_file.h>

#include <io_utils.h>

#include "io_platform.h"
#include "io_mon.h"
#include "io_src_msg_uad.h"

/**
 * @def to_src_msg_uad
 * @brief Convert a source to it's message source container
 */
#define to_src_msg_uad(p) ut_container_of(p, struct io_src_msg_uad, src_msg)

/**
 * Performs input
 * @param uad Source
 * @return errno compatible value on error, 0 on success
 */
static int process_in_event(struct io_src_msg_uad *uad)
{
	ssize_t sret;

	sret = io_recvfrom(uad->src_msg.src.fd, uad->src_msg.rcv_buf,
			uad->src_msg.rcv_buf_size, 0, NULL, NULL);
	if (-1 == sret)
		return -errno;

	uad->cb(uad, IO_IN);

	return 0;
}

/**
 * Performs output
 * @param uad Source
 * @return errno compatible value on error, 0 on success
 */
static int process_out_event(struct io_src_msg_uad *uad)
{
	ssize_t sret;

	uad->cb(uad, IO_OUT);

	sret = io_sendto(uad->src_msg.src.fd, uad->src_msg.send_buf,
			uad->src_msg.rcv_buf_size, 0,
			(const struct sockaddr *)&(uad->addr),
			sizeof(uad->addr));
	if (-1 == sret)
		return -errno;

	return 0;
}

/**
 * Performs I/O, after arguments are already verified
 * @param uad Source
 * @param evt Either IO_IN or IO_OUT, not both
 * @return errno compatible value on error, 0 on success
 */
static int process_event(struct io_src_msg_uad *uad, enum io_src_event evt)
{
	/* here evt is either IO_IN or IO_OUT, not both */
	/* coverity[mixed_enums] */
	return IO_IN == evt ? process_in_event(uad) : process_out_event(uad);
}

/**
 * Called when the underlying io_src_msg has I/O ready. Performs I/O via
 * recvfrom / sendto
 * @param src Source
 * @param evt Either IO_IN or IO_OUT, not both
 */
static void uad_cb(struct io_src_msg *src, enum io_src_event evt)
{
	struct io_src_msg_uad *uad = to_src_msg_uad(src);

	/* coverity[mixed_enums] */
	if (IO_IN != evt && IO_OUT != evt)
		return;

	process_event(uad, evt);
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

	uad->cb = cb;

	/* can fail only on parameters */
	return io_src_msg_init(&(uad->src_msg), sockfd, IO_DUPLEX, uad_cb,
			rcv_buf, len, 0);

out:
	ut_file_fd_close(&sockfd);

	return ret;
}

void io_src_msg_uad_clean(struct io_src_msg_uad *uad)
{
	if (NULL == uad)
		return;

	uad->cb = NULL;
	memset(&(uad->addr), 0, sizeof(uad->addr));
	shutdown(uad->src_msg.src.fd, SHUT_RDWR);

	ut_file_fd_close(&uad->src_msg.src.fd);

	io_src_msg_clean((&uad->src_msg));
}

