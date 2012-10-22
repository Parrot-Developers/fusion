/**
 * @file utils_test.c
 * @date 17 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for io_utils module
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <unistd.h>
#include <fcntl.h>

#include <CUnit/Basic.h>

#include "../src/io_utils.h"

#include <fautes.h>

static void testSET_NON_BLOCKING(void)
{
	int pipefd[2] = {-1, -1};
	int ret = -1;
	int flags = -1;

	ret = pipe(pipefd);
	CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);

	/* normal use cases */
	flags = fcntl(pipefd[0], F_GETFL, 0);
	CU_ASSERT_EQUAL(flags & O_NONBLOCK, 0);
	ret = set_non_blocking(pipefd[0]);
	CU_ASSERT_EQUAL(ret, 0);
	flags = fcntl(pipefd[0], F_GETFL, 0);
	CU_ASSERT_EQUAL(!!(flags & O_NONBLOCK), 1);

	/* error use cases */
	ret = set_non_blocking(-1);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	close(pipefd[0]);
	close(pipefd[1]);
}

static const test_t tests[] = {
		{
				.fn = testSET_NON_BLOCKING,
				.name = "set_non_blocking"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_utils_suite(void)
{
	return 0;
}

static int clean_utils_suite(void)
{
	return 0;
}

suite_t utils_suite = {
		.name = "io_utils",
		.init = init_utils_suite,
		.clean = clean_utils_suite,
		.tests = tests,
};
