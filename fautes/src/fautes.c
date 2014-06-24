/**
 * @file test.c
 * @date Mar 21, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests loader for suitably adapted libraries or binaries
 *
 * Copyright (C) 2011 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <unistd.h>
#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include <CUnit/Automated.h>
#include <CUnit/Basic.h>

#include <fautes.h>

/**
 * Registers a test suite and all it's tests to the CUnit framework
 * @param suite Suite of unit tests
 * @return Error code of the first error to happen in either CU_add_suite
 * or CU_add_test
 */
static CU_ErrorCode suite_register(struct suite_t *suite)
{
	CU_pSuite pSuite = NULL;
	CU_pTest pTest = NULL;
	const struct test_t *test = &(suite->tests[0]);

	pSuite = CU_add_suite(suite->name, suite->init, suite->clean);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		fprintf(stderr, "CU_add_suite %s\n", CU_get_error_msg());
		return CU_get_error();
	}

	/* add the tests to the suite */
	do {
		pTest = CU_add_test(pSuite, test->name, test->fn);
		if (NULL == pTest) {
			CU_cleanup_registry();
			fprintf(stderr, "CU_add_test %s\n", CU_get_error_msg());
			return CU_get_error();
		}
	} while ((++test)->fn);

	return CUE_SUCCESS;
}

static void usage(char *progname)
{
	printf("usage %s [xml] lib1.so [lib2.so...]\n"
			"\tRuns the Fautes tests suites embedded in each "
			"library passed as final arguments\n"
			"\tIf xml is passed, result of the tests will be"
			" stored into CUnitAutomated-Results.xml, otherwise, "
			"they are echoed to standard output\n",
			basename(progname));
}

static int get_sym(void *lib_handle, const char *name, void **output)
{
	char *dl_error_name;

	dlerror();
	*output = dlsym(lib_handle, name);
	dl_error_name = dlerror();
	if (NULL != dl_error_name) {
		fprintf(stderr, "%s\n", dl_error_name);
		*output = NULL;
		return -1;
	}

	return 0;
}

static struct pool_t *get_test_pool(const char *so_lib, void **lib_handle)
{
	void *sym;
	struct pool_t *res = NULL;
	int ret;

	/* load the library to test */
	*lib_handle = dlopen(so_lib, RTLD_NOW);
	if (NULL == *lib_handle) {
		fprintf(stderr, "%s\n", dlerror());
		return NULL;
	}

	/* get it's name */
	ret = get_sym(*lib_handle, "fautes_pool", &sym);
	if (-1 == ret)
		goto out;
	res = (struct pool_t *)sym;

	if (0 != strncmp(res->name, basename(so_lib), strlen(res->name))) {
		fprintf(stderr, "so name '%s' mismatches with pool name '%s'\n",
				so_lib, res->name);
		res = NULL;
		goto out;
	}

	if ((*res).initializer)
		(*res).initializer();

	printf("Found test suite %s for %s\n", res->name, so_lib);
out:
	if (NULL == res)
		dlclose(lib_handle);

	return res;
}

int main(int argc, char *argv[])
{
	int xml = 0;
	CU_ErrorCode cu_err = CUE_SUCCESS;
	struct suite_t **suite;
	char **so_lib;
	void *lib_handle;
	unsigned tests_failed = 0;
	int failure;
	struct pool_t *pool;

	if (argc <= 1) {
		usage(argv[0]);
		return 1;
	}
	if (0 == strcmp(argv[1], "-h") || 0 == strcmp(argv[1], "--help")) {
		usage(argv[0]);
		return 0;
	}

	xml = 0 == strcmp(argv[1], "xml");
	/* the first library path follows the progname or the xml flag */
	so_lib = argv + 1 + xml;

	do {
		pool = get_test_pool(*so_lib, &lib_handle);
		if (NULL == pool) {
			fprintf(stderr, "No valid Fautes test pool in %s\n",
					*so_lib);
			continue;
		}
		suite = pool->suites;

		/* TODO extract / split what follows */
		/* initialize the CUnit test registry */
		cu_err = CU_initialize_registry();
		if (CUE_SUCCESS != cu_err) {
			fprintf(stderr, "CU_initialize_registry %s\n",
					CU_get_error_msg());
			dlclose(lib_handle);
			return CU_get_error();
		}

		/* register all tests suites */
		do {
			if ((*suite)->active) {
				cu_err = suite_register(*suite);
				if (CUE_SUCCESS != cu_err) {
					fprintf(stderr,
						"CU_initialize_registry %s\n",
						CU_get_error_msg());
					dlclose(lib_handle);
					return CU_get_error();
				}
			} else {
				printf("WARNING suite %s inactive\n",
						(*suite)->name);
			}
		} while (*(++suite));

		if (xml) {
			/* Run all tests using the automated interface */
			/* generates an xml output */
			CU_set_output_filename(pool->name);
			CU_automated_run_tests();
			CU_list_tests_to_file();
			tests_failed += CU_get_number_of_tests_failed();
		} else {
			/* Run all tests using the CUnit Basic interface */
			/* results are echoed to standard output */
			CU_basic_set_mode(CU_BRM_VERBOSE);
			CU_basic_run_tests();
			tests_failed += CU_get_number_of_tests_failed();
		}

		CU_cleanup_registry();

		dlclose(lib_handle);
	} while (*(++so_lib));

	failure = CU_get_error() != CUE_SUCCESS || tests_failed != 0;

	return failure ? EXIT_FAILURE : EXIT_SUCCESS;
}
