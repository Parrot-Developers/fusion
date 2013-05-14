/******************************************************************************
* @file mb_timer.h
*
* @brief mambo timer
*
* Copyright (C) 2011 Parrot S.A.
*
* @author Jean-Baptiste Dubois
* @date May 2011
******************************************************************************/

#ifndef _MB_TIMER_H_
#define _MB_TIMER_H_

#include <stdint.h>

struct mb_timer;
typedef void (*mb_timer_cb_t) (struct mb_timer *timer, uint64_t *nbexpired,
		void *data);

/* timer */
struct mb_timer {
	struct mb_fd mbfd;
	struct mb_fd_loop *loop;
	int timeout;
	mb_timer_cb_t cb;
	void *data;
};

/* create timer */
int mb_timer_create(struct mb_timer *timer, struct mb_fd_loop *loop,
		    mb_timer_cb_t cb, void *data);

/* destroy timer */
int mb_timer_destroy(struct mb_timer *timer);

/* set relative timer timeout in ms */
int mb_timer_set(struct mb_timer *timer, int timeout);

/* clear timer */
int mb_timer_clear(struct mb_timer *timer);

#endif /*_MB_TIMER_H_*/
