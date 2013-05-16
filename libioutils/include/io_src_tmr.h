/**
 * @file io_tmr.h
 *
 * @brief Timer io source, imported and adapted from mambo
 *
 * Copyright (C) 2011 Parrot S.A.
 *
 * @author Jean-Baptiste Dubois
 * @author nicolas.carrier@parrot.com
 * @date May 2011
 */

#ifndef IO_SRC_TMR_H_
#define IO_SRC_TMR_H_

#include <stdint.h>

#include <io_src.h>

/**
 * @def IO_SRC_TMR_DISARM
 * @brief timeout value to disarm a timer
 */
#define IO_SRC_TMR_DISARM 0

/**
 * @struct io_tmr_src
 * @brief Timer source type
 */
struct io_src_tmr;

/**
 * @typedef io_pid_cb_t
 * @brief Called when the timer has expired
 * @param tmr Timer source
 * @param nbexpired Number of expirations of the timer. Always 0 or 1, since the
 * timer is forced one shot
 */
typedef void (*io_tmr_cb_t) (struct io_src_tmr *tmr, uint64_t *nbexpired);

/**
 * @struct io_src_tmr
 * @brief Timer source type
 */
struct io_src_tmr {
	/** inner monitor source */
	struct io_src src;
	/** user callback, notified the timer expires */
	io_tmr_cb_t cb;
};

/**
 * Initializes a relative one shot timer.
 * @param tmr Timer source to initialize
 * @param cb User callback, notified the timer expires
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_tmr_init(struct io_src_tmr *tmr, io_tmr_cb_t cb);

/*  */
/**
 * Arms (or disarms) the timer and sets it's relative timeout
 * @param tmr Timer source to arm
 * @param timeout Timeout of the timer IO_SRC_TMR_DISARM for timeout to disarm
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_tmr_set(struct io_src_tmr *tmr, int timeout);

/**
 * Cleans up a timer source, by properly closing fd, zeroing fields etc...
 * @param tmr Timer source
 */
void io_src_tmr_clean(struct io_src_tmr *tmr);

#endif /* IO_SRC_TMR_H_ */
