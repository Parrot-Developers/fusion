/**
 * @file io_src_msg.c
 * @date 24 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Source for reading fixed length messages
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <errno.h>
#include <string.h>

#include "io_mon.h"
#include "io_src_msg.h"
#include "io_utils.h"

/**
 * Receives a message and notifies it to the client
 * @param msg Message source
 * @param fd Source's file descriptor
 * @return errno compatible value on error, 0 on success
 */
static int in_msg(struct io_src_msg *msg, int fd)
{
	ssize_t sret;

	/* get some data */
	if (msg->perform_io) {
		sret = io_read(fd, msg->rcv_buf, msg->rcv_buf_size);
		if (-1 == sret)
			return -errno;
		if ((ssize_t)msg->rcv_buf_size != sret)
			return -EIO;
	} /* else, the source implementation performs input itself */

	msg->cb(msg, IO_IN);

	return 0;
}

/**
 * Asks the client, via it's callback, to provide a message, then writes it into
 * the source's file descriptor.
 * @param msg Message source
 * @param fd Source's file descriptor
 * @return errno compatible value on error, 0 on success
 */
static int out_msg(struct io_src_msg *msg, int fd)
{
	ssize_t sret;

	/* the client callback prepares us something to send */
	msg->cb(msg, IO_OUT);

	if (msg->perform_io) {
		/* msg->msg contains the message to send */
		/* casting to void * is needed because of write's prototype */
		sret = io_write(fd, (void *)msg->send_buf, msg->send_buf_size);
		if (-1 == sret)
			return -errno;
	} /* else, the source implementation performs output itself */

	return 0;
}

/**
 * Source callback, either performs in or out operation, depending on the event
 * type
 * @param src Underlying monitor source of the message source
 */
static void msg_cb(struct io_src *src)
{
	struct io_src_msg *msg = to_src_msg(src);

	/* TODO treat I/O THEN errors */
	if (io_src_has_error(src->events))
		return;

	if (io_src_has_in(src->events)) {
		in_msg(msg, src->fd);
		return;
	}

	out_msg(msg, src->fd);
}

int io_src_msg_set_next_message(struct io_src_msg *msg_src,
		const void *send_buf, unsigned send_buf_size)
{
	if (NULL == msg_src || NULL == send_buf)
		return -EINVAL;

	msg_src->send_buf = send_buf;
	msg_src->send_buf_size = send_buf_size;

	return 0;
}

int io_src_msg_get_message(struct io_src_msg *msg_src, void **msg)
{
	if (NULL == msg_src || NULL == msg)
		return -EINVAL;

	*msg = msg_src->rcv_buf;

	return 0;
}

int io_src_msg_init(struct io_src_msg *msg_src, int fd, enum io_src_event type,
		io_src_msg_cb_t *cb, void *rcv_buf,
		unsigned len, unsigned perform_io)
{
	if (NULL == msg_src || -1 == fd || NULL == rcv_buf || NULL == cb ||
			0 == len)
		return -EINVAL;

	memset(msg_src, 0, sizeof(*msg_src));

	msg_src->cb = cb;
	msg_src->rcv_buf = rcv_buf;
	msg_src->rcv_buf_size = len;
	msg_src->send_buf_size = 0;
	msg_src->perform_io = perform_io;

	/* can fail only on parameters */
	return io_src_init(&(msg_src->src), fd, type, msg_cb);
}

void io_src_msg_clean(struct io_src_msg *msg)
{
	if (NULL == msg)
		return;

	msg->cb = NULL;
	msg->rcv_buf_size = 0;
	msg->rcv_buf = NULL;
	msg->send_buf = NULL;
	msg->perform_io = 0;

	io_src_clean(&(msg->src));
}

