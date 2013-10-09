/**
 * @file fautes.h
 * @date 19 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Header a library need to include in order to add Fautes support
 *
 * A library willing to add fautes support must define a variable
 * struct pool_t fautes_pool with at least the optional parameters defined
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
 * @struct test_t
 * @brief Unit test
 */
struct test_t {
	CU_TestFunc fn;		/**< function called for running the tests */
	const char *name;	/**< name of the test, must be unique */
};

/**
 * @struct suite_t
 * @brief Unit tests suite, one must define a structure of this type prior to
 * a suite_register() call
 */
struct suite_t {
	const char *name;		/**< test suite's name */
	CU_InitializeFunc init;		/**< CUnit initialize function (opt.) */
	CU_CleanupFunc clean;		/**< CUnit cleanup function (opt.) */
	const struct test_t *tests;	/**< NULL-terminated list of tests */
	int active;			/**< true if the test must be ran */
};

/**
 * @def fautes_pool_initializer_t
 * @brief Initialization function for a test pool. For example, read the active
 * state of test suites from environment with
 * FAUTES_GET_ACTIVE_STATE_FROM_ENVIRONMENT
 */
typedef void (*fautes_pool_initializer_t)(void);

/**
 * @struct pool_t
 * @brief test pool, gathering the tests suites. Main structure which must be
 * defined by a library or an executable for fautes support
 */
struct pool_t {
	/**
	 * test pool name (e.g. library name), required and must be unique.
	 * It must match the shared object basename, e.g. "libfautes" for
	 * testing /usr/lib/libfautes.so
	 */
	const char *name;
	/** initialization function of the pool (optional) */
	fautes_pool_initializer_t initializer;
	/** NULL-terminated list of test suites (required) */
	struct suite_t **suites;
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
