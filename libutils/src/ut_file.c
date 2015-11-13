/**
 * @file ut_file.c
 * @brief utilities for files manipulations
 *
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "ut_file.h"
#include "ut_string.h"
#include "ut_log.h"

/**
 * Reads the file size of an opened file stream
 * @param f file stream previously opened
 */
static long get_file_size(FILE *f)
{
	int ret;
	long size;

	if (f == NULL)
		return -EINVAL;

	ret = fseek(f, 0, SEEK_END);
	if (ret != 0) {
		ut_perr("fseek SEEK_END", errno);
		return -errno;
	}
	size = ftell(f);
	if (size == -1) {
		ut_perr("fopen", errno);
		return -errno;

	}
	ret = fseek(f, 0, SEEK_SET);
	if (ret != 0) {
		ut_perr("fseek SEEK_CUR", errno);
		return -errno;
	}

	return size;
}

/**
 * Read the content of a file, which reports a size of 0
 * @param f File to read
 * @param string in output, contains the content of the file, left untouched on
 * error
 * @return errno compatible negative value on error, 0 on success
 */
static int do_file_to_string_zero_size(FILE *f, char **string)
{
#define BASE_SIZE 0x40
	size_t sret;
	size_t size = BASE_SIZE;
	char *s = NULL;

	while (true) {
		s = realloc(s, size + 1);
		if (s == NULL)
			return -errno;

		sret = fread(s + size - BASE_SIZE, 1, BASE_SIZE, f);
		if (sret != BASE_SIZE) {
			if (ferror(f)) {
				ut_string_free(string);
				ut_string_free(&s);
				return -EIO;
			}

			/* be sure the string is nul-terminated */
			size = size - BASE_SIZE + sret;
			s[size] = '\0';

			break;
		}

		size += BASE_SIZE;
	}

	*string = s;

	return 0;
#undef BASE_SIZE
}

/**
 * Actually reads a previously opened file and stores it into a string
 * @param f opened file stream
 * @param string in output, points to the newly allocated buffer containing the
 * content of the file, left untouched on error.
 * @return errno-compatible negative value on error, 0 on success
 */
static int do_file_to_string(FILE *f, char **string)
{
	int ret;
	long size;
	size_t sret;
	char *s;

	size = get_file_size(f);
	if (size == -1) {
		ut_perr("get_file_size", size);
		return -errno;
	}
	/*
	 * size is NULL, it can happen (e.g. /proc/modules), we can't allocate
	 * the definitive size at once, so we use a different strategy
	 */
	if (size == 0)
		return do_file_to_string_zero_size(f, string);

	s = calloc(size + 1, 1);
	if (s == NULL) {
		ut_perr("calloc", errno);
		return -errno;
	}
	sret = fread(s, size, 1, f);
	if (sret != 1 && ferror(f)) {
		ret = -errno;
		ut_string_free(&s);
		ut_err("fread error");
		return ret;
	}
	*string = s;

	return 0;
}

/**
 * Reads a file and stores it's content to a string
 * @param path Path of the file to read.
 * @param string In input, must point to a valid storage for a char *, which, in
 * output, will be filled with an address of a newly allocated string, the user
 * has to free after usage. Left untouched on error.
 * @return errno compatible negative value on error, 0 on success
 */
static int file_to_string(const char *path, char **string)
{
	FILE __attribute__((cleanup(ut_file_close)))*f = NULL;

	if (ut_string_is_invalid(path))
		return -EINVAL;

	f = fopen(path, "rbe");
	if (f == NULL) {
		ut_perr("fopen", errno);
		return -errno;
	}

	return do_file_to_string(f, string);
}

void ut_file_close(FILE **file)
{
	if (file == NULL || *file == NULL)
		return;

	fclose(*file);
	*file = NULL;
}

int ut_file_fd_close(int *fd)
{
	int ret;

	if (NULL == fd || -1 == *fd)
		return -EBADF;

	/*
	 * Although 0 is a perfectly valid file descriptor, we forbid closing it
	 * because allowing close(0) can hide some subtle bugs. The aim is to
	 * fail early when these bugs occur.
	 * If one really want to close 0, he must use close explicitly.
	 */
	if (*fd == 0)
		return -EBADF;

	ret = close(*fd);
	if (ret == -1)
		return errno;

	*fd = -1;

	return 0;
}

long ut_file_get_file_size(const char *path)
{
	FILE __attribute__((cleanup(ut_file_close)))*f = NULL;

	if (ut_string_is_invalid(path))
		return -EINVAL;

	f = fopen(path, "rbe");
	if (f == NULL) {
		ut_perr("fopen", errno);
		return -errno;
	}

	return get_file_size(f);
}

int ut_file_to_string(const char *fmt, char **string, ...)
{
	int ret;
	va_list args;
	char __attribute__((cleanup(ut_string_free)))*path = NULL;

	if (string == NULL)
		return -EINVAL;
	*string = NULL;
	if (ut_string_is_invalid(fmt))
		return -EINVAL;

	va_start(args, string);
	ret = vasprintf(&path, fmt, args);
	va_end(args);
	if (ret == -1) {
		path = NULL;
		return -ENOMEM;
	}

	return file_to_string(path, string);
}

int ut_file_write_buffer(const void *buffer, size_t size, const char *path)
{
	size_t sret;
	FILE __attribute__((cleanup(ut_file_close))) *file = NULL;

	file = fopen(path, "wbe");
	if (file == NULL)
		return -errno;

	sret = fwrite(buffer, size, 1, file);

	return sret != 1 ? -EIO : 0;
}

/**
 * Perform access(2) like test on a file
 * @param path path of the file to test
 * @param how how flag passed to access
 * @return true iif the test is successful
 */
static bool test_file(const char *path, int how, unsigned type)
{
	struct stat st;
	int ret;

	if (ut_string_is_invalid(path))
		return false;

	memset(&st, 0, sizeof(st));
	ret = stat(path, &st);
	if (ret == -1)
		return false;

	if (!((st.st_mode & S_IFMT) == type))
		return false;

	return access(path, how) == 0;
}

bool ut_file_is_executable(const char *path)
{
	return test_file(path, X_OK, S_IFREG);
}

bool ut_file_exists(const char *path)
{
	return test_file(path, F_OK, S_IFREG);
}

bool ut_file_is_dir(const char *path)
{
	return test_file(path, F_OK, S_IFDIR);
}
