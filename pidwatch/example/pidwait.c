#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>

#include <unistd.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>

#include <pidwatch.h>

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

//void testPIDWATCH_WAIT(void)
//{
//	pid_t pid;
//	pid_t pid_ret;
//	int pidfd;
//	int status;
//	int ret;
//
//	/* normal cases */
//	/* normal termination */
//	pid = E(pid_t, launch("sleep", "0.5", NULL));
//	assert(pid != -1);
//	pidfd = E(int, pidwatch_create(pid, SOCK_CLOEXEC));
//	assert(pidfd != -1);
//	pid_ret = E(pid_t, pidwatch_wait(pidfd, &status));
//	assert(pid_ret != -1);
//	/* cleanup */
//	waitpid(pid, &status, 0);
//	assert(WIFEXITED(status));
//	assert(0 == WEXITSTATUS(status));
//	close(pidfd);
//
//	/* terminated by signal */
//	pid = E(pid_t, launch("sleep", "1", NULL));
//	assert(pid != -1);
//	pidfd = E(int, pidwatch_create(pid, SOCK_CLOEXEC));
//	assert(pidfd != -1);
//	ret = E(int, kill(pid, 9));
//	assert(-1 != ret);
//	pid_ret = E(pid_t, pidwatch_wait(pidfd, &status));
//	assert(pid_ret != -1);
//	/* cleanup */
//	waitpid(pid, &status, 0);
//	assert(WIFSIGNALED(status));
//	assert(9 == WTERMSIG(status));
//	close(pidfd);
//
//
//	/* error cases */
//	pid_ret = pidwatch_wait(-1, &status);
//	assert(pid_ret == -1);
//	pid_ret = pidwatch_wait(1, NULL);
//	assert(pid_ret == -1);
//}

void usage(int exit_code)
{
	printf("usage : pidwait -p|--pid PID\n"
	       "        pidwait COMMAND_LINE\n"
	       "\tThe first form waits for the termination a process of a given"
	       "pid.\n"
	       "\tThe second form launches the command whose arguments are "
	       "passed, and wait for it's termination.\n"
	       "Both two forms return the waited process return value.\n");

	exit(exit_code);
}

int main(int argc, char *argv[])
{



	return EXIT_SUCCESS;
}
