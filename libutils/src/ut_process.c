/**
 * @file ut_process.c
 * @brief utilities for executing processes
 *
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "ut_string.h"
#include "ut_process.h"

int ut_process_vsystem(const char *fmt, ...)
{
	int ret = -1;
	char __attribute__((cleanup(ut_string_free))) *cmd = NULL;
	va_list args;

	va_start(args, fmt);
	ret = vasprintf(&cmd, fmt, args);
	va_end(args);
	if (-1 == ret) {
		cmd = NULL;
		return -1;
	}

	return system(cmd);
}
