/**
 * @file io_src_pid_test.c
 * @date 29 apr. 2013
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for pid source.
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <unistd.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include <CUnit/Basic.h>

#include <fautes.h>

#include <io_mon.h>
#include <io_src_pid.h>

void dump_args(int argc, char *argv[])
{
	do {
		fprintf(stderr, "%s ", *argv);
	} while (*(++argv));
}

pid_t __attribute__((sentinel)) launch(char *prog, ...)
{
	int ret;
	int child_argc = 0;
	char *child_argv[10] = {NULL};
	char *child_envp[] = {NULL};
	char *arg = (char *)-1;
	va_list args;
	pid_t pid;

	child_argv[child_argc++] = prog;

	va_start(args, prog);
	for (; child_argc < 10 && NULL != arg; child_argc++) {
		arg = va_arg(args, char *);
		child_argv[child_argc] = arg;
	}
	va_end(args);

	if (NULL != arg) {
		errno = E2BIG;
		return -1;
	}
	child_argv[child_argc] = NULL; /* not necessary but clearer */

	pid = fork();
	if (-1 == pid)
		return -1;

	if (0 == pid) {
		/* in child */
		fprintf(stderr, "Executing ");
		dump_args(child_argc, child_argv);
		fprintf(stderr, "\n");
		ret = execvpe(child_argv[0], child_argv, child_envp);
		if (-1 == ret) {
			perror("execve");
			exit(1);
		}
	}

	/* in parent */
	return pid;
}

static void testSRC_PID_INIT(void)
{
	int ret;
	fd_set rfds;
	struct io_mon mon;
	struct io_src_pid pid_src;
	pid_t pid;
	struct timeval timeout;
	bool process_dead = 0;
	void cb(struct io_src_pid *src_pid)
	{
		CU_ASSERT(WIFEXITED(src_pid->status));
		CU_ASSERT_EQUAL(WEXITSTATUS(src_pid->status), 0);
		process_dead = true;
	};

	/* normal use cases */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	pid = launch("sleep", "1", NULL);
	CU_ASSERT_NOT_EQUAL_FATAL(pid, -1);
	ret = io_src_pid_init(&pid_src, pid, cb);
	CU_ASSERT_NOT_EQUAL(ret, -1);
	ret = io_mon_add_source(&mon, &(pid_src.src));
	CU_ASSERT_EQUAL(ret, 0);

	/* set the timer */
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	/* restore the read file descriptor set */
	FD_ZERO(&rfds);
	FD_SET(mon.epollfd, &rfds);

	ret = select(mon.epollfd + 1, &rfds, NULL, NULL, &timeout);
	/* error, not normal */
	CU_ASSERT_NOT_EQUAL(ret, -1);
	if (-1 == ret)
		goto out;

	/* timeout, not normal */
	CU_ASSERT_NOT_EQUAL(ret, 0);
	if (0 == ret)
		goto out;

	ret = io_mon_process_events(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	if (0 != ret)
		goto out;

	CU_ASSERT(process_dead)

out :
	/* cleanup */
	io_mon_clean(&mon);
	io_src_pid_clean(&pid_src);

	/* error use cases */
	ret = io_src_pid_init(0, pid, cb);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_pid_init(&pid_src, -1, cb);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_pid_init(&pid_src, pid, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testSRC_PID_GET_SOURCE(void)
{
	int ret;
	struct io_src_pid pid_src;
	struct io_src *src;

	void dummy_cb(struct io_src_pid *src)
	{

	}

	/* normal use cases */
	ret = io_src_pid_init(&(pid_src), 1, dummy_cb);
	CU_ASSERT_EQUAL(ret, 0);
	src = io_src_pid_get_source(&(pid_src));
	CU_ASSERT_EQUAL(src, &(pid_src.src));

	/* cleanup */
	io_src_clean(src);

	/* error use cases */
	src = io_src_pid_get_source(NULL);
	CU_ASSERT_EQUAL(src, NULL);
}

static const test_t tests[] = {
		{
				.fn = testSRC_PID_INIT,
				.name = "io_src_pid_init"
		},
		{
				.fn = testSRC_PID_GET_SOURCE,
				.name = "io_src_pid_get_source"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

suite_t src_pid_suite = {
		 /* name of the module, corresponds to the functions prefix */
		.name = "io_src_pid",
		.init = NULL,
		.clean = NULL,
		.tests = tests,
};

