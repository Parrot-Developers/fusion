/**
 * @file io_src_thread.c
 * @date 15 jan. 2016
 * @author nicolas.carrier@parrot.com
 * @brief Source for monitoring a thread in an event loop
 *
 * Copyright (C) 2016 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <string.h>

#include <io_src_thread.h>

#include <ut_file.h>

/**
 * Main function of the thead, executes the user-provided start routine, then
 * notifies via the thread's file descriptor
 * @param arg thread source
 */
static void *thread_src_start_routine(void *arg)
{
	char *byte = 0;
	ssize_t sret;
	struct io_src_thread *thread_src = arg;

	thread_src->ret = thread_src->start_routine(thread_src);

	sret = write(thread_src->pipefd[1], &byte, sizeof(byte));
	/* sret is discarded */

	return &thread_src->ret;
}

static void thread_src_cb(struct io_src *src)
{
	union {
		void *v;
		int *i;
	} ret;
	ssize_t sret;
	char byte;
	struct io_src_thread *thread_src = ut_container_of(src,
			struct io_src_thread, src);

	sret = read(thread_src->pipefd[0], &byte, sizeof(byte));
	if (sret == -1)
		byte = -errno;

	pthread_join(thread_src->thread, &ret.v);
	thread_src->termination_cb(thread_src, *ret.i);
}

int io_src_thread_init(struct io_src_thread *thread_src)
{
	int ret;

	if (thread_src == NULL)
		return -EINVAL;

	memset(thread_src, 0, sizeof(*thread_src));
	ret = pipe2(thread_src->pipefd, O_CLOEXEC | O_NONBLOCK);
	if (ret == -1)
		return -errno;

	return io_src_init(&thread_src->src, thread_src->pipefd[0], IO_IN,
			thread_src_cb);
}

int io_src_thread_start(struct io_src_thread *thread_src,
		const pthread_attr_t *attr,
		io_src_thread_start_routine *start_routine,
		io_src_thread_termination_cb *termination_cb)
{
	int ret;

	if (thread_src == NULL || start_routine == NULL ||
			termination_cb == NULL)
		return -EINVAL;

	thread_src->start_routine = start_routine;
	thread_src->termination_cb = termination_cb;
	ret = pthread_create(&thread_src->thread, attr,
			thread_src_start_routine, thread_src);
	if (ret != 0)
		return -ret;
	thread_src->thread_initialized = true;

	return 0;
}

void io_src_thread_clean(struct io_src_thread *thread_src)
{
	io_src_clean(&thread_src->src);
	if (thread_src->thread_initialized) {
		pthread_cancel(thread_src->thread);
		pthread_join(thread_src->thread, NULL);
	}
	thread_src->thread_initialized = false;
	ut_file_fd_close(thread_src->pipefd + 0);
	ut_file_fd_close(thread_src->pipefd + 1);
	thread_src->ret = 0;
	thread_src->start_routine = NULL;
	thread_src->termination_cb = NULL;
}
