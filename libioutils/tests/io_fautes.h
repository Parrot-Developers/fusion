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

extern struct suite_t io_suite;
extern struct suite_t mon_suite;
extern struct suite_t process_suite;
extern struct suite_t src_inot_suite;
extern struct suite_t src_msg_suite;
extern struct suite_t src_msg_uad_suite;
extern struct suite_t src_pid_suite;
extern struct suite_t src_sep_suite;
extern struct suite_t src_sig_suite;
extern struct suite_t src_suite;
extern struct suite_t src_tmr_suite;
extern struct suite_t utils_suite;

/**
 * Entry point of the library when the .so in executed directly.
 */
void libioutils_tests(void);

#endif /* IO_FAUTES_H_ */
