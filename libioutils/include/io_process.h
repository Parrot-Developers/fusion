/**
 * @file io_process.h
 * @brief Launches a process, allows to retrieve what it writes to it's standard
 * files, to kill it and to be notified when the process terminates
 *
 * @date June 2015
 * @author Nicolas Carrier
 * @copyright Copyright (C) 2011 Parrot S.A.
 */

#ifndef IO_PROCESS_H_
#define IO_PROCESS_H_
#include <stdbool.h>
#include <stdarg.h>

#include "io_mon.h"
#include "io_src.h"
#include "io_src_pid.h"
#include "io_src_sep.h"
#include "io_src_tmr.h"

/**
 * @enum io_process_state
 * @brief state of a process
 */
enum io_process_state {
	/* we let 0 free to be able to detect some potential bad states */
	IO_PROCESS_INITIALIZED = 1,/**< IO_PROCESS_INITIALIZED */
	IO_PROCESS_STARTED,        /**< IO_PROCESS_STARTED */
	IO_PROCESS_DEAD,           /**< IO_PROCESS_DEAD */
};

/**
 * @struct io_process
 * @brief main structure wrapping a process
 */
struct io_process {
	/**
	 * inner monitor, eases the process of registering the process to
	 * another monitor
	 */
	struct io_mon mon;
	/** pid src notifying when the process dies */
	struct io_src_pid pid_src;
	/** client callback called when the process terminates */
	io_pid_cb *termination_cb;
	/**
	 * buffer for input data, if the stdin isn't configured as a source,
	 * will be used to feed data in the process' standard input
	 */
	union {
		/** reference on the buffer passed, not freed */
		const char *ro;
		/** copy of the user supplied buffer, freed once written */
		char *rw;
	};
	/**
	 * true if the user asked to copy internally the buffer content.
	 * tells if we should use ro or rw and if we must free the buffer or not
	 * when we're done writing it's content
	 */
	bool copy;
	/**
	 * length of the data contained in the previous buffer. Relevant only if
	 * buffer isn't NULL
	 */
	size_t len;
	/**
	 * source for feeding data into the process' standard input. Can be
	 * configured manually by the user with io_process_set_stdin_src or
	 * automatically by providing an input buffer
	 */
	struct io_src stdin_src;
	/** underlying file descriptors pair used by the stdin_src */
	int stdin_pipe[2];
	/** source used to retrieve what the process writes to it's stdout */
	struct io_src_sep stdout_src;
	/** underlying file descriptors pair used by the stdout_src */
	int stdout_pipe[2];
	/** source used to retrieve what the process writes to it's stderr */
	struct io_src_sep stderr_src;
	/** underlying file descriptors pair used by the stderr_src */
	int stderr_pipe[2];
	/**
	 * timer the client can set to program signal sending to terminate the
	 * process
	 */
	struct io_src_tmr timeout_src;
	/**
	 * if a timer for killing the process is set, this signal will be sent
	 */
	int signum;
	/**
	 * command line of the process to launch
	 */
	char *command_line;
	/**
	 * size of the command_line buffer
	 */
	size_t command_line_len;
	/**
	 * current state of the process
	 */
	enum io_process_state state;

	/**
	 * public source, we can't publish the monitor's source since we want to
	 * customize the callback
	 */
	struct io_src src;
};

/**
 * @struct io_process_parameters
 * @brief structure used to configure the process, after it has been initialized
 * but before it has been launched
 */
struct io_process_parameters {
	/**
	 * buffer fed into the process' input, mutually exclusive with stdin_src
	 */
	const char *buffer;
	/** used only if buffer is not NULL: amount of data it contains */
	size_t len;
	/**
	 * if true, the buffer will be copied internally, if false, only a
	 * reference will be kept, which implies it must have a longer life
	 * cycle than the io_process structure
	 */
	bool copy;
	/** for stdin_src, mutually exclusive with input buffer parameters */
	io_src_cb_t *stdin_cb;
	/** for stdout_sep_src, mutually exclusive with stdout_src parameters */
	io_src_sep_cb_t *stdout_sep_cb;
	/** first separator character for the stdout separator source */
	int out_sep1;
	/**
	 * second separator character for the stdout separator source, set to
	 * IO_SRC_SEP_NO_SEP2 to disable
	 */
	int out_sep2;
	/** for stdout_src, mutually exclusive with stdout_sep_src parameters */
	io_src_cb_t *stdout_cb;
	/** for stderr_sep_src, mutually exclusive with stderr_src parameters */
	io_src_sep_cb_t *stderr_sep_cb;
	/** first separator character for the stderr separator source */
	int err_sep1;
	/**
	 * second separator character for the stderr separator source, set to
	 * IO_SRC_SEP_NO_SEP2 to disable
	 */
	int err_sep2;
	/** for stderr_src, mutually exclusive with stderr_sep_src parameters */
	io_src_cb_t *stderr_cb;
	/**
	 * when elapsed, the process will receive the signal signum isn't
	 * already dead
	 */
	int timeout;
	/** signal the process will receive when timeout expires, if defined */
	int signum;
};

/**
 * Initializes a process, in the INITIALIZED state
 * @note the structure is automatically cleaned it's last event has been
 * processed, so the client must unregister the source from it's monitor, in the
 * termination callback. After this last event, the io_process structure can be
 * freely destroyed
 * @param process Process to initialize
 * @param termination_cb Function called when the process terminates
 * @param ... List of the command line arguments, the first one must be an
 * absolute path and will be used for both the path to the file to exec() and
 * the argv[0] of the process created. The list must end with a NULL pointer
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_init(struct io_process *process, io_pid_cb termination_cb, ...)
	__attribute__ ((sentinel(0)));

/**
 * Initializes a process, in the INITIALIZED state
 * @note the structure is automatically cleaned it's last event has been
 * processed, so the client must unregister the source from it's monitor, in the
 * termination callback. After this last event, the io_process structure can be
 * freely destroyed
 * @param process Process to initialize
 * @param termination_cb Function called when the process terminates
 * @param args List of the command line arguments, the first one must be an
 * absolute path and will be used for both the path to the file to exec() and
 * the argv[0] of the process created. The list must end with a NULL pointer
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_vinit(struct io_process *process, io_pid_cb termination_cb,
		va_list args);

/*
 * setters for the process' parameters, must be set after init but before
 * launch
 */

/**
 * Defines a buffer which will be fed to the process' standard input. Once
 * finished, the input will be closed
 * @param process Process to configure
 * @param buffer Buffer to feed into standard input
 * @param len Number of bytes contained in the buffer.
 * @param copy If true, the buffer will be copied internally, if false, only a
 * reference will be kept so the buffer must have a longer life cycle than the
 * io_process structure
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_set_input_buffer(struct io_process *process, const char *buffer,
		size_t len, bool copy);

/**
 * Defines a source responsible of feeding data to the process' input
 * @param process Process to configure
 * @param cb Function called when the process is ready to read some data
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_set_stdin_src(struct io_process *process, io_src_cb_t *cb);

/**
 * Defines a separator source which will be notified each time the process
 * writes something to it's standard output
 * @param process Process to configure
 * @param cb Callback notified when a chunk of data is available
 * @param sep1 First separator
 * @param sep2 Second separator, set to IO_SRC_SEP_NO_SEP2 for none
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_set_stdout_sep_src(struct io_process *process,
		io_src_sep_cb_t *cb, int sep1, int sep2);

/**
 * Defines source which will be notified each time the process writes something
 * to it's standard output
 * @param process Process to configure
 * @param cb Callback notified when there is something to read
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_set_stdout_src(struct io_process *process, io_src_cb_t *cb);

/**
 * Defines a separator source which will be notified each time the process
 * writes something to it's standard error
 * @param process Process to configure
 * @param cb Callback notified when a chunk of data is available
 * @param sep1 First separator
 * @param sep2 Second separator, set to IO_SRC_SEP_NO_SEP2 for none
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_set_stderr_sep_src(struct io_process *process,
		io_src_sep_cb_t *cb, int sep1, int sep2);

/**
 * Defines source which will be notified each time the process writes something
 * to it's standard error
 * @param process Process to configure
 * @param cb Callback notified when there is something to read
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_set_stderr_src(struct io_process *process, io_src_cb_t *cb);

/**
 * Defines a timeout after which the process will receive a signal, if not
 * already terminated.
 * @note can be called repeatedly after the process has been launch, to rearm
 * the timer, in a watchdog fashion
 * @param process Process to configure
 * @param timeout Time in ms
 * @param signum Signal to send when the timeout expires
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_set_timeout(struct io_process *process, int timeout, int signum);

/* sets errno on error */
/**
 * Retrieves the libioutils source for the process, to register in a monitor.
 * This is a convenient function to avoid redefining one from
 * io_process_get_fd() and io_process_process_events() manually
 * @param process Process context
 * @return errno-compatible negative value on error, 0 on success
 */
struct io_src *io_process_get_src(struct io_process *process);

/**
 * Retrieves the underlying file descriptor for inclusion in a select/poll/epoll
 * event loop
 * @param process Process context
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_get_fd(struct io_process *process);

/**
 * Process the events, which have been signaled on the file descriptor
 * @param process Process context
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_process_events(struct io_process *process);

/**
 * Starts the process, once it has been initialized and configured
 * @param process Process context
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_launch(struct io_process *process);

/**
 * Waits for the process, once it has been launched. The termination callback
 * passed at init will be called
 * @note this function will block until the process is dead, be sure that it
 * will die by signalling it before, or setting a timer
 * @param process Process context
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_wait(struct io_process *process);

/**
 * Launches the process and waits for it's termination after it has been
 * initialized and configured
 * @note this function will block until the process is dead, be sure that it
 * will die by signalling it before, or setting a timer
 * @param process Process context
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_launch_and_wait(struct io_process *process);

/**
 * Configures a process with a parameters set. This is a convenience function to
 * replace all the io_process_set_XXX function calls by only one function call
 * @param process Process context
 * @param parameters Set of parameters to use to configure the process.
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_prepare(struct io_process *process,
		struct io_process_parameters *parameters);

/* in any of the following functions, the "parameters" parameter can be NULL */
/**
 * Convenience function which calls io_process_init() and io_process_prepare()
 * @param process Process context
 * @param parameters Set of parameters to use to configure the process.
 * @param termination_cb Function called when the process terminates
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_init_prepare(struct io_process *process,
		struct io_process_parameters *parameters,
		io_pid_cb termination_cb, ...);

/**
 * Convenience function which calls io_process_vinit() and io_process_prepare()
 * @param process Process context
 * @param parameters Set of parameters to use to configure the process.
 * @param termination_cb Function called when the process terminates
 * @return errno-compatible negative value on error, 0 on success
 * @return
 */
int io_process_vinit_prepare(struct io_process *process,
		struct io_process_parameters *parameters,
		io_pid_cb termination_cb, va_list args);

/**
 * Convenience function which calls io_process_init(), io_process_prepare() and
 * io_process_launch()
 * @param process Process context
 * @param parameters Set of parameters to use to configure the process.
 * @param termination_cb Function called when the process terminates
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_init_prepare_and_launch(struct io_process *process,
		struct io_process_parameters *parameters,
		io_pid_cb termination_cb, ...)
__attribute__ ((sentinel(0)));

/**
 * Convenience function which calls io_process_vinit(), io_process_prepare() and
 * io_process_launch()
 * @param process Process context
 * @param parameters Set of parameters to use to configure the process.
 * @param termination_cb Function called when the process terminates
 * @param args
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_vinit_prepare_and_launch(struct io_process *process,
		struct io_process_parameters *parameters,
		io_pid_cb termination_cb, va_list args);

/**
 * Convenience function which calls io_process_init(), io_process_prepare(),
 * io_process_launch() and io_process_wait()
 * @param process Process context
 * @param parameters Set of parameters to use to configure the process.
 * @param termination_cb Function called when the process terminates
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_init_prepare_launch_and_wait(struct io_process *process,
		struct io_process_parameters *parameters,
		io_pid_cb termination_cb, ...)
__attribute__ ((sentinel(0)));

/**
 * Convenience function which calls io_process_init(), io_process_prepare(),
 * io_process_launch() and io_process_wait()
 * @param process Process context
 * @param parameters Set of parameters to use to configure the process.
 * @param termination_cb Function called when the process terminates
 * @param args
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_vinit_prepare_launch_and_wait(struct io_process *process,
		struct io_process_parameters *parameters,
		io_pid_cb termination_cb, va_list args);

/* waits for the process thus can block */
/**
 * Sends a sigkill to the process and waits (blocking) for it's termination.
 * @param process Process to kill
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_kill(struct io_process *process);

/**
 * Sends a signal to the process without waiting for it's termination
 * @param process Process to signal
 * @return errno-compatible negative value on error, 0 on success
 */
int io_process_signal(struct io_process *process, int signum);

#endif /* IO_PROCESS_H_ */
