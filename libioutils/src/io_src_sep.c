/**
 * @file io_src_sep.c
 * @date 23 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Separator source, for input. Bytes read are stored in an internal
 * buffer of size IO_SRC_SEP_SIZE and the client is notified each time a given
 * character separator is found, or the buffer is full.
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "io_mon.h"
#include "io_src_sep.h"

/**
 * @def to_src
 * @brief Convert a source to it's signal source container
 */
#define to_src_sep(p) container_of(p, io_src_sep_t, src)

/**
 * @def buf_write_start
 * @brief computes where the data newly read from the fd, must be written to
 * the buffer
 */
#define buf_write_start(sep) ((sep)->buf + (sep)->up_to)

/**
 * @def already_read
 * @brief computes what has already been read in a separator source
 */
#define already_read(sep) ((sep)->up_to - (sep)->from)

/**
 * @def to_read
 * @brief computes what has to be read in order to build a chunk of acceptable
 * size
 */
#define to_read(sep) (IO_SRC_SEP_SIZE - already_read(sep))

/**
 * @define buf_read_start
 * @brief computes where the data which must be sent to the client start from
 */
#define buf_read_start(sep) ((sep)->buf + (sep)->from)

/**
 * Calls back the client with a chunk and updates the internal buffer state to
 * reflect that we have consumed some bytes
 * @param sep
 * @param len
 * @return result of the user callback
 */
static int notify_user(io_src_sep_t *sep, unsigned len)
{
	int ret;
	char *chunk = buf_read_start(sep);

	ret = sep->cb(sep, chunk, len);
	sep->from += len;
	if (sep->from >= IO_SRC_SEP_SIZE) {
		memmove(sep->buf, buf_read_start(sep), to_read(sep));
		sep->up_to -= sep->from;
		sep->from = 0;
	}

	return ret;
}

/**
 * Send what we have in a first client notification, then notify with a zero-len
 * chunk
 * @param sep Separator source
 * @return return of notify_user
 */
static int end_of_file(io_src_sep_t *sep)
{
	int ret = notify_user(sep, already_read(sep));

	if (0 > ret)
		return ret;
	if (0 < ret)
		fprintf(stderr, "sep->cb: %s", strerror(-ret));

	return notify_user(sep, 0);
}

/**
 * Source callback, reads the signal and notifies the client
 * @param src Underlying monitor source of the signal source
 */
static int sep_cb(io_src_t *src)
{
	int ret;
	char *cur = NULL;
	ssize_t sret;
	io_src_sep_t *sep = to_src_sep(src);

	if (io_mon_has_error(src->events))
		return -EIO;

	/* get some data */
	sret = read(src->fd, buf_write_start(sep), to_read(sep));
	if (-1 == sret)
		return -errno;
	if (0 == sret)
		return end_of_file(sep);

	/* something has been read */
	sep->up_to += sret;

	/* search for a separator and notify for each occurrence */
	for (cur = buf_read_start(sep); cur < buf_write_start(sep); cur++)
		if (*cur == sep->sep) {
			ret = notify_user(sep, 1 + cur - buf_read_start(sep));
			if (0 > ret)
				return ret;
			if (0 < ret)
				fprintf(stderr, "sep->cb: %s", strerror(-ret));
		}

	/* buffer is full : notify */
	if (already_read(sep) == IO_SRC_SEP_SIZE)
		return notify_user(sep, already_read(sep));

	return 0;
}

/**
 * Callback called when the source is removed
 * @param src Underlying monitor source of the signal source
 */
static void sep_cleanup(io_src_t *src)
{
	io_src_sep_t *sep;

	if (NULL == src)
		return;
	sep = to_src_sep(src);

	memset(sep, 0, sizeof(*sep));
	src->fd = -1;
}

int io_src_sep_init(io_src_sep_t *sep_src, int fd, io_src_sep_cb_t *cb,
		char sep)
{
	if (NULL == sep_src || NULL == cb || -1 == fd)
		return -EINVAL;

	memset(sep_src, 0, sizeof(*sep_src));

	sep_src->cb = cb;
	sep_src->sep = sep;

	/* can fail only on parameters */
	return io_src_init(&(sep_src->src), fd, IO_IN, sep_cb, sep_cleanup);
}
