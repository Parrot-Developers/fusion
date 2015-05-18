/**
 * @file ut_log.h
 * @brief logging facilities for libutils
 *
 * @date 2 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef UT_LOG_H_
#define UT_LOG_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * @enum ut_log_level
 * @brief log levels supported
 */
enum ut_log_level {
	UT_CRIT		= 2,	/**< critical conditions */
	UT_ERR		= 3,	/**< error conditions */
	UT_WARN		= 4,	/**< warning conditions */
	UT_NOTICE	= 5,	/**< normal but significant condition */
	UT_INFO		= 6,	/**< informational message */
	UT_DEBUG	= 7,	/**< debug-level message */

};

/**
 * @typedef ut_log_f_t
 * @brief type of the callbacks accepted by the logging system
 */
typedef int (ut_log_f_t)(const char *format, ...);

/**
 * @var ut_log_f
 * @brief callback used for logging messages, can be set, to change the logging
 * facility
 */
extern ut_log_f_t *ut_log_f;

/**
 * @var ut_log_level_str
 * @brief prefixes prepended to each message, used to indicate the log level.
 * Not to be used directly
 */
extern const char *ut_log_level_str[];

/**
 * @def ut_log_wrap
 * @brief wrapper around the log callback, preserving errno. Not to be used
 * directly
 */
#define ut_log_wrap(fmt, args...) do { \
	int __errno_backup__ = errno; \
	\
	ut_log_f((fmt), ##args); \
	errno = __errno_backup__; \
} while (0)

/**
 * @def ut_log
 * @brief base logging macro, for defining the level specific ones. Not to be
 * used directly
 */
#define ut_log(lvl, fmt, ...) ut_log_wrap("[%s] %s: " fmt "\n",\
		ut_log_level_str[lvl], __func__, ##__VA_ARGS__);

/**
 * @def ut_crit
 * @brief logs a message at the error level
 */
#define ut_crit(fmt, ...) ut_log(UT_CRIT, fmt, ##__VA_ARGS__)

/**
 * @def ut_err
 * @brief logs a message at the error level
 */
#define ut_err(fmt, ...) ut_log(UT_ERR, fmt, ##__VA_ARGS__)

/**
 * @def ut_perr
 * @brief logs an error concerning a function, at the error level, interpreting
 * the error code with strerror
 */
#define ut_perr(func, err) ut_err("%s: %s", (func), strerror(abs((err))))

/**
 * @def ut_inf
 * @brief logs a message at the info level
 */
#define ut_info(fmt, ...) ut_log(UT_INFO, fmt, ##__VA_ARGS__)

#endif /* UT_LOG_H_ */
