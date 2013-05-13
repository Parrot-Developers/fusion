/**
 * @file rs_htable.c
 *
 * @brief hash table implementation, extracted from mambo
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

#include "rs_htable.h"


/* In the course of designing a good hashing configuration,
 * it is helpful to have a list of prime numbers for the hash table size.
 * it minimizes clustering in the hashed table
 * */
static const uint32_t hash_prime[] = {
	2, 3, 7, 13, 31, 61, 127, 251, 509, 1021,
	2039, 4093, 8191, 16381, 32749, 65521, 131071,
	262139, 524287, 1048573, 2097143, 4194301, 8388593,
	16777213, 33554393, 67108859, 134217689, 268435399,
	536870909, 1073741789, 2147483647
};

/*
 * create a hash table
 */
int rs_htable_create(struct rs_htable *tab, size_t size)
{
	size_t i = 0;
	/* get upper prime number */
	while (hash_prime[i] <= size)
		i++;

	tab->size = hash_prime[i];
	tab->buckets = calloc(tab->size, sizeof(*tab->buckets));
	tab->nbentries = 0;
	return 0;
}

/*
 * destroy a hash table
 */
int rs_htable_destroy(struct rs_htable *tab)
{
	size_t i;
	struct rs_htable_entry *entry, *next;
	for (i = 0; i < tab->size; i++) {
		entry = tab->buckets[i];
		while (entry) {
			next = entry->next;
			free(entry->key);
			free(entry);
			entry = next;
		}
	}
	free(tab->buckets);
	return 0;
}

/**
 * Converts a string to a hash value.
 * by Daniel Bernstein djb2 string hash function
 * http://www.cse.yorku.ca/~oz/hash.html
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
/**
 * Lookup an entry in hash table.
 *
 * @param tab hash table
 * @param key string key
 * @return A  pointer to a matching data, or NULL if no entry was found
 */
int rs_htable_lookup(struct rs_htable *tab, const char *key,
		void **data)
{
	struct rs_htable_entry *entry;
	uint32_t hash;

	hash = hash_string(key);
	hash = hash % tab->size;
	entry = tab->buckets[hash];

	if (!entry)
		return -ENOENT;

	/* compare keys only on collision */
	if (entry->next) {
		while (entry && (strcmp(key, entry->key) != 0))
			entry = entry->next;

		if (!entry)
			return -ENOENT;
	}

	(*data) = entry->data;
	return 0;
}

/**
 * Insert an entry in hash table.
 *
 * @param tab hash table
 * @param key string key
 * @return     0 upon success, or -1 upon failure
 */
int rs_htable_insert(struct rs_htable *tab, const char *key, void *data)
{
	uint32_t hash;
	struct rs_htable_entry *entry;

	assert(tab && key && data);
	entry = (struct rs_htable_entry *)malloc(sizeof(struct rs_htable_entry));
	if (!entry)
		return -ENOMEM;

	entry->data = data;
	entry->key = strdup(key);
	hash = hash_string(key);
	hash = hash % tab->size;

	/* insert at list head */
	entry->next = tab->buckets[hash];
	tab->buckets[hash] = entry;
	tab->nbentries++;
	return 0;
}

/**
 * Remove an entry from hash table and retrieve associated data .
 *
 * @param tab hash table
 * @param key string key
 * @param free 1 to free key
 * @return     0 upon success, or -1 upon failure
 */
int rs_htable_remove(struct rs_htable *tab, const char *key,
			 void **data)
{
	struct rs_htable_entry *entry, *prev = NULL;
	uint32_t hash;

	hash = hash_string(key);
	hash = hash % tab->size;
	entry = tab->buckets[hash];

	/* compare keys only on collision */
	if (entry->next) {
		while (entry && (strcmp(key, entry->key) != 0)) {
			prev = entry;
			entry = entry->next;
		}
	}

	if (!entry)
		return -ENOENT;

	/* copy data */
	if (data)
		*data = entry->data;

	/* remove entry */
	if (!prev)
		tab->buckets[hash] = entry->next;
	else
		prev->next = entry->next;

	free(entry->key);
	free(entry);
	tab->nbentries--;
	return 0;
}
