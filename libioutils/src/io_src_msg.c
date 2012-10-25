/**
 * @file io_src_msg.c
 * @date 24 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Source for reading fixed length messages
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <unistd.h>

#include <errno.h>
#include <string.h>

#include "io_mon.h"
#include "io_src_msg.h"

/**
 * @def to_src_msg
 * @brief Convert a source to it's message source container
 */
#define to_src_msg(p) container_of(p, struct io_src_msg, src)

/**
 * Source callback, reads the message and notifies the client
 * @param src Underlying monitor source of the message source
 */
static int msg_cb(struct io_src *src)
{
	ssize_t sret;
	struct io_src_msg *msg = to_src_msg(src);

	if (io_mon_has_error(src->events))
		return -EIO;

	/* get some data */
	sret = TEMP_FAILURE_RETRY(read(src->fd, msg->msg, msg->len));
	if (-1 == sret)
		return -errno;
	if ((ssize_t)msg->len != sret)
		return -EIO;

	return msg->cb(msg);
}

/**
 * Callback called when the source is removed
 * @param src Underlying monitor source of the signal source
 */
static void msg_clean(struct io_src *src)
{
	struct io_src_msg *msg;

	if (NULL == src)
		return;
	msg = to_src_msg(src);

	memset(msg, 0, sizeof(*msg));
	src->fd = -1;
}

int io_src_msg_init(struct io_src_msg *msg_src, int fd, void *msg, unsigned len,
	       io_src_msg_cb_t *cb)
{
	if (NULL == msg_src || -1 == fd || NULL == msg || NULL == cb ||
			0 == len)
		return -EINVAL;

	memset(msg_src, 0, sizeof(*msg_src));

	msg_src->cb = cb;
	msg_src->msg = msg;
	msg_src->len = len;

	/* can fail only on parameters */
	return io_src_init(&(msg_src->src), fd, IO_IN, msg_cb, msg_clean);
}
