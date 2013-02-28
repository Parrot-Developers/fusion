/**
 * @file io_src_msg_uas.h
 * @date 25 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Source for reading / writing fixed length messages from a Unix socket
 * with an Abstract name in Seqpacket mode (UAS)
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef IO_SRC_MSG_UAS_H_
#define IO_SRC_MSG_UAS_H_

#include <sys/un.h>

#include "io_src_msg.h"

/**
 * @typedef io_src_msg_uas
 * @brief Fixed length message source type, from an UAS
 */
struct io_src_msg_uas;

#ifndef UNIX_PATH_MAX
struct sockaddr_un sun_sizecheck;
#define UNIX_PATH_MAX sizeof(sun_sizecheck.sun_path)
#endif

/**
 * Callback called when I/O is possible.
 * If evt is IO_OUT, must set next message to send, with
 * io_src_msg_uas_set_next_message().
 * If evt is IO_IN, the callback is called when a message has been read, stored
 * in the rcv_buf specified at io_src_msg_uas_init().
 * @param src Uas source
 * @param evt Event type, either IO or OUT not both
 * @return errno compatible value, positive for only a warning, negative if the
 * source must be removed, 0 on success
 */
typedef int (io_src_msg_uas_cb_t)(struct io_src_msg_uas *src,
		enum io_src_event evt);

/**
 * Called when the source is removed from a monitor, on error or at cleanup.
 * Should perform client cleanup
 * @param src Uas source
 */
typedef void (io_src_msg_uas_clean_t)(struct io_src_msg_uas *src);

/**
 * @typedef io_src_msg_uas
 * @brief Message source type
 */
struct io_src_msg_uas {
	/** inner msg monitor source */
	struct io_src_msg src_msg;
	/** user callback, notified when I/O is possible */
	io_src_msg_uas_cb_t *cb;
	/**
	 * user clean callback, called when io_mon_clean is called , after the
	 * underlying message source itself has been cleaned
	 */
	io_src_msg_uas_clean_t *clean;
	/** path of the socket */
	struct sockaddr_un addr;
};

/**
 * Facility function to set the next message to be sent on the next "OUT" event.
 * Must be used in user callback on such events.
 * @param uas_src UAS source
 * @param msg Points to a buffer containing the next message to send.
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_msg_uas_set_next_message(struct io_src_msg_uas *uas_src,
		const void *rcv_buf);

/**
 * Initializes a bidirectional message source, from an UAS.
 * @param uas_src Message source to initialize
 * @param uas_src UAS source
 * @param cb Callback notified
 * @param clean Source cleanup callback
 * @param rcv_buf Buffer where received data are stored
 * @param len Size of rcv_buf
 * @param fmt A la printf format string for the construction of the path
 * @return errno compatible negative value
 */
int io_src_msg_uas_init(struct io_src_msg_uas *uas_src, io_src_msg_uas_cb_t *cb,
		io_src_msg_uas_clean_t *clean, void *rcv_buf, unsigned len,
		const char *fmt, ...) __attribute__((format(printf, 6, 7)));

/**
 * Returns the underlying io_src of the UAS source
 * @param uas UAS source
 * @return io_src of the UAS source
 */
static inline struct io_src *io_src_msg_uas_get_source(struct io_src_msg_uas
		*uas)
{
	return NULL == uas ? NULL : io_src_msg_get_source(&(uas->src_msg));
}

#endif /* IO_SRC_MSG_UAS_H_ */
