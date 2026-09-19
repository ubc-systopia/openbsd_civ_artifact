#ifndef OPENBSD_COMPAT_H
#define OPENBSD_COMPAT_H

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#ifndef SIMPLEQ_HEAD
#define SIMPLEQ_HEAD(name, type) STAILQ_HEAD(name, type)
#define SIMPLEQ_ENTRY(type) STAILQ_ENTRY(type)
#define SIMPLEQ_INIT(head) STAILQ_INIT(head)
#define SIMPLEQ_FIRST(head) STAILQ_FIRST(head)
#define SIMPLEQ_NEXT(elm, field) STAILQ_NEXT(elm, field)
#define SIMPLEQ_EMPTY(head) STAILQ_EMPTY(head)
#define SIMPLEQ_INSERT_HEAD(head, elm, field) STAILQ_INSERT_HEAD(head, elm, field)
#define SIMPLEQ_INSERT_TAIL(head, elm, field) STAILQ_INSERT_TAIL(head, elm, field)
#define SIMPLEQ_REMOVE_HEAD(head, field) STAILQ_REMOVE_HEAD(head, field)
#define SIMPLEQ_FOREACH(var, head, field) STAILQ_FOREACH(var, head, field)
#define SIMPLEQ_CONCAT(head1, head2) STAILQ_CONCAT(head1, head2)
#endif

#ifndef LINK_STATE_IS_UP
#define LINK_STATE_IS_UP(state) ((state) == LINK_STATE_UP)
#endif

#ifndef IFT_CARP
#define IFT_CARP 0xf7
#endif

#ifndef RTP_ANY
#define RTP_ANY 64
#endif
#ifndef RTF_CONNECTED
#define RTF_CONNECTED 0x800000
#endif
#ifndef ROUTE_PRIOFILTER
#define ROUTE_PRIOFILTER 3
#endif
#ifndef ROUTE_FLAGFILTER
#define ROUTE_FLAGFILTER 4
#endif
#ifndef RTAX_LABEL
#define RTAX_LABEL 10
#endif
#ifndef RTM_DESYNC
#define RTM_DESYNC 0x10
#endif

#ifndef __dead
#define __dead __attribute__((__noreturn__))
#endif

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif

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
	uint8_t		sr_dns[sizeof(struct in_addr) * 8];
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
