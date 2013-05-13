/******************************************************************************
* @file mb_hash.h
*
* @brief mambo hash table
*
* Copyright (C) 2011 Parrot S.A.
*
* @author Jean-Baptiste Dubois
* @date May 2011
******************************************************************************/

#ifndef _MB_HASH_H_
#define _MB_HASH_H_

/* hash table entry structure*/
struct mb_hash_entry {
	void *data;			/* entry data */
	char *key;			/* entry key */
	struct mb_hash_entry *next;	/* next entry with same hash value*/
};

/* hash table structure */
struct mb_hash_table {
	struct mb_hash_entry **buckets;	/* hash table buckets */
	size_t size;			/* hash table size (prime number) */
	size_t nbentries;
};

int mb_hash_table_create(struct mb_hash_table *tab, size_t size);

int mb_hash_table_destroy(struct mb_hash_table *tab);

int mb_hash_table_lookup(struct mb_hash_table *tab, const char *key,
			 void **data);

int mb_hash_table_insert(struct mb_hash_table *tab, const char* key,
			 void *data);

int mb_hash_table_remove(struct mb_hash_table *tab, const char *key,
			 void **data);

#endif /*_MB_HASH_H_*/
