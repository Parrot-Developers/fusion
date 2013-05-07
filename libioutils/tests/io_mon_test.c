/**
 * @file mon_test.c
 * @date 17 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for io_mon module
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <unistd.h>
#include <fcntl.h>

#include <stdbool.h>

#include <CUnit/Basic.h>

#include <io_mon.h>

#include <fautes.h>
#include <fautes_utils.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC       02000000        /* set close_on_exec */
#endif

static void testMON_INIT(void)
{
	struct io_mon mon;
	int ret;

	/* normal use case */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_NOT_EQUAL(mon.epollfd, -1);
	CU_ASSERT_EQUAL(mon.nb_sources, 0);
	CU_ASSERT_EQUAL(mon.source, NULL);

	/* cleanup */
	io_mon_clean(&mon);

	/* error use cases */
	ret = io_mon_init(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testMON_GET_FD(void)
{
	struct io_mon mon;
	int ret;
	int fd;

	/* normal use case */
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	fd = io_mon_get_fd(&mon);
	CU_ASSERT_EQUAL(fd, mon.epollfd);

	/* cleanup */
	io_mon_clean(&mon);

	/* error use cases */
	ret = io_mon_get_fd(NULL);
	CU_ASSERT_EQUAL(ret, -EINVAL);
}

static void my_dummy_callback(__attribute__((unused)) struct io_src *src)
{
	/* do nothing */
}

static void testMON_ADD_SOURCE(void)
{
	int pipefd[2] = {-1, -1};
	int fd;
	struct io_mon mon;
	struct io_src src_in;
	struct io_src src_out;
	struct io_src src_duplex;
	int ret;
	int flags;

	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	ret = pipe(pipefd);
	CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);
	ret = io_src_init(&src_in, pipefd[0], IO_IN, my_dummy_callback);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_init(&src_out, pipefd[1], IO_OUT, my_dummy_callback);
	CU_ASSERT_EQUAL(ret, 0);
	fd = open("/dev/random", O_RDWR | O_CLOEXEC);
	CU_ASSERT_NOT_EQUAL_FATAL(fd, -1);
	ret = io_src_init(&src_duplex, fd, IO_DUPLEX, my_dummy_callback);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = io_mon_add_source(&mon, &src_in);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT(!!(src_in.active & IO_IN));
	flags = fcntl(src_in.fd, F_GETFL, 0);
	CU_ASSERT_EQUAL(!!(flags & O_NONBLOCK), 1);

	ret = io_mon_add_source(&mon, &src_out);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(src_out.active, 0);
	flags = fcntl(src_out.fd, F_GETFL, 0);
	CU_ASSERT_EQUAL(!!(flags & O_NONBLOCK), 1);

	ret = io_mon_add_source(&mon, &src_duplex);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT(!!(src_duplex.active & IO_IN));
	flags = fcntl(src_duplex.fd, F_GETFL, 0);
	CU_ASSERT_EQUAL(!!(flags & O_NONBLOCK), 1);

	/* error use cases */
	/* adding a source twice, is forbidden */
	ret = io_mon_add_source(&mon, &src_duplex);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_mon_add_source(NULL, &src_out);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_mon_add_source(&mon, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	io_mon_clean(&mon);
	close(pipefd[0]);
	close(pipefd[1]);
	close(fd);
}

static void testMON_ADD_SOURCES(void)
{
	int pipefd[2] = {-1, -1};
	int fd;
	struct io_mon mon;
	struct io_src src_in;
	struct io_src src_out;
	struct io_src src_duplex;
	int ret;
	int flags;

	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	ret = pipe(pipefd);
	CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);
	ret = io_src_init(&src_in, pipefd[0], IO_IN, my_dummy_callback);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_init(&src_out, pipefd[1], IO_OUT, my_dummy_callback);
	CU_ASSERT_EQUAL(ret, 0);
	fd = open("/dev/random", O_RDWR | O_CLOEXEC);
	CU_ASSERT_NOT_EQUAL_FATAL(fd, -1);
	ret = io_src_init(&src_duplex, fd, IO_DUPLEX, my_dummy_callback);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = io_mon_add_sources(&mon,
			&src_in,
			&src_out,
			&src_duplex,
			NULL /* NULL sentinel */
			);
	CU_ASSERT_EQUAL(ret, 0);
	flags = fcntl(src_in.fd, F_GETFL, 0);
	CU_ASSERT_EQUAL(!!(flags & O_NONBLOCK), 1);
	flags = fcntl(src_out.fd, F_GETFL, 0);
	CU_ASSERT_EQUAL(!!(flags & O_NONBLOCK), 1);
	flags = fcntl(src_duplex.fd, F_GETFL, 0);
	CU_ASSERT_EQUAL(!!(flags & O_NONBLOCK), 1);
	CU_ASSERT(!!(src_in.active & IO_IN));
	CU_ASSERT_EQUAL(src_out.active, 0);
	CU_ASSERT(!!(src_duplex.active & IO_IN));

	/* error use cases */
	ret = io_mon_add_sources(NULL, &src_out, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_mon_add_source(&mon, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	io_mon_clean(&mon);
	close(pipefd[0]);
	close(pipefd[1]);
	close(fd);
}

static void testMON_DUMP_EPOLL_EVENT(void)
{
	char str[1024];
	int ret;
	const char *ref1 = "epoll events :\n"
			"\tEPOLLOUT\n";
	const char *ref2 = "epoll events :\n"
			"\tEPOLLOUT\n"
			"\tEPOLLHUP\n";

	/* normal use cases */
	ret = store_function_output_to_string(str, 1024,
			io_mon_dump_epoll_event, STDERR_FILENO, EPOLLOUT);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_STRING_EQUAL(str, ref1);
	ret = store_function_output_to_string(str, 1024,
			io_mon_dump_epoll_event, STDERR_FILENO,
			EPOLLOUT | EPOLLHUP);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_STRING_EQUAL(str, ref2);
}

static void testMON_ACTIVATE_OUT_SOURCE(void)
{
	int pipefd[2] = {-1, -1};
	int fd;
	struct io_mon mon;
	struct io_src src_out;
	struct io_src src_in;
	struct io_src src_duplex;
	int ret;

	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	ret = pipe(pipefd);
	CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);
	ret = io_src_init(&src_in, pipefd[0], IO_IN, my_dummy_callback);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_init(&src_out, pipefd[1], IO_OUT, my_dummy_callback);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_mon_add_source(&mon, &src_out);
	CU_ASSERT_EQUAL(ret, 0);
	fd = open("/dev/random", O_RDWR | O_CLOEXEC);
	CU_ASSERT_NOT_EQUAL_FATAL(fd, -1);

	/* normal use cases */
	/* output source */
	CU_ASSERT_EQUAL(src_out.active, 0);
	ret = io_mon_activate_out_source(&mon, &src_out, 1);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(src_out.active, IO_OUT);
	ret = io_mon_activate_out_source(&mon, &src_out, 0);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(src_out.active, 0);

	/* duplex source */
	io_mon_clean(&mon);
	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_init(&src_duplex, fd, IO_DUPLEX, my_dummy_callback);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_mon_add_source(&mon, &src_duplex);
	CU_ASSERT_EQUAL(ret, 0);

	CU_ASSERT_EQUAL(src_duplex.active, IO_IN);
	ret = io_mon_activate_out_source(&mon, &src_duplex, 1);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(src_duplex.active, IO_DUPLEX);
	ret = io_mon_activate_out_source(&mon, &src_duplex, 0);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(src_duplex.active, IO_IN);

	/* error use cases */
	ret = io_mon_activate_out_source(NULL, &src_duplex, 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_mon_activate_out_source(&mon, NULL, 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_mon_activate_out_source(&mon, &src_in, 1);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	io_mon_clean(&mon);
	close(pipefd[0]);
	close(pipefd[1]);
	close(fd);
}

static void reached_state(int *glob_state, int state)
{
	*glob_state |= state;
}

/*
 * Note : an "src->cb : Input/output error" is normal and results from error
 * test cases
 */
static void testMON_PROCESS_EVENTS(void)
{
	fd_set rfds;
	int pipefd[2] = {-1, -1};
	int ret;
	struct io_mon mon;
	struct io_src src_out;
	struct io_src src_in;
	bool loop = true;
	struct timeval timeout;
	const char const *msg1 = "Salut !";
	const char const *msg2 = "Ça va ? !";

#define STATE_START 0
#define STATE_MSG1_RECEIVED 1
#define STATE_MSG2_SENT 2
#define STATE_MSG2_RECEIVED 4
#define STATE_ALL_DONE 7
	int state = STATE_START;
	void in_cb(struct io_src *src)
	{
		char buf[1024];
		int r;

		r = read(src->fd, buf, 1024);
		CU_ASSERT_NOT_EQUAL_FATAL(r, -1);

		if (0 == strcmp(msg1, buf)) {
			CU_ASSERT(0 == (state & STATE_MSG1_RECEIVED));
			reached_state(&state, STATE_MSG1_RECEIVED);

			/* monitor out to sent the second message */
			r = io_mon_activate_out_source(&mon, &src_out, 1);
			CU_ASSERT_NOT_EQUAL(r, -1);
		} else if (0 == strcmp(msg2, buf)) {
			CU_ASSERT(0 == (state & STATE_MSG2_RECEIVED));
			reached_state(&state, STATE_MSG2_RECEIVED);
			/* generates an input/output error */
			close(pipefd[0]);
		}

		return;
	}
	void out_cb(struct io_src *src)
	{
		int r;

		/* TODO treat I/O THEN errors */
		if (io_mon_has_error(src->events))
			return;

		r = write(src->fd, msg2, strlen(msg2) + 1);
		CU_ASSERT_NOT_EQUAL_FATAL(r, -1);

		/* disable out source when unneeded to avoid event loop panic */
		r = io_mon_activate_out_source(&mon, &src_out, 0);
		CU_ASSERT_NOT_EQUAL(r, -1);

		reached_state(&state, STATE_MSG2_SENT);
	}

	ret = pipe(pipefd);
	CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);
	ret = io_src_init(&src_in, pipefd[0], IO_IN, in_cb);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_src_init(&src_out, pipefd[1], IO_OUT, out_cb);
	CU_ASSERT_EQUAL(ret, 0);

	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_mon_add_source(&mon, &src_out);
	CU_ASSERT_EQUAL(ret, 0);
	ret = io_mon_add_source(&mon, &src_in);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use case */
	ret = write(pipefd[1], msg1, strlen(msg1) + 1);
	CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);

	while (loop) {
		/* restore the timer */
		timeout.tv_sec = 5;
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

		loop = (STATE_ALL_DONE) != state;
	}

out:
	/* cleanup */
	io_mon_clean(&mon);
	io_src_clean(&src_in);
	io_src_clean(&src_out);

	/* debriefing */
	CU_ASSERT(state & STATE_MSG1_RECEIVED);
	CU_ASSERT(state & STATE_MSG2_SENT);
	CU_ASSERT(state & STATE_MSG2_RECEIVED);
}

static void testMON_CLEAN(void)
{
	struct io_mon mon;
	int ret;

	ret = io_mon_init(&mon);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use case */
	ret = io_mon_clean(&mon);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(mon.epollfd, -1);
	CU_ASSERT_EQUAL(mon.nb_sources, 0);
	CU_ASSERT_EQUAL(mon.source, NULL);

	/* cleanup */
	io_mon_clean(&mon);

	/* error use cases */
	ret = io_mon_clean(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static const test_t tests[] = {
		{
				.fn = testMON_INIT,
				.name = "io_mon_init"
		},
		{
				.fn = testMON_GET_FD,
				.name = "io_mon_get_fd"
		},
		{
				.fn = testMON_ADD_SOURCE,
				.name = "io_mon_add_source"
		},
		{
				.fn = testMON_ADD_SOURCES,
				.name = "io_mon_add_sources"
		},
		{
				.fn = testMON_DUMP_EPOLL_EVENT,
				.name = "io_mon_dump_epoll_event"
		},
		{
				.fn = testMON_ACTIVATE_OUT_SOURCE,
				.name = "io_mon_activate_out_source"
		},
		{
				.fn = testMON_PROCESS_EVENTS,
				.name = "io_mon_process_events"
		},
		{
				.fn = testMON_CLEAN,
				.name = "io_mon_clean"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_mon_suite(void)
{
	return 0;
}

static int clean_mon_suite(void)
{
	return 0;
}

suite_t mon_suite = {
		.name = "io_mon",
		.init = init_mon_suite,
		.clean = clean_mon_suite,
		.tests = tests,
};
