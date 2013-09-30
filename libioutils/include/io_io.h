/**
 * @file io_io.h
 * @brief Duplex io source with managed reads and writes
 *
 * @date May 2011
 * @author Jean-Baptiste Dubois
 * @copyright Copyright (C) 2011 Parrot S.A.
 */

/* TODO get rid of user data */

#ifndef IO_IO_H_
#define IO_IO_H_

#include <rs_rb.h>
#include <rs_dll.h>

#include <io_mon.h>
#include <io_src_tmr.h>

/**
 * @enum io_io_state
 * @brief current state of the read or write contexts, for internal use only
 */
enum io_io_state {
	IO_IO_STOPPED,/**< read or write is stopped */
	IO_IO_STARTED,/**< read or write is started */
	IO_IO_ERROR,  /**< read or write encountered an error */
};

/* forward reference for io_io_read_cb_t definition */
struct io_io;

/**
 * Callback called when some data is ready to be consumed
 * @param io IO context
 * @param rb ring buffer containing the data just read
 * @param data user data as was passed in io_io_read_start()
 * @return 0 if and only if more data is needed on read
 */
typedef int (*io_io_read_cb_t) (struct io_io *io, struct rs_rb *rb, void *data);

/**
 * @def IO_IO_RB_BUFFER_SIZE
 * @brief size of the ring buffer's buffer
 */
#define IO_IO_RB_BUFFER_SIZE 2048

/**
 * @struct io_io_read_ctx
 * @brief context for reading data from the IO
 */
struct io_io_read_ctx {
	enum io_io_state state;			/**< io read ctx state */
	struct rs_rb rb;			/**< io read ring buffer */
	char rb_buffer[IO_IO_RB_BUFFER_SIZE];	/**< ring buffer buffer */
	io_io_read_cb_t cb;			/**< io read callback */
	void *data;				/**< callback user data */
	int ign_eof;				/**< ignore end of file */
};

/**
 * @enum io_io_write_status
 * @brief status of the io's write context
 */
enum io_io_write_status {
	IO_IO_WRITE_OK,		/**< write succeeded */
	IO_IO_WRITE_ERROR,	/**< write failed */
	IO_IO_WRITE_TIMEOUT,	/**< write ready timeout */
	IO_IO_WRITE_ABORTED,	/**< write aborted */
};

/* forward reference for io_io_write_cb_t definition */
struct io_io_write_buffer;

/**
 * Type of the callbacks notified when a buffer has been written or on error
 * @param buffer buffer written or on which an error occurred
 * @param status Status of the IO context concerning this buffer
 */
typedef void (*io_io_write_cb_t) (struct io_io_write_buffer *buffer,
		enum io_io_write_status status);

/**
 * @struct io_io_write_buffer
 * @brief Buffer for feeding to an IO's write context
 */
struct io_io_write_buffer {
	struct rs_node node;	/**< node for chaining */
	io_io_write_cb_t cb;	/**< user callback */
	void *data;		/**< user data */
	size_t length;		/**< write buffer length */
	void *address;		/**< write buffer data address*/
};

/**
 * @struct io_io_write_ctx
 * @brief context for writing data to the IO
 */
struct io_io_write_ctx {
	struct io_src src;		/**< io write source, used if needed */
	enum io_io_state state;		/**< io write state */
	int timeout;			/**< io write ready timeout in ms */
	struct io_src_tmr timer;	/**< io write timer */
	struct rs_dll buffers;		/**< io write buffers */
	struct io_io_write_buffer *current;	/**< io write current buffer */
	size_t nbwritten;		/**< current buffer bytes written */
	size_t nbeagain;		/**< number of eagain received */
};

/**
 * @struct io_io
 * @brief Main context, represents a duplex IO source, with one duplex or two
 * half-duplex file descriptor(s)
 */
struct io_io {
	/** io duplex source if fd_in == fd_out, read source otherwise */
	struct io_src src;
	/** equals &src if duplex, writectx.src otherwise */
	struct io_src *write_src;
	char *name;			/**< io name, for logging purpose */
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
 * @return Negative errno-compatible value on error, 0 on success
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

/**
 * Start reading io
 * @param io IO context
 * @param cb Callback called when some data is ready to be consumed
 * @param data User data which will be passed back to cb on each calls
 * @param clear
 * @return Negative errno-compatible value on error, 0 on success
 */
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

/**
 * Stops reading io
 * @param io IO context
 * @return Negative errno-compatible value on error, 0 on success
 */
int io_io_read_stop(struct io_io *io);

/**
 * Says if the IO has it's read started
 * @param io IO context
 * @return non-zero if read is started, 0 otherwise
 */
int io_io_is_read_started(struct io_io *io);

/**
 * Says if the IO has encountered an IO error
 * @param io IO context
 * @return non-zero if an error occurred, 0 otherwise
 */
int io_io_has_read_error(struct io_io *io);

/**
 * Adds a buffer in the write queue. Order is preserve across writes
 * @param io IO context
 * @param buffer Buffer to append.
 * @return Negative errno-compatible value on error, 0 on success
 */
int io_io_write_add(struct io_io *io, struct io_io_write_buffer *buffer);

/* abort all write buffers in io write queue
 * (buffer cb invoked with status IO_IO_WRITE_ABORTED) */
/**
 * Aborts all the buffers currently pending in the write queue
 * @param io IO context
 * @return Negative errno-compatible value on error, 0 on success
 */
int io_io_write_abort(struct io_io *io);

#endif /* IO_IO_H_ */

