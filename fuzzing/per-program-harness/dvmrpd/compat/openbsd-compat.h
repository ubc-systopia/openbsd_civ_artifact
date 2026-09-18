#ifndef OPENBSD_COMPAT_H
#define OPENBSD_COMPAT_H

#include <stdlib.h>
#include <string.h>

#ifndef __dead
#define __dead __attribute__((__noreturn__))
#endif

#ifndef __unused
#define __unused __attribute__((__unused__))
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
