/**
 * @file io_test.c
 * @date 27 sept 2013
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for th io module
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <sys/socket.h>

#include <CUnit/Basic.h>

#include <fautes.h>
#include <fautes_utils.h>

#include <io_mon.h>
#include <io_io.h>
#include <io_utils.h>

#define SUITE_NAME "io_suite"

static void testIO_CREATE(void)
{
	int ret;
	int sockets[2];
	struct io_mon mon;
	struct io_io io;

	/* initialization */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
			sockets);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	ret = io_io_create(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 0);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use cases */
	ret = io_io_create(NULL, &mon, SUITE_NAME, sockets[0], sockets[0], 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_io_create(&io, NULL, SUITE_NAME, sockets[0], sockets[0], 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_io_create(&io, &mon, NULL, sockets[0], sockets[0], 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_io_create(&io, NULL, "", sockets[0], sockets[0], 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_io_create(&io, &mon, SUITE_NAME, -1, sockets[0], 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_io_create(&io, &mon, SUITE_NAME, sockets[0], -1, 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	io_io_destroy(&io);
	io_close(sockets + 0);
	io_close(sockets + 1);
	io_mon_clean(&mon);
}

static void testIO_DESTROY(void)
{
	int ret;
	int sockets[2];
	struct io_mon mon;
	struct io_io io;

	/* initialization */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
			sockets);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_io_create(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 0);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	ret = io_io_destroy(&io);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use cases */
	ret = io_io_destroy(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	io_mon_clean(&mon);
	io_close(sockets + 0);
	io_close(sockets + 1);
}

static void testIO_READ_START(void)
{
	int ret;
	int sockets[2];
	struct io_mon mon;
	struct io_io io;
	int io_cb(struct io_io *io, struct rs_rb *rb, size_t newbytes,
			void *data)
	{

		return 0;
	}
	/* initialization */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
			sockets);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_io_create(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 0);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	ret = io_io_read_start(&io, io_cb, (void *)42, 0);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(io_io_read_state(&io), IO_IO_STARTED);

	/* error use cases */
	ret = io_io_read_start(NULL, io_cb, (void *)42, 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_io_read_start(&io, NULL, (void *)42, 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	io_io_destroy(&io);
	io_mon_clean(&mon);
	io_close(sockets + 0);
	io_close(sockets + 1);
}

static void testIO_LOG_RX(void)
{
	/* initialization */
	fprintf(stderr, "%s STUBBED !!!\n", __func__);

	/* normal use cases */

	/* error use cases */

	/* cleanup */
}

static void testIO_LOG_TX(void)
{
	/* initialization */
	fprintf(stderr, "%s STUBBED !!!\n", __func__);

	/* normal use cases */

	/* error use cases */

	/* cleanup */
}

static void testIO_READ_STOP(void)
{
	int ret;
	int sockets[2];
	struct io_mon mon;
	struct io_io io;
	int io_cb(struct io_io *io, struct rs_rb *rb, size_t newbytes,
			void *data)
	{

		return 0;
	}
	/* initialization */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
			sockets);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_io_create(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 0);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_io_read_start(&io, io_cb, (void *)42, 0);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = io_io_read_stop(&io);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(io_io_read_state(&io), IO_IO_STOPPED);

	/* error use cases */
	ret = io_io_read_stop(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	io_io_destroy(&io);
	io_mon_clean(&mon);
	io_close(sockets + 0);
	io_close(sockets + 1);
}

static void testIO_READ_STATE(void)
{
	int ret;
	int sockets[2];
	struct io_mon mon;
	struct io_io io;
	enum io_io_state state;
	int io_cb(struct io_io *io, struct rs_rb *rb, size_t newbytes,
			void *data)
	{

		return 0;
	}
	/* initialization */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
			sockets);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_io_create(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 0);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	ret = io_io_read_start(&io, io_cb, (void *)42, 0);
	CU_ASSERT_EQUAL(ret, 0);
	state = io_io_read_state(&io);
	CU_ASSERT_EQUAL(state, IO_IO_STARTED);
	ret = io_io_read_stop(&io);
	CU_ASSERT_EQUAL(ret, 0);
	state = io_io_read_state(&io);
	CU_ASSERT_EQUAL(state, IO_IO_STOPPED);

	/* error use cases */
	state = io_io_read_state(NULL);
	CU_ASSERT_EQUAL(state, IO_IO_ERROR);

	/* cleanup */
	io_io_destroy(&io);
	io_mon_clean(&mon);
	io_close(sockets + 0);
	io_close(sockets + 1);
}

static void testIO_WRITE_ADD(void)
{
	int ret;
	int sockets[2];
	struct io_mon mon;
	struct io_io io;
	enum io_io_state state;
	int io_cb(struct io_io *io, struct rs_rb *rb, size_t newbytes,
			void *data)
	{

		return 0;
	}
	/* initialization */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
			sockets);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_io_create(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 0);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	ret = io_io_read_start(&io, io_cb, (void *)42, 0);
	CU_ASSERT_EQUAL(ret, 0);
	state = io_io_read_state(&io);
	CU_ASSERT_EQUAL(state, IO_IO_STARTED);
	ret = io_io_read_stop(&io);
	CU_ASSERT_EQUAL(ret, 0);
	state = io_io_read_state(&io);
	CU_ASSERT_EQUAL(state, IO_IO_STOPPED);

	/* error use cases */
	state = io_io_read_state(NULL);
	CU_ASSERT_EQUAL(state, IO_IO_ERROR);

	/* cleanup */
	io_io_destroy(&io);
	io_mon_clean(&mon);
	io_close(sockets + 0);
	io_close(sockets + 1);
}

static void testIO_ABORT(void)
{
	/* initialization */
	fprintf(stderr, "%s STUBBED !!!\n", __func__);

	/* normal use cases */

	/* error use cases */

	/* cleanup */
}

static const struct test_t tests[] = {
		{
				.fn = testIO_CREATE,
				.name = "io_io_create"
		},
		{
				.fn = testIO_DESTROY,
				.name = "io_io_destroy"
		},
		{
				.fn = testIO_READ_START,
				.name = "io_io_read_start"
		},
		{
				.fn = testIO_LOG_RX,
				.name = "io_io_log_rx"
		},
		{
				.fn = testIO_LOG_TX,
				.name = "io_io_log_tx"
		},
		{
				.fn = testIO_READ_STOP,
				.name = "io_io_read_stop"
		},
		{
				.fn = testIO_READ_STATE,
				.name = "io_io_read_state"
		},
		{
				.fn = testIO_WRITE_ADD,
				.name = "io_io_write_add"
		},
		{
				.fn = testIO_ABORT,
				.name = "io_io_write_abort"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_io_suite(void)
{
	return 0; /* return non-zero on error */
}

static int clean_io_suite(void)
{
	return 0; /* return non-zero on error */
}

struct suite_t io_suite = {
		.name = SUITE_NAME,
		.init = init_io_suite,
		.clean = clean_io_suite,
		.tests = tests,
};

