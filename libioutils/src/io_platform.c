/**
 * @file io_platform.c
 * @date Nov 26, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Compatibility module, define constants and function lacking on some
 * ancient tool chains
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <sys/syscall.h> /* syscall */
#include <sys/epoll.h>   /* epoll_create1 */
#include <sys/signalfd.h>

#include <unistd.h>

#include <errno.h>

#include "io_platform.h"

int io_epoll_create1(int flags)
{
#ifdef __arm__
	return syscall(357, flags);
#else
	return epoll_create1(flags);
#endif
}

int io_signalfd(int fd, const sigset_t *mask, int flags)
{
	int ret;
	int new_fd;

	new_fd = ret = signalfd(fd, mask, flags);
	if (0 > ret) {
		/*
		 * error can be related to unsupported flags. try the safe
		 * version first and the racy one on error
		 */
		ret = new_fd = signalfd(fd, mask, 0);
		if (0 > ret)
			return -errno;
		if (SFD_NONBLOCK & flags) {
			ret = fcntl(new_fd, F_SETFD, FD_CLOEXEC);
			if (0 > ret) {
				/* free fd only if allocated by us */
				if (-1 == fd)
					close(new_fd);
				return -errno;
			}
		}
		if (SFD_NONBLOCK & flags) {
			ret = fcntl(new_fd, F_SETFL, O_NONBLOCK);
			if (0 > ret) {
				/* free fd only if allocated by us */
				if (-1 == fd)
					close(new_fd);
				return -errno;
			}
		}
	}

	return new_fd;
}

int io_pipe2(int pipefd[2], int flags)
{
#ifdef __arm__
	/* __NR_pipe2 == 359 on arm */
	return syscall(359, pipefd, flags);
#else
	return pipe2(pipefd, flags);
#endif
}
