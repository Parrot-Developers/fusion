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
	int default_active_state = 1;

	dll_suite.active = default_active_state;
	hmap_suite.active = default_active_state;
	node_suite.active = default_active_state;
	rb_suite.active = default_active_state;
	utils_suite.active = default_active_state;
}
