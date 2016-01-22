/**
 * @file io_src_thread.h
 * @date 15 jan. 2016
 * @author nicolas.carrier@parrot.com
 * @brief Source for monitoring a thread in an event loop
 *
 * Copyright (C) 2016 Parrot S.A.
 */
#ifndef IO_SRC_THREAD_H_
#define IO_SRC_THREAD_H_
#include <pthread.h>

#include <stdbool.h>

#include <io_src.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct io_src_thread
 * @brief thread source type
 */
struct io_src_thread;

/**
 * @typedef io_src_thread_start_routine
 * @brief main function of the thread, whose termination will be notified by the
 * termination callback
 * @param thread_src source representing the thread
 * @return this value will be passed as the ret argument of the termination
 * callback, when the thread terminates
 */
typedef int (io_src_thread_start_routine)(struct io_src_thread *thread_src);

/**
 * @typedef io_src_thread_termination_cb
 * @brief callback called when a thread has terminated
 * @param thread_src source representing the thread
 * @param ret return value of the thread's start routine
 */
typedef void (io_src_thread_termination_cb)(struct io_src_thread *thread_src,
		int ret);

/**
 * @struct io_src_thread
 * @brief thread source type
 */
struct io_src_thread {
	/** inner monitor source */
	struct io_src src;
	/** the very thread */
	pthread_t thread;
	/** true iif the pthread_create call has succeeded */
	bool thread_initialized;
	/** communication pipe, notifying of the thread's death */
	int pipefd[2];
	/** main function of the thread */
	io_src_thread_start_routine *start_routine;
	/** callback called when the thread terminates */
	io_src_thread_termination_cb *termination_cb;
	/** storage for the start routine's return value */
	int ret;
};

/**
 * Initializes a thread source.
 * @param thread_src Thread source to initialize
 * @return errno compatible value
 */
int io_src_thread_init(struct io_src_thread *thread_src);

/**
 * Starts the thread, by calling it's start_routine
 * @param thread_src thread source representing the thread to start
 * @param attr optional pthread attributes to configure the thread with
 * @param start_routine main function of the thread
 * @param termination_cb callback called when the thread terminates
 * @return errno-compatible negative value on error
 */
int io_src_thread_start(struct io_src_thread *thread_src,
		const pthread_attr_t *attr,
		io_src_thread_start_routine *start_routine,
		io_src_thread_termination_cb *termination_cb);

/**
 * Retrieves the underlying io_src of the thread
 * @param thread_src I/O source representing the thread
 * @return io_src of the thread
 */
static inline struct io_src *io_src_thread_get_source(
		struct io_src_thread *thread_src)
{
	return NULL == thread_src ? NULL : &thread_src->src;
}

/**
 * Cleans the io_src_thread, if the thread hasn't terminated yet, it is
 * canceled.
 * @param thread_src Thread source to initialize
 */
void io_src_thread_clean(struct io_src_thread *thread_src);

#ifdef __cplusplus
}
#endif

#endif /* IO_SRC_THREAD_H_ */
