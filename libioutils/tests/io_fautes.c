/**
 * @file io_fautes.c
 * @date Oct 19, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests libioutils. Definitions for Fautes support
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <stdlib.h>

#include <fautes.h>

#include "io_fautes.h"

struct suite_t *libioutils_test_suites[] = {
		&io_suite,
		&mon_suite,
		&src_inot_suite,
		&src_msg_suite,
		&src_msg_uad_suite,
		&src_pid_suite,
		&src_sep_suite,
		&src_sig_suite,
		&src_suite,
		&src_tmr_suite,
		&utils_suite,

		NULL, /* NULL guard */
};

static void libioutils_pool_initializer(void)
{
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(io_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(mon_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_inot_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_msg_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_msg_uad_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_pid_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_sep_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_sig_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(src_tmr_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(utils_suite);
}

struct pool_t fautes_pool = {
	.name = "libioutils",
	.initializer = libioutils_pool_initializer,
	.suites = libioutils_test_suites,
};
