/**
 * @file rs_hmap.c
 *
 * @brief hash map implementation, extracted from mambo
 *
 * Copyright (C) 2011 Parrot S.A.
 *
 * @author Jean-Baptiste Dubois
 * @date May 2011
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include "rs_hmap.h"
#include "rs_utils.h"

/**
 * @var hash_prime
 * @brief In the course of designing a good hashing configuration, it is helpful
 * to have a list of prime numbers for the hash map size. It minimizes
 * clustering in the hashed table.
 * */
static const uint32_t hash_prime[] = {
	2, 3, 7, 13, 31, 61, 127, 251, 509, 1021,
	2039, 4093, 8191, 16381, 32749, 65521, 131071,
	262139, 524287, 1048573, 2097143, 4194301, 8388593,
	16777213, 33554393, 67108859, 134217689, 268435399,
	536870909, 1073741789,
	RS_HMAP_PRIME_MAX
};

/**
 * Converts a string to a hash value. by Daniel Bernstein djb2 string hash
 * function
 * @see http://www.cse.yorku.ca/~oz/hash.html
 * @param key String to compute the hash of
 * @return hash value
 **/
static uint32_t hash_string(const char *key)
{
	const unsigned char *str = (const unsigned char *)key;
	uint32_t hash = 5381;
	unsigned int c;
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

int rs_hmap_init(struct rs_hmap *map, size_t size)
{
	size_t i = 0;

	if (NULL == map)
		return -EINVAL;
	if (RS_HMAP_PRIME_MAX < size)
		return -E2BIG;

	/* get upper prime number */
	while (hash_prime[i] <= size)
		i++;

	map->size = hash_prime[i];
	map->buckets = calloc(map->size, sizeof(*map->buckets));
	if (NULL == map->buckets)
		return -errno;

	return 0;
}

int rs_hmap_clean(struct rs_hmap *map)
{
	size_t i;
	struct rs_hmap_entry *entry, *next;

	if (NULL == map)
		return -EINVAL;

	for (i = 0; i < map->size; i++) {
		entry = map->buckets[i];
		while (entry) {
			next = entry->next;
			free(entry->key);
			free(entry);
			entry = next;
		}
	}
	free(map->buckets);
	memset(map, 0, sizeof(*map));

	return 0;
}

int rs_hmap_lookup(struct rs_hmap *map, const char *key,
		void **data)
{
	struct rs_hmap_entry *entry;
	uint32_t hash;

	if (NULL == map || str_is_invalid(key) || NULL == data)
		return -EINVAL;

	*data = NULL;
	hash = hash_string(key);
	hash = hash % map->size;
	entry = map->buckets[hash];

	if (!entry)
		return -ENOENT;

	/* compare keys only on collision */
	if (NULL != entry->next) {
		while (entry && (strcmp(key, entry->key) != 0))
			entry = entry->next;

		if (NULL == entry)
			return -ENOENT;
	}

	(*data) = entry->data;

	return 0;
}

int rs_hmap_insert(struct rs_hmap *map, const char *key, void *data)
{
	int ret;
	uint32_t hash;
	struct rs_hmap_entry *entry;

	if (NULL == map || str_is_invalid(key))
		return -EINVAL;

	entry = calloc(1, sizeof(*entry));
	if (NULL == entry) {
		ret = -ENOMEM;
		goto out;
	}

	entry->data = data;
	entry->key = strdup(key);
	if (NULL == entry->key) {
		ret = -ENOMEM;
		goto out;
	}
	hash = hash_string(key);
	hash = hash % map->size;

	/* insert at list head */
	entry->next = map->buckets[hash];
	map->buckets[hash] = entry;

	return 0;
out:
	if (NULL != entry) {
		free(entry->key);
		free(entry);
	}

	return ret;
}

int rs_hmap_remove(struct rs_hmap *map, const char *key,
			 void **data)
{
	struct rs_hmap_entry *entry, *prev = NULL;
	uint32_t hash;

	if (NULL == map || str_is_invalid(key) || NULL == data)
		return -EINVAL;
	if (NULL != data)
		*data = NULL;

	hash = hash_string(key);
	hash = hash % map->size;
	entry = map->buckets[hash];

	/* compare keys only on collision */
	if (NULL != entry->next) {
		while (NULL != entry && (strcmp(key, entry->key) != 0)) {
			prev = entry;
			entry = entry->next;
		}
	}

	if (NULL == entry)
		return -ENOENT;

	/* copy data */
	if (NULL != data)
		*data = entry->data;

	/* remove entry */
	if (NULL == prev)
		map->buckets[hash] = entry->next;
	else
		prev->next = entry->next;

	free(entry->key);
	free(entry);

	return 0;
}
