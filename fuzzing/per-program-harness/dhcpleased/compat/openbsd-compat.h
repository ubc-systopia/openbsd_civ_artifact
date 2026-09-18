#ifndef OPENBSD_COMPAT_H
#define OPENBSD_COMPAT_H

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <stdlib.h>
#include <string.h>
#include <vis.h>

#ifndef SIMPLEQ_HEAD
#define SIMPLEQ_HEAD(name, type)	STAILQ_HEAD(name, type)
#define SIMPLEQ_ENTRY(type)		STAILQ_ENTRY(type)
#define SIMPLEQ_INIT(head)		STAILQ_INIT(head)
#define SIMPLEQ_FIRST(head)		STAILQ_FIRST(head)
#define SIMPLEQ_NEXT(elm, field)	STAILQ_NEXT(elm, field)
#define SIMPLEQ_INSERT_HEAD(head, elm, field) \
	STAILQ_INSERT_HEAD(head, elm, field)
#define SIMPLEQ_INSERT_TAIL(head, elm, field) \
	STAILQ_INSERT_TAIL(head, elm, field)
#define SIMPLEQ_REMOVE_HEAD(head, field)	STAILQ_REMOVE_HEAD(head, field)
#define SIMPLEQ_FOREACH(var, head, field)	STAILQ_FOREACH(var, head, field)
#define SIMPLEQ_CONCAT(head1, head2)	STAILQ_CONCAT(head1, head2)
#endif

#ifndef __dead
#define __dead __attribute__((__noreturn__))
#endif

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif

static inline int
openbsd_strnunvis(char *dst, const char *src, size_t len)
{
	return (strnunvis(dst, len, src));
}
#define strnunvis(dst, src, len)	openbsd_strnunvis((dst), (src), (len))

#ifndef RTABLE_ANY
#define RTABLE_ANY	0xffffffff
#endif

#ifndef ROUTE_FILTER
#define ROUTE_FILTER(_m)	(1 << (_m))
#endif

#ifndef ROUTE_MSGFILTER
#define ROUTE_MSGFILTER	1
#endif

#ifndef ROUTE_TABLEFILTER
#define ROUTE_TABLEFILTER	2
#endif

#ifndef SO_RTABLE
#define SO_RTABLE	0x1021
#endif

#ifndef RTF_CLONING
#define RTF_CLONING	0x100
#endif

#ifndef RTF_MPATH
#define RTF_MPATH	0x40000
#endif

#ifndef RTA_LABEL
#define RTA_LABEL	0x400
#endif

#ifndef RTA_DNS
#define RTA_DNS		0x1000
#endif

#ifndef RTM_PROPOSAL
#define RTM_PROPOSAL	0x13
#endif

#ifndef RTP_NONE
#define RTP_NONE	0
#endif

#ifndef RTP_PROPOSAL_DHCLIENT
#define RTP_PROPOSAL_DHCLIENT	58
#endif

#ifdef __FreeBSD__
#define rtm_tableid	_rtm_spare1
#define rtm_priority	rtm_rmx.rmx_weight

struct sockaddr_rtlabel {
	uint8_t		sr_len;
	sa_family_t	sr_family;
	char		sr_label[32];
};

struct sockaddr_rtdns {
	uint8_t		sr_len;
	sa_family_t	sr_family;
	char		sr_dns[128];
};
#endif

#ifndef HAVE_FREEZERO
static inline void
freezero(void *ptr, size_t len)
{
	if (ptr == NULL)
		return;
	explicit_bzero(ptr, len);
	free(ptr);
}
#endif

#endif /* OPENBSD_COMPAT_H */
