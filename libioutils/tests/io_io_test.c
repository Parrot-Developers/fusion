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
	ret = io_io_init(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 0);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use cases */
	ret = io_io_init(NULL, &mon, SUITE_NAME, sockets[0], sockets[0], 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_io_init(&io, NULL, SUITE_NAME, sockets[0], sockets[0], 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_io_init(&io, &mon, NULL, sockets[0], sockets[0], 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_io_init(&io, NULL, "", sockets[0], sockets[0], 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_io_init(&io, &mon, SUITE_NAME, -1, sockets[0], 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_io_init(&io, &mon, SUITE_NAME, sockets[0], -1, 0);
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
	ret = io_io_init(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 1);
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
	ret = io_io_init(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 0);
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
	ret = io_io_init(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 1);
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
	ret = io_io_init(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 1);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_io_read_start(&io, io_cb, (void *)42, 0);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
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
	/* initialization */
	fprintf(stderr, "%s STUBBED !!!\n", __func__);

	/* normal use cases */

	/* error use cases */

	/* cleanup */
}

static void testIO_ABORT(void)
{
	/* initialization */
	fprintf(stderr, "%s STUBBED !!!\n", __func__);

	/* normal use cases */

	/* error use cases */

	/* cleanup */
}

static void testIO_SIMPLE_USE_CASE(void)
{
#define MSG1 "first message"
#define ANS1 "first answer"
#define MSG2 "second message"
#define ANS2 "second answer"
#define BUF_SIZE 1024
#define STATE_START 0
#define STATE_MSG1_RECEIVED 1
#define STATE_MSG2_RECEIVED 2
#define STATE_ANS1_RECEIVED 4
#define STATE_ANS2_RECEIVED 8
#define STATE_ALL_DONE 15
	int state = STATE_START;
	int ret;
	struct timeval timeout;
	int sockets[2];
	struct io_mon mon;
	struct io_io io;
	struct io_src sock_src;
	int monfd;
	fd_set rfds;
	int count = 0;
	void io_buf_write_cb(struct io_io_write_buffer *buffer,
		enum io_io_write_status status)
	{
		count++;
		// stubbed !!! TODO
	}
	struct io_io_write_buffer io_buffers[2] = {
			[0] = {
				.node = {
						.next = NULL,
						.prev = NULL,
				},
				.cb = io_buf_write_cb,
				.data = (void *)666,
				.length = sizeof(MSG1),
				.address = MSG1,
			},
			[1] = {
				.node = {
						.next = NULL,
						.prev = NULL,
				},
				.cb = io_buf_write_cb,
				.data = (void *)421,
				.length = sizeof(MSG2),
				.address = MSG2,
			},
	};
	int io_cb(struct io_io *io, struct rs_rb *rb, size_t newbytes,
			void *data)
	{
		if (strncmp(rs_rb_get_read_ptr(rb), ANS1, sizeof(ANS1)) == 0) {
			CU_ASSERT_EQUAL(newbytes, sizeof(ANS1));
			CU_ASSERT(!(STATE_ANS1_RECEIVED & state));
			state |= STATE_ANS1_RECEIVED;
			rs_rb_read_incr(rb, newbytes);
		} else if (strncmp(rs_rb_get_read_ptr(rb), ANS2,
				sizeof(ANS2)) == 0) {
			CU_ASSERT_EQUAL(newbytes, sizeof(ANS2));
			CU_ASSERT(!(STATE_ANS2_RECEIVED & state));
			state |= STATE_ANS2_RECEIVED;
			rs_rb_read_incr(rb, newbytes);
		}

		return 0;
	}
	void sock_cb(struct io_src *src)
	{
		char buf[1024];
		ssize_t sret;

		sret = read(io_src_get_fd(src), buf, 1024);
		CU_ASSERT(sret > 0);

		if (STATE_START == state) {
			CU_ASSERT_STRING_EQUAL(buf, MSG1);
			sret = write(io_src_get_fd(src), ANS1, sizeof(ANS1));
			CU_ASSERT(sret > 0);
			CU_ASSERT(!(STATE_MSG1_RECEIVED & state));
			state |= STATE_MSG1_RECEIVED;
			CU_ASSERT_EQUAL(count, 1);
			ret = io_io_write_add(&io, io_buffers + 1);
			CU_ASSERT_EQUAL(ret, 0);
		} else {
			CU_ASSERT_STRING_EQUAL(buf, MSG2);
			sret = write(io_src_get_fd(src), ANS2, sizeof(ANS2));
			CU_ASSERT(sret > 0);
			CU_ASSERT(!(STATE_MSG2_RECEIVED & state));
			state |= STATE_MSG2_RECEIVED;
		}
	}

	/* initialization */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
			sockets);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_src_init(&sock_src, sockets[1], IO_IN, sock_cb);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_mon_add_source(&mon, &sock_src);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_io_init(&io, &mon, SUITE_NAME, sockets[0], sockets[0], 1);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_io_read_start(&io, io_cb, (void *)42, 0);
	CU_ASSERT_EQUAL(ret, 0);

	/* simple test case, send two messages and read two */
	ret = io_io_write_add(&io, io_buffers + 0);
	CU_ASSERT_EQUAL(ret, 0);

	monfd = io_mon_get_fd(&mon);
	while (STATE_ALL_DONE != state) {
		/* restore the timer */
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		FD_ZERO(&rfds);
		FD_SET(monfd, &rfds);

		ret = select(monfd + 1, &rfds, NULL, NULL, &timeout);
		/* error, not normal */
		CU_ASSERT_NOT_EQUAL(ret, -1);
		if (-1 == ret)
			goto out;

		/* timeout, not normal */
		CU_ASSERT_NOT_EQUAL(ret, 0);
		if (0 == ret)
			goto out;
		ret = io_mon_process_events(&mon);
		CU_ASSERT_EQUAL(ret, 0);
		if (0 != ret)
			goto out;
	}

out:
	CU_ASSERT(STATE_MSG1_RECEIVED & state);
	CU_ASSERT(STATE_MSG2_RECEIVED & state);
	CU_ASSERT(STATE_ANS1_RECEIVED & state);
	CU_ASSERT(STATE_ANS2_RECEIVED & state);
	CU_ASSERT(ret > -1);
	CU_ASSERT_EQUAL(count, 2);

	/* cleanup */
	io_io_read_stop(&io);
	io_mon_remove_source(&mon, &sock_src);
	io_src_clean(&sock_src);
	io_io_destroy(&io);
	io_mon_clean(&mon);
	io_close(sockets + 0);
	io_close(sockets + 1);
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
		{
				.fn = testIO_SIMPLE_USE_CASE,
				.name = "io_simple_use_case"
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

