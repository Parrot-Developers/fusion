/**
 * @file io_src_msg.h
 * @date 24 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Source for reading / writing fixed length messages
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include "io_src.h"

/**
 * @typedef io_src_msg
 * @brief Fixed length message source type
 */
struct io_src_msg;

/**
 * @def to_src_msg
 * @brief Convert a source to it's message source container
 */
#define to_src_msg(p) container_of(p, struct io_src_msg, src)

/**
 * @typedef io_src_msg_cb_t
 * @brief Called when an entire message has been received or when the source is
 * ready to write a message. No partial message can be read or written.
 * If the event type is IO_IN, the message received is stored in src->msg and
 * can be used by the callback.
 * If the event type is IO_OUT, the callback must set src->msg to point to the
 * message to send, which will be sent just after the callback has been called.
 * @param src Message source
 * @param evt Type of the event to process, either IO_IN or IO_OUT, not
 * IO_DUPLEX
 * @return errno compatible value, positive for only a warning, negative if the
 * source must be removed, 0 on success
 */
typedef int (io_src_msg_cb_t)(struct io_src_msg *src, enum io_src_event evt);

/**
 * @typedef io_src_msg
 * @brief Message source type
 */
struct io_src_msg {
	/** inner monitor source */
	struct io_src src;
	/** user callback, notified when I/O is possible */
	io_src_msg_cb_t *cb;
	/** fixed-length of the messages to read/write */
	unsigned len;
	/**
	 * points to a user buffer able to contain a whole message, filled at
	 * each cb call with the message just read on an IO_IN event
	 */
	void *rcv_buf;
	/** points to a user buffer containing the next message to send */
	const void *send_buf;
};

/**
 * Facility function to set the next message to be sent on the next "OUT" event.
 * Must be used in user callback on such events.
 * @param msg_src Message source
 * @param msg Points to a buffer containing the next mesage to send.
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_msg_set_next_message(struct io_src_msg *msg_src, const void *msg);

/**
 * Initializes a message source.
 * @param msg_src Message source to initialize
 * @param fd File descriptor of the source
 * @param type Type, in, out or duplex
 * @param cb Callback called when a message has been received, stored in msg
 * @param clean Cleanup callback, called when the source is removed from the
 * @param msg Buffer able to receive a message, filled at each cb call with the
 * message just read
 * @param len Size of the msg buffer
 * monitor
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_msg_init(struct io_src_msg *msg_src, int fd, enum io_src_event type,
		io_src_msg_cb_t *cb, io_src_clean_t *clean, void *msg,
		unsigned len);
