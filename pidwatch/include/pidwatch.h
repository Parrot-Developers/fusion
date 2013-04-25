/**
 * @file pidwatch.h
 * @author carrier.nicolas0@gmail.com
 * @brief Watch for the termination of a process via a file descriptor. Uses
 * netlink connector internally, instead of SIGCHLD.
 */
#ifndef PIDWATCH_H_
#define PIDWATCH_H_

/* for SOCK_CLOEXEC and SOCK_NONBLOCK */
#include <sys/socket.h>
/* for pid_t */
#include <signal.h>

/**
 * Creates a watch on a given pid. This is a priviledged action, the
 * corresponding mandatory capability is cap_net_admin.
 * @param pid Pid of the process one want to watch for termination.
 * @param flags Bitwise combination of SOCK_CLOEXEC and SOCK_NONBLOCK
 * @return Selectable file descriptor, notified when the process dies. Must be
 * closed by close() when not used anymore, i.e. when not needed or when
 * pidwatch_wait() has returned 1. For possible errno values on error, see
 * socket(2), bind(2), setsockopt(2), writev(2) and kill(2). An ESRCH value
 * indicates that either the process doesn't exist (maybe) anymore, or that it
 * waits to be wait(2)-ed for.
 *
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
 * terminated. for possible errno values see recvmsg(2)
 */
int pidwatch_wait(int pidfd, int *status);

#endif /* PIDWATCH_H_ */
