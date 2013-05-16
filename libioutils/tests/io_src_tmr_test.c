/**
 * @file io_tmr_src_test.c
 * @date 16 may 2013
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for timer io source
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <CUnit/Basic.h>

#include <fautes.h>

static void testIO_SRC_TMR_INIT(void)
{
	/* initialization */

	/* normal use cases */

	/* error use cases */

	/* cleanup */
}

static const struct test_t tests[] = {
		{
				.fn = testIO_SRC_TMR_INIT,
				.name = "io_src_tmr_init"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

struct suite_t src_tmr_suite = {
		 /* name of the module, corresponds to the functions prefix */
		.name = "io_src_tmr",
		.init = NULL,
		.clean = NULL,
		.tests = tests,
};

