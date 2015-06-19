/**
 * @file io_mon.h
 * @date 16 oct. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Monitor, i.e. set of sources (file descriptors) with facilities for
 * I/O event processing
 *
 * Copyright (C) 2012 Parrot S.A.
 */

#ifndef IO_MONITOR_H_
#define IO_MONITOR_H_
#include <sys/epoll.h>

#include <stdbool.h>

#include <io_src.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct io_mon
 * @brief global monitor's context, handles the pool of sources and callbacks
 */
struct io_mon {
	/** source for simplifying the integration with another io_mon */
	struct io_src src;
	/** sources list for I/O operations */
	/*
	 * note the first node can't be a real source in order to guarantee that
	 * removing directly a node from the monitor, won't let the head broken.
	 * So the first valid source is source.next (if not NULL)
	 */
	struct rs_node source;
	/** file descriptor for monitoring all the sources */
	int epollfd;
};

/**
 * Initializes a monitor context
 * @param mon Monitor context to initialize
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_init(struct io_mon *mon);

/**
 * Gets the underlying file descriptor of the monitor
 * @param mon Monitor
 * @return file descriptor, negative errno-compatible value on error
 */
int io_mon_get_fd(struct io_mon *mon);

/**
 * If a user of a library using libioutils internally, want to monitor it in
 * another libioutils monitor, the library can export it's monitor's source in
 * order to "nest" the corresponding monitor without having to create manually
 * a source for it. This source can be retrieved at any time on a monitor, but
 * will be valid only after the io_mon_init() call
 * @param mon Monitor to retrieve the source of
 * @return Source whose file descriptor is the one returned by io_mon_get_fd()
 * and whose callback calls io_mon_process_events()
 */
struct io_src *io_mon_get_source(struct io_mon *mon);

/**
 * Add a source to the pool of sources we monitor. The monitoring is activated
 * automatically only for the input direction of the source, if relevant
 * @param mon Monitor's context
 * @param src Source to add, previously initialized. Source's file descriptor
 * must be unique across sources. The file descriptor is forced non-blocking
 * when added
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_add_source(struct io_mon *mon, struct io_src *src);

/**
 * Add sources to the pool of sources we monitor. The monitoring is activated
 * automatically only for the input direction of each source, if relevant
 * @param mon Monitor's context
 * @param ... Sources to add, previously initialized. Source's file descriptor
 * must be unique across sources. The file descriptor is forced non-blocking
 * when added. A NULL source must end the list.
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_add_sources(struct io_mon *mon, ...)
	__attribute__ ((sentinel(0)));

/**
 * Checks whether a source is registered in a given monitor.
 * @param mon Monitor's context
 * @param src Source to test for registration
 * @return true if the source is registered and false if it isn't or on error.
 * In this case, errno is set.
 */
bool io_mon_is_registered(struct io_mon *mon, struct io_src *src);

/**
 * De-registers a source from the monitor
 * @param mon Monitor's context
 * @param src Source to de-register
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_remove_source(struct io_mon *mon, struct io_src *src);

/**
 * De-registers a list of source from the monitor
 * @param mon Monitor's context
 * @param src Source to de-register
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_remove_sources(struct io_mon *mon, ...)
	__attribute__ ((sentinel(0)));

/**
 * Dumps the events in an epoll event flag set
 * @param events Epoll events set
 */
void io_mon_dump_epoll_event(uint32_t events);

/**
 * (De-)Activates the monitoring of a particular output source
 * @param mon Monitor
 * @param src Source to (de-)activate
 * @param active non-zero if the source must be monitored, 0 otherwise
 *
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_activate_out_source(struct io_mon *mon, struct io_src *src,
		int active);

/**
 * (De-)Activates the monitoring of a particular input source
 * @param mon Monitor
 * @param src Source to (de-)activate
 * @param active non-zero if the source must be monitored, 0 otherwise
 *
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_activate_in_source(struct io_mon *mon, struct io_src *src,
		int active);

/**
 * @brief Polls for events on the registered sources.
 *
 * When monitor's fd is ready for reading operation, a call to
 * io_mon_poll will dispatch each event to the relevant
 * callback.<br />
 * If no source has pending events, blocks during the given amount of time<br />
 * Sources which encounter errors (io_src_has_error() returns true) are removed
 * automatically
 * @param timeout Number of milliseconds io_mon_poll should block waiting for
 * events. If -1, blocks indefinitely, if 0, returns immediately
 *
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_poll(struct io_mon *mon, int timeout);

/**
 * @brief processes pending events. Doesn't block. Any source with error is
 * removed after the user has been called back.
 *
 * Equivalent to io_mon_poll(mon, 0);
 *
 * @see io_mon_poll
 * @param mon Monitor's context
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_process_events(struct io_mon *mon);

/**
 * Cleans up a monitor, unregister the sources and releases the resources
 * @param mon Monitor context
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_clean(struct io_mon *mon);

#ifdef __cplusplus
}
#endif

#endif /* IO_MONITOR_H_ */
