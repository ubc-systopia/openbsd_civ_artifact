CC ?= cc
BUFFER_CHECKER_ROOT ?= ${.CURDIR}/ptr_checker

PROG= acme-fileproc-harness
SRCS= main.c fileproc.c util.c dbg.c
OBJS= ${SRCS:.c=.o}

CFLAGS+= -Wall -Wextra -g -I${.CURDIR} -Icompat \
	-I${BUFFER_CHECKER_ROOT} -DACME_FILEPROC_FUZZ
CFLAGS+= -include compat/openbsd-compat.h
CFLAGS+= -ffunction-sections -fdata-sections
CFLAGS+= -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations
CFLAGS+= -Wshadow -Wpointer-arith -Wsign-compare
LDFLAGS+= -Wl,--gc-sections -L${BUFFER_CHECKER_ROOT}
LDFLAGS+= -Wl,-rpath,'$$ORIGIN/ptr_checker'
LDADD+= ${BUFFER_CHECKER_ROOT}/libmsg_generator.a
LDADD+= -lbuffer_check -lprocstat -lmd

CHECKS ?= none
.if "${CHECKS}" == "msan"
CFLAGS+= -fsanitize=memory -fsanitize-recover=memory
LDFLAGS+= -fsanitize=memory
.endif

all: ${PROG}

${PROG}: ${OBJS}
	${CC} ${LDFLAGS} -o $@ ${OBJS} ${LDADD}

clean:
	rm -f ${PROG} ${OBJS}

.PHONY: all clean
