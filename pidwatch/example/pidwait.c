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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#include <pidwatch.h>

/* for socket */
#ifndef SOCK_CLOEXEC
/**
 * @def SOCK_CLOEXEC
 * @brief Set the flag O_CLOEXEC at socket's creation
 */
#define SOCK_CLOEXEC O_CLOEXEC
#endif

static void dump_args(int argc, char *argv[])
{
	do {
		printf("%s ", *argv);
	} while (*(++argv));
}

static pid_t launch(int argc, char *argv[])
{
	int ret;
	pid_t pid;

	pid = fork();
	if (-1 == pid)
		return -1;

	if (0 == pid) {
		/* in child */
		ret = execvp(argv[0], argv);
		if (-1 == ret) {
			perror("execvp");
			exit(1);
		}
	}

	/* in parent */
	return pid;
}

static void usage(int exit_code)
{
	FILE *out = exit_code ? stderr : stdout;
	fprintf(out, "usage : pidwait -p|--pid PID\n"
	       "        pidwait COMMAND_LINE\n"
	       "\tThe first form waits for the termination a process of a given"
	       "pid.\n"
	       "\tThe second form launches the command whose arguments are "
	       "passed, and wait for it's termination.\n"
	       "Both two forms return the waited process return value.\n");

	exit(exit_code);
}

static void close_p(int *fd)
{
	if (fd)
		if (-1 != *fd) {
			close(*fd);
			*fd = -1;
		}
}

static void process_args(int argc, char *argv[], int *child, pid_t *pid)
{
	if (NULL == child || NULL == pid)
		error(EXIT_FAILURE, EINVAL, "Coding error : NULL argument\n");

	if (argc < 2)
		usage(EXIT_FAILURE);

	if ('-' == argv[1][0]) {
		if (0 == strcmp("-h", argv[1]))
			usage(EXIT_SUCCESS);
		if (0 == strcmp("-p", argv[1]) ||
				0 == strcmp("--pid", argv[1])) {
			if (3 != argc) {
				usage(EXIT_FAILURE);
			} else {
				*child = 0;
				*pid = atoi(argv[2]);
				printf("Attach to process %d\n", *pid);
			}
		} else {
			fprintf(stderr, "Unsupported option \"%s\"\n", argv[1]);
			usage(EXIT_FAILURE);
		}
	} else {
		*child = 1;
		*pid = launch(argc - 1, argv + 1);
		printf("Watch \"");
		dump_args(argc - 1, argv + 1);
		printf("\"with pid %d\n", *pid);
	}
}

static void debriefing(pid_t pid, int status)
{
	printf("Process of pid %d ", pid);
	if (WIFEXITED(status)) {
		printf("exited with return code %d\n", WEXITSTATUS(status));
	} else if (WIFSIGNALED(status)) {
		printf("was killed by signal %d", WTERMSIG(status));
#ifdef WCOREDUMP
		if (WCOREDUMP(status))
			printf(" and produced a coredump");
#endif
		printf("\n");
	} else {
		/* normally, can't be reached */
		fprintf(stderr, "Unhandheld exit status case\n");
	}
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

int main(int argc, char *argv[])
{
	int __attribute__((cleanup(close_p))) pidfd;
	int status;
	int child = 0;
	int ret = 0;
#ifdef PIDWATCH_HAS_CAPABILITY_SUPPORT
	int ret;
#endif /* PIDWATCH_HAS_CAPABILITY_SUPPORT */
	pid_t pid_ret;
	pid_t pid = 0;

	process_args(argc, argv, &child, &pid);

#ifdef PIDWATCH_HAS_CAPABILITY_SUPPORT
	ret = check_proc_cap(CAP_NET_ADMIN, 1);
	if (-1 == ret) {
		fprintf(stderr, "CAP_NET_ADMIN is needed for pidwatch\n");
		fprintf(stderr, "try \"setcap cap_net_admin=+p pidwait\"\n");
		return EXIT_FAILURE;
	}
#endif /* PIDWATCH_HAS_CAPABILITY_SUPPORT */

	pidfd = pidwatch_create(SOCK_CLOEXEC);
	if (-1 == pidfd) {
		perror("pidwatch_create");
		return EXIT_FAILURE;
	}
	ret = pidwatch_set_pid(pidfd, pid);
	if (-1 == ret) {
		perror("pidwatch_set_pid");
		return EXIT_FAILURE;
	}
	/* here, block directly. One can rather use a select-like event loop */
	pid_ret = pidwatch_wait(pidfd, &status);
	assert(pid == pid_ret);
	if (-1 == pid_ret) {
		perror("pidwatch_wait");
		return EXIT_FAILURE;
	}

	debriefing(pid, status);

	/* if we are the parent of the child, we should reap it */
	if (child)
		waitpid(pid, &status, WNOHANG);

	return EXIT_SUCCESS;
}
