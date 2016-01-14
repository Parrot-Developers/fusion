/**
 * @file ut_process.c
 * @brief utilities for executing processes
 *
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <sys/prctl.h>

#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "ut_string.h"
#include "ut_process.h"
#include "ut_log.h"
#include "ut_file.h"
#include "ut_utils.h"

/**
 * Launches a shell command and retrieve it's output in a user supplied buffer
 * @param buf Buffer of at least size size in which the command's output will be
 * stored
 * @param size amont of data to read from the command's output
 * @param cmd command to run
 * @return errno negative value on error, 0 on success
 */
static int do_read_from_output(char *buf, size_t size, const char *cmd)
{
	FILE *f = NULL;
	int ret;
	size_t bytes_read;

	if (size == 0)
		return -EINVAL;

	/*
	 * Note : An "e" should be added to the modes, but our toolchains are
	 * too old and this test code isn't sufficiently critical, so re-coding
	 * popen to support cloexec doesn't worth it
	 */
	f = popen(cmd, "r");
	if (f == NULL)
		return -errno;

	memset(buf, 0, size);
	bytes_read = fread(buf, 1, size - 1, f);
	/*
	 * implicit nul termination due to memset isn't sufficient for coverity
	 * what's more, explicit is better than implicit...
	 */
	buf[size - 1] = '\0';
	if (ferror(f)) {
		ret = -EIO;
		goto out;
	}

	ret = 0;
out:
	pclose(f);

	return ret;
}

int ut_process_vsystem(const char *fmt, ...)
{
	int ret = -1;
	char __attribute__((cleanup(ut_string_free))) *cmd = NULL;
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

FILE *ut_process_vpopen(const char *fmt, const char *type, ...)
{
	int ret = -1;
	char __attribute__((cleanup(ut_string_free))) *cmd = NULL;
	va_list args;

	va_start(args, type);
	ret = vasprintf(&cmd, fmt, args);
	va_end(args);
	if (-1 == ret) {
		cmd = NULL;
		errno = ENOMEM;
		return NULL;
	}

	return popen(cmd, type);
}

int ut_process_change_name(const char *fmt, ...)
{
	int ret;
	char __attribute__((cleanup(ut_string_free))) *new_name = NULL;
	va_list args;

	va_start(args, fmt);
	ret = vasprintf(&new_name, fmt, args);
	va_end(args);
	if (-1 == ret) {
		new_name = NULL;
		return -ENOMEM;
	}
	ret = prctl(PR_SET_NAME, (unsigned long)new_name, 0, 0, 0);
	if (ret < 0)
		return -errno;

	return 0;
}

char *ut_process_get_name(char name[17])
{
	int ret;

	memset(name, 0, 17);
	ret = prctl(PR_GET_NAME, (unsigned long)name, 0, 0, 0);
	name[16] = 0;

	return ret == 0 ? name : NULL;
}

bool ut_process_is_being_ptraced(void)
{
	char __attribute__((cleanup(ut_string_free))) *line = NULL;
	FILE __attribute__((cleanup(ut_file_close))) *fp = NULL;
	ssize_t n;
	size_t len = 0;
	const char tracer_needle[] = "TracerPid:";

	fp = fopen("/proc/self/status", "re");
	if (fp == NULL)
		return false;

	while (true) {
		n = getline(&line, &len, fp);
		if (n <= 0)
			return false;

		if (strstr(line, tracer_needle) != NULL)
			return atoi(line + UT_ARRAY_SIZE(tracer_needle)) != 0;
	}

	return false; /* in theory, never reached */
}

int ut_process_read_from_output(char **buf, size_t size, const char *fmt, ...)
{
	int ret;
	char __attribute__((cleanup(ut_string_free))) *cmd = NULL;
	va_list args;

	if (buf == NULL || *buf == NULL || ut_string_is_invalid(fmt))
		return -EINVAL;

	va_start(args, fmt);
	ret = vasprintf(&cmd, fmt, args);
	va_end(args);
	if (ret == -1) {
		cmd = NULL;
		return -ENOMEM;
	}

	/*
	 * TODO implementation accepting NULL for *buf, allocating the buffer
	 * to fit what the process outputs
	 */
	return do_read_from_output(*buf, size, cmd);
}

int ut_process_sync_init(struct ut_process_sync *sync, bool cloexec)
{
	int ret;
	int flags = cloexec ? O_CLOEXEC : 0;

	if (sync == NULL)
		return -EINVAL;

	ret = pipe2(sync->pico, flags);
	if (ret < 0) {
		ret = -errno;
		goto err;
	}
	ret = pipe2(sync->poci, flags);
	if (ret < 0) {
		ret = -errno;
		goto err;
	}

	return 0;
err:
	ut_process_sync_clean(sync);

	return ret;
}

int ut_process_sync_child_lock(struct ut_process_sync *sync)
{
	ssize_t sret;
	char c;

	if (sync == NULL)
		return -EINVAL;

	sret = TEMP_FAILURE_RETRY(read(sync->poci[0], &c, 1));

	return sret == -1 ? -errno : 0;
}

int ut_process_sync_child_unlock(struct ut_process_sync *sync)
{
	ssize_t sret;
	char c = 0;

	if (sync == NULL)
		return -EINVAL;

	sret = TEMP_FAILURE_RETRY(write(sync->pico[1], &c, 1));

	return sret == -1 ? -errno : 0;
}

int ut_process_sync_parent_lock(struct ut_process_sync *sync)
{
	ssize_t sret;
	char c;

	if (sync == NULL)
		return -EINVAL;

	sret = TEMP_FAILURE_RETRY(read(sync->pico[0], &c, 1));

	return sret == -1 ? -errno : 0;
}

int ut_process_sync_parent_unlock(struct ut_process_sync *sync)
{
	ssize_t sret;
	char c = 0;

	if (sync == NULL)
		return -EINVAL;

	sret = TEMP_FAILURE_RETRY(write(sync->poci[1], &c, 1));

	return sret == -1 ? -errno : 0;
}

void ut_process_sync_clean(struct ut_process_sync *sync)
{
	if (sync == NULL)
		return;

	ut_file_fd_close(sync->pico + 0);
	ut_file_fd_close(sync->pico + 1);
	ut_file_fd_close(sync->poci + 0);
	ut_file_fd_close(sync->poci + 1);
}
