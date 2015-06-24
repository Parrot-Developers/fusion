/**
 * @file io_process_test.c
 * @date 17 juin 2015
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for io_process module
 *
 * Copyright (C) 2015 Parrot S.A.
 */
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <stdbool.h>

#include <CUnit/Basic.h>

#include <ut_string.h>
#include <ut_file.h>

#include <io_process.h>

#include <fautes.h>

#define PROCESS_TEST_SCRIPT_ENV "PROCESS_TEST_SCRIPT"

struct process_test {
	struct io_process process;
	char stdout_buffer[0x100];
	char stderr_buffer[0x100];
	bool terminated;
	bool stdin_cb_called;
	bool stdout_cb_called;
	bool stdout_sep_cb_called;
	bool stderr_cb_called;
	bool stderr_cb_error_encountered;
	bool stderr_sep_cb_called;
};

static void stdout_sep_cb(struct io_src_sep *sep, char *chunk, unsigned len)
{
	struct io_process *process = ut_container_of(sep, struct io_process,
			stdout_src);
	struct process_test *test = ut_container_of(process,
			struct process_test, process);

	if (len != 0) {
		snprintf(test->stdout_buffer,
				UT_ARRAY_SIZE(test->stdout_buffer), "%*s", len,
				chunk);
		test->stdout_sep_cb_called = true;
	}

	/* an HUP error is normal since the pipes are destroyed */
}

static void stderr_sep_cb(struct io_src_sep *sep, char *chunk, unsigned len)
{
	struct io_process *process = ut_container_of(sep, struct io_process,
			stderr_src);
	struct process_test *test = ut_container_of(process,
			struct process_test, process);

	if (len != 0) {
		snprintf(test->stderr_buffer,
				UT_ARRAY_SIZE(test->stderr_buffer), "%*s", len,
				chunk);
		test->stderr_sep_cb_called = true;
	}

	/* an HUP error is normal since the pipes are destroyed */
	if (io_src_has_error(io_src_sep_get_source(sep)))
		test->stderr_cb_error_encountered = true;
}

static void termination_cb(struct io_src_pid *pid_src, pid_t pid, int status)
{
	struct io_process *process = ut_container_of(pid_src, struct io_process,
			pid_src);
	struct process_test *test = ut_container_of(process,
			struct process_test, process);

	CU_ASSERT(WIFEXITED(status));
	CU_ASSERT_EQUAL(WEXITSTATUS(status), 1);
	test->terminated = true;
}

static void testPROCESS_INIT_PREPARE_LAUNCH_AND_WAIT(void)
{
	int ret;
	struct process_test test = {
			.terminated = false,
			.stdin_cb_called = false,
			.stdout_cb_called = false,
			.stdout_sep_cb_called = false,
			.stderr_cb_called = false,
			.stderr_sep_cb_called = false,
	};
	const char *msg = "tutu\ntata\n";
	struct io_process_parameters parameters = {
			.buffer = msg,
			.len = sizeof(msg) + 1,
			.copy = true,
			.stdout_sep_cb = stdout_sep_cb,
			.out_sep1 = '\n',
			.out_sep2 = IO_SRC_SEP_NO_SEP2,
			.stderr_sep_cb = stderr_sep_cb,
			.err_sep1 = '\n',
			.err_sep2 = IO_SRC_SEP_NO_SEP2,
			.timeout = 3000,
			.signum = SIGKILL,
	};

	/* normal use case */
	ret = io_process_init_prepare_launch_and_wait(&test.process,
			&parameters, termination_cb,
			getenv(PROCESS_TEST_SCRIPT_ENV), NULL);
	CU_ASSERT(ret >= 0);
	CU_ASSERT_STRING_EQUAL(test.stdout_buffer, "tutu\n");
	CU_ASSERT_STRING_EQUAL(test.stderr_buffer, "tata\n");
	CU_ASSERT(test.stdout_sep_cb_called);
	CU_ASSERT(test.stderr_sep_cb_called);
	CU_ASSERT(test.terminated);
	CU_ASSERT(test.stderr_cb_error_encountered);


	/* error use cases, pfiouuuu, there are plenty... */
	/* normally, nothing to clean */
}

static void testPROCESS_INIT_PREPARE(void)
{
	int ret;
	struct process_test test = {
			.terminated = false,
			.stdin_cb_called = false,
			.stdout_cb_called = false,
			.stdout_sep_cb_called = false,
			.stderr_cb_called = false,
			.stderr_sep_cb_called = false,
	};
	const char *msg = "tutu\ntata\n";
	struct io_process_parameters parameters = {
			.buffer = msg,
			.len = sizeof(msg) + 1,
			.copy = true,
			.stdout_sep_cb = stdout_sep_cb,
			.out_sep1 = '\n',
			.out_sep2 = IO_SRC_SEP_NO_SEP2,
			.stderr_sep_cb = stderr_sep_cb,
			.err_sep1 = '\n',
			.err_sep2 = IO_SRC_SEP_NO_SEP2,
			.timeout = 3000,
			.signum = SIGKILL,
	};

	/* normal use case */
	ret = io_process_init_prepare(&test.process, &parameters,
			termination_cb, getenv(PROCESS_TEST_SCRIPT_ENV), NULL);
	CU_ASSERT(ret >= 0);
	ret = io_process_launch(&test.process);
	CU_ASSERT(ret >= 0);
	ret = io_process_wait(&test.process);
	CU_ASSERT(ret >= 0);
	CU_ASSERT_STRING_EQUAL(test.stdout_buffer, "tutu\n");
	CU_ASSERT_STRING_EQUAL(test.stderr_buffer, "tata\n");
	CU_ASSERT(test.stdout_sep_cb_called);
	CU_ASSERT(test.stderr_sep_cb_called);
	CU_ASSERT(test.terminated);
	CU_ASSERT(test.stderr_cb_error_encountered);

	/* error use cases, pfiouuuu, there are plenty... */
	/* normally, nothing to clean */
}

static const struct test_t tests[] = {
		{
				.fn = testPROCESS_INIT_PREPARE_LAUNCH_AND_WAIT,
				.name = "io_process_init_prepare_launch_and_"
						"wait"
		},
		{
				.fn = testPROCESS_INIT_PREPARE,
				.name = "io_process_init_prepare"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_process_suite(void)
{
	if (getuid() != 0) {
		fprintf(stderr, "\n"__FILE__" test suite can't run without "
				"root privileges\n");
		return -1;
	}
	if (!ut_file_is_executable(getenv(PROCESS_TEST_SCRIPT_ENV))) {
		fprintf(stderr, "\nThe environment variable "
				PROCESS_TEST_SCRIPT_ENV"must be set to the "
				"path of the test.process script.\n");
		return -1;
	}

	return 0;
}

static int clean_process_suite(void)
{
	return 0;
}

struct suite_t process_suite = {
		.name = "io_process",
		.init = init_process_suite,
		.clean = clean_process_suite,
		.tests = tests,
};
