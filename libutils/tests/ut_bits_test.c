/**
 * @file ut_file_test.c
 * @date 19 nov. 2014
 * @author nicolas.carrier@parrot.com
 * @brief unit tests for libutils' bits module
 *
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#include <CUnit/Basic.h>

#include "../include/ut_bits.h"

#include <fautes.h>
#include <fautes_utils.h>

static void testUT_FILE_IS_EXECUTABLE(void)
{
	int c;
	char lut[] = {
			['A'] = 'a',
			['B'] = 'b',
			['C'] = 'c',
			['D'] = 'd',
			['E'] = 'e',
			['F'] = 'f',
			['G'] = 'g',
			['H'] = 'h',
			['I'] = 'i',
			['J'] = 'j',
			['K'] = 'k',
			['L'] = 'l',
			['M'] = 'm',
			['N'] = 'n',
			['O'] = 'o',
			['P'] = 'p',
			['Q'] = 'q',
			['R'] = 'r',
			['S'] = 's',
			['T'] = 't',
			['U'] = 'u',
			['V'] = 'v',
			['W'] = 'w',
			['X'] = 'x',
			['Y'] = 'y',
			['Z'] = 'z',
			['a'] = 'a',
			['b'] = 'b',
			['c'] = 'c',
			['d'] = 'd',
			['e'] = 'e',
			['f'] = 'f',
			['g'] = 'g',
			['h'] = 'h',
			['i'] = 'i',
			['j'] = 'j',
			['k'] = 'k',
			['l'] = 'l',
			['m'] = 'm',
			['n'] = 'n',
			['o'] = 'o',
			['p'] = 'p',
			['q'] = 'q',
			['r'] = 'r',
			['s'] = 's',
			['t'] = 't',
			['u'] = 'u',
			['v'] = 'v',
			['w'] = 'w',
			['x'] = 'x',
			['y'] = 'y',
			['z'] = 'z',
	};

	for (c = 'a'; c <= 'z'; c++)
		CU_ASSERT(lut[c] == UT_TO_LOWER(c));
	for (c = 'A'; c <= 'Z'; c++)
		CU_ASSERT(lut[c] == UT_TO_LOWER(c));
}

static void testUT_IS_09_OR_AZ_NO_CASE(void)
{
	int c;

	for (c = 'a'; c <= 'z'; c++)
		CU_ASSERT(UT_IS_09_OR_AZ_NO_CASE(c));
	for (c = 'A'; c <= 'Z'; c++)
		CU_ASSERT(UT_IS_09_OR_AZ_NO_CASE(c));
	for (c = '0'; c <= '9'; c++)
		CU_ASSERT(UT_IS_09_OR_AZ_NO_CASE(c));
}

static const struct test_t tests[] = {
		{
				.fn = testUT_FILE_IS_EXECUTABLE,
				.name = "UT_TO_LOWER"
		},
		{
				.fn = testUT_IS_09_OR_AZ_NO_CASE,
				.name = "UT_IS_09_OR_AZ_NO_CASE"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_bits_suite(void)
{
	return 0;
}

static int clean_bits_suite(void)
{

	return 0;
}

struct suite_t bits_suite = {
		.name = "ut_bits",
		.init = init_bits_suite,
		.clean = clean_bits_suite,
		.tests = tests,
};
