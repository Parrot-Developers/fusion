/**
 * @file io_src_sep.h
 * @date 23 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Separator source, for input. Bytes read are stored in an internal
 * buffer of size IO_SRC_SEP_SIZE and the client is notified each time a given
 * separator or separator pair (for example \r\n) is found, if the buffer is
 * full or if end of file is reached.
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
 *   <li>
 *     a complete line has been read from the source, including the separator
 *   </li>
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
	/** first separator byte */
	char sep1;
	/** second separator byte, INT_MAX for none */
	char sep2;
	/** 1 if the separator is made of two bytes, 0 otherwise */
	int two_bytes;
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
 * @def to_src
 * @brief Convert a source to it's signal source container
 */
#define to_src_sep(p) container_of(p, struct io_src_sep, src)

/**
 * Initializes a separator source
 * @param sep_src Separator source to initialize
 * @param fd File descriptor
 * @param cb Callback called on each cunk of data, retrieved before a separator
 * @param sep1 First separator between the chunks of data (only one byte)
 * @param sep2 second separator between the chunks of data, pass INT_MAX to use
 * only one separator (only one byte)
 * @return Negative errno compatible value on error, 0 otherwise
 */
int io_src_sep_init(struct io_src_sep *sep_src, int fd, io_src_sep_cb_t *cb,
		io_src_clean_t *clean, int sep1, int sep2);

/**
 * Returns the underlying io_src of the separator source
 * @param sep Separator source
 * @return io_src of the separator source
 */
static inline struct io_src *io_src_sep_get_source(struct io_src_sep *sep)
{
	return NULL == sep ? NULL : &(sep->src);
}

#endif /* IO_SRC_SEP_H_ */
