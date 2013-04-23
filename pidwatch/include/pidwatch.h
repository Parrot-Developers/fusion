#ifndef PIDWATCH_H_
#define PIDWATCH_H_

/* for SOCK_CLOEXEC and SOCK_NONBLOCK */
#include <sys/socket.h>
/* for pid_t */
#include <signal.h>

/**
 * Creates a watch on a given pid.
 * @param pid Pid of the process one want to watch for termination.
 * @param flags Bitwise combination of SOCK_CLOEXEC and SOCK_NONBLOCK
 * @return Selectable file descriptor, notified when the process dies. Must be
 * closed by close() when not used anymore, i.e. when not needed or when
 * pidwatch_wait() has returned 1.
 * @note NONBLOCK concerns only pidwatch_wait calls. pidwatch_create() itself
 * could block, even if NONBLOCK is set.
 */
int pidwatch_create(pid_t pid, int flags);

/**
 * Reads the termination event of a process watched.
 * @param pidfd Watch on a process previously created by pidwatch_create
 * @param status If not NULL, pidwatch_wait() stores status information in the
 * int to which it points.
 * @return -1 on error, with errno set suitably, pid of the process if it has
 * terminated, 0 if not (unexpected message, not filtered at socket level)
 */
int pidwatch_wait(int pidfd, int *status);

#endif /* PIDWATCH_H_ */
