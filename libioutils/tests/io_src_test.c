/**
 * @file src_test.c
 * @date 17 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for io_src module
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <unistd.h>
#include <fcntl.h>

#include <CUnit/Basic.h>

#include <io_src.h>

#include <fautes.h>

static int my_dummy_cb(__attribute__((unused)) struct io_src *source)
{
	return 0;
}

static void clean_cb(__attribute__((unused)) struct io_src *src)
{

}

static void testSRC_INIT(void)
{
	int pipefd[2] = {-1, -1};
	int fd;
	struct io_src src;
	int ret;

	ret = pipe(pipefd);
	CU_ASSERT_NOT_EQUAL_FATAL(ret, -1);
	fd = open("/dev/urandom", O_RDWR | O_CLOEXEC);
	CU_ASSERT_NOT_EQUAL_FATAL(fd, -1);

	/* normal use case */
	/* put garbage in the struct */
	ret = read(fd, &src, sizeof(src));
	CU_ASSERT_EQUAL(ret, sizeof(src));
	ret = io_src_init(&src, pipefd[0], IO_IN, my_dummy_cb, clean_cb);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(src.fd, pipefd[0]);
	CU_ASSERT_EQUAL(src.type, IO_IN);
	CU_ASSERT_EQUAL(src.cb, my_dummy_cb);
	CU_ASSERT_EQUAL(src.clean, clean_cb);

	CU_ASSERT_EQUAL(src.events, 0);

	CU_ASSERT_EQUAL(src.active, 0);
	CU_ASSERT_PTR_NULL(src.node.next);
	CU_ASSERT_PTR_NULL(src.node.prev);

	/* put garbage in the struct */
	ret = read(fd, &src, sizeof(src));
	CU_ASSERT_EQUAL(ret, sizeof(src));
	ret = io_src_init(&src, pipefd[1], IO_OUT, my_dummy_cb, NULL);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(src.fd, pipefd[1]);
	CU_ASSERT_EQUAL(src.type, IO_OUT);
	CU_ASSERT_EQUAL(src.cb, my_dummy_cb);

	CU_ASSERT_EQUAL(src.events, 0);

	CU_ASSERT_EQUAL(src.active, 0);
	CU_ASSERT_PTR_NULL(src.node.next);
	CU_ASSERT_PTR_NULL(src.node.prev);

	/* put garbage in the struct */
	ret = read(fd, &src, sizeof(src));
	CU_ASSERT_EQUAL(ret, sizeof(src));
	ret = io_src_init(&src, fd, IO_DUPLEX, my_dummy_cb, NULL);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(src.fd, fd);
	CU_ASSERT_EQUAL(src.type, IO_DUPLEX);
	CU_ASSERT_EQUAL(src.cb, my_dummy_cb);

	CU_ASSERT_EQUAL(src.events, 0);

	CU_ASSERT_EQUAL(src.active, 0);
	CU_ASSERT_PTR_NULL(src.node.next);
	CU_ASSERT_PTR_NULL(src.node.prev);

	/* error use cases */
	ret = io_src_init(NULL, pipefd[0], IO_IN, my_dummy_cb, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_init(&src, -1, IO_IN, my_dummy_cb, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = io_src_init(&src, pipefd[0], IO_IN, NULL, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	close(pipefd[0]);
	close(pipefd[1]);
	close(fd);
}

static void testTO_SRC(void)
{
	struct io_src src;

	CU_ASSERT_EQUAL(to_src(&(src.node)), &src);
}

static const test_t tests[] = {
		{
				.fn = testSRC_INIT,
				.name = "io_src_init"
		},
		{
				.fn = testTO_SRC,
				.name = "to_src"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_src_suite(void)
{
	return 0;
}

static int clean_src_suite(void)
{
	return 0;
}

suite_t src_suite = {
		.name = "io_src",
		.init = init_src_suite,
		.clean = clean_src_suite,
		.tests = tests,
};
