/**
 * @file io_fautes.h
 * @date 24 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef IO_FAUTES_H_
#define IO_FAUTES_H_

extern suite_t mon_suite;
extern suite_t src_msg_suite;
extern suite_t src_sep_suite;
extern suite_t src_sig_suite;
extern suite_t src_suite;
extern suite_t utils_suite;

void libioutils_init_test_suites(void);

#endif /* IO_FAUTES_H_ */
