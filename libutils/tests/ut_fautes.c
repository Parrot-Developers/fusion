/**
 * @file rs_fautes.c
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests libutils. Definitions for Fautes support
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <unistd.h>

#include <stdlib.h>

#include <fautes.h>

#include <ut_module.h>

#include "ut_fautes.h"

/* set an interpreter section so that this lib can be run as an executable */
const char ut_interp[] __attribute__((section(".interp"))) = FUSION_INTERPRETER;

struct suite_t *libutils_test_suites[] = {
		&bits_suite,
		&file_suite,
		&module_suite,
		&process_suite,
		&string_suite,
		NULL, /* NULL guard */
};

static void libutils_pool_initializer(void)
{
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(bits_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(file_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(module_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(process_suite);
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(string_suite);
}

struct pool_t fautes_pool = {
	.name = "libutils",
	.initializer = libutils_pool_initializer,
	.suites = libutils_test_suites,
};

void libutils_tests(void)
{
	int ret;

	ut_module_init();

	ret = fautes_run_test_pool(&fautes_pool, fautes_generate_xml());

	/*
	 * the libutils desstructor can't be called when libutils.so is ran
	 * directly, we need to call it manually
	 */
	ut_module_clean();

	_exit(ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
