#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <pidwatch.h>

int main(int argc, char *argv[])
{
	pid_t pid;
	int pidfd;
	char *child_argv[] = {
		"/bin/sleep",
		"3",
		NULL,
	};
	char *child_envp[] = {
		NULL,
	};
	int ret;
	int status;

	pid = fork();
	if (-1 == pid) {
		perror("pid");
		return EXIT_FAILURE;
	}

	if (0 == pid) {
		/* in child */
		printf("Executing %s %s\n", child_argv[0], child_argv[1]);
		ret = execve(child_argv[0], child_argv, child_envp);
		if (-1 == ret) {
			perror("execve");
			return EXIT_FAILURE;
		}
	}

	/* in parent */
	pidfd = pidwatch_create(pid, SOCK_CLOEXEC);
	if (-1 == pidfd) {
		perror("pidwatch");
		return EXIT_FAILURE;
	}

	ret = pidwatch_wait(pidfd, &status);
	if (-1 == ret) {
		perror("pidwatch_wait");
		return EXIT_FAILURE;
	}

	printf("Process of pid %lld terminated\n", (long long int)pid);

	return EXIT_SUCCESS;
}
