/**
 * @file io_src.c
 * @date 16 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <sys/stat.h>

#include <errno.h>
#include <string.h>

#include <io_src.h>

int io_src_init(struct io_src *src, int fd, enum io_src_event type,
		io_src_cb_t *cb, io_src_clean_t *clean)
{
	struct stat st;
	int ret;
	if (NULL == src || -1 == fd || NULL == cb)
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
	src->clean = clean;

	return 0;
}

void io_src_clean(struct io_src *src)
{
	memset(src, 0, sizeof(*src));
	src->fd = -1;
}
