#	$OpenBSD: Makefile,v 1.9 2022/01/13 10:34:07 martijn Exp $

PROG=	syslogd
SRCS=	evbuffer_tls.c log.c parsemsg.c privsep.c privsep_fdpass.c ringbuf.c \
	syslogd.c ttymsg.c getdtablecount.c


CFLAGS+= -I${.CURDIR}/../../libevent/src/lib/libevent
CFLAGS+= -I${.CURDIR}/../../compat
CFLAGS+= -I/usr/local/include
CFLAGS+= -I${.CURDIR}/../../include
CFLAGS+= -I${BUFFER_CHECKER_ROOT}
CFLAGS+= -fsanitize-recover=memory

.PATH: ${.CURDIR}/../../compat
LDFLAGS+= -L${.CURDIR}/../../libevent/src/lib/libevent -Wl,-rpath,${.CURDIR}/../../libevent/src/lib/libevent
LDFLAGS+= -L/usr/local/lib
MAN=	syslogd.8 syslog.conf.5
LDADD=	-levent -ltls -lssl -lcrypto
LDADD+=	${BUFFER_CHECKER_ROOT}/libmsg_generator.a
DPADD=	${LIBEVENT} ${LIBTLS} ${LIBSSL} ${LIBCRYPTO}
DPADD+=	${BUFFER_CHECKER_ROOT}/libmsg_generator.a

.include <bsd.prog.mk>

CFLAGS+= -g --coverage
