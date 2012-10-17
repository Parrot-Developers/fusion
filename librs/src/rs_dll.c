/**
 * @file rs_dll.c
 * @date 30 juil. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Doubly linked list implementation
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <rs_dll.h>

/**
 * Default equality method, compares on nodes addresses
 * @param a First node
 * @param b Second node
 * @return 1 if a's and b's addresses are equal, non-zero otherwise
 */
static int default_equals(rs_node_t *a, const rs_node_t *b)
{
	return a == b;
}

/**
 * Default display method, displays the node address
 * @param node Node to display
 */
static void default_print(rs_node_t *node)
{
	fprintf(stderr, "%p\n", node);
};

static const rs_dll_vtable_t default_vtable = {
		.equals = default_equals,
		.print = default_print,
};

int rs_dll_init(rs_dll_t *dll, const rs_dll_vtable_t *vtable)
{
	if (NULL == dll)
		return -EINVAL;

	if (NULL == vtable)
		vtable = &default_vtable;
	dll->count = 0;
	dll->cur = NULL;
	dll->head = NULL;
	memcpy(&(dll->vtable), vtable, sizeof(*vtable));
	if (NULL == dll->vtable.equals)
		dll->vtable.equals = default_equals;
	if (NULL == dll->vtable.print)
		dll->vtable.print = default_print;

	return 0;
}

void rs_dll_dump(rs_dll_t *dll)
{
	int display(rs_node_t *node,
			__attribute__((unused)) void *data)
	{
		void (*print)(rs_node_t *node) = data;

		print(node);

		return 0;
	};

	rs_node_foreach(dll->head, display, dll->vtable.print);
}

int rs_dll_push(rs_dll_t *dll, rs_node_t *node)
{
	int err;

	if (NULL == dll || NULL == node)
		return -EINVAL;

	err = rs_node_push(&(dll->head), node);

	dll->count++;
	dll->cur = dll->head;

	return err ? -EINVAL : 0;
}

unsigned rs_dll_get_count(rs_dll_t *dll)
{
	return NULL == dll ? UINT_MAX : dll->count;
}

rs_node_t *rs_dll_find_match(rs_dll_t *dll, rs_node_match_cb_t match,
		void *data)
{
	if (NULL == dll || NULL == match)
		return NULL;

	return rs_node_find_match(dll->head, match, data);
}

rs_node_t *rs_dll_find(rs_dll_t *dll, rs_node_t *node)
{
	int match(rs_node_t *n, const void *data) {
		return dll->vtable.equals(n, data);
	};

	return rs_dll_find_match(dll, match, node);
}

rs_node_t *rs_dll_pop(rs_dll_t *dll)
{
	if (NULL == dll)
		return NULL;

	return rs_dll_remove(dll, dll->head);
}

rs_node_t *rs_dll_next(rs_dll_t *dll)
{
	rs_node_t *next;

	if (NULL == dll)
		return NULL;

	next = dll->cur;

	if (NULL == next)
		dll->cur = dll->head;
	else
		dll->cur = rs_node_next(next);

	return next;
}

rs_node_t *rs_dll_remove_match(rs_dll_t *dll,
		rs_node_match_cb_t match, void *data)
{
	rs_node_t *head_next_bkp = NULL;
	rs_node_t *needle;

	if (NULL == dll || NULL == match)
		return NULL;

	/* in case we remove the head, we want it to be replaced by it's next */
	head_next_bkp = rs_node_next(dll->head);

	needle = rs_node_remove_match(dll->head, match, data);
	if (NULL != needle) {
		/* keep list coherent */
		if (needle == dll->head)
			dll->head = head_next_bkp;
		dll->cur = dll->head;
		dll->count--;
	}

	return needle;
}

rs_node_t *rs_dll_remove(rs_dll_t *dll, rs_node_t *node)
{
	int match(rs_node_t *n, const void *data) {
		return dll->vtable.equals(n, data);
	};

	return rs_dll_remove_match(dll, match, node);
}

int rs_dll_foreach(rs_dll_t *dll, rs_node_cb_t cb, void *data)
{
	rs_node_t *n;
	int res = 0;

	if (NULL == cb || NULL == dll)
		return -EINVAL;

	for (n = dll->head; n && 0 == res; n = rs_node_next(n))
		res = cb(n, data);

	return res;
}

