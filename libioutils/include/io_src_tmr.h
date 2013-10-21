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

#ifdef __cplusplus
extern "C" {
#endif

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
 * @param nbexpired Number of expirations of the timer
 */
typedef void (*io_tmr_cb_t)(struct io_src_tmr *tmr, uint64_t *nbexpired);

/**
 * @struct io_src_tmr
 * @brief Timer source type
 */
struct io_src_tmr {
	/** inner monitor source */
	struct io_src src;
	/** user callback, notified the timer expires */
	io_tmr_cb_t cb;
	/** 0 if the timer triggers only once, non-zero if it is periodic */
	int periodic;
};

/**
 * Initializes a relative timer.
 * @param tmr Timer source to initialize
 * @param cb User callback, notified the timer expires
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_tmr_init(struct io_src_tmr *tmr, io_tmr_cb_t cb);

/**
 * Arms (or disarms) the timer and sets it's relative timeout. By default, the
 * timer is one shot. Call io_src_tmr_set_periodic() before io_src_tmr_set(), to
 * make it periodic.
 * @param tmr Timer source to arm
 * @param timeout Timeout of the timer IO_SRC_TMR_DISARM for timeout to disarm
 * in ms
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_tmr_set(struct io_src_tmr *tmr, int timeout);

/* TODO implement get_source */

/**
 * Allows to choose if the timer is periodic or one shot. This will be taken
 * into account at the following call to io_src_tmr_set()
 * @param tmr Timer source to alter
 * @param periodic 1 for a periodic timer, 0 for a one shot timer
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_tmr_set_periodic(struct io_src_tmr *tmr, int periodic);

/**
 * Returns the underlying io_src of the timer source
 * @param tmr Timer source
 * @return io_src of the timer source
 */
static inline struct io_src *io_src_tmr_get_source(struct io_src_tmr *tmr)
{
	return NULL == tmr ? NULL : &tmr->src;
}

/**
 * Cleans up a timer source, by properly closing fd, zeroing fields etc...
 * @param tmr Timer source
 */
void io_src_tmr_clean(struct io_src_tmr *tmr);

#ifdef __cplusplus
}
#endif

#endif /* IO_SRC_TMR_H_ */
