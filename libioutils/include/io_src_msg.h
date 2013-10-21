/**
 * @file io_src_msg.h
 * @date 24 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Source for reading / writing fixed length messages
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef IO_SRC_MSG_H_
#define IO_SRC_MSG_H_

#include <rs_utils.h>

#include <io_src.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef io_src_msg
 * @brief Fixed length message source type
 */
struct io_src_msg;

/**
 * @def to_src_msg
 * @brief Convert a source to it's message source container
 */
#define to_src_msg(p) rs_container_of(p, struct io_src_msg, src)

/**
 * @typedef io_src_msg_cb_t
 * @brief Called when an entire message has been received or when the source is
 * ready to write a message. No partial message can be read or written.
 * If the event type is IO_IN, the message received can be retrieved with a call
 * to io_src_msg_get_message() and is of the length given at initialization
 * If the event type is IO_OUT, the callback must set the next message to be
 * sent with a call to io_src_msg_set_next_message(). It will be sent just after
 * the callback returns.
 * @param src Message source
 * @param evt Type of the event to process, either IO_IN or IO_OUT, not
 * IO_DUPLEX
 */
typedef void (io_src_msg_cb_t)(struct io_src_msg *src, enum io_src_event evt);

/**
 * @typedef io_src_msg
 * @brief Message source type
 */
struct io_src_msg {
	/** inner monitor source */
	struct io_src src;
	/** user callback, notified when I/O is possible */
	io_src_msg_cb_t *cb;
	/** size of the message to read */
	unsigned rcv_buf_size;
	/**
	 * points to a user buffer able to contain a whole message, filled at
	 * each cb call with the message just read on an IO_IN event
	 */
	void *rcv_buf;
	/** size of the message to write */
	unsigned send_buf_size;
	/** points to a user buffer containing the next message to send */
	const void *send_buf;
	/**
	 * Non-zero if IO must be done automatically (by read/write), 0 if the
	 * source manages it itself in it's io cb
	 */
	unsigned perform_io;
};

/**
 * Facility function to set the next message to be sent on the next "OUT" event.
 * Must be used in user callback on such events.
 * @param msg_src Message source
 * @param send_buf Points to a buffer containing the next message to send.
 * @param send_buf_size Size of the message to send
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_msg_set_next_message(struct io_src_msg *msg_src,
		const void *send_buf, unsigned send_buf_size);

/**
 * Gets the address of the receive buffer
 * @param msg_src Message source
 * @param msg In output, contains the address of the receive buffer. Can't be
 * NULL
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_msg_get_message(struct io_src_msg *msg_src, void **msg);

/**
 * Initializes a message source.
 * @param msg_src Message source to initialize
 * @param fd File descriptor of the source
 * @param type Type, in, out or duplex
 * @param cb Callback called when a message has been received, stored in msg
 * @param rcv_buf Buffer of size len, filled at each cb call with the message
 * just read
 * @param len Size of rcv_buf
 * @param perform_io Non-zero if IO must be done automatically (by read/write),
 * 0 if the source manages it itself in it's io cb
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_msg_init(struct io_src_msg *msg_src, int fd, enum io_src_event type,
		io_src_msg_cb_t *cb, void *rcv_buf, unsigned len,
		unsigned perform_io);

/**
 * Returns the underlying io_src of the message source
 * @param msg Message source
 * @return io_src of the message source
 */
static inline struct io_src *io_src_msg_get_source(struct io_src_msg *msg)
{
	return NULL == msg ? NULL : &msg->src;
}

/**
 * Cleans up a message source, by properly closing fd, zeroing fields etc...
 * @param msg Message source to clean
 */
void io_src_msg_clean(struct io_src_msg *msg);

#ifdef __cplusplus
}
#endif

#endif /* IO_SRC_MSG_H_ */
