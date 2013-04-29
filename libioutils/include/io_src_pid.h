/**
 * @file io_src_pid.h
 * @date 29 apr. 2013
 * @author nicolas.carrier@parrot.com
 * @brief Source for watching for a process' death
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#ifndef IO_SRC_PID_H_
#define IO_SRC_PID_H_
#include "io_src.h"

/**
 * @typedef io_src_pid
 * @brief Pid source type
 */
struct io_src_pid;

/**
 * @typedef io_pid_cb_t
 * @brief Called when the monitored process has died
 * @param pid Signal source
 */
typedef void (io_pid_cb_t)(struct io_src_pid *pid);

/**
 * @typedef io_src_pid
 * @brief Pid source type
 */
struct io_src_pid {
	/** inner monitor source */
	struct io_src src;
	/** pid of the process being monitored */
	pid_t pid;
	/** status of the process when it dies. Same semantic as waitpid's */
	int status;
	/** user callback, notified when one of the registered signals occur */
	io_pid_cb_t *cb;
};

/**
 * Initializes a pid source.
 * @param pid_src Pid source to initialize
 * @param pid Pid of the process to monitor
 * @param cb User calback, notified when the process dies
 * @return errno compatible negative value
 */
int io_src_pid_init(struct io_src_pid *pid_src, pid_t pid, io_pid_cb_t *cb);

/**
 * Returns the underlying io_src of the pid source
 * @param pid Pid source
 * @return io_src of the pid source
 */
static inline struct io_src *io_src_pid_get_source(struct io_src_pid *pid)
{
	return NULL == pid ? NULL : &(pid->src);
}

/**
 * Cleans up a pid source, by properly closing fd, zeroing fields etc...
 * @param pid Pid source
 */
void io_src_pid_clean(struct io_src_pid *pid);

#endif /* IO_SRC_PID_H_ */
