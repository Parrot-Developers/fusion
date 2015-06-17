/**
 * @file pw_fautes.c
 * @date 29 apr. 2013
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests libpidwatch. Definitions for Fautes support
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <unistd.h>

#include <fautes.h>

#include "pw_fautes.h"

/* set an interpreter section so that this lib can be run as an executable */
const char pw_interp[] __attribute__((section(".interp"))) = FUSION_INTERPRETER;

struct suite_t *libpidwatch_test_suites[] = {
		&pidwatch_suite,

		NULL, /* NULL guard */
};

static void libpidwatch_pool_initializer(void)
{
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(pidwatch_suite);
}

struct pool_t fautes_pool = {
	.name = "libpidwatch",
	.initializer = libpidwatch_pool_initializer,
	.suites = libpidwatch_test_suites,
};

void libpidwatch_tests(void)
{
	int ret;

	ret = fautes_run_test_pool(&fautes_pool, false);

	_exit(ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
