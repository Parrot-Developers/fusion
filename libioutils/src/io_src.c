/**
 * @file io_src.c
 * @date 16 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <errno.h>
#include <string.h>

#include <io_src.h>

int io_src_init(struct io_src *src, int fd, enum io_src_event type,
	       io_callback_t *cb, io_src_cleanup_t *cleanup)
{
	if (NULL == src || -1 == fd || NULL == cb)
		return -EINVAL;

	/*
	 * TODO check with fstat and the file type (S_ISREG etc...) that the
	 * file des is pollable with epoll or return -EPERM as add source will
	 * do later or add an abstraction with aoi or a read / write thread ?
	 */

	memset(src, 0, sizeof(*src));

	src->fd = fd;
	src->type = type;
	src->callback = cb;
	src->cleanup = cleanup;

	return 0;
}

void io_src_cleanup(struct io_src *src)
{
	memset(src, 0, sizeof(*src));
	src->fd = -1;
}
