#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

#include "buffer_check_lib.h"

struct imsgbuf;

#ifdef USE_IMSG
struct wire_imsg_hdr {
	uint32_t type;
	uint32_t len;
	uint32_t peerid;
	pid_t pid;
};

static void
check_imsg_iov(const struct iovec *iov, size_t iovcnt)
{
	struct wire_imsg_hdr hdr;
	unsigned char *wire;
	size_t i, offset, total;

	total = 0;
	for (i = 0; i < iovcnt; i++) {
		if (iov[i].iov_len > SIZE_MAX - total)
			return;
		total += iov[i].iov_len;
	}
	if (total == 0)
		return;
	if ((wire = malloc(total)) == NULL)
		return;
	offset = 0;
	for (i = 0; i < iovcnt; i++) {
		memcpy(wire + offset, iov[i].iov_base, iov[i].iov_len);
		offset += iov[i].iov_len;
	}

	offset = 0;
	while (total - offset >= sizeof(hdr)) {
		memcpy(&hdr, wire + offset, sizeof(hdr));
		if (hdr.len < sizeof(hdr) || hdr.len > total - offset)
			break;
		if (hdr.len > sizeof(hdr))
			check_buffer(wire + offset + sizeof(hdr),
			    hdr.len - sizeof(hdr));
		offset += hdr.len;
	}
	free(wire);
}
#endif

#ifdef INTERCEPT_WRITE
ssize_t
write(int fd, const void *buf, size_t nbytes)
{
	static ssize_t (*real_write)(int, const void *, size_t);

	check_buffer(buf, nbytes);
	if (real_write == NULL) {
		real_write = (ssize_t (*)(int, const void *, size_t))
		    dlsym(RTLD_NEXT, "write");
	}
	return real_write(fd, buf, nbytes);
}
#endif

#ifdef INTERCEPT_IMSG_COMPOSE
int
imsg_compose(struct imsgbuf *imsgbuf, uint32_t type, uint32_t id, pid_t pid,
    int fd, const void *data, size_t datalen)
{
	static int (*real_imsg_compose)(struct imsgbuf *, uint32_t, uint32_t,
	    pid_t, int, const void *, size_t);

	check_buffer(data, datalen);
	if (real_imsg_compose == NULL) {
		real_imsg_compose = (int (*)(struct imsgbuf *, uint32_t,
		    uint32_t, pid_t, int, const void *, size_t))
		    dlsym(RTLD_NEXT, "imsg_compose");
	}
	return real_imsg_compose(imsgbuf, type, id, pid, fd, data, datalen);
}
#endif

#ifdef INTERCEPT_IMSG_COMPOSEV
int
imsg_composev(struct imsgbuf *imsgbuf, uint32_t type, uint32_t id, pid_t pid,
    int fd, const struct iovec *iov, int iovcnt)
{
	static int (*real_imsg_composev)(struct imsgbuf *, uint32_t, uint32_t,
	    pid_t, int, const struct iovec *, int);
	int i;

	if (iov != NULL) {
		for (i = 0; i < iovcnt; i++)
			check_buffer(iov[i].iov_base, iov[i].iov_len);
	}
	if (real_imsg_composev == NULL) {
		real_imsg_composev = (int (*)(struct imsgbuf *, uint32_t,
		    uint32_t, pid_t, int, const struct iovec *, int))
		    dlsym(RTLD_NEXT, "imsg_composev");
	}
	return real_imsg_composev(imsgbuf, type, id, pid, fd, iov, iovcnt);
}
#endif

#ifdef INTERCEPT_SENDMSG
ssize_t
sendmsg(int fd, const struct msghdr *msg, int flags)
{
	static ssize_t (*real_sendmsg)(int, const struct msghdr *, int);

	printf("intercepting sendmsg!!!\n");
	if (msg != NULL && msg->msg_iov != NULL) {
#ifdef USE_IMSG
		check_imsg_iov(msg->msg_iov, msg->msg_iovlen);
#else
		size_t i;

		for (i = 0; i < msg->msg_iovlen; i++) {
			check_buffer(msg->msg_iov[i].iov_base,
			    msg->msg_iov[i].iov_len);
		}
#endif
	}

	if (real_sendmsg == NULL) {
		real_sendmsg = (ssize_t (*)(int, const struct msghdr *, int))
		    dlsym(RTLD_NEXT, "sendmsg");
	}

	return real_sendmsg(fd, msg, flags);
}
#endif
