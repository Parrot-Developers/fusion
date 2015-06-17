/**
 * @file io_fautes.c
 * @date Oct 19, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests libioutils. Definitions for Fautes support
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <unistd.h>

#include <stdlib.h>

#include <fautes.h>

#include "io_fautes.h"

/* set an interpreter section so that this lib can be run as an executable */
const char io_interp[] __attribute__((section(".interp"))) =
		FUSION_INTERPRETER;

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

void libioutils_tests(void)
{
	int ret;
	fprintf(stderr, "HERE\n");

	ret = fautes_run_test_pool(&fautes_pool, false);

	_exit(ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
