/**
 * @file ut_file_test.c
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @brief unit tests for libutils' file module
 *
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <CUnit/Basic.h>

#include "../include/ut_file.h"
#include "../include/ut_string.h"

#include <fautes.h>
#include <fautes_utils.h>

static void testUT_FILE_CLOSE(void)
{
	FILE *file = fopen("/dev/null", "rb");

	CU_ASSERT_PTR_NOT_NULL_FATAL(file);

	/* normal use case */
	ut_file_close(&file);
	CU_ASSERT_PTR_NULL(file);
	ut_file_close(&file);

	/* error use cases */
	ut_file_close(NULL);
}

static void testUT_FILE_FD_CLOSE(void)
{
	int ret;
	int fd = open("/dev/null", O_CLOEXEC | O_WRONLY);

	CU_ASSERT_NOT_EQUAL_FATAL(fd, -1);

	/* normal use case */
	ret = ut_file_fd_close(&fd);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(fd, -1);

	/* error use cases */
	ret = ut_file_fd_close(&fd);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = ut_file_fd_close(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	fd = 0;
	ret = ut_file_fd_close(&fd);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testUT_FILE_GET_SIZE(void)
{
#define MSG "blablablabla"
#define PATH "/tmp/titi_tata_toto_ut_file_test"
	long ret;

	system("echo -n "MSG" > "PATH);

	/* normal use case */
	ret = ut_file_get_file_size(PATH);
	CU_ASSERT_EQUAL(ret, strlen(MSG));

	/* cleanup */
	system("rm "PATH);

	/* error use cases */
	ret = ut_file_get_file_size("");
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = ut_file_get_file_size(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = ut_file_get_file_size("hopefullynonexistantfilepath...ssasasasa");
	CU_ASSERT_NOT_EQUAL(ret, 0);
#undef PATH
#undef MSG
}

static void testUT_FILE_TO_STRING(void)
{
#define MSG "blablablabla pim pam poum"
#define PATH "/tmp/titi_tata_toto_ut_file_to_string_test"
	int ret;
	char *string = NULL;

	system("echo -n "MSG" > "PATH);

	/* normal use case */
	ret = ut_file_to_string(PATH, &string);
	CU_ASSERT_STRING_EQUAL(string, MSG);

	/* cleanup */
	system("rm "PATH);
	ut_string_free(&string);
	ret = ut_file_to_string("/proc/modules", &string);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_PTR_NOT_NULL(string);
	ut_string_free(&string);

	/* error use cases */
	ret = ut_file_to_string(NULL, &string);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	CU_ASSERT_PTR_NULL(string);
	ret = ut_file_to_string(PATH, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = ut_file_to_string("hopefullynonexistantfilepath.sasasa", &string);
	CU_ASSERT_NOT_EQUAL(ret, 0);
#undef PATH
#undef MSG
}

static void testUT_FILE_IS_EXECUTABLE(void)
{
	CU_ASSERT(ut_file_is_executable("/bin/sh"));
	CU_ASSERT(!ut_file_is_executable(""));
	CU_ASSERT(!ut_file_is_executable("/bin"));
	CU_ASSERT(!ut_file_is_executable(NULL));
}

static void testUT_FILE_IS_DIR(void)
{
	CU_ASSERT(ut_file_is_dir("/bin"));
	CU_ASSERT(!ut_file_is_dir("/bin/sh"));
	CU_ASSERT(!ut_file_is_dir(""));
	CU_ASSERT(!ut_file_is_dir(NULL));
}

static void testUT_FILE_MKDIR(void)
{
#define testUT_FILE_MKDIR_fmt "/tmp/dir_format_%s"
	int ret;

	ret = ut_file_mkdir(testUT_FILE_MKDIR_fmt, 644, "%s");
	CU_ASSERT(ret == 0);

	rmdir(testUT_FILE_MKDIR_fmt);
#undef testUT_FILE_MKDIR_fmt

	ret = ut_file_mkdir(NULL, 00);
	CU_ASSERT(ret != 0);
}

static const struct test_t tests[] = {
		{
				.fn = testUT_FILE_CLOSE,
				.name = "ut_file_close"
		},
		{
				.fn = testUT_FILE_FD_CLOSE,
				.name = "ut_file_fd_close"
		},
		{
				.fn = testUT_FILE_GET_SIZE,
				.name = "ut_file_get_size"
		},
		{
				.fn = testUT_FILE_TO_STRING,
				.name = "ut_file_to_string"
		},
		{
				.fn = testUT_FILE_IS_EXECUTABLE,
				.name = "ut_file_is_executable"
		},
		{
				.fn = testUT_FILE_IS_DIR,
				.name = "ut_file_is_dir"
		},
		{
				.fn = testUT_FILE_MKDIR,
				.name = "ut_file_mkdir"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_file_suite(void)
{
	return 0;
}

static int clean_file_suite(void)
{

	return 0;
}

struct suite_t file_suite = {
		.name = "ut_file",
		.init = init_file_suite,
		.clean = clean_file_suite,
		.tests = tests,
};
