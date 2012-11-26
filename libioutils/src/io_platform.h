/**
 * @file io_platform.h
 * @date Nov 26, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Compatibility module, define constants and function lacking on some
 * ancient tool chains
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef IO_PLATFORM_H_
#define IO_PLATFORM_H_

#include <fcntl.h>       /* O_CLOEXEC */

/* Give us a chance to use regular definitions: */
#include <sys/epoll.h>   /* EPOLL_CLOEXEC */

#ifndef O_CLOEXEC
#define O_CLOEXEC       02000000        /* set close_on_exec */
#endif


/* for epoll_create1 */
#ifndef EPOLL_CLOEXEC
/**
 * @def EPOLL_CLOEXEC
 * @brief Set the flag O_CLOEXEC at poll fd's creation
 */
#define EPOLL_CLOEXEC O_CLOEXEC
#endif

/* for signalfd */
#ifndef SFD_CLOEXEC
/**
 * @def SFD_CLOEXEC
 * @brief Set the flag O_CLOEXEC at signal fd's creation
 */
#define SFD_CLOEXEC O_CLOEXEC
#endif

/* for signalfd */
#ifndef SFD_NONBLOCK
/**
 * @def SFD_NONBLOCK
 * @brief Set the flag O_NONBLOCK at signal fd's creation
 */
#define SFD_NONBLOCK O_NONBLOCK
#endif

/**
 * Wrapper around epoll_create1, defining it on toolchains where it is missing
 * @see epoll_create1
 * @param flags can be EPOLL_CLOEXEC
 * @return -1 is returned, with errno set, file descriptor created on success
 */
int io_epoll_create1(int flags);

#endif /* IO_PLATFORM_H_ */
