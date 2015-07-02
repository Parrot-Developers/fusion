/**
 * @file ut_log.c
 * @brief logging facilities for libutils
 *
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#include "ut_log.h"

const char *ut_log_level_str[] = {
		[UT_CRIT]	= "\e[7;31mC\e[0m",
		[UT_ERR]	= "\e[1;31mE\e[0m",
		[UT_WARN]	= "\e[1;33mW\e[0m",
		[UT_NOTICE]	= "\e[1;32mN\e[0m",
		[UT_INFO]	= "\e[1;35mI\e[0m",
		[UT_DEBUG]	= "\e[1;36mD\e[0m",
};

ut_log_f *ut_log_cb = printf;
