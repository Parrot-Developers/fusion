/**
 * @file io_src.h
 * @date 16 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Generic source source definition
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef IO_SOURCE_H_
#define IO_SOURCE_H_
#include <sys/epoll.h>

#include <stddef.h>

#include <rs_node.h>

/**
 * @typedef io_src_t
 * @brief Source to register in a monitor
 */
typedef struct io_src io_src_t;

/**
 * @typedef io_callback_t
 * @brief Callback notified when a source is ready to perform I/O. If an I/O
 * error occurs, the source is notified by mean of the callback and is
 * automatically removed once the callback returns.<br />
 * Before the callback is called, the event fields is updated according to the
 * type of event which occurred, be it an I/O error or a normal event.
 * @param src source ready for I/O
 *
 * @return Negative errno compatible value on error which implies source
 * removal, positive errno compatible value for a warning, 0 on success
 */
typedef int (io_callback_t)(io_src_t *src);

/**
 * @typedef io_src_cleanup_t
 * @brief Callback called after the source has been removed from the monitor
 * @param src Source to cleanup
 */
typedef void (io_src_cleanup_t)(io_src_t *src);

/**
 * @typedef io_src_event_t
 * @brief Indicates if one can read or write to a given source
 */
/**
 * @enum io_src_event
 * @brief Indicates if one can read or write to a given source
 */
typedef enum io_src_event {
	/** No events */
	IO_NONE = 0,
	/** The source is an "IN" source, i.e. readable */
	IO_IN = EPOLLIN,
	/** The source is an "out" source, i.e. writable */
	IO_OUT = EPOLLOUT,
	/** The source is a full duplex source, i.e. writable and readable */
	IO_DUPLEX = EPOLLIN | EPOLLOUT,
} io_src_event_t;

/**
 * @struct io_src
 * @brief Source to register in a monitor
 */
struct io_src {
	/** file descriptor of the source */
	int fd;
	/**
	 * type of the source (IN or OUT or DUPLEX)
	 * @see man epoll_ctl
	 */
	io_src_event_t type;
	/** callback responsible of this source */
	io_callback_t *callback;
	/** callback called to cleanup when the source is removed */
	io_src_cleanup_t *cleanup;

	/**
	 * epoll events which occurred on this source, set before the callback
	 * is called
	 * @see man epoll_ctl
	 */
	uint32_t events;

	/* fields used by a monitor */
	/**
	 * epoll events activated on this source, altered by io_mon_add_source
	 * and io_mon_activate_out_source
	 * @see man epoll_ctl
	 */
	io_src_event_t active;
	/** node for linking */
	rs_node_t node;
};

/**
 * @def container_of
 * @brief Retrieves the address of a structure knowing the address of one of
 * it's members
 * @param ptr Member address
 * @param type Enclosing structure type
 * @param member Member name
 */
#ifndef container_of
#define container_of(ptr, type, member) ({ \
	const typeof(((type *)0)->member)*__mptr = (ptr); \
	(type *)((char *)__mptr - offsetof(type, member)); \
})
#endif

/**
 * @def to_src
 * @brief Convert a list node to it's container interface
 */
#define to_src(p) container_of(p, io_src_t, node)

/**
 * Initializes a source
 * @param src Source to initialize. Can't be NULL
 * @param fd File descriptor of the source
 * @param type Type, in out or both
 * @param cb Callback notified whe fd is ready for I/O
 * @param cleanup Called to cleanup the source when removed
 * @return Negative errno compatible value on error otherwise zero
 */
int io_src_init(io_src_t *src, int fd, io_src_event_t type, io_callback_t *cb,
		io_src_cleanup_t *cleanup);

/**
 * Cleans up a source (basically a memset...)
 * @param src Source to cleanup
 */
void io_src_cleanup(io_src_t *src);

#endif /* IO_SOURCE_H_ */
