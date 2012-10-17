/**
 * @file tests_common.h
 * @date Mar 22, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Common definitions for unit tests
 *
 * Copyright (C) 2011 Parrot S.A.
 */

#ifndef TESTS_COMMON_H_
#define TESTS_COMMON_H_

#include <stdbool.h>

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
	const char *name;
	CU_InitializeFunc init;
	CU_CleanupFunc clean;
	const test_t *tests;
	int active;
} suite_t;

/**
 * @brief tests suites
 */
extern suite_t dhcpc_suite;
extern suite_t dhcps_suite;
extern suite_t dll_suite;
extern suite_t ifconfig_suite;
extern suite_t ifevent_suite;
extern suite_t nat_suite;
extern suite_t netbox_suite;
extern suite_t node_suite;
extern suite_t resolver_suite;
extern suite_t route_suite;
extern suite_t sysctl_suite;

/**
 * Registers a test suite and all it's tests to the CUnit framework
 * @param suite Suite of unit tests
 * @return Error code of the first error to happen in either CU_add_suite
 * or CU_add_test
 */
CU_ErrorCode suite_register(suite_t *suite);

/**
 * Launches a command and read it's output.
 * @param buf Buffer where the output will be stored
 * @param size Maximum size of data to read
 * @param cmd Command to launch
 * @return -1 in case of error, otherwise number of bytes read
 */
int read_from_output(char *buf, size_t size, const char *cmd);

/**
 * Launches a command and read it's output.
 * Same as above but variadic
 * @param buf Buffer where the output will be stored
 * @param size Maximum size of data to read
 * @param cmd Command to launch
 * @return -1 in case of error, otherwise number of bytes read
 */
int vread_from_output(char *buf, size_t size, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

/**
 * Launches a command and write the content of a buffer to it's standard input.
 * @param buf Buffer where the input string is stored
 * @param size Size of data to write
 * @param cmd Command to launch
 * @return -1 in case of error, otherwise 0
 */
int write_to_input(char *buf, size_t size, const char *cmd);

/**
 * Launches a command and write the content of a buffer to it's standard input.
 * Same as above but variadic
 * @param buf Buffer where the input string is stored
 * @param size Size of data to write
 * @param cmd Command to launch
 * @return -1 in case of error, otherwise 0
 */
int vwrite_to_input(char *buf, size_t size, const char *cmd, ...)
__attribute__ ((format (printf, 3, 4)));

/**
 * Variadic version of system function
 * @see man system
 * @param command
 * @return -1 in case of allocation error or return code of the system function
 */
int vsystem(const char *command, ...)
__attribute__ ((format (printf, 1, 2)));

/**
 * Check the availability of a system command.
 * Intended to check for external tools required by the tests, when initializing
 * a test suite.
 * @param cmd the command to be checked
 * @return true if the command is available, false otherwise
 */
bool cmd_is_available(const char *cmd);

/**
 * Retrieves tun special file's path, as created by udev
 * @return NULL on error, path if found, must be freed when not used anymore
 */
char *get_tun_path(void);

/**
 * Compares the content of a file to a string
 * @param path Path of the file to compare
 * @param buf Buffer containing the string to compare the file's content to
 * @param size Size of the string, not including the terminating '\0' (i.e. the
 * result of strlen)
 * @return 1 if file's content is identical to the buffer's content, 0 otherwise
 * or if an error occurs
 */
int compare_string_to_file(char *path, char *buf, size_t size);

#endif /* TESTS_COMMON_H_ */
