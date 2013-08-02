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
#include <sys/wait.h>

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

ssize_t io_write(int fd, void *buf, size_t count)
{
	return TEMP_FAILURE_RETRY(write(fd, buf, count));
}

ssize_t io_recvfrom(int sockfd, void *buf, size_t len, int flags,
		struct sockaddr *src_addr, socklen_t *addrlen)
{
	return TEMP_FAILURE_RETRY(recvfrom(sockfd, buf, len, flags, src_addr,
			addrlen));
}

ssize_t io_sendto(int sockfd, const void *buf, size_t len, int flags,
		const struct sockaddr *dest_addr, socklen_t addrlen)
{
	return TEMP_FAILURE_RETRY(sendto(sockfd, buf, len, flags, dest_addr,
			addrlen));
}

pid_t io_waitpid(pid_t pid, int *status, int options)
{
	return TEMP_FAILURE_RETRY(waitpid(pid, status, options));
}

ssize_t io_send(int fd, const void *buf, size_t n, int flags)
{
	return TEMP_FAILURE_RETRY(send(fd, buf, n, flags));
}

ssize_t io_recv(int fd, void *buf, size_t n, int flags)
{
	return TEMP_FAILURE_RETRY(recv(fd, buf, n, flags));
}

int io_set_non_blocking(int fd)
{
	int flags;
	int ret;

	if (0 > fd)
		return -EINVAL;

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

int io_close(int *fd)
{
	int ret;

	if (NULL == fd || -1 == *fd) {
		errno = EBADF;
		return -1;
	}
	/*
	 * Although 0 is a perfectly valid file descriptor, we forbid closing it
	 * because allowing close(0) can hide some subtle bugs. The aim is to
	 * fail early when these bugs occur.
	 * If one really want to close 0, he must use close explicitly.
	 */
	if (0 == *fd) {
		errno = EBADF;
		return -1;
	}

	ret = close(*fd);
	if (-1 == ret)
		return ret;

	*fd = -1;

	return 0;
}
