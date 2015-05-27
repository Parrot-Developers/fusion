/**
 * @file ut_file.c
 * @brief utilities for files manipulations
 *
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
 * @param string in output, contains the content of the file
 * @return errno compatible negative value on error, 0 on success
 */
static int do_file_to_string_zero_size(FILE *f, char **string)
{
#define BASE_SIZE 0x40
	size_t sret;
	size_t size = BASE_SIZE;

	while (true) {
		*string = realloc(*string, size + 1);
		if (*string == NULL)
			return -errno;

		sret = fread(*string + size - BASE_SIZE, 1, BASE_SIZE, f);
		if (sret != BASE_SIZE) {
			if (ferror(f)) {
				ut_string_free(string);
				return -EIO;
			}

			/* be sure the string is nul-terminated */
			size = size - BASE_SIZE + sret;
			(*string)[size] = '\0';

			break;
		}

		size += BASE_SIZE;
	}

	return 0;
#undef BASE_SIZE
}

/**
 * Actually reads a previously opened file and stores it into a string
 * @param f opened file stream
 * @param string in output, points to the newly allocated buffer containing the
 * content of the file
 * @return errno-compatible negative value on error, 0 on success
 */
static int do_file_to_string(FILE *f, char **string)
{
	int ret;
	long size;
	size_t sret;

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

	*string = calloc(size + 1, 1);
	if (*string == NULL) {
		ut_perr("calloc", errno);
		return -errno;
	}
	sret = fread(*string, size, 1, f);
	if (sret != 1) {
		ret = -errno;
		ut_string_free(string);
		ut_err("fread error");
		return ret;
	}

	return 0;
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

int ut_file_to_string(const char *path, char **string)
{
	FILE __attribute__((cleanup(ut_file_close)))*f = NULL;

	if (string == NULL)
		return -EINVAL;
	*string = NULL;
	if (ut_string_is_invalid(path))
		return -EINVAL;

	f = fopen(path, "rbe");
	if (f == NULL) {
		ut_perr("fopen", errno);
		return -errno;
	}

	return do_file_to_string(f, string);
}

int ut_file_write_buffer(const void *buffer, size_t size, const char *path)
{
	size_t sret;
	FILE __attribute__((cleanup(ut_file_close))) *file = NULL;

	file = fopen(path, "wbe");
	if (path == NULL)
		return -errno;

	sret = fwrite(buffer, size, 1, file);

	return sret != 1 ? -EIO : 0;
}

bool ut_file_is_executable(const char *path)
{
	struct stat st;
	int ret;

	if (ut_string_is_invalid(path))
		return false;

	memset(&st, 0, sizeof(st));
	ret = stat(path, &st);
	if (ret == -1)
		return false;

	if (!S_ISREG(st.st_mode))
		return false;

	return access(path, X_OK) == 0;
}
