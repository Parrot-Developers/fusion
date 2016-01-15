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
#include <io_platform.h>
#include <sys/poll.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Wrapper around read, discarding EINTR errors
 * @see read
 */
ssize_t io_read(int fd, void *buf, size_t count);

/**
 * Wrapper around write, discarding EINTR errors
 * @see write
 */
ssize_t io_write(int fd, const void *buf, size_t count);

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

/**
 * Wrapper around waitpid, discarding EINTR errors
 * @see waitpid
 */
pid_t io_waitpid(pid_t pid, int *status, int options);

/**
 * Wrapper around poll, discarding EINTR errors
 * @see poll
 */
int io_poll(struct pollfd *fds, nfds_t nfds, int timeout);

/**
 * Wrapper around send, discarding EINTR errors
 * @see send
 */
ssize_t io_send(int fd, const void *buf, size_t n, int flags);

/**
 * Wrapper around recv, discarding EINTR errors
 * @see recv
 */
ssize_t io_recv(int fd, void *buf, size_t n, int flags);

/**
 * Sets a file descriptor non-blocking
 * @param fd File descriptor
 * @return errno compatible negative value on error, 0 otherwise
 * @see fcntl
 */
int io_set_non_blocking(int fd);

/**
 * Close a file descriptor and sets it to -1.
 * @param fd pointer to the file descriptor to close
 * @return 0 on success, -1 on error with errno set. On error, fd is left
 * untouched
 * @deprecated use ut_file_fd_close from libutils
 */
__attribute__ ((deprecated("use ut_file_fd_close from libutils")))
int io_close(int *fd);

#ifdef __cplusplus
}
#endif

#endif /* IO_UTILS_H_ */
