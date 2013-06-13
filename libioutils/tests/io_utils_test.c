/**
 * @file skel_test.c
 * @date 13 june 2013
 * @author nicolas.carrier@parrot.com
 * @brief unit tests for utils module
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <unistd.h>
#include <fcntl.h>

#include <CUnit/Basic.h>

#include <fautes.h>

#include <io_platform.h>

#include "io_utils.h"

static void testSET_NON_BLOCKING(void)
{
	int flags;
	int ret;
	int fd = open("/dev/urandom", O_RDWR | O_CLOEXEC);

	CU_ASSERT_NOT_EQUAL_FATAL(fd, -1);

	/* normal use cases */
	ret = io_set_non_blocking(fd);
	CU_ASSERT_EQUAL(ret, 0);
	flags = fcntl(fd, F_GETFL, 0);
	CU_ASSERT(flags & O_NONBLOCK);

	/* error use cases */
	ret = io_set_non_blocking(-1);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	close(fd);
}

static const struct test_t tests[] = {
		{
				.fn = testSET_NON_BLOCKING,
				.name = "io_set_non_blocking"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

struct suite_t utils_suite = {
		.name = "io_utils",
		.init = NULL,
		.tests = tests,
};

