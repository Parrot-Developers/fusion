/**
 * @file rs_fautes.h
 * @date 24 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef RS_FAUTES_H_
#define RS_FAUTES_H_

extern struct suite_t dll_suite;
extern struct suite_t hmap_suite;
extern struct suite_t node_suite;
extern struct suite_t rb_suite;

/**
 * Entry point of the library when the .so in executed directly.
 */
void librs_tests(void);

#endif /* RS_FAUTES_H_ */
