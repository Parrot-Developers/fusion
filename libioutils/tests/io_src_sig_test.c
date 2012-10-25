/**
 * @file struct io_src_sigest.c
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
#include <io_src_sig.h>

#include <fautes.h>
#include <fautes_utils.h>

static void reached_state(int *glob_state, int state)
{
	*glob_state |= state;
}

static int sigsets_are_equals(sigset_t *mask1, sigset_t *mask2)
{
	/* more reasonable solution than a comparison on each possible value */
	return 0 == memcmp(mask1, mask2, sizeof(*mask1));
}

/*
 * main and only test. sends ourselves signals and check we receive them :
 *   * send / received a sigusr1
 *   * send / received a sigusr2
 * then exit
 */
static void testSRC_SIG_INIT(void)
{
	sigset_t old_mask;
	sigset_t new_mask;
	fd_set rfds;
	int mon_fd;
	int ret;
	struct io_mon *mon;
	struct io_src_sig src_sig;
	bool loop = true;
	struct timeval timeout;
#define STATE_START 0
#define STATE_SIGUSR1_RECEIVED 1
#define STATE_SIGUSR2_RECEIVED 2
#define STATE_ALL_DONE 3
	int state = STATE_START;
	int sig_cb(struct io_src_sig *sig)
	{
		if (SIGUSR1 == sig->si.ssi_signo) {
			CU_ASSERT(!(STATE_SIGUSR2_RECEIVED & state));
			reached_state(&state, STATE_SIGUSR1_RECEIVED);
			ret = kill(getpid(), SIGUSR2);
		} else if (SIGUSR2 == sig->si.ssi_signo) {
			CU_ASSERT(STATE_SIGUSR1_RECEIVED & state);
			reached_state(&state, STATE_SIGUSR2_RECEIVED);
			loop = false;
		} else {
			CU_ASSERT(0);
		}

		return 0;
	}

	mon = io_mon_new();
	CU_ASSERT_PTR_NOT_NULL(mon);
	mon_fd = io_mon_get_fd(mon);
	/* save sigmask for further use */
	sigemptyset(&old_mask);
	ret = sigprocmask(SIG_SETMASK, NULL, &old_mask);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_sig_init(&src_sig, sig_cb, SIGUSR1, SIGUSR2, NULL);
	CU_ASSERT_EQUAL(ret, 0);

	ret = io_mon_add_source(mon, &(src_sig.src));
	CU_ASSERT_EQUAL(ret, 0);

	ret = kill(getpid(), SIGUSR1);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use case */
	while (loop) {
		/* restore the timer */
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		/* restore the read file descriptor set */
		FD_ZERO(&rfds);
		FD_SET(mon_fd, &rfds);
		ret = select(mon_fd + 1, &rfds, NULL, NULL, &timeout);

		/* error, not normal */
		CU_ASSERT_NOT_EQUAL(ret, -1);
		if (-1 == ret)
			goto out;

		/* timeout, not normal */
		CU_ASSERT_NOT_EQUAL(ret, 0);
		if (0 == ret)
			goto out;
		ret = io_mon_process_events(mon);
		CU_ASSERT_EQUAL(ret, 0);
		if (0 != ret)
			goto out;

		loop = STATE_ALL_DONE != state;
	}

out:
	/* debriefing */
	CU_ASSERT(state & STATE_SIGUSR1_RECEIVED);
	CU_ASSERT(state & STATE_SIGUSR2_RECEIVED);

	/* cleanup */
	io_mon_delete(&mon);

	/* check sigmask hasn't changed */
	sigemptyset(&new_mask);
	ret = sigprocmask(SIG_SETMASK, NULL, &new_mask);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT(sigsets_are_equals(&old_mask, &new_mask));

	/* error use cases TODO */

}

static const test_t tests[] = {
		{
				.fn = testSRC_SIG_INIT,
				.name = "io_src_sig_init"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

suite_t src_sig_suite = {
		.name = "io_src_sig",
		.init = NULL,
		.clean = NULL,
		.tests = tests,
};
