/**
 * @file ut_string_test.c
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @brief unit tests for libutils' string module
 *
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#include <CUnit/Basic.h>

#include "../include/ut_string.h"

#include <fautes.h>
#include <fautes_utils.h>

static void testUT_STRING_FREE(void)
{
	char *str;

	str = strdup("titi");
	CU_ASSERT_PTR_NOT_NULL_FATAL(str);

	/* normal use case */
	ut_string_free(&str);
	CU_ASSERT_PTR_NULL(str);

	/* error use cases */
	/* str now points to NULL */
	ut_string_free(&str);
	ut_string_free(NULL);
}

static void testUT_STRING_IS_INVALID(void)
{
	int ret = 0;

	ret = ut_string_is_invalid("titi");
	CU_ASSERT_FALSE(ret);
	ret = ut_string_is_invalid("");
	CU_ASSERT(ret);
	ret = ut_string_is_invalid(NULL);
	CU_ASSERT(ret);
}

static void testUT_STRING_APPEND(void)
{
	char *str = NULL;
	int ret;

	/* normal use cases */
	ret = ut_string_append(&str, "toto %d ", 42);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_STRING_EQUAL(str, "toto 42 ");
	ret = ut_string_append(&str, "tata %d", 666);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_STRING_EQUAL(str, "toto 42 tata 666");

	/* cleanup */
	ut_string_free(&str);

	/* error use cases */
	ret = ut_string_append(&str, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testUT_STRING_MATCH_PREFIX(void)
{
	bool ret;

	/* normal use cases */
	ret = ut_string_match_prefix("tata", "ta");
	CU_ASSERT(ret);
	ret = ut_string_match_prefix("toto", "ta");
	CU_ASSERT_FALSE(ret);
	ret = ut_string_match_prefix("tata", "tatata");
	CU_ASSERT_FALSE(ret);

	/* error use cases */
	ret = ut_string_match_prefix("plop", NULL);
	CU_ASSERT_FALSE(ret);
	ret = ut_string_match_prefix(NULL, "pas glop");
	CU_ASSERT_FALSE(ret);
}

static void testUT_STRING_RSTRIP(void)
{
	char test1[] = "my string    \t\v\r\n";
	char test2[] = "my string";
	char test3[] = "";

	/* normal use cases */
	CU_ASSERT_STRING_EQUAL(ut_string_rstrip(test1), "my string");
	CU_ASSERT_STRING_EQUAL(ut_string_rstrip(test2), "my string");
	CU_ASSERT_STRING_EQUAL(ut_string_rstrip(test3), "");

	/* error use cases */
	ut_string_rstrip(NULL);
}

static void testUT_STRING_LSTRIP(void)
{
	char test1[] = "    \t\v\r\nmy string";
	char test2[] = "my string";
	char test3[] = "";

	/* normal use cases */
	CU_ASSERT_STRING_EQUAL(ut_string_lstrip(test1), "my string");
	CU_ASSERT_STRING_EQUAL(ut_string_lstrip(test2), "my string");
	CU_ASSERT_STRING_EQUAL(ut_string_lstrip(test3), "");

	/* error use cases */
	ut_string_rstrip(NULL);
}

static void testUT_STRING_STRIP(void)
{
	char test1[] = "    \t\v\r\nmy string    \t\v\r\n";
	char test2[] = "my string";
	char test3[] = "";

	/* normal use cases */
	CU_ASSERT_STRING_EQUAL(ut_string_strip(test1), "my string");
	CU_ASSERT_STRING_EQUAL(ut_string_strip(test2), "my string");
	CU_ASSERT_STRING_EQUAL(ut_string_strip(test3), "");

	/* error use cases */
	ut_string_rstrip(NULL);
}

static const struct test_t tests[] = {
		{
				.fn = testUT_STRING_FREE,
				.name = "ut_string_free"
		},
		{
				.fn = testUT_STRING_IS_INVALID,
				.name = "ut_string_is_invalid"
		},
		{
				.fn = testUT_STRING_APPEND,
				.name = "ut_string_append"
		},
		{
				.fn = testUT_STRING_MATCH_PREFIX,
				.name = "ut_string_match_prefix"
		},
		{
				.fn = testUT_STRING_RSTRIP,
				.name = "ut_string_rstrip",
		},
		{
				.fn = testUT_STRING_LSTRIP,
				.name = "ut_string_lstrip",
		},
		{
				.fn = testUT_STRING_STRIP,
				.name = "ut_string_strip",
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_string_suite(void)
{
	return 0;
}

static int clean_string_suite(void)
{

	return 0;
}

struct suite_t string_suite = {
		.name = "ut_string",
		.init = init_string_suite,
		.clean = clean_string_suite,
		.tests = tests,
};
