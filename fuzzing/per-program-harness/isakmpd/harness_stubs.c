#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

int verbose_logging;
int pf_key_v2_socket = -1;
char *ui_fifo = "/tmp/isakmpd-fuzz.fifo";

void
set_slave_signals(void)
{
}

int
pf_key_v2_open(void)
{
	return (-1);
}

void
log_to(FILE *fp)
{
	(void)fp;
}

void
log_debug(int class, int level, const char *fmt, ...)
{
	(void)class;
	(void)level;
	(void)fmt;
}

void
log_error(const char *fmt, ...)
{
	(void)fmt;
}

void
log_errorx(const char *fmt, ...)
{
	(void)fmt;
}

void
log_print(const char *fmt, ...)
{
	(void)fmt;
}

void
log_fatal(const char *fmt, ...)
{
	(void)fmt;
	exit(1);
}

void
log_fatalx(const char *fmt, ...)
{
	(void)fmt;
	exit(1);
}
