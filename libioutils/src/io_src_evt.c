/**
 * @file io_src_evt.c
 * @date 22 jan. 2016
 * @author nicolas.carrier@parrot.com
 * @brief Source for event fds
 *
 * Copyright (C) 2016 Parrot S.A.
 */
#include <sys/eventfd.h>

#include <stdlib.h>
#include <errno.h>

#include <io_src_evt.h>
#include <io_utils.h>

#include <ut_utils.h>

/**
 * Callback called on events notified by the event fd
 * @param src Underlying io_src of the eventfd source
 */
static void src_evt_wrapper_cb(struct io_src *src)
{
	uint64_t value;
	int ret;

	struct io_src_evt *evt = ut_container_of(src, struct io_src_evt, src);

	ret = io_read(evt->src.fd, &value, sizeof(value));
	if (ret == -1)
		value = UINT64_MAX;

	evt->cb(evt, value);
}

int io_src_evt_init(struct io_src_evt *evt, io_src_evt_cb *cb,
		bool semaphore, int initval)
{
	int flags = EFD_CLOEXEC | EFD_NONBLOCK;
	int fd;

	if (evt == NULL || cb == NULL)
		return -EINVAL;

	if (semaphore)
		flags |= EFD_SEMAPHORE;

	fd = eventfd(initval, flags);
	if (fd == -1)
		return -errno;

	evt->cb = cb;

	return io_src_init(&evt->src, fd, IO_IN, src_evt_wrapper_cb);
}

int io_src_evt_notify(struct io_src_evt *evt, uint64_t value)
{
	if (evt == NULL)
		return -EINVAL;

	return io_write(evt->src.fd, &value, sizeof(value));
}

void io_src_evt_clean(struct io_src_evt *evt)
{
	if (evt == NULL)
		return;
	io_src_close_fd(&evt->src);
	evt = NULL;
	io_src_clean(&evt->src);
}
