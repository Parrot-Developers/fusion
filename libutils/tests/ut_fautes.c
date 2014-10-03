/**
 * @file rs_fautes.c
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests libutils. Definitions for Fautes support
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <stdlib.h>

#include <fautes.h>

#include "ut_fautes.h"

struct suite_t *libutils_test_suites[] = {
		&file_suite,
		&module_suite,
		&process_suite,
		&string_suite,
		NULL, /* NULL guard */
};

static void libutils_pool_initializer(void)
{
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(file_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(module_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(process_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(string_suite);
}

struct pool_t fautes_pool = {
	.name = "libutils",
	.initializer = libutils_pool_initializer,
	.suites = libutils_test_suites,
};
