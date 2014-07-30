/**
 * @file io_src_inot.h
 * @date 18 jul. 2014
 * @author nicolas.carrier@parrot.com
 * @brief Source for inotify file descriptors. Allows to monitor events
 * occurring on files/directories: creation, modification, open...
 *
 * Copyright (C) 2014 Parrot S.A.
 */

#ifndef IO_SRC_INOT_H_
#define IO_SRC_INOT_H_
#include <sys/inotify.h>

#include "io_src.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct io_src_inot
 * @brief Inotify source type
 */
struct io_src_inot;

/**
 * @struct io_src_inot_watch
 * @brief structure for describing a watch to install
 */
struct io_src_inot_watch;

/**
 * @typedef io_src_inot_cb_t
 * @brief Type of the callback used to be notified of inotify events
 */
typedef void (io_src_inot_cb_t)(struct io_src_inot *inot,
		struct inotify_event *evt, struct io_src_inot_watch *watch);

/**
 * @struct io_src_inot
 * @brief Inotify source type
 */
struct io_src_inot {
	/** inner monitor source */
	struct io_src src;
	/*
	 * watch descriptors are stored in two containers: one indexing by path,
	 * so that lookups performed in io_src_inot_add_watch() and
	 * io_src_inot_rm_watch(), the other indexing by the watch descriptor
	 * use internally by the inotify API, use by lookups performed when
	 * inotify events are processed.
	 * tsearch(3) is used, because they are already part of POSIX, so they
	 * are already implemented and because the eglibc implementation is
	 * based on red-black trees, which should be fairly efficient.
	 */
	/** watch descriptors indexed by their path */
	void *watches_by_path;
	/** watch descriptors indexed by their watch descriptor */
	void *watches_by_wd;
};

/**
 * @struct io_src_inot_watch
 * @brief structure for describing a watch to install
 */
struct io_src_inot_watch {
	/** watch descriptor, identifies a watch */
	int wd;
	/** path of the file system element being watched */
	const char *path;
	/** inotify events set */
	uint32_t events;
	/** callback called on events concerning the watch descriptor */
	io_src_inot_cb_t *cb;
};

/**
 * @brief Initializes an inotify source for watching events on files and
 * directories
 * @param inot Inotify source
 * @return errno-compatible negative value on error, 0 on success
 */
int io_src_inot_init(struct io_src_inot *inot);

/**
 * @brief Returns the underlying io_src of the inotify source
 * @param inot inotify source
 * @return io_src of the inotify source
 */
static inline struct io_src *io_src_inot_get_source(struct io_src_inot *inot)
{
	return NULL == inot ? NULL : &inot->src;
}

/**
 * @brief adds, or modifies a watch for a file
 * @see inotify_add_watch
 * @param inot Inotify source
 * @param watch Description of the watch to install. The path field indicates
 * which is the file to monitor, the events fields says what events to monitor,
 * as described in man inotify(7), section inotify events and the cb field
 * indicates the function to call when an event occurs. Both three fields must
 * be valid and the other fields are ignored in input. In output, wd is set to
 * the watch descriptor value returned by inotify_add_watch. The content of the
 * path, this API is interested in, is copied internally, thus the caller is
 * free to dispose this structure after the function has returned.
 * @return errno-compatible negative value on error, 0 on success
 */
int io_src_inot_add_watch(struct io_src_inot *inot,
		struct io_src_inot_watch *watch);

/**
 * @brief Removes a watch for a path
 * @param inot Inotify source
 * @param watch Watch to remove, only the field "path" is used, for the lookup
 * of the internal watch descriptor copy
 * @return errno-compatible negative value on error, -ENOENT if no watch is
 * installed for the given path, 0 on success
 */
int io_src_inot_rm_watch(struct io_src_inot *inot,
		struct io_src_inot_watch *watch);

/**
 * @brief Cleans up an inotify source, by properly closing fd, zeroing fields
 * etc...
 * @param inot Inotify source
 */
void io_src_inot_clean(struct io_src_inot *inot);

#ifdef __cplusplus
}
#endif

#endif /* IO_SRC_INOT_H_ */
