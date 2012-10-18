/**
 * @file io_utils.c
 * @date 17 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <fcntl.h>
#include <errno.h>

#include "io_utils.h"

int set_non_blocking(int fd)
{
	int flags;
	int ret;

	flags = fcntl(fd, F_GETFL, 0);
	if (-1 == flags)
		flags = 0;
	if (!(flags & O_NONBLOCK)) {
		ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		if (0 != ret)
			return -errno;
	}

	return 0;
}

