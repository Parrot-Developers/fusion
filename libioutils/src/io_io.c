/**
 * @file io_io.c
 * @brief Duplex io source with managed reads and writes
 *
 * @date May 2011
 * @author Jean-Baptiste Dubois
 * @copyright Copyright (C) 2011 Parrot S.A.
 */

/* TODO update doc of static functions and other private symbols */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include <ut_string.h>

#include <io_src_tmr.h>

#include "io_io.h"

/**
 *
 * @param log_cb
 * @param func
 * @param buffer
 * @param length
 * @param fmt
 */
static void io_log_raw(void (*log_cb)(const char *), const char *func,
		const void *buffer, size_t length, const char *fmt, ...)
{
	static const char hexdigits[] = "0123456789ABCDEF";
	static char log_buf[512];
	va_list args;
	uint8_t byte;
	size_t n = 0, p = 0;
	size_t i;

	if (!log_cb)
		return;

	/* log "header" */
	va_start(args, fmt);
	vsnprintf(log_buf, 512, fmt, args);
	va_end(args);
	log_buf[511] = '\0';
	(*log_cb)(log_buf);

	/* log format */
	/* xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
	 * .................*/
	for (i = 0; i < length; i++) {
		byte = ((const uint8_t *)buffer)[i] & 0xff;
		log_buf[((n % 16) * 3)] = hexdigits[byte >> 4];
		log_buf[((n % 16) * 3) + 1] = hexdigits[byte & 0xf];
		log_buf[((n % 16) * 3) + 2] = ' ';
		if (isprint(byte))
			log_buf[(n % 16) + 51] = byte;
		else
			log_buf[(n % 16) + 51] = '.';

		if ((n + 1) % 16 == 0) {
			log_buf[48] = ' ';
			log_buf[49] = '|';
			log_buf[50] = ' ';
			log_buf[67] = '\0';
			log_cb(log_buf);
		}
		n++;
	}

	if (n % 16 > 0) {
		p = (n % 16);
		while (p < 16) {
			log_buf[((p % 16) * 3)] = ' ';
			log_buf[((p % 16) * 3) + 1] = ' ';
			log_buf[((p % 16) * 3) + 2] = ' ';
			log_buf[(p % 16) + 51] = ' ';
			p++;
		}
		log_buf[48] = ' ';
		log_buf[49] = '|';
		log_buf[50] = ' ';
		log_buf[67] = '\0';
		log_cb(log_buf);
	}
}

/**
 *
 * @param fd
 * @param ign_eof
 * @param log_cb
 * @param name
 * @param buffer
 * @param size
 * @param length
 * @return
 */
static int read_io(int fd, int ign_eof, void (*log_cb)(const char *),
		const char *name, void *buffer, size_t size, size_t *length)
{
	ssize_t nbytes;

	*length = 0;
	/* read without blocking */
	do {
		nbytes = read(fd, buffer, size);
	} while (nbytes == -1 && errno == EINTR);

	if (nbytes == -1)
		return -errno;
	if (0 != nbytes) {
		*length = (size_t)(nbytes);

		/* log data read */
		if (log_cb)
			io_log_raw(log_cb, __func__, buffer, *length,
					"%s read fd=%d length=%d", name, fd,
					*length);
	}

	return 0;
}

/**
 *
 * @param read_src
 */
static void read_src_cb(struct io_src *read_src)
{
	struct io_io *io = ut_container_of(read_src, struct io_io, src);
	struct io_io_read_ctx *readctx = &io->readctx;
	size_t length = 0;
	void *buffer;
	size_t size;
	int cbret = 0;
	int eof = 0;
	int ret = 0;
	int fd = io_src_get_fd(read_src);

	/* remove source from loop on error */
	if (io_src_has_error(read_src))
		/*
		 * TODO change for an explicit value other than EAGAIN, e.g EIO
		 */
		ret = -1;

	/* do not treat event other than read available */
	if (!io_src_has_in(read_src))
		return;

	/* read until no more space in ring buffer or read error */
	while (ret == 0 && !eof && rs_rb_get_write_length(&readctx->rb) > 0) {
		buffer = rs_rb_get_write_ptr(&readctx->rb);
		size = rs_rb_get_write_length_no_wrap(&readctx->rb);
		assert(size > 0);
		ret = read_io(fd, io->readctx.ign_eof, io->log_rx, io->name,
				buffer, size, &length);

		/* check if first part of ring buffer is full-filled */
		if (ret == 0 && length > 0) {
			rs_rb_write_incr(&readctx->rb, length);
			/* if free space available in ring buffer read again */
			if (rs_rb_get_write_length(&readctx->rb) > 0)
				continue;

		} else if (ret == 0 && length == 0) {
			/* end of file */
			eof = 1;
		}

		/* notify client if new bytes available */
		/* TODO move this out of the loop for notifying only once ? */
		if (rs_rb_get_read_length(&readctx->rb) > 0) {
			cbret = (*readctx->cb)(io, &readctx->rb, readctx->data);
			/* continue only if client need more data */
			if (cbret != 0)
				return;
		}
	}

	/* log something if read buffer is full */
	if (rs_rb_get_write_length(&readctx->rb) == 0) {
		/* TODO replace with a client notification */
/*		at_log_warn("%s fd=%d, io read buffer(%dB) full, data lost!",
				io->name, fd, rs_rb_get_size(&readctx->rb)); */
		rs_rb_empty(&readctx->rb);
	}

	/* remove source if end of file or read error
	 * (other than no more data!)
	 */
	if ((eof && !io->readctx.ign_eof) || (ret < 0 && ret != -EAGAIN)) {
		io_mon_remove_source(io->mon, read_src);
		io_src_clean(read_src);
		/* update state and notify client */
		readctx->state = IO_IO_ERROR;
		(*readctx->cb)(io, &readctx->rb, readctx->data);
	}
}

/**
 *
 * @param fd
 * @param log_cb
 * @param name
 * @param buffer
 * @param size
 * @param length
 * @return
 */
static int write_io(int fd, void (*log_cb)(const char *), const char *name,
		const void *buffer, size_t size, size_t *length)
{
	int ret = 0;
	ssize_t nbytes;

	*length = 0;
	/* write without blocking */
	do {
		nbytes = write(fd, buffer, size);
	} while (nbytes == -1 && errno == EINTR);

	if (nbytes == -1)
		return -errno;
	*length = (size_t)(nbytes);

	/* log data written */
	if (NULL != log_cb)
		io_log_raw(log_cb, __func__, buffer, *length,
				"%s written fd=%d length=%d", name, fd,
				*length);


	return ret;
}

/**
 *
 * @param io
 */
static void process_next_write(struct io_io *io)
{
	struct rs_node *first;
	struct io_io_write_ctx *ctx = &io->writectx;

	/* reset current buffer info */
	ctx->current = NULL;
	ctx->nbwritten = 0;
	ctx->nbeagain = 0;

	first = rs_dll_pop(&ctx->buffers);
	if (first) {
		/* get next write buffer */
		ctx->current = ut_container_of(first, struct io_io_write_buffer,
				node);
		/* add fd object in loop if not already done */
		io_mon_activate_out_source(io->mon, io->write_src, 1);

		/* set write timer */
		io_src_tmr_set(&ctx->timer, ctx->timeout);
	} else {
		/* no more buffer, clear timer */
		io_src_tmr_set(&ctx->timer, IO_SRC_TMR_DISARM);
		/* remove fd object if added */
		io_mon_activate_out_source(io->mon, io->write_src, 0);
	}
}

/**
 *
 * @param timer
 * @param nbexpired
 */
static void write_timer_cb(struct io_src_tmr *timer, uint64_t *nbexpired)
{
	struct io_io_write_ctx *ctx = ut_container_of(timer,
			struct io_io_write_ctx, timer);
	struct io_io *io = ut_container_of(ctx, struct io_io, writectx);
	struct io_io_write_buffer *buffer;

	/* get current write buffer */
	buffer = ctx->current;
	if (!buffer) {
		io_src_tmr_set(&ctx->timer, IO_SRC_TMR_DISARM);
		return;
	}

	/* process next buffer */
	process_next_write(io);

	/* notify buffer cb */
	(*buffer->cb)(buffer, IO_IO_WRITE_TIMEOUT);
}

/**
 *
 * @param src
 */
static void write_src_cb(struct io_src *src)
{
	struct io_io_write_ctx *writectx = ut_container_of(src,
			struct io_io_write_ctx, src);
	struct io_io *io = ut_container_of(writectx, struct io_io, writectx);
	struct io_io_write_buffer *buffer;
	enum io_io_write_status status;
	size_t length = 0;
	int ret = 0;
	struct io_src *write_src = io->write_src;

	/* remove source from loop on error */
	if (io_src_has_error(write_src)) {
		io_mon_remove_source(io->mon, write_src);
		io_src_clean(write_src);
		return;
	}

	/* do not treat event other than write ready */
	if (!io_src_has_out(write_src))
		return;

	/* get current write buffer */
	buffer = writectx->current;
	if (!buffer) { /* TODO can this really happen ? replace by an assert? */
		io_mon_activate_out_source(io->mon, write_src, 0);
		return;
	}

	/* write current buffer */
	while (ret == 0 && writectx->nbwritten < buffer->length) {
		ret = write_io(write_src->fd, io->log_tx, io->name,
				(const uint8_t *) buffer->address
						+ writectx->nbwritten,
				buffer->length - writectx->nbwritten, &length);
		if (ret == 0) {
			/* clear eagain flags */
			writectx->nbeagain = 0;
			writectx->nbwritten += length;
		} else if (ret == -EAGAIN) {
			writectx->nbeagain++;
		}
	}

	/* if we received more than 20 EAGAIN with POLLOUT set,
	 * a PROBLEM occurs in driver */
	if (writectx->nbeagain >= 20)
		ret = -ENOBUFS;

	/* wait for a next write ready if needed else buffer process completed*/
	if (ret != -EAGAIN) {

		process_next_write(io);

		/* notify buffer cb */
		status = ret == 0 ? IO_IO_WRITE_OK : IO_IO_WRITE_ERROR;
		(*buffer->cb)(buffer, status);
	}
}

/**
 *
 * @param buffer
 * @param status
 */
static void default_write_cb(struct io_io_write_buffer *buffer,
	enum io_io_write_status status)
{

}

/**
 *
 * @param src
 */
static void duplex_src_cb(struct io_src *src)
{
	struct io_io *io = ut_container_of(src, struct io_io, src);

	/* TODO errors ? */

	if (io_src_has_in(src))
		read_src_cb(src);

	if (io_src_has_out(src))
		write_src_cb(&io->writectx.src);
}

int io_io_init(struct io_io *io, struct io_mon *mon, const char *name,
		int fd_in, int fd_out, int ign_eof)
{
	int ret;
	int duplex = fd_in == fd_out;
	enum io_src_event source_type = duplex ? IO_DUPLEX : IO_IN;

	if (NULL == io || NULL == mon || ut_string_is_invalid(name))
		return -EINVAL;

	if (fd_in < 0 || fd_out < 0)
		return -EINVAL;

	memset(io, 0, sizeof(*io));

	/* disable io log by default */
	io->log_rx = NULL;
	io->log_tx = NULL;

	/* create a magic ring buffer for reads, the size should be 4096 */
	ret = rs_rb_init(&io->readctx.rb, NULL, IO_IO_RB_BUFFER_SIZE);
	if (ret < 0)
		return ret;

	/* TODO split out creation/initialization of read and write contexts */

	/* add read fd object */
	io_src_init(&io->src, fd_in, source_type, &duplex_src_cb);

	/* set read state to disable */
	io->readctx.state = IO_IO_STOPPED;
	io->readctx.cb = NULL;
	io->readctx.data = NULL;
	io->readctx.ign_eof = ign_eof;

	/* create write timer */
	ret = io_src_tmr_init(&io->writectx.timer, &write_timer_cb);
	if (ret < 0)
		goto free_rb;

	io->mon = mon;
	io->writectx.current = NULL;
	io->writectx.nbwritten = 0;

	/* init write buffer queue */
	rs_dll_init(&io->writectx.buffers, NULL);

	/* set default write ready timeout to 10s */
	io->writectx.timeout = 10000;
	io->writectx.state = IO_IO_STARTED;
	io->name = strdup(name);

	ret = io_mon_add_sources(mon,
			io_src_tmr_get_source(&io->writectx.timer),
			&io->src,
			NULL /* NULL guard */
			);
	if (0 != ret)
		goto free_rb;

	if (duplex) {
		io->write_src = &io->src;
	} else {
		/* create a separate write source with fd_out, only if needed */
		io_src_init(&io->writectx.src, fd_out, IO_OUT, &write_src_cb);
		ret = io_mon_add_source(mon, &io->writectx.src);
		if (0 != ret)
			goto free_rb;
		io->write_src = &io->writectx.src;
	}

	return 0;

	/* TODO better error handling */
free_rb:
	rs_rb_clean(&io->readctx.rb);

	return ret;
}

int io_io_clean(struct io_io *io)
{
	if (NULL == io)
		return -EINVAL;

	/* stop read if started */
	if (io->readctx.state == IO_IO_STARTED)
		io_io_read_stop(io);

	io_mon_remove_source(io->mon,
			io_src_tmr_get_source(&io->writectx.timer));
	io_mon_remove_source(io->mon, &io->writectx.src);
	io_mon_remove_source(io->mon, &io->src);

	rs_rb_clean(&io->readctx.rb);

	/* destroy write */
	io_io_write_abort(io);

	io_src_clean(&io->writectx.src);
	io_src_clean(&io->src);
	io_src_tmr_clean(&io->writectx.timer);

	free(io->name);
	memset(io, 0, sizeof(*io));

	return 0;
}

int io_io_read_start(struct io_io *io, io_io_read_cb_t cb, void *data,
		int clear)
{
	int ret;

	if (!io || !cb)
		return -EINVAL;

	if (io->readctx.state != IO_IO_STOPPED)
		return -EBUSY;

	/*
	 * activate out source, useless at init, but needed after calls to
	 * io_io_read_stop()
	 */
	ret = io_mon_activate_in_source(io->mon, &io->src, 1);
	if (ret < 0)
		return ret;

	/* set callback info */
	io->readctx.cb = cb;
	io->readctx.data = data;

	/* clear read buffer if needed */
	if (clear)
		rs_rb_empty(&io->readctx.rb);

	/* update read state */
	io->readctx.state = IO_IO_STARTED;
	return 0;
}

int io_io_log_rx(struct io_io *io, void (*log_rx)(const char *))
{
	if (NULL == io)
		return -EINVAL;

	io->log_rx = log_rx;

	return 0;
}

int io_io_log_tx(struct io_io *io, void (*log_tx)(const char *))
{
	if (NULL == io)
		return -EINVAL;

	io->log_tx = log_tx;

	return 0;
}

int io_io_read_stop(struct io_io *io)
{
	if (NULL == io)
		return -EINVAL;

	if (io->readctx.state != IO_IO_STARTED)
		return -EBUSY;

	/* reset callback info */
	io->readctx.cb = NULL;
	io->readctx.data = NULL;

	/* update state */
	io->readctx.state = IO_IO_STOPPED;

	return io_mon_activate_in_source(io->mon, &io->src, 0);
}

int io_io_is_read_started(struct io_io *io)
{
	return NULL != io ? io->readctx.state == IO_IO_STARTED : 0;
}

int io_io_has_read_error(struct io_io *io)
{
	return NULL != io ? io->readctx.state == IO_IO_ERROR : 0;
}

int io_io_write_add(struct io_io *io, struct io_io_write_buffer *buffer)
{
	int ret = 0;
	struct io_io_write_ctx *ctx;

	if (NULL == io || NULL == buffer)
		return -EINVAL;
	if (!buffer->address || buffer->length == 0)
		return -EINVAL;

	ctx = &io->writectx;

	if (!buffer->cb) {
		buffer->cb = &default_write_cb;
		buffer->data = io;
	}

	rs_dll_enqueue(&ctx->buffers, &buffer->node);
	if (ctx->current == NULL)
		process_next_write(io);

	return ret;
}

int io_io_write_abort(struct io_io *io)
{
	struct io_io_write_ctx *ctx;
	struct io_io_write_buffer *buffer = NULL;
	struct rs_node *node;

	if (NULL == io)
		return -EINVAL;
	ctx = &io->writectx;
	buffer = ctx->current;

	/* TODO: how to be safe on io destroy call in write cb here ? */
	if (buffer) {
		(*buffer->cb)(buffer, IO_IO_WRITE_ABORTED);
		ctx->current = NULL;
		ctx->nbwritten = 0;
	}

	while ((node = rs_dll_pop(&ctx->buffers))) {
		buffer = ut_container_of(node, struct io_io_write_buffer, node);
		(*buffer->cb)(buffer, IO_IO_WRITE_ABORTED);
	}

	process_next_write(io);

	return 0;
}

int io_io_write_buffer_init(struct io_io_write_buffer *buf, io_io_write_cb_t cb,
		void *data, size_t length, const void *address)
{
	int ret;

	if (NULL == buf || 0 == length || NULL == address)
		return -EINVAL;

	ret = io_io_write_buffer_clean(buf);
	if (0 != ret)
		return ret;

	buf->cb = cb;
	buf->data = data;
	buf->length = length;
	buf->address = address;

	return 0;
}

int io_io_write_buffer_clean(struct io_io_write_buffer *buf)
{
	if (NULL == buf)
		return -EINVAL;

	memset(buf, 0, sizeof(*buf));

	return 0;
}
