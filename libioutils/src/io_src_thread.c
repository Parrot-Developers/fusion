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
	int ret;
	struct io_src_thread *thread_src = arg;

	thread_src->ret = thread_src->start_routine(thread_src);

	ret = io_src_evt_notify(&thread_src->evt, 1);
	/* ret is discarded */

	return &thread_src->ret;
}

/**
 * Callback notified via the event source, of when the thread has terminated
 * @param evt event fd source
 * @param value discarded
 */
static void thread_src_cb(struct io_src_evt *evt, uint64_t value)
{
	union {
		void *v;
		int *i;
	} ret;
	struct io_src_thread *thread_src = ut_container_of(evt,
			struct io_src_thread, evt);

	pthread_join(thread_src->thread, &ret.v);
	thread_src->termination_cb(thread_src, *ret.i);
}

int io_src_thread_init(struct io_src_thread *thread_src)
{
	int ret;

	if (thread_src == NULL)
		return -EINVAL;

	memset(thread_src, 0, sizeof(*thread_src));

	return io_src_evt_init(&thread_src->evt, thread_src_cb, false, 0);
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
	if (thread_src->thread_initialized) {
		pthread_cancel(thread_src->thread);
		pthread_join(thread_src->thread, NULL);
	}
	thread_src->thread_initialized = false;
	io_src_evt_clean(&thread_src->evt);
	thread_src->ret = 0;
	thread_src->start_routine = NULL;
	thread_src->termination_cb = NULL;
}
