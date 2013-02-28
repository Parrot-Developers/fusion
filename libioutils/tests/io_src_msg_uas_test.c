/**
 * @file io_src_msg_uas.c
 * @date 14 feb. 2013
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <stdbool.h>

#include <CUnit/Basic.h>

#include <io_src_msg_uas.h>
#include <io_mon.h>

#include <fautes.h>

enum {
	STATE_START = 0,
	STATE_MSG1_SENT = 1,
	STATE_MSG1_RECEIVED = 2,
	STATE_MSG2_SENT = 4,
	STATE_MSG2_RECEIVED = 8,
	STATE_ALL_DONE = 15,
};

/* global state of the test, messages sent, received... */
static int state = STATE_START;
struct io_mon mon;

struct msg {
	char string[17];
	double number;
};

static const struct msg MSG1 = {"a string", 3.14};
static const struct msg MSG2 = {"gloubi boulga", 1.414};

struct my_uas_src {
	struct msg msg;
	struct io_src_msg_uas uas_src;
};

#define to_src_my_uas_src(p) container_of(p, struct my_uas_src, uas_src)

static void reached_state(int s)
{
	CU_ASSERT_FALSE(s & state);
	state |= s;
}

int uas_cb(struct io_src_msg_uas *src, enum io_src_event evt)
{
	struct my_uas_src *my_uas;
	int ret;

	CU_ASSERT_PTR_NOT_NULL_FATAL(src);
	my_uas = to_src_my_uas_src(src);
	CU_ASSERT_PTR_NOT_NULL_FATAL(my_uas);

	CU_ASSERT(IO_IN == evt || IO_OUT == evt);

	if (evt & IO_OUT) {
		/* this occurs before write */
		if (STATE_START == state) {
			ret = io_src_msg_uas_set_next_message(src, &MSG1);
			CU_ASSERT_NOT_EQUAL(ret, -1);
			reached_state(STATE_MSG1_SENT);
		} else if ((STATE_MSG1_SENT | STATE_MSG1_RECEIVED) == state) {
			ret = io_src_msg_uas_set_next_message(src, &MSG2);
			CU_ASSERT_NOT_EQUAL(ret, -1);
			reached_state(STATE_MSG2_SENT);
			ret = io_mon_activate_out_source(&mon,
					&(my_uas->uas_src.src_msg.src), 0);
			CU_ASSERT_EQUAL(ret, 0);
		} else {
			CU_ASSERT(0);
		}
	}
	if (evt & IO_IN) {
		if (STATE_MSG1_SENT == state) {
			if (0 == memcmp(&(my_uas->msg), &MSG1, sizeof(MSG1)))
				reached_state(STATE_MSG1_RECEIVED);
			else
				CU_ASSERT(0);
		} else if ((STATE_ALL_DONE & ~STATE_MSG2_RECEIVED) == state) {
			if (0 == memcmp(&(my_uas->msg), &MSG2, sizeof(MSG2)))
				reached_state(STATE_MSG2_RECEIVED);
			else
				CU_ASSERT(0);
		} else {
			CU_ASSERT(0);
		}
	}

	return 0;
}

void uas_clean(struct io_src_msg_uas *src)
{
	/* TODO stub */
}

static void testSRC_MSG_UAS_INIT(void)
{
	fd_set rfds;
	int ret;
	struct my_uas_src src;
	bool loop = true;
	struct timeval timeout;

	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);

	ret = io_src_msg_uas_init(&(src.uas_src), uas_cb, uas_clean, &(src.msg),
			sizeof(src.msg), "my_cool_socket_name_%d", 42);
	CU_ASSERT_EQUAL(ret, 0);

	ret = io_mon_add_source(&mon, &(src.uas_src.src_msg.src));
	CU_ASSERT_EQUAL(ret, 0);

	ret = io_mon_activate_out_source(&mon, &(src.uas_src.src_msg.src), 1);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
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
	/* cleanup */
	io_mon_clean(&mon);

	/* debriefeing */
	CU_ASSERT(state & STATE_MSG1_SENT);
	CU_ASSERT(state & STATE_MSG1_RECEIVED);
	CU_ASSERT(state & STATE_MSG2_SENT);
	CU_ASSERT(state & STATE_MSG2_RECEIVED);

	/* error use cases */
}

static void testSRC_MSG_UAS_GET_SOURCE(void)
{
	int ret;
	struct io_src_msg_uas uas_src;
	struct io_src *src;
	char buf[22];

	int dummy_cb(struct io_src_msg_uas *src, enum io_src_event evt)
	{
		return 0;
	}

	void dummy_clean(struct io_src_msg_uas *msg)
	{

	}

	/* normal use cases */
	ret = io_src_msg_uas_init(&(uas_src),
			dummy_cb,
			dummy_clean,
			buf,
			22,
			"my_socket_name");
	CU_ASSERT_EQUAL(ret, 0);
	src = io_src_msg_uas_get_source(&(uas_src));
	CU_ASSERT_EQUAL(src, &(uas_src.src_msg.src));

	/* cleanup */
	io_src_clean(src);

	/* error use cases */
	src = io_src_msg_uas_get_source(NULL);
	CU_ASSERT_EQUAL(src, NULL);
}

static const test_t tests[] = {
		/* TODO add set next message test */
		{
				.fn = testSRC_MSG_UAS_INIT,
				.name = "io_src_msg_uas_init"
		},
		{
				.fn = testSRC_MSG_UAS_GET_SOURCE,
				.name = "io_src_msg_uas_get_source"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_src_msg_uas_suite(void)
{
	return 0; /* return non-zero on error */
}

static int clean_src_msg_uas_suite(void)
{
	/* cleanup code concerning the whole tests suite */

	return 0; /* return non-zero on error */
}

suite_t src_msg_uas_suite = {
		.name = "io_src_msg_uas",
		.init = init_src_msg_uas_suite,
		.clean = clean_src_msg_uas_suite,
		.tests = tests,
};

