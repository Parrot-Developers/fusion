/**
 * @file rs_hmap_test.c
 * @date 14 mai 2013
 * @author nicolas.carrier@parrot.com
 * @brief unit tests for librs hash map implementation
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <errno.h>

#include <CUnit/Basic.h>

#include <fautes.h>

#include <rs_hmap.h>

static void testRS_HMAP_INIT(void)
{
	int ret;
	struct rs_hmap map;

	/* normal use cases */
	ret = rs_hmap_init(&map, 10);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT(map.size >= 10);
	CU_ASSERT_PTR_NOT_NULL(map.buckets);
	rs_hmap_clean(&map);

	/* error use cases */
	ret = rs_hmap_init(&map, RS_HMAP_PRIME_MAX + 1U);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_hmap_init(NULL, 10);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_HMAP_CLEAN(void)
{
	int ret;
	struct rs_hmap map;

	/* normal use cases */
	ret = rs_hmap_init(&map, 10);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_hmap_clean(&map);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use cases */
	ret = rs_hmap_clean(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_HMAP_CLEAN_FREE(void)
{
	int ret;
	struct rs_hmap map;
	int *data1 = calloc(1, sizeof(int));
	int *data2 = calloc(1, sizeof(int));

	*data1 = 88;
	*data2 = 66;

	/* init */
	ret = rs_hmap_init(&map, 10);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_hmap_insert(&map, "ursule", data1);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_hmap_insert(&map, "gédéon", data2);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = rs_hmap_clean_cb(&map, free);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_hmap_clean_cb(&map, NULL);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use cases */
	ret = rs_hmap_clean_cb(NULL, free);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_HMAP_LOOKUP(void)
{
	int ret;
	struct rs_hmap map;
	void *data1 = (void *)42;
	void *data2 = (void *)66;
	void *needle = NULL;

	/* initialization */
	ret = rs_hmap_init(&map, 10);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_hmap_insert(&map, "ursule", data1);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_hmap_insert(&map, "gédéon", data2);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = rs_hmap_lookup(&map, "ursule", &needle);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_PTR_EQUAL(needle, data1);
	ret = rs_hmap_lookup(&map, "gédéon", &needle);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_PTR_EQUAL(needle, data2);
	ret = rs_hmap_lookup(&map, "frénégonde", &needle);
	CU_ASSERT_EQUAL(ret, -ENOENT);
	CU_ASSERT_PTR_EQUAL(needle, NULL);

	/* error use cases */
	ret = rs_hmap_lookup(NULL, "ursule", &needle);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_hmap_lookup(&map, NULL, &needle);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_hmap_lookup(&map, "", &needle);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_hmap_lookup(&map, "ursule", NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	rs_hmap_clean(&map);
}

static void testRS_HMAP_INSERT(void)
{
	int ret;
	struct rs_hmap map;
	void *needle = NULL;

	/* initialization */
	ret = rs_hmap_init(&map, 10);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	/* other use cases are already tested in lookup */
	/* inserting NULL is allowed */
	ret = rs_hmap_insert(&map, "ursule", NULL);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use cases */
	ret = rs_hmap_insert(NULL, "ursule", &needle);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_hmap_insert(&map, NULL, &needle);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_hmap_insert(&map, "", &needle);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	rs_hmap_clean(&map);
	/* searching in a hash map with no bucket must fail cleanly */
	ret = rs_hmap_insert(&map, "ursule", &needle);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_HMAP_REMOVE(void)
{
	int ret;
	struct rs_hmap map;
	void *data1 = (void *)42;
	void *data2 = (void *)66;
	void *needle = NULL;

	/* initialization */
	ret = rs_hmap_init(&map, 10);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_hmap_insert(&map, "ursule", data1);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_hmap_insert(&map, "gédéon", data2);
	CU_ASSERT_EQUAL(ret, 0);

	/* normal use cases */
	ret = rs_hmap_remove(&map, "ursule", &needle);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_PTR_EQUAL(needle, data1);
	/* if we wan't to drop data, needle can be NULL */
	ret = rs_hmap_remove(&map, "gédéon", NULL);
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_hmap_remove(&map, "ursule", &needle);
	CU_ASSERT_EQUAL(ret, -ENOENT);
	CU_ASSERT_PTR_EQUAL(needle, NULL);
	ret = rs_hmap_remove(&map, "frénégonde", &needle);
	CU_ASSERT_EQUAL(ret, -ENOENT);
	CU_ASSERT_PTR_EQUAL(needle, NULL);

	/* error use cases */
	ret = rs_hmap_remove(NULL, "ursule", &needle);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_hmap_remove(&map, NULL, &needle);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_hmap_remove(&map, "", &needle);
	CU_ASSERT_NOT_EQUAL(ret, 0);

	/* cleanup */
	rs_hmap_clean(&map);
}

static const struct test_t tests[] = {
		{
				.fn = testRS_HMAP_INIT,
				.name = "rs_hmap_init"
		},
		{
				.fn = testRS_HMAP_CLEAN,
				.name = "rs_hmap_clean"
		},
		{
				.fn = testRS_HMAP_CLEAN_FREE,
				.name = "rs_hmap_clean_free"
		},
		{
				.fn = testRS_HMAP_LOOKUP,
				.name = "rs_hmap_lookup"
		},
		{
				.fn = testRS_HMAP_INSERT,
				.name = "rs_hmap_insert"
		},
		{
				.fn = testRS_HMAP_REMOVE,
				.name = "rs_hmap_remove"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_hmap_suite(void)
{
	return 0; /* return non-zero on error */
}

static int clean_hmap_suite(void)
{
	return 0; /* return non-zero on error */
}

struct suite_t hmap_suite = {
		.name = "rs_hmap",
		.init = init_hmap_suite,
		.clean = clean_hmap_suite,
		.tests = tests,
};

