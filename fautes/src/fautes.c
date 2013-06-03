/**
 * @file test.c
 * @date Mar 21, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests for netbox internal modules
 *
 * Copyright (C) 2011 Parrot S.A.
 */
#include <unistd.h>
#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
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
CU_ErrorCode suite_register(struct suite_t *suite)
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

typedef void (*init_fun_t)(void);

static struct suite_t **get_test_suite(const char *so_lib, void **lib_handle,
		char **libname)
{
	void *sym;
	struct suite_t **res = NULL;
	int ret;
	char names_buf[512];
	init_fun_t init_fun;

	/* load the library to test */
	*lib_handle = dlopen(so_lib, RTLD_NOW);
	if (NULL == *lib_handle) {
		fprintf(stderr, "%s\n", dlerror());
		return NULL;
	}

	/* get it's name */
	ret = get_sym(*lib_handle, "fautes_lib_name", &sym);
	if (-1 == ret)
		goto out;
	*libname = *(char **)sym;
	if (0 != strncmp(*libname, so_lib, strlen(*libname))) {
		fprintf(stderr, "Mismatch between so name '%s' and "
				"fautes_lib_name '%s' \n", so_lib, *libname);
		goto out;
	}

	/* get it's test suite */
	snprintf(names_buf, 512, "%s_test_suites", *libname);
	ret = get_sym(*lib_handle, names_buf, &sym);
	if (-1 == ret)
		goto out;
	res = (struct suite_t **)sym;

	/* if there is a suites initialization function, call it */
	snprintf(names_buf, 512, "%s_init_test_suites", *libname);
	ret = get_sym(*lib_handle, names_buf, &sym);
	if (-1 == ret)
		goto out;
	init_fun = (init_fun_t)sym;
	if (NULL != init_fun)
		init_fun();

	printf("Found test suite for library %s\n", *libname);
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
	char *libname;
	unsigned tests_failed = 0;
	int failure;

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
		suite = get_test_suite(*so_lib, &lib_handle, &libname);
		if (NULL == suite) {
			fprintf(stderr, "Shared object %s does not contain a "
					"valid Fautes test suite\n", *so_lib);
			continue;
		}

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
			CU_set_output_filename(libname);
			CU_automated_run_tests();
			CU_list_tests_to_file();
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


	/* to clean up valgrind's output */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	failure = CU_get_error() != CUE_SUCCESS || tests_failed != 0;

	return failure ? EXIT_FAILURE : EXIT_SUCCESS;
}
