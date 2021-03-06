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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def RS_HMAP_PRIME_MAX
 * @brief Maximum size of a has map's bucket, which is a big prime number,
 * fitting into an uint32_t
 */
#define RS_HMAP_PRIME_MAX 2147483647U

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
 * Reinitializes a hash map. Releases internally used resources. Equivalent to
 * rs_hmap_clean_cb(map, NULL);
 * @param map Hash map to clean
 * @return Negative errno-compatible value on error, 0 on success
 */
int rs_hmap_clean(struct rs_hmap *map);

/**
 * Reinitializes a hash map. Releases internally used resources and allows the
 * user to free the resources allocated, still referenced in the map, or to
 * perform any operation needed on each node, via a callback.
 * @param map Hash map to clean
 * @param free_cb callback called on each value still stored in the map, so that
 * the caller can free the corresponding ressources if any.
 * @return Negative errno-compatible value on error, 0 on success
 */
int rs_hmap_clean_cb(struct rs_hmap *map, void (*free_cb)(void *));

/**
 * Lookup an entry in hash map. If two pieces of data have been inserted for the
 * same key, the first found is returned.
 *
 * @param map Hash map
 * @param key String key
 * @param data In output, a pointer to a matching data, or to NULL if no entry
 * was found. can't be NULL
 * @return Negative errno-compatible value on error, 0 on success. -ENOENT if
 * the entry wasn't found
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
int rs_hmap_insert(struct rs_hmap *map, const char *key, void *data);

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

#ifdef __cplusplus
}
#endif

#endif /* RS_HTABLE_H_ */
