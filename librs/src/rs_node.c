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

rs_node_t *rs_node_head(rs_node_t *node)
{
	if (NULL == node)
		return NULL;

	for (; node->prev; node = rs_node_prev(node));

	return node;
}

rs_node_t *rs_node_insert(rs_node_t *next, rs_node_t *node)
{
	rs_node_t *prev;

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

int rs_node_push(rs_node_t **head, rs_node_t *node)
{
	if (NULL == head)
		return -1;

	if (NULL == node)
		return 0;

	*head = rs_node_insert(*head, node);

	return 0;
}

rs_node_t *rs_node_pop(rs_node_t **head)
{
	rs_node_t *next;
	rs_node_t *res;

	if (NULL == head || NULL == *head)
		return NULL;

	next = (*head)->next;
	res = rs_node_remove(*head, *head);
	*head = next;

	return res;
}

unsigned rs_node_count(rs_node_t *node)
{
	unsigned res = node != NULL;

	while ((node = rs_node_next(node)))
		res++;

	return res;
}

rs_node_t *rs_node_next(rs_node_t *node)
{
	return NULL == node ? NULL : node->next;
}

rs_node_t *rs_node_prev(rs_node_t *node)
{
	return NULL == node ? NULL : node->prev;
}

rs_node_t *rs_node_find_match(rs_node_t *node,
		rs_node_match_cb_t match, const void *data)
{
	if (NULL == node || NULL == match || NULL == data)
		return NULL;

	/* match */
	for (; node && !match(node, data); node = rs_node_next(node));

	return node;
}

rs_node_t *rs_node_remove(rs_node_t *list, rs_node_t *trash)
{
	int match(rs_node_t *node, const void *data) {
		return node == data;
	};

	return rs_node_remove_match(list, match, trash);
}

rs_node_t *rs_node_remove_match(rs_node_t *list,
		rs_node_match_cb_t match, const void *data)
{
	rs_node_t *needle;

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

int rs_node_foreach(rs_node_t *list, rs_node_cb_t cb, void *data)
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
