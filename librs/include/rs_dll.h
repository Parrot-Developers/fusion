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
	int (*equals)(rs_node_t *a, const rs_node_t *b);
	/** displays the data associated with node */
	void (*print)(rs_node_t *node);
};

/**
 * @typedef rs_dll_vtable_t
 * @brief User defined operation on list nodes
 */
typedef struct rs_dll_vtable rs_dll_vtable_t;

/**
 * @struct rs_dll
 * @brief Implementation of a doubly-linked list
 */
struct rs_dll {
	/** first element of the list */
	rs_node_t *head;
	/** cursor of the list, i.e. next element returned by rs_dll_next */
	rs_node_t *cur;
	/** number of elements of the list */
	unsigned count;
	/** user defined operation on list nodes */
	rs_dll_vtable_t vtable;
};

/**
 * @typedef rs_dll_t
 * @brief Implementation of a doubly-linked list
 */
typedef struct rs_dll rs_dll_t;

/**
 * Initializes the doubly linked list
 * @param dll Doubly linked list to initialize
 * @param vtable User defined operations on list nodes. Can be NULL as well as
 * any of it's members
 * @return RS_ERROR_PARAM
 */
int rs_dll_init(rs_dll_t *dll, const rs_dll_vtable_t *vtable);

/**
 * Dumps the content of the list, using the print method provided
 * @param dll Doubly linked list
 */
void rs_dll_dump(rs_dll_t *dll);

/**
 * Adds an element to the list
 * @param dll Doubly linked list
 * @param node Node to push
 * @return RS_ERROR_PARAM
 */
int rs_dll_push(rs_dll_t *dll, rs_node_t *node);

/**
 * @param dll Doubly linked list
 * @return Number of elements of the list, UINT_MAX in case of errors
 */
unsigned rs_dll_get_count(rs_dll_t *dll);

/**
 * Searches a node in the list, based on the user defined equality method
 * @param dll Doubly linked list
 * @param node Node to find
 * @return Node if found, NULL otherwise
 */
rs_node_t *rs_dll_find(rs_dll_t *dll, rs_node_t *node);

/**
 * Searches a node in the list, based on a matching callback
 * @param dll Doubly linked list
 * @param match Matching function
 * @param data User data, passed to the match callback
 * @return Node if found, NULL otherwise
 */
rs_node_t *rs_dll_find_match(rs_dll_t *dll, rs_node_match_cb_t match,
		void *data);

/**
 * Removes and returns the first element of the list.
 * @param dll Doubly linked list
 * @return First element of the list, un-chained, NULL on error
 */
rs_node_t *rs_dll_pop(rs_dll_t *dll);

/**
 * When NULL is returned, the internal cursor is rewinded so that the next call
 * will return the first element of the list
 * @param dll Doubly linked list
 * @return Next element of the list, NULL if none
 */
rs_node_t *rs_dll_next(rs_dll_t *dll);

/**
 * Unchains and returns a given element of the list
 * @param dll Doubly linked list
 * @param node Node to find, matched against list's internal nodes using
 * vtable.compare()
 * @return Node if found, NULL otherwise or on error (NULL parameter)
 */
rs_node_t *rs_dll_remove(rs_dll_t *dll, rs_node_t *node);

/**
 * Unchains and returns a given element of the list
 * @param dll Doubly linked list
 * @param match Matching function
 * @param data User data, passed to the match callback
 * @return Node if found, NULL otherwise or on error (NULL parameter)
 */
rs_node_t *rs_dll_remove_match(rs_dll_t *dll, rs_node_match_cb_t match,
		void *data);

/**
 * Applies a callback to each element of the list, in the list order
 * @param dll Doubly linked list
 * @param cb Callback to apply
 * @param data User data, passe to each callback's call
 * @return RS_ERROR_PARAM if cb is NULL, or the first cb's error code which
 * isn't RS_ERROR_NONE
 */
int rs_dll_foreach(rs_dll_t *dll, rs_node_cb_t cb, void *data);

#endif /* RS_DLL_H_ */
