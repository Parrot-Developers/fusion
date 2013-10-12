/**
 * @file node_test.c
 * @date 30 juil. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for rs_node module
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <limits.h>
#include <inttypes.h>
#include <stddef.h>

#include <CUnit/Basic.h>

#include <rs_node.h>

#include <fautes.h>
#include <fautes_utils.h>

struct int_node {
	struct rs_node node;
	int val;
};

struct str_node {
	struct rs_node node;
	char *val;
};

#define to_int_node(p) rs_container_of(p, struct int_node, node)

static int int_node_test_equals(struct rs_node *node_a, void *int_node_b)
{
	struct int_node *int_node_a = to_int_node(node_a);

	if (NULL == node_a || NULL == int_node_b)
		return 0;

	return 0 == (int_node_a->val - ((struct int_node *)int_node_b)->val);
}

/*
 * defines match_str_val function, waits for a pointer to the comparison data
 * as the second argument
 */
static RS_NODE_MATCH_STR_MEMBER(str_node, val, node)

/*
 * defines match_val function, beware ! waits for a char * as the second
 * argument
 */
static RS_NODE_MATCH_MEMBER(struct int_node, val, node)

static void testRS_NODE_HEAD(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct rs_node *list = NULL;
	struct rs_node *head = NULL;
	int err;

	/* normal use cases */
	head = rs_node_head(list);
	CU_ASSERT_PTR_NULL(head);

	err = rs_node_push(&list, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(err, 0);
	head = rs_node_head(list);
	CU_ASSERT_EQUAL(head, &(int_node_a.node));

	err = rs_node_push(&list, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(err, 0);
	head = rs_node_head(list);
	CU_ASSERT_EQUAL(head, &(int_node_b.node));

	/* error cases : none */
}

static void testRS_NODE_INSERT_BEFORE(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *list = NULL;
	struct rs_node *tmp = &(int_node_a.node);

	/* normal use cases */
	/* inserting to a NULL (empty) list returns a list composed of node */
	list = rs_node_insert_before(list, &(int_node_a.node));
	CU_ASSERT_EQUAL(list, &(int_node_a.node));
	CU_ASSERT_EQUAL(to_int_node(list)->val, 17);
	CU_ASSERT_PTR_NULL(list->next);
	CU_ASSERT_PTR_NULL(list->prev);

	list = rs_node_insert_before(list, &(int_node_b.node));
	CU_ASSERT_EQUAL(list, &(int_node_b.node));
	CU_ASSERT_EQUAL(to_int_node(list)->val, 42);
	CU_ASSERT_PTR_NULL(list->next->next);
	CU_ASSERT_EQUAL(list->next, &(int_node_a.node));
	CU_ASSERT_PTR_NULL(list->next->next);
	CU_ASSERT_EQUAL(list->next->prev, &(int_node_b.node));
	CU_ASSERT_PTR_NULL(list->next->prev->prev);



	list = rs_node_insert_before(list, &(int_node_c.node));
	CU_ASSERT_EQUAL(list, &(int_node_c.node));
	CU_ASSERT_EQUAL(to_int_node(list)->val, 666);

	/* inserting a NULL element returns the initial list unchanged */
	tmp = rs_node_insert_before(list, NULL);
	CU_ASSERT_EQUAL(list, tmp);

	/* error cases : none */
}

static void testRS_NODE_PUSH(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *list = NULL;
	int err;

	/* normal use cases */
	err = rs_node_push(&list, &(int_node_a.node));
	CU_ASSERT_EQUAL(err, 0);
	CU_ASSERT_EQUAL(to_int_node(list)->val, 17);
	err = rs_node_push(&list, &(int_node_b.node));
	CU_ASSERT_EQUAL(err, 0);
	CU_ASSERT_EQUAL(to_int_node(list)->val, 42);
	err = rs_node_push(&list, &(int_node_c.node));
	CU_ASSERT_EQUAL(err, 0);
	CU_ASSERT_EQUAL(to_int_node(list)->val, 666);

	/* pushing nothing, well... does nothing */
	err = rs_node_push(&list, NULL);
	CU_ASSERT_EQUAL(err, 0);

	/* error cases */
	err = rs_node_push(NULL, &(int_node_a.node));
	CU_ASSERT_NOT_EQUAL(err, 0);
}

static void testRS_NODE_POP(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *list = NULL;
	struct rs_node *node = NULL;

	rs_node_push(&list, &(int_node_a.node));
	rs_node_push(&list, &(int_node_b.node));
	rs_node_push(&list, &(int_node_c.node));

	/* normal use cases */
	node = rs_node_pop(&list);
	CU_ASSERT_EQUAL(node, &(int_node_c.node));
	CU_ASSERT_EQUAL(list, &(int_node_b.node));
	node = rs_node_pop(&list);
	CU_ASSERT_EQUAL(node, &(int_node_b.node));
	CU_ASSERT_EQUAL(list, &(int_node_a.node));
	node = rs_node_pop(&list);
	CU_ASSERT_EQUAL(node, &(int_node_a.node));
	CU_ASSERT_PTR_NULL(list);
	node = rs_node_pop(&list);
	CU_ASSERT_PTR_NULL(node);

	/* error cases : none */
}

static void testRS_NODE_COUNT(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *list = NULL;
	unsigned count;

	/* normal use cases */
	count = rs_node_count(list);
	CU_ASSERT_EQUAL(0, count);
	rs_node_push(&list, &(int_node_a.node));
	count = rs_node_count(list);
	CU_ASSERT_EQUAL(1, count);
	rs_node_push(&list, &(int_node_b.node));
	count = rs_node_count(list);
	CU_ASSERT_EQUAL(2, count);
	rs_node_push(&list, &(int_node_c.node));
	count = rs_node_count(list);
	CU_ASSERT_EQUAL(3, count);

	/* count from middle of the list is possible */
	count = rs_node_count(&(int_node_a.node));
	CU_ASSERT_EQUAL(1, count);
	count = rs_node_count(&(int_node_b.node));
	CU_ASSERT_EQUAL(2, count);
	count = rs_node_count(&(int_node_c.node));
	CU_ASSERT_EQUAL(3, count);

	/* error cases : none */
}

static void testRS_NODE_NEXT(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *list = NULL;
	struct rs_node *node = NULL;

	rs_node_push(&list, &(int_node_a.node));
	rs_node_push(&list, &(int_node_b.node));
	rs_node_push(&list, &(int_node_c.node));

	/* normal use cases */
	CU_ASSERT_EQUAL(to_int_node(list)->val, 666);
	node = rs_node_next(list);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);
	node = rs_node_next(node);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);
	node = rs_node_next(node);
	CU_ASSERT_PTR_NULL(node);

	/* error cases : none */
}

static void testRS_NODE_PREV(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *list = NULL;
	struct rs_node *node = NULL;

	rs_node_push(&list, &(int_node_a.node));
	rs_node_push(&list, &(int_node_b.node));
	rs_node_push(&list, &(int_node_c.node));

	/* normal use cases */
	node = rs_node_prev(NULL);
	CU_ASSERT_PTR_NULL(node);
	node = rs_node_prev(&(int_node_a.node));
	CU_ASSERT(int_node_test_equals(node, &int_node_b));
	node = rs_node_prev(&(int_node_b.node));
	CU_ASSERT(int_node_test_equals(node, &int_node_c));
	node = rs_node_prev(&(int_node_c.node));
	CU_ASSERT_PTR_NULL(node);

	/* error cases : none */
}

static void testRS_NODE_FIND(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *haystack = NULL;
	struct rs_node *needle = NULL;

	rs_node_push(&haystack, &(int_node_a.node));
	rs_node_push(&haystack, &(int_node_b.node));
	rs_node_push(&haystack, &(int_node_c.node));

	/* normal use cases */
	needle = rs_node_find(NULL, &(int_node_a.node));
	CU_ASSERT_PTR_NULL(needle);

	needle = rs_node_find(haystack, &(int_node_a.node));
	CU_ASSERT_EQUAL(needle, &(int_node_a.node));
	needle = rs_node_find(haystack, &(int_node_b.node));
	CU_ASSERT_EQUAL(needle, &(int_node_b.node));
	needle = rs_node_find(haystack, &(int_node_c.node));
	CU_ASSERT_EQUAL(needle, &(int_node_c.node));

	needle = rs_node_find(haystack, NULL);
	CU_ASSERT_PTR_NULL(needle);

	/* error cases : none */
}

static void testRS_NODE_FIND_MATCH(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	int val;
	struct rs_node *haystack = NULL;
	struct rs_node *needle = NULL;

	rs_node_push(&haystack, &(int_node_a.node));
	rs_node_push(&haystack, &(int_node_b.node));
	rs_node_push(&haystack, &(int_node_c.node));

	/* normal use cases */
	val = 17;
	needle = rs_node_find_match(NULL, match_val, &val);
	CU_ASSERT_PTR_NULL(needle);

	val = 17;
	needle = rs_node_find_match(haystack, match_val, &val);
	CU_ASSERT(int_node_test_equals(needle, &int_node_a));
	val = 42;
	needle = rs_node_find_match(haystack, match_val, &val);
	CU_ASSERT(int_node_test_equals(needle, &int_node_b));
	val = 666;
	needle = rs_node_find_match(haystack, match_val, &val);
	CU_ASSERT(int_node_test_equals(needle, &int_node_c));

	needle = rs_node_find_match(haystack, match_val, &val);
	CU_ASSERT(int_node_test_equals(needle, &int_node_c));

	needle = rs_node_find_match(haystack, NULL, &val);
	CU_ASSERT_PTR_NULL(needle);

	needle = rs_node_find_match(haystack, match_val, NULL);
	CU_ASSERT_PTR_NULL(needle);

	/* NULL data is possible, useful for nested matching functions */
	int match(struct rs_node *node, const void *data)
	{
		struct int_node *int_node = to_int_node(node);
		return int_node->val == 666;
	}
	needle = rs_node_find_match(haystack, match, NULL);
	CU_ASSERT(int_node_test_equals(needle, &int_node_c));

	/* error cases : none */
}

static void testRS_NODE_FIND_MATCH_str(void)
{
	struct str_node str_node_a = {.val = "17",};
	struct str_node str_node_b = {.val = "42",};
	struct str_node str_node_c = {.val = "666",};
	char *val;
	struct rs_node *haystack = NULL;
	struct rs_node *needle = NULL;

	rs_node_push(&haystack, &(str_node_a.node));
	rs_node_push(&haystack, &(str_node_b.node));
	rs_node_push(&haystack, &(str_node_c.node));

	/* normal use cases */
	val = "17";
	needle = rs_node_find_match(NULL, str_node_match_str_val, &val);
	CU_ASSERT_PTR_NULL(needle);

	val = "17";
	needle = rs_node_find_match(haystack, str_node_match_str_val, val);
	CU_ASSERT_PTR_EQUAL(needle, &str_node_a.node);
	val = "42";
	needle = rs_node_find_match(haystack, str_node_match_str_val, val);
	CU_ASSERT_PTR_EQUAL(needle, &str_node_b.node);
	val = "666";
	needle = rs_node_find_match(haystack, str_node_match_str_val, val);
	CU_ASSERT_PTR_EQUAL(needle, &str_node_c.node);

	needle = rs_node_find_match(haystack, str_node_match_str_val, val);
	CU_ASSERT_PTR_EQUAL(needle, &str_node_c.node);

	needle = rs_node_find_match(haystack, str_node_match_str_val, NULL);
	CU_ASSERT_PTR_NULL(needle);

	/* error cases : none */
}

static void testRS_NODE_REMOVE(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct int_node int_node_unknown = {.val = 666,};
	struct rs_node *list = NULL;
	struct rs_node *node = NULL;

	rs_node_push(&list, &(int_node_a.node));
	rs_node_push(&list, &(int_node_b.node));
	rs_node_push(&list, &(int_node_c.node));

	/* normal use cases */
	node = rs_node_remove(list, &(int_node_b.node));
	CU_ASSERT(int_node_test_equals(node, &int_node_b));

	node = rs_node_remove(list, &(int_node_unknown.node));
	CU_ASSERT_PTR_NULL(node);

	node = rs_node_remove(list, &(int_node_c.node));
	CU_ASSERT(int_node_test_equals(node, &int_node_c));

	node = rs_node_remove(NULL, &(int_node_a.node));
	CU_ASSERT_PTR_NULL(node);

	node = rs_node_remove(list, NULL);
	CU_ASSERT_PTR_NULL(node);

	/* error cases : none */
}

static void testRS_NODE_REMOVE_MATCH(void)
{
	int val;
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *list = NULL;
	struct rs_node *node = NULL;

	rs_node_push(&list, &(int_node_a.node));
	rs_node_push(&list, &(int_node_b.node));
	rs_node_push(&list, &(int_node_c.node));

	/* normal use cases */
	val = 42;
	node = rs_node_remove_match(list, match_val, &val);
	CU_ASSERT(int_node_test_equals(node, &int_node_b));

	val = 911;
	node = rs_node_remove_match(list, match_val, &val);
	CU_ASSERT_PTR_NULL(node);

	val = 666;
	node = rs_node_remove_match(list, match_val, &val);
	CU_ASSERT(int_node_test_equals(node, &int_node_c));

	val = 17;
	node = rs_node_remove_match(NULL, match_val, &val);
	CU_ASSERT_PTR_NULL(node);

	node = rs_node_remove_match(list, match_val, NULL);
	CU_ASSERT_PTR_NULL(node);

	node = rs_node_remove_match(list, NULL, &val);
	CU_ASSERT_PTR_NULL(node);

	/* error cases : none */
}

static void testRS_NODE_FOREACH(void)
{
	int val = 2;
	int ret;
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *list = NULL;
	int cb(struct rs_node *node)
	{
		struct int_node *in = to_int_node(node);

		in->val *= val;

		return 0;
	};

	rs_node_push(&list, &(int_node_a.node));
	rs_node_push(&list, &(int_node_b.node));
	rs_node_push(&list, &(int_node_c.node));

	/* normal use cases */
	ret = rs_node_foreach(list, cb);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(int_node_a.val, 34);
	CU_ASSERT_EQUAL(int_node_b.val, 84);
	CU_ASSERT_EQUAL(int_node_c.val, 1332);

	/* error use cases */
}

static void testRS_NODE_REMOVE_ALL(void)
{
	int ret;
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *list = NULL;
	int cb(struct rs_node *node)
	{
		struct int_node *in = to_int_node(node);

		in->val =0;

		return 0;
	};

	rs_node_push(&list, &(int_node_a.node));
	rs_node_push(&list, &(int_node_b.node));
	rs_node_push(&list, &(int_node_c.node));

	/* normal use cases */
	ret = rs_node_remove_all(&list, cb);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(int_node_a.val, 0);
	CU_ASSERT_EQUAL(int_node_b.val, 0);
	CU_ASSERT_EQUAL(int_node_c.val, 0);
	CU_ASSERT_PTR_NULL(list);
	CU_ASSERT_PTR_NULL(int_node_a.node.next);
	CU_ASSERT_PTR_NULL(int_node_a.node.prev);
	CU_ASSERT_PTR_NULL(int_node_b.node.next);
	CU_ASSERT_PTR_NULL(int_node_b.node.prev);
	CU_ASSERT_PTR_NULL(int_node_c.node.next);
	CU_ASSERT_PTR_NULL(int_node_c.node.prev);
	/* it's ok to call remove_all on an empty list */
	ret = rs_node_remove_all(&list, cb);
	CU_ASSERT_EQUAL(ret, 0);
	/*
	 * it's ok to call remove_all with no callback, all elements will only
	 * be unchained
	 */
	ret = rs_node_remove_all(&list, NULL);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use cases */
	ret = rs_node_remove_all(NULL, cb);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static const struct test_t tests[] = {
		{
				.fn = testRS_NODE_HEAD,
				.name = "rs_node_head"
		},
		{
				.fn = testRS_NODE_INSERT_BEFORE,
				.name = "rs_node_insert"
		},
		{
				.fn = testRS_NODE_PUSH,
				.name = "rs_node_push"
		},
		{
				.fn = testRS_NODE_POP,
				.name = "rs_node_pop"
		},
		{
				.fn = testRS_NODE_COUNT,
				.name = "rs_node_count"
		},
		{
				.fn = testRS_NODE_NEXT,
				.name = "rs_node_next"
		},
		{
				.fn = testRS_NODE_PREV,
				.name = "rs_node_prev"
		},
		{
				.fn = testRS_NODE_FIND,
				.name = "rs_node_find"
		},
		{
				.fn = testRS_NODE_FIND_MATCH,
				.name = "rs_node_find_match"
		},
		{
				.fn = testRS_NODE_FIND_MATCH_str,
				.name = "rs_node_find_match str"
		},
		{
				.fn = testRS_NODE_REMOVE,
				.name = "rs_node_remove"
		},
		{
				.fn = testRS_NODE_REMOVE_MATCH,
				.name = "rs_node_remove_match"
		},
		{
				.fn = testRS_NODE_FOREACH,
				.name = "rs_node_foreach"
		},
		{
				.fn = testRS_NODE_REMOVE_ALL,
				.name = "rs_node_remove_all"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_node_suite(void)
{
	return 0;
}

static int clean_node_suite(void)
{

	return 0;
}

struct suite_t node_suite = {
		.name = "rs_node",
		.init = init_node_suite,
		.clean = clean_node_suite,
		.tests = tests,
};
