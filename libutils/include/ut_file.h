/**
 * @file ut_file.h
 * @brief utilities for files manipulations
 *
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef UT_FILE_H_
#define UT_FILE_H_
#include <stdbool.h>
#include <stdio.h>

/**
 * Closes a file and sets it to NULL
 * @param file File to close
 */
void ut_file_close(FILE **file);

/**
 * Closes a file descriptor and sets it to NULL
 * @param fd pointer to the file descriptor to close, set to -1 in output
 * @note this this function refuses to close the 0 file descriptor to avoid some
 * nasty bugs
 * @return errno-compatible negative value on error, 0 on success
 */
int ut_file_fd_close(int *fd);

/**
 * Gets the size of a file, knowing it's path
 * @param path errno-compatible negative value on error, the file's size on
 * success
 * @return size of the file. Be aware that some files can legitimately report a
 * size of 0, even if they have content available as, e.g., /proc/modules
 */
long ut_file_get_file_size(const char *path);

/**
 * Reads the content of a file and store it in a suitably allocated buffer
 * @param path path of the file to open
 * @param string in input, must be a valid pointer, in output, points to the
 * allocated string read, which must be freed after usage with either free() or
 * ut_string_free(). Set to NULL on error.
 * @return errno-compatible negative value on error, 0 on success
 */
int ut_file_to_string(const char *path, char **string);

/**
 * Writes the content of a buffer to a file.
 * @param buffer Buffer to write
 * @param size Number of bytes from buffer, to write to the file
 * @param path Path of the file to write into
 * @return 0 on success, -EIO on write error, -errno code from fopen on any
 * other error.
 */
int ut_file_write_buffer(const void *buffer, size_t size, const char *path);

/**
 * Tests whether or not a path exists as a file and is executable
 * @param path Path to test
 * @return true iif the given path is executable
 */
bool ut_file_is_executable(const char *path);

/**
 * Tests whether or not a path exists as a file
 * @param path Path to test
 * @return true iif the given path is a file and exists
 */
bool ut_file_exists(const char *path);

/**
 * Tests whether or not a path exists as a directory
 * @param path Path to test
 * @return true iif the given path is a directory and exists
 */
bool ut_file_is_dir(const char *path);

#endif /* UT_FILE_H_ */
