/**
 * @file fautes_utils.c
 * @date Mar 22, 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2011 Parrot S.A.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

#include <CUnit/Automated.h>
#include <CUnit/Basic.h>

#include <fautes_utils.h>

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

int read_from_output(char *buf, size_t size, const char *cmd)
{
	FILE *f = NULL;
	int ret = -1;
	size_t bytes_read;

	if (0 == size)
		return -1;

	/*
	 * Note : An "e" should be added to the modes, but our toolchains are
	 * too old and this test code isn't sufficiently critical, so re-coding
	 * popen to support cloexec doesn't worth it
	 */
	f = popen(cmd, "r");
	if (NULL == f) {
		fprintf(stderr, "popen '%s' failed\n", cmd);
		goto out;
	}

	memset(buf, 0, size);
	bytes_read = fread(buf, 1, size - 1, f);
	/*
	 * implicit nul termination due to memset isn't sufficient for coverity
	 * what's more, explicit is better than implicit...
	 */
	buf[size - 1] = '\0';
	if (ferror(f)) {
		fprintf(stderr, "error reading '%s' output\n", cmd);
		goto out;
	}

	ret = (int)bytes_read; /* possible but improbable overflow */
out:
	if (f)
		pclose(f);

	return ret;
}

static void str_free(char **str)
{
	if (NULL == str || NULL == *str)
		return;

	free(*str);
	*str = NULL;
}

int vread_from_output(char *buf, size_t size, const char *fmt, ...)
{
	int ret = -1;
	char __attribute__((cleanup(str_free))) *cmd = NULL;
	va_list args;

	va_start(args, fmt);
	ret = vasprintf(&cmd, fmt, args);
	va_end(args);
	if (-1 == ret) {
		cmd = NULL;
		return -1;
	}

	return read_from_output(buf, size, cmd);
}

int write_to_input(char *buf, size_t size, const char *cmd)
{
	FILE *f = NULL;
	int ret = -1;

	/*
	 * Note : An "e" should be added to the modes, but our toolchains are
	 * too old and this test code isn't sufficiently critical, so re-coding
	 * popen to support cloexec doesn't worth it
	 */
	f = popen(cmd, "w");
	if (NULL == f) {
		fprintf(stderr, "popen '%s' failed\n", cmd);
		goto out;
	}

	fwrite(buf, 1, size, f);
	if (ferror(f)) {
		fprintf(stderr, "error writing to '%s'\n", cmd);
		goto out;
	}

	ret = 0;
out:
	if (f)
		pclose(f);

	return ret;
}

int vwrite_to_input(char *buf, size_t size, const char *fmt, ...)
{
	int ret = -1;
	char __attribute__((cleanup(str_free))) *cmd = NULL;
	va_list args;

	va_start(args, fmt);
	ret = vasprintf(&cmd, fmt, args);
	va_end(args);
	if (-1 == ret) {
		cmd = NULL;
		return -1;
	}

	return write_to_input(buf, size, cmd);
}

int vsystem(const char *fmt, ...)
{
	int ret = -1;
	char __attribute__((cleanup(str_free))) *cmd = NULL;
	va_list args;

	va_start(args, fmt);
	ret = vasprintf(&cmd, fmt, args);
	va_end(args);
	if (-1 == ret) {
		cmd = NULL;
		return -1;
	}

	return system(cmd);
}

bool cmd_is_available(const char *cmd)
{
	int res = vsystem("which %s > /dev/null", cmd);
	if (-1 == res)
		return false;

	return (0 == WEXITSTATUS(res));
}

char *get_tun_path(void)
{
	int ret = -1;
	char *tmp __attribute__((cleanup(str_free))) = calloc(PATH_MAX, 1);
	char *tun_path = NULL;

	if (NULL == tmp)
		return NULL;

	ret = read_from_output(tmp, PATH_MAX,
			"udevadm info --root 2> /dev/null");
	if (-1 == ret) {
		perror("read_from_output");
		return NULL;
	}

	/* it the previous command failed, try a reasonable default value */
	if (strlen(tmp) == 0)
		strcpy(tmp, "/dev/");

	tmp[strlen(tmp) - 1] = '/';
	ret = asprintf(&tun_path, "%snet/tun", tmp);
	if (-1 == ret) {
		tmp = NULL;
		fprintf(stderr, "memory allocation\n");
		return NULL;
	}

	if (0 != access(tun_path, R_OK | W_OK)) {
		str_free(&tun_path);
		ret = asprintf(&tun_path, "/dev/net/tun");
		if (-1 == ret) {
			fprintf(stderr, "memory allocation\n");
			return NULL;
		}
	}

	return tun_path;
}

int compare_string_to_file(char *path, char *buf, size_t size)
{
	int ret = -1;
	int fd = -1;
	struct stat st;
	void *mem = NULL;
	int result = 0;

	if (NULL == path || '\0' == path[0] || NULL == buf)
		goto out;

	if (0 == size) {
		result = 1;
		goto out;
	}

	fd = open(path, O_CLOEXEC);
	if (-1 == fd) {
		perror("open");
		goto out;
	}
	ret = fstat(fd, &st);
	if (-1 == ret) {
		perror("fstat");
		goto out;
	}

	if ((long long)st.st_size != (long long)size)
		goto out;

	mem = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (MAP_FAILED == mem) {
		perror("mmap");
		goto out;
	}

	result = memcmp(mem, buf, size) == 0;

out:
	if (NULL != mem)
		munmap(mem, size);
	if (-1 != fd)
		close(fd);

	return result;
}

int fautes_run_test_pool(struct pool_t *pool, bool xml)
{
	CU_ErrorCode cu_err = CUE_SUCCESS;
	struct suite_t **suite;
	int tests_failed = 0;

	if (pool->initializer)
		pool->initializer();

	suite = pool->suites;

	cu_err = CU_initialize_registry();
	if (CUE_SUCCESS != cu_err) {
		fprintf(stderr, "CU_initialize_registry %s\n",
				CU_get_error_msg());
		return -CU_get_error();
	}

	/* register all tests suites */
	do {
		if ((*suite)->active) {
			cu_err = suite_register(*suite);
			if (CUE_SUCCESS != cu_err) {
				fprintf(stderr, "CU_initialize_registry %s\n",
						CU_get_error_msg());
				return -CU_get_error();
			}
		} else {
			printf("WARNING suite %s inactive\n", (*suite)->name);
		}
	} while (*(++suite));

	if (xml) {
		CU_set_output_filename(pool->name);
		CU_automated_run_tests();
		CU_list_tests_to_file();
	} else {
		CU_basic_set_mode(CU_BRM_VERBOSE);
		CU_basic_run_tests();
	}
	tests_failed += CU_get_number_of_tests_failed();

	CU_cleanup_registry();

	return tests_failed;
}
