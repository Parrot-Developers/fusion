/**
 * @file test.c
 * @date Mar 21, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests for netbox internal modules. Definitions for
 * Fautes support
 *
 * Copyright (C) 2011 Parrot S.A.
 */
#include <fautes.h>

extern suite_t dll_suite;
extern suite_t node_suite;

const char const *fautes_lib_name = "librs";

suite_t *librs_test_suites[] = {
		&dll_suite,
		&node_suite,
		NULL, /* NULL guard */
};

extern void librs_init_test_suites(void);
void librs_init_test_suites(void)
{
	int default_active_state = 1;

	dll_suite.active = default_active_state;
	node_suite.active = default_active_state;
}
