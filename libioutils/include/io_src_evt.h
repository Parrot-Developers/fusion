/**
 * @file io_src_evt.h
 * @date 22 jan. 2016
 * @author nicolas.carrier@parrot.com
 * @brief Source for evt fds
 *
 * Copyright (C) 2016 Parrot S.A.
 */
#ifndef IO_SRC_EVT_H_
#define IO_SRC_EVT_H_
#include <stdint.h>
#include <stdbool.h>

#include <io_src.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct io_src_evt
 * @brief eventfd source type
 */
struct io_src_evt;

/**
 * @typedef io_src_evt_cb
 * @param evt event source which has been notified
 * @param value value read, it's content depends on whether the semaphore was
 * set to true or not
 */
typedef void (io_src_evt_cb)(struct io_src_evt *evt, uint64_t value);

/**
 * @struct io_src_evt
 * @brief eventfd source type
 */
struct io_src_evt {
	/** underlying io_src */
	struct io_src src;
	/** client callback called on events */
	io_src_evt_cb *cb;
};

/**
 * Initializes an eventfd source
 * @param evt eventfd source to initialize
 * @param cb user callback called on events
 * @param semaphore if set to true, the callback will be notified with a value
 * of one and the internal counter will be decremented by one until it reaches
 * 0, if set to false, the callback will be notified once with value holding the
 * value of the internal counter which will be reset to 0
 * @param initval Initial value to set the counter to
 * @return errno-compatible negative value on error, 0 otherwise
 */
int io_src_evt_init(struct io_src_evt *evt, io_src_evt_cb *cb,
		bool semaphore, int initval);

/**
 * @brief Returns the underlying io_src of the event source
 * @param evt eventfd source
 * @return io_src of the evt source
 */
static inline struct io_src *io_src_evt_get_source(struct io_src_evt *evt)
{
	return NULL == evt ? NULL : &evt->src;
}

/**
 * Send a value to the event sourte
 * @param evt event source to notify
 * @param value value the internal counter will be incremented of
 * @return errno-compatible negative value on erron, 0 otherwise
 */
int io_src_evt_notify(struct io_src_evt *evt, uint64_t value);

/**
 * Cleans the eventfd source, releasing all it's resources
 * @param evt event source
 */
void io_src_evt_clean(struct io_src_evt *evt);

#ifdef __cplusplus
}
#endif

#endif /* IO_SRC_EVT_H_ */
