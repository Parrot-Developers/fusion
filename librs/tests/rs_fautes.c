/**
 * @file rs_fautes.c
 * @date Oct 19, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests librs. Definitions for Fautes support
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <fautes.h>

#include "rs_fautes.h"

const char const *fautes_lib_name = "librs";

struct suite_t *librs_test_suites[] = {
		&dll_suite,
		&hmap_suite,
		&node_suite,
		&rb_suite,
		&utils_suite,
		NULL, /* NULL guard */
};

void librs_init_test_suites(void)
{
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(dll_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(hmap_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(node_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(rb_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(utils_suite);
}
