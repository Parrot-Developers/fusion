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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dlfcn.h>

#include <fautes.h>
#include <fautes_utils.h>

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

	printf("Found test suite %s for %s\n", res->name, so_lib);
out:
	if (NULL == res)
		dlclose(lib_handle);

	return res;
}

int main(int argc, char *argv[])
{
	int ret;
	bool xml = 0;
	char **so_lib;
	void *lib_handle;
	int tests_failed = 0;
	bool failure;
	struct pool_t *pool;

	if (argc <= 1) {
		usage(argv[0]);
		return 1;
	}
	if (0 == strcmp(argv[1], "-h") || 0 == strcmp(argv[1], "--help")) {
		usage(argv[0]);
		return 0;
	}

	xml = 0 == strcmp(argv[1], "xml") || fautes_generate_xml();
	/* the first library path follows the progname or the xml flag */
	so_lib = argv + 1 + xml;

	do {
		pool = get_test_pool(*so_lib, &lib_handle);
		if (NULL == pool) {
			fprintf(stderr, "No valid Fautes test pool in %s\n",
					*so_lib);
			continue;
		}
		ret = fautes_run_test_pool(pool, xml);
		if (ret < 0) {
			dlclose(lib_handle);
			break;
		}
		if (ret > 0)
			tests_failed += ret;

		dlclose(lib_handle);
	} while (*(++so_lib));

	failure = CU_get_error() != CUE_SUCCESS || tests_failed != 0;

	return failure ? EXIT_FAILURE : EXIT_SUCCESS;
}
