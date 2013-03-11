/**
 * @file io_src.c
 * @date 16 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <sys/stat.h>

#include <unistd.h>

#include <errno.h>
#include <string.h>

#include <io_src.h>

/**
 * Checks if the arguments of io_src_init are valid
 * @param src Source to initialize. Can't be NULL
 * @param fd File descriptor of the source
 * @param type Type, in, out or both
 * @param cb Callback notified when fd is ready for I/O
 * @return 1 if one argument at least is invalid, 0 otherwise
 */
static int init_args_are_invalid(struct io_src *src, int fd,
		enum io_src_event type, io_src_cb_t *cb)
{
	return NULL == src || -1 == fd || (type & ~IO_DUPLEX) ||
			!(type & IO_DUPLEX) || NULL == cb;
}

int io_src_init(struct io_src *src, int fd, enum io_src_event type,
		io_src_cb_t *cb)
{
	struct stat st;
	int ret;
	if (init_args_are_invalid(src, fd, type, cb))
		return -EINVAL;

	/*
	 * regular files are not pollable with epoll and in general and when
	 * they are (e.g. with select), they are never reported as WOULDBLOCK,
	 * so sources can't be regular file file descriptor. Rather use a source
	 * wrapping aio, or a thread
	 */
	ret = fstat(fd, &st);
	if (-1 == ret)
		return -errno;

	if (S_ISREG(st.st_mode))
		return -EBADF;

	memset(src, 0, sizeof(*src));

	src->fd = fd;
	src->type = type;
	src->cb = cb;

	return 0;
}

void io_src_clean(struct io_src *src)
{
	if (NULL == src)
		return;

	if (-1 != src->fd)
		close(src->fd);
	memset(src, 0, sizeof(*src));

	src->fd = -1;
}
