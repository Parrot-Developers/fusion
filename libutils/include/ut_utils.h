/**
 * @file ut_utils.h
 * @brief
 *
 * @date 7 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef UT_UTILS_H_
#define UT_UTILS_H_

/* only used for UT_STRINGIFY implementation */
#define UT_STRINGIFY_HELPER(s) #s

/**
 * @def UT_STRINGIFY
 * @brief transforms it's argument to a valid string
 */
#define UT_STRINGIFY(s) UT_STRINGIFY_HELPER(s)

/**
 * @def ut_container_of
 * @brief Retrieves the address of a structure knowing the address of one of
 * it's members
 * @param ptr Member address
 * @param type Enclosing structure type
 * @param member Member name
 */
#ifndef ut_container_of
#define ut_container_of(ptr, type, member) ({ \
	const typeof(((type *)0)->member)*__mptr = (ptr); \
	(type *)((uintptr_t)__mptr - offsetof(type, member)); })
#endif /* ut_container_of */

/**
 * @def UT_ARRAY_SIZE
 * @brief Computes the size of an array
 * @param _A Array
 */
#ifndef UT_ARRAY_SIZE
#define UT_ARRAY_SIZE(_A) (sizeof(_A) / sizeof((_A)[0]))
#endif /* UT_ARRAY_SIZE */

#endif /* UT_UTILS_H_ */
