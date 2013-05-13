/**
 * @file rs_htable.h
 *
 * @brief hash table implementation, extracted from mambo
 *
 * Copyright (C) 2011 Parrot S.A.
 *
 * @author Jean-Baptiste Dubois
 * @date May 2011
 */

#ifndef RS_HTABLE_H_
#define RS_HTABLE_H_

/* hash table entry structure*/
struct rs_htable_entry {
	void *data;			/* entry data */
	char *key;			/* entry key */
	struct rs_htable_entry *next;	/* next entry with same hash value*/
};

/* hash table structure */
struct rs_htable {
	struct rs_htable_entry **buckets;	/* hash table buckets */
	size_t size;			/* hash table size (prime number) */
	size_t nbentries;
};

int rs_htable_create(struct rs_htable *tab, size_t size);

int rs_htable_destroy(struct rs_htable *tab);

int rs_htable_lookup(struct rs_htable *tab, const char *key, void **data);

int rs_htable_insert(struct rs_htable *tab, const char* key, void *data);

int rs_htable_remove(struct rs_htable *tab, const char *key, void **data);

#endif /* RS_HTABLE_H_ */
