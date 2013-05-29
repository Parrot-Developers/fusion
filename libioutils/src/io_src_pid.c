/**
 * @file io_src_pid.c
 * @date 29 apr. 2013
 * @author nicolas.carrier@parrot.com
 * @brief Source for watching for a process' death
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <assert.h>
#include <errno.h>

#include <pidwatch.h>

#include "io_mon.h"
#include "io_src_pid.h"

#ifndef O_CLOEXEC
#ifdef __arm__
	/* value taken from linux kernel header
	 * include/asm-generic/fcntl.h */
	#define O_CLOEXEC 02000000
#else
	#error O_CLOEXEC not defined !
#endif
#endif

#ifndef SOCK_CLOEXEC
/**
 * @def SOCK_CLOEXEC
 * @brief Set the flag O_CLOEXEC at socket's creation
 */
#define SOCK_CLOEXEC O_CLOEXEC
#endif

#ifndef SOCK_NONBLOCK
/**
 * @def SOCK_NONBLOCK
 * @brief Set the flag O_NONBLOCK at socket's creation
 */
#define SOCK_NONBLOCK O_NONBLOCK
#endif

/**
 * @def to_src
 * @brief Convert a source to it's pid source container
 */
#define to_src_pid(p) container_of(p, struct io_src_pid, src)

/**
 * Source callback, reads the pidwatch event and notifies the client
 * @param src Underlying monitor source of the pid source
 */
static void pid_cb(struct io_src *src)
{
	pid_t pid_ret;
	struct io_src_pid *pid_src = to_src_pid(src);

	/* TODO treat I/O THEN errors */
	if (io_src_has_error(src->events))
		return;

	pid_ret = pidwatch_wait(src->fd, &pid_src->status);
	assert(pid_ret == pid_src->pid);
	if (-1 == pid_ret)
		return;

	pid_src->cb(pid_src, pid_src->pid, pid_src->status);
}

int io_src_pid_init(struct io_src_pid *pid_src, io_pid_cb_t *cb)
{
	int pidfd;

	if (NULL == pid_src || NULL == cb)
		return -EINVAL;

	pidfd = pidwatch_create(SOCK_CLOEXEC | SOCK_NONBLOCK);
	if (0 > pidfd)
		goto out;

	pid_src->pid = IO_SRC_PID_DISABLE;
	pid_src->status = 0;
	pid_src->cb = cb;

	/* can fail only on parameters */
	return io_src_init(&(pid_src->src), pidfd, IO_IN, pid_cb);
out:
	io_src_pid_clean(pid_src);

	return -errno;
}

int io_src_pid_set_pid(struct io_src_pid *pid_src, pid_t pid)
{
	if (NULL == pid_src)
		return -EINVAL;

	pid_src->pid = pid;

	return pidwatch_set_pid(pid_src->src.fd, pid);
}

void io_src_pid_clean(struct io_src_pid *pid)
{
	if (NULL == pid)
		return;

	pid->pid = 0;
	pid->status = 0;
	pid->cb = NULL;

	io_src_clean(&(pid->src));
}
