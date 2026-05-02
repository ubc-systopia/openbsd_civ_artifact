#	$OpenBSD: Makefile,v 1.5 2022/08/29 17:00:30 deraadt Exp $

CC=afl-clang-lto
PROG=	mountd
MAN=	exports.5 mountd.8

.PATH: ${.CURDIR}/../../lib/libutil
.PATH: ${.CURDIR}/../../lib/libc/rpc
SRCS+= mountd.c imsg.c imsg-buffer.c rpc_commondata.c svc.c
# SRCS+= mountd.c imsg.c imsg-buffer.c rpc_commondata.c

LDADD=	-lutil
LDADD+=	${BUFFER_CHECKER_ROOT}/libmsg_generator.a
DPADD=	${LIBUTIL}
DPADD+=	${BUFFER_CHECKER_ROOT}/libmsg_generator.a

CFLAGS+= -I${.CURDIR}/../../sys
CFLAGS+= -I${.CURDIR}/../../compat
CFLAGS+= -I${.CURDIR}/../../include
CFLAGS+= -I${.CURDIR}/../../lib/libutil
CFLAGS+= -I${BUFFER_CHECKER_ROOT}
CFLAGS+= -fsanitize-recover=memory

.include <bsd.prog.mk>

# Don't compile mountd as static binary by default
LDSTATIC=       
