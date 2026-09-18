/*	$OpenBSD: snmpd.c,v 1.54 2026/06/18 10:45:33 martijn Exp $	*/

/*
 * Copyright (c) 2007, 2008, 2012 Reyk Floeter <reyk@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/wait.h>

#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "log.h"
#include "snmpd.h"
#ifdef SNMPD_PRIV_ONLY
#include "msg_generator.h"
#include "buffer_check_lib.h"
#endif

#ifdef SNMPD_PRIV_ONLY
#define IMSG_EOM	0x7fffffff
#endif

__dead void	 usage(void);

void	 snmpd_shutdown(struct snmpd *);
#ifndef SNMPD_PRIV_ONLY
void	 snmpd_sig_handler(int, short, void *);
#endif
int	 snmpd_dispatch_snmpe(int, struct privsep_proc *, struct imsg *);
void	 snmpd_backend(struct snmpd *);
#ifndef SNMPD_PRIV_ONLY
int	 check_child(pid_t, const char *);
#endif

struct snmpd	*snmpd_env;

static struct privsep_proc procs[] = {
#ifdef SNMPD_PRIV_ONLY
	{ "snmpe", PROC_SNMPE, snmpd_dispatch_snmpe },
#else
	{ "snmpe", PROC_SNMPE, snmpd_dispatch_snmpe, snmpe, snmpe_shutdown },
#endif
};

#ifndef SNMPD_PRIV_ONLY
void
snmpd_sig_handler(int sig, short event, void *arg)
{
	struct privsep	*ps = arg;
	struct snmpd	*env = ps->ps_env;
	int		 die = 0, status, fail, id;
	pid_t		pid;
	char		*cause;

	switch (sig) {
	case SIGTERM:
	case SIGINT:
		die = 1;
		/* FALLTHROUGH */
	case SIGCHLD:
		do {
			int len;

			pid = waitpid(WAIT_ANY, &status, WNOHANG);
			if (pid <= 0)
				continue;

			fail = 0;
			if (WIFSIGNALED(status)) {
				fail = 1;
				len = asprintf(&cause, "terminated; signal %d",
				    WTERMSIG(status));
			} else if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) != 0) {
					fail = 1;
					len = asprintf(&cause,
					    "exited abnormally");
				} else
					len = asprintf(&cause, "exited okay");
			} else
				fatalx("unexpected cause of SIGCHLD");

			if (len == -1)
				fatal("asprintf");
			
			for (id = 0; id < PROC_MAX; id++) {
				if (pid == ps->ps_pid[id] &&
				    check_child(ps->ps_pid[id],
				    ps->ps_title[id])) {
					die  = 1;
					if (fail)
						log_warnx("lost child: %s %s",
						    ps->ps_title[id], cause);
					break;
				}
			}
			free(cause);
		} while (pid > 0 || (pid == -1 && errno == EINTR));

		if (die)
			snmpd_shutdown(env);
		break;
	case SIGHUP:
		/* reconfigure */
		break;
	case SIGUSR1:
		/* ignore */
		break;
	default:
		fatalx("unexpected signal");
	}
}
#endif

__dead void
usage(void)
{
	extern char	*__progname;

	fprintf(stderr, "usage: %s [-dNnv] [-D macro=value] "
	    "[-f file]\n", __progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int		 c;
	int		 argc0 = argc;
	char		**argv0 = argv;
	struct snmpd	*env;
	int		 debug = 0, verbose = 0;
	u_int		 flags = 0;
#ifndef SNMPD_PRIV_ONLY
	int		 noaction = 0;
	const char	*conffile = CONF_FILE;
	int		 proc_id = PROC_PARENT, proc_instance = 0;
	const char	*errp, *title = NULL;
#else
	struct imsgbuf	 snmpe_fuzz_ibuf;
	struct msg_interface fuzz_iface;
	struct msg_data	 fuzz_msg;
	struct msg_data	 fuzz_eom;
	uint8_t		 i;
#endif
	struct privsep	*ps;

	smi_init();

	/* log to stderr until daemonized */
	log_init(1, LOG_DAEMON);

	while ((c = getopt(argc, argv, "dD:nNf:I:P:v")) != -1) {
		switch (c) {
		case 'd':
			debug++;
			flags |= SNMPD_F_DEBUG;
			break;
		case 'D':
#ifndef SNMPD_PRIV_ONLY
			if (cmdline_symset(optarg) < 0)
				log_warnx("could not parse macro definition %s",
				    optarg);
#endif
			break;
		case 'n':
#ifndef SNMPD_PRIV_ONLY
			noaction = 1;
#endif
			break;
		case 'N':
			flags |= SNMPD_F_NONAMES;
			break;
		case 'f':
#ifndef SNMPD_PRIV_ONLY
			conffile = optarg;
#endif
			break;
		case 'I':
#ifndef SNMPD_PRIV_ONLY
			proc_instance = strtonum(optarg, 0,
			    PROC_MAX_INSTANCES, &errp);
			if (errp)
				fatalx("invalid process instance");
#endif
			break;
		case 'P':
#ifndef SNMPD_PRIV_ONLY
			title = optarg;
			proc_id = proc_getid(procs, nitems(procs), title);
			if (proc_id == PROC_MAX)
				fatalx("invalid process name");
#endif
			break;
		case 'v':
			verbose++;
			flags |= SNMPD_F_VERBOSE;
			break;
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;
	if (argc > 0)
		usage();

	log_setverbose(verbose);

#ifndef SNMPD_PRIV_ONLY
	if ((env = parse_config(conffile, flags)) == NULL)
		exit(1);
#else
	if ((env = calloc(1, sizeof(*env))) == NULL)
		fatal("calloc");
	env->sc_flags = flags | SNMPD_F_DEBUG;
	TAILQ_INIT(&env->sc_addresses);
	TAILQ_INIT(&env->sc_agentx_masters);
	TAILQ_INIT(&env->sc_trapreceivers);
#endif

	ps = &env->sc_ps;
	ps->ps_env = env;
	snmpd_env = env;
#ifndef SNMPD_PRIV_ONLY
	ps->ps_instance = proc_instance;
	if (title)
		ps->ps_title[proc_id] = title;

	if (noaction) {
		fprintf(stderr, "configuration ok\n");
		exit(0);
	}

	if (geteuid())
		errx(1, "need root privileges");

	if ((ps->ps_pw = getpwnam(SNMPD_USER)) == NULL)
		errx(1, "unknown user %s", SNMPD_USER);
#else
	ps->ps_instance = 0;
	if ((ps->ps_pw = getpwuid(0)) == NULL)
		fatal("getpwuid");
#endif

	log_init(debug, LOG_DAEMON);
	log_setverbose(verbose);

	env->sc_engine_boots = 0;

#ifndef SNMPD_PRIV_ONLY
	proc_init(ps, procs, nitems(procs), debug, argc0, argv0, proc_id);
#else
	proc_init(ps, procs, nitems(procs), 1, argc0, argv0, PROC_PARENT);
#endif

	log_procinit("parent");
	log_info("startup");

	event_init();

#ifndef SNMPD_PRIV_ONLY
	signal_set(&ps->ps_evsigint, SIGINT, snmpd_sig_handler, ps);
	signal_set(&ps->ps_evsigterm, SIGTERM, snmpd_sig_handler, ps);
	signal_set(&ps->ps_evsigchld, SIGCHLD, snmpd_sig_handler, ps);
	signal_set(&ps->ps_evsighup, SIGHUP, snmpd_sig_handler, ps);
	signal_set(&ps->ps_evsigpipe, SIGPIPE, snmpd_sig_handler, ps);
	signal_set(&ps->ps_evsigusr1, SIGUSR1, snmpd_sig_handler, ps);

	signal_add(&ps->ps_evsigint, NULL);
	signal_add(&ps->ps_evsigterm, NULL);
	signal_add(&ps->ps_evsigchld, NULL);
	signal_add(&ps->ps_evsighup, NULL);
	signal_add(&ps->ps_evsigpipe, NULL);
	signal_add(&ps->ps_evsigusr1, NULL);
#endif

	proc_connect(ps, NULL);

#ifdef SNMPD_PRIV_ONLY
	/* Install the synthetic receiver before privileged setup messages. */
	if (imsgbuf_init(&snmpe_fuzz_ibuf,
	    ps->ps_pipes[PROC_SNMPE][0].pp_pipes[PROC_PARENT][0]) == -1)
		fatal("imsgbuf_init");
#endif

	snmpd_backend(env);

#ifndef SNMPD_PRIV_ONLY
	if (pledge("stdio dns sendfd proc exec id", NULL) == -1)
		fatal("pledge");
#else
	/*
	 * Harness mode uses the child side of the snmpe socketpair as the
	 * generated endpoint and feeds only snmpe-to-parent messages into the
	 * original parent dispatcher.  Generated fds are discarded because
	 * these parent-side messages do not legitimately carry descriptors.
	 */
	if (msg_interface_init(&fuzz_iface, STDIN_FILENO, 1,
	    IMSG_SENDFD_DONE - IMSG_NONE + 2) !=
	    MSG_GEN_SUCCESS)
		fatalx("msg_interface_init");
	if (msg_interface_set_message_types_batch(&fuzz_iface, 0,
	    IMSG_SENDFD_DONE - IMSG_NONE + 2, IMSG_NONE) !=
	    MSG_GEN_SUCCESS)
		fatalx("set message type range");
	msg_interface_set_endpoint(&fuzz_iface, 0, &snmpe_fuzz_ibuf);
	msg_interface_set_eom_type(&fuzz_iface, IMSG_EOM);

	while (msg_generate(&fuzz_iface, &fuzz_msg) == MSG_GEN_SUCCESS) {
		struct imsgbuf	*target = fuzz_msg.endpoint;
		const void	*payload = fuzz_msg.payload;
		size_t		 payload_size = fuzz_msg.actual_payload_size;

		if (fuzz_msg.fd != -1)
			close(fuzz_msg.fd);
		if (payload != NULL && payload_size > 0)
			ptr_check_skip(payload, payload_size);
		if (target == NULL)
			continue;
		if (imsg_compose(target, fuzz_msg.type, 0, 0, -1, payload,
		    payload_size) == -1)
			fatal("imsg_compose");
		if (imsgbuf_flush(target) == -1)
			fatal("imsgbuf_flush");
	}

	for (i = 0; i < fuzz_iface.num_compartments; i++) {
		struct imsgbuf	*target;

		if (msg_generate_eom(&fuzz_iface, i, &fuzz_eom) !=
		    MSG_GEN_SUCCESS)
			continue;
		target = fuzz_eom.endpoint;
		if (target == NULL)
			continue;
		if (imsg_compose(target, fuzz_eom.type, 0, 0, -1, NULL,
		    0) == -1)
			fatal("imsg_compose EOM");
		if (imsgbuf_flush(target) == -1)
			fatal("imsgbuf_flush EOM");
	}
#endif

	event_dispatch();

	log_debug("%d parent exiting", getpid());

	return (0);
}

#ifndef SNMPD_PRIV_ONLY
void
snmpd_shutdown(struct snmpd *env)
{
	proc_kill(&env->sc_ps);

	free(env);

	log_info("terminating");
	exit(0);
}

int
check_child(pid_t pid, const char *pname)
{
	int	status;

	if (waitpid(pid, &status, WNOHANG) > 0) {
		if (WIFEXITED(status)) {
			log_warnx("check_child: lost child: %s exited", pname);
			return (1);
		}
		if (WIFSIGNALED(status)) {
			log_warnx("check_child: lost child: %s terminated; "
			    "signal %d", pname, WTERMSIG(status));
			return (1);
		}
	}

	return (0);
}
#endif

int
snmpd_dispatch_snmpe(int fd, struct privsep_proc *p, struct imsg *imsg)
{
	switch (imsg->hdr.type) {
	case IMSG_TRAP_EXEC:
		return (traphandler_priv_recvmsg(p, imsg));
#ifdef SNMPD_PRIV_ONLY
	case IMSG_EOM:
		eom_counter_inc();
		return (0);
#endif
	default:
		break;
	}

	return (-1);
}

int
snmpd_socket_af(struct sockaddr_storage *ss, int type)
{
	int fd, serrno;
	const int enable = 1;

	fd = socket(ss->ss_family, (type == SOCK_STREAM ?
	    SOCK_STREAM | SOCK_NONBLOCK : SOCK_DGRAM) | SOCK_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable,
	    sizeof(enable)) == -1) {
		serrno = errno;
		close(fd);
		errno = serrno;
		return -1;
	}
	return fd;
}

u_long
snmpd_engine_time(void)
{
	struct timeval	 now;

	/*
	 * snmpEngineBoots should be stored in a non-volatile storage.
	 * snmpEngineTime is the number of seconds since snmpEngineBoots
	 * was last incremented. We don't rely on non-volatile storage.
	 * snmpEngineBoots is set to zero and snmpEngineTime to the system
	 * clock. Hence, the tuple (snmpEngineBoots, snmpEngineTime) is
	 * still unique and protects us against replay attacks. It only
	 * 'expires' a little bit sooner than the RFC3414 method.
	 */
	gettimeofday(&now, NULL);
	return now.tv_sec;
}

void
snmpd_backend(struct snmpd *env)
{
#ifdef SNMPD_PRIV_ONLY
	int pair[2];

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == -1)
		fatal("backend socketpair");
	if (proc_compose_imsg(&env->sc_ps, PROC_SNMPE, -1,
	    IMSG_AX_FD, -1, pair[1], NULL, 0) == -1)
		fatal("proc_compose_imsg");
	if (proc_compose(&env->sc_ps, PROC_SNMPE,
	    IMSG_SENDFD_DONE, NULL, 0) == -1)
		fatal("proc_compose");
	close(pair[0]);
	return;
#else
	DIR *dir;
	struct dirent *file;
	int pair[2];
	char *argv[8];
	char execpath[PATH_MAX];
	size_t i = 0;

	if ((dir = opendir(SNMPD_BACKEND)) == NULL)
		fatal("opendir \"%s\"", SNMPD_BACKEND);

	argv[i++] = execpath;
	if (env->sc_rtfilter) {
		argv[i++] = "-C";
		argv[i++] = "filter-routes";
	}
	if (env->sc_flags & SNMPD_F_VERBOSE)
		argv[i++] = "-vv";
	if (env->sc_flags & SNMPD_F_DEBUG)
		argv[i++] = "-d";
	argv[i++] = "-x";
	argv[i++] = "3";
	argv[i] = NULL;
	while ((file = readdir(dir)) != NULL) {
		if (file->d_name[0] == '.')
			continue;
		if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == -1)
			fatal("socketpair");
		switch (fork()) {
		case -1:
			fatal("fork");
		case 0:
			close(pair[1]);
			if (dup2(pair[0], 3) == -1)
				fatal("dup2");
			if (closefrom(4) == -1)
				fatal("closefrom");
			(void)snprintf(execpath, sizeof(execpath), "%s/%s",
			    SNMPD_BACKEND, file->d_name);
			execv(argv[0], argv);
			fatal("execv");
		default:
			close(pair[0]);
			if (proc_compose_imsg(&env->sc_ps, PROC_SNMPE, -1,
			    IMSG_AX_FD, -1, pair[1], NULL, 0) == -1)
				fatal("proc_compose_imsg");
			continue;
		}
	}
	if (proc_compose(&env->sc_ps, PROC_SNMPE,
	    IMSG_SENDFD_DONE, NULL, 0) == -1)
		fatal("proc_compose");
#endif
}
