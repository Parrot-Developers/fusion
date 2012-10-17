/**
 * @file rs_utils.h
 * @date 25 juil. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Common generic utilities
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef RS_UTILS_H_
#define RS_UTILS_H_

#include <sys/types.h>

/**
 * @def container_of
 * @brief Retrieves the address of a structure knowing the address of one of
 * it's members
 * @param ptr Member address
 * @param type Enclosing structure type
 * @param member Member name
 */
#define container_of(ptr, type, member) ({ \
	const typeof( ((type *)0)->member ) *__mptr = (ptr); \
	(type *)( (char *)__mptr - offsetof(type,member) ); \
})

/**
 * Liberation function for malloc-ed strings. Intended to be used with
 * __attribute__((cleanup( )))
 * @param p Pointer to the string to free
 */
void str_free(char **p);

#endif /* RS_UTILS_H_ */
