/******************************************************************************
* @file rs_rb.h
*
* @brief ring buffer implementation, imported from mambo
*
* Copyright (C) 2011 Parrot S.A.
*
* @author Jean-Baptiste Dubois
* @date July 2011
******************************************************************************/

#ifndef _MB_RB_H_
#define _MB_RB_H_
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

/* create ring buffer */
static inline int rs_rb_create(struct rs_rb *rb, size_t size)
{
	size_t real_size;

	/* size must be power of two */
	real_size = 1;
	while (real_size < size)
		real_size = real_size << 1;

	/* TODO : align buffer on system page boundary */
	rb->base = malloc(real_size);
	if (!rb->base)
		return -ENOMEM;

	rb->size = real_size;
	rb->size_mask = real_size;
	rb->size_mask -= 1;
	rb->read = 0;
	rb->write = 0;
	rb->len = 0;
	return 0;
}

/* get ring buffer size */
static inline size_t rs_rb_size(struct rs_rb *rb)
{
	return rb->size;
}

/* empty ring buffer */
static inline void rs_rb_empty(struct rs_rb *rb)
{
	rb->read = 0;
	rb->write = 0;
	rb->len = 0;
}
/* destroy ring buffer */
static inline int rs_rb_destroy(struct rs_rb *rb)
{
	rs_rb_empty(rb);
	rb->size = 0;
	rb->size_mask = 0;
	free(rb->base);
	return 0;
}

/**
 *  ring buffer read functions
 **/

/* get ring buffer read ptr */
static inline void *rs_rb_read_ptr(struct rs_rb *rb)
{
	return (uint8_t *)rb->base + rb->read;
}

/* get ring buffer read length (number of bytes that can be read) */
static inline size_t rs_rb_read_length(struct rs_rb *rb)
{
	return rb->len;
}

/* get ring buffer read length with out wrapping
 * (number of bytes that can be read from read ptr without wrapping) */
static inline size_t rs_rb_read_length_no_wrap(struct rs_rb *rb)
{
	return (rb->write >= rb->read) ? rb->len : rb->len - rb->write;
}

/* increment read pointer, freeing up ring buffer space */
static inline void rs_rb_read_incr(struct rs_rb *rb, size_t length)
{
	assert(length <= rb->len);
	rb->len -= length;
	rb->read += length;
	rb->read &= rb->size_mask;
}

/* read byte at offset  */
static inline uint8_t rs_rb_read_offset(struct rs_rb *rb, size_t offset)
{
	assert(offset < rb->len);
	return ((uint8_t *)rb->base)[(rb->read + offset) & rb->size_mask];
}
/**
 *  ring buffer write functions
 **/

/* get ring buffer write ptr */
static inline void *rs_rb_write_ptr(struct rs_rb *rb)
{
	return (uint8_t *)rb->base + rb->write;
}

/* get ring buffer write length (number of bytes that can be written) */
static inline size_t rs_rb_write_length(struct rs_rb *rb)
{
	return rb->size - rb->len;
}

/* get ring buffer write length with out wrapping
 * (number of bytes that can be written from write ptr without wrapping) */
static inline size_t rs_rb_write_length_no_wrap(struct rs_rb *rb)
{
	if (rb->write >= rb->read)
		return rb->size - rb->write;
	else
		return rb->read - rb->write;
}

/* increment write pointer */
static inline void rs_rb_write_incr(struct rs_rb *rb, size_t length)
{
	assert(rb->len + length <= rb->size);
	rb->len += length;
	rb->write += length;
	rb->write &= rb->size_mask;
}

#endif /* _MB_RBUFFER_H_ */
