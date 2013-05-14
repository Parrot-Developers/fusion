/**
 * @file rs_hmap.h
 *
 * @brief hash map implementation, extracted from mambo
 *
 * Copyright (C) 2011 Parrot S.A.
 *
 * @author Jean-Baptiste Dubois
 * @author nicolas.carrier@parrot.com
 * @date May 2011
 */

#ifndef RS_HMAP_H_
#define RS_HMAP_H_

/**
 * @struct rs_hmap_entry
 * @brief Hash map entry structure
 */
struct rs_hmap_entry {
	void *data;			/**< entry data */
	char *key;			/**< entry key */
	struct rs_hmap_entry *next;	/**< next entry with same hash value*/
};

/**
 * @struct rs_hmap
 * @brief Hash map structure
 */
struct rs_hmap {
	struct rs_hmap_entry **buckets;	/**< hash map buckets */
	size_t size;			/**< hash map size (prime number) */
};

/**
 * Initializes a hash map. When not used anymore, a hash map must be cleaned
 * with a call to rs_hmap_clean()
 * @param map Hash map to initialize
 * @param size Size of the bucket. It is fixed for all the lifetime of the hash
 * map. can't be more than RS_HMAP_PRIME_MAX
 * @return Negative errno-compatible value on error, 0 on success
 */
int rs_hmap_init(struct rs_hmap *map, size_t size);

/**
 * Reinitializes a hash map. Releases internally used ressources.
 * @param map Hash map to clean
 * @return Negative errno-compatible value on error, 0 on success
 */
int rs_hmap_clean(struct rs_hmap *map);

/**
 * Lookup an entry in hash map. If two pieces of data have been inserted for the
 * same key, the first found is returned.
 *
 * @param map Hash map
 * @param key String key
 * @param data In output, a pointer to a matching data, or NULL if no entry was
 * found. can't be NULL
 * @return Negative errno-compatible value on error, 0 on success
 */
int rs_hmap_lookup(struct rs_hmap *map, const char *key, void **data);

/**
 * Insert an entry in hash map. Insertion is performed even if the element is
 * already present.
 *
 * @param map Hash map
 * @param key String key
 * @param data Piece of data to associate with the key. Can be NULL
 * @return Negative errno-compatible value on error, 0 on success
 */
int rs_hmap_insert(struct rs_hmap *map, const char* key, void *data);

/**
 * Remove an entry from hash map and retrieve associated data. If the data has
 * been inserted twice, only the first occurrence found is removed.
 *
 * @param map Hash map
 * @param key String key
 * @param data In output, a pointer to a matching data, or NULL if no entry was
 * found. can't be NULL
 * @return Negative errno-compatible value on error, 0 on success. -ENOENT if
 * the entry wasn't found
 */
int rs_hmap_remove(struct rs_hmap *map, const char *key, void **data);

#endif /* RS_HTABLE_H_ */
