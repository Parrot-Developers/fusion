/**
 * @file skel_test.c
 * @date 19 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Example skeleton source file for unit tests implementation for a
 * module. On this example, we test the module skel of a lib named libmylib.
 * The functions of this libraries are prefixed with "mylib_" and functions of
 * the skel module are prefixed with "mylib_skel_"
 * @note naming conventions are not mandatory but strongly encouraged...
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <CUnit/Basic.h>

/*
 * optional, only needed if the testing facilities provided by Fautes are
 * needed
 */
#include <fautes.h>

/* one example testing function */
static void testSKEL_A_FUNCTION(void)
{
	/* initialization */
	/* initialization proper to this test */

	/* normal use cases */
	/* calls to mylib_skel_a_function, with test assertions */

	/* error use cases */
	/*
	 * calls to mylib_skel_a_function, with test assertions in error cases
	 * (e.g. wrong parameters values...)
	 */

	/* cleanup */
	/* cleanup proper to this test */

}

/*
 * set of test which compose the test suite, executed in order, until the NULL
 * guard
 */
static const test_t tests[] = {
		/*
		 * to add another test, simply add a {.fn = FN, .name = NAME}
		 * element before the NULL guard
		 */
		{
				.fn = testSKEL_A_FUNCTION,
				.name = "mylib_skel_a_function"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_skel_suite(void)
{
	/* initialization code concerning the whole tests suite */

	return 0; /* return non-zero on error */
}

static int clean_skel_suite(void)
{
	/* cleanup code concerning the whole tests suite */

	return 0; /* return non-zero on error */
}

suite_t src_suite = {
		 /* name of the module, corresponds to the functions prefix */
		.name = "mylib_skel",
		.init = init_skel_suite,
		.clean = clean_skel_suite,
		.tests = tests,
};

