/**
 * @file io_utils.c
 * @date 17 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <unistd.h>

#include <fcntl.h>
#include <errno.h>

#include "io_utils.h"

ssize_t io_epoll_wait(int epfd, struct epoll_event *events, int maxevents,
		int timeout)
{
	return TEMP_FAILURE_RETRY(epoll_wait(epfd, events, maxevents, timeout));
}

ssize_t io_read(int fd, void *buf, size_t count)
{
	return TEMP_FAILURE_RETRY(read(fd, buf, count));
}

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

