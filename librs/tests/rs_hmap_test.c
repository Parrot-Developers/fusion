/**
 * @file rs_hmap_test.c
 * @date 14 mai 2013
 * @author nicolas.carrier@parrot.com
 * @brief unit tests for librs hash map implementation
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <CUnit/Basic.h>

#include <fautes.h>

static void testRS_HMAP_INIT(void)
{
	/* initialization */

	/* normal use cases */

	/* error use cases */

	/* cleanup */
}

static const struct test_t tests[] = {
		{
				.fn = testRS_HMAP_INIT,
				.name = "rs_hmap_init"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_hmap_suite(void)
{
	return 0; /* return non-zero on error */
}

static int clean_hmap_suite(void)
{
	return 0; /* return non-zero on error */
}

struct suite_t hmap_suite = {
		.name = "rs_hmap",
		.init = init_hmap_suite,
		.clean = clean_hmap_suite,
		.tests = tests,
};

