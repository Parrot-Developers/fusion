/**
 * @file ut_string.h
 * @brief utilities for string manipulations
 *
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef UT_STRING_H_
#define UT_STRING_H_
#include <stdbool.h>

/**
 * Frees a string and passes the corresponding pointer to NULL, useful with
 * __attribute__((cleanup(...)))
 * @param str String to free, points to NULL in output
 */
void ut_string_free(char **str);

/**
 * Says if a string is invalid, i.e. if it is NULL or empty
 * @param str String to check the validity of
 * @return true if the string is NULL or empty, false otherwise
 */
bool ut_string_is_invalid(const char *str);

/**
 * Appends a string specified by a format, to another, reallocating the latter
 * @param str string to which the text will be appended. Can be NULL. If not
 * NULL, will be reallocated (thus the address is likely to change)
 * @param fmt a-la-printf string format specifier, followed by the matching
 * parameters
 * @return errno_compatible negative value on error, 0 on success
 */
int ut_string_append(char **str, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3)));

/**
 * Says whether or not a string matches a given prefix
 * @param str String to test, can be NULL
 * @param prefix Prefix to match str with, can be NULL
 * @return true if the string does match
 */
bool ut_string_match_prefix(const char *str, const char *prefix);

/**
 * Says whether or not, two strings are identical or not
 * @param str1 First string, can be NULL
 * @param str2 Second string, can be NULL
 * @return true if the strings are identical and not NULL does match
 */
bool ut_string_match(const char *str1, const char *str2);

/**
 * Suppresses the white spaces from the right of a string
 * @param str String to right strip
 * @return str
 */
char* ut_string_rstrip(char *str);

/**
 * Suppresses the white spaces from the left of a string
 * @note pay attention to the fact that the string returned can be a further
 * location in the original string, thus if it must be freed, the caller has the
 * responsibility to backup  it's original string pointer
 * @param str String to strip
 * @return The string stripped
 */
char *ut_string_lstrip(char *str);

/**
 * Suppresses the white spaces from the left and from the right of a string
 * @note pay attention to the fact that the string returned can be a further
 * location in the original string, thus if it must be freed, the caller has the
 * responsibility to backup it's original string pointer
 * @param str String to strip
 * @return The string stripped
 */
char *ut_string_strip(char *str);

#endif /* UT_STRING_H_ */
