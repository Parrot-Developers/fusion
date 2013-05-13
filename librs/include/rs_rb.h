/**
 * @file rs_rb.h
 *
 * @brief ring buffer implementation, imported from mambo
 *
 * read means data read, written to the ring buffer
 *
 * write means data which can be read from the ring buffer, written to an output
 *
 * Copyright (C) 2011 Parrot S.A.
 *
 * @author Jean-Baptiste Dubois
 * @date July 2011
 */

#ifndef RS_RB_H_
#define RS_RB_H_

#include <stdint.h>

/* ring buffer */
struct rs_rb {
	void *base;		/* base address of memory buffer */
	size_t size;		/* size of memory buffer */
	size_t size_mask;	/* size mask of memory buffer */
	size_t len;		/* buffer length (bytes ready to be read) */
	size_t read;		/* read offset */
	size_t write;		/* write offset */
};

int rs_rb_init(struct rs_rb *rb, void *buffer, size_t size);

/* get ring buffer size */
size_t rs_rb_get_size(struct rs_rb *rb);

/* empty ring buffer */
int rs_rb_empty(struct rs_rb *rb);

/* resets a ring buffer */
int rs_rb_clean(struct rs_rb *rb);

/* *  ring buffer read functions * */

/* get ring buffer read ptr */
void *rs_rb_get_read_ptr(struct rs_rb *rb);

/* get ring buffer read length, i.e. free space left in the ring buffer */
size_t rs_rb_get_read_length(struct rs_rb *rb);

/* get ring buffer read length with out wrapping
 * (number of bytes that can be read from read ptr without wrapping) */
size_t rs_rb_get_read_length_no_wrap(struct rs_rb *rb);

/* increment read pointer, freeing up ring buffer space */
int rs_rb_read_incr(struct rs_rb *rb, size_t length);

/* read byte at offset  */
int rs_rb_read_at(struct rs_rb *rb, size_t offset, uint8_t *value);

/* * ring buffer write functions * */

/* get ring buffer write ptr */
void *rs_rb_get_write_ptr(struct rs_rb *rb);

/* get ring buffer write length (number of bytes that can be written) */
size_t rs_rb_get_write_length(struct rs_rb *rb);

/* get ring buffer write length with out wrapping
 * (number of bytes that can be written from write ptr without wrapping) */
size_t rs_rb_get_write_length_no_wrap(struct rs_rb *rb);

/* increment write pointer */
int rs_rb_write_incr(struct rs_rb *rb, size_t length);

#endif /* RS_RB_H_ */
