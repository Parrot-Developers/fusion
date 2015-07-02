/**
 * @file io_mon.c
 * @date 16 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#include <signal.h>
#include <unistd.h>

#include <error.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include <argz.h>

#include <ut_string.h>
#include <ut_file.h>

#include "io_process.h"
#include "io_utils.h"

/**
 * Builds the command line for the process from the list of arguments supplied
 * @param process Process context
 * @param args NULL-terminated list of strings for building the command-line
 */
static int command_line_new(struct io_process *process, va_list args)
{
	int ret;
	char *arg;
	int nargs = 0;

	if (process == NULL)
		return -EINVAL;

	do {
		arg = va_arg(args, char *);
		if (arg == NULL)
			break;
		ret = argz_add(&process->command_line,
				&process->command_line_len, arg);
		if (ret != 0)
			return ret;
	} while (true);

	return 0;
}

/**
 * Generic function to setup an src_sep source, be it for stdout or stderr.
 * @param process Process context
 * @param src Separator source to setup
 * @param pipefd Pipe corresponding to the separator source
 * @param cb Callback called on each chunk of data, retrieved before a separator
 * @param sep1 First separator between the chunks of data (only one byte)
 * @param sep2 second separator between the chunks of data (only one byte), pass
 * IO_SRC_SEP_NO_SEP2 to use only one separator
 * @return Negative errno compatible value on error, 0 otherwise
 */
static int set_sep_src(struct io_process *process, struct io_src_sep *sep_src,
		int pipefd[2], io_src_sep_cb_t *cb, int sep1, int sep2)
{
	int ret;
	struct io_src *src = NULL;

	if (process == NULL || cb == NULL || sep_src == NULL ||
			pipefd == NULL ||
			process->state != IO_PROCESS_INITIALIZED)
		return -EINVAL;

	src = io_src_sep_get_source(sep_src);
	if (io_src_get_fd(src) != -1)
		return -EINVAL;

	ret = pipe(pipefd);
	if (ret < 0)
		return -errno;

	ret = io_src_sep_init(sep_src, pipefd[0], cb, sep1, sep2);
	if (ret < 0)
		return ret;

	return io_mon_add_source(&process->mon, src);
}

/**
 * Generic function to setup a source, be it for stdin, stdout or stderr.
 * @param process Process context
 * @param src Source to setup
 * @param cb Callback notified when fd is ready for I/O
 * @param pipefd Pipe corresponding to the source
 * @param fd_index 0 if we want to monitor the input end of the pipe, for stdout
 * or stderr, 1 if we want to monitor the output end for writing to the
 * program's stdin
 * @return Negative errno compatible value on error, 0 otherwise
 */
static int set_src(struct io_process *process, struct io_src *src,
		io_src_cb_t *cb, int pipefd[2], int fd_index)
{
	int ret;
	enum io_src_event type[2] = {IO_IN, IO_OUT};

	if (process == NULL || cb == NULL || src == NULL || pipefd == NULL ||
			process->state != IO_PROCESS_INITIALIZED ||
			(fd_index != 0 && fd_index != 1))
		return -EINVAL;
	if (io_src_get_fd(src) != -1)
		return -EALREADY;

	ret = pipe(pipefd);
	if (ret < 0)
		return -errno;

	ret = io_src_init(src, pipefd[fd_index], type[fd_index], cb);
	if (ret < 0)
		return ret;

	return io_mon_add_source(&process->mon, src);
}

/**
 * Cleans up process context, once it's corresponding process is dead
 * @param process Process context
 */
static void io_process_clean(struct io_process *process)
{
	if (process == NULL)
		return;

	ut_string_free(&process->command_line);

	io_mon_remove_source(&process->mon,
			io_src_pid_get_source(&process->pid_src));
	io_src_pid_clean(&process->pid_src);
	io_mon_remove_source(&process->mon, &process->stdin_src);
	io_src_clean(&process->stdin_src);
	ut_file_fd_close(process->stdin_pipe + 0);
	ut_file_fd_close(process->stdin_pipe + 1);
	io_mon_remove_source(&process->mon,
			io_src_sep_get_source(&process->stdout_src));
	io_src_sep_clean(&process->stdout_src);
	ut_file_fd_close(process->stdout_pipe + 0);
	ut_file_fd_close(process->stdout_pipe + 1);
	io_mon_remove_source(&process->mon,
			io_src_sep_get_source(&process->stderr_src));
	io_src_sep_clean(&process->stderr_src);
	ut_file_fd_close(process->stderr_pipe + 0);
	ut_file_fd_close(process->stderr_pipe + 1);
	io_mon_remove_source(&process->mon,
			io_src_tmr_get_source(&process->timeout_src));
	io_src_tmr_clean(&process->timeout_src);
	io_mon_clean(&process->mon);

	if (process->copy)
		free(process->rw);

	memset(process, 0, sizeof(*process));
}

/**
 * Wrapper around the user supplied termination cb, which cleans the process
 * structure after notifying the client
 * @param pid_src Signal source
 * @param pid Pid of the process which has just died
 * @param status Status of the process, same as that of waitpid(2)
 */
static void termination_cb_wrapper(struct io_src_pid *pid_src, pid_t pid,
		int status)
{
	struct io_process *process = ut_container_of(pid_src, struct io_process,
			pid_src);

	process->state = IO_PROCESS_DEAD;
	process->termination_cb(pid_src, pid, status);
}

/**
 * Timer source which kills the program when it expires
 * @param tmr Timer source
 * @param nbexpired Number of expirations of the timer
 */
static void timeout_kill_cb(struct io_src_tmr *tmr, uint64_t *nmexpired)
{
	struct io_process *process = ut_container_of(tmr, struct io_process,
			timeout_src);

	io_process_signal(process, SIGKILL);
}

/**
 * Function responsible of feeding data from the user's registered buffer,
 * to the process' standard input
 * @param src Standard input source of the process
 */
static void stdin_buffer_cb(struct io_src *src)
{
	ssize_t sret;
	struct io_process *process = ut_container_of(src, struct io_process,
			stdin_src);
	const char *buf;

	buf = process->copy ? process->rw : process->ro;

	sret = io_write(src->fd, buf, process->len);
	/* write error, do nothing, the process is likely to crash soon */
	if (sret < 0)
		return;

	if (process->copy)
		process->rw += sret;
	else
		process->ro += sret;
	process->len -= sret;


	if (process->len == 0) {
		io_mon_remove_source(&process->mon, src);
		ut_file_fd_close(process->stdin_pipe + 1);
		io_src_clean(src);
	}
}

/**
 * Function responsible of execv-ing the process and duplicate the right file
 * descriptors to the process's standard files
 * @note the process is configured to receive a SIGKILL when it's parent dies
 * @param process Process to exec
 */
__attribute__ ((noreturn))
static void in_child(struct io_process *process)
{
	int i;
	int ret;
	size_t argc;
	char **argv;

	if (process->stdin_pipe[0] != -1) {
		ret = dup2(process->stdin_pipe[0], STDIN_FILENO);
		if (ret < 0)
			error(EXIT_FAILURE, errno, "dup stdin");
	}
	if (process->stdout_pipe[1] != -1) {
		ret = dup2(process->stdout_pipe[1], STDOUT_FILENO);
		if (ret < 0)
			error(EXIT_FAILURE, errno, "dup stdout");
	}
	if (process->stderr_pipe[1] != -1) {
		ret = dup2(process->stderr_pipe[1], STDERR_FILENO);
		if (ret < 0)
			error(EXIT_FAILURE, errno, "dup stderr");
	}
	/* prepare the command-line */
	argc = argz_count(process->command_line, process->command_line_len);
	argv = calloc(argc + 1, sizeof(*argv));
	if (argv == NULL) {
		ULOGE("calloc: %m");
		_exit(EXIT_FAILURE);
	}
	argz_extract(process->command_line, process->command_line_len, argv);
	/* from here, log will be available to the parent if redirect enabled */
	ret = prctl(PR_SET_PDEATHSIG, SIGKILL);
	if (ret < 0)
		error(EXIT_FAILURE, errno, "prctl");
	for (i = sysconf(_SC_OPEN_MAX) - 1; i > 2; i--)
		close(i);
	ret = execv(argv[0], argv);
	if (ret < 0)
		error(EXIT_FAILURE, errno, "execve");

	_exit(EXIT_FAILURE);
}

/**
 * Callback used by the external source to process the process' events
 * @param src Global source for the whole process
 */
static void src_cb(struct io_src *src)
{
	struct io_process *process = ut_container_of(src, struct io_process,
			src);

	io_process_process_events(process);
}

int io_process_init(struct io_process *process, io_pid_cb_t termination_cb, ...)
{
	int ret;
	va_list args;

	va_start(args, termination_cb);
	ret = io_process_vinit(process, termination_cb, args);
	va_end(args);

	return ret;
}

int io_process_vinit(struct io_process *process, io_pid_cb_t termination_cb,
		va_list args)
{
	int ret;

	if (process == NULL)
		return -EINVAL;
	memset(process, 0, sizeof(*process));

	if (termination_cb == NULL)
		return -EINVAL;

	process->pid_src.src.fd = -1;
	process->stdin_src.fd = -1;
	process->stdout_src.src.fd = -1;
	process->stderr_src.src.fd = -1;
	process->timeout_src.src.fd = -1;
	process->stdin_pipe[0] = process->stdin_pipe[1] = -1;
	process->stdout_pipe[0] = process->stdout_pipe[1] = -1;
	process->stderr_pipe[0] = process->stderr_pipe[1] = -1;
	process->termination_cb = termination_cb;

	ret = command_line_new(process, args);
	if (ret < 0)
		goto err;

	process->state = IO_PROCESS_INITIALIZED;

	ret = io_mon_init(&process->mon);
	if (ret < 0)
		goto err;

	ret = io_src_init(&process->src, io_mon_get_fd(&process->mon), IO_IN,
			src_cb);
	if (ret < 0)
		return ret;

	ret = io_src_pid_init(&process->pid_src, termination_cb_wrapper);
	if (ret < 0)
		goto err;

	return io_mon_add_source(&process->mon,
			io_src_pid_get_source(&process->pid_src));
err:
	io_process_clean(process);

	return ret;
}

int io_process_set_input_buffer(struct io_process *process, const char *buffer,
		size_t len, bool copy)
{
	int ret;

	if (process == NULL || buffer == NULL || len <= 0 ||
			io_src_get_fd(&process->stdin_src) != -1)
		return -EINVAL;

	ret = pipe(process->stdin_pipe);
	if (ret < 0)
		return -errno;

	ret = io_src_init(&process->stdin_src, process->stdin_pipe[1], IO_OUT,
			stdin_buffer_cb);
	if (ret < 0)
		goto err;
	process->len = len;
	if (copy) {
		process->rw = malloc(len * sizeof(char));
		if (process->rw == NULL) {
			ret = -errno;
			goto err;
		}
		memcpy(process->rw, buffer, len);
	} else {
		process->ro = buffer;
	}

	return io_mon_add_source(&process->mon, &process->stdin_src);
err:
	io_src_clean(&process->stdin_src);
	ut_file_fd_close(process->stdin_pipe + 0);
	ut_file_fd_close(process->stdin_pipe + 1);

	return ret;
}

int io_process_set_stdin_src(struct io_process *process, io_src_cb_t *cb)
{
	if ((process->copy && process->rw != NULL) ||
			(!process->copy && process->ro != NULL))
		return -EINVAL;

	return set_src(process, &process->stdin_src, cb, process->stdin_pipe,
			1);
}

int io_process_set_stdout_sep_src(struct io_process *process,
		io_src_sep_cb_t *cb, int sep1, int sep2)
{
	return set_sep_src(process, &process->stdout_src, process->stdout_pipe,
			cb, sep1, sep2);
}

int io_process_set_stdout_src(struct io_process *process, io_src_cb_t *cb)
{
	return set_src(process, io_src_sep_get_source(&process->stdout_src), cb,
			process->stdout_pipe, 0);
}

int io_process_set_stderr_sep_src(struct io_process *process,
		io_src_sep_cb_t *cb, int sep1, int sep2)
{
	return set_sep_src(process, &process->stderr_src, process->stderr_pipe,
			cb, sep1, sep2);
}

int io_process_set_stderr_src(struct io_process *process, io_src_cb_t *cb)
{
	return set_src(process, io_src_sep_get_source(&process->stderr_src), cb,
			process->stderr_pipe, 0);
}

int io_process_set_timeout(struct io_process *process, int timeout, int signum)
{
	int ret;
	bool already_initialized;
	struct io_src *src;

	if (process == NULL || timeout < 0 || signum <= 0 ||
			process->state == IO_PROCESS_DEAD)
		return -EINVAL;
	src = io_src_tmr_get_source(&process->timeout_src);
	already_initialized = io_src_get_fd(src) != -1;

	if (!already_initialized) {
		ret = io_src_tmr_init(&process->timeout_src, timeout_kill_cb);
		if (ret < 0)
			return ret;
	}
	process->signum = signum;
	ret = io_src_tmr_set(&process->timeout_src, timeout);
	if (ret < 0)
		goto err;
	if (!already_initialized) {
		ret = io_mon_add_source(&process->mon, src);
		if (ret < 0)
			goto err;
	}

	return 0;
err:
	if (!already_initialized)
		io_src_tmr_clean(&process->timeout_src);

	return ret;
}

/* sets errno on error */
struct io_src *io_process_get_src(struct io_process *process)
{
	errno = EINVAL;
	if (process == NULL)
		return NULL;

	return &process->src;
}

int io_process_get_fd(struct io_process *process)
{
	if (process == NULL || process->state == IO_PROCESS_DEAD)
		return -EINVAL;

	return io_mon_get_fd(&process->mon);
}

int io_process_process_events(struct io_process *process)
{
	int ret;
	if (process == NULL || process->state == IO_PROCESS_DEAD)
		return -EINVAL;

	ret = io_mon_process_events(&process->mon);
	if (process->state == IO_PROCESS_DEAD)
		io_process_clean(process);

	return ret;
}

int io_process_launch(struct io_process *process)
{
	int ret;
	pid_t pid;

	if (process == NULL || process->state != IO_PROCESS_INITIALIZED)
		return -EINVAL;

	pid = fork();
	if (pid < 0)
		return -errno;
	if (pid == 0)
		in_child(process);
	ut_file_fd_close(process->stdin_pipe + 0);
	ut_file_fd_close(process->stdout_pipe + 1);
	ut_file_fd_close(process->stderr_pipe + 1);

	if (io_mon_is_registered(&process->mon, &process->stdin_src))
		io_mon_activate_out_source(&process->mon, &process->stdin_src,
				true);

	ret = io_src_pid_set_pid(&process->pid_src, pid);
	if (ret < 0)
		goto err;

	process->state = IO_PROCESS_STARTED;
	return 0;
err:
	kill(pid, SIGKILL);
	waitpid(pid, NULL, 0);

	return ret;
}

int io_process_wait(struct io_process *process)
{
	int ret;

	if (process == NULL || process->state != IO_PROCESS_STARTED)
		return -EINVAL;

	do {
		ret = io_mon_poll(&process->mon, -1);
		if (ret < 0)
			break;
	} while (process->state != IO_PROCESS_DEAD);

	if (process->state != IO_PROCESS_DEAD)
		io_process_kill(process);

	return ret;
}

int io_process_launch_and_wait(struct io_process *process)
{
	int ret;

	if (process == NULL)
		return -EINVAL;

	ret = io_process_launch(process);
	if (ret < 0)
		return ret;

	return io_process_wait(process);
}

int io_process_prepare(struct io_process *process,
		struct io_process_parameters *p)
{
	int ret;

	if (process == NULL || p == NULL ||
			process->state != IO_PROCESS_INITIALIZED)
		return -EINVAL;

	if (p->buffer != NULL) {
		ret = io_process_set_input_buffer(process, p->buffer, p->len,
				p->copy);
		if (ret < 0)
			return ret;
	}
	if (p->stdin_cb != NULL) {
		ret = io_process_set_stdin_src(process, p->stdin_cb);
		if (ret < 0)
			return ret;
	}
	if (p->stdout_sep_cb != NULL) {
		ret = io_process_set_stdout_sep_src(process, p->stdout_sep_cb,
				p->out_sep1, p->out_sep2);
		if (ret < 0)
			return ret;
	}
	if (p->stdout_cb != NULL) {
		ret = io_process_set_stdout_src(process, p->stdout_cb);
		if (ret < 0)
			return ret;
	}
	if (p->stderr_sep_cb != NULL) {
		ret = io_process_set_stderr_sep_src(process, p->stderr_sep_cb,
				p->err_sep1, p->err_sep2);
		if (ret < 0)
			return ret;
	}
	if (p->stderr_cb != NULL) {
		ret = io_process_set_stderr_src(process, p->stderr_cb);
		if (ret < 0)
			return ret;
	}
	if (p->timeout > 0) {
		ret = io_process_set_timeout(process, p->timeout, p->signum);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int io_process_init_prepare(struct io_process *process,
		struct io_process_parameters *parameters,
		io_pid_cb_t termination_cb, ...)
{
	int ret;
	va_list args;

	va_start(args, termination_cb);
	ret = io_process_vinit_prepare(process, parameters, termination_cb,
			args);
	va_end(args);

	return ret;
}

int io_process_vinit_prepare(struct io_process *process,
		struct io_process_parameters *p,
		io_pid_cb_t termination_cb, va_list args)
{
	int ret;

	ret = io_process_vinit(process, termination_cb, args);
	if (ret < 0)
		return ret;

	if (p == NULL)
		return 0;

	ret = io_process_prepare(process, p);
	if (ret < 0)
		goto err;

	return 0;
err:
	io_process_clean(process);

	return ret;
}

int io_process_init_prepare_and_launch(struct io_process *process,
		struct io_process_parameters *parameters,
		io_pid_cb_t termination_cb, ...)
{
	int ret;
	va_list args;

	va_start(args, termination_cb);
	ret = io_process_vinit_prepare_and_launch(process, parameters,
			termination_cb, args);
	va_end(args);

	return ret;
}

int io_process_vinit_prepare_and_launch(struct io_process *process,
		struct io_process_parameters *parameters,
		io_pid_cb_t termination_cb, va_list args)
{
	int ret;

	ret = io_process_vinit_prepare(process, parameters,
			termination_cb, args);
	if (ret < 0)
		return ret;

	return io_process_launch(process);
}

int io_process_init_prepare_launch_and_wait(struct io_process *process,
		struct io_process_parameters *parameters,
		io_pid_cb_t termination_cb, ...)
{
	int ret;
	va_list args;

	va_start(args, termination_cb);
	ret = io_process_vinit_prepare_launch_and_wait(process, parameters,
			termination_cb, args);
	va_end(args);

	return ret;
}

int io_process_vinit_prepare_launch_and_wait(struct io_process *process,
		struct io_process_parameters *parameters,
		io_pid_cb_t termination_cb, va_list args)
{
	int ret;

	ret = io_process_vinit_prepare_and_launch(process, parameters,
			termination_cb, args);
	if (ret < 0)
		return ret;

	ret = io_process_wait(process);

	if (process->state != IO_PROCESS_DEAD)
		io_process_kill(process);

	return ret;
}

/* waits for the process thus can block */
int io_process_kill(struct io_process *process)
{
	int ret;

	ret = io_process_signal(process, SIGKILL);
	if (ret < 0)
		return ret;

	return io_process_wait(process);
}

int io_process_signal(struct io_process *process, int signum)
{
	int ret;
	pid_t pid;

	if (process == NULL || signum < 0 ||
			process->state != IO_PROCESS_STARTED)
		return -EINVAL;

	pid = io_src_pid_get_pid(&process->pid_src);
	if (pid <= 0)
		return -ESRCH;

	ret = kill(io_src_pid_get_pid(&process->pid_src), signum);
	if (ret < 0)
		return -errno;

	return 0;
}
