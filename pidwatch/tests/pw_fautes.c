/**
 * @file io_fautes.c
 * @date 29 apr. 2013
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests libpidwatch. Definitions for Fautes support
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <fautes.h>

#include "pw_fautes.h"

const char const *fautes_lib_name = "libpidwatch";

suite_t *libpidwatch_test_suites[] = {
		&pidwatch_suite,

		NULL, /* NULL guard */
};

void libpidwatch_init_test_suites(void)
{
	pidwatch_suite.active = 1;
}
