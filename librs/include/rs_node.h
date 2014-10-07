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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct rs_node
 * @brief Node of a doubly-linked list
 */
struct rs_node {
	/** next node in the list */
	struct rs_node *next;
	/** next node in the list */
	struct rs_node *prev;
};

/**
 * @typedef rs_node_match_cb_t
 * @brief Matching callback
 * @param node Node to match. Can be NULL
 * @param data User defined data. Can be NULL
 * @return Returns 1 if the node matches, 0 otherwise
 */
typedef int (*rs_node_match_cb_t)(struct rs_node *node, const void *data);

/**
 * @typedef rs_node_cb_t
 * @param node Node of the linked list the callback must apply to
 * @return 0 on success, non-zero on error
 */
typedef int (*rs_node_cb_t)(struct rs_node *node);

/**
 * @def RS_NODE_MATCH_MEMBER
 * @brief Convenience macro which declares a matching function on a given member
 * of a struct, enclosing a dll node. Matching is performed with the equal
 * operator, applied to the member and the value pointed at by the user data
 * argument, casted to the same type as the member. If data is NULL, the
 * matching function returns 0;
 * @param type Base struct type name
 * @param member Member on which the matching will be performed
 * @param node_member Name of the member containing the node
 */
#define RS_NODE_MATCH_MEMBER(type, member, node_member) \
int type##_match_##member(struct rs_node *__n, const void *__d) \
{ \
	struct type *__o = ut_container_of(__n, struct type, node_member); \
	const typeof(((struct type *)0)->member)*__v; \
	 \
	if (NULL == __d) \
		return 0; \
	__v = __d; \
 \
	return __o->member == *__v; \
}

/**
 * @def RS_NODE_MATCH_STR_MEMBER
 * @brief Convenience macro which declares a matching function on a given member
 * of a struct, enclosing a dll node. Matching is performed with the strcmp
 * operator, applied to the member and the value pointed at by the user data
 * argument, casted to the same type as the member. If data is NULL, the
 * matching function returns 0;
 * @param type Base struct type name
 * @param member Member on which the matching will be performed
 * @param node_member Name of the member containing the node
 */
#define RS_NODE_MATCH_STR_MEMBER(type, member, node_member) \
int type##_match_str_##member(struct rs_node *__n, const void *__d) \
{ \
	struct type *__o = ut_container_of(__n, struct type, node_member); \
	if (__d == NULL || __o->member == NULL) \
		return 0; \
	return 0 == strcmp(__o->member, (const char *)__d); \
}

/**
 * Returns the first element of a list given any of it's nodes
 * @param node One node of the list
 * @return First node of the list
 */
struct rs_node *rs_node_head(struct rs_node *node);

/**
 * Inserts a node before another one
 * @param next Node before which the node will be inserted
 * @param node Node to insert
 * @return node if not NULL, next otherwise
 */
struct rs_node *rs_node_insert_before(struct rs_node *next,
		struct rs_node *node);

/**
 * Inserts a node after another one
 * @param prev Node after which the node will be inserted
 * @param node Node to insert
 * @return node if not NULL, prev otherwise
 */
struct rs_node *rs_node_insert_after(struct rs_node *prev,
		struct rs_node *node);

/**
 * Pushes a node to a list whose head is given
 * @param head Head of the list to push to, can point to NULL but can't be NULL.
 * If it has a previous element, an insertion is performed
 * @param node Node to push
 * @return -EINVAL if an invalid argument is passed
 */
int rs_node_push(struct rs_node **head, struct rs_node *node);

/**
 * Removes the first node of a list whose head is given
 * @param head Head of the list to pop to. If it has a previous element, a
 * removal is performed
 * @return Node popped, NULL if none or on error
 */
struct rs_node *rs_node_pop(struct rs_node **head);

/**
 * Counts the number of elements contained on a list whose head is given.
 * Counting is performed forward
 * @param head Lists head. If it has previous elements, they won't be counted.
 */
unsigned rs_node_count(struct rs_node *head);

/**
 * Returns the next element of a given list
 * @param node Current node
 * @return Next node, NULL if none or node is NULL
 */
struct rs_node *rs_node_next(struct rs_node *node);

/**
 * Returns the previous element of a given list
 * @param node Current node
 * @return Previous node, NULL if none or node is NULL
 */
struct rs_node *rs_node_prev(struct rs_node *node);

/**
 * Finds a node matching based on it's address
 * @param needle One node of the list (not necessarily the first), the search
 * is performed forward, starting from needle
 * @param haystack Node to find, compared by address
 * @return Node if found, NULL otherwise
 */
struct rs_node *rs_node_find(struct rs_node *needle, struct rs_node *haystack);

/**
 * Finds a node matching a given criteria on a list of whom any node is known
 * @param node One node of the list (not necessarily the first), the search is
 * performed forward, starting from node
 * @param match Matching criteria
 * @param data User data passed to the matching function
 * @return Node if found, NULL otherwise
 */
struct rs_node *rs_node_find_match(struct rs_node *node,
		rs_node_match_cb_t match, const void *data);

/**
 * Removes a node from a list of whom any node is known
 * @param list One node of the list (not necessarily the first), the search is
 * performed forward, starting from node
 * @param trash Node to remove, matched on address
 * @note Beware that if the node removed is the "handle" of the list, this
 * handle must be modified after. E.g. if the node removed is the head, then one
 * must update the head reference, to point to the next element
 * TODO should take a rs_node **list and update the head ?
 * @return Node removed if found, NULL otherwise or on error
 */
struct rs_node *rs_node_remove(struct rs_node *list, struct rs_node *trash);

/**
 * Removes a node matching a given criteria from a list of whom any node is
 * known
 * @param list One node of the list (not necessarily the first)
 * @param match Matching criteria
 * @param data User data, passed to the matching function
 * @return Node removed if found, NULL otherwise or on error
 */
struct rs_node *rs_node_remove_match(struct rs_node *list,
		rs_node_match_cb_t match, const void *data);

/**
 * Applies a callback to each element of the list, in the list order. Stops at
 * the firt callback's non zero return value
 * @param list Doubly linked list
 * @param cb Callback to apply
 * @return -EINVAL if an invalid argument is passed, 0 on success, or the first
 * cb's call non-zero return value
 */
int rs_node_foreach(struct rs_node *list, rs_node_cb_t cb);

/**
 * Pops all the nodes of a linked list and allow user to perform a cleanup
 * action on each node
 * @param list Point to a list's head, NULL in output
 * @param cb Callback called on each node. Can be NULL
 * @return -EINVAL if an invalid argument is passed
 */
int rs_node_remove_all(struct rs_node **list, rs_node_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif /* RS_NODE_H_ */
