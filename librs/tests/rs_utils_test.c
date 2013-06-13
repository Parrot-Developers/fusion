/**
 * @file rs_utils_test.c
 * @date Sept 28, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for rs_utils module
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <unistd.h>

#include <stdio.h>

#include <CUnit/Basic.h>

#include <rs_utils.h>

#include <fautes.h>

static void testRS_STR_IS_INVALID(void)
{
	int ret = 0;

	ret = rs_str_is_invalid("titi");
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_str_is_invalid("");
	CU_ASSERT_EQUAL(ret, 1);
	ret = rs_str_is_invalid(NULL);
	CU_ASSERT_EQUAL(ret, 1);
}

static void testRS_STR_FREE(void)
{
	char *str;

	str = strdup("titi");
	CU_ASSERT_PTR_NOT_NULL_FATAL(str);

	/* normal use case */
	rs_str_free(&str);
	CU_ASSERT_PTR_NULL(str);

	/* error use cases */
	/* str now points to NULL */
	rs_str_free(&str);
	rs_str_free(NULL);
}

static const struct test_t tests[] = {
		{
				.fn = testRS_STR_IS_INVALID,
				.name = "rs_str_is_invalid"
		},
		{
				.fn = testRS_STR_FREE,
				.name = "rs_str_free"
		},
		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

struct suite_t utils_suite = {
		.name = "rs_utils",
		.init = NULL,
		.clean = NULL,
		.tests = tests,
};
