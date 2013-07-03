/**
 * @file tests_common.c
 * @date Mar 22, 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2011 Parrot S.A.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* asprintf vasprintf */
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

#include <fautes_utils.h>

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
	if (-1 == ret)
		return -1;

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
	if (-1 == ret)
		return -1;

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
	if (-1 == ret)
		return -1;

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

	ret = read_from_output(tmp, PATH_MAX, "udevadm info --root");
	if (-1 == ret) {
		perror("read_from_output");
		return NULL;
	}
	tmp[strlen(tmp) - 1] = '/';
	ret = asprintf(&tun_path, "%snet/tun", tmp);
	if (-1 == ret) {
		fprintf(stderr, "memory allocation\n");
		return NULL;
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
