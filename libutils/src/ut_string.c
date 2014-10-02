/**
 * @file ut_string.c
 * @brief utilities for string manipulations
 *
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>

#include "ut_log.h"
#include "ut_string.h"

void ut_string_free(char **str)
{
	if (str == NULL || *str == NULL)
		return;

	free(*str);
	*str = NULL;
}

bool ut_string_is_invalid(const char *str)
{
	return str == NULL || *str == '\0';
}

int ut_string_append(char **str, const char *fmt, ...)
{
	va_list args;
	char __attribute__((cleanup(ut_string_free))) *suffix = NULL;
	size_t len = *str != NULL ? strlen(*str) : 0;
	int suffix_len;

	if (ut_string_is_invalid(fmt))
		return -EINVAL;

	va_start(args, fmt);
	suffix_len = vasprintf(&suffix, fmt, args);
	va_end(args);
	if (suffix_len < 0) {
		ut_err("vasprintf failed");
		return -ENOMEM;
	}

	*str = realloc(*str, len + suffix_len + 1);
	if (*str == NULL)
		return -ENOMEM;

	snprintf(*str + len, suffix_len + 1, "%s", suffix);

	return 0;
}

bool ut_string_match_prefix(const char *str, const char *prefix)
{
	if (str == NULL || prefix == NULL)
		return 0;

	return strncmp(str, prefix, strlen(prefix)) == 0;
}

char *ut_string_rstrip(char *str)
{
	size_t s;

	if (ut_string_is_invalid(str))
		return str;

	s = strlen(str);
	while (s-- && isspace(str[s]))
		str[s] = '\0';

	return str;
}

char *ut_string_lstrip(char *str)
{
	if (ut_string_is_invalid(str))
		return str;

	while (*str && isspace(*str))
		*(str++) = '\0';

	return str;
}

char *ut_string_strip(char *str)
{
	return ut_string_lstrip(ut_string_rstrip(str));
}
