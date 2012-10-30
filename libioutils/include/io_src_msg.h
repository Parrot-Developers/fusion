/**
 * @file io_src_msg.h
 * @date 24 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Source for reading fixed length messages
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
 * @brief Called when an entire message has been received
 * @param msg Message source
 * @return errno compatible value, positive for only a warning, negative if the
 * source must be removed, 0 on success
 */
typedef int (io_src_msg_cb_t)(struct io_src_msg *msg);

/**
 * @typedef io_src_msg
 * @brief Message source type
 */
struct io_src_msg {
	/** inner monitor source */
	struct io_src src;
	/** user callback, notified a message has been received */
	io_src_msg_cb_t *cb;
	/** fixed-length of the messages to read */
	unsigned len;
	/**
	 * points to a user buffer able to receive a message, filled at each cb
	 * call with the message just read
	 */
	void *msg;
};

/**
 * Initializes a message source.
 * @param msg_src Message source to initialise
 * @param fd File descriptor of the source
 * @param msg Buffer able to receive a message, filled at each cb call with the
 * message just read
 * @param len Size of the msg buffer
 * @param cb Callback called when a message has been received, stored in msg
 * @param clean Cleanup callback, called when the source is removed from the
 * monitor
 * @return errno compatible negative value
 */
int io_src_msg_init(struct io_src_msg *msg_src, int fd, io_src_msg_cb_t *cb,
		io_src_clean_t *clean, void *msg, unsigned len);
