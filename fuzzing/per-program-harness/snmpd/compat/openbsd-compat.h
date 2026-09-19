#ifndef OPENBSD_COMPAT_H
#define OPENBSD_COMPAT_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/queue.h>

#ifndef __dead
#define __dead __attribute__((__noreturn__))
#endif

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX	255
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

#ifndef SIMPLEQ_HEAD
#define SIMPLEQ_HEAD(name, type)	STAILQ_HEAD(name, type)
#define SIMPLEQ_ENTRY(type)		STAILQ_ENTRY(type)
#define SIMPLEQ_INIT(head)		STAILQ_INIT(head)
#define SIMPLEQ_FIRST(head)		STAILQ_FIRST(head)
#define SIMPLEQ_NEXT(elm, field)	STAILQ_NEXT(elm, field)
#define SIMPLEQ_EMPTY(head)		STAILQ_EMPTY(head)
#define SIMPLEQ_INSERT_HEAD(head, elm, field) \
	STAILQ_INSERT_HEAD(head, elm, field)
#define SIMPLEQ_INSERT_TAIL(head, elm, field) \
	STAILQ_INSERT_TAIL(head, elm, field)
#define SIMPLEQ_REMOVE_HEAD(head, field)	STAILQ_REMOVE_HEAD(head, field)
#define SIMPLEQ_FOREACH(var, head, field)	STAILQ_FOREACH(var, head, field)
#endif

#ifndef LINK_STATE_IS_UP
#define LINK_STATE_IS_UP(_s)	((_s) == LINK_STATE_UP)
#endif

#ifndef RTP_ANY
#define RTP_ANY		64
#endif

#ifndef RTF_MPATH
#define RTF_MPATH	0x40000
#endif

#ifndef RTF_CONNECTED
#define RTF_CONNECTED	0x800000
#endif

#ifndef IN6_IS_ADDR_MC_INTFACELOCAL
#define IN6_IS_ADDR_MC_INTFACELOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a) && (((const uint8_t *)(a))[1] & 0x0f) == 0x01)
#endif

#endif /* OPENBSD_COMPAT_H */
