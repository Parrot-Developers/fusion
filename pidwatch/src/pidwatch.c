#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
/* is it ok to include this ? */
#include <sys/user.h>

#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>

#include <unistd.h>

#include <errno.h>
#include <stdio.h> /* TODO remove */

#include <pidwatch.h>

static int subscription_message(int pidfd)
{
	char nlmsghdrbuf[NLMSG_LENGTH(0)];
	struct nlmsghdr *nlmsghdr = (struct nlmsghdr *)nlmsghdrbuf;
	enum proc_cn_mcast_op op;
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

	op = PROC_CN_MCAST_LISTEN;

	return TEMP_FAILURE_RETRY(writev(pidfd, iov, 3));
}

pid_t gpid;

int pidwatch_create(pid_t pid, int flags)
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

	printf("pidwatch_create for pid %lld\n", (long long int)pid);
	if (((SOCK_NONBLOCK | SOCK_CLOEXEC) & flags) != flags) {
		errno = EINVAL;
		return -1;
	}
	pidfd = socket (PF_NETLINK, SOCK_DGRAM | flags, NETLINK_CONNECTOR);
	if (-1 == pidfd)
		return -1;

	ret = bind(pidfd, (struct sockaddr *)&addr, sizeof(addr));
	if (-1 == ret)
		goto err;

	ret = subscription_message(pidfd);
	if (-1 == ret)
		goto err;

	gpid = pid;
	/* 
	 * once ready, verify with a kill(pid, 0) that the process is still
	 * alive, in case it died between the fork and the pidwatch installation
	 */
	return pidfd;
err:
	/* is saving errno really needed ? does close modify errno ? */
	saved_errno = errno;
	close(pidfd);
	errno = saved_errno;

	return -1;
}

int pidwatch_wait(int pidfd, int *status)
{
	printf("pidwatch_wait stub\n");

	struct msghdr msghdr;
	struct sockaddr_nl addr;
	struct iovec iov[1];
	char buf[PAGE_SIZE];
	ssize_t len;
	struct nlmsghdr *nlmsghdr;
	struct cn_msg *cn_msg;
	struct proc_event *ev;

	msghdr.msg_name = &addr;
	msghdr.msg_namelen = sizeof addr;
	msghdr.msg_iov = iov;
	msghdr.msg_iovlen = 1;
	msghdr.msg_control = NULL;
	msghdr.msg_controllen = 0;
	msghdr.msg_flags = 0;

	iov[0].iov_base = buf;
	iov[0].iov_len = sizeof buf;

	do {
		len = recvmsg(pidfd, &msghdr, 0);
		if (-1 == len)
			return -1;

		if (addr.nl_pid != 0)
			continue;

		for (nlmsghdr = (struct nlmsghdr *)buf;
				NLMSG_OK(nlmsghdr, len);
				nlmsghdr = NLMSG_NEXT(nlmsghdr, len)) {
			if ((nlmsghdr->nlmsg_type == NLMSG_ERROR)
					|| (nlmsghdr->nlmsg_type == NLMSG_NOOP))
				continue;

			cn_msg = NLMSG_DATA (nlmsghdr);
			if ((cn_msg->id.idx != CN_IDX_PROC)
					|| (cn_msg->id.val != CN_VAL_PROC))
				continue;
			ev = (struct proc_event *)cn_msg->data;

			switch (ev->what) {
				case PROC_EVENT_EXIT:
					printf ("EXIT %d/%d -> %d/%d\n",
							ev->event_data.exit.process_pid,
							ev->event_data.exit.process_tgid,
							ev->event_data.exit.exit_code,
							ev->event_data.exit.exit_signal);
					if (ev->event_data.exit.process_pid == gpid) {
						printf("FOUND ***\n");
						*status = ev->event_data.exit.exit_code;
						return 0;
					}
					break;
				default:
					printf("dropped packet, type %d\n",
							ev->what);
			}
		}
	} while(1);

	return -1;
}

