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

#include <CUnit/Automated.h>
#include <CUnit/Basic.h>

#include "tests_common.h"

/**
 * @brief tests suites
 */
extern suite_t mon_suite;
extern suite_t src_suite;
extern suite_t utils_suite;

suite_t *suites[] = {
		&mon_suite,
		&src_suite,
		&utils_suite,

		NULL, /* NULL guard */
};

int main(int argc, char *argv[])
{
	int basic = 0;
	int defaut_active_state = 1;
	CU_ErrorCode cu_err = CUE_SUCCESS;
	suite_t **suite = &(suites[0]);

	/*
	 * choose which tests to run
	 *   to activate just one, set defaut_active_state to 0 and put a ! in
	 *   front of the relevant active state value.
	 */
	  mon_suite.active = defaut_active_state;
	  src_suite.active = defaut_active_state;
	utils_suite.active = defaut_active_state;

	if (argc >= 2) {
		if (0 == strcmp(argv[1], "basic"))
			basic = 1;

		else {
			printf("usage %s [basic]\n\tIf basic is passed, result "
					"of the tests will be echoed to "
					"standard output, otherwise they will "
					"be written to "
					"CUnitAutomated-Results.xml\n",
					argv[0]);
			return 0;
		}
	}

	/* initialize the CUnit test registry */
	cu_err = CU_initialize_registry();
	if (CUE_SUCCESS != cu_err) {
		fprintf(stderr, "CU_initialize_registry %s\n",
				CU_get_error_msg());
		return CU_get_error();
	}

	/* register all tests suites */
	do {
		if ((*suite)->active) {
			cu_err = suite_register(*suite);
			if (CUE_SUCCESS != cu_err) {
				fprintf(stderr, "CU_initialize_registry %s\n",
						CU_get_error_msg());
				return CU_get_error();
			}
		} else {
			printf("WARNING suite %s inactive\n", (*suite)->name);
		}
	} while (*(++suite));

	if (basic) {
		/* Run all tests using the CUnit Basic interface */
		CU_basic_set_mode(CU_BRM_VERBOSE);
		CU_basic_run_tests();
	} else {
		/* Run all tests using the automated interface */
		CU_automated_run_tests();
		CU_list_tests_to_file();
	}

	CU_cleanup_registry();

	/* to clean up valgrind's output */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	return CU_get_error();
}
