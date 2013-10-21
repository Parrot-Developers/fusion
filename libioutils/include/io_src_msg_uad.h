/**
 * @file io_src_msg_uad.h
 * @date 25 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Source for reading / writing fixed length messages from a Unix socket
 * with an Abstract name in Datagram mode (UAD)
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef IO_SRC_MSG_UAD_H_
#define IO_SRC_MSG_UAD_H_

#include <sys/un.h>

#include "io_src_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef io_src_msg_uad
 * @brief Fixed length message source type, from an UAD
 */
struct io_src_msg_uad;

#ifndef UNIX_PATH_MAX
struct sockaddr_un sun_sizecheck;
#define UNIX_PATH_MAX sizeof(sun_sizecheck.sun_path)
#endif

/**
 * Callback called when I/O is possible.
 * If evt is IO_OUT, must set next message to send, with
 * io_src_msg_uad_set_next_message().
 * If evt is IO_IN, the callback is called when a message has been read, stored
 * in the rcv_buf specified at io_src_msg_uad_init().
 * @param src UAD source
 * @param evt Event type, either IO or OUT not both. The evt parameter must be
 * used instead of io_src_has(in|out) in the callback.
 */
typedef void (io_src_msg_uad_cb_t)(struct io_src_msg_uad *src,
		enum io_src_event evt);

/**
 * @struct io_src_msg_uad
 * @brief Message source type
 */
struct io_src_msg_uad {
	/** inner msg monitor source */
	struct io_src_msg src_msg;
	/** user callback, notified when I/O is possible */
	io_src_msg_uad_cb_t *cb;
	/** path of the socket */
	struct sockaddr_un addr;
};

/**
 * Facility function to set the next message to be sent on the next "OUT" event.
 * Must be used in user callback on such events.
 * @param uad_src UAD source
 * @param send_buf Points to a buffer containing the next message to send.
 * @param send_buf_size Size of the message to send
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_msg_uad_set_next_message(struct io_src_msg_uad *uad,
		const void *send_buf, unsigned send_buf_size);

/**
 * Gets the address of the receive buffer
 * @param uad_src Message source
 * @param msg In output, contains the address of the receive buffer. Can't be
 * NULL
 * @return errno compatible negative value on error, 0 on success
 */
int io_src_msg_uad_get_message(struct io_src_msg_uad *uad, void **msg);

/**
 * Initializes a bidirectional message source, from an UAD.
 * @param uad UAD source to initialize
 * @param cb Callback notified
 * @param rcv_buf Buffer where received data are stored
 * @param len Size of rcv_buf
 * @param fmt A la printf format string for the construction of the path
 * @return errno compatible negative value
 */
int io_src_msg_uad_init(struct io_src_msg_uad *uad, io_src_msg_uad_cb_t *cb,
		void *rcv_buf, unsigned len, const char *fmt, ...)
__attribute__((format(printf, 5, 6)));

/**
 * Returns the underlying io_src of the UAD source
 * @param uad UAD source
 * @return io_src of the UAD source
 */
static inline struct io_src *io_src_msg_uad_get_source(struct io_src_msg_uad
		*uad)
{
	return NULL == uad ? NULL : io_src_msg_get_source(&uad->src_msg);
}

/**
 * Re-initializes a uad source, releasing all it's resources.
 * @param uad Source to clean
 */
void io_src_msg_uad_clean(struct io_src_msg_uad *uad);

#ifdef __cplusplus
}
#endif

#endif /* IO_SRC_MSG_UAD_H_ */
