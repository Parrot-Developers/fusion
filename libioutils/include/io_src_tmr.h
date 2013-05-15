/**
 * @file io_tmr.h
 *
 * @brief mambo timer
 *
 * Copyright (C) 2011 Parrot S.A.
 *
 * @author Jean-Baptiste Dubois
 * @date May 2011
 */

#ifndef IO_SRC_TMR_H_
#define IO_SRC_TMR_H_

#include <stdint.h>

#include <io_src.h>

/* usefull time ratio value */
#define MSEC_PER_SEC  1000
#define NSEC_PER_SEC  1000000000

struct io_timer;
typedef void (*io_timer_cb_t) (struct io_timer *timer, uint64_t *nbexpired,
		void *data);

/* timer */
struct io_timer {
	struct io_src src;
	int timeout;
	io_timer_cb_t cb;
	void *data;
};

/* create timer */
int io_timer_create(struct io_timer *timer, io_timer_cb_t cb, void *data);

/* destroy timer */
int io_timer_destroy(struct io_timer *timer);

/* set relative timer timeout in ms */
int io_timer_set(struct io_timer *timer, int timeout);

/* clear timer */
int io_timer_clear(struct io_timer *timer);

#endif /* IO_SRC_TMR_H_ */
