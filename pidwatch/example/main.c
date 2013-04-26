#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <pidwatch.h>

pid_t g_pid_max;

void read_pid_max(void)
{
	int ret;
	FILE *pmf = NULL;
	long long ll_pm;

	pmf = fopen("/proc/sys/kernel/pid_max", "rb");
	if (NULL == pmf) {
		fprintf(stderr, "Can't read /proc/sys/kernel/pid_max\n");
		exit(1);
	}

	ret = fscanf(pmf, "%lld", &ll_pm);
	if (1 != ret) {
		if (ferror(pmf))
			perror("fscanf");
		else
			fprintf(stderr, "unexpected EOF reading pid_max\n");
		exit(1);
	}

	g_pid_max = (pid_t)ll_pm;
}

void dump_args(int argc, char *argv[])
{
	do {
		printf("%s ", *argv);
	} while (*(++argv));
}

pid_t __attribute__((sentinel)) launch(char *prog, ...)
{
	int ret;
	int child_argc = 0;
	char *child_argv[10] = {NULL};
	char *child_envp[] = {NULL};
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
		errno = ERANGE;
		return -1;
	}
	child_argv[child_argc] = NULL; /* not necessary but clearer */

	pid = fork();
	if (-1 == pid)
		return -1;

	if (0 == pid) {
		/* in child */
		printf("Executing ");
		dump_args(child_argc, child_argv);
		printf("\n");
		ret = execvpe(child_argv[0], child_argv, child_envp);
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

void testPIDWATCH_CREATE(void)
{
	pid_t pid;
	int pidfd;
	int status;
	int invalid_flag;

	/* normal cases */
	pid = E(pid_t, launch("sleep", "0.5", NULL));
	assert(pid != -1);
	pidfd = E(int, pidwatch_create(pid, SOCK_CLOEXEC));
	assert(pidfd != -1);
	/* cleanup */
	waitpid(pid, &status, 0);
	close(pidfd);

	/* error cases */
	pid = E(pid_t, launch("echo", "titi tata tutu", NULL));
	assert(pid != -1);
	sleep(1);
	/*
	 * if the child dies before we set up the watch, it is a zombie, thus
	 * considered dead, ESRCH is raised
	 */
	pidfd = pidwatch_create(pid, SOCK_CLOEXEC);
	assert(ESRCH == errno);
	assert(pidfd == -1);
	/* cleanup */
	waitpid(pid, &status, 0);

	/* invalid arguments */
	pidfd = pidwatch_create(-63, SOCK_CLOEXEC);
	assert(pidfd == -1);
	invalid_flag = ~(SOCK_CLOEXEC | SOCK_NONBLOCK);
	pidfd = pidwatch_create(g_pid_max, SOCK_CLOEXEC);
	assert(pidfd == -1);
	pidfd = pidwatch_create(1, invalid_flag); /* pid 1 is always valid */
	assert(pidfd == -1);
}

void testPIDWATCH_WAIT(void)
{
	pid_t pid;
	pid_t pid_ret;
	int pidfd;
	int status;
	int ret;

	/* normal cases */
	/* normal termination */
	pid = E(pid_t, launch("sleep", "0.5", NULL));
	assert(pid != -1);
	pidfd = E(int, pidwatch_create(pid, SOCK_CLOEXEC));
	assert(pidfd != -1);
	pid_ret = E(pid_t, pidwatch_wait(pidfd, &status));
	assert(pid_ret != -1);
	/* cleanup */
	waitpid(pid, &status, 0);
	assert(WIFEXITED(status));
	assert(0 == WEXITSTATUS(status));
	close(pidfd);

	/* terminated by signal */
	pid = E(pid_t, launch("sleep", "1", NULL));
	assert(pid != -1);
	pidfd = E(int, pidwatch_create(pid, SOCK_CLOEXEC));
	assert(pidfd != -1);
	ret = E(int, kill(pid, 9));
	assert(-1 != ret);
	pid_ret = E(pid_t, pidwatch_wait(pidfd, &status));
	assert(pid_ret != -1);
	/* cleanup */
	waitpid(pid, &status, 0);
	assert(WIFSIGNALED(status));
	assert(9 == WTERMSIG(status));
	close(pidfd);


	/* error cases */
	pid_ret = pidwatch_wait(-1, &status);
	assert(pid_ret == -1);
	pid_ret = pidwatch_wait(1, NULL);
	assert(pid_ret == -1);
}

int main(int argc, char *argv[])
{
	printf("*** Automated unit tests for pidwatch ***\n");
	testPIDWATCH_CREATE();
	testPIDWATCH_WAIT();

	printf("*** No error found ***\n");

	return EXIT_SUCCESS;
}
