/**
 * @file io_platform.c
 * @date Nov 26, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Compatibility module, define constants and function lacking on some
 * ancient tool chains
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#include <unistd.h>      /* syscall */
#include <sys/syscall.h> /* syscall */
#include <sys/epoll.h>   /* epoll_create1 */

#include "io_platform.h"

int io_epoll_create1(int flags)
{
#ifdef __arm__
	return syscall(357, flags);
#else
	return epoll_create1(flags);
#endif
}
