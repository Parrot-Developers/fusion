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

struct suite_t *libioutils_test_suites[] = {
		&mon_suite,
		&src_msg_suite,
		&src_msg_uad_suite,
		&src_pid_suite,
		&src_sep_suite,
		&src_sig_suite,
		&src_suite,
		&src_tmr_suite,

		NULL, /* NULL guard */
};

void libioutils_init_test_suites(void)
{
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(mon_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_msg_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_msg_uad_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_pid_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_sep_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_sig_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_tmr_suite);
}
