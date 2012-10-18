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

int io_src_init(io_src_t *src, int fd, io_src_event_t type, io_callback_t *cb)
{
	if (NULL == src || -1 == fd || NULL == cb)
		return -EINVAL;

	memset(src, 0, sizeof(*src));

	src->fd = fd;
	src->type = type;
	src->callback = cb;

	return 0;
}

