/**
 * @file rs_utils.c
 * @date 14 mai 2013
 * @author nicolas.carrier@parrot.com
 * @brief Common utility functions
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#ifndef RS_UTILS_H_
#define RS_UTILS_H_

/**
 * @def ARRAY_SIZE
 * @brief Computes the size of an array
 * @param _A Array
 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_A) (sizeof(_A) / sizeof((_A)[0]))
#endif /* ARRAY_SIZE */

/**
 * Frees a string and pass the corresponding pointer to NULL, useful with
 * __attribute__((cleanup(...)))
 * @param str String to free
 */
static inline void str_free(char **str)
{
	if (NULL == str || NULL == *str)
		return;

	free(*str);
	*str = NULL;
}

/**
 * Says if a string is invalid, i.e. if it is NULL or empty
 * @param str String to check the validity of
 * @return non-zero if the string is NULL or empty, 0 otherwise
 */
static inline int str_is_invalid(const char *str)
{
	return NULL == str || '\0' == *str;
}

#endif /* RS_UTILS_H_ */
