/**
 * @file rs_utils.c
 * @date 27 juil. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Common generic utilities
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#include <stdlib.h>

#include "rs_utils.h"

void str_free(char **p)
{
	if (NULL == p || NULL == *p)
		return;

	free(*(p));
	*p = NULL;
}
