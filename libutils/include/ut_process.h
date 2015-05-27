/**
 * @file ut_process.h
 * @brief utilities for executing processes
 *
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef UT_PROCESS_H_
#define UT_PROCESS_H_
#include <stdbool.h>

/**
 * Variadic version of the system() function
 * @see man system
 * @param command
 * @return -1 in case of allocation error or return code of the system function
 * (in normal case, returns a wait() status code)
 */
int ut_process_vsystem(const char *command, ...)
__attribute__ ((format (printf, 1, 2)));

/**
 * Variadic version of the popen() function
 * @see man popen
 * @param command a-la-printf format for building the command-line.
 * @param type
 * @return NULL on error with errno set, otherwise FILE * object as popen would
 * return. On error, errno is set to ENOMEM if the command allocation failed, or
 * according to popen in other error cases.
 */
__attribute__ ((format (printf, 1, 3)))
FILE *ut_process_vpopen(const char *command, const char *type, ...);

/**
 * Change the name of the current process. Useful for differentiating processes
 * between a fork and an exec.
 * @param fmt printf format string
 * @return errno-compatible negative value on error, 0 on success
 */
__attribute__ ((format (printf, 1, 2)))
int ut_process_change_name(const char *fmt, ...);

/**
 * Gets the name of the current process
 * @param name char buffer of at least 17 bytes
 * @return name on success, NULL otherwise, with errno set as specified
 * in prctl(PR_GET_NAME...)
 */
char *ut_process_get_name(char name[17]);

/**
 * @struct ut_process_sync
 * @brief parent/child synchronization mechanism
 *
 * Allows a parent to wait for it's child to complete an action, or vice-versa
 */
struct ut_process_sync {
	int pico[2]; /**< parent in child out */
	int poci[2]; /**< parent out child in */
};

/**
 * Initializes a parent/child sync structure
 * @param sync Synchronisation structure to initialize
 * @param cloexec true iif the O_CLOEXEC must be passed to the underlying file
 * descriptors
 * @return errno-compatible negative value on error, 0 on success, for possible
 * values, see pipe2(2)
 */
int ut_process_sync_init(struct ut_process_sync *sync, bool cloexec);

/**
 * Make the children wait for it's parent to unlock it with a call to
 * ut_process_sync_parent_unlock()
 * @param sync synchronization structure
 * @return errno-compatible negative value on error, 0 on success, for possible
 * values, see read(2)
 */
int ut_process_sync_child_lock(struct ut_process_sync *sync);

/**
 * Unlock the parent, currently blocked in a ut_process_sync_parent_lock() call
 * @param sync synchronization structure
 * @return errno-compatible negative value on error, 0 on success, for possible
 * values, see write(2)
 */
int ut_process_sync_child_unlock(struct ut_process_sync *sync);

/**
 * Make the parent wait for it's children to unlock it with a call to
 * ut_process_sync_child_unlock()
 * @param sync synchronization structure
 * @return errno-compatible negative value on error, 0 on success, for possible
 * values, see read(2)
 */
int ut_process_sync_parent_lock(struct ut_process_sync *sync);

/**
 * Unlock the children, currently blocked in a ut_process_sync_child_lock() call
 * @param sync synchronization structure
 * @return errno-compatible negative value on error, 0 on success, for possible
 * values, see write(2)
 */
int ut_process_sync_parent_unlock(struct ut_process_sync *sync);

/**
 * Release the resources held by a parent/child synchronization structure
 * @param sync synchronization structure
 */
void ut_process_sync_clean(struct ut_process_sync *sync);

#endif /* UT_PROCESS_H_ */
