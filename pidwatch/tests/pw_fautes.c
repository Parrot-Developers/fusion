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

struct suite_t *libpidwatch_test_suites[] = {
		&pidwatch_suite,

		NULL, /* NULL guard */
};

void libpidwatch_pool_initializer(void)
{
	FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(pidwatch_suite);
}

struct pool_t fautes_pool = {
	.name = "libpidwatch",
	.initializer = libpidwatch_pool_initializer,
	.suites = libpidwatch_test_suites,
};
