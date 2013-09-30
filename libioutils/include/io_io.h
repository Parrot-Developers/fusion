/**
 * @file io_io.h
 * @brief Duplex io source with managed reads and writes
 *
 * @date May 2011
 * @author Jean-Baptiste Dubois
 * @copyright Copyright (C) 2011 Parrot S.A.
 */

#ifndef IO_IO_H_
#define IO_IO_H_

#include <rs_rb.h>
#include <rs_dll.h>

#include <io_mon.h>
#include <io_src_tmr.h>

enum io_io_dir {
	IO_IO_RX = 0,
	IO_IO_TX,
	IO_IO_BOTH,
};

enum io_io_state {
	IO_IO_STOPPED,
	IO_IO_STARTED,
	IO_IO_ERROR,
};

struct io_io;
/*
 * io read callback,
 * return 0 if more data is needed on read
 */
typedef int (*io_io_read_cb_t) (struct io_io *io, struct rs_rb *rb, void *data);

#define IO_IO_RB_BUFFER_SIZE 2048

/* io read context */
struct io_io_read_ctx {
	struct io_src src;	/**< io read source */
	enum io_io_state state;	/**< io read ctx state */
	struct rs_rb rb;	/**< io read ring buffer */
	char rb_buffer[IO_IO_RB_BUFFER_SIZE];	/**< ring buffer buffer */
	io_io_read_cb_t cb;	/**< io read callback */
	void *data;		/**< io read callback user data */
	int ign_eof;		/**< ignore end of file */
};

enum io_io_write_status {
	IO_IO_WRITE_OK,		/**< write succeed */
	IO_IO_WRITE_ERROR,	/**< write failed */
	IO_IO_WRITE_TIMEOUT,	/**< write ready timeout */
	IO_IO_WRITE_ABORTED,	/**< write aborted */
};

struct io_io_write_buffer;
typedef void (*io_io_write_cb_t) (struct io_io_write_buffer *buffer,
		enum io_io_write_status status);

/* io write buffer */
struct io_io_write_buffer {
	struct rs_node node;	/**< node for chaining */
	io_io_write_cb_t cb;	/**< user callback */
	void *data;		/**< user data */
	size_t length;		/**< write buffer length */
	void *address;		/**< write buffer data address*/
};

/* io write context */
struct io_io_write_ctx {
	struct io_src src;		/**< io write source */
	enum io_io_state state;		/**< io write state */
	int timeout;			/**< io write ready timeout in ms */
	struct io_src_tmr timer;	/**< io write timer */
	struct rs_dll buffers;		/**< io write buffers */
	struct io_io_write_buffer *current;	/**< io write current buffer */
	size_t nbwritten;		/**< current buffer bytes written */
	size_t nbeagain;		/**< number of eagain received */
};

struct io_io {
	char *name;			/**< io name */
	int fds[IO_IO_BOTH];		/**< io fds */
	int dupped;			/**< non-zero if the fd in == fd out */
	void (*log_rx)(const char *);	/**< io log in input */
	void (*log_tx)(const char *);	/**< io log in output */
	struct io_mon *mon;		/**< io monitor */
	struct io_io_read_ctx readctx;	/**< io read context */
	struct io_io_write_ctx writectx;/**< io write context */
};

/**
 * Initializes an io
 * @param io IO context to initialize
 * @param mon Monitor
 * @param name Name of the IO, used for logging purpose only
 * @param fd_in File descriptor for reading
 * @param fd_out File descriptor for writing, can be the same as fd_in
 * @param ign_eof set 0 to stop read on end of file, 1 to continue read
 * @return
 */
int io_io_init(struct io_io *io, struct io_mon *mon, const char *name,
		int fd_in, int fd_out, int ign_eof);

/**
 * Cleans up all the resources associated to an io and make it ready for a new
 * call to io_io_init()
 * @param io IO context to cleanup
 * @return Negative errno-compatible value on error, 0 on success
 */
int io_io_clean(struct io_io *io);

/* start reading io */
int io_io_read_start(struct io_io *io, io_io_read_cb_t cb, void *data,
		int clear);

/**
 * Sets the function used for logging input traffic
 * @param io IO context
 * @param log_rx Logging callback for input traffic, NULL for disabling logging
 * @return Negative errno-compatible value on error, 0 on success
 */
int io_io_log_rx(struct io_io *io, void (*log_rx)(const char *));

/**
 * Sets the function used for logging output traffic
 * @param io IO context
 * @param log_tx Logging callback for output traffic, NULL for disabling logging
 * @return Negative errno-compatible value on error, 0 on success
 */
int io_io_log_tx(struct io_io *io, void (*log_tx)(const char *));

/* stop reading io */
int io_io_read_stop(struct io_io *io);

/* get read state */
int io_io_read_state(struct io_io *io);

/* write buffer in io (append it in a buffer queue) */
int io_io_write_add(struct io_io *io, struct io_io_write_buffer *buffer);

/* abort all write buffers in io write queue
 * (buffer cb invoked with status IO_IO_WRITE_ABORTED) */
int io_io_write_abort(struct io_io *io);

#endif /* IO_IO_H_ */

