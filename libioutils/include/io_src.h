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
 * @enum io_src_event
 * @brief Indicates if one can read or write to a given source
 */
enum io_src_event {
	/** No events */
	IO_NONE = 0,
	/** The source is an "IN" source, i.e. readable */
	IO_IN = EPOLLIN,
	/** The source is an "OUT" source, i.e. writable */
	IO_OUT = EPOLLOUT,
	/** The source is a full duplex source, i.e. writable and readable */
	IO_DUPLEX = EPOLLIN | EPOLLOUT,
};

/**
 * @struct io_src
 * @brief Source to register in a monitor
 */
struct io_src;

/**
 * @typedef io_src_cb_t
 * @brief Callback notified when a source is ready to perform I/O. If an I/O
 * error occurs, the source is notified by mean of the callback and is
 * automatically removed once the callback returns.<br />
 * Before the callback is called, the event fields is updated according to the
 * type of event which occurred, be it an I/O error or a normal event.
 * @param src source ready for I/O
 */
typedef void (io_src_cb_t)(struct io_src *src);

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
	enum io_src_event type;
	/** callback responsible of this source */
	io_src_cb_t *cb;

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
	enum io_src_event active;
	/** node for linking */
	struct rs_node node;
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
	const typeof(((type *)0)->member)*__mptr##member = (ptr); \
	(type *)((char *)__mptr##member - offsetof(type, member)); \
})
#endif

/**
 * @def to_src
 * @brief Convert a list node to it's container interface
 */
#define to_src(p) container_of(p, struct io_src, node)

/**
 * Initializes a source
 * @param src Source to initialize. Can't be NULL
 * @param fd File descriptor of the source
 * @param type Type, in, out or both
 * @param cb Callback notified when fd is ready for I/O
 * @return Negative errno compatible value on error otherwise zero
 */
int io_src_init(struct io_src *src, int fd, enum io_src_event type,
		io_src_cb_t *cb);

/**
 * Says whether a source is active for a given set of events
 * @param src Source to test
 * @param event_set Event set to test
 * @return non-zero if the source is active for all the events present on
 * event_set, 0 otherwise or on error
 */
int io_src_is_active(struct io_src *src, enum io_src_event event_set);

/**
 * Returns the underlying file descriptor of a given source
 * @param src Source to retrieve the file descriptor of
 * @return -1 in case of error, otherwise, file descriptor of the source
 */
static inline int io_src_get_fd(struct io_src *src)
{
	return NULL == src ? -1 : src->fd;
}

/**
 * Reinitializes the source for further use, closing the file descriptor.
 * @param src Source to initialize. Can't be NULL
 */
void io_src_clean(struct io_src *src);

#endif /* IO_SOURCE_H_ */
