/**
 * @file ut_process.h
 * @brief utilities for executing processes
 *
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef UT_PROCESS_H_
#define UT_PROCESS_H_

/**
 * Variadic version of the system() function
 * @see man system
 * @param command
 * @return -1 in case of allocation error or return code of the system function
 * (in normal case, returns a wait() status code)
 */
int ut_process_vsystem(const char *command, ...)
__attribute__ ((format (printf, 1, 2)));

#endif /* UT_PROCESS_H_ */
