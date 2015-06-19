/**
 * @file struct io_inot_test.c
 * @date 23 jul. 2014
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for inotify source.
 *
 * Copyright (C) 2014 Parrot S.A.
 */
#define _GNU_SOURCE
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>

#include <string.h>
#include <inttypes.h>

#include <CUnit/Basic.h>

#include <ut_utils.h>

#include <io_mon.h>
#include <io_src_inot.h>

#include <fautes.h>
#include <fautes_utils.h>

static void testSRC_INOT_INIT(void)
{
	int ret;
	struct io_src_inot inot;

	/* normal use cases */
	ret = io_src_inot_init(&inot);
	CU_ASSERT_EQUAL(ret, 0);

	/* cleanup */
	io_src_inot_clean(&inot);

	/* error use cases */
	ret = io_src_inot_init(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void dummy_cb(struct io_src_inot *inot,
		struct inotify_event *evt, struct io_src_inot_watch *watch)
{

}

static void testSRC_INOT_ADD_WATCH(void)
{
	int ret;
	struct io_src_inot inot;
	struct io_src_inot_watch watch = {
			.path = "/",
			.events = IN_ALL_EVENTS,
			.cb = dummy_cb,
	};

	ret = io_src_inot_init(&inot);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = io_src_inot_add_watch(&inot, &watch);
	CU_ASSERT_EQUAL(ret, 0);
	/* update a watch */
	watch.events = IN_ALL_EVENTS & ~IN_DELETE;
	ret = io_src_inot_add_watch(&inot, &watch);
	CU_ASSERT_EQUAL(ret, 0);
	/* reinstalling the same watch is a noop */
	ret = io_src_inot_add_watch(&inot, &watch);
	CU_ASSERT_EQUAL(ret, 0);

	/* cleanup */
	io_src_inot_rm_watch(&inot, &watch);

	/* error use cases */
	watch.events = 0;
	ret = io_src_inot_add_watch(&inot, &watch);
	CU_ASSERT_EQUAL(ret, -EINVAL);
	watch.events = IN_ALL_EVENTS;
	watch.path = "";
	ret = io_src_inot_add_watch(&inot, &watch);
	CU_ASSERT_EQUAL(ret, -EINVAL);
	watch.path = NULL;
	ret = io_src_inot_add_watch(&inot, &watch);
	CU_ASSERT_EQUAL(ret, -EINVAL);
	watch.path = "/";
	watch.cb = NULL;
	ret = io_src_inot_add_watch(&inot, &watch);
	CU_ASSERT_EQUAL(ret, -EINVAL);
	ret = io_src_inot_add_watch(NULL, &watch);
	CU_ASSERT_EQUAL(ret, -EINVAL);
	ret = io_src_inot_add_watch(&inot, NULL);
	CU_ASSERT_EQUAL(ret, -EINVAL);

	/* cleanup */
	io_src_inot_clean(&inot);
}

static void testSRT_INOT_RM_WATCH(void)
{
	int ret;
	struct io_src_inot inot;
	struct io_src_inot_watch watch = {
			.path = "/",
			.events = IN_ALL_EVENTS,
			.cb = dummy_cb,
	};

	ret = io_src_inot_init(&inot);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_inot_add_watch(&inot, &watch);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = io_src_inot_rm_watch(&inot, &watch);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use cases */
	ret = io_src_inot_rm_watch(&inot, &watch);
	CU_ASSERT_EQUAL(ret, -ENOENT);
	ret = io_src_inot_rm_watch(NULL, &watch);
	CU_ASSERT_EQUAL(ret, -EINVAL);
	ret = io_src_inot_rm_watch(&inot, NULL);
	CU_ASSERT_EQUAL(ret, -EINVAL);

	/* cleanup */
	io_src_inot_clean(&inot);
}

static void testSRC_INOT_CLEAN(void)
{
	int ret;
	struct io_src_inot inot;
	struct io_src_inot_watch watch = {
			.path = "/",
			.events = IN_ALL_EVENTS,
			.cb = dummy_cb,
	};

	ret = io_src_inot_init(&inot);
	CU_ASSERT_EQUAL(ret, 0);
	/* add a watch so that we can verify if it is properly destroyed */
	ret = io_src_inot_add_watch(&inot, &watch);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	io_src_inot_clean(&inot);

	/* error use cases */
	io_src_inot_clean(NULL);
}


static bool check_event(uint32_t *states, uint32_t event, uint32_t step,
		uint32_t state)
{
	uint32_t step_mask = 1u << state;

	if (*states != step_mask - 1 || step != event)
		return false;

	*states |= step_mask;

	return true;
}

#define TEST_DIR "/tmp/src_inot_test/"
#define TEST_FILE TEST_DIR "a_file"

static const uint32_t file_steps[] = {
		IN_OPEN,
		IN_MODIFY,
		IN_CLOSE_WRITE,
		/* ATTRIB generated because unlink lowers link count */
		IN_ATTRIB,
		IN_DELETE_SELF,
};

static uint32_t file_state = 0;

#define STATE_FILE_DONE ((1 << UT_ARRAY_SIZE(file_steps)) - 1)

static const uint32_t dir_steps[] = {
		IN_CREATE,
		IN_OPEN,
		IN_CLOSE_WRITE,
		IN_OPEN,
		IN_MODIFY,
		IN_CLOSE_WRITE,
		IN_DELETE,
};

static uint32_t dir_state = 0;

#define STATE_DIR_DONE ((1 << UT_ARRAY_SIZE(dir_steps)) - 1)

struct main_ctx {
	struct io_src_inot inot;
	struct io_src_inot_watch dir_watch;
	struct io_src_inot_watch file_watch;
};

#define to_main_ctx(i) ut_container_of((i), struct main_ctx, inot)

static void inot_cb(struct io_src_inot *inot, struct inotify_event *evt,
		struct io_src_inot_watch *watch)
{
	int ret;
	uint32_t i;
	uint32_t *state;
	const uint32_t *steps;
	uint32_t size;
	struct main_ctx *ctx = to_main_ctx(inot);
	bool test_dir = evt->wd == ctx->dir_watch.wd;

	if (test_dir) {
		/* event concerning TEST_DIR */
		state = &dir_state;
		steps = dir_steps;
		size = UT_ARRAY_SIZE(dir_steps);
		if (evt->mask == IN_DELETE) {
			ret = io_src_inot_rm_watch(&ctx->inot, &ctx->dir_watch);
			CU_ASSERT_EQUAL(ret, 0);
			ret = rmdir(TEST_DIR);
			CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);
		}
	} else {
		/* event concerning TEST_FILE */
		state = &file_state;
		steps = file_steps;
		size = UT_ARRAY_SIZE(file_steps);
	}

	if (evt->len != 0)
		CU_ASSERT(strcmp(evt->name, basename(TEST_FILE)) == 0);

	for (i = 0; i < size; i++)
		if (check_event(state, evt->mask, steps[i], i))
			return;

	CU_ASSERT(false);
}

/*
 * main test of a fairly realistic use case :
 * * creates the directory /tmp/src_inot_test
 * * install a watch on this directory
 * * creates a file inside /tmp/src_inot_test/a_file
 * * installs a watch on this file
 * * write some content to this file
 * * close it
 * * remove it
 * * remove the directory
 */
static void testSRC_INOT_FULL_TEST(void)
{
#define TEST_FILE_CONTENT "titi tata tutu"
	fd_set rfds;
	int ret;
	int fd;
	struct timeval timeout;
	struct io_mon mon;
	struct main_ctx ctx = {
			.dir_watch = {
					.path = TEST_DIR,
					.events = IN_ALL_EVENTS,
					.cb = inot_cb,
			},
			.file_watch = {
					.path = TEST_FILE,
					.events = IN_ALL_EVENTS,
					.cb = inot_cb,
			},
	};

	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_inot_init(&ctx.inot);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_mon_add_source(&mon, io_src_inot_get_source(&ctx.inot));
	CU_ASSERT_EQUAL(ret, 0);

	/* start of the scenario */
	unlink(TEST_FILE);
	rmdir(TEST_DIR);
	ret = mkdir(TEST_DIR, S_IRWXU | S_IRWXO | S_IRWXG);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = io_src_inot_add_watch(&ctx.inot, &ctx.dir_watch);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	fd = open(TEST_FILE, O_WRONLY | O_CLOEXEC | O_CREAT, S_IRUSR | S_IRUSR);
	CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);
	close(fd);
	ret = io_src_inot_add_watch(&ctx.inot, &ctx.file_watch);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	fd = open(TEST_FILE, O_WRONLY | O_CLOEXEC | O_CREAT, S_IRUSR | S_IRUSR);
	CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);
	ret = write(fd, TEST_FILE_CONTENT, sizeof(TEST_FILE_CONTENT));
	CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);
	close(fd);
	ret = unlink(TEST_FILE);
	CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);

	/* normal use case */
	do {
		/* restore the timer */
		timeout.tv_sec = 3;
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
		CU_ASSERT(ret >= 0);
		if (ret < 0)
			goto out;
	} while (dir_state != STATE_DIR_DONE || file_state != STATE_FILE_DONE);

out:
	/* debriefing */
	CU_ASSERT_EQUAL(dir_state, STATE_DIR_DONE);
	if (dir_state != STATE_DIR_DONE)
		fprintf(stderr, "dir_state is 0x%"PRIx32", expected 0x%x\n",
				dir_state ,STATE_DIR_DONE);
	CU_ASSERT_EQUAL(file_state, STATE_FILE_DONE);
	if (file_state != STATE_FILE_DONE)
		fprintf(stderr, "file_state is 0x%"PRIx32", expected 0x%x\n",
				file_state, STATE_FILE_DONE);

	/* cleanup */
	io_mon_remove_source(&mon, io_src_inot_get_source(&ctx.inot));
	io_mon_clean(&mon);
	io_src_inot_clean(&ctx.inot);
}

static const struct test_t tests[] = {
		{
				.fn = testSRC_INOT_INIT,
				.name = "io_src_inot_init"
		},
		{
				.fn = testSRC_INOT_ADD_WATCH,
				.name = "io_src_inot_add_watch"
		},
		{
				.fn = testSRT_INOT_RM_WATCH,
				.name = "io_src_inot_rm_watch"
		},
		{
				.fn = testSRC_INOT_CLEAN,
				.name = "io_src_inot_clean"
		},
		{
				.fn = testSRC_INOT_FULL_TEST,
				.name = "io_src_inot_full_test"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

struct suite_t src_inot_suite = {
		.name = "io_src_inot",
		.init = NULL,
		.clean = NULL,
		.tests = tests,
};
