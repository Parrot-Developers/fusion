/**
 * @file rs_node.c
 * @date 2 août 2012
 * @author nicolas.carrier@parrot.com
 * @brief Node implementation for use in linked list. The empty list is a NULL
 * node
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#include <rs_node.h>

/**
 * Matching callback based on the node adress
 * @param node Node to find
 * @param data Current node to compare to
 * @return 1 if addresses do match
 */
static int match(struct rs_node *node, const void *data)
{
	return node == data;
};

struct rs_node *rs_node_head(struct rs_node *node)
{
	if (NULL == node)
		return NULL;

	while (node->prev)
		node = rs_node_prev(node);

	return node;
}

struct rs_node *rs_node_insert_before(struct rs_node *next,
		struct rs_node *node)
{
	struct rs_node *prev;

	if (NULL == node)
		return next;

	/* chain with next */
	node->next = next;
	if (next) {
		prev = next->prev;
		next->prev = node;

		/* chain with previous */
		node->prev = prev;
		if (prev)
			prev->next = node;
	} else {
		node->prev = NULL;
	}

	return node;
}

struct rs_node *rs_node_insert_after(struct rs_node *prev, struct rs_node *node)
{
	struct rs_node *next;

	if (NULL == node)
		return prev;

	/* chain with prev */
	node->prev = prev;
	if (prev) {
		next = prev->next;
		prev->next = node;

		/* chain with next */
		node->next = next;
		if (next)
			next->prev = node;
	} else {
		node->prev = NULL;
	}

	return node;
}

int rs_node_push(struct rs_node **head, struct rs_node *node)
{
	if (NULL == head)
		return -1;

	if (NULL == node)
		return 0;

	*head = rs_node_insert_before(*head, node);

	return 0;
}

struct rs_node *rs_node_pop(struct rs_node **head)
{
	struct rs_node *next;
	struct rs_node *res;

	if (NULL == head || NULL == *head)
		return NULL;

	next = (*head)->next;
	res = rs_node_remove(*head, *head);
	*head = next;

	return res;
}

unsigned rs_node_count(struct rs_node *node)
{
	unsigned res = node != NULL;

	while ((node = rs_node_next(node)))
		res++;

	return res;
}

struct rs_node *rs_node_next(struct rs_node *node)
{
	return NULL == node ? NULL : node->next;
}

struct rs_node *rs_node_prev(struct rs_node *node)
{
	return NULL == node ? NULL : node->prev;
}

struct rs_node *rs_node_find(struct rs_node *needle, struct rs_node *haystack)
{
	return rs_node_find_match(needle, match, haystack);
}

struct rs_node *rs_node_find_match(struct rs_node *node,
		rs_node_match_cb_t match, const void *data)
{
	if (NULL == node || NULL == match)
		return NULL;

	/* match */
	while (node && !match(node, data))
		node = rs_node_next(node);

	return node;
}

struct rs_node *rs_node_remove(struct rs_node *list, struct rs_node *trash)
{
	return rs_node_remove_match(list, match, trash);
}

struct rs_node *rs_node_remove_match(struct rs_node *list,
		rs_node_match_cb_t match, const void *data)
{
	struct rs_node *needle;

	if (NULL == list || NULL == match)
		return NULL;

	needle = rs_node_find_match(list, match, data);
	if (NULL != needle) {
		/* unchain */
		if (needle->next)
			needle->next->prev = needle->prev;
		if (needle->prev)
			needle->prev->next = needle->next;

		/* mask private information */
		needle->next = needle->prev = NULL;
	}

	return needle;
}

int rs_node_foreach(struct rs_node *list, rs_node_cb_t cb, void *data)
{
	int err = 0;

	if (NULL == cb || NULL == list)
		return -1;

	for (; list; list = rs_node_next(list)) {
		err = cb(list, data);
		if (0 != err)
			return err;
	}

	return 0;
}
