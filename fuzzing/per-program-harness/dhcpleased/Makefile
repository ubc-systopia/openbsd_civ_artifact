#	$OpenBSD: Makefile,v 1.3 2022/08/29 17:00:29 deraadt Exp $

CC ?= cc
BUFFER_CHECKER_ROOT ?= ${.CURDIR}/ptr_checker

PROG=	dhcpleased-priv-harness
SRCS=	bpf.c dhcpleased.c log.c parse.c
OBJS=	$(SRCS:.c=.o)
IMSG_OBJS= lib/libutil/imsg.pic.o lib/libutil/imsg-buffer.pic.o
IMSG_LIB= libprivimsg.so

CFLAGS+= -Wall -I${.CURDIR} -Icompat -Ilib/libutil \
	-I${BUFFER_CHECKER_ROOT} -I/usr/local/include
CFLAGS+= -include compat/openbsd-compat.h
CFLAGS+= -DDHCPLEASED_PRIV_ONLY
CFLAGS+= -ffunction-sections -fdata-sections
CFLAGS+= -Wextra -Wstrict-prototypes -Wmissing-prototypes
CFLAGS+= -Wmissing-declarations -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+= -Wsign-compare -g
LDFLAGS+= -Wl,--gc-sections -L/usr/local/lib -L${BUFFER_CHECKER_ROOT} -L${.CURDIR}
LDFLAGS+= -Wl,-rpath,'$$ORIGIN' -Wl,-rpath,'$$ORIGIN/ptr_checker'
LDADD+=	${BUFFER_CHECKER_ROOT}/libmsg_generator.a
LDADD+=	-lbuffer_check -lprivimsg -levent -lprocstat -lmd

CHECKS ?= none
.if "$(CHECKS)" == "msan"
CFLAGS+= -fsanitize=memory -fsanitize-recover=memory
LDFLAGS+= -fsanitize=memory
.endif

all: $(IMSG_LIB) $(PROG)

parse.c: parse.y
	byacc -o $@ parse.y

lib/libutil/imsg.pic.o: lib/libutil/imsg.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ lib/libutil/imsg.c

lib/libutil/imsg-buffer.pic.o: lib/libutil/imsg-buffer.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ lib/libutil/imsg-buffer.c

$(IMSG_LIB): $(IMSG_OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ $(IMSG_OBJS)

$(PROG): $(OBJS) $(IMSG_LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDADD)

clean:
	rm -f $(PROG) $(OBJS) $(IMSG_OBJS) $(IMSG_LIB) parse.c

.PHONY: all clean
