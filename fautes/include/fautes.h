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

#include <stdlib.h>

#include <CUnit/CUError.h>

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * @def GET_ACTIVE_STATE_FROM_ENVIRONMENT
 * @param suite Name of the test suite to get the active state of
 * @brief Reads from environment variables, if a given test suite must be
 * activated or not. If FAUTES_SUITE_ACTIVE_STATE_my_suite_name is set to "0",
 * the suite "my_suite_name" will be disabled, if set to anything but "0", it
 * will be enabled. If FAUTES_SUITE_ACTIVE_STATE_my_suite_name isn't defined,
 * but FAUTES_DEFAULT_ACTIVE_STATE is, then if it is set to "0", the suite will
 * be disabled if set to anything but "0", it will be enabled
 */
#define FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT(suite) do { \
	char *__das = getenv("FAUTES_DEFAULT_ACTIVE_STATE"); \
	char *__sas = getenv("FAUTES_SUITE_ACTIVE_STATE_" #suite); \
	int __default_active_state; \
	int __active_state; \
\
	if (NULL == __das) \
		__default_active_state = 1; \
	else \
		__default_active_state = 0 != strcmp(__das, "0"); \
	if (NULL == __sas) \
		__active_state = __default_active_state; \
	else \
		__active_state = 0 != strcmp(__sas, "0"); \
	suite.active = __active_state; \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* FAUTES_H_ */
