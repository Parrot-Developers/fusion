/**
 * @file io_fautes.c
 * @date Oct 19, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests libioutils. Definitions for Fautes support
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <fautes.h>

#include "io_fautes.h"

const char const *fautes_lib_name = "libioutils";

suite_t *libioutils_test_suites[] = {
		&mon_suite,
		&src_msg_suite,
		&src_msg_uad_suite,
		&src_pid_suite,
		&src_sep_suite,
		&src_sig_suite,
		&src_suite,

		NULL, /* NULL guard */
};

void libioutils_init_test_suites(void)
{
	mon_suite.active = 1;
	src_msg_suite.active = 1;
	src_msg_uad_suite.active = 1;
	src_pid_suite.active = 1;
	src_sep_suite.active = 1;
	src_sig_suite.active = 1;
	src_suite.active = 1;
}
