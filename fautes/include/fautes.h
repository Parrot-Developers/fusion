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
typedef struct {
	CU_TestFunc fn;
	const char *name;
} test_t;

/**
 * @typedef suite_t
 * @brief Unit tests suite, one must define a structure of this type prior to
 * a suite_register() call
 */
typedef struct {
	const char *name;	/**< test suite's name */
	CU_InitializeFunc init;
	CU_CleanupFunc clean;
	const test_t *tests;
	int active;
} suite_t;

#endif /* FAUTES_H_ */
