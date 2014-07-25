/**
 * @file rs_fautes.c
 * @date Oct 19, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests librs. Definitions for Fautes support
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <stdlib.h>

#include <fautes.h>

#include "rs_fautes.h"

struct suite_t *librs_test_suites[] = {
		&dll_suite,
		&hmap_suite,
		&node_suite,
		&rb_suite,
		&utils_suite,
		NULL, /* NULL guard */
};

static void librs_pool_initializer(void)
{
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(dll_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(hmap_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(node_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(rb_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(utils_suite);
}

struct pool_t fautes_pool = {
	.name = "librs",
	.initializer = librs_pool_initializer,
	.suites = librs_test_suites,
};
