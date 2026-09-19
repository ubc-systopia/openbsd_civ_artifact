CC ?= cc
BUFFER_CHECKER_ROOT ?= ${.CURDIR}/ptr_checker

PROG= isakmpd-priv-harness
SRCS= isakmpd.c monitor.c monitor_fdpass.c harness_stubs.c
OBJS= $(SRCS:.c=.o)
GENERATED= exchange_num.h ipsec_fld.h ipsec_num.h isakmp_fld.h \
	isakmp_num.h
GENERATED_OUTPUTS= exchange_num.c exchange_num.h ipsec_fld.c ipsec_fld.h \
	ipsec_num.c ipsec_num.h isakmp_fld.c isakmp_fld.h isakmp_num.c \
	isakmp_num.h

CFLAGS+= -Wall -I${.CURDIR} -Icompat -I${BUFFER_CHECKER_ROOT}
CFLAGS+= -include compat/openbsd-compat.h
CFLAGS+= -DISAKMPD_PRIV_ONLY
CFLAGS+= -ffunction-sections -fdata-sections
CFLAGS+= -Wextra -Wstrict-prototypes -Wpointer-arith -Wcast-qual -g
LDFLAGS+= -Wl,--gc-sections -L${BUFFER_CHECKER_ROOT}
LDFLAGS+= -Wl,-rpath,'$$ORIGIN/ptr_checker'
LDADD+= ${BUFFER_CHECKER_ROOT}/libmsg_generator.a
LDADD+= -lbuffer_check -lprocstat -lmd

CHECKS ?= none
.if "$(CHECKS)" == "msan"
CFLAGS+= -fsanitize=memory -fsanitize-recover=memory
LDFLAGS+= -fsanitize=memory
.endif

all: $(PROG)

$(OBJS): $(GENERATED)

exchange_num.c exchange_num.h: genconstants.sh exchange_num.cst
	/bin/sh ${.CURDIR}/genconstants.sh ${.CURDIR}/exchange_num

ipsec_fld.c ipsec_fld.h: genfields.sh ipsec_fld.fld
	/bin/sh ${.CURDIR}/genfields.sh ${.CURDIR}/ipsec_fld

ipsec_num.c ipsec_num.h: genconstants.sh ipsec_num.cst
	/bin/sh ${.CURDIR}/genconstants.sh ${.CURDIR}/ipsec_num

isakmp_fld.c isakmp_fld.h: genfields.sh isakmp_fld.fld
	/bin/sh ${.CURDIR}/genfields.sh ${.CURDIR}/isakmp_fld

isakmp_num.c isakmp_num.h: genconstants.sh isakmp_num.cst
	/bin/sh ${.CURDIR}/genconstants.sh ${.CURDIR}/isakmp_num

$(PROG): $(GENERATED) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDADD)

clean:
	rm -f $(PROG) $(OBJS) $(GENERATED_OUTPUTS)

.PHONY: all clean
