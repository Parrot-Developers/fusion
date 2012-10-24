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
#include "io_fautes.h"

const char const *fautes_lib_name = "libioutils";

suite_t *libioutils_test_suites[] = {
		&mon_suite,
		&src_sep_suite,
		&src_sig_suite,
		&src_suite,
		&utils_suite,

		NULL, /* NULL guard */
};

void libioutils_init_test_suites(void)
{
	int default_active_state = 1;

	mon_suite.active = default_active_state;
	src_sep_suite.active = default_active_state;
	src_sig_suite.active = default_active_state;
	src_suite.active = default_active_state;
	utils_suite.active = default_active_state;
}
