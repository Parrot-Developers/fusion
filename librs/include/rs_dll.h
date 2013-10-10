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

#ifdef __cplusplus
extern "C" {
#endif

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
 * @return Negative errno compatible value on error otherwise zero
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
 * @return Negative errno compatible value on error otherwise zero
 */
int rs_dll_push(struct rs_dll *dll, struct rs_node *node);

/**
 * Adds an element to the list in last position
 * @param dll Doubly linked list
 * @param node Node to enqueue
 * @return Negative errno compatible value on error otherwise zero
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
 * Rewinds the doubly linked list, so that the next call to rs_dll_next will
 * return it's first element
 * @param dll Doubly linked list
 * @return Negative errno compatible value on error otherwise zero
 */
int rs_dll_rewind(struct rs_dll *dll);

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
 * @return Negative errno compatible value on error otherwise zero
 */
int rs_dll_foreach(struct rs_dll *dll, rs_node_cb_t cb);

/**
 * Pops all the nodes of a linked list and allow user to perform a cleanup
 * action on each node
 * @param dll Point to a list's head, NULL in output
 * @param cb Callback called on each node. Can be NULL
 * @return -1 on error, 0 on success
 */
int rs_dll_remove_all(struct rs_dll *dll, rs_node_cb_t cb);

/*
 * TODO check that the list is rewinded before "walk through" actions and at
 * least after each modification action (maybe before, think of it on each case
 */

#ifdef __cplusplus
}
#endif

#endif /* RS_DLL_H_ */
