/*	$OpenBSD: eigrpd.c,v 1.36 2024/11/21 13:38:14 claudio Exp $ */

/*
 * Copyright (c) 2015 Renato Westphal <renato@openbsd.org>
 * Copyright (c) 2005 Claudio Jeker <claudio@openbsd.org>
 * Copyright (c) 2004 Esben Norby <norby@openbsd.org>
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
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
#ifndef EIGRPD_PRIV_ONLY
#include <sys/wait.h>
#include <sys/sysctl.h>
#endif

#include <arpa/inet.h>
#ifndef EIGRPD_PRIV_ONLY
#include <err.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eigrpd.h"
#ifndef EIGRPD_PRIV_ONLY
#include "eigrpe.h"
#include "rde.h"
#else
#include "msg_generator.h"
#include "buffer_check_lib.h"
#endif
#include "log.h"

#ifdef EIGRPD_PRIV_ONLY
#define IMSG_INVALID	(IMSG_RECONF_END + 1)
#define IMSG_EOM	(IMSG_INVALID + 1)
#endif

#ifndef EIGRPD_PRIV_ONLY
static void		 main_sig_handler(int, short, void *);
static __dead void	 eigrpd_shutdown(void);
static pid_t		 start_child(enum eigrpd_process, char *, int, int, int,
			    char *);
#endif
static __dead void	 usage(void);
static void		 main_dispatch_eigrpe(int, short, void *);
static void		 main_dispatch_rde(int, short, void *);
static int		 main_imsg_send_ipc_sockets(struct imsgbuf *,
			    struct imsgbuf *);
static int		 main_imsg_send_config(struct eigrpd_conf *);
#ifndef EIGRPD_PRIV_ONLY
static int		 eigrp_reload(void);
#endif
static int		 eigrp_sendboth(enum imsg_type, void *, uint16_t);
static void		 main_copy_config(struct eigrpd_conf *,
			    const struct eigrpd_conf *);
static void		 main_copy_instance(struct eigrp *,
			    const struct eigrp *);
static void		 main_copy_iface(struct iface *, const struct iface *);
static void		 main_copy_eigrp_iface(struct eigrp_iface *,
			    const struct eigrp_iface *);
#ifndef EIGRPD_PRIV_ONLY
static void		 merge_instances(struct eigrpd_conf *, struct eigrp *,
			    struct eigrp *);
#endif

struct eigrpd_conf	*eigrpd_conf;

#ifndef EIGRPD_PRIV_ONLY
static char		*conffile;
#endif
static struct imsgev	*iev_eigrpe;
static struct imsgev	*iev_rde;
#ifndef EIGRPD_PRIV_ONLY
static pid_t		 eigrpe_pid;
static pid_t		 rde_pid;
#endif

#ifndef EIGRPD_PRIV_ONLY
static void
main_sig_handler(int sig, short event, void *arg)
{
	/* signal handler rules don't apply, libevent decouples for us */
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		eigrpd_shutdown();
		/* NOTREACHED */
	case SIGHUP:
		if (eigrp_reload() == -1)
			log_warnx("configuration reload failed");
		else
			log_debug("configuration reloaded");
		break;
	default:
		fatalx("unexpected signal");
		/* NOTREACHED */
		}
	}

#endif

static __dead void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-dnv] [-D macro=value]"
	    " [-f file] [-s socket]\n",
	    __progname);
	exit(1);
}

struct eigrpd_global global;

int
main(int argc, char *argv[])
{
#ifndef EIGRPD_PRIV_ONLY
	struct event		 ev_sigint, ev_sigterm, ev_sighup;
	char			*saved_argv0;
	int			 rflag = 0, eflag = 0;
	int			 ipforwarding;
	int			 mib[4];
	size_t			 len;
	char			*sockname;
#endif
#ifdef EIGRPD_PRIV_ONLY
	/*
	 * Fuzz-only state.  The harness writes generated imsgs to the child
	 * ends of the original parent/child socketpairs, so the real parent
	 * dispatchers remain the code under test.
	 */
	struct eigrpd_conf	*xconf;
	struct eigrp		*eigrp;
	struct iface		*iface0;
	struct eigrp_iface	*ei0;
	struct imsgbuf		 eigrpe_fuzz_ibuf, rde_fuzz_ibuf;
	struct msg_interface	 fuzz_iface;
	struct msg_data		 fuzz_msg, fuzz_eom;
	uint8_t			 i;
#endif
	int			 ch;
	int			 debug = 0;
	int			 pipe_parent2eigrpe[2];
	int			 pipe_parent2rde[2];

#ifndef EIGRPD_PRIV_ONLY
	conffile = CONF_FILE;
	sockname = EIGRPD_SOCKET;
#endif
	log_procname = "parent";

	log_init(1);	/* log to stderr until daemonized */
	log_verbose(1);

#ifndef EIGRPD_PRIV_ONLY
	saved_argv0 = argv[0];
	if (saved_argv0 == NULL)
		saved_argv0 = "eigrpd";
#endif

	while ((ch = getopt(argc, argv, "dD:f:ns:vRE")) != -1) {
		switch (ch) {
		case 'd':
			debug = 1;
			break;
		case 'D':
#ifndef EIGRPD_PRIV_ONLY
			if (cmdline_symset(optarg) < 0)
				log_warnx("could not parse macro definition %s",
				    optarg);
#endif
			break;
		case 'f':
#ifndef EIGRPD_PRIV_ONLY
			conffile = optarg;
#endif
			break;
		case 'n':
			global.cmd_opts |= EIGRPD_OPT_NOACTION;
			break;
		case 's':
#ifndef EIGRPD_PRIV_ONLY
			sockname = optarg;
#endif
			break;
		case 'v':
			if (global.cmd_opts & EIGRPD_OPT_VERBOSE)
				global.cmd_opts |= EIGRPD_OPT_VERBOSE2;
			global.cmd_opts |= EIGRPD_OPT_VERBOSE;
			break;
		case 'R':
#ifndef EIGRPD_PRIV_ONLY
			rflag = 1;
#endif
			break;
		case 'E':
#ifndef EIGRPD_PRIV_ONLY
			eflag = 1;
#endif
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}

	argc -= optind;
	argv += optind;
#ifndef EIGRPD_PRIV_ONLY
	if (argc > 0 || (rflag && eflag))
#else
	if (argc > 0)
#endif
		usage();

#ifndef EIGRPD_PRIV_ONLY
	if (rflag)
		rde(debug, global.cmd_opts & EIGRPD_OPT_VERBOSE);
	else if (eflag)
		eigrpe(debug, global.cmd_opts & EIGRPD_OPT_VERBOSE, sockname);

	mib[0] = CTL_NET;
	mib[1] = PF_INET;
	mib[2] = IPPROTO_IP;
	mib[3] = IPCTL_FORWARDING;
	len = sizeof(ipforwarding);
	if (sysctl(mib, 4, &ipforwarding, &len, NULL, 0) == -1)
		log_warn("sysctl");

	if (ipforwarding != 1)
		log_warnx("WARNING: IP forwarding NOT enabled");

	/* fetch interfaces early */
	kif_init();

	/* parse config file */
	if ((eigrpd_conf = parse_config(conffile)) == NULL) {
		kif_clear();
		exit(1);
	}

	if (global.cmd_opts & EIGRPD_OPT_NOACTION) {
		if (global.cmd_opts & EIGRPD_OPT_VERBOSE)
			print_config(eigrpd_conf);
		else
			fprintf(stderr, "configuration OK\n");
		kif_clear();
		exit(0);
	}

	/* check for root privileges  */
	if (geteuid())
		errx(1, "need root privileges");

	/* check for eigrpd user */
	if (getpwnam(EIGRPD_USER) == NULL)
		errx(1, "unknown user %s", EIGRPD_USER);
#else
	/*
	 * Harness fixture: parse.y and interface discovery are not compiled in
	 * the privileged-only build.  Seed the smallest config that lets the
	 * original parent setup and config-sending paths run.  This models the
	 * presence of one configured EIGRP instance and interface, not real
	 * routing state.
	 */
	if ((xconf = calloc(1, sizeof(*xconf))) == NULL ||
	    (eigrp = calloc(1, sizeof(*eigrp))) == NULL ||
	    (iface0 = calloc(1, sizeof(*iface0))) == NULL ||
	    (ei0 = calloc(1, sizeof(*ei0))) == NULL)
		fatal(NULL);

	TAILQ_INIT(&xconf->instances);
	TAILQ_INIT(&xconf->iface_list);
	xconf->rtr_id.s_addr = htonl(0x0a000001);
	xconf->rdomain = 0;
	xconf->fib_priority_internal = 48;
	xconf->fib_priority_external = 52;
	xconf->fib_priority_summary = 16;

	SIMPLEQ_INIT(&eigrp->redist_list);
	TAILQ_INIT(&eigrp->ei_list);
	RB_INIT(&eigrp->nbrs);
	RB_INIT(&eigrp->topology);
	eigrp->af = AF_INET;
	eigrp->as = 1;
	eigrp->kvalues[0] = 1;
	eigrp->kvalues[2] = 1;
	eigrp->active_timeout = DEFAULT_ACTIVE_TIMEOUT;
	eigrp->maximum_hops = DEFAULT_MAXIMUM_HOPS;
	eigrp->maximum_paths = DEFAULT_MAXIMUM_PATHS;
	eigrp->variance = DEFAULT_VARIANCE;

	TAILQ_INIT(&iface0->ei_list);
	TAILQ_INIT(&iface0->addr_list);
	iface0->ifindex = 1;
	iface0->mtu = 1500;
	iface0->type = IF_TYPE_BROADCAST;
	iface0->baudrate = 1000000000ULL;
	iface0->flags = IFF_UP;
	iface0->linkstate = 1;

	ei0->eigrp = eigrp;
	ei0->iface = iface0;
	ei0->state = IF_STA_ACTIVE;
	ei0->ifaceid = 1;
	ei0->delay = DEFAULT_DELAY;
	ei0->bandwidth = DEFAULT_BANDWIDTH;
	ei0->hello_holdtime = DEFAULT_HELLO_HOLDTIME;
	ei0->hello_interval = DEFAULT_HELLO_INTERVAL;
	ei0->splithorizon = 1;
	TAILQ_INIT(&ei0->nbr_list);
	TAILQ_INIT(&ei0->update_list);
	TAILQ_INIT(&ei0->query_list);
	TAILQ_INIT(&ei0->summary_list);

	TAILQ_INSERT_TAIL(&xconf->instances, eigrp, entry);
	TAILQ_INSERT_TAIL(&xconf->iface_list, iface0, entry);
	TAILQ_INSERT_TAIL(&eigrp->ei_list, ei0, e_entry);
	TAILQ_INSERT_TAIL(&iface0->ei_list, ei0, i_entry);
	eigrpd_conf = xconf;
#endif

	log_init(debug);
	log_verbose(global.cmd_opts & EIGRPD_OPT_VERBOSE);

#ifndef EIGRPD_PRIV_ONLY
	if (!debug)
		daemon(1, 0);
#endif

	log_info("startup");

	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,
	    PF_UNSPEC, pipe_parent2eigrpe) == -1)
		fatal("socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,
	    PF_UNSPEC, pipe_parent2rde) == -1)
		fatal("socketpair");

#ifndef EIGRPD_PRIV_ONLY
	/* start children */
	rde_pid = start_child(PROC_RDE_ENGINE, saved_argv0, pipe_parent2rde[1],
	    debug, global.cmd_opts & EIGRPD_OPT_VERBOSE, NULL);
	eigrpe_pid = start_child(PROC_EIGRP_ENGINE, saved_argv0,
	    pipe_parent2eigrpe[1], debug, global.cmd_opts & EIGRPD_OPT_VERBOSE,
	    sockname);
#endif

	event_init();

#ifndef EIGRPD_PRIV_ONLY
	/* setup signal handler */
	signal_set(&ev_sigint, SIGINT, main_sig_handler, NULL);
	signal_set(&ev_sigterm, SIGTERM, main_sig_handler, NULL);
	signal_set(&ev_sighup, SIGHUP, main_sig_handler, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sighup, NULL);
	signal(SIGPIPE, SIG_IGN);
#endif

	/* setup pipes to children */
#ifdef EIGRPD_PRIV_ONLY
	/*
	 * Keep the child ends open in this single-process harness.  The message
	 * generator writes to those ends below, while the original parent event
	 * handlers read from the parent ends.
	 */
#endif
	if ((iev_eigrpe = malloc(sizeof(struct imsgev))) == NULL ||
	    (iev_rde = malloc(sizeof(struct imsgev))) == NULL)
		fatal(NULL);
	if (imsgbuf_init(&iev_eigrpe->ibuf, pipe_parent2eigrpe[0]) == -1)
		fatal(NULL);
	imsgbuf_allow_fdpass(&iev_eigrpe->ibuf);
	iev_eigrpe->handler = main_dispatch_eigrpe;
	if (imsgbuf_init(&iev_rde->ibuf, pipe_parent2rde[0]) == -1)
		fatal(NULL);
	imsgbuf_allow_fdpass(&iev_rde->ibuf);
	iev_rde->handler = main_dispatch_rde;

	/* setup event handler */
	iev_eigrpe->events = EV_READ;
	event_set(&iev_eigrpe->ev, iev_eigrpe->ibuf.fd, iev_eigrpe->events,
	    iev_eigrpe->handler, iev_eigrpe);
	event_add(&iev_eigrpe->ev, NULL);

	iev_rde->events = EV_READ;
	event_set(&iev_rde->ev, iev_rde->ibuf.fd, iev_rde->events,
	    iev_rde->handler, iev_rde);
	event_add(&iev_rde->ev, NULL);

	if (main_imsg_send_ipc_sockets(&iev_eigrpe->ibuf, &iev_rde->ibuf))
		fatal("could not establish imsg links");

	main_imsg_send_config(eigrpd_conf);

	/* notify eigrpe about existing interfaces and addresses */
	kif_redistribute();

	if (kr_init(!(eigrpd_conf->flags & EIGRPD_FLAG_NO_FIB_UPDATE),
	    eigrpd_conf->rdomain) == -1)
		fatalx("kr_init failed");

#ifndef EIGRPD_PRIV_ONLY
	if (pledge("stdio rpath inet sendfd", NULL) == -1)
		fatal("pledge");
#endif

#ifdef EIGRPD_PRIV_ONLY
	/*
	 * Privileged-compartment harness.  Parent initialization is complete;
	 * now stdin is decoded as child-to-parent imsg traffic and injected
	 * through the same socketpairs used by the real eigrpe and rde children.
	 */
	if (imsgbuf_init(&eigrpe_fuzz_ibuf, pipe_parent2eigrpe[1]) == -1 ||
	    imsgbuf_init(&rde_fuzz_ibuf, pipe_parent2rde[1]) == -1)
		fatal(NULL);

	/*
	 * Two endpoints are modeled: eigrpe and rde.  Map the complete
	 * production enum plus one invalid value; the parent dispatchers decide
	 * which messages each sender may use.
	 */
	if (msg_interface_init(&fuzz_iface, STDIN_FILENO, 2,
	    IMSG_INVALID - IMSG_CTL_RELOAD + 1) !=
	    MSG_GEN_SUCCESS)
		fatalx("msg_interface_init failed");
	msg_interface_set_endpoint(&fuzz_iface, 0, &eigrpe_fuzz_ibuf);
	msg_interface_set_endpoint(&fuzz_iface, 1, &rde_fuzz_ibuf);
	msg_interface_set_eom_type(&fuzz_iface, IMSG_EOM);
	if (msg_interface_set_message_types_batch(&fuzz_iface, 0,
	    IMSG_INVALID - IMSG_CTL_RELOAD + 1, IMSG_CTL_RELOAD) !=
	    MSG_GEN_SUCCESS)
		fatalx("setting message types failed");

	while (msg_generate(&fuzz_iface, &fuzz_msg) == MSG_GEN_SUCCESS) {
		struct imsgbuf	*target = fuzz_msg.endpoint;
		void		*payload = fuzz_msg.payload;
		uint16_t	 payload_size = fuzz_msg.actual_payload_size;

		/*
		 * The parent dispatchers do not consume fds from child messages.
		 * Discard any fd produced by the generic generator and keep the
		 * injected protocol on the child-to-parent message surface.
		 */
		if (fuzz_msg.fd != -1) {
			close(fuzz_msg.fd);
			fuzz_msg.fd = -1;
			fuzz_msg.has_fd = 0;
		}
		if (target == NULL)
			continue;
		/*
		 * These bytes are synthetic harness input.  Skip them so the
		 * checker reports leaks from privileged output, not pointer-like
		 * values that AFL placed in the incoming payload.
		 */
		if (payload != NULL && payload_size > 0)
			ptr_check_skip(payload, payload_size);
		imsg_compose(target, fuzz_msg.type, 0, 0, -1, payload,
		    payload_size);
		imsgbuf_flush(target);
	}

	/*
	 * Stdin is exhausted.  Send one EOM to each modeled child endpoint so
	 * the normal event loop can drain pending messages and exit cleanly.
	 */
	for (i = 0; i < fuzz_iface.num_compartments; i++) {
		struct imsgbuf	*target;

		if (msg_generate_eom(&fuzz_iface, i, &fuzz_eom) !=
		    MSG_GEN_SUCCESS)
			continue;
		target = fuzz_eom.endpoint;
		if (target == NULL)
			continue;
		imsg_compose(target, fuzz_eom.type, 0, 0, -1, NULL, 0);
		imsgbuf_flush(target);
	}
#endif

	event_dispatch();

#ifndef EIGRPD_PRIV_ONLY
	eigrpd_shutdown();
	/* NOTREACHED */
#endif
	return (0);
}

#ifndef EIGRPD_PRIV_ONLY
static __dead void
eigrpd_shutdown(void)
{
	pid_t		 pid;
	int		 status;

	/* close pipes */
	imsgbuf_clear(&iev_eigrpe->ibuf);
	close(iev_eigrpe->ibuf.fd);
	imsgbuf_clear(&iev_rde->ibuf);
	close(iev_rde->ibuf.fd);

	kr_shutdown();
	config_clear(eigrpd_conf, PROC_MAIN);

	log_debug("waiting for children to terminate");
	do {
		pid = wait(&status);
		if (pid == -1) {
			if (errno != EINTR && errno != ECHILD)
				fatal("wait");
		} else if (WIFSIGNALED(status))
			log_warnx("%s terminated; signal %d",
			    (pid == rde_pid) ? "route decision engine" :
			    "eigrp engine", WTERMSIG(status));
	} while (pid != -1 || (pid == -1 && errno == EINTR));

	free(iev_eigrpe);
	free(iev_rde);

	log_info("terminating");
	exit(0);
}

static pid_t
start_child(enum eigrpd_process p, char *argv0, int fd, int debug, int verbose,
    char *sockname)
{
	char	*argv[7];
	int	 argc = 0;
	pid_t	 pid;

	switch (pid = fork()) {
	case -1:
		fatal("cannot fork");
	case 0:
		break;
	default:
		close(fd);
		return (pid);
	}

	if (fd != 3) {
		if (dup2(fd, 3) == -1)
			fatal("cannot setup imsg fd");
	} else if (fcntl(fd, F_SETFD, 0) == -1)
		fatal("cannot setup imsg fd");

	argv[argc++] = argv0;
	switch (p) {
	case PROC_MAIN:
		fatalx("Can not start main process");
	case PROC_RDE_ENGINE:
		argv[argc++] = "-R";
		break;
	case PROC_EIGRP_ENGINE:
		argv[argc++] = "-E";
		break;
	}
	if (debug)
		argv[argc++] = "-d";
	if (verbose)
		argv[argc++] = "-v";
	if (sockname) {
		argv[argc++] = "-s";
		argv[argc++] = sockname;
	}
	argv[argc++] = NULL;

	execvp(argv0, argv);
	fatal("execvp");
}
#endif

/* imsg handling */
static void
main_dispatch_eigrpe(int fd, short event, void *bula)
{
	struct imsgev		*iev = bula;
	struct imsgbuf		*ibuf;
	struct imsg		 imsg;
	ssize_t			 n;
	int			 shut = 0, verbose;

	ibuf = &iev->ibuf;

	if (event & EV_READ) {
		if ((n = imsgbuf_read(ibuf)) == -1)
			fatal("imsgbuf_read error");
		if (n == 0)	/* connection closed */
			shut = 1;
	}
	if (event & EV_WRITE) {
		if (imsgbuf_write(ibuf) == -1) {
			if (errno == EPIPE)	/* connection closed */
				shut = 1;
			else
				fatal("imsgbuf_write");
		}
	}

	for (;;) {
		if ((n = imsg_get(ibuf, &imsg)) == -1)
			fatal("imsg_get");

		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_CTL_RELOAD:
#ifndef EIGRPD_PRIV_ONLY
			if (eigrp_reload() == -1)
				log_warnx("configuration reload failed");
			else
				log_debug("configuration reloaded");
#else
			log_debug("main_dispatch_eigrpe: IMSG_CTL_RELOAD");
#endif
			break;
		case IMSG_CTL_FIB_COUPLE:
			kr_fib_couple();
			break;
		case IMSG_CTL_FIB_DECOUPLE:
			kr_fib_decouple();
			break;
		case IMSG_CTL_KROUTE:
			kr_show_route(&imsg);
			break;
		case IMSG_CTL_IFINFO:
			if (imsg.hdr.len == IMSG_HEADER_SIZE)
				kr_ifinfo(NULL, imsg.hdr.pid);
			else if (imsg.hdr.len == IMSG_HEADER_SIZE + IFNAMSIZ)
				kr_ifinfo(imsg.data, imsg.hdr.pid);
			else
				log_warnx("IFINFO request with wrong len");
			break;
		case IMSG_CTL_LOG_VERBOSE:
			/* already checked by eigrpe */
			memcpy(&verbose, imsg.data, sizeof(verbose));
			log_verbose(verbose);
			break;
#ifdef EIGRPD_PRIV_ONLY
		case IMSG_EOM:
			eom_counter_inc();
			break;
#endif
		default:
			log_debug("%s: error handling imsg %d", __func__,
			    imsg.hdr.type);
			break;
		}
		imsg_free(&imsg);
	}
	if (!shut)
		imsg_event_add(iev);
	else {
		/* this pipe is dead, so remove the event handler */
		event_del(&iev->ev);
		event_loopexit(NULL);
	}
}

static void
main_dispatch_rde(int fd, short event, void *bula)
{
	struct imsgev	*iev = bula;
	struct imsgbuf  *ibuf;
	struct imsg	 imsg;
	ssize_t		 n;
	int		 shut = 0;

	ibuf = &iev->ibuf;

	if (event & EV_READ) {
		if ((n = imsgbuf_read(ibuf)) == -1)
			fatal("imsgbuf_read error");
		if (n == 0)	/* connection closed */
			shut = 1;
	}
	if (event & EV_WRITE) {
		if (imsgbuf_write(ibuf) == -1) {
			if (errno == EPIPE)	/* connection closed */
				shut = 1;
			else
				fatal("imsgbuf_write");
		}
	}

	for (;;) {
		if ((n = imsg_get(ibuf, &imsg)) == -1)
			fatal("imsg_get");

		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_KROUTE_CHANGE:
			if (imsg.hdr.len - IMSG_HEADER_SIZE !=
			    sizeof(struct kroute))
				fatalx("invalid size of IMSG_KROUTE_CHANGE");
			if (kr_change(imsg.data))
				log_warnx("%s: error changing route", __func__);
			break;
		case IMSG_KROUTE_DELETE:
			if (imsg.hdr.len - IMSG_HEADER_SIZE !=
			    sizeof(struct kroute))
				fatalx("invalid size of IMSG_KROUTE_DELETE");
			if (kr_delete(imsg.data))
				log_warnx("%s: error deleting route", __func__);
			break;
#ifdef EIGRPD_PRIV_ONLY
		case IMSG_EOM:
			eom_counter_inc();
			break;
#endif

		default:
			log_debug("%s: error handling imsg %d", __func__,
			    imsg.hdr.type);
			break;
		}
		imsg_free(&imsg);
	}
	if (!shut)
		imsg_event_add(iev);
	else {
		/* this pipe is dead, so remove the event handler */
		event_del(&iev->ev);
		event_loopexit(NULL);
	}
}

int
main_imsg_compose_eigrpe(int type, pid_t pid, void *data, uint16_t datalen)
{
	if (iev_eigrpe == NULL)
		return (-1);
	return (imsg_compose_event(iev_eigrpe, type, 0, pid, -1, data, datalen));
}

int
main_imsg_compose_rde(int type, pid_t pid, void *data, uint16_t datalen)
{
	if (iev_rde == NULL)
		return (-1);
	return (imsg_compose_event(iev_rde, type, 0, pid, -1, data, datalen));
}

void
imsg_event_add(struct imsgev *iev)
{
	iev->events = EV_READ;
	if (imsgbuf_queuelen(&iev->ibuf) > 0)
		iev->events |= EV_WRITE;

	event_del(&iev->ev);
	event_set(&iev->ev, iev->ibuf.fd, iev->events, iev->handler, iev);
	event_add(&iev->ev, NULL);
}

int
imsg_compose_event(struct imsgev *iev, uint16_t type, uint32_t peerid,
    pid_t pid, int fd, void *data, uint16_t datalen)
{
	int	ret;

	if ((ret = imsg_compose(&iev->ibuf, type, peerid,
	    pid, fd, data, datalen)) != -1)
		imsg_event_add(iev);
	return (ret);
}

static int
main_imsg_send_ipc_sockets(struct imsgbuf *eigrpe_buf, struct imsgbuf *rde_buf)
{
	int pipe_eigrpe2rde[2];

	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,
	    PF_UNSPEC, pipe_eigrpe2rde) == -1)
		return (-1);

	if (imsg_compose(eigrpe_buf, IMSG_SOCKET_IPC, 0, 0, pipe_eigrpe2rde[0],
	    NULL, 0) == -1)
		return (-1);
	if (imsg_compose(rde_buf, IMSG_SOCKET_IPC, 0, 0, pipe_eigrpe2rde[1],
	    NULL, 0) == -1)
		return (-1);

	return (0);
}

struct eigrp *
eigrp_find(struct eigrpd_conf *xconf, int af, uint16_t as)
{
	struct eigrp	*eigrp;

	TAILQ_FOREACH(eigrp, &xconf->instances, entry)
		if (eigrp->af == af && eigrp->as == as)
			return (eigrp);

	return (NULL);
}

/*
 * These structures contain process-local pointers.  Sending them directly
 * would disclose addresses from the privileged process and weaken ASLR.
 * Build zeroed messages containing only non-pointer fields.  Source objects
 * are allocated with calloc, so copying those fields is safe.
 *
 * FUZZ NOTE (kept deliberately -- see KEPT_SECURITY_FIXES.md): unlike the
 * other maskable fixes, this sanitization is REQUIRED for fuzzing to start.
 * main_imsg_send_config() runs at startup, before any fuzzer input, and the
 * asan detector build (ptr_checker ENABLE_PTR_CHECK=1) aborts on the first
 * pointer seen on an outbound imsg.  Without this copy the raw config send
 * trips the detector on every execution, so AFL never gets a non-crashing
 * seed and the campaign cannot begin (it would hide the input-triggered CIVs
 * behind an unconditional startup abort).
 */
static void
main_copy_config(struct eigrpd_conf *to, const struct eigrpd_conf *from)
{
	memset(to, 0, sizeof(*to));
	to->rtr_id = from->rtr_id;
	to->rdomain = from->rdomain;
	to->fib_priority_internal = from->fib_priority_internal;
	to->fib_priority_external = from->fib_priority_external;
	to->fib_priority_summary = from->fib_priority_summary;
	to->flags = from->flags;
}

static void
main_copy_instance(struct eigrp *to, const struct eigrp *from)
{
	memset(to, 0, sizeof(*to));
	to->af = from->af;
	to->as = from->as;
	memcpy(to->kvalues, from->kvalues, sizeof(to->kvalues));
	to->active_timeout = from->active_timeout;
	to->maximum_hops = from->maximum_hops;
	to->maximum_paths = from->maximum_paths;
	to->variance = from->variance;
	to->seq_num = from->seq_num;
	to->stats = from->stats;
}

static void
main_copy_iface(struct iface *to, const struct iface *from)
{
	memset(to, 0, sizeof(*to));
	to->ifindex = from->ifindex;
	to->rdomain = from->rdomain;
	memcpy(to->name, from->name, sizeof(to->name));
	to->linklocal = from->linklocal;
	to->mtu = from->mtu;
	to->type = from->type;
	to->if_type = from->if_type;
	to->baudrate = from->baudrate;
	to->flags = from->flags;
	to->linkstate = from->linkstate;
	to->group_count_v4 = from->group_count_v4;
	to->group_count_v6 = from->group_count_v6;
}

static void
main_copy_eigrp_iface(struct eigrp_iface *to,
    const struct eigrp_iface *from)
{
	memset(to, 0, sizeof(*to));
	to->state = from->state;
	to->ifaceid = from->ifaceid;
	/* hello_timer: process-local libevent handle, meaningless to the children */
	to->delay = from->delay;
	to->bandwidth = from->bandwidth;
	to->hello_holdtime = from->hello_holdtime;
	to->hello_interval = from->hello_interval;
	to->splithorizon = from->splithorizon;
	to->passive = from->passive;
	to->uptime = from->uptime;
}

static int
main_imsg_send_config(struct eigrpd_conf *xconf)
{
	struct eigrp		*eigrp;
	struct eigrp_iface	*ei;
	struct eigrpd_conf	 xconf_san;
	struct eigrp		 eigrp_san;
	struct iface		 iface_san;
	struct eigrp_iface	 ei_san;

	/* Sanitize configuration structures before sending them to the children. */
	main_copy_config(&xconf_san, xconf);
	if (eigrp_sendboth(IMSG_RECONF_CONF, &xconf_san,
	    sizeof(xconf_san)) == -1)
		return (-1);

	TAILQ_FOREACH(eigrp, &xconf->instances, entry) {
		main_copy_instance(&eigrp_san, eigrp);
		if (eigrp_sendboth(IMSG_RECONF_INSTANCE, &eigrp_san,
		    sizeof(eigrp_san)) == -1)
			return (-1);

		TAILQ_FOREACH(ei, &eigrp->ei_list, e_entry) {
			main_copy_iface(&iface_san, ei->iface);
			if (eigrp_sendboth(IMSG_RECONF_IFACE, &iface_san,
			    sizeof(iface_san)) == -1)
				return (-1);

			main_copy_eigrp_iface(&ei_san, ei);
			if (eigrp_sendboth(IMSG_RECONF_EIGRP_IFACE,
			    &ei_san, sizeof(ei_san)) == -1)
				return (-1);
		}
	}

	if (eigrp_sendboth(IMSG_RECONF_END, NULL, 0) == -1)
		return (-1);

	return (0);
}

#ifndef EIGRPD_PRIV_ONLY
static int
eigrp_reload(void)
{
	struct eigrpd_conf	*xconf;

	if ((xconf = parse_config(conffile)) == NULL)
		return (-1);

	if (main_imsg_send_config(xconf) == -1)
		return (-1);

	merge_config(eigrpd_conf, xconf, PROC_MAIN);

	return (0);
}
#endif

static int
eigrp_sendboth(enum imsg_type type, void *buf, uint16_t len)
{
	if (main_imsg_compose_eigrpe(type, 0, buf, len) == -1)
		return (-1);
	if (main_imsg_compose_rde(type, 0, buf, len) == -1)
		return (-1);
	return (0);
}

#ifndef EIGRPD_PRIV_ONLY
void
merge_config(struct eigrpd_conf *conf, struct eigrpd_conf *xconf,
    enum eigrpd_process proc)
{
	struct iface		*iface, *itmp, *xi;
	struct eigrp		*eigrp, *etmp, *xe;

	conf->rtr_id = xconf->rtr_id;
	conf->flags = xconf->flags;
	conf->rdomain= xconf->rdomain;
	conf->fib_priority_internal = xconf->fib_priority_internal;
	conf->fib_priority_external = xconf->fib_priority_external;
	conf->fib_priority_summary = xconf->fib_priority_summary;

	/* merge instances */
	TAILQ_FOREACH_SAFE(eigrp, &conf->instances, entry, etmp) {
		/* find deleted instances */
		if ((xe = eigrp_find(xconf, eigrp->af, eigrp->as)) == NULL) {
			TAILQ_REMOVE(&conf->instances, eigrp, entry);

			switch (proc) {
			case PROC_RDE_ENGINE:
				rde_instance_del(eigrp);
				break;
			case PROC_EIGRP_ENGINE:
				eigrpe_instance_del(eigrp);
				break;
			case PROC_MAIN:
				free(eigrp);
				break;
			}
		}
	}
	TAILQ_FOREACH_SAFE(xe, &xconf->instances, entry, etmp) {
		/* find new instances */
		if ((eigrp = eigrp_find(conf, xe->af, xe->as)) == NULL) {
			TAILQ_REMOVE(&xconf->instances, xe, entry);
			TAILQ_INSERT_TAIL(&conf->instances, xe, entry);

			switch (proc) {
			case PROC_RDE_ENGINE:
				rde_instance_init(xe);
				break;
			case PROC_EIGRP_ENGINE:
				eigrpe_instance_init(xe);
				break;
			case PROC_MAIN:
				break;
			}
			continue;
		}

		/* update existing instances */
		merge_instances(conf, eigrp, xe);
	}

	/* merge interfaces */
	TAILQ_FOREACH_SAFE(iface, &conf->iface_list, entry, itmp) {
		/* find deleted ifaces */
		if ((xi = if_lookup(xconf, iface->ifindex)) == NULL) {
			TAILQ_REMOVE(&conf->iface_list, iface, entry);
			free(iface);
		}
	}
	TAILQ_FOREACH_SAFE(xi, &xconf->iface_list, entry, itmp) {
		/* find new ifaces */
		if ((iface = if_lookup(conf, xi->ifindex)) == NULL) {
			TAILQ_REMOVE(&xconf->iface_list, xi, entry);
			TAILQ_INSERT_TAIL(&conf->iface_list, xi, entry);
			continue;
		}

		/* TODO update existing ifaces */
	}	

	/* resend addresses to activate new interfaces */
	if (proc == PROC_MAIN)
		kif_redistribute();

	free(xconf);
}

static void
merge_instances(struct eigrpd_conf *xconf, struct eigrp *eigrp, struct eigrp *xe)
{
	/* TODO */
}
#endif

struct eigrpd_conf *
config_new_empty(void)
{
	struct eigrpd_conf	*xconf;

	xconf = calloc(1, sizeof(*xconf));
	if (xconf == NULL)
		fatal(NULL);

	TAILQ_INIT(&xconf->instances);
	TAILQ_INIT(&xconf->iface_list);

	return (xconf);
}

#ifndef EIGRPD_PRIV_ONLY
void
config_clear(struct eigrpd_conf *conf, enum eigrpd_process proc)
{
	struct eigrpd_conf	*xconf;

	/* merge current config with an empty config */
	xconf = config_new_empty();
	merge_config(conf, xconf, proc);

	free(conf);
}
#endif
