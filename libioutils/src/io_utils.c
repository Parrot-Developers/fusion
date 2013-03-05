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
