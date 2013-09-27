/**
 * @file skel_test.c
 * @date 19 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for th io module
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <CUnit/Basic.h>

#include <fautes.h>
#include <fautes_utils.h>

static void testIO_CREATE(void)
{
	/* initialization */

	/* normal use cases */

	/* error use cases */

	/* cleanup */
}

static const struct test_t tests[] = {
		{
				.fn = testIO_CREATE,
				.name = "io_io_create"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_io_suite(void)
{
	return 0; /* return non-zero on error */
}

static int clean_io_suite(void)
{
	return 0; /* return non-zero on error */
}

struct suite_t io_suite = {
		.name = "io_io",
		.init = init_io_suite,
		.clean = clean_io_suite,
		.tests = tests,
};

