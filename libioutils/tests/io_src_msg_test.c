/**
 * @file io_src_msg_test.c
 * @date 22 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for signal source.
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#include <unistd.h>
#include <fcntl.h>

#include <limits.h>
#include <stdbool.h>
#include <signal.h>

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

static const struct msg MSG1 = {11, 11111, 11.111};
static const struct msg MSG2 = {22, 22222, 22.222};
static const struct msg MSG3 = {33, 33333, 33.333};
static const struct msg MSG4 = {44, 44444, 44.444};

/* main and only test. sends ourselves messages and check we receive them */
static void testSRC_MSG_INIT(void)
{
	int pipefds[2] = {-1, -1};
	fd_set rfds;
	struct msg msg;
	int ret;
	struct io_mon mon;
	struct io_src_msg src_msg;
	bool loop = true;
	struct timeval timeout;
#define STATE_START 0
#define STATE_MSG1_RECEIVED 1
#define STATE_MSG2_RECEIVED 2
#define STATE_MSG3_RECEIVED 4
#define STATE_MSG4_RECEIVED 8
#define STATE_ALL_DONE 15
	int state = STATE_START;
	int msg_cb(struct io_src_msg *src)
	{
		__attribute__((unused))struct msg *my_msg = src->msg;
/*		printf("received : \"%d %d %f\"\n", my_msg->a, my_msg->b,
				my_msg->c);*/

		CU_ASSERT_NOT_EQUAL(state, STATE_ALL_DONE);

		if (0 == memcmp(src->msg, &MSG1, src->len)) {
			CU_ASSERT_EQUAL(state, STATE_START);
			reached_state(&state, STATE_MSG1_RECEIVED);

			ret = write(pipefds[1], &MSG2, sizeof(struct msg));
			CU_ASSERT_NOT_EQUAL(ret, -1);
		} else if (0 == memcmp(src->msg, &MSG2, src->len)) {
			CU_ASSERT_EQUAL(state, STATE_MSG1_RECEIVED);
			reached_state(&state, STATE_MSG2_RECEIVED);

			ret = write(pipefds[1], &MSG3, sizeof(struct msg));
			CU_ASSERT_NOT_EQUAL(ret, -1);
		} else if (0 == memcmp(src->msg, &MSG3, src->len)) {
			CU_ASSERT_EQUAL(state, STATE_MSG1_RECEIVED |
					STATE_MSG2_RECEIVED);
			reached_state(&state, STATE_MSG3_RECEIVED);

			ret = write(pipefds[1], &MSG4, sizeof(struct msg));
			CU_ASSERT_NOT_EQUAL(ret, -1);
		} else if (0 == memcmp(src->msg, &MSG4, src->len)) {
			CU_ASSERT_EQUAL(state, STATE_MSG1_RECEIVED |
					STATE_MSG2_RECEIVED |
					STATE_MSG3_RECEIVED);
			reached_state(&state, STATE_MSG4_RECEIVED);
		}

		return 0;
	}

	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	ret = pipe(pipefds);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_msg_init(&src_msg, pipefds[0], &msg, sizeof(msg), msg_cb);
	CU_ASSERT_EQUAL(ret, 0);

	ret = io_mon_add_source(&mon, &(src_msg.src));
	CU_ASSERT_EQUAL(ret, 0);

	ret = write(pipefds[1], &MSG1, sizeof(msg));
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

	/* error cases */
	ret = io_src_msg_init(NULL, pipefds[0], &msg, sizeof(msg), msg_cb);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&src_msg, -1, &msg, sizeof(msg), msg_cb);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&src_msg, pipefds[0], NULL, sizeof(msg), msg_cb);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&src_msg, pipefds[0], &msg, 0, msg_cb);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_msg_init(&src_msg, pipefds[0], &msg, sizeof(msg), NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	io_mon_clean(&mon);
	close(pipefds[0]);
	close(pipefds[1]);
}

static const test_t tests[] = {
		{
				.fn = testSRC_MSG_INIT,
				.name = "io_src_msg_init"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

suite_t src_msg_suite = {
		.name = "io_src_msg",
		.init = NULL,
		.clean = NULL,
		.tests = tests,
};
