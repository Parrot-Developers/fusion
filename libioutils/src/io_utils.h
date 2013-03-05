/**
 * @file io_utils.h
 * @date 17 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef IO_UTILS_H_
#define IO_UTILS_H_
#include <sys/epoll.h>
#include <sys/socket.h>

/**
 * Wrapper around read, discarding EINTR errors
 * @see read
 */
ssize_t io_read(int fd, void *buf, size_t count);

/**
 * Wrapper around epoll_wait, discarding EINTR errors
 * @see epoll_wait
 */
ssize_t io_epoll_wait(int epfd, struct epoll_event *events, int maxevents,
		int timeout);

/**
 * Wrapper around recvfrom, discarding EINTR errors
 * @see recvfrom
 */
ssize_t io_recvfrom(int sockfd, void *buf, size_t len, int flags,
		struct sockaddr *src_addr, socklen_t *addrlen);

/**
 * Wrapper around sendto, discarding EINTR errors
 * @see sendto
 */
ssize_t io_sendto(int sockfd, const void *buf, size_t len, int flags,
		const struct sockaddr *dest_addr, socklen_t addrlen);

#endif /* IO_UTILS_H_ */
