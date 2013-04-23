#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <pidwatch.h>

int main(int argc, char *argv[])
{
	pid_t pid;
	pid_t pid_ret;
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
	int count = 0;

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
	printf("pidwatch_create for pid %lld\n", (long long int)pid);
	pidfd = pidwatch_create(pid, SOCK_CLOEXEC);
	if (-1 == pidfd) {
		perror("pidwatch");
		return EXIT_FAILURE;
	}

	do {
		pid_ret = pidwatch_wait(pidfd, &status);
		if (-1 == ret) {
			perror("pidwatch_wait");
			return EXIT_FAILURE;
		}
		count++;
	} while (0 == pid_ret || pid != pid_ret);

	printf("Process of pid %lld terminated\n", (long long int)pid);
	printf("%d message%s received\n", count, 1 < count ? "s" : "");

	return EXIT_SUCCESS;
}
