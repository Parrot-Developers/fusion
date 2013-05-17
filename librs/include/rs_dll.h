/**
 * @file rs_dll.h
 * @date 30 juil. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Doubly linked list implementation
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef RS_DLL_H_
#define RS_DLL_H_

#include <rs_node.h>

/**
 * @struct rs_dll_vtable
 * @brief User defined operation on list nodes
 */
struct rs_dll_vtable {
	/** returns 0 if a equals b, non-zero otherwise */
	int (*equals)(struct rs_node *a, const struct rs_node *b);
	/** displays the data associated with node */
	void (*print)(struct rs_node *node);
};

/**
 * @struct rs_dll
 * @brief Implementation of a doubly-linked list
 */
struct rs_dll {
	/** first element of the list */
	struct rs_node *head;
	/** last element of the list */
	struct rs_node *tail;
	/** cursor of the list, i.e. next element returned by rs_dll_next */
	struct rs_node *cur;
	/** number of elements of the list */
	unsigned count;
	/** user defined operation on list nodes */
	struct rs_dll_vtable vtable;
};

/**
 * Initializes the doubly linked list
 * @param dll Doubly linked list to initialize
 * @param vtable User defined operations on list nodes. Can be NULL as well as
 * any of it's members
 * @return RS_ERROR_PARAM
 */
int rs_dll_init(struct rs_dll *dll, const struct rs_dll_vtable *vtable);

/**
 * Dumps the content of the list, using the print method provided
 * @param dll Doubly linked list
 */
void rs_dll_dump(struct rs_dll *dll);

/**
 * Adds an element to the list in first position
 * @param dll Doubly linked list
 * @param node Node to push
 * @return RS_ERROR_PARAM
 */
int rs_dll_push(struct rs_dll *dll, struct rs_node *node);

/**
 * Adds an element to the list in last position
 * @param dll Doubly linked list
 * @param node Node to enqueue
 * @return RS_ERROR_PARAM
 */
int rs_dll_enqueue(struct rs_dll *dll, struct rs_node *node);

/**
 * @param dll Doubly linked list
 * @return Number of elements of the list, UINT_MAX in case of errors
 */
unsigned rs_dll_get_count(struct rs_dll *dll);

/**
 * Says whether or not th dll contains at least one element or not
 * @param dll Doubly linked-list
 * @return non-zero if the dll is empty, 0 if it is
 */
int rs_dll_is_empty(struct rs_dll *dll);

/**
 * Searches a node in the list, based on the user defined equality method
 * @param dll Doubly linked list
 * @param node Node to find
 * @return Node if found, NULL otherwise
 */
struct rs_node *rs_dll_find(struct rs_dll *dll, struct rs_node *node);

/**
 * Searches a node in the list, based on a matching callback
 * @param dll Doubly linked list
 * @param match Matching function.
 * @note An action can be performed in the matching callback, when there is a
 * match
 * @param data User data, passed to the match callback
 * @return Node if found, NULL otherwise
 */
struct rs_node *rs_dll_find_match(struct rs_dll *dll, rs_node_match_cb_t match,
		void *data);

/**
 * Removes and returns the first element of the list.
 * @param dll Doubly linked list
 * @return First element of the list, un-chained, NULL on error
 */
struct rs_node *rs_dll_pop(struct rs_dll *dll);

/**
 * When NULL is returned, the internal cursor is rewinded so that the next call
 * will return the first element of the list
 * @param dll Doubly linked list
 * @return Next element of the list, NULL if none
 */
struct rs_node *rs_dll_next(struct rs_dll *dll);

/**
 * Unchains and returns a given element of the list
 * @param dll Doubly linked list
 * @param node Node to find, matched against list's internal nodes using
 * vtable.compare()
 * @return Node if found, NULL otherwise or on error (NULL parameter)
 */
struct rs_node *rs_dll_remove(struct rs_dll *dll, struct rs_node *node);

/**
 * Unchains and returns a given element of the list
 * @param dll Doubly linked list
 * @param match Matching function
 * @param data User data, passed to the match callback
 * @return Node if found, NULL otherwise or on error (NULL parameter)
 */
struct rs_node *rs_dll_remove_match(struct rs_dll *dll,
		rs_node_match_cb_t match, void *data);

/**
 * Applies a callback to each element of the list, in the list order
 * @param dll Doubly linked list
 * @param cb Callback to apply
 * @param data User data, passe to each callback's call
 * @return RS_ERROR_PARAM if cb is NULL, or the first cb's error code which
 * isn't RS_ERROR_NONE
 */
int rs_dll_foreach(struct rs_dll *dll, rs_node_cb_t cb, void *data);

#endif /* RS_DLL_H_ */
