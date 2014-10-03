/**
 * @file ut_process_test.c
 * @date 3 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @brief unit tests for libutils' process module
 *
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <CUnit/Basic.h>

#include "../include/ut_process.h"

#include <fautes.h>
#include <fautes_utils.h>

static void testUT_PROCESS_VSYSTEM(void)
{
	int ret;

	/* normal use cases */
	ret = ut_process_vsystem("echo tutu %s %x", "tata", 0x7070);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use cases */
	ret = ut_process_vsystem("hopefullynonexistent%s", "executable");
	CU_ASSERT(WIFEXITED(ret));
	CU_ASSERT_EQUAL(WEXITSTATUS(ret), 127);
	ret = ut_process_vsystem(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static const struct test_t tests[] = {
		{
				.fn = testUT_PROCESS_VSYSTEM,
				.name = "ut_process_vsystem"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_process_suite(void)
{
	return 0;
}

static int clean_process_suite(void)
{

	return 0;
}

struct suite_t process_suite = {
		.name = "ut_process",
		.init = init_process_suite,
		.clean = clean_process_suite,
		.tests = tests,
};
