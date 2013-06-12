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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <unistd.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "io_mon.h"
#include "io_src_sep.h"
#include "io_utils.h"

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
 * @brief computes the maximum number of bytes which can be read in order to
 * build a chunk of acceptable size
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
 * @return 0
 */
static int notify_user(struct io_src_sep *sep, unsigned len)
{
	char *chunk = buf_read_start(sep);

	sep->cb(sep, chunk, len);
	sep->from += len;
	if (sep->from >= IO_SRC_SEP_SIZE) {
		memmove(sep->buf, buf_read_start(sep), already_read(sep));
		sep->up_to -= sep->from;
		sep->from = 0;
	}

	return 0;
}

/**
 * Send what we have in a first client notification, then notify with a zero-len
 * chunk
 * @param sep Separator source
 * @return return of notify_user
 */
static int end_of_file(struct io_src_sep *sep)
{
	int ret = notify_user(sep, already_read(sep));

	if (0 > ret)
		return ret;
	if (0 < ret)
		fprintf(stderr, "sep->cb: %s", strerror(-ret));

	return notify_user(sep, 0);
}

/**
 * Checks if the separator byt(s) is (are) found at a given position in a string
 * @param sep Separator source
 * @param c Current position in the string
 * @return
 */
static int separator_found(struct io_src_sep *sep, char *c)
{
	return *c == sep->sep1 && (!sep->two_bytes || *(c + 1) == sep->sep2);
}

/**
 * @def chunk_len
 * @param sep Separator source
 * @param c Current position in the string
 * @return Length of the last chunk found, including separator(s)
 */
#define chunk_len(sep, c) (1 + (sep)->two_bytes + (c) - buf_read_start((sep)))

/**
 * Searches each separator occurrences and notifies user of each corresponding
 * chunk
 * @param sep Separator source
 * @return First critical error code from user callback
 */
static int parse(struct io_src_sep *sep)
{
	int ret;
	char *cur = NULL;

	for (cur = buf_read_start(sep);
			cur + sep->two_bytes < buf_write_start(sep);
			cur++)
		if (separator_found(sep, cur)) {
			ret = notify_user(sep, chunk_len(sep, cur));
			if (0 > ret)
				return ret;
			if (0 < ret)
				fprintf(stderr, "sep->cb: %s", strerror(-ret));
			cur += sep->two_bytes;
		}

	return 0;
}

/**
 * Consumes all that can be consumes from the data we have read so far
 * @param sep Separator source
 * @return First critical error code from user callback
 */
static int consume(struct io_src_sep *sep)
{
	int ret;

	/* search for a separator and notify for each occurrence */
	ret = parse(sep);
	if (0 > ret)
		return ret;

	/* buffer is full : notify */
	if (already_read(sep) == IO_SRC_SEP_SIZE)
		return notify_user(sep, already_read(sep));

	return 0;
}

/**
 * Source callback, reads the signal and notifies the client
 * @param src Underlying monitor source of the signal source
 */
static void sep_cb(struct io_src *src)
{
	ssize_t sret;
	struct io_src_sep *sep = to_src_sep(src);

	/* TODO treat I/O THEN errors */
	if (io_src_has_error(src->events))
		return;

	/* get some data */
	sret = io_read(src->fd, buf_write_start(sep), to_read(sep));
	if (-1 == sret)
		return;
	if (0 == sret) {
		end_of_file(sep);
		return;
	}

	/* something has been read */
	sep->up_to += sret;

	consume(sep);
}

int io_src_sep_init(struct io_src_sep *sep_src, int fd, io_src_sep_cb_t *cb,
		int sep1, int sep2)
{
	if (NULL == sep_src || NULL == cb || -1 == fd)
		return -EINVAL;

	memset(sep_src, 0, sizeof(*sep_src));

	sep_src->cb = cb;
	sep_src->sep1 = sep1;
	sep_src->sep2 = sep2;
	sep_src->two_bytes = IO_SRC_SEP_NO_SEP2 != sep2;

	/* can fail only on parameters */
	return io_src_init(&(sep_src->src), fd, IO_IN, sep_cb);
}

void io_src_sep_clean(struct io_src_sep *sep)
{
	if (NULL == sep)
		return;

	io_src_clean(&(sep->src));
}
