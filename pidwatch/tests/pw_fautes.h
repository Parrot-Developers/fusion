/**
 * @file pw_fautes.h
 * @date 29 apr. 2013
 * @author nicolas.carrier@parrot.com
 * @brief Automated unit tests libpidwatch. Definitions for Fautes support
 *
 * Copyright (C) 2013 Parrot S.A.
 */

#ifndef PW_FAUTES_H_
#define PW_FAUTES_H_

extern struct suite_t pidwatch_suite;

/**
 * Entry point of the library when the .so in executed directly.
 */
void libpidwatch_tests(void);

#endif /* PW_FAUTES_H_ */
