/**
 * @file io_fautes.c
 * @date Oct 19, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests libioutils. Definitions for Fautes support
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <unistd.h>
#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <CUnit/Automated.h>
#include <CUnit/Basic.h>

#include <fautes.h>

extern suite_t mon_suite;
extern suite_t src_suite;
extern suite_t src_sig_suite;
extern suite_t utils_suite;

const char const *fautes_lib_name = "libioutils";

suite_t *libioutils_test_suites[] = {
		&mon_suite,
		&src_suite,
		&src_sig_suite,
		&utils_suite,

		NULL, /* NULL guard */
};

extern void libioutils_init_test_suites(void);
void libioutils_init_test_suites(void)
{
	int default_active_state = 1;

	mon_suite.active = default_active_state;
	src_suite.active = default_active_state;
	src_sig_suite.active = default_active_state;
	utils_suite.active = default_active_state;
}
