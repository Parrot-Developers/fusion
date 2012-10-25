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

suite_t *librs_test_suites[] = {
		&dll_suite,
		&node_suite,
		NULL, /* NULL guard */
};

void librs_init_test_suites(void)
{
	int default_active_state = 1;

	dll_suite.active = default_active_state;
	node_suite.active = default_active_state;
}
