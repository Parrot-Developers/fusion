/******************************************************************************
* @file mb_rb.h
*
* @brief mambo ring buffer
*
* Copyright (C) 2011 Parrot S.A.
*
* @author Jean-Baptiste Dubois
* @date July 2011
******************************************************************************/

#ifndef _MB_RB_H_
#define _MB_RB_H_
#include <stdint.h>
#include "mb_log.h"

/* ring buffer */
struct mb_rb {
	void *base;		/* base address of memory buffer */
	size_t size;		/* size of memory buffer */
	size_t size_mask;	/* size mask of memory buffer */
	size_t len;		/* buffer length (bytes ready to be read) */
	size_t read;		/* read offset */
	size_t write;		/* write offset */
};

/* create ring buffer */
static inline int mb_rb_create(struct mb_rb *rb, size_t size)
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

static inline int mb_rb_grow_up(struct mb_rb *rb, size_t extra)
{
	void *base;
	size_t real_size, real_extra, copy_size, old_size;

	/* size must be power of two */
	real_size = 1;
	while (real_size < (rb->size + extra))
		real_size = real_size << 1;

	real_extra = real_size - rb->size;
	base = realloc(rb->base, real_size);
	if (!base)
		return -ENOMEM;

	rb->base = base;
	old_size = rb->size;
	rb->size = real_size;
	rb->size_mask = real_size;
	rb->size_mask -= 1;

	/* move data from beginning of buffer to extra place */
	if (rb->read > rb->write) {
		copy_size = rb->write < real_extra ? rb->write : real_extra;
		memcpy((uint8_t *)rb->base + old_size, rb->base, copy_size);
		if (rb->write > copy_size) {
			memmove(rb->base, (uint8_t *)rb->base + copy_size,
				rb->write - copy_size);
			rb->write -= copy_size;
		} else {
			rb->write = rb->size + copy_size;
		}
	}

	return 0;
}


/* get ring buffer size */
static inline size_t mb_rb_size(struct mb_rb *rb)
{
	return rb->size;
}

/* empty ring buffer */
static inline void mb_rb_empty(struct mb_rb *rb)
{
	rb->read = 0;
	rb->write = 0;
	rb->len = 0;
}
/* destroy ring buffer */
static inline int mb_rb_destroy(struct mb_rb *rb)
{
	mb_rb_empty(rb);
	rb->size = 0;
	rb->size_mask = 0;
	free(rb->base);
	return 0;
}

/* get ring buffer base ptr */
static inline void *mb_rb_base_ptr(struct mb_rb *rb)
{
	return rb->base;
}
/**
 *  ring buffer read functions
 **/

/* get ring buffer read ptr */
static inline void *mb_rb_read_ptr(struct mb_rb *rb)
{
	return (uint8_t *)rb->base + rb->read;
}

/* get ring buffer read length (number of bytes that can be read) */
static inline size_t mb_rb_read_length(struct mb_rb *rb)
{
	return rb->len;
}

/* get ring buffer read length with out wrapping
 * (number of bytes that can be read from read ptr without wrapping) */
static inline size_t mb_rb_read_length_no_wrap(struct mb_rb *rb)
{
	return (rb->write >= rb->read) ? rb->len : rb->len - rb->write;
}

/* increment read pointer, freeing up ring buffer space */
static inline void mb_rb_read_incr(struct mb_rb *rb, size_t length)
{
	mb_assert(length <= rb->len);
	rb->len -= length;
	rb->read += length;
	rb->read &= rb->size_mask;
}

/* read byte at offset  */
static inline uint8_t mb_rb_read_offset(struct mb_rb *rb, size_t offset)
{
	mb_assert(offset < rb->len);
	return ((uint8_t *)rb->base)[(rb->read + offset) & rb->size_mask];
}
/**
 *  ring buffer write functions
 **/

/* get ring buffer write ptr */
static inline void *mb_rb_write_ptr(struct mb_rb *rb)
{
	return (uint8_t *)rb->base + rb->write;
}

/* get ring buffer write length (number of bytes that can be written) */
static inline size_t mb_rb_write_length(struct mb_rb *rb)
{
	return rb->size - rb->len;
}

/* get ring buffer write length with out wrapping
 * (number of bytes that can be written from write ptr without wrapping) */
static inline size_t mb_rb_write_length_no_wrap(struct mb_rb *rb)
{
	if (rb->write >= rb->read)
		return rb->size - rb->write;
	else
		return rb->read - rb->write;
}

/* increment write pointer */
static inline void mb_rb_write_incr(struct mb_rb *rb, size_t length)
{
	mb_assert(rb->len + length <= rb->size);
	rb->len += length;
	rb->write += length;
	rb->write &= rb->size_mask;
}

/* write first byte from ring buffer (and increment read pointer) */
static inline void mb_rb_write_byte(struct mb_rb *rb, uint8_t byte)
{
	mb_assert(rb->len >= rb->size);
	((uint8_t *)rb->base)[rb->write++] = byte;
	rb->write &= rb->size_mask;
	rb->len++;
}

/* find first byte from offset in ring buffer */
static inline int mb_rb_find(struct mb_rb *rb, uint8_t byte, size_t *offset)
{
	size_t i;
	for (i = *offset; i < rb->len; i++) {
		if ((char)mb_rb_read_offset(rb, i) == byte) {
			*offset = i;
			return 0;
		}
	}
	return -ENOENT;
}

static inline int mb_rb_hexa_to_uint8(struct mb_rb *rb, size_t offset,
				       uint8_t *value)
{
	size_t i, digit = 0;
	char c;

	mb_assert(offset + 2 < rb->len);

	*value = 0;
	for (i = 0; i < 2; i++) {
		c = mb_rb_read_offset(rb, offset + 2 - (1 + i));
		if (c >= '0' && c <= '9')
			digit = c - '0';
		else if (c >= 'A' && c <= 'F')
			digit = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			digit = c - 'a' + 10;
		else
			return -ENOENT;

		*value |= digit << 4*i;
	}
	return 0;
}

static inline int mb_rb_hexa_to_uint32(struct mb_rb *rb, size_t offset,
				       uint32_t *value)
{
	size_t i, digit = 0;
	char c;

	mb_assert(offset + 8 < rb->len);

	*value = 0;
	for (i = 0; i < 8; i++) {
		c = mb_rb_read_offset(rb, offset + 8 - (1 + i));
		if (c >= '0' && c <= '9')
			digit = c - '0';
		else if (c >= 'A' && c <= 'F')
			digit = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			digit = c - 'a' + 10;
		else
			return -ENOENT;

		*value |= digit << 4*i;
	}
	return 0;
}

static inline int mb_rb_strncmp(struct mb_rb *rb, size_t offset,
				const char *str, size_t length)
{
	size_t i;

	if (offset + length >= rb->len)
		return -1;

	for (i = 0; i < length; i++) {
		if (mb_rb_read_offset(rb, offset + i) != str[i])
			return -1;
	}
	return 0;
}

#endif /* _MB_RBUFFER_H_ */
