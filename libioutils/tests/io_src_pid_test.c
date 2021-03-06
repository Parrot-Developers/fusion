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
#include <sys/wait.h>

#include <unistd.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>

#include <CUnit/Basic.h>

#include <fautes.h>

#include <io_mon.h>
#include <io_src_pid.h>

static void dump_args(int argc, const char * const argv[])
{
	do {
		fprintf(stderr, "%s ", *argv);
	} while (*(++argv));
}

static pid_t __attribute__((sentinel)) launch(const char *prog, ...)
{
	int ret;
	int child_argc = 0;
	const char *child_argv[10] = {NULL};
	/* NULL would stop prematurely the for loop, hence (char *)-1 */
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
		fprintf(stderr, " (%jd)\n", (intmax_t)getpid());
		/*
		 * generates a -Wcast-qual warning, don't know what to do or if
		 * I can do anything in this case
		 */
		ret = execvp(child_argv[0], (char *const *)child_argv);
		if (-1 == ret) {
			perror("execve");
			exit(1);
		}
	}

	/* in parent */
	return pid;
}

struct my_pid_src {
	struct io_src_pid pid_src;
	bool process_dead;
};

static void cb(struct io_src_pid *src_pid, pid_t local_pid, int status)
{
	int wait_status;
	struct my_pid_src *s = ut_container_of(src_pid, struct my_pid_src,
			pid_src);

	CU_ASSERT(WIFEXITED(status));
	CU_ASSERT_EQUAL(WEXITSTATUS(status), 0);
	s->process_dead = true;
	waitpid(local_pid, &wait_status, 0);
	CU_ASSERT_EQUAL(status, wait_status);
};

static void testSRC_PID_INIT(void)
{
	int ret;
	fd_set rfds;
	struct io_mon mon;
	struct my_pid_src s = {
			.process_dead = false,
	};
	pid_t pid;
	struct timeval timeout;

	/* normal use cases */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	pid = launch("sleep", ".01", NULL);
	CU_ASSERT_NOT_EQUAL_FATAL(pid, -1);
	ret = io_src_pid_init(&s.pid_src, cb);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_pid_set_pid(&s.pid_src, pid);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_mon_add_source(&mon, &(s.pid_src.src));
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
	CU_ASSERT(ret >= 0);
	if (ret < 0)
		goto out;

	CU_ASSERT(s.process_dead);

out:
	/* cleanup */
	io_mon_clean(&mon);
	io_src_pid_clean(&s.pid_src);

	/* error use cases */
	ret = io_src_pid_init(0, cb);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_pid_init(&s.pid_src, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testSRC_PID_SET_PID(void)
{
	int ret;
	fd_set rfds;
	struct io_mon mon;
	struct my_pid_src s;
	pid_t pid;
	struct timeval timeout;

	/* normal use cases */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	pid = launch("sleep", ".01", NULL);
	CU_ASSERT_NOT_EQUAL_FATAL(pid, -1);
	ret = io_src_pid_init(&s.pid_src, cb);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_pid_set_pid(&s.pid_src, pid);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_mon_add_source(&mon, &(s.pid_src.src));
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
	CU_ASSERT(ret >= 0);
	if (ret < 0)
		goto out;

	CU_ASSERT(s.process_dead);

	/* reuse the pid source */
	s.process_dead = false;
	pid = launch("sleep", ".01", NULL);
	CU_ASSERT_NOT_EQUAL_FATAL(pid, -1);
	ret = io_src_pid_set_pid(&s.pid_src, pid);
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
	CU_ASSERT(ret >= 0);
	if (ret < 0)
		goto out;

	CU_ASSERT(s.process_dead);

out:
	/* cleanup */
	io_mon_clean(&mon);
	io_src_pid_clean(&s.pid_src);

	/* error use cases */
	/* fail if watching an already dead process */
	ret = io_src_pid_set_pid(&s.pid_src, pid);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_pid_set_pid(NULL, 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void dummy_cb(struct io_src_pid *local_src, pid_t pid, int status)
{

}

static void testSRC_PID_GET_SOURCE(void)
{
	int ret;
	struct io_src_pid pid_src;
	struct io_src *src;

	/* normal use cases */
	ret = io_src_pid_init(&(pid_src), dummy_cb);
	CU_ASSERT_EQUAL(ret, 0);
	src = io_src_pid_get_source(&(pid_src));
	CU_ASSERT_EQUAL(src, &(pid_src.src));

	/* cleanup */
	io_src_pid_clean(&pid_src);

	/* error use cases */
	src = io_src_pid_get_source(NULL);
	CU_ASSERT_EQUAL(src, NULL);
}

static const struct test_t tests[] = {
		{
				.fn = testSRC_PID_INIT,
				.name = "io_src_pid_init"
		},
		{
				.fn = testSRC_PID_GET_SOURCE,
				.name = "io_src_pid_get_source"
		},
		{
				.fn = testSRC_PID_SET_PID,
				.name = "io_src_pid_set_pid"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_src_pid_suite(void)
{
	if (getuid() != 0) {
		fprintf(stderr, "\n"__FILE__" test suite can't run without "
				"root privileges\n");
		return -1;
	}

	return 0;
}

struct suite_t src_pid_suite = {
		 /* name of the module, corresponds to the functions prefix */
		.name = "io_src_pid",
		.init = init_src_pid_suite,
		.clean = NULL,
		.tests = tests,
};

