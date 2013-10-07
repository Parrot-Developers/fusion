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
static int default_equals(struct rs_node *a, const struct rs_node *b)
{
	return a == b;
}

/**
 * Default display method, displays the node address
 * @param node Node to display
 */
static void default_print(struct rs_node *node)
{
	fprintf(stderr, "%p\n", node);
};

static const struct rs_dll_vtable default_vtable = {
		.equals = default_equals,
		.print = default_print,
};

int rs_dll_init(struct rs_dll *dll, const struct rs_dll_vtable *vtable)
{
	if (NULL == dll)
		return -EINVAL;

	if (NULL == vtable)
		vtable = &default_vtable;
	dll->count = 0;
	dll->cur = NULL;
	dll->head = NULL;
	dll->tail = NULL;
	memcpy(&(dll->vtable), vtable, sizeof(*vtable));
	if (NULL == dll->vtable.equals)
		dll->vtable.equals = default_equals;
	if (NULL == dll->vtable.print)
		dll->vtable.print = default_print;

	return 0;
}

void rs_dll_dump(struct rs_dll *dll)
{
	int display(struct rs_node *node, __attribute__((unused)) void *unused)
	{
		dll->vtable.print(node);

		return 0;
	}

	rs_node_foreach(dll->head, display);
}

int rs_dll_push(struct rs_dll *dll, struct rs_node *node)
{
	int err;

	if (NULL == dll || NULL == node)
		return -EINVAL;

	if (rs_dll_is_empty(dll))
		dll->tail = node;

	err = rs_node_push(&(dll->head), node);

	dll->count++;
	dll->cur = dll->head;

	return err ? -EINVAL : 0;
}

int rs_dll_enqueue(struct rs_dll *dll, struct rs_node *node)
{
	if (NULL == dll || NULL == node)
		return -EINVAL;

	if (rs_dll_is_empty(dll)) {
		dll->head = dll->tail = node;
		node->next = node->prev = NULL;
		dll->cur = dll->head;
	} else {
		rs_node_insert_after(dll->tail, node);
		dll->tail = node;
	}
	dll->count++;

	return 0;
}

unsigned rs_dll_get_count(struct rs_dll *dll)
{
	return NULL == dll ? UINT_MAX : dll->count;
}

struct rs_node *rs_dll_find_match(struct rs_dll *dll, rs_node_match_cb_t match,
		void *data)
{
	if (NULL == dll || NULL == match)
		return NULL;

	return rs_node_find_match(dll->head, match, data);
}

int rs_dll_is_empty(struct rs_dll *dll)
{
	if (NULL == dll)
		return 0;

	return rs_dll_get_count(dll) == 0;
}

struct rs_node *rs_dll_find(struct rs_dll *dll, struct rs_node *node)
{
	int match(struct rs_node *n, const void *data)
	{
		return dll->vtable.equals(n, data);
	}

	return rs_dll_find_match(dll, match, node);
}

struct rs_node *rs_dll_pop(struct rs_dll *dll)
{
	struct rs_node *ret;

	if (NULL == dll)
		return NULL;

	ret = rs_dll_remove(dll, dll->head);

	if (0 == dll->count)
		dll->tail = NULL;

	return ret;
}

struct rs_node *rs_dll_next(struct rs_dll *dll)
{
	struct rs_node *next;

	if (NULL == dll)
		return NULL;

	next = dll->cur;

	if (NULL == next)
		dll->cur = dll->head;
	else
		dll->cur = rs_node_next(next);

	return next;
}

int rs_dll_rewind(struct rs_dll *dll)
{
	if (NULL == dll)
		return -EINVAL;

	dll->cur = dll->head;

	return 0;
}

struct rs_node *rs_dll_remove_match(struct rs_dll *dll,
		rs_node_match_cb_t match, void *data)
{
	struct rs_node *head_next_bkp = NULL;
	struct rs_node *tail_prev_bkp = NULL;
	struct rs_node *needle;

	if (NULL == dll || NULL == match)
		return NULL;

	/* in case we remove the head, we want it to be replaced by it's next */
	head_next_bkp = rs_node_next(dll->head);
	/* in case we remove the tail, we want it to be replaced by it's prev */
	tail_prev_bkp = rs_node_prev(dll->tail);

	needle = rs_node_remove_match(dll->head, match, data);
	if (NULL != needle) {
		/* keep list coherent */
		if (needle == dll->head)
			dll->head = head_next_bkp;
		if (needle == dll->tail)
			dll->tail = tail_prev_bkp;
		dll->cur = dll->head;
		dll->count--;
	}

	return needle;
}

struct rs_node *rs_dll_remove(struct rs_dll *dll, struct rs_node *node)
{
	int match(struct rs_node *n, const void *data)
	{
		return dll->vtable.equals(n, data);
	}

	return rs_dll_remove_match(dll, match, node);
}

int rs_dll_foreach(struct rs_dll *dll, rs_node_cb_t cb, void *data)
{
	struct rs_node *n;
	int res = 0;

	if (NULL == cb || NULL == dll)
		return -EINVAL;

	for (n = dll->head; n && 0 == res; n = rs_node_next(n))
		res = cb(n, data);

	return res;
}

