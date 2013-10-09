/**
 * @file pidwatch.c
 * @author carrier.nicolas0@gmail.com
 * @brief Watch for the termination of a process via a file descriptor. Uses
 * netlink connector internally, instead of SIGCHLD.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/socket.h>

#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#include <linux/filter.h>

#include <unistd.h>
#include <fcntl.h>

#include <arpa/inet.h>

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <pidwatch.h>

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

#define MSG_BUF_SIZE 1024

static int subscription_message(int pidfd)
{
	char nlmsghdrbuf[NLMSG_LENGTH(0)];
	struct nlmsghdr *nlmsghdr = (struct nlmsghdr *)nlmsghdrbuf;
	enum proc_cn_mcast_op op = PROC_CN_MCAST_LISTEN;
	struct cn_msg cn_msg = {
		.id = {
			.idx = CN_IDX_PROC,
			.val = CN_VAL_PROC,
		},
		.seq = 0,
		.ack = 0,
		.len = sizeof(op),
	};
	struct iovec iov[3] = {
		[0] = {
			.iov_base = nlmsghdrbuf,
			.iov_len = NLMSG_LENGTH(0),
		},
		[1] = {
			.iov_base = &cn_msg,
			.iov_len = sizeof(cn_msg),
		},
		[2] = {
			.iov_base = &op,
			.iov_len = sizeof(op),
		}
	};

	nlmsghdr->nlmsg_len = NLMSG_LENGTH(sizeof(cn_msg) + sizeof(op));
	nlmsghdr->nlmsg_type = NLMSG_DONE;
	nlmsghdr->nlmsg_flags = 0;
	nlmsghdr->nlmsg_seq = 0;
	nlmsghdr->nlmsg_pid = 0;

	return TEMP_FAILURE_RETRY(writev(pidfd, iov, 3));
}

struct __attribute__ ((__packed__)) cn_proc_msg {
	struct cn_msg msg;
	struct proc_event evt;
};

/**
 * Installs a packet filter to the netlink socket, so that our client process is
 * woken up only for messages it is interested on
 * @param pidfd Netlink socket for parocess connector messages
 * @return -1 on error with errno set suitably, 0 on success
 */
static int install_filter(int pidfd, pid_t pid)
{
	struct sock_filter filter[] = {
		/* check only one message is contained */
		BPF_STMT(BPF_LD | BPF_H | BPF_ABS,
				offsetof(struct nlmsghdr, nlmsg_type)),
		BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, htons(NLMSG_DONE), 1, 0),
		BPF_STMT(BPF_RET|BPF_K, 0x0), /* message is dropped */

		/* check message isn't an error */
		BPF_STMT(BPF_LD | BPF_H | BPF_ABS,
				offsetof(struct nlmsghdr, nlmsg_type)),
		BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, htons(NLMSG_ERROR), 0, 1),
		BPF_STMT(BPF_RET|BPF_K, 0x0), /* message is dropped */

		/* check message isn't an no-op */
		BPF_STMT(BPF_LD | BPF_H | BPF_ABS,
				offsetof(struct nlmsghdr, nlmsg_type)),
		BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, htons(NLMSG_NOOP), 0, 1),
		BPF_STMT(BPF_RET|BPF_K, 0x0), /* message is dropped */

		/* check message comes from the kernel */
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
				offsetof(struct nlmsghdr, nlmsg_pid)),
		BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, 0, 1, 0),
		BPF_STMT(BPF_RET|BPF_K, 0x0), /* message is dropped */

		/* check it's a proc connector event part 1 */
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
				offsetof(struct cn_msg, id) +
				offsetof(struct cb_id, idx)),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htonl(CN_IDX_PROC), 1, 0),
		BPF_STMT(BPF_RET|BPF_K, 0x0), /* message is dropped */

		/* check it's a proc connector event part 2 */
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
				offsetof(struct cn_msg, id) +
				offsetof(struct cb_id, val)),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htonl(CN_VAL_PROC), 1, 0),
		BPF_STMT(BPF_RET|BPF_K, 0x0), /* message is dropped */

		/* check it's an exit message*/
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
				offsetof(struct cn_proc_msg, evt.what)),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htonl(PROC_EVENT_EXIT), 1,
				0),
		BPF_STMT(BPF_RET|BPF_K, 0x0), /* message is dropped */

		/* check the pid matches */
		BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
				offsetof(struct cn_proc_msg,
					evt.event_data.exit.process_pid)),
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htonl(pid), 0, 1),

		/* message is sent to user space */
		BPF_STMT(BPF_RET|BPF_K, 0xffffffff),

		BPF_STMT(BPF_RET|BPF_K, 0x0), /* message is dropped */
	};
	struct sock_fprog fprog;

	memset(&fprog, 0, sizeof(fprog));
	fprog.filter = filter;
	fprog.len = sizeof(filter) / sizeof(*filter);

	return setsockopt(pidfd, SOL_SOCKET, SO_ATTACH_FILTER, &fprog,
			sizeof(fprog));
}

/* reads the state of a process, knowing it's pid */
static int read_process_state(pid_t pid)
{
#define BUF_MAX 128
	char status_path[BUF_MAX];
	FILE *stat_file = NULL;
	char state;
	int ret;

	ret = snprintf(status_path, BUF_MAX, "/proc/%lld/stat", (long long)pid);
	if (0 > ret || ret >= BUF_MAX)
		return -1;

	stat_file = fopen(status_path, "rb");
	if (NULL == stat_file)
		return -1;

	/* first two pieces of information are dropped */
	ret = fscanf(stat_file, "%*d %*s %c", &state);
	if (EOF == ret) {
		fclose(stat_file);
		return -1;
	}
	fclose(stat_file);

	return state;
#undef BUF_MAX
}

int pidwatch_create(int flags)
{
	int pidfd;
	int ret;
	struct sockaddr_nl addr = {
		.nl_family = AF_NETLINK,
		/*
		 * from man netlink (7), nl_pid must be unique among netlink
		 * sockets, a value of 0 makes the kernel take care of
		 * attributing a unique number to the socket : mandatory for a
		 * library when we don't know if another netlink socket will or
		 * has been opened somewhere else in the program.
		 */
		.nl_pid = 0,
		.nl_groups = CN_IDX_PROC,
	};
	int saved_errno;

	if (((SOCK_NONBLOCK | SOCK_CLOEXEC) & flags) != flags) {
		errno = EINVAL;
		return -1;
	}
	pidfd = socket(PF_NETLINK, SOCK_DGRAM | flags, NETLINK_CONNECTOR);
	if (-1 == pidfd)
		return -1;

	ret = bind(pidfd, (struct sockaddr *)&addr, sizeof(addr));
	if (-1 == ret)
		goto err;

	/* must filter before subscription (start of message stream) */
	/*
	 * monitor for init's death : quite unlikely...
	 * by doing this, we avoid receiving messages until we knwo which
	 * process we want to monitor
	 */
	ret = install_filter(pidfd, 1);
	if (-1 == ret)
		goto err;

	ret = subscription_message(pidfd);
	if (-1 == ret)
		goto err;

	return pidfd;
err:
	/* is saving errno really needed ? i.e. does close modify errno ? */
	saved_errno = errno;
	close(pidfd);
	errno = saved_errno;

	return -1;
}

int pidwatch_set_pid(int pidfd, pid_t pid)
{
	int state;
	int ret;

	if (0 > pidfd || 1 >= pid) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * there is a risk of tocttou here :
	 *
	 */

	ret = install_filter(pidfd, pid);
	if (-1 == ret)
		return -1;

	/* once subscribed, check the process still exists */
	state = read_process_state(pid);
	if (-1 == state || 'Z' == state) {
		/*
		 * on error, assume process disappeared, the same goes for
		 * zombie process, which won't generate any EXIT event
		 */
		errno = ESRCH;
		goto err;
	}

	return 0;
err:
	/*
	 * be sure we continue to filter and we don't get messages concerning a
	 * process we're not interested in and which could have obtained the
	 * pid we wanted to watch, in the interval.
	 */
	(void)install_filter(pidfd, 1);

	return -1;
}

int pidwatch_wait(int pidfd, int *status)
{
	char buf[MSG_BUF_SIZE];
	struct iovec iov[1] = {
		[0] = {
			.iov_base = buf,
			.iov_len = sizeof(buf),
		},
	};
	struct sockaddr_nl addr;
	struct msghdr msghdr = {
		.msg_name = &addr,
		.msg_namelen = sizeof(addr),
		.msg_iov = iov,
		.msg_iovlen = 1,
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_flags = 0,
	};
	ssize_t len;
	struct nlmsghdr *nlmsghdr = (struct nlmsghdr *)buf;
	struct cn_msg *cn_msg;
	struct proc_event *ev;

	len = recvmsg(pidfd, &msghdr, 0);
	if (-1 == len)
		/*
		 * return -1 is valid : no pid can take this value and pid_t can
		 * contain it, otherwise, kill() wouldn't work with negative pid
		 * values.
		 */
		return -1;

	cn_msg = NLMSG_DATA(nlmsghdr);
	ev = (struct proc_event *)cn_msg->data;

	/* exit_code has the same semantic as status from wait(2) */
	if (NULL != status)
		*status = ev->event_data.exit.exit_code;

	return ev->event_data.exit.process_pid;
}

