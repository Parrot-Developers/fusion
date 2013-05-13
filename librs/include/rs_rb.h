/**
 * @file rs_rb.h
 *
 * @brief ring buffer implementation, imported from mambo
 *
 * all operations are from the point of view of the ring buffer (i.e. write is
 * write to the ring buffer)
 *
 * Copyright (C) 2011 Parrot S.A.
 *
 * @author Jean-Baptiste Dubois
 * @date July 2011
 */

#ifndef RS_RB_H_
#define RS_RB_H_

#include <stdint.h>

/**
 * @struct rs_rb
 * @brief Ring buffer structure
 */
struct rs_rb {
	void *base;		/** base address of memory buffer */
	size_t size;		/** size of memory buffer */
	size_t size_mask;	/** size mask of memory buffer */
	size_t len;		/** buffer length i.e. bytes stored in the rb */
	size_t read;		/** read offset */
	size_t write;		/** write offset */
};

/**
 * Initializes a ring buffer, with a buffer and it's size
 * @param rb Ring buffer to initialize
 * @param buffer Buffer used by the ring buffer
 * @param size Size of the buffer, must be a power of two
 * @return -1 on error, 0 otherwise
 */
int rs_rb_init(struct rs_rb *rb, void *buffer, size_t size);

/**
 * Get the ring buffer size. i.e. the total number of bytes the buffer can
 * contain
 * @param rb Ring buffer
 * @return The ring buffer size
 */
size_t rs_rb_get_size(struct rs_rb *rb);

/**
 * Empty a buffer
 * @param rb Ring buffer
 * @return non-zero on error (NULL pointer), 0 otherwise
 */
int rs_rb_empty(struct rs_rb *rb);

/**
 * Resets a ring buffer for future reuse
 * @param rb
 */
int rs_rb_clean(struct rs_rb *rb);

/* *  ring buffer read functions * */

/**
 * Get ring buffer read ptr, i.e. where data can be consumed from the ring
 * buffer
 * @param rb Ring buffer
 * @return place in the buffer, when data can be read from
 * @note not more than the return of rs_rb_get_read_length_no_wrap() must be
 * read from here.
 */
void *rs_rb_get_read_ptr(struct rs_rb *rb);

/**
 * Get the ring buffer read length, i.e. bytes stored in the ring buffer,
 * available to read from it
 * @param rb Ring buffer
 * @return Data available to read from the buffer, 0 if non, or error
 */
size_t rs_rb_get_read_length(struct rs_rb *rb);

/**
 * Get ring buffer read length without wrapping to the start of the buffer
 * @param rb Ring buffer
 * @return Size which can be read from the buffer, without wrapping i.e. in a
 * consecutive manner
 */
size_t rs_rb_get_read_length_no_wrap(struct rs_rb *rb);

/**
 * Increment read pointer, freeing up ring buffer space
 * @param rb Ring buffer
 * @param length Amount of data we have consumed from the ring buffer
 * @return non-zero negative errno-compatible value on error, 0 otherwise
 */
int rs_rb_read_incr(struct rs_rb *rb, size_t length);

/**
 * Read a byte at a given offset
 * @param rb Ring buffer
 * @param offset Offset of the byte to read
 * @param value Must point to a storage for the value to retrieve. Can't be NULL
 * @return non-zero negative errno-compatible value on error, 0 otherwise
 */
int rs_rb_read_at(struct rs_rb *rb, size_t offset, uint8_t *value);

/* * ring buffer write functions * */

/**
 * get ring buffer write ptr, i.e. the place where data can be read to the rb.
 * @param rb Ring buffer
 * @return NULL on error, place to read to otherwise
 * @note not more than the return of rs_rb_get_write_length_no_wrap() must be
 * written
 */
void *rs_rb_get_write_ptr(struct rs_rb *rb);

/**
 * get ring buffer write room left
 * @param rb Ring buffer
 * @return Number of bytes which can't still be stored into the ring buffer. 0
 * on error
 */
size_t rs_rb_get_write_length(struct rs_rb *rb);

/**
 * get ring buffer write length with out wrapping i.e., the number of bytes that
 * can be written to the buffer without wrapping
 * @param rb Ring buffer
 * @return Amount of data which can be written linearly in one chunk. 0 on error
 */
size_t rs_rb_get_write_length_no_wrap(struct rs_rb *rb);

/**
 * increment write pointer, i.e., data have been written th the write buffer
 * @param rb Ring buffer
 * @param length Amount of data written
 * @return non-zero negative errno-compatible value on error, 0 otherwise
 */
int rs_rb_write_incr(struct rs_rb *rb, size_t length);

#endif /* RS_RB_H_ */
