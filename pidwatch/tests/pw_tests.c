#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/wait.h>
#ifdef PIDWATCH_HAS_CAPABILITY_SUPPORT
#include <sys/capability.h>
#endif /* PIDWATCH_HAS_CAPABILITY_SUPPORT */

#include <unistd.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include <CUnit/Basic.h>

#include <pidwatch.h>

#include <fautes.h>

#ifndef O_NONBLOCK
#ifdef __arm__
	/* value taken from linux kernel header
	 * include/asm-generic/fcntl.h */
	#define O_NONBLOCK 02000000
#else
	#error O_NONBLOCK not defined !
#endif
#endif

#ifndef O_CLOEXEC
#ifdef __arm__
	/* value taken from linux kernel header
	 * include/asm-generic/fcntl.h */
	#define O_CLOEXEC 02000000
#else
	#error O_CLOEXEC not defined !
#endif
#endif

#ifndef SOCK_CLOEXEC
/**
 * @def SOCK_CLOEXEC
 * @brief Set the flag O_CLOEXEC at socket's creation
 */
#define SOCK_CLOEXEC O_CLOEXEC
#endif

#ifndef SOCK_NONBLOCK
/**
 * @def SOCK_NONBLOCK
 * @brief Set the flag O_NONBLOCK at socket's creation
 */
#define SOCK_NONBLOCK O_NONBLOCK
#endif

pid_t g_pid_max;

static void read_pid_max(void)
{
	int ret;
	FILE *pmf = NULL;
	long long ll_pm;

	pmf = fopen("/proc/sys/kernel/pid_max", "rb");
	if (NULL == pmf) {
		fprintf(stderr, "Can't read /proc/sys/kernel/pid_max\n");
		goto out;
	}

	ret = fscanf(pmf, "%lld", &ll_pm);
	if (1 != ret) {
		if (ferror(pmf))
			perror("fscanf");
		else
			fprintf(stderr, "unexpected EOF reading pid_max\n");
		goto out;
	}

	g_pid_max = (pid_t)ll_pm;

out:
	fclose(pmf);
}

static void dump_args(int argc, const char *argv[])
{
	do {
		fprintf(stderr, "%s ", *argv);
	} while (*(++argv));
}

static pid_t __attribute__((sentinel)) launch(const char *prog, ...)
{
	int ret;
	int child_argc = 0;
	const char *child_argv[10] = {NULL};
	char *arg = (char *)-1;
	va_list args;
	pid_t pid;

	child_argv[child_argc++] = prog;

	va_start(args, prog);
	for (; child_argc < 10 && NULL != arg; child_argc++) {
		arg = va_arg(args, char *);
		child_argv[child_argc] = arg;
	}
	va_end(args);

	if (NULL != arg) {
		errno = E2BIG;
		return -1;
	}
	child_argv[child_argc] = NULL; /* not necessary but clearer */

	pid = fork();
	if (-1 == pid)
		return -1;

	if (0 == pid) {
		/* in child */
		fprintf(stderr, "Executing ");
		dump_args(child_argc, child_argv);
		fprintf(stderr, " (%jd)\n", (intmax_t)getpid());
		ret = execvp(child_argv[0], child_argv);
		if (-1 == ret) {
			perror("execve");
			exit(1);
		}
	}

	/* in parent */
	return pid;
}

#define E(type, ret)  \
({ \
	type _return = ret; \
	if (-1 == _return) { \
		int _old_errno = errno; \
\
		fprintf(stderr, "%s():%d : ", __func__, __LINE__); \
		perror("" #ret); \
		errno = _old_errno; \
	} \
\
	_return;\
})\

static void testPIDWATCH_CREATE(void)
{
	int pidfd;
	int invalid_flag;

	/* normal cases */
	pidfd = E(int, pidwatch_create(SOCK_CLOEXEC));
	CU_ASSERT_NOT_EQUAL(pidfd, -1);
	/* cleanup */
	close(pidfd);

	/* error cases */
	invalid_flag = ~(SOCK_CLOEXEC | SOCK_NONBLOCK);
	pidfd = pidwatch_create(invalid_flag);
	CU_ASSERT_EQUAL(pidfd, -1);
}

static void testPIDWATCH_WAIT(void)
{
	pid_t pid;
	pid_t pid_ret;
	int pidfd;
	int status;
	int wstatus;
	int ret;

	/* initialization */
	pidfd = E(int, pidwatch_create(SOCK_CLOEXEC));
	CU_ASSERT_NOT_EQUAL(pidfd, -1);

	/* normal cases */
	/* normal termination */
	pid = E(pid_t, launch("sleep", "1", NULL));
	CU_ASSERT_NOT_EQUAL(pid, -1);
	ret = E(int, pidwatch_set_pid(pidfd, pid));
	CU_ASSERT_NOT_EQUAL(ret, -1);
	pid_ret = E(pid_t, pidwatch_wait(pidfd, &status));
	CU_ASSERT_NOT_EQUAL(pid_ret, -1);
	/* cleanup */
	waitpid(pid, &wstatus, 0);
	CU_ASSERT_EQUAL(status, wstatus);

	/* terminated by signal */
	pid = E(pid_t, launch("sleep", "1", NULL));
	CU_ASSERT_NOT_EQUAL(pid, -1);
	ret = E(int, pidwatch_set_pid(pidfd, pid));
	CU_ASSERT_NOT_EQUAL(ret, -1);
	ret = E(int, kill(pid, 9));
	CU_ASSERT_NOT_EQUAL(ret, -1);
	pid_ret = E(pid_t, pidwatch_wait(pidfd, &status));
	CU_ASSERT_NOT_EQUAL(pid_ret, -1);
	/* cleanup */
	waitpid(pid, &wstatus, 0);
	CU_ASSERT_EQUAL(status, wstatus);

	/* error cases */
	pid_ret = pidwatch_wait(-1, &status);
	CU_ASSERT_EQUAL(pid_ret, -1);

	close(pidfd);
}

static const char *get_process_name(pid_t pid)
{
	FILE *fp;
	static char buf[80];
	char *nl, *ret = "";

	snprintf(buf, sizeof(buf), "/proc/%d/comm", pid);
	fp = fopen(buf, "r");
	if (fp) {
		ret = fgets(buf, sizeof(buf), fp);
		/* clobber newline character */
		nl = ret ? strchr(ret, '\n') : NULL;
		if (nl)
			*nl = '\0';
		fclose(fp);
	}
	return ret;
}

static bool testPIDWATCH_WAIT_145990_loop(void)
{
	pid_t pid;
	pid_t pid_ret;
	int pidfd;
	int status;
	int wstatus;
	int ret;
	int reaped = false;
	bool quit = false;

	/* initialization */
	pidfd = E(int, pidwatch_create(SOCK_CLOEXEC));
	CU_ASSERT_NOT_EQUAL(pidfd, -1);

	/* normal cases */
	/* normal termination */
	pid = E(pid_t, launch("usleep", "10000", NULL));
	CU_ASSERT_NOT_EQUAL(pid, -1);
	ret = E(int, pidwatch_set_pid(pidfd, pid));
	CU_ASSERT_NOT_EQUAL(ret, -1);
	while (!reaped) {
		pid_ret = E(pid_t, pidwatch_wait(pidfd, &status));
		CU_ASSERT_NOT_EQUAL(pid_ret, -1);
		CU_ASSERT_EQUAL(pid_ret, pid);
		if (pid_ret != pid) {
			fprintf(stderr, "pid_ret = %jd, pid = %jd\n",
					(intmax_t)pid_ret, (intmax_t)(pid));
			fprintf(stderr, "process 1: %s\n", get_process_name(pid_ret));
			fprintf(stderr, "process 2: %s\n", get_process_name(pid));
			quit = true;
		} else {
			/* cleanup */
			waitpid(pid, &wstatus, 0);
			CU_ASSERT_EQUAL(status, wstatus);
			reaped = true;
		}
	}

	close(pidfd);

	return quit;
}

static bool loop = true;

static void sighandler(int tata)
{
	loop = false;
}

static pid_t start_fork_loop(void)
{
	pid_t pid;

	signal(SIGUSR1, sighandler);
	pid = fork();
	if (0 == pid) {/* in child */
		while (loop) {
			pid = fork();
			if (pid == 0) /* in grand child */
				exit(0);
			waitpid(pid, NULL, 0);
		}
		exit(0);
	}

	/* in parent */
	return pid;
}

static void stop_fork_loop(pid_t pid)
{
	kill(pid, SIGUSR1);
	waitpid(pid, NULL, 0);
}

static void testPIDWATCH_WAIT_145990(void)
{
	int count = 1000;
	pid_t pid;

	pid = start_fork_loop();
	fprintf(stderr, "fork loop has pid (%jd)\n", (intmax_t)pid);

	while (count--) {
		fprintf(stderr, "************** turn %d\n", 1000 - count);
		if (testPIDWATCH_WAIT_145990_loop())
			break;
	}

	stop_fork_loop(pid);
}

static void testPIDWATCH_SET_PID(void)
{
	pid_t pid;
	pid_t pid_ret;
	int pidfd;
	int status;
	int ret;

	/* initialization */
	pidfd = E(int, pidwatch_create(SOCK_CLOEXEC));
	CU_ASSERT_NOT_EQUAL(pidfd, -1);

	/* normal cases */
	pid = E(pid_t, launch("sleep", "1", NULL));
	CU_ASSERT_NOT_EQUAL(pid, -1);
	ret = E(int, pidwatch_set_pid(pidfd, pid));
	CU_ASSERT_NOT_EQUAL(ret, -1);
	pid_ret = E(pid_t, pidwatch_wait(pidfd, &status));
	CU_ASSERT_NOT_EQUAL(pid_ret, -1);
	waitpid(pid, &status, 0);

	/* pidwatch can be reused to monitor an new process */
	pid = E(pid_t, launch("sleep", "1", NULL));
	CU_ASSERT_NOT_EQUAL(pid, -1);
	ret = E(int, pidwatch_set_pid(pidfd, pid));
	CU_ASSERT_NOT_EQUAL(ret, -1);
	pid_ret = E(pid_t, pidwatch_wait(pidfd, &status));
	CU_ASSERT_NOT_EQUAL(pid_ret, -1);
	waitpid(pid, &status, 0);

	/* error cases */
	pid = E(pid_t, launch("ls", "supercalifragilistic", NULL));
	CU_ASSERT_NOT_EQUAL(pid, -1);
	sleep(1);
	/*
	 * if the child dies before we set up the watch, it is a zombie, thus
	 * considered dead, ESRCH is raised
	 */
	ret = pidwatch_set_pid(pidfd, pid);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT(ESRCH == errno);
	/* reap */
	waitpid(pid, &status, 0);

	/* invalid arguments */
	ret =  pidwatch_set_pid(-1, pid);
	CU_ASSERT_EQUAL(ret, -1);
	ret = pidwatch_set_pid(pidfd, g_pid_max);
	CU_ASSERT_EQUAL(ret, -1);

	/* cleanup */
	close(pidfd);
}

#ifdef PIDWATCH_HAS_CAPABILITY_SUPPORT
void free_cap(cap_t *cap)
{
	if (cap)
		cap_free(*cap);
}

/**
 * Checks if a capability is effective for the current process. If not, allows
 * to try activating it.
 * @param value Capability to check
 * @param try If non-zero and if the capability isn't set, try to raise it
 * @return -1 on error, with errno set, 0 if the capability was already
 * effective, 1 if not, but it was permetted, try was non-zero and it was raised
 */
int check_proc_cap(cap_value_t value, int try)
{
	int ret;
	cap_t __attribute__((cleanup(free_cap))) caps;
	cap_flag_value_t flag_value;

	caps = cap_get_proc();
	if (NULL == caps)
		return -1;

	ret = cap_get_flag(caps, value, CAP_EFFECTIVE, &flag_value);
	if (-1 == ret)
		return -1;
	if (CAP_SET != value) {
		ret = cap_set_flag(caps, CAP_EFFECTIVE, 1, &value, CAP_SET);
		if (-1 == ret)
			return -1;
		ret = cap_set_proc(caps);
		if (-1 == ret)
			return -1;
		return 1;
	}

	return 0;
}
#endif /* PIDWATCH_HAS_CAPABILITY_SUPPORT */

static const struct test_t tests[] = {
		{
				.fn = testPIDWATCH_CREATE,
				.name = "pidwatch_create"
		},
		{
				.fn = testPIDWATCH_WAIT,
				.name = "pidwatch_wait"
		},
		{
				.fn = testPIDWATCH_WAIT_145990,
				.name = "pidwatch_wait_145990"
		},
		{
				.fn = testPIDWATCH_SET_PID,
				.name = "pidwatch_set_pid"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_pw_suite(void)
{
#ifdef PIDWATCH_HAS_CAPABILITY_SUPPORT
	int ret;

	ret = check_proc_cap(CAP_NET_ADMIN, 1);
	if (-1 == ret) {
		fprintf(stderr, "CAP_NET_ADMIN is needed for pidwatch\n");
		return 1;
	}
#endif /* PIDWATCH_HAS_CAPABILITY_SUPPORT */

	read_pid_max();

	return 0;
}

static int clean_pw_suite(void)
{
	return 0;
}

struct suite_t pidwatch_suite = {
		.name = "pidwatch",
		.init = init_pw_suite,
		.clean = clean_pw_suite,
		.tests = tests,
};
