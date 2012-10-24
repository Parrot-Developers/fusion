/**
 * @file io_src_sep.h
 * @date 23 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Separator source, for input. Bytes read are stored in an internal
 * buffer of size IO_SRC_SEP_SIZE and the client is notified each time a given
 * character separator is found, if the buffer is full or if end of file is
 * reached.
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef IO_SRC_SEP_H_
#define IO_SRC_SEP_H_

#include "io_src.h"

#define IO_SRC_SEP_SIZE 1024

/**
 * @typedef io_src_sep
 * @brief Separator source type
 */
struct io_src_sep;

/**
 * @typedef io_src_sep_cb_t
 * @brief User callback, called when :
 * <ul>
 *   <li>a complete line has been read from the source</li>
 *   <li>the buffer is full</li>
 *   <li>
 *     end of file is reached, in this case, it is called once more with a zero
 *     len argument
 *   </li>
 * </ul>
 * @param sep Source separator buffer
 * @param chunk Chunk of data read
 * @param len amount of data contained in the chunk, including the unmodified
 * separator if present
 * @note it is guaranteed that chunk[len] can be written, for example to add a
 * null byte for terminating a string, even if len == IO_SRC_SEP_SIZE
 * @return errno compatible value, positive for only a warning, negative if the
 * source must be removed, 0 on success
 */
typedef int (io_src_sep_cb_t)(struct io_src_sep *sep, char *chunk,
		unsigned len);

/**
 * @typedef io_src_sep
 * @brief Separator source type
 */
struct io_src_sep {
	/** separator byte */
	char sep;
	/** user callback, notified when one of the registered signals occur */
	io_src_sep_cb_t *cb;
	/** inner monitor source */
	struct io_src src;
	/** buffer containing the bytes read from the source */
	char buf[2 * IO_SRC_SEP_SIZE + 1];

	/** first byte to be consumed for the next line to send to the client */
	unsigned from;
	/**
	 * one after the last byte to be consumed for the next line to send to
	 * the client
	 */
	unsigned up_to;
};

/**
 * Initializes a separator source
 * @param sep_src Separator source to initialize
 * @param fd File descriptor
 * @param cb Callback called on each cunk of data, retrieved before a separator
 * @param sep Separator between the chunks of data
 * @return Negative errno compatible value on error, 0 otherwise
 */
int io_src_sep_init(struct io_src_sep *sep_src, int fd, io_src_sep_cb_t *cb,
		char sep);

#endif /* IO_SRC_SEP_H_ */
