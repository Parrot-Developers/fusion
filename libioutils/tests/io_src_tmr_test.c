/**
 * @file io_tmr_src_test.c
 * @date 16 may 2013
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for timer io source
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <CUnit/Basic.h>

#include <fautes.h>

#include <io_mon.h>
#include <io_src_tmr.h>

static void dummy_tmr_cb(struct io_src_tmr *tmr, uint64_t *nbexpired)
{
	/* does nothing */
};

static void testIO_SRC_TMR_INIT(void)
{
	int ret;
	struct io_src_tmr tmr;

	/* normal use cases */
	ret = io_src_tmr_init(&tmr, dummy_tmr_cb);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT(tmr.src.fd > -1);
	CU_ASSERT_EQUAL(tmr.cb, dummy_tmr_cb);

	/* error use cases */
	ret = io_src_tmr_init(NULL, dummy_tmr_cb);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_tmr_init(&tmr, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	io_src_tmr_clean(&tmr);
}

static void testIO_SRC_TMR_SET(void)
{
	int ret;
	fd_set rfds;
	struct io_mon mon;
	struct io_src_tmr tmr;
	int expired = 0;
	struct timeval timeout = {
			.tv_sec = 1,
			.tv_usec = 0,
	};
	void tmr_cb(struct io_src_tmr *local_tmr, uint64_t *nbexpired)
	{
		expired = 1;
	};

	/* initialization */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_src_tmr_init(&tmr, tmr_cb);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(tmr.cb, tmr_cb);
	ret = io_mon_add_source(&mon, io_src_tmr_get_source(&tmr));
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	/* one shoot */
	ret = io_src_tmr_set(&tmr, 999);
	CU_ASSERT_EQUAL(ret, 0);

	FD_ZERO(&rfds);
	FD_SET(mon.epollfd, &rfds);

	ret = select(mon.epollfd + 1, &rfds, NULL, NULL, &timeout);
	/* error, not normal */
	CU_ASSERT_NOT_EQUAL(ret, -1);
	if (-1 == ret)
		goto out;
	ret = io_mon_process_events(&mon);
	CU_ASSERT(ret >= 0);

	CU_ASSERT(expired);

	/* disarm test */
	expired = 0;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;

	ret = io_src_tmr_set(&tmr, 10000);
	CU_ASSERT_EQUAL(ret, 0);

	FD_ZERO(&rfds);
	FD_SET(mon.epollfd, &rfds);

	ret = io_src_tmr_set(&tmr, IO_SRC_TMR_DISARM);
	CU_ASSERT_EQUAL(ret, 0);

	ret = select(mon.epollfd + 1, &rfds, NULL, NULL, &timeout);
	/* error, not normal */
	CU_ASSERT_NOT_EQUAL(ret, -1);
	if (-1 == ret)
		goto out;
	ret = io_mon_process_events(&mon);
	CU_ASSERT(ret >= 0);

	CU_ASSERT_FALSE(expired);

out:
	/* cleanup */
	io_mon_clean(&mon);
	io_src_tmr_clean(&tmr);

	/* error use cases */
	ret = io_src_tmr_set(NULL, 100);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testIO_SRC_TMR_CLEAN(void)
{
	int ret;
	struct io_src_tmr tmr;

	/* initialization */
	ret = io_src_tmr_init(&tmr, dummy_tmr_cb);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	io_src_tmr_clean(&tmr);
	CU_ASSERT_PTR_NULL(tmr.cb);

	/* error use cases */
	io_src_tmr_clean(&tmr);
}

static void testIO_SRC_SET_PERIODIC(void)
{
	int ret;
	struct io_src_tmr tmr;

	/* initialization */
	ret = io_src_tmr_init(&tmr, dummy_tmr_cb);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = io_src_tmr_set_periodic(&tmr, 1);
	CU_ASSERT_EQUAL(tmr.periodic, 1);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use cases */
	ret = io_src_tmr_set_periodic(NULL, 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	io_src_tmr_clean(&tmr);
}

static const struct test_t tests[] = {
		{
				.fn = testIO_SRC_TMR_INIT,
				.name = "io_src_tmr_init"
		},
		{
				.fn = testIO_SRC_TMR_SET,
				.name = "io_src_tmr_set"
		},
		{
				.fn = testIO_SRC_TMR_CLEAN,
				.name = "io_src_tmr_clean"
		},
		{
				.fn = testIO_SRC_SET_PERIODIC,
				.name = "io_src_tmr_set_periodic"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

struct suite_t src_tmr_suite = {
		.name = "io_src_tmr",
		.init = NULL,
		.clean = NULL,
		.tests = tests,
};

