#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
SHAREDIR?=	/usr/local
LIB=		event
SHLIB_NAME=	lib${SHLIB}.so
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=	-Werror -Wstrict-prototypes -fPIC -I${.CURDIR} \
		-I${.CURDIR}/../../include
SRCS=		buffer.c evbuffer.c event.c event_tagging.c evutil.c kqueue.c \
		log.c poll.c select.c signal.c \
		../libc/stdlib/reallocarray.c \
		../libc/stdlib/recallocarray.c \
		../libc/asr/asr.c
INCS=		event.h evutil.h
MAN=		event.3 evbuffer_new.3
NO_OBJ=		yes

.include <bsd.lib.mk>
