/**
 * @file io_src_sig.h
 * @date 22 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Signal source. Don't use. Signals are crap.
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef IO_SRC_SIG_H_
#define IO_SRC_SIG_H_
#include <sys/signalfd.h>

#include "io_src.h"

/**
 * @typedef io_src_sig_t
 * @brief Signal source type
 */
typedef struct io_src_sig io_src_sig_t;

/**
 * @typedef io_sig_callback_t
 * @brief Called when one of the monitored signals happens, with the
 * signalfd_siginfo field properly filled for use by the client
 * @param sig Signal source
 * @return errno compatible value, positive for only a warning, negative if the
 * source must be removed, 0 on success
 */
typedef int (io_sig_callback_t)(io_src_sig_t *sig);

/**
 * @typedef io_src_sig
 * @brief Signal source type
 */
struct io_src_sig {
	/** masks of the signals being monitored by this source */
	sigset_t mask;
	/** signal mask state before the source setup, for being reinstalled */
	sigset_t old_mask;
	/** user callback, notified when one of the registered signals occur */
	io_sig_callback_t *cb;
	/** inner monitor source */
	io_src_t src;
	/** signal info structure, filled in before calling back the client */
	struct signalfd_siginfo si;
};

/**
 * Initializes a signal source. For the proper operation of underlying code, the
 * signals being monitored are blocked. When the source is removed from the
 * monitor, the signals are restored to their state before io_src_sig_init
 * @param sig Signal source to initialize
 * @param cb User calback, notified when a monitored signal occur
 * @param nb Number of signals to mask
 * @return
 */
int io_src_sig_init(io_src_sig_t *sig, io_sig_callback_t *cb, unsigned nb, ...);

#endif /* IO_SRC_SIG_H_ */
