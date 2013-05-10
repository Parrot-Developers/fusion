/**
 * @file io_src_msg_test.c
 * @date 22 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for fixed length message sources
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#include <unistd.h>
#include <fcntl.h>

#include <limits.h>
#include <stdbool.h>
#include <signal.h>
#include <stddef.h>

#include <CUnit/Basic.h>

#include <io_mon.h>
#include <io_src_msg.h>

#include <fautes.h>
#include <fautes_utils.h>

static void reached_state(int *glob_state, int state)
{
	*glob_state |= state;
}

struct msg {
	char a;
	int b;
	double c;
};

struct my_msg_src {
	struct msg msg;
	struct io_src_msg msg_src;
	int pipefds[2];
};

#define to_src_my_msg_src(p) container_of(p, struct my_msg_src, msg_src)

static void my_msg_src_clean(struct my_msg_src *my_src)
{
	/*
	 * don't close the source fd, it will be closed by the io_src_msg_clean
	 */
	if (!io_src_has_in(my_src->msg_src.src.type))
		close(my_src->pipefds[0]);
	if (!io_src_has_out(my_src->msg_src.src.type))
		close(my_src->pipefds[1]);

	memset(&(my_src->msg), 0, sizeof(my_src->msg));

	io_src_msg_clean(&(my_src->msg_src));
	my_src->pipefds[0] = my_src->pipefds[1] = -1;
}

static const struct msg MSG1 = {11, 11111, 11.111};
static const struct msg MSG2 = {22, 22222, 22.222};
static const struct msg MSG3 = {33, 33333, 33.333};
static const struct msg MSG4 = {44, 44444, 44.444};

#define STATE_START 0
#define STATE_MSG1_RECEIVED 1
#define STATE_MSG2_RECEIVED 2
#define STATE_MSG3_RECEIVED 4
#define STATE_MSG4_RECEIVED 8
#define STATE_ALL_DONE 15
static int state;

static void testSRC_MSG_SET_NEXT_MESSAGE(void)
{
	int ret;
	char msg[1024];
	struct io_src_msg src;

	ret = io_src_msg_set_next_message(&src, &msg);
	CU_ASSERT_NOT_EQUAL(ret, -1);
	CU_ASSERT_PTR_EQUAL(src.send_buf, &msg);

	/* error use cases */
	ret = io_src_msg_set_next_message(NULL, &msg);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_set_next_message(&src, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

/*
 * message callback for the read tests, checks the messages received correspond
 * to what is expected ant sends the next one
 */
static void msg_cb_read(struct io_src_msg *src, enum io_src_event evt)
{
	int ret;
	void *msg;
	struct my_msg_src *my_src = to_src_my_msg_src(src);
/*		printf("received : \"%d %d %f\"\n", my_msg->a, my_msg->b,
			my_msg->c);*/

	CU_ASSERT_NOT_EQUAL(state, STATE_ALL_DONE);
	CU_ASSERT_EQUAL(evt, IO_IN);

	/*
	 * note : what follows isn't really necessary, one can directly use :
	 *     my_src->msg
	 * but it's cleaner for public API
	 */
	ret = io_src_msg_get_message(src, &msg);
	CU_ASSERT_EQUAL(ret, 0);

	if (0 == memcmp(msg, &MSG1, src->len)) {
		CU_ASSERT_EQUAL(state, STATE_START);
		reached_state(&state, STATE_MSG1_RECEIVED);

		ret = write(my_src->pipefds[1], &MSG2,
				sizeof(struct msg));
		CU_ASSERT_NOT_EQUAL(ret, -1);
	} else if (0 == memcmp(msg, &MSG2, src->len)) {
		CU_ASSERT_EQUAL(state, STATE_MSG1_RECEIVED);
		reached_state(&state, STATE_MSG2_RECEIVED);

		ret = write(my_src->pipefds[1], &MSG3,
				sizeof(struct msg));
		CU_ASSERT_NOT_EQUAL(ret, -1);
	} else if (0 == memcmp(msg, &MSG3, src->len)) {
		CU_ASSERT_EQUAL(state, STATE_MSG1_RECEIVED |
				STATE_MSG2_RECEIVED);
		reached_state(&state, STATE_MSG3_RECEIVED);

		ret = write(my_src->pipefds[1], &MSG4,
				sizeof(struct msg));
		CU_ASSERT_NOT_EQUAL(ret, -1);
	} else if (0 == memcmp(msg, &MSG4, src->len)) {
		CU_ASSERT_EQUAL(state, STATE_MSG1_RECEIVED |
				STATE_MSG2_RECEIVED |
				STATE_MSG3_RECEIVED);
		reached_state(&state, STATE_MSG4_RECEIVED);
	}
}

/* sends ourselves messages manually and check we receive them */
static void testSRC_MSG_INIT_read(void)
{
	fd_set rfds;
	int ret;
	struct io_mon mon;
	struct my_msg_src msg_src;
	bool loop = true;
	struct timeval timeout;

	state = STATE_START;

	ret = pipe(msg_src.pipefds);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src),
			msg_src.pipefds[0],
			IO_IN,
			msg_cb_read,
			&(msg_src.msg),
			sizeof(msg_src.msg),
			1);
	CU_ASSERT_EQUAL(ret, 0);

	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_mon_add_source(&mon, &(msg_src.msg_src.src));
	CU_ASSERT_EQUAL(ret, 0);

	ret = write(msg_src.pipefds[1], &MSG1, sizeof(msg_src.msg));
	CU_ASSERT_NOT_EQUAL(ret, -1);

	/* normal use case */
	while (loop) {
		/* restore the timer */
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		/* restore the read file descriptor set */
		FD_ZERO(&rfds);
		FD_SET(mon.epollfd, &rfds);
		ret = select(mon.epollfd + 1, &rfds, NULL, NULL, &timeout);

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

		loop = STATE_ALL_DONE != state;
	}

out:
	/* debriefing */
	CU_ASSERT(state & STATE_MSG1_RECEIVED);
	CU_ASSERT(state & STATE_MSG2_RECEIVED);
	CU_ASSERT(state & STATE_MSG3_RECEIVED);
	CU_ASSERT(state & STATE_MSG4_RECEIVED);

	/* cleanup */
	io_mon_clean(&mon);
	my_msg_src_clean(&(msg_src));

	/* error cases */
	ret = io_src_msg_init(NULL, msg_src.pipefds[0], IO_IN, msg_cb_read,
			&(msg_src.msg), sizeof(struct msg),
			1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src), -1, IO_IN, msg_cb_read,
			&(msg_src.msg), sizeof(struct msg),
			1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src), msg_src.pipefds[0], IO_IN,
			msg_cb_read, NULL,
			sizeof(struct msg), 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src), msg_src.pipefds[0], IO_IN,
			msg_cb_read, &(msg_src.msg), 0, 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src), msg_src.pipefds[0], IO_IN,
			NULL, &(msg_src.msg),
			sizeof(struct msg), 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src), msg_src.pipefds[0], 666,
			msg_cb_read, &(msg_src.msg),
			sizeof(struct msg), 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src), msg_src.pipefds[0], 0,
			msg_cb_read, &(msg_src.msg),
			sizeof(struct msg), 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void msg_cb_write(struct io_src_msg *src, enum io_src_event evt)
{
	CU_ASSERT_EQUAL(evt, IO_OUT);

	if (0 == (state & STATE_MSG1_RECEIVED))
		io_src_msg_set_next_message(src, &MSG1);
	else if (0 == (state & STATE_MSG2_RECEIVED))
		io_src_msg_set_next_message(src, &MSG2);
	else if (0 == (state & STATE_MSG3_RECEIVED))
		io_src_msg_set_next_message(src, &MSG3);
	else if (0 == (state & STATE_MSG4_RECEIVED))
		io_src_msg_set_next_message(src, &MSG4);
	else
		CU_ASSERT(0);
}

/* sends ourselves messages and check we receive them manually */
static void testSRC_MSG_INIT_write(void)
{
	fd_set rfds;
	int maxfd;
	int ret;
	struct io_mon mon;
	struct my_msg_src msg_src;
	struct msg rcvd_msg;
	bool loop = true;
	struct timeval timeout;

	state = STATE_START;

	ret = pipe(msg_src.pipefds);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src),
			msg_src.pipefds[1],
			IO_OUT,
			msg_cb_write,
			&(msg_src.msg),
			sizeof(msg_src.msg),
			1);
	CU_ASSERT_EQUAL(ret, 0);

	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_mon_add_source(&mon, &(msg_src.msg_src.src));
	CU_ASSERT_EQUAL(ret, 0);

	ret = io_mon_activate_out_source(&mon, &(msg_src.msg_src.src), 1);
	CU_ASSERT_EQUAL(ret, 0);

	maxfd = MAX(mon.epollfd, msg_src.pipefds[0]) + 1;
	/* normal use case */
	while (loop) {
		/* restore the timer */
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		/* restore the read file descriptor set */
		FD_ZERO(&rfds);
		FD_SET(mon.epollfd, &rfds);
		FD_SET(msg_src.pipefds[0], &rfds);
		ret = select(maxfd, &rfds, NULL, NULL, &timeout);

		/* error, not normal */
		CU_ASSERT_NOT_EQUAL(ret, -1);
		if (-1 == ret)
			goto out;

		/* timeout, not normal */
		CU_ASSERT_NOT_EQUAL(ret, 0);
		if (0 == ret)
			goto out;

		if (FD_ISSET(mon.epollfd, &rfds)) {
			ret = io_mon_process_events(&mon);
			CU_ASSERT_EQUAL(ret, 0);
			if (0 != ret)
				goto out;
		}
		if (FD_ISSET(msg_src.pipefds[0], &rfds)) {
			ret = read(msg_src.pipefds[0], &rcvd_msg,
					sizeof(msg_src.msg));
			CU_ASSERT_NOT_EQUAL(ret, -1);

			if (0 == memcmp(&rcvd_msg, &MSG1, sizeof(rcvd_msg))) {
				reached_state(&state, STATE_MSG1_RECEIVED);
			} else if (0 == memcmp(&rcvd_msg, &MSG2,
					sizeof(rcvd_msg))) {
				reached_state(&state, STATE_MSG2_RECEIVED);
			} else if (0 == memcmp(&rcvd_msg, &MSG3,
					sizeof(rcvd_msg))) {
				reached_state(&state, STATE_MSG3_RECEIVED);
			} else if (0 == memcmp(&rcvd_msg, &MSG4,
					sizeof(rcvd_msg))) {
				reached_state(&state, STATE_MSG4_RECEIVED);
			}
		}

		loop = STATE_ALL_DONE != state;
	}

	/* cleanup */
	io_mon_clean(&mon);
	my_msg_src_clean(&(msg_src));

out:
	/* debriefing */
	CU_ASSERT(state & STATE_MSG1_RECEIVED);
	CU_ASSERT(state & STATE_MSG2_RECEIVED);
	CU_ASSERT(state & STATE_MSG3_RECEIVED);
	CU_ASSERT(state & STATE_MSG4_RECEIVED);

	/* error cases */
	ret = io_src_msg_init(NULL, msg_src.pipefds[0], IO_OUT, msg_cb_read,
			&(msg_src.msg), sizeof(struct msg),
			1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src), -1, IO_OUT, msg_cb_read,
			&(msg_src.msg), sizeof(struct msg),
			1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src), msg_src.pipefds[0], IO_OUT,
			msg_cb_read, NULL, sizeof(struct msg),
			1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src), msg_src.pipefds[0], IO_OUT,
			msg_cb_read, &(msg_src.msg), 0, 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&(msg_src.msg_src), msg_src.pipefds[0], IO_OUT,
			NULL, &(msg_src.msg),
			sizeof(struct msg), 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);

}

static void testSRC_MSG_GET_SOURCE(void)
{
	int ret;
	struct io_src_msg msg_src;
	struct io_src *src;
	char buf[22];

	void dummy_cb(struct io_src_msg *src, enum io_src_event evt)
	{

	}

	/* normal use cases */
	ret = io_src_msg_init(&(msg_src),
			STDOUT_FILENO,
			IO_OUT,
			dummy_cb,
			buf,
			22,
			1);
	CU_ASSERT_EQUAL(ret, 0);
	src = io_src_msg_get_source(&(msg_src));
	CU_ASSERT_EQUAL(src, &(msg_src.src));

	/* error use cases */
	src = io_src_msg_get_source(NULL);
	CU_ASSERT_EQUAL(src, NULL);
}

static void testSRC_MSG_GET_MESSAGE(void)
{
	int ret;
	void *msg;
	struct io_src_msg src;

	src.rcv_buf = (void *)0xDEADBEEF;

	ret = io_src_msg_get_message(&src, &msg);
	CU_ASSERT_NOT_EQUAL(ret, -1);
	CU_ASSERT_PTR_EQUAL(msg, src.rcv_buf);

	/* error use cases */
	ret = io_src_msg_get_message(NULL, &msg);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_get_message(&src, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static const struct test_t tests[] = {
		{
				.fn = testSRC_MSG_SET_NEXT_MESSAGE,
				.name = "io_src_msg_set_next_message"
		},
		{
				.fn = testSRC_MSG_INIT_read,
				.name = "io_src_msg_init read"
		},
		{
				.fn = testSRC_MSG_INIT_write,
				.name = "io_src_msg_init write"
		},
		{
				.fn = testSRC_MSG_GET_SOURCE,
				.name = "io_src_msg_get_source"
		},
		{
				.fn = testSRC_MSG_GET_MESSAGE,
				.name = "io_src_msg_get_message"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

struct suite_t src_msg_suite = {
		.name = "io_src_msg",
		.init = NULL,
		.clean = NULL,
		.tests = tests,
};
