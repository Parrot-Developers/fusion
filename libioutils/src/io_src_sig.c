/**
 * @file io_src_sig.c
 * @date 22 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Signal source. Don't use. Signals are crap.
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <signal.h>
#include <unistd.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "io_mon.h"
#include "io_src_sig.h"

/**
 * @def to_src
 * @brief Convert a source to it's signal source container
 */
#define to_src_sig(p) container_of(p, struct io_src_sig, src)

/**
 * Source callback, reads the signal and notifies the client
 * @param src Underlying monitor source of the signal source
 */
static int sig_cb(struct io_src *src)
{
	ssize_t ret;
	struct io_src_sig *sig = to_src_sig(src);

	if (io_mon_has_error(src->events))
		return -EIO;

	ret = read(src->fd, &(sig->si), sizeof(sig->si));
	if (sizeof(sig->si) != ret)
		return -errno;

	return sig->cb(sig);
}

/**
 * Callback called when the source is removed
 * @param src Underlying monitor source of the signal source
 */
static void sig_cleanup(struct io_src *src)
{
	struct io_src_sig *sig;

	if (NULL == src)
		return;
	sig = to_src_sig(src);

	close(sig->src.fd);
	/* restore gently the signal mask */
	sigprocmask(SIG_SETMASK, &(sig->old_mask), NULL);
	memset(src, 0, sizeof(*src));
	src->fd = -1;
}

int io_src_sig_init(struct io_src_sig *sig, io_sig_callback_t *cb, unsigned nb,
		...)
{
	int ret;
	int fd;
	va_list args;
	sigset_t *m;

	if (NULL == sig || NULL == cb || 0 == nb)
		return -EINVAL;
	m = &(sig->mask);

	memset(sig, 0, sizeof(*sig));

	/* add all the signals to the mask */
	sigemptyset(m);
	va_start(args, nb);
	while (nb--) {
		ret = sigaddset(m, va_arg(args, int));
		if (-1 == ret) {
			ret = -errno;
			goto out;
		}
	}
	va_end(args);

	/*
	 * block signals so that they aren't handled according to their default
	 * dispositions
	 */
	ret = sigprocmask(SIG_BLOCK, m, &(sig->old_mask));
	if (-1 == ret)
		return -errno;

	/* set up signal fd */
	fd = signalfd(-1, m, SFD_NONBLOCK | SFD_CLOEXEC);
	if (-1 == fd) {
		ret = -errno;
		goto out;
	}

	sig->cb = cb;

	/* can fail only on parameters */
	return io_src_init(&(sig->src), fd, IO_IN, sig_cb, sig_cleanup);
out:
	sig_cleanup(&(sig->src));

	return ret;
}
