/**
 * @file rs_hmap.h
 *
 * @brief hash map implementation, extracted from mambo
 *
 * Copyright (C) 2011 Parrot S.A.
 *
 * @author Jean-Baptiste Dubois
 * @date May 2011
 */

#ifndef RS_HMAP_H_
#define RS_HMAP_H_

/**
 * @struct rs_hmap_entry
 * @brief hash map entry structure
 */
struct rs_hmap_entry {
	void *data;			/**< entry data */
	char *key;			/**< entry key */
	struct rs_hmap_entry *next;	/**< next entry with same hash value*/
};

/**
 * @struct rs_hmap
 * @brief hash map structure
 */
struct rs_hmap {
	struct rs_hmap_entry **buckets;	/**< hash map buckets */
	size_t size;			/**< hash map size (prime number) */
	size_t nbentries;
};

int rs_hmap_create(struct rs_hmap *tab, size_t size);

int rs_hmap_destroy(struct rs_hmap *tab);

int rs_hmap_lookup(struct rs_hmap *tab, const char *key, void **data);

int rs_hmap_insert(struct rs_hmap *tab, const char* key, void *data);

int rs_hmap_remove(struct rs_hmap *tab, const char *key, void **data);

#endif /* RS_HTABLE_H_ */
