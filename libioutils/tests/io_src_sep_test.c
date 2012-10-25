/**
 * @file io_src_sig_test.c
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
#include <io_src_sep.h>

#include <fautes.h>
#include <fautes_utils.h>

static void reached_state(int *glob_state, int state)
{
	*glob_state |= state;
}

static const int sep_mono[2] = {'\n', INT_MAX};
#define SSEP_MONO "\n"
static const int sep_double[2] = {'\r', '\n'};
#define SSEP_DOUBLE "\r\n"

#define CSEP '\n'
#define MSG1 "lou lou lou, je cueille des pommes"
#define MSG2 "lou lou lou, et toi itou"
#define MSG3 ""
#define MSG4 "lou lou lou, on s'met tous nus"

/* main and only test. sends ourselves messages and check we receive them */
static void testSRC_SEP(const int sep_pair[2], const char *big_msg, size_t sz)
{
	int pipefds[2] = {-1, -1};
	fd_set rfds;
	int ret;
	struct io_mon mon;
	struct io_src_sep src_sep;
	bool loop = true;
	struct timeval timeout;
#define STATE_START 0
#define STATE_MSG1_RECEIVED 1
#define STATE_MSG2_RECEIVED 2
#define STATE_MSG3_RECEIVED 4
#define STATE_TIMER_EXPIRED 8
#define STATE_ALL_DONE 15
	int state = STATE_START;
	int sep_cb(struct io_src_sep *sep, char *chunk, unsigned len)
	{
/*		printf("received %u byte(s) : \"%.*s\"\n", len, len, chunk); */

		CU_ASSERT_NOT_EQUAL(state,
				STATE_ALL_DONE & ~STATE_TIMER_EXPIRED);
		CU_ASSERT_NOT_EQUAL(0, memcmp(chunk, MSG4, strlen(MSG4)));

		if (0 == memcmp(chunk, MSG1, strlen(MSG1))) {
			CU_ASSERT_EQUAL(state, STATE_START);
			reached_state(&state, STATE_MSG1_RECEIVED);

			CU_ASSERT_EQUAL(len, strlen(MSG1) + 1 + sep->two_bytes);
		} else if (0 == memcmp(chunk, MSG2, strlen(MSG2))) {
			CU_ASSERT_EQUAL(state, STATE_MSG1_RECEIVED);
			reached_state(&state, STATE_MSG2_RECEIVED);
		} else if (0 == memcmp(chunk, MSG3, strlen(MSG3))) {
			CU_ASSERT_EQUAL(state, STATE_MSG1_RECEIVED |
					STATE_MSG2_RECEIVED);
			reached_state(&state, STATE_MSG3_RECEIVED);
		}

		return 0;
	}

	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	ret = pipe(pipefds);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_sep_init(&src_sep, pipefds[0], sep_cb, sep_pair[0],
			sep_pair[1]);
	CU_ASSERT_EQUAL(ret, 0);

	ret = io_mon_add_source(&mon, &(src_sep.src));
	CU_ASSERT_EQUAL(ret, 0);

	ret = write(pipefds[1], big_msg, sz);
	CU_ASSERT_NOT_EQUAL(ret, -1);

	/* normal use case */
	while (loop) {
		/* restore the timer */
		timeout.tv_sec = 0;
		timeout.tv_usec = 50;

		/* restore the read file descriptor set */
		FD_ZERO(&rfds);
		FD_SET(mon.epollfd, &rfds);
		ret = select(mon.epollfd + 1, &rfds, NULL, NULL, &timeout);

		/* error, not normal */
		CU_ASSERT_NOT_EQUAL(ret, -1);
		if (-1 == ret)
			goto out;

		/* timeout, normal */
		if (0 == ret) {
			CU_ASSERT_NOT_EQUAL(state, STATE_ALL_DONE);
			reached_state(&state, STATE_TIMER_EXPIRED);
		} else {
			ret = io_mon_process_events(&mon);
			CU_ASSERT_EQUAL(ret, 0);
			if (0 != ret)
				goto out;
		}

		loop = STATE_ALL_DONE != state;
	}

out:
	/* debriefing */
	CU_ASSERT(state & STATE_MSG1_RECEIVED);
	CU_ASSERT(state & STATE_MSG2_RECEIVED);
	CU_ASSERT(state & STATE_MSG3_RECEIVED);
	CU_ASSERT(state & STATE_TIMER_EXPIRED);

	/* error cases */
	ret = io_src_sep_init(NULL, pipefds[0], sep_cb, sep_pair[0],
			sep_pair[1]);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_sep_init(&src_sep, -1, sep_cb, sep_pair[0],
			sep_pair[1]);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_sep_init(&src_sep, pipefds[0], NULL, sep_pair[0],
			sep_pair[1]);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	io_mon_clean(&mon);
	close(pipefds[0]);
	close(pipefds[1]);
}


static void testSRC_SEP_INIT(void)
{
#define BIG_MSG(sep) MSG1 sep MSG2 sep MSG3 sep MSG4
	const char big_msg_mono[] = BIG_MSG(SSEP_MONO);
	const char big_msg_double[] =  BIG_MSG(SSEP_DOUBLE);

	/* test for linux-style line messages : ending with \n */
	testSRC_SEP(sep_mono, big_msg_mono, strlen(big_msg_mono));
	/* test for modem-style line messages : ending with \r\n (CRLF) */
	testSRC_SEP(sep_double, big_msg_double, strlen(big_msg_double));
}

static const test_t tests[] = {
		{
				.fn = testSRC_SEP_INIT,
				.name = "io_src_sep_init"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

suite_t src_sep_suite = {
		.name = "io_src_sep",
		.init = NULL,
		.clean = NULL,
		.tests = tests,
};
