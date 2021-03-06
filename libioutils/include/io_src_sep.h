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
#include <stdbool.h>
#include <limits.h>

#include <ut_utils.h>

#include <io_src.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def IO_SRC_SEP_SIZE
 * @brief Maximum size of a message that can be reads
 */
#define IO_SRC_SEP_SIZE 0x100

#define IO_SRC_SEP_NO_SEP2 INT_MAX

/**
 * @typedef io_src_sep
 * @brief Separator source type
 */
struct io_src_sep;

/**
 * @typedef io_src_sep_cb
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
 *   <li>
 *     an error has occurred, in this case,
 *     io_src_has_error(io_src_sep_get_source(sep)) will be true
 *   </li>
 * </ul>
 * @param sep Source separator buffer
 * @param chunk Chunk of data read
 * @param len amount of data contained in the chunk, including the unmodified
 * separator if present
 * @note it is guaranteed that chunk[len] can be written, for example to add a
 * null byte for terminating a string, even if len == IO_SRC_SEP_SIZE
 */
typedef void (io_src_sep_cb)(struct io_src_sep *sep, char *chunk,
		unsigned len);

/**
 * @typedef io_src_sep
 * @brief Separator source type
 */
struct io_src_sep {
	/** inner monitor source */
	struct io_src src;
	/** first separator byte */
	char sep1;
	/** second separator byte, INT_MAX for none */
	char sep2;
	/** 1 if the separator is made of two bytes, 0 otherwise */
	bool two_bytes;
	/** user callback, notified when one of the registered signals occur */
	io_src_sep_cb *cb;
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
#define to_src_sep(p) ut_container_of(p, struct io_src_sep, src)

/**
 * Initializes a separator source
 * @param sep_src Separator source to initialize
 * @param fd File descriptor
 * @param cb Callback called on each chunk of data, retrieved before a separator
 * @param sep1 First separator between the chunks of data (only one byte)
 * @param sep2 second separator between the chunks of data (only one byte), pass
 * IO_SRC_SEP_NO_SEP2 to use only one separator
 * @return Negative errno compatible value on error, 0 otherwise
 */
int io_src_sep_init(struct io_src_sep *sep_src, int fd, io_src_sep_cb *cb,
		int sep1, int sep2);

/**
 * Returns the underlying io_src of the separator source
 * @param sep Separator source
 * @return io_src of the separator source
 */
static inline struct io_src *io_src_sep_get_source(struct io_src_sep *sep)
{
	return NULL == sep ? NULL : &sep->src;
}

/**
 * Cleans up a separator source
 * @param sep Separator source
 */
void io_src_sep_clean(struct io_src_sep *sep);

#ifdef __cplusplus
}
#endif

#endif /* IO_SRC_SEP_H_ */
