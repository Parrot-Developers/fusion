/**
 * @file rs_rb.c
 * @date 13 mai 2013
 * @author Jean-Baptiste Dubois
 * @author nicolas.carrier@parrot.com
 * @brief ring buffer implementation, imported from mambo
 *
 * data stored into the ring buffer span from rb->read included to rb->write
 * excluded
 *
 * Copyright (C) 2013 Parrot S.A.
 */
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <rs_rb.h>

/**
 * @def is_power_of_two
 * @brief Says whether a number is a power of two or not, evaluates it's
 * argument only once
 */
#define is_power_of_two(x) ({ \
	typeof (x) _x = (x);\
	((_x != 0) && ((_x & (~_x + 1)) == _x)); \
	})

int rs_rb_init(struct rs_rb *rb, void *buffer, size_t size)
{
	if (!is_power_of_two(size) || NULL == rb || NULL == buffer)
		return -EINVAL;

	rb->base = buffer;
	rb->size = size;
	rb->size_mask = size - 1;
	rb->read = 0;
	rb->write = 0;
	rb->len = 0;

	return 0;
}

size_t rs_rb_get_size(struct rs_rb *rb)
{
	return NULL == rb ? 0 : rb->size;
}

int rs_rb_empty(struct rs_rb *rb)
{
	if (NULL == rb)
		return -EINVAL;

	rb->read = 0;
	rb->write = 0;
	rb->len = 0;

	return 0;
}

int rs_rb_clean(struct rs_rb *rb)
{
	if (NULL == rb)
		return -EINVAL;

	memset(rb, 0, sizeof(*rb));

	return 0;
}

void *rs_rb_get_read_ptr(struct rs_rb *rb)
{
	if (NULL == rb)
		return NULL;

	return (char *)(rb->base) + rb->read;
}

size_t rs_rb_get_read_length(struct rs_rb *rb)
{
	return NULL == rb ? 0 : rb->len;
}

size_t rs_rb_get_read_length_no_wrap(struct rs_rb *rb)
{
	if (NULL == rb)
		return 0;

	return rb->len + rb->read <= rb->size ? rb->len : rb->len - rb->write;
}

int rs_rb_read_incr(struct rs_rb *rb, size_t length)
{
	if (NULL == rb)
		return -EINVAL;
	if (length > rb->len)
		return -ENOSR;

	rb->len -= length;
	rb->read += length;
	rb->read &= rb->size_mask;

	return 0;
}

int rs_rb_read_at(struct rs_rb *rb, size_t offset, char *value)
{
	if (NULL == rb || offset >= rb->len || NULL == value)
		return -EINVAL;

	*value = ((char *) rb->base)[(rb->read + offset) & rb->size_mask];

	return 0;
}

void *rs_rb_get_write_ptr(struct rs_rb *rb)
{
	if (NULL == rb)
		return NULL;

	return (char *) rb->base + rb->write;
}

size_t rs_rb_get_write_length(struct rs_rb *rb)
{
	return NULL == rb ? 0 : rb->size - rb->len;
}

size_t rs_rb_get_write_length_no_wrap(struct rs_rb *rb)
{
	if (NULL == rb)
		return 0;

	return (rb->write >= rb->read ? rb->size : rb->read) - rb->write;
}

int rs_rb_write_incr(struct rs_rb *rb, size_t length)
{
	if (NULL == rb)
		return -EINVAL;
	if (rb->len + length > rb->size)
		return -ENOBUFS;

	rb->len += length;
	rb->write += length;
	rb->write &= rb->size_mask;

	return 0;
}
