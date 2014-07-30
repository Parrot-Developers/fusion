/**
 * @file io_src_inot.c
 * @brief Source for inotify file descriptors. Allows to monitor events
 * occurring on files/directories: creation, modification, open...
 *
 * When registering a watch on a file (or directory), information concerning
 * this watch are stored in the 'watches' container, so that it can be given
 * back to the client when he is notified of an event.
 *
 * @date 21 juil. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <sys/inotify.h>
#include <sys/ioctl.h>

#include <search.h>

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <inttypes.h>

#include "io_src_inot.h"
#include "io_platform.h"
#include "io_utils.h"

#include "rs_utils.h"

/**
 * @def IO_SRC_INVALID_WATCH_DESCRIPTOR
 * @brief invalid value for an inotify watch descriptor
 */
#define IO_SRC_INVALID_WATCH_DESCRIPTOR -1

/**
 * @def to_inot
 * @brief retrieve an inotify context knowing it's libioutils source
 */
#define to_inot(s) rs_container_of((s), struct io_src_inot, src)

/**
 * @def to_inot_watch
 * @brief retrieve an inotify watch descriptor knowing it's node
 */
#define to_inot_watch(n) rs_container_of((n), struct io_src_inot_watch, node)

/**
 * @brief Destroys a watch and frees it's allocated resources
 * @param watch Inotify watch descriptor structure to free
 */
static void inot_watch_delete(struct io_src_inot_watch **watch)
{
	struct io_src_inot_watch *w;

	if (watch == NULL || *watch == NULL)
		return;
	w = *watch;

	/*
	 * the path field is const to inform the user that in add_watch and
	 * rm_watch, this field won't be modified. But when we allocate ourself
	 * a watch (clone) we strdup() it and so we must free it
	 */
	rs_str_free((char **)&w->path);
	memset(w, 0, sizeof(*w));
	free(w);

	*watch = NULL;
}

/**
 * @brief Callback for deleting a watch in a list
 * @param node node of the watch to delete
 */
static void watch_remove(void *watch)
{
	struct io_src_inot_watch *w = watch;

	inot_watch_delete(&w);
}

/**
 * @brief Does nothing. Needed because tdestroy stupidly doesn't accept a NULL
 * pointer
 * @param node node of the watch to delete
 */
static void dummy_watch_remove(void *watch)
{

}

/**
 * @brief Compare watch descriptors by their path field.
 * @param w1 first watch descriptor to compare
 * @param w2 second watch descriptor to compare
 * @return an integer less than, equal to, or greater than zero if w1 is found,
 * respectively, to be less than, to match, or be greater than w2.
 */
static int compare_watches_by_path(const void *w1, const void *w2)
{
	const struct io_src_inot_watch *watch1 = w1;
	const struct io_src_inot_watch *watch2 = w2;

	return strcmp(watch1->path, watch2->path);
}

/**
 * @brief Compare watch descriptors by their path wd.
 * @param w1 first watch descriptor to compare
 * @param w2 second watch descriptor to compare
 * @return an integer less than, equal to, or greater than zero if w1 is found,
 * respectively, to be less than, to match, or be greater than w2.
 */
static int compare_watches_by_wd(const void *w1, const void *w2)
{
	const struct io_src_inot_watch *watch1 = w1;
	const struct io_src_inot_watch *watch2 = w2;

	return watch1->wd - watch2->wd;
}

/**
 * Lookup function for finding a watch descriptor in a container, with a given
 * matching criteria
 * @param watches watch descriptors container
 * @param watch structure holding the information used to match the watch
 * descriptor with
 * @param compare function of comparison for the matching criteria
 * @return watch descriptor if found, NULL otherwise
 */
static struct io_src_inot_watch *find_watch(void *watches,
		struct io_src_inot_watch *watch, comparison_fn_t compare)
{
	struct io_src_inot_watch **ret;

	ret = tfind(watch, watches, compare);
	if (ret == NULL)
		return NULL;

	return *ret;
}

/**
 * @brief Finds a watch descriptor structure, knowing it's watch descriptor
 * index
 * @param inot inotify source
 * @param wd inotify watch descriptor index
 * @return watch descriptor if found, NULL otherwise
 */
static struct io_src_inot_watch *find_watch_by_wd(struct io_src_inot *inot,
		int wd)
{
	struct io_src_inot_watch w = { .wd = wd };

	return find_watch(&inot->watches_by_wd, &w, compare_watches_by_wd);
}

/**
 * @brief Finds a watch descriptor structure, knowing the path it monitors
 * @param inot inotify source
 * @param path Path of the file or directory being monitored
 * @return watch descriptor if found, NULL otherwise
 */
static struct io_src_inot_watch *find_watch_by_path(struct io_src_inot *inot,
		const char *path)
{
	struct io_src_inot_watch w = { .path = path };

	return find_watch(&inot->watches_by_path, &w, compare_watches_by_path);
}

/**
 * @brief Store a watch descriptor in the watch descriptors set
 * @param inot inotify source
 * @param watch watch descriptor to store
 * @return errno-compatible negative value on error, 0 otherwise
 */
static int store_watch(struct io_src_inot *inot,
		struct io_src_inot_watch *watch)
{
	void *val;

	val = tsearch(watch, &inot->watches_by_path, compare_watches_by_path);
	if (val == NULL)
		return -ENOMEM;
	val = tsearch(watch, &inot->watches_by_wd, compare_watches_by_wd);
	if (val == NULL) {
		tdelete(watch, &inot->watches_by_path, compare_watches_by_path);
		return -ENOMEM;
	}

	return 0;
}

/**
 * @brief Removes a watch descriptor, knowing the path being monitored
 * @param inot inotify source
 * @param path Path of the watch descriptor to remove
 * @return watch descriptor removed
 */
static struct io_src_inot_watch *remove_watch(struct io_src_inot *inot,
		const char *path)
{
	struct io_src_inot_watch *w = find_watch_by_path(inot, path);

	if (w == NULL)
		return NULL;

	tdelete(w, &inot->watches_by_path, compare_watches_by_path);
	tdelete(w, &inot->watches_by_wd, compare_watches_by_wd);

	return w;
}

/**
 * @brief Initializes the watch descriptors set
 * @param inot inotify source
 * @return errno-compatible negative value on error, 0 otherwise
 */
static int init_watches(struct io_src_inot *inot)
{
	inot->watches_by_path = inot->watches_by_wd = NULL;

	return 0;
}



/**
 * @brief Cleans up the watch descriptors set and destroys all the watch
 * descriptors
 * @param inot inotify source
 */
static void clean_watches(struct io_src_inot *inot)
{
	tdestroy(inot->watches_by_path, dummy_watch_remove);
	inot->watches_by_path = NULL;
	tdestroy(inot->watches_by_wd, watch_remove);
	inot->watches_by_wd = NULL;
}

/**
 * @brief Processes inotify events, calling back the user
 * @param inot inotify events source
 * @param buf buffer containing the inotify events just read, to process
 * @param bytes number of bytes in buf
 * @return errno-compatible negative value on error, 0 on success
 */
static int process_events(struct io_src_inot *inot, char *buf, size_t bytes)
{
	char *p, *upper_bound;
	struct inotify_event *event;
	struct io_src_inot_watch *watch;

	p = buf;
	upper_bound = buf + bytes;
	while (p < upper_bound) {
		/* inotify events can't be partial, according to the kernel */
		event = (struct inotify_event *)p;

		watch = find_watch_by_wd(inot, event->wd);
		/*
		 * if we receive an ignored event and we couldn't find the
		 * watch, it must be because the caller has removed the watch
		 * explicitely by calling io_src_inot_rm_watch()
		 */
		if (watch == NULL && event->mask == IN_IGNORED)
			return 0;
		if (watch == NULL && event->mask != IN_IGNORED)
			return -ENOENT;

		/*
		 * IN_IGNORED are not passed to the user, but they are used
		 * internally to remove the corresponding watch
		 */
		if (event->mask == IN_IGNORED)
			io_src_inot_rm_watch(inot, watch);
		else
			watch->cb(inot, event, watch);

		p += sizeof(struct inotify_event) + event->len;
	}

	return 0;
}

/**
 * @brief callback called when inotify events are ready to be read
 * @param src I/O source
 */
static void inot_cb(struct io_src *src)
{
	ssize_t sret;
	int ret;
	int toread;
	char __attribute__((cleanup(rs_str_free)))*buf = NULL;
	size_t buf_size;

	ret = ioctl(src->fd, FIONREAD, &toread);
	if (ret < 0 || toread == 0)
		return;
	/*
	 * according to fs/notify/inotify/inotify_user.c:inotify_ioctl(),
	 * the value returned by FIONREAD is positive, hence the cast is safe.
	 */
	assert(toread >= 0);
	buf_size = (size_t)toread;

	buf = calloc(1, buf_size);
	if (buf == NULL)
		return;

	sret = read(src->fd, buf, buf_size);
	if (sret < 0)
		return;

	ret = process_events(to_inot(src), buf, buf_size);
}

/**
 * @brief Clones a watch descriptor structure
 *
 * Copies all the fields except wd (which has no meaning before the call to
 * inotify_add_watch() and creates a private copy of the path field
 * @param src Source watch to clone
 * @param dst Output parameter, clone of src
 * @return errno-compatible negative value on error, 0 on success
 */
static int inot_watch_clone(struct io_src_inot_watch *src,
		struct io_src_inot_watch **dst)
{
	int ret;
	struct io_src_inot_watch *d;

	if (src == NULL || dst == NULL)
		return -EINVAL;

	d = calloc(1, sizeof(*d));
	if (d == NULL)
		return -ENOMEM;
	d->wd = IO_SRC_INVALID_WATCH_DESCRIPTOR;
	*d = *src;

	d->path = strdup(src->path);
	if (d->path == NULL) {
		ret = -errno;
		goto err;
	}

	*dst = d;

	return 0;
err:
	inot_watch_delete(&d);

	return ret;
}

/**
 * @brief Tests if a watch is valid and ok to be installed
 * @param watch Watch to test
 * @return
 */
static bool watch_is_invalid(struct io_src_inot_watch *watch)
{
	return watch == NULL || rs_str_is_invalid(watch->path) ||
			watch->events == 0 || watch->cb == NULL;
}

int io_src_inot_init(struct io_src_inot *inot)
{
	int fd;
	int ret;
	if (inot == NULL)
		return -EINVAL;

	memset(inot, 0, sizeof(*inot));

	fd = io_inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
	if (fd < 0)
		return -errno;

	ret = io_src_init(&inot->src, fd, IO_IN, inot_cb);
	if (ret != 0) {
		close(fd);
		return ret;
	}

	ret = init_watches(inot);
	if (ret < 0)
		goto err;

	return 0;
err:
	io_src_inot_clean(inot);

	return ret;
}
/*
 * performs the inotify_add_watch call and keeps the watches set up to date
 * if a watch is already registered for the given path, it will be updated,
 * otherwise, a clone of the watch parameter will be stored
 */
int io_src_inot_add_watch(struct io_src_inot *inot,
		struct io_src_inot_watch *watch)
{
	int ret;
	struct io_src_inot_watch *w;
	bool update; /* true iif the file is already being watched */

	if (inot == NULL || watch_is_invalid(watch))
		return -EINVAL;

	/*
	 * mustn't clone if already registered, just update: guarantees
	 * uniqueness
	 */
	w = find_watch_by_path(inot, watch->path);
	if (w == NULL) {
		/* not watch found for the file, build one */
		update = false;
		ret = inot_watch_clone(watch, &w);
		if (ret != 0)
			return ret;
	} else {
		/* file is already watched, update the watch information */
		update = true;
		w->events = watch->events;
		w->cb = watch->cb;
	}

	ret = inotify_add_watch(inot->src.fd, w->path, w->events);
	if (ret < 0) {
		ret = -errno;
		goto err;
	}

	if (!update) {
		watch->wd = w->wd = ret;
		ret = store_watch(inot, w);
		if (ret != 0)
			goto err;
	}

	return 0;
err:
	remove_watch(inot, watch->path);
	/* on update, because we destroy the watch, we must unregister it */
	if (update && w->wd != IO_SRC_INVALID_WATCH_DESCRIPTOR)
		inotify_rm_watch(inot->src.fd, w->wd);
	inot_watch_delete(&w);

	return ret;
}

int io_src_inot_rm_watch(struct io_src_inot *inot,
		struct io_src_inot_watch *watch)
{
	int ret;
	struct io_src_inot_watch *w;

	if (inot == NULL || watch == NULL || rs_str_is_invalid(watch->path))
		return -EINVAL;

	w = remove_watch(inot, watch->path);
	if (w == NULL)
		return -ENOENT;

	ret = inotify_rm_watch(inot->src.fd, w->wd);
	if (ret < 0) {
		ret = -errno;
		goto out;
	}

	ret = 0;
out:
	inot_watch_delete(&w);

	return ret;
}

void io_src_inot_clean(struct io_src_inot *inot)
{
	if (inot == NULL)
		return;

	clean_watches(inot);
	io_src_clean(&inot->src);
	io_close(&inot->src.fd);

	memset(inot, 0, sizeof(*inot));
}
