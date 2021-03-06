/**
 * @file rs_utils.h
 * @date 14 mai 2013
 * @author nicolas.carrier@parrot.com
 * @brief Common utility functions
 * @note all the content of this header is deprecated, one must use ut_utils,
 * ut_string or ut_file from libutils
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#ifndef RS_UTILS_H_
#define RS_UTILS_H_

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* only used for RS_STRINGIFY implementation */
#define RS_STRINGIFY_HELPER(s) #s

/**
 * @def RS_STRINGIFY
 * @brief transforms it's argument to a valid string
 */
#define RS_STRINGIFY(s) RS_STRINGIFY_HELPER(s)

/**
 * @def rs_container_of
 * @brief Retrieves the address of a structure knowing the address of one of
 * it's members
 * @param ptr Member address
 * @param type Enclosing structure type
 * @param member Member name
 */
#ifndef rs_container_of
#define rs_container_of(ptr, type, member) ({ \
	const typeof(((type *)0)->member)*__mptr = (ptr); \
	(type *)((uintptr_t)__mptr - offsetof(type, member)); })
#endif /* rs_container_of */

/**
 * @def RS_ARRAY_SIZE
 * @brief Computes the size of an array
 * @param _A Array
 */
#ifndef RS_ARRAY_SIZE
#define RS_ARRAY_SIZE(_A) (sizeof(_A) / sizeof((_A)[0]))
#endif /* RS_ARRAY_SIZE */

/**
 * Frees a string and pass the corresponding pointer to NULL, useful with
 * __attribute__((cleanup(...)))
 * @param str String to free
 * @deprecated use ut_string_free from libutils
 */
__attribute__ ((deprecated("use ut_string_free from libutils")))
static inline void rs_str_free(char **str)
{
	if (NULL == str || NULL == *str)
		return;

	free(*str);
	*str = NULL;
}

/**
 * Closes a file and sets it to NULL
 * @param file File to close
 * @deprecated use ut_string_match_prefix from ut_file_close
 */
__attribute__ ((deprecated("use ut_string_match_prefix from ut_file_close")))
static inline void rs_file_close(FILE **file)
{
	if (NULL == file || NULL == *file)
		return;

	fclose(*file);
	*file = NULL;
}

/**
 * Says whether or not a string matches a given prefix
 * @param str String to test
 * @param prefix Prefix to match str with
 * @return non-zero if the string does match
 * @deprecated use ut_string_match_prefix from libutils
 */
__attribute__ ((deprecated("use ut_string_match_prefix from libutils")))
static inline int rs_str_match_prefix(const char *str, const char *prefix)
{
	if (NULL == str || NULL == prefix)
		return 0;

	return strncmp(str, prefix, strlen(prefix)) == 0;
}

/**
 * Says if a string is invalid, i.e. if it is NULL or empty
 * @param str String to check the validity of
 * @return true if the string is NULL or empty, 0 otherwise
 * @deprecated use ut_string_is_invalid from libutils
 */
__attribute__ ((deprecated("use ut_string_is_invalid from libutils")))
static inline bool rs_str_is_invalid(const char *str)
{
	return NULL == str || '\0' == *str;
}

/**
 * Suppresses the white spaces from the right of a string
 * @param str String to right strip
 * @deprecated use ut_string_rstrip from libutils
 */
__attribute__ ((deprecated("use ut_string_rstrip from libutils")));
static inline void rs_str_rstrip(char *str)
{
	size_t s;

	if (rs_str_is_invalid(str))
		return;

	s = strlen(str);
	while (s-- && isspace(str[s]))
		str[s] = '\0';
}

#ifdef __cplusplus
}
#endif

#endif /* RS_UTILS_H_ */
