/**
 * @file dll_test.c
 * @date 30 juil. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for rs_dll module
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <limits.h>
#include <inttypes.h>
#include <stddef.h>

#include <CUnit/Basic.h>

#include "../include/rs_dll.h"

#include <fautes.h>
#include <fautes_utils.h>

struct int_node {
	struct rs_node node;
	int val;
};

#define to_int_node(p) rs_container_of(p, struct int_node, node)

static int dll_test_equals(struct rs_node *a, const struct rs_node *b)
{
	struct int_node *int_node_a = to_int_node(a);
	struct int_node *int_node_b = to_int_node(b);

	if (NULL == a || NULL == b)
		return 0;

	return 0 == (int_node_a->val - int_node_b->val);
}

static int dll_test_compare(struct rs_node *a, const struct rs_node *b)
{
	struct int_node *int_node_a = to_int_node(a);
	struct int_node *int_node_b = to_int_node(b);

	if (NULL == a || NULL == b)
		return 0;

	return int_node_a->val - int_node_b->val;
}

static void dll_test_print(struct rs_node *node)
{
	struct int_node *int_node = to_int_node(node);

	fprintf(stderr, "| [%p] %d (p %p, n %p) ", node,
			int_node->val,
			node->prev, node->next);
}

static int dll_test_remove(struct rs_node *node)
{
	struct int_node *int_node = to_int_node(node);

	int_node->val = INT_MAX;

	return 0;
}

static const struct rs_dll_vtable dll_test_vtable = {
		.compare = dll_test_compare,
		.equals = dll_test_equals,
		.print = dll_test_print,
		.remove = dll_test_remove,
};

static void testRS_DLL_INIT(void)
{
	struct rs_dll dll;
	int ret = 0;

	/* normal use cases */
	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(dll.count, 0);
	CU_ASSERT_EQUAL(dll.cur, NULL);
	CU_ASSERT_EQUAL(dll.head, NULL);
	CU_ASSERT_EQUAL(dll.vtable.equals, dll_test_equals);
	CU_ASSERT_PTR_NOT_NULL(dll.vtable.print);
	ret = rs_dll_init(&dll, NULL);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_PTR_NOT_NULL(dll.vtable.equals);
	CU_ASSERT_PTR_NOT_NULL(dll.vtable.print);

	/* error use case */
	ret = rs_dll_init(NULL, &dll_test_vtable);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_DLL_PUSH(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_dll dll;
	int ret = 0;
	int two_times_cb(struct rs_node *node)
	{
		struct int_node *in = to_int_node(node);

		CU_ASSERT_PTR_NOT_NULL(node);

		in->val *= 2;

		return 0;
	};

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_foreach(&dll, two_times_cb);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(int_node_a.val, 34);
	CU_ASSERT_EQUAL(int_node_b.val, 84);
	CU_ASSERT_EQUAL(int_node_c.val, 1332);

	/* error use case */
	ret = rs_dll_push(NULL, &(int_node_a.node));
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_dll_push(&dll, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

#define DUMP_DLL(d) do { \
	fprintf(stderr, "list content : "); \
	rs_dll_dump((d)); \
	fprintf(stderr, "| head %p, tail %p\n", (d)->head, (d)->tail); \
} while (0)


static void testRS_DLL_ENQUEUE(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_dll dll;
	int ret = 0;
	int two_times_cb(struct rs_node *node, void *data)
	{
		struct int_node *in = to_int_node(node);

		CU_ASSERT_EQUAL_FATAL((uint32_t)data, 0xdeadbeef);
		CU_ASSERT_PTR_NOT_NULL(node);

		in->val *= 2;

		return 0;
	};

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	CU_ASSERT_EQUAL(dll.count, 0);

	/* normal use cases */
	ret = rs_dll_enqueue(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	CU_ASSERT_EQUAL(dll.count, 1);
	ret = rs_dll_enqueue(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	CU_ASSERT_EQUAL(dll.count, 2);
	ret = rs_dll_enqueue(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	CU_ASSERT_EQUAL(dll.count, 3);

	CU_ASSERT_EQUAL(dll.head, &(int_node_a.node));
	CU_ASSERT_PTR_NULL(dll.head->prev);
	CU_ASSERT_EQUAL(dll.head->next, &(int_node_b.node));
	CU_ASSERT_EQUAL(dll.head->next->prev, &(int_node_a.node));
	CU_ASSERT_EQUAL(dll.head->next->next, &(int_node_c.node));
	CU_ASSERT_EQUAL(dll.head->next->next->prev, &(int_node_b.node));
	CU_ASSERT_PTR_NULL(dll.head->next->next->next);
	rs_dll_pop(&dll);
	CU_ASSERT_EQUAL(dll.head, &(int_node_b.node));
	CU_ASSERT_EQUAL(dll.count, 2);
	rs_dll_pop(&dll);
	CU_ASSERT_EQUAL(dll.head, &(int_node_c.node));
	CU_ASSERT_EQUAL(dll.count, 1);
	rs_dll_pop(&dll);
	CU_ASSERT_PTR_NULL(dll.head);
	CU_ASSERT_EQUAL(dll.count, 0);

	/* error use case */
	ret = rs_dll_enqueue(NULL, &(int_node_a.node));
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_dll_enqueue(&dll, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_DLL_INSERT_SORTED(void)
{
	struct int_node int_node_a = {.val = 1,};
	struct int_node int_node_b = {.val = 0,};
	struct int_node int_node_c = {.val = 3,};
	struct int_node int_node_d = {.val = 2,};
	struct rs_dll dll;
	struct rs_node *node;
	int ret = 0;

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	ret = rs_dll_insert_sorted(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_dll_insert_sorted(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_dll_insert_sorted(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL(ret, 0);
	ret = rs_dll_insert_sorted(&dll, &(int_node_d.node));
	CU_ASSERT_EQUAL(ret, 0);

	CU_ASSERT_EQUAL(rs_dll_get_count(&dll), 4);

	/* check the list is really well ordered */
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 0);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 1);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 2);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 3);

	/* error use case */
}

static void testRS_DLL_GET_COUNT(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	unsigned count;
	struct rs_dll dll;
	int ret = 0;

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	count = rs_dll_get_count(&dll);
	CU_ASSERT_EQUAL(count, 0);

	rs_dll_push(&dll, &(int_node_a.node));
	count = rs_dll_get_count(&dll);
	CU_ASSERT_EQUAL(count, 1);

	rs_dll_push(&dll, &(int_node_b.node));
	count = rs_dll_get_count(&dll);
	CU_ASSERT_EQUAL(count, 2);

	rs_dll_push(&dll, &(int_node_c.node));
	count = rs_dll_get_count(&dll);
	CU_ASSERT_EQUAL(count, 3);

	/* error use case */
	count = rs_dll_get_count(NULL);
	CU_ASSERT_EQUAL(count, UINT_MAX);
}

static void testRS_DLL_IS_EMPTY(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	int result;
	struct rs_dll dll;
	int ret = 0;

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	result = rs_dll_is_empty(&dll);
	CU_ASSERT(result);

	rs_dll_push(&dll, &(int_node_a.node));
	result = rs_dll_is_empty(&dll);
	CU_ASSERT_FALSE(result);

	rs_dll_push(&dll, &(int_node_b.node));
	result = rs_dll_is_empty(&dll);
	CU_ASSERT_FALSE(result);

	rs_dll_pop(&dll);
	rs_dll_pop(&dll);
	result = rs_dll_is_empty(&dll);
	CU_ASSERT(result);

	/* error use case */
	result = rs_dll_is_empty(NULL);
	CU_ASSERT_FALSE(result);
}

static void testRS_DLL_FIND(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct int_node int_needle;
	struct rs_node *node;
	struct rs_dll dll;
	int ret = 0;

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	int_needle.val = 42;
	node = rs_dll_find(&dll, &(int_needle.node));
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);
	int_needle.val = 17;
	node = rs_dll_find(&dll, &(int_needle.node));
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);
	int_needle.val = 666;
	node = rs_dll_find(&dll, &(int_needle.node));
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	int_needle.val = 421;
	node = rs_dll_find(&dll, &(int_needle.node));
	CU_ASSERT_PTR_NULL(node);

	/* error use case */
	node = rs_dll_find(NULL, &(int_needle.node));
	CU_ASSERT_PTR_NULL(node);
	node = rs_dll_find(&dll, NULL);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_FIND_MATCH(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	int match_cb(struct rs_node *node, const void *data)
	{
		int searched_value = (int)data;
		struct int_node *int_node = to_int_node(node);

		return int_node->val == searched_value;
	}
	struct rs_node *node;
	struct rs_dll dll;
	int ret = 0;

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	node = rs_dll_find_match(&dll, match_cb, (void *)42);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);
	node = rs_dll_find_match(&dll, match_cb, (void *)17);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);
	node = rs_dll_find_match(&dll, match_cb, (void *)666);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	node = rs_dll_find_match(&dll, match_cb, (void *)421);
	CU_ASSERT_PTR_NULL(node);

	/* error use case */
	node = rs_dll_find_match(NULL, match_cb, (void *)17);
	CU_ASSERT_PTR_NULL(node);
	node = rs_dll_find_match(&dll, NULL, (void *)17);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_POP(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *node;
	struct rs_dll dll;
	int ret = 0;

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	CU_ASSERT_PTR_EQUAL(dll.tail, &(int_node_a.node));
	CU_ASSERT_PTR_EQUAL(dll.tail->prev, &(int_node_b.node));
	CU_ASSERT_PTR_EQUAL(dll.tail->prev->prev, &(int_node_c.node));

	/* normal use cases */
	node = rs_dll_pop(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	CU_ASSERT_PTR_EQUAL(dll.tail, &(int_node_a.node));
	node = rs_dll_pop(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);
	CU_ASSERT_PTR_EQUAL(dll.tail, &(int_node_a.node));
	node = rs_dll_pop(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);
	node = rs_dll_pop(&dll);
	CU_ASSERT_PTR_NULL(node);
	CU_ASSERT_PTR_NULL(dll.tail);

	/* error use case */
	node = rs_dll_pop(NULL);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_NEXT(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *node;
	struct rs_dll dll;
	int ret = 0;

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);
	node = rs_dll_next(&dll);
	CU_ASSERT_PTR_NULL(node);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);

	/* error use case */
	node = rs_dll_next(NULL);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_NEXT_FROM(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *node = NULL;
	struct rs_dll dll;
	int ret = 0;

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	node = rs_dll_next_from(&dll, node);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	node = rs_dll_next_from(&dll, node);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);
	node = rs_dll_next_from(&dll, node);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);
	node = rs_dll_next_from(&dll, node);
	CU_ASSERT_PTR_NULL(node);
	node = rs_dll_next_from(&dll, node);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);

	/* error use case */
	node = rs_dll_next_from(NULL, node);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_REWIND(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *node;
	struct rs_dll dll;
	int ret = 0;

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	ret = rs_dll_rewind(&dll);
	CU_ASSERT_EQUAL(ret, 0);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);

	ret = rs_dll_rewind(&dll);
	CU_ASSERT_EQUAL(ret, 0);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);

	ret = rs_dll_rewind(&dll);
	CU_ASSERT_EQUAL(ret, 0);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);

	ret = rs_dll_rewind(&dll);
	CU_ASSERT_EQUAL(ret, 0);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);

	/* error use case */
	ret = rs_dll_rewind(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_DLL_REMOVE(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct int_node int_node_needle = {.val = 42,};
	struct rs_node *node;
	struct rs_dll dll;
	int ret = 0;

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	node = rs_dll_remove(&dll, &(int_node_needle.node));
	CU_ASSERT_PTR_NOT_NULL_FATAL(node);

	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);

	int_node_needle.val = 17;
	node = rs_dll_remove(&dll, &(int_node_needle.node));
	CU_ASSERT_PTR_NOT_NULL_FATAL(node);

	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);

	int_node_needle.val = 666;
	node = rs_dll_remove(&dll, &(int_node_needle.node));
	CU_ASSERT_PTR_NOT_NULL_FATAL(node);

	node = rs_dll_next(&dll);
	CU_ASSERT_PTR_NULL(node);

	/* error use case */
	node = rs_dll_remove(NULL, &(int_node_needle.node));
	CU_ASSERT_PTR_NULL(node);
	node = rs_dll_remove(&dll, NULL);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_REMOVE_MATCH(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_node *node;
	int odd;
	struct rs_dll dll;
	int ret = 0;
	int parity_cb(struct rs_node *n, const void *data)
	{
		const int my_odd = *((const int *)data);
		struct int_node *int_node = to_int_node(n);

		if (my_odd)
			return int_node->val % 2;
		else
			return !(int_node->val % 2);
	};

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	/* remove the first even node */
	odd = 0;
	node = rs_dll_remove_match(&dll, parity_cb, &odd);
	CU_ASSERT_PTR_NOT_NULL_FATAL(node);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);

	/* remove the first odd node */
	odd = 1;
	node = rs_dll_remove_match(&dll, parity_cb, &odd);
	CU_ASSERT_PTR_NOT_NULL_FATAL(node);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);

	/* error use case */
	node = rs_dll_remove_match(NULL, parity_cb, &odd);
	CU_ASSERT_PTR_NULL(node);
	node = rs_dll_remove_match(&dll, NULL, &odd);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_FOREACH(void)
{
	struct rs_dll dll;
	int ret = 0;
	int cb(struct rs_node __attribute__((unused))*node)
	{
		/* do nothing */
		return 0;
	};

	/* normal use case is tested in testRS_DLL_PUSH */

	/* error use cases */
	ret = rs_dll_foreach(NULL, cb);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = rs_dll_foreach(&dll, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_DLL_REMOVE_ALL_CB(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_dll dll;
	int ret = 0;
	int cleanup_cb(struct rs_node *n)
	{
		struct int_node *int_node = to_int_node(n);

		int_node->val = -1;

		return 0;
	};

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	/* on an empty list */
	ret = rs_dll_remove_all_cb(&dll, cleanup_cb);
	CU_ASSERT_EQUAL(ret, 0);
	/* on a list with one single element */
	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_remove_all_cb(&dll, cleanup_cb);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(int_node_a.val, -1);
	/* with the complete one */
	int_node_a.val = 17;
	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_remove_all_cb(&dll, cleanup_cb);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(int_node_a.val, -1);
	CU_ASSERT_EQUAL(int_node_b.val, -1);
	CU_ASSERT_EQUAL(int_node_c.val, -1);
	CU_ASSERT_EQUAL(rs_dll_get_count(&dll), 0);
	/* no problem when calling it twice */
	ret = rs_dll_remove_all_cb(&dll, cleanup_cb);
	CU_ASSERT_EQUAL(ret, 0);
	/* no problem with no cb */
	ret = rs_dll_remove_all_cb(&dll, NULL);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use case */
	ret = rs_dll_remove_all_cb(NULL, cleanup_cb);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testRS_DLL_REMOVE_ALL(void)
{
	struct int_node int_node_a = {.val = 17,};
	struct int_node int_node_b = {.val = 42,};
	struct int_node int_node_c = {.val = 666,};
	struct rs_dll dll;
	int ret = 0;

	ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(ret, 0);

	/* normal use cases */
	/* on an empty list */
	ret = rs_dll_remove_all(&dll);
	CU_ASSERT_EQUAL(ret, 0);
	/* on a list with one single element */
	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_remove_all(&dll);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(int_node_a.val, INT_MAX);
	/* with the complete one */
	int_node_a.val = 17;
	ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(ret, 0);
	ret = rs_dll_remove_all(&dll);
	CU_ASSERT_EQUAL(ret, 0);
	CU_ASSERT_EQUAL(int_node_a.val, INT_MAX);
	CU_ASSERT_EQUAL(int_node_b.val, INT_MAX);
	CU_ASSERT_EQUAL(int_node_c.val, INT_MAX);
	CU_ASSERT_EQUAL(rs_dll_get_count(&dll), 0);
	/* no problem when calling it twice */
	ret = rs_dll_remove_all(&dll);
	CU_ASSERT_EQUAL(ret, 0);

	/* error use case */
	ret = rs_dll_remove_all(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static const struct test_t tests[] = {
		{
				.fn = testRS_DLL_INIT,
				.name = "rs_dll_init"
		},
		{
				.fn = testRS_DLL_PUSH,
				.name = "rs_dll_push"
		},
		{
				.fn = testRS_DLL_ENQUEUE,
				.name = "rs_dll_enqueue"
		},
		{
				.fn = testRS_DLL_INSERT_SORTED,
				.name = "rs_dll_insert_sorted"
		},
		{
				.fn = testRS_DLL_GET_COUNT,
				.name = "rs_dll_get_count"
		},
		{
				.fn = testRS_DLL_IS_EMPTY,
				.name = "rs_dll_is_empty"
		},
		{
				.fn = testRS_DLL_FIND,
				.name = "rs_dll_find"
		},
		{
				.fn = testRS_DLL_FIND_MATCH,
				.name = "rs_dll_find_match"
		},
		{
				.fn = testRS_DLL_POP,
				.name = "rs_dll_pop"
		},
		{
				.fn = testRS_DLL_NEXT,
				.name = "rs_dll_next"
		},
		{
				.fn = testRS_DLL_NEXT_FROM,
				.name = "rs_dll_next_from"
		},
		{
				.fn = testRS_DLL_REWIND,
				.name = "rs_dll_rewind"
		},
		{
				.fn = testRS_DLL_REMOVE,
				.name = "rs_dll_remove"
		},
		{
				.fn = testRS_DLL_REMOVE_MATCH,
				.name = "rs_dll_remove_match"
		},
		{
				.fn = testRS_DLL_FOREACH,
				.name = "rs_dll_foreach"
		},
		{
				.fn = testRS_DLL_REMOVE_ALL_CB,
				.name = "rs_dll_remove_all_cb"
		},
		{
				.fn = testRS_DLL_REMOVE_ALL,
				.name = "rs_dll_remove_all"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_dll_suite(void)
{
	return 0;
}

static int clean_dll_suite(void)
{

	return 0;
}

struct suite_t dll_suite = {
		.name = "rs_dll",
		.init = init_dll_suite,
		.clean = clean_dll_suite,
		.tests = tests,
};
