/**
 * @file fautes.h
 * @date 19 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Header a library need to include in order to add Fautes support
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef FAUTES_H_
#define FAUTES_H_

#include <CUnit/CUError.h>

/**
 * @typedef test_t
 * @brief Unit test
 */
struct test_t {
	CU_TestFunc fn;
	const char *name;
};

/**
 * @typedef suite_t
 * @brief Unit tests suite, one must define a structure of this type prior to
 * a suite_register() call
 */
struct suite_t {
	const char *name;	/**< test suite's name */
	CU_InitializeFunc init;
	CU_CleanupFunc clean;
	const struct test_t *tests;
	int active;
};

#endif /* FAUTES_H_ */
