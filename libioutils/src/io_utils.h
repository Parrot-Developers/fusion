/**
 * @file io_utils.h
 * @date 17 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef IO_UTILS_H_
#define IO_UTILS_H_

/**
 * Sets a file descriptor non-blocking
 * @param fd File descriptor
 * @return errno compatible negative value on error, 0 otherwise
 * @see fcntl
 */
int set_non_blocking(int fd);

#endif /* IO_UTILS_H_ */
