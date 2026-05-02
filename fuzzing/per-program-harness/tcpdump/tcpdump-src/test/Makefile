PROG= test
SRCS= test.c
CFLAGS += -I${.CURDIR}/../lib/libpcap
CFLAGS += -I${.CURDIR}/../sys
CFLAGS += -lpcap

LDFLAGS+= -L${.CURDIR}/../lib/libpcap

.include <bsd.prog.mk>

run:
	sudo LD_LIBRARY_PATH=${.CURDIR}/../lib/libpcap ./test

