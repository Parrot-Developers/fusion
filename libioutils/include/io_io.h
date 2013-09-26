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

enum at_io_dir {
	AT_IO_RX = 0,
	AT_IO_TX,
	AT_IO_BOTH,
};

enum at_io_state {
	AT_IO_STOPPED,
	AT_IO_STARTED,
	AT_IO_ERROR,
};

struct at_io;
/*
 * io read callback,
 * return 0 if more data is needed on read
 */
typedef int (*at_io_read_cb_t) (struct at_io *io, struct rs_rb *rb,
		size_t newbytes, void *data);

#define AT_IO_RB_BUFFER_SIZE 2048

/* io read context */
struct at_io_read_ctx {
	struct io_src src;	/* io read source */
	enum at_io_state state;	/* io read ctx state */
	struct rs_rb rb;	/* io read ring buffer */
	char rb_buffer[AT_IO_RB_BUFFER_SIZE];	/* ring buffer buffer */
	size_t newbytes;	/* number of new bytes added in buffer */
	at_io_read_cb_t cb;	/* io read callback */
	void *data;		/* io read callback user data */
	int ign_eof;		/* ignore end of file */
};

enum at_io_write_status {
	AT_IO_WRITE_OK,		/* write succeed */
	AT_IO_WRITE_ERROR,	/* write failed */
	AT_IO_WRITE_TIMEOUT,	/* write ready timeout */
	AT_IO_WRITE_ABORTED,	/* write aborted */
};

struct at_io_write_buffer;
typedef void (*at_io_write_cb_t) (struct at_io_write_buffer *buffer,
		enum at_io_write_status status);

/* io write buffer */
struct at_io_write_buffer {
	struct rs_node node;	/* node */
	at_io_write_cb_t cb;	/* user callback */
	void *data;		/* user data */
	size_t length;		/* write buffer length */
	void *address;		/* write buffer data address*/
};

/* io write context */
struct at_io_write_ctx {
	struct io_src src;		/* io write source */
	enum at_io_state state;		/* io write state */
	int timeout;			/* io write ready timeout in ms */
	struct io_src_tmr timer;		/* io write timer */
	struct rs_dll buffers;		/* io write buffers */
	struct at_io_write_buffer *current;	/* io write current buffer */
	size_t nbwritten;		/* current buffer bytes written */
	size_t nbeagain;		/* number of eagain received */
};

struct at_io {
	char *name;			/* io name */
	int fds[AT_IO_BOTH];		/* io fds */
	int dupped;
	int log[AT_IO_BOTH];		/* io log */
	struct io_mon *mon;		/* io monitor */
	struct at_io_read_ctx readctx;	/* io read context */
	struct at_io_write_ctx writectx;/* io write context */
};

/* create an io
 * set 0 to stop read on end of file, 1 to continue read */
int at_io_create(struct at_io *io, struct io_mon * mon, const char *name,
		 int fd_in, int fd_out, int ign_eof);

/* destroy an io */
int at_io_destroy(struct at_io *io);

/* start reading io */
int at_io_read_start(struct at_io *io, at_io_read_cb_t cb, void *data,
		int clear);

/* enable/disable rx traffic log */
int at_io_log_rx(struct at_io *io, int enable);

/* enable/disable tx traffic log */
int at_io_log_tx(struct at_io *io, int enable);

/* stop reading io */
int at_io_read_stop(struct at_io *io);

/* get read state */
int at_io_read_state(struct at_io *io);

/* write buffer in io (append it in a buffer queue) */
int at_io_write_add(struct at_io *io, struct at_io_write_buffer *buffer);

/* abort all write buffers in io write queue
 * (buffer cb invoked with status AT_IO_WRITE_ABORTED) */
int at_io_write_abort(struct at_io *io);

#endif /* IO_IO_H_ */

