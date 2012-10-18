/**
 * @file rs_node.h
 * @date 2 août 2012
 * @author nicolas.carrier@parrot.com
 * @brief Node implementation for use in linked list. The empty list is a NULL
 * node
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef RS_NODE_H_
#define RS_NODE_H_

#include <stddef.h>

/**
 * @typedef rs_node_t
 * @brief Node of a doubly-linked list
 */
typedef struct rs_node rs_node_t;

/**
 * @struct rs_node
 * @brief Node of a doubly-linked list
 */
struct rs_node {
	/** next node in the list */
	rs_node_t *next;
	/** next node in the list */
	rs_node_t *prev;
};

/**
 * @typedef rs_node_match_cb_t
 * @brief Matching callback
 * @param node Node to match. Can be NULL
 * @param data User defined data. Can be NULL
 * @return Returns 1 if the node matches, 0 otherwise
 */
typedef int (*rs_node_match_cb_t)(rs_node_t *node, const void *data);

/**
 * @typedef rs_node_cb_t
 * @param node Node of the linked list the callback must apply to
 * @param data User defined data
 * @return 0 on success, non-zero on error
 */
typedef int (*rs_node_cb_t)(rs_node_t *node, void *data);

/**
 * @def RS_NODE_MATCH_MEMBER
 * @brief Convenience macro which declares a matching function on a given member
 * of a struct, enclosing a dll node. Matching is performed with the equal
 * operator, applied to the member and the value pointed at by the user data
 * argument, casted to the same type as the member
 * @param type Base struct type
 * @param member Member on which the matching will be performed
 * @param node_member Name of the member containing the node
 */
#define RS_NODE_MATCH_MEMBER(type, member, node_member) \
int match_##member(rs_node_t *__n, const void *__d) \
{ \
	type *__o = container_of(__n, type, node_member); \
	typeof(((type *)0)->member) *__v = (typeof(((type *)0)->member) *)__d; \
	return __o->member == *__v; \
}

/**
 * Returns the first element of a list given any of it's nodes
 * @param node One node of the list
 * @return First node of the list
 */
rs_node_t *rs_node_head(rs_node_t *node);

/**
 * Inserts a node before another one
 * @param next Node before which the node will be inserted
 * @param node Node to insert
 * @return node if not NULL, next otherwise
 */
rs_node_t *rs_node_insert(rs_node_t *next, rs_node_t *node);

/**
 * Pushes a node to a list whose head is given
 * @param head Head of the list to push to, can point to NULL but can't be NULL.
 * If it has a previous element, an insertion is performed
 * @param node Node to push
 * @return -1 on error, 0 otherwise
 */
int rs_node_push(rs_node_t **head, rs_node_t *node);

/**
 * Removes the first node of a list whose head is given
 * @param head Head of the list to pop to. If it has a previous element, a
 * removal is performed
 * @return Node popped, NULL if none or on error
 */
rs_node_t *rs_node_pop(rs_node_t **head);

/**
 * Counts the number of elements contained on a list whose head is given.
 * Counting is performed forward
 * @param head Lists head. If it has previous elements, they won't be counted.
 */
unsigned rs_node_count(rs_node_t *node);

/**
 * Returns the next element of a given list
 * @param node Current node
 * @return Next node, NULL if none or node is NULL
 */
rs_node_t *rs_node_next(rs_node_t *node);

/**
 * Returns the previous element of a given list
 * @param node Current node
 * @return Previous node, NULL if none or node is NULL
 */
rs_node_t *rs_node_prev(rs_node_t *node);

/**
 * Finds a node matching a given criteria on a list of whom any node is known
 * @param node One node of the list (not necessarily the first), the search is
 * performed forward, starting from node
 * @param match Matching criteria
 * @param data User data passed to the matching function
 * @return Node if found, NULL otherwise
 */
rs_node_t *rs_node_find_match(rs_node_t *node, rs_node_match_cb_t match,
		const void *data);

/**
 * Removes a node from a list of whom any node is known
 * @param list One node of the list (not necessarily the first), the search is
 * performed forward, starting from node
 * @param trash Node to remove, matched on address
 * @note Beware that if the node removed is the "handle" of the list, this
 * handle must be modified after. E.g. if the node removed is the head, then one
 * must update the head reference, to point to the next element
 * @return Node removed if found, NULL otherwise or on error
 */
rs_node_t *rs_node_remove(rs_node_t *list, rs_node_t *trash);

/**
 * Removes a node matching a given criteria from a list of whom any node is
 * known
 * @param list One node of the list (not necessarily the first)
 * @param match Matching criteria
 * @param data User data, passed to the matching function
 * @return Node removed if found, NULL otherwise or on error
 */
rs_node_t *rs_node_remove_match(rs_node_t *list,
		rs_node_match_cb_t match, const void *data);

/**
 * Applies a callback to each element of the list, in the list order. Stops at
 * the firt callback's non zero return value
 * @param list Doubly linked list
 * @param cb Callback to apply
 * @param data User data, passe to each callback's call
 * @return 0 on success, or the first cb's call non-zero return value
 */
int rs_node_foreach(rs_node_t *list, rs_node_cb_t cb, void *data);

#endif /* RS_NODE_H_ */
