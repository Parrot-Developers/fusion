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
 * @typedef io_src_sig
 * @brief Signal source type
 */
struct io_src_sig;

/**
 * @typedef io_sig_cb_t
 * @brief Called when one of the monitored signals happens, with the
 * signalfd_siginfo field properly filled for use by the client
 * @param sig Signal source
 */
typedef void (io_sig_cb_t)(struct io_src_sig *sig);

/**
 * @typedef io_src_sig
 * @brief Signal source type
 */
struct io_src_sig {
	/** inner monitor source */
	struct io_src src;
	/** masks of the signals being monitored by this source */
	sigset_t mask;
	/** signal mask state before the source setup, for being reinstalled */
	sigset_t old_mask;
	/** user callback, notified when one of the registered signals occur */
	io_sig_cb_t *cb;
	/* TODO add a cleanup user callback ? */
	/** signal info structure, filled in before calling back the client */
	struct signalfd_siginfo si;
};

/**
 * Initializes a signal source. For the proper operation of underlying code, the
 * signals being monitored are blocked. When the source is removed from the
 * monitor, the signals are restored to their state before io_src_sig_init
 * @param sig Signal source to initialize
 * @param cb User calback, notified when a monitored signal occur
 * @param ... List of signal, terminated by NULL, which corresponds to 0 or null
 * signal
 * @return errno compatible negative value
 */
int io_src_sig_init(struct io_src_sig *sig, io_sig_cb_t *cb, ...)
	__attribute__ ((sentinel(0)));

/**
 * Returns the underlying io_src of the signal source
 * @param sig Signal source
 * @return io_src of the signal source
 */
static inline struct io_src *io_src_sig_get_source(struct io_src_sig *sig)
{
	return NULL == sig ? NULL : &(sig->src);
}

#endif /* IO_SRC_SIG_H_ */
