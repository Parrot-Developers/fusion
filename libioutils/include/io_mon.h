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

#include <io_src.h>

/**
 * @def IO_EPOLL_ERROR_EVENTS
 * @brief Epoll events considered as an error when occurring on a source
 */
#define IO_EPOLL_ERROR_EVENTS (EPOLLERR | EPOLLHUP | EPOLLRDHUP)

/**
 * @typedef io_mon_t
 * @brief Monitor's context
 */
typedef struct io_mon io_mon_t;

/**
 * @brief Monitor creation and initialization
 *
 * Creates a monitoring context, ready for use to monitor IO events in an event
 * loop
 * @return Monitor context on success, NULL otherwise
 */
io_mon_t *io_mon_new(void);

/**
 * Add a source to the pool of sources we monitor. The monitoring is activated
 * automatically only if it's an input source
 * @param mon Monitor's context
 * @param src Source to add, source's file descriptor must be unique across
 * sources
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_add_source(io_mon_t *mon, io_src_t *src);

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
int io_mon_activate_out_source(io_mon_t *mon, io_src_t *src, int active);

/**
 * Returns the monitor's file descriptor for use in an I/O loop
 * @param mon Monitor context
 * @return Monitor's file descriptor, -1 on error
 */
int io_mon_get_fd(io_mon_t *monitor);

/**
 * When monitor's fd is ready for reading operation, a call to
 * io_mon_process_events will dispatch each event to the relevant
 * callback
 * @param mon Monitor's context
 * @return negative errno value on error, 0 otherwise
 */
int io_mon_process_events(io_mon_t *mon);

/**
 * Destroys monitor's allocated resources.
 * @param monitor Monitor's context, NULL in output
 */
void io_mon_delete(io_mon_t **mon);

#endif /* IO_MONITOR_H_ */
