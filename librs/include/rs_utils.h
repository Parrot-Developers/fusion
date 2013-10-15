/**
 * @file rs_utils.h
 * @date 14 mai 2013
 * @author nicolas.carrier@parrot.com
 * @brief Common utility functions
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#ifndef RS_UTILS_H_
#define RS_UTILS_H_

#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

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
	(type *)((char *)__mptr - offsetof(type, member)); })
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
 */
static inline void rs_str_free(char **str)
{
	if (NULL == str || NULL == *str)
		return;

	free(*str);
	*str = NULL;
}

static inline void rs_file_close(FILE **file)
{
	if (NULL == file || NULL == *file)
		return;

	fclose(*file);
	*file = NULL;
}

/**
 * @def RS_STRINGIFY
 * @brief transforms it's argument to a valid string
 */
#define RS_STRINGIFY(s) RS_STRINGIFY_HELPER(s)

/* only used for RS_STRINGIFY implementation */
#define RS_STRINGIFY_HELPER(s) #s

/**
 * Says if a string is invalid, i.e. if it is NULL or empty
 * @param str String to check the validity of
 * @return non-zero if the string is NULL or empty, 0 otherwise
 */
static inline int rs_str_is_invalid(const char *str)
{
	return NULL == str || '\0' == *str;
}

#ifdef __cplusplus
}
#endif

#endif /* RS_UTILS_H_ */
