/**
 * @file rs_rb_test.c
 * @date 13 may 2013
 * @author nicolas.carrier@parrot.com
 * @brief
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <CUnit/Basic.h>

#include <fautes.h>

#include <rs_rb.h>

static void testRS_RB_INIT(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4];

	/* normal use cases */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(rb.base, buffer);
	CU_ASSERT_EQUAL(rb.size, 4);
	CU_ASSERT_EQUAL(rb.size_mask, 3);
	CU_ASSERT_EQUAL(rb.len, 0);
	CU_ASSERT_EQUAL(rb.read, 0);
	CU_ASSERT_EQUAL(rb.write, 0);

	/* error use cases */
	ret = rs_rb_init(NULL, buffer, 4);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_rb_init(&rb, buffer, 0);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	/* non power of two sizes aren't allowed */
	ret = rs_rb_init(&rb, buffer, 17);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_RB_GET_SIZE(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4];
	size_t size;

	/* normal use cases */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);
	size = rs_rb_get_size(&rb);
	CU_ASSERT_EQUAL(size, 4);

	/* error use cases */
	size = rs_rb_get_size(NULL);
	CU_ASSERT_EQUAL(size, 0);
}

static void testRS_RB_EMPTY(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4];

	/* initialization */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);
	/* fake we have written 3 bytes then consumed 2 */
	ret = rs_rb_write_incr(&rb, 3);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = rs_rb_empty(&rb);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(rb.len, 0);
	CU_ASSERT_EQUAL(rb.read, 0);
	CU_ASSERT_EQUAL(rb.write, 0);

	/* error use cases */
	ret = rs_rb_empty(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_RB_CLEAN(void)
{
	struct rs_rb rb;
	struct rs_rb rb_ref;
	int ret;
	char buffer[4];

	/* initialization */
	memset(&rb_ref, 0, sizeof(rb_ref));
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);
	/* fake we have written 3 bytes then consumed 2 */
	ret = rs_rb_write_incr(&rb, 3);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = rs_rb_clean(&rb);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(memcmp(&rb, &rb_ref, sizeof(rb)), 0);

	/* error use cases */
	ret = rs_rb_clean(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_RB_GET_READ_PTR(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4];
	void *p;

	/* initialization */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);
	/* fake we have written 3 bytes then consumed 2 */
	ret = rs_rb_write_incr(&rb, 3);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	p = rs_rb_get_read_ptr(&rb);
	CU_ASSERT_EQUAL(p, buffer + 2);

	/* error use cases */
	p = rs_rb_get_read_ptr(NULL);
	CU_ASSERT_PTR_NULL(p);
}

static void testRS_RB_GET_READ_LENGTH(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4];
	size_t read_length;

	/* initialization */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);
	/* fake we have written 3 bytes then consumed 2 */
	ret = rs_rb_write_incr(&rb, 3);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	read_length = rs_rb_get_read_length(&rb);
	CU_ASSERT_EQUAL(read_length, 1);

	/* error use cases */
	read_length = rs_rb_get_read_length(NULL);
	CU_ASSERT_EQUAL(read_length, 0);
}

static void testRS_RB_GET_READ_LENGTH_NO_WRAP(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4];
	size_t read_length;

	/* initialization */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);
	/* fake we have written 3 bytes then consumed 2 */
	ret = rs_rb_write_incr(&rb, 3);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);
	/* fake we have written another 3 bytes, to produce a wrap */
	ret = rs_rb_write_incr(&rb, 3);

	/* normal use cases */
	read_length = rs_rb_get_read_length_no_wrap(&rb);
	CU_ASSERT_EQUAL(read_length, 2); /* from base + 2, to the end */

	/* error use cases */
	read_length = rs_rb_get_read_length_no_wrap(NULL);
	CU_ASSERT_EQUAL(read_length, 0);
}

static void testRS_RB_READ_INCR(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4];

	/* initialization */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = rs_rb_write_incr(&rb, 4);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_read_incr(&rb, 3);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(rb.read, 3);

	/* produces a wrap of 1 byte */
	ret = rs_rb_write_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(rb.read, 1);

	/* error use cases */
	ret = rs_rb_read_incr(NULL, 2);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	/* can't read more than length */
	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_RB_READ_AT(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4] = {1, 2, 3, 4};
	char value;

	/* initialization */
	/* fill buffer with data, wrapping from 2 to 2 */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_write_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_write_incr(&rb, 4);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = rs_rb_read_at(&rb, 0, &value);
	CU_ASSERT_EQUAL(value, 3);
	ret = rs_rb_read_at(&rb, 1, &value);
	CU_ASSERT_EQUAL(value, 4);
	ret = rs_rb_read_at(&rb, 2, &value);
	CU_ASSERT_EQUAL(value, 1);
	ret = rs_rb_read_at(&rb, 3, &value);
	CU_ASSERT_EQUAL(value, 2);

	ret = rs_rb_read_incr(&rb, 1);
	CU_ASSERT_EQUAL(ret, 0);
	/* error use cases */
	ret = rs_rb_read_at(NULL, 0, &value);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	/* impossible to read at an offset greater than len (which is 3 here) */
	ret = rs_rb_read_at(&rb, 3, &value);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_rb_read_at(&rb, 0, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_RB_GET_WRITE_PTR(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4] = {1, 2, 3, 4};
	void *p;

	/* initialization */
	/* fill buffer with data, wrapping from 2 to 2 */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_write_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	p = rs_rb_get_write_ptr(&rb);
	CU_ASSERT_PTR_EQUAL(p, (char *)rb.base + 2);

	ret = rs_rb_write_incr(&rb, 3);
	CU_ASSERT_EQUAL(ret, 0);
	p = rs_rb_get_write_ptr(&rb);
	CU_ASSERT_PTR_EQUAL(p, (char *)rb.base + 1);

	/* error use cases */
	p = rs_rb_get_write_ptr(NULL);
	CU_ASSERT_PTR_NULL(p);
}

static void testRS_RB_GET_WRITE_LENGTH(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4];
	size_t write_length;

	/* initialization */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	write_length = rs_rb_get_write_length(&rb);
	CU_ASSERT_EQUAL(write_length, 4);

	ret = rs_rb_write_incr(&rb, 3);
	CU_ASSERT_EQUAL(ret, 0);
	write_length = rs_rb_get_write_length(&rb);
	CU_ASSERT_EQUAL(write_length, 1);

	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);
	write_length = rs_rb_get_write_length(&rb);
	CU_ASSERT_EQUAL(write_length, 3);

	ret = rs_rb_write_incr(&rb, 3);
	CU_ASSERT_EQUAL(ret, 0);
	write_length = rs_rb_get_write_length(&rb);
	CU_ASSERT_EQUAL(write_length, 0);

	/* error use cases */
	write_length = rs_rb_get_write_length(NULL);
	CU_ASSERT_EQUAL(write_length, 0);
}

static void testRS_RB_GET_WRITE_LENGTH_NO_WRAP(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4];
	size_t write_length;

	/* initialization */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	write_length = rs_rb_get_write_length_no_wrap(&rb);
	CU_ASSERT_EQUAL(write_length, 4);

	ret = rs_rb_write_incr(&rb, 3);
	CU_ASSERT_EQUAL(ret, 0);
	write_length = rs_rb_get_write_length_no_wrap(&rb);
	CU_ASSERT_EQUAL(write_length, 1);

	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);
	write_length = rs_rb_get_write_length_no_wrap(&rb);
	CU_ASSERT_EQUAL(write_length, 1);

	ret = rs_rb_write_incr(&rb, 1);
	CU_ASSERT_EQUAL(ret, 0);
	write_length = rs_rb_get_write_length_no_wrap(&rb);
	CU_ASSERT_EQUAL(write_length, 2);

	/* error use cases */
	write_length = rs_rb_get_write_length_no_wrap(NULL);
	CU_ASSERT_EQUAL(write_length, 0);
}

static void testRS_RB_WRITE_INCR(void)
{
	struct rs_rb rb;
	int ret;
	char buffer[4];

	/* initialization */
	ret = rs_rb_init(&rb, buffer, 4);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = rs_rb_write_incr(&rb, 4);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(rb.write, 0);
	ret = rs_rb_read_incr(&rb, 3);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(rb.read, 3);
	CU_ASSERT_EQUAL(rb.write, 0);

	/* produces a wrap of 1 byte */
	ret = rs_rb_write_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(rb.write, 2);
	ret = rs_rb_read_incr(&rb, 2);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(rb.read, 1);
	CU_ASSERT_EQUAL(rb.write, 2);

	/* error use cases */
	ret = rs_rb_write_incr(NULL, 2);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	/* can't write more than remaining room (here : 3) */
	ret = rs_rb_write_incr(&rb, 4);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static const struct test_t tests[] = {
		{
				.fn = testRS_RB_INIT,
				.name = "rs_rb_init"
		},
		{
				.fn = testRS_RB_GET_SIZE,
				.name = "rs_rb_get_size"
		},
		{
				.fn = testRS_RB_EMPTY,
				.name = "rs_rb_empty"
		},
		{
				.fn = testRS_RB_CLEAN,
				.name = "rs_rb_clean"
		},
		{
				.fn = testRS_RB_GET_READ_PTR,
				.name = "rs_rb_get_read_ptr"
		},
		{
				.fn = testRS_RB_GET_READ_LENGTH,
				.name = "rs_rb_get_read_length"
		},
		{
				.fn = testRS_RB_GET_READ_LENGTH_NO_WRAP,
				.name = "rs_rb_get_read_length_no_wrap"
		},
		{
				.fn = testRS_RB_READ_INCR,
				.name = "rs_rb_read_incr"
		},
		{
				.fn = testRS_RB_READ_AT,
				.name = "rs_rb_read_at"
		},
		{
				.fn = testRS_RB_GET_WRITE_PTR,
				.name = "rs_rb_get_write_ptr"
		},
		{
				.fn = testRS_RB_GET_WRITE_LENGTH,
				.name = "rs_rb_get_write_length"
		},
		{
				.fn = testRS_RB_GET_WRITE_LENGTH_NO_WRAP,
				.name = "rs_rb_get_write_length_no_wrap"
		},
		{
				.fn = testRS_RB_WRITE_INCR,
				.name = "rs_rb_write_incr"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_rb_suite(void)
{
	return 0; /* return non-zero on error */
}

static int clean_rb_suite(void)
{
	return 0; /* return non-zero on error */
}

struct suite_t rb_suite = {
		.name = "rs_rb",
		.init = init_rb_suite,
		.clean = clean_rb_suite,
		.tests = tests,
};

