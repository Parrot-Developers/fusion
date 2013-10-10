/**
 * @file fautes_utils.h
 * @date Mar 22, 2012
 * @author nicolas.carrier@parrot.com
 * @brief Common definitions for unit tests
 *
 * Copyright (C) 2011 Parrot S.A.
 */

#ifndef FAUTES_UTILS_H_
#define FAUTES_UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Launches a shell command and read it's output.
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
 * @param fmt A-la-printf format, matching the ellipsis parameters, to name the
 * command to launch
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

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif /* MIN */
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif /* MAX */

/**
 * Executes a function echoing something to a file descriptor and stores it
 * @param string String where the output is stored
 * @param len Size of the buffer, including the room for a trailing 0 which is
 * automatically added
 * @param out_func Function to test the output of, parameters from the ellipsis
 * are passed to it
 * @param filedes File descriptor out_func writes to
 * @return 0 if everything went successful, 0 otherwise
 */
#define store_function_output_to_string(string, len, out_func, filedes, ...) \
({ \
	int __old_out_fd = -1; \
	int __pipe[2] = {-1, -1}; \
	int __ret; \
\
__old_out_fd = dup(filedes); \
	if (-1 == __old_out_fd) { \
		__ret = -errno; \
		perror("dup"); \
	} else { \
		__ret = pipe(__pipe); \
		if (-1 == __ret) { \
			__ret = -errno; \
			perror("pipe"); \
		} else { \
			__ret = dup2(__pipe[1], filedes); \
			if (-1 == __ret) { \
				__ret = -errno; \
				perror("dup2"); \
			} else { \
				(out_func)(__VA_ARGS__); \
				__ret = read(__pipe[0], (string), (len)); \
				if (-1 == __ret) { \
					__ret = -errno; \
					perror("read"); \
				} else { \
					(string)[MIN(__ret, (int)(len) - 1)] =\
					'\0';\
					__ret = 0; \
				} \
			} \
		} \
	} \
	dup2(__old_out_fd, filedes); \
	if (-1 != __pipe[0]) \
		close(__pipe[0]); \
	if (-1 != __pipe[1]) \
		close(__pipe[1]); \
	if (-1 != __old_out_fd) \
		close(__old_out_fd); \
\
	/* "return" value */ \
	__ret; \
})

/**
 * Executes a function echoing something to standard output and stores it
 * @param string String where the output is stored
 * @param len Size of the buffer, including the room for a trailing 0 which is
 * automatically added
 * @param out_func Function to test the output of, parameters from the ellipsis
 * are passed to it
 * @return 0 if everything went successful, 0 otherwise
 */
#define store_function_stdoutput_to_string(string, len, out_func, ...) \
	store_function_output_to_string(string, len, out_func, STDOUT_FILENO, \
			__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* FAUTES_UTILS_H_ */
