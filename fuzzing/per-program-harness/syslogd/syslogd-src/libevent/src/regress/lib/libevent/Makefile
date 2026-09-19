#       $OpenBSD: Makefile,v 1.7 2010/06/26 23:31:11 nicm Exp $

REGRESS_TARGETS=  run-regress run-test-eof run-test-init run-test-time
REGRESS_TARGETS+= run-test-weof

CFLAGS+= -I${.CURDIR}/../../../lib/libevent \
	-DHAVE_FCNTL \
	-DHAVE_SIGACTION \
	-DHAVE_SYS_SOCKET_H \
	-DHAVE_SYS_TIME_H \
	-DHAVE_UNISTD_H
LDFLAGS+= -L${.CURDIR}/../../../lib/libevent -levent
CLEANFILES= event_regress test-eof test-init test-time test-weof

.PHONY: ${REGRESS_TARGETS}

run-regress: event_regress
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOPOLL=1 EVENT_NOSELECT=1 ./event_regress
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOKQUEUE=1 EVENT_NOSELECT=1 ./event_regress
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOKQUEUE=1 EVENT_NOPOLL=1 ./event_regress

run-test-eof: test-eof
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOPOLL=1 EVENT_NOSELECT=1 ./test-eof
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOKQUEUE=1 EVENT_NOSELECT=1 ./test-eof
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOKQUEUE=1 EVENT_NOPOLL=1 ./test-eof

run-test-init: test-init
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOPOLL=1 EVENT_NOSELECT=1 ./test-init
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOKQUEUE=1 EVENT_NOSELECT=1 ./test-init
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOKQUEUE=1 EVENT_NOPOLL=1 ./test-init

run-test-time: test-time
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOPOLL=1 EVENT_NOSELECT=1 ./test-time
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOKQUEUE=1 EVENT_NOSELECT=1 ./test-time
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOKQUEUE=1 EVENT_NOPOLL=1 ./test-time

run-test-weof: test-weof
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOPOLL=1 EVENT_NOSELECT=1 ./test-weof
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOKQUEUE=1 EVENT_NOSELECT=1 ./test-weof
	LD_LIBRARY_PATH=../../../lib/libevent EVENT_NOKQUEUE=1 EVENT_NOPOLL=1 ./test-weof

.include <bsd.regress.mk>
