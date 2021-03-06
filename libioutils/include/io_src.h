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

#include <ut_utils.h>
#include <rs_node.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 * @def IO_EPOLL_ERROR_EVENTS
 * @brief Epoll events considered as an error when occurring on a source
 */
#define IO_EPOLL_ERROR_EVENTS (EPOLLERR | EPOLLHUP | EPOLLRDHUP)

/**
 * @def io_src_has_error
 * @brief Returns 1 if the epoll event set contains at least one error event
 */
#define io_src_has_error(src) (IO_EPOLL_ERROR_EVENTS & (src)->events)

/**
 * @def io_src_has_in
 * @brief Returns 1 if the epoll event set contains the IN event flag
 */
#define io_src_has_in(src) (!!((src)->events & IO_IN))

/**
 * @def io_src_has_out
 * @brief Returns 1 if the epoll event set contains the OUT event flag
 */
#define io_src_has_out(src) (!!((src)->events & IO_OUT))

/**
 * @struct io_src
 * @brief Source to register in a monitor
 */
struct io_src;

/**
 * @typedef io_src_cb
 * @brief Callback notified when a source is ready to perform I/O. If an I/O
 * error occurs, the source is notified by mean of the callback and is
 * automatically removed once the callback returns.<br />
 * Before the callback is called, the event fields is updated according to the
 * type of event which occurred, be it an I/O error or a normal event.
 * @param src source ready for I/O
 */
typedef void (io_src_cb)(struct io_src *src);

/**
 * @struct io_src
 * @brief Source to register in a monitor
 */
struct io_src {
	/* fields used internally by a monitor */
	/** node for linking */
	struct rs_node node;
	/**
	 * epoll events activated on this source, altered by io_mon_add_source
	 * and io_mon_activate_out_source
	 * @see man epoll_ctl
	 */
	enum io_src_event active;

	/** file descriptor of the source */
	int fd;
	/**
	 * type of the source (IN or OUT or DUPLEX)
	 * @see man epoll_ctl
	 */
	enum io_src_event type;
	/** callback responsible of this source */
	io_src_cb *cb;

	/**
	 * epoll events which occurred on this source, set before the callback
	 * is called
	 * @see man epoll_ctl
	 */
	uint32_t events;
};

/**
 * @def to_src
 * @brief Convert a list node to it's container interface
 */
#define to_src(p) ut_container_of(p, struct io_src, node)

/**
 * Initializes a source
 * @param src Source to initialize. Can't be NULL
 * @param fd File descriptor of the source
 * @param type Type, in, out or both
 * @param cb Callback notified when fd is ready for I/O
 * @return Negative errno compatible value on error otherwise zero
 */
int io_src_init(struct io_src *src, int fd, enum io_src_event type,
		io_src_cb *cb);

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
 * Closes the source's underlying file descriptor.
 * @param src Source to close the file descriptor of
 * @return Negative errno-compatible value on error otherwise zero
 */
int io_src_close_fd(struct io_src *src);

/**
 * Reinitializes the source for further use. If it was chained to other nodes,
 * the source is unchained. It is the responsibility of the client to close the
 * file descriptor, by either calling io_src_close_fd() prior to calling
 * io_src_clean() or by directly closing the file descriptor.
 * @param src Source to initialize. Can't be NULL
 */
void io_src_clean(struct io_src *src);

#ifdef __cplusplus
}
#endif

#endif /* IO_SOURCE_H_ */
