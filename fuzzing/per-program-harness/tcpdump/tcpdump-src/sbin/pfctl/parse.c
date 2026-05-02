/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 2
#define YYMINOR 0
#define YYPATCH 20230201

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0
#define YYPREFIX "yy"

#define YYPURE 0

#line 30 "parse.y"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <net/pfvar.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <err.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#include <md5.h>

#include "pfctl_parser.h"
#include "pfctl.h"
#include "compat.h"

static struct pfctl	*pf = NULL;
static int		 debug = 0;
static u_int16_t	 returnicmpdefault =
			    (ICMP_UNREACH << 8) | ICMP_UNREACH_PORT;
static u_int16_t	 returnicmp6default =
			    (ICMP6_DST_UNREACH << 8) | ICMP6_DST_UNREACH_NOPORT;
static int		 blockpolicy = PFRULE_DROP;
static int		 default_statelock;

TAILQ_HEAD(files, file)		 files = TAILQ_HEAD_INITIALIZER(files);
static struct file {
	TAILQ_ENTRY(file)	 entry;
	FILE			*stream;
	char			*name;
	size_t			 ungetpos;
	size_t			 ungetsize;
	u_char			*ungetbuf;
	int			 eof_reached;
	int			 lineno;
	int			 errors;
} *file, *topfile;
struct file	*pushfile(const char *, int);
int		 popfile(void);
int		 check_file_secrecy(int, const char *);
int		 yyparse(void);
int		 yylex(void);
int		 yyerror(const char *, ...)
    __attribute__((__format__ (printf, 1, 2)))
    __attribute__((__nonnull__ (1)));
int		 kw_cmp(const void *, const void *);
int		 lookup(char *);
int		 igetc(void);
int		 lgetc(int);
void		 lungetc(int);
int		 findeol(void);

TAILQ_HEAD(symhead, sym)	 symhead = TAILQ_HEAD_INITIALIZER(symhead);
struct sym {
	TAILQ_ENTRY(sym)	 entry;
	int			 used;
	int			 persist;
	char			*nam;
	char			*val;
};
int		 symset(const char *, const char *, int);
char		*symget(const char *);

int		 atoul(char *, u_long *);

struct node_proto {
	u_int8_t		 proto;
	struct node_proto	*next;
	struct node_proto	*tail;
};

struct node_port {
	u_int16_t		 port[2];
	u_int8_t		 op;
	struct node_port	*next;
	struct node_port	*tail;
};

struct node_uid {
	uid_t			 uid[2];
	u_int8_t		 op;
	struct node_uid		*next;
	struct node_uid		*tail;
};

struct node_gid {
	gid_t			 gid[2];
	u_int8_t		 op;
	struct node_gid		*next;
	struct node_gid		*tail;
};

struct node_icmp {
	u_int16_t		 code;	/* aux. value 256 is legit */
	u_int16_t		 type;	/* aux. value 256 is legit */
	u_int8_t		 proto;
	struct node_icmp	*next;
	struct node_icmp	*tail;
};

enum	{ PF_STATE_OPT_MAX, PF_STATE_OPT_NOSYNC, PF_STATE_OPT_SRCTRACK,
	    PF_STATE_OPT_MAX_SRC_STATES, PF_STATE_OPT_MAX_SRC_CONN,
	    PF_STATE_OPT_MAX_SRC_CONN_RATE, PF_STATE_OPT_MAX_SRC_NODES,
	    PF_STATE_OPT_OVERLOAD, PF_STATE_OPT_STATELOCK,
	    PF_STATE_OPT_TIMEOUT, PF_STATE_OPT_SLOPPY,
	    PF_STATE_OPT_PFLOW };

enum	{ PF_SRCTRACK_NONE, PF_SRCTRACK, PF_SRCTRACK_GLOBAL, PF_SRCTRACK_RULE };

struct node_state_opt {
	int			 type;
	union {
		u_int32_t	 max_states;
		u_int32_t	 max_src_states;
		u_int32_t	 max_src_conn;
		struct {
			u_int32_t	limit;
			u_int32_t	seconds;
		}		 max_src_conn_rate;
		struct {
			u_int8_t	flush;
			char		tblname[PF_TABLE_NAME_SIZE];
		}		 overload;
		u_int32_t	 max_src_nodes;
		u_int8_t	 src_track;
		u_int32_t	 statelock;
		struct {
			int		number;
			u_int32_t	seconds;
		}		 timeout;
	}			 data;
	struct node_state_opt	*next;
	struct node_state_opt	*tail;
};

struct peer {
	struct node_host	*host;
	struct node_port	*port;
};

struct node_queue {
	char			 queue[PF_QNAME_SIZE];
	char			 parent[PF_QNAME_SIZE];
	char			 ifname[IFNAMSIZ];
	int			 scheduler;
	struct node_queue	*next;
	struct node_queue	*tail;
};

struct node_qassign {
	char		*qname;
	char		*pqname;
};

struct range {
	int		 a;
	int		 b;
	int		 t;
};
struct redirection {
	struct node_host	*host;
	struct range		 rport;
};

struct pool_opts {
	int			 marker;
#define POM_TYPE		0x01
#define POM_STICKYADDRESS	0x02
	u_int8_t		 opts;
	int			 type;
	int			 staticport;
	struct pf_poolhashkey	*key;

} pool_opts;

struct divertspec {
	struct node_host	*addr;
	u_int16_t		 port;
	enum pf_divert_types	 type;
};

struct redirspec {
	struct redirection	*rdr;
	struct pool_opts	 pool_opts;
	int			 binat;
	int			 af;
};

struct filter_opts {
	int			 marker;
#define FOM_FLAGS	0x0001
#define FOM_ICMP	0x0002
#define FOM_TOS		0x0004
#define FOM_KEEP	0x0008
#define FOM_SRCTRACK	0x0010
#define FOM_MINTTL	0x0020
#define FOM_MAXMSS	0x0040
#define FOM_AFTO	0x0080
#define FOM_SETTOS	0x0100
#define FOM_SCRUB_TCP	0x0200
#define FOM_SETPRIO	0x0400
#define FOM_ONCE	0x1000
#define FOM_PRIO	0x2000
#define FOM_SETDELAY	0x4000
	struct node_uid		*uid;
	struct node_gid		*gid;
	struct node_if		*rcv;
	struct {
		u_int8_t	 b1;
		u_int8_t	 b2;
		u_int16_t	 w;
		u_int16_t	 w2;
	} flags;
	struct node_icmp	*icmpspec;
	u_int32_t		 tos;
	u_int32_t		 prob;
	struct {
		int			 action;
		struct node_state_opt	*options;
	} keep;
	int			 fragment;
	int			 allowopts;
	char			*label;
	struct node_qassign	 queues;
	char			*tag;
	char			*match_tag;
	u_int8_t		 match_tag_not;
	u_int			 rtableid;
	u_int8_t		 prio;
	u_int8_t		 set_prio[2];
	u_int16_t		 delay;
	struct divertspec	 divert;
	struct redirspec	 nat;
	struct redirspec	 rdr;
	struct redirspec	 rroute;
	u_int8_t		 rt;

	/* scrub opts */
	int			 nodf;
	int			 minttl;
	int			 settos;
	int			 randomid;
	int			 max_mss;

	struct {
		u_int32_t	limit;
		u_int32_t	seconds;
	}			 pktrate;
} filter_opts;

struct antispoof_opts {
	char			*label;
	u_int			 rtableid;
} antispoof_opts;

struct scrub_opts {
	int			marker;
	int			nodf;
	int			minttl;
	int			maxmss;
	int			randomid;
	int			reassemble_tcp;
} scrub_opts;

struct node_sc {
	struct node_queue_bw	m1;
	u_int			d;
	struct node_queue_bw	m2;
};

struct node_fq {
	u_int			flows;
	u_int			quantum;
	u_int			target;
	u_int			interval;
};

struct queue_opts {
	int		 marker;
#define	QOM_BWSPEC	0x01
#define	QOM_PARENT	0x02
#define	QOM_DEFAULT	0x04
#define	QOM_QLIMIT	0x08
#define	QOM_FLOWS	0x10
#define	QOM_QUANTUM	0x20
	struct node_sc	 realtime;
	struct node_sc	 linkshare;
	struct node_sc	 upperlimit;
	struct node_fq	 flowqueue;
	char		*parent;
	int		 flags;
	u_int		 qlimit;
} queue_opts;

struct table_opts {
	int			flags;
	int			init_addr;
	struct node_tinithead	init_nodes;
} table_opts;

struct node_hfsc_opts	 hfsc_opts;
struct node_state_opt	*keep_state_defaults = NULL;
struct pfctl_watermarks	 syncookie_opts;

int		 validate_range(u_int8_t, u_int16_t, u_int16_t);
int		 disallow_table(struct node_host *, const char *);
int		 disallow_urpf_failed(struct node_host *, const char *);
int		 disallow_alias(struct node_host *, const char *);
int		 rule_consistent(struct pf_rule *);
int		 process_tabledef(char *, struct table_opts *, int);
void		 expand_label_str(char *, size_t, const char *, const char *);
void		 expand_label_if(const char *, char *, size_t, const char *);
void		 expand_label_addr(const char *, char *, size_t, u_int8_t,
		    struct node_host *);
void		 expand_label_port(const char *, char *, size_t,
		    struct node_port *);
void		 expand_label_proto(const char *, char *, size_t, u_int8_t);
void		 expand_label(char *, size_t, const char *, u_int8_t,
		    struct node_host *, struct node_port *, struct node_host *,
		    struct node_port *, u_int8_t);
int		 expand_divertspec(struct pf_rule *, struct divertspec *);
int		 collapse_redirspec(struct pf_pool *, struct pf_rule *,
		    struct redirspec *rs, int);
int		 apply_redirspec(struct pf_pool *, struct pf_rule *,
		    struct redirspec *, int, struct node_port *);
void		 expand_rule(struct pf_rule *, int, struct node_if *,
		    struct redirspec *, struct redirspec *, struct redirspec *,
		    struct node_proto *,
		    struct node_os *, struct node_host *, struct node_port *,
		    struct node_host *, struct node_port *, struct node_uid *,
		    struct node_gid *, struct node_if *, struct node_icmp *);
int		 expand_queue(char *, struct node_if *, struct queue_opts *);
int		 expand_skip_interface(struct node_if *);

int	 getservice(char *);
int	 rule_label(struct pf_rule *, char *);

void	 mv_rules(struct pf_ruleset *, struct pf_ruleset *);
void	 mv_tables(struct pfctl *, struct pfr_ktablehead *,
		    struct pf_anchor *, struct pf_anchor *);
void	 decide_address_family(struct node_host *, sa_family_t *);
int	 invalid_redirect(struct node_host *, sa_family_t);
u_int16_t parseicmpspec(char *, sa_family_t);
int	 kw_casecmp(const void *, const void *);
int	 map_tos(char *string, int *);
int	 lookup_rtable(u_int);
int	 filteropts_to_rule(struct pf_rule *, struct filter_opts *);

TAILQ_HEAD(loadanchorshead, loadanchors)
    loadanchorshead = TAILQ_HEAD_INITIALIZER(loadanchorshead);

struct loadanchors {
	TAILQ_ENTRY(loadanchors)	 entries;
	char				*anchorname;
	char				*filename;
};

typedef struct {
	union {
		int64_t			 number;
		double			 probability;
		int			 i;
		char			*string;
		u_int			 rtableid;
		u_int16_t		 weight;
		struct {
			u_int8_t	 b1;
			u_int8_t	 b2;
			u_int16_t	 w;
			u_int16_t	 w2;
		}			 b;
		struct range		 range;
		struct node_if		*interface;
		struct node_proto	*proto;
		struct node_icmp	*icmp;
		struct node_host	*host;
		struct node_os		*os;
		struct node_port	*port;
		struct node_uid		*uid;
		struct node_gid		*gid;
		struct node_state_opt	*state_opt;
		struct peer		 peer;
		struct {
			struct peer	 src, dst;
			struct node_os	*src_os;
		}			 fromto;
		struct redirection	*redirection;
		struct {
			int			 action;
			struct node_state_opt	*options;
		}			 keep_state;
		struct {
			u_int8_t	 log;
			u_int8_t	 logif;
			u_int8_t	 quick;
		}			 logquick;
		struct {
			int		 neg;
			char		*name;
		}			 tagged;
		struct pf_poolhashkey	*hashkey;
		struct node_queue	*queue;
		struct node_queue_opt	 queue_options;
		struct node_queue_bw	 queue_bwspec;
		struct node_qassign	 qassign;
		struct node_sc		 sc;
		struct filter_opts	 filter_opts;
		struct antispoof_opts	 antispoof_opts;
		struct queue_opts	 queue_opts;
		struct scrub_opts	 scrub_opts;
		struct table_opts	 table_opts;
		struct pool_opts	 pool_opts;
		struct node_hfsc_opts	 hfsc_opts;
		struct pfctl_watermarks	*watermarks;
	} v;
	int lineno;
} YYSTYPE;

#define PPORT_RANGE	1
#define PPORT_STAR	2
int	parseport(char *, struct range *r, int);

#define DYNIF_MULTIADDR(addr) ((addr).type == PF_ADDR_DYNIFTL && \
	(!((addr).iflags & PFI_AFLAG_NOALIAS) ||		 \
	!isdigit((unsigned char)(addr).v.ifname[strlen((addr).v.ifname)-1])))

#line 463 "parse.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

#if !(defined(yylex) || defined(YYSTATE))
int YYLEX_DECL();
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();

#define PASS 257
#define BLOCK 258
#define MATCH 259
#define SCRUB 260
#define RETURN 261
#define IN 262
#define OS 263
#define OUT 264
#define LOG 265
#define QUICK 266
#define ON 267
#define FROM 268
#define TO 269
#define FLAGS 270
#define RETURNRST 271
#define RETURNICMP 272
#define RETURNICMP6 273
#define PROTO 274
#define INET 275
#define INET6 276
#define ALL 277
#define ANY 278
#define ICMPTYPE 279
#define ICMP6TYPE 280
#define CODE 281
#define KEEP 282
#define MODULATE 283
#define STATE 284
#define PORT 285
#define BINATTO 286
#define NODF 287
#define MINTTL 288
#define ERROR 289
#define ALLOWOPTS 290
#define FILENAME 291
#define ROUTETO 292
#define DUPTO 293
#define REPLYTO 294
#define NO 295
#define LABEL 296
#define NOROUTE 297
#define URPFFAILED 298
#define FRAGMENT 299
#define USER 300
#define GROUP 301
#define MAXMSS 302
#define MAXIMUM 303
#define TTL 304
#define TOS 305
#define DROP 306
#define TABLE 307
#define REASSEMBLE 308
#define ANCHOR 309
#define SYNCOOKIES 310
#define SET 311
#define OPTIMIZATION 312
#define TIMEOUT 313
#define LIMIT 314
#define LOGINTERFACE 315
#define BLOCKPOLICY 316
#define RANDOMID 317
#define SYNPROXY 318
#define FINGERPRINTS 319
#define NOSYNC 320
#define DEBUG 321
#define SKIP 322
#define HOSTID 323
#define ANTISPOOF 324
#define FOR 325
#define INCLUDE 326
#define MATCHES 327
#define BITMASK 328
#define RANDOM 329
#define SOURCEHASH 330
#define ROUNDROBIN 331
#define LEASTSTATES 332
#define STATICPORT 333
#define PROBABILITY 334
#define WEIGHT 335
#define BANDWIDTH 336
#define FLOWS 337
#define QUANTUM 338
#define QUEUE 339
#define PRIORITY 340
#define QLIMIT 341
#define RTABLE 342
#define RDOMAIN 343
#define MINIMUM 344
#define BURST 345
#define PARENT 346
#define LOAD 347
#define RULESET_OPTIMIZATION 348
#define PRIO 349
#define ONCE 350
#define DEFAULT 351
#define DELAY 352
#define STICKYADDRESS 353
#define MAXSRCSTATES 354
#define MAXSRCNODES 355
#define SOURCETRACK 356
#define GLOBAL 357
#define RULE 358
#define MAXSRCCONN 359
#define MAXSRCCONNRATE 360
#define OVERLOAD 361
#define FLUSH 362
#define SLOPPY 363
#define PFLOW 364
#define MAXPKTRATE 365
#define TAGGED 366
#define TAG 367
#define IFBOUND 368
#define FLOATING 369
#define STATEPOLICY 370
#define STATEDEFAULTS 371
#define ROUTE 372
#define DIVERTTO 373
#define DIVERTREPLY 374
#define DIVERTPACKET 375
#define NATTO 376
#define AFTO 377
#define RDRTO 378
#define RECEIVEDON 379
#define NE 380
#define LE 381
#define GE 382
#define STRING 383
#define NUMBER 384
#define PORTBINARY 385
#define YYERRCODE 256
typedef int YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  114,  123,  123,  123,  123,   18,   13,
   13,  115,  115,  115,  115,  115,  115,  115,  115,  115,
  115,  115,  115,  115,  115,  115,  115,  115,  115,   19,
  113,  129,  113,  130,  130,  132,   71,   71,   73,   73,
   74,   74,   75,   75,  120,   72,   72,  133,  133,  133,
  133,  133,  133,  135,  134,  134,  117,  118,  136,  104,
  106,  106,  105,  105,  105,  105,  105,  121,   85,   85,
   86,   86,   87,   87,  137,   96,   96,   98,   98,   97,
   97,   11,   11,  122,  138,  107,  107,  109,  109,  108,
  108,  108,  108,   54,   54,   53,   53,  119,  139,   99,
  101,  101,  100,  100,  100,  100,  100,  100,  102,  102,
  102,  102,  102,  103,  103,   89,   89,  116,  140,   90,
   90,   92,   92,   91,   91,   91,   91,   91,   91,   91,
   91,   91,   91,   91,   91,   91,   91,   91,   91,   91,
   91,   91,   91,   91,   91,   91,   91,   91,   91,   91,
   91,   91,   91,   91,   93,   93,   95,   95,   94,   94,
   94,   94,   28,   28,   14,   14,   24,   24,   24,   27,
   27,   27,   27,   27,   27,   27,   27,   27,   27,   42,
   42,   43,   43,   16,   16,   16,   81,   81,   80,   80,
   80,   80,   80,   82,   82,   83,   83,   84,   84,   84,
   84,    1,    1,    1,    2,    2,    3,    4,    4,    4,
   17,   17,   17,   33,   33,   33,   34,   34,   35,   36,
   36,   44,   44,   57,   57,   57,   58,   59,   59,   46,
   46,   47,   47,   45,   45,   45,  125,  125,   48,   48,
   48,   52,   52,   49,   49,   49,   15,   15,   50,   50,
   50,   50,   50,   50,   50,   50,    5,    5,   51,   60,
   60,   61,   61,   62,   62,   62,   29,   31,   63,   63,
   64,   64,   65,   65,   65,    8,    8,   66,   66,   67,
   67,   68,   68,   68,    9,    9,   26,   25,   25,   25,
   37,   37,   37,   37,   38,   38,   40,   40,   39,   39,
   39,   41,   41,   41,    6,    6,    7,    7,   10,   10,
   20,   20,   20,   23,   23,   76,   76,   76,   76,   21,
   21,   21,   77,   77,   78,   78,   79,   79,   79,   79,
   79,   79,   79,   79,   79,   79,   79,   79,   70,   88,
   88,   88,   30,   56,   56,   55,   55,   69,   69,   32,
   32,  142,  110,  110,  112,  112,  111,  111,  111,  111,
  111,  111,  111,  141,  124,  126,  126,  127,  128,  128,
  131,  131,   12,   12,   22,   22,   22,   22,   22,   22,
};
static const YYINT yylen[] = {                            2,
    0,    3,    2,    3,    3,    3,    3,    3,    3,    3,
    3,    4,    3,    2,    2,    3,    3,    3,    1,    0,
    1,    4,    3,    3,    3,    6,    3,    6,    3,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    4,    1,
    0,    0,    4,    3,    1,    2,    1,    1,    2,    1,
    2,    1,    1,    1,    3,    1,    0,    0,    2,    3,
    3,    3,    3,    0,    5,    0,   10,    5,    0,    2,
    3,    1,    1,    2,    2,    2,    1,    5,    2,    5,
    2,    4,    1,    3,    0,    2,    0,    2,    1,    2,
    2,    1,    0,    5,    0,    2,    0,    2,    1,    1,
    3,    4,    2,    2,    4,    2,    4,    4,    0,    2,
    2,    1,    3,    2,    1,    2,    2,    2,    0,    3,
    3,    6,    6,    1,    5,    1,    1,    8,    0,    2,
    0,    2,    1,    2,    2,    1,    1,    2,    2,    1,
    1,    1,    2,    2,    2,    3,    2,    2,    4,    1,
    3,    4,    3,    5,    8,    3,    3,    2,    2,    2,
    3,    1,    4,    1,    4,    2,    3,    1,    1,    2,
    2,    2,    2,    6,    1,    1,    1,    1,    2,    0,
    1,    1,    5,    1,    1,    4,    4,    6,    1,    1,
    1,    1,    1,    0,    1,    1,    0,    1,    0,    1,
    1,    2,    2,    1,    4,    1,    3,    1,    1,    1,
    2,    0,    2,    5,    2,    4,    2,    1,    1,    2,
    0,    1,    1,    0,    2,    5,    2,    4,    1,    1,
    1,    1,    3,    0,    2,    5,    1,    2,    4,    0,
    2,    0,    2,    1,    3,    2,    2,    0,    1,    1,
    4,    2,    4,    2,    2,    2,    2,    0,    1,    3,
    3,    3,    1,    3,    3,    2,    1,    1,    3,    1,
    4,    2,    4,    1,    2,    3,    1,    1,    1,    4,
    2,    4,    1,    2,    3,    1,    1,    1,    4,    2,
    4,    1,    2,    3,    1,    1,    1,    4,    3,    2,
    2,    5,    2,    5,    2,    4,    2,    4,    1,    3,
    3,    1,    3,    3,    1,    1,    1,    1,    1,    1,
    0,    1,    1,    1,    1,    2,    3,    3,    3,    0,
    1,    2,    3,    0,    1,    3,    2,    1,    2,    2,
    4,    5,    2,    2,    1,    1,    1,    2,    1,    1,
    3,    5,    1,    2,    4,    3,    5,    1,    3,    0,
    1,    0,    2,    0,    2,    1,    1,    1,    2,    1,
    1,    1,    1,    2,    2,    4,    2,    2,    4,    2,
    1,    0,    1,    1,    1,    1,    1,    1,    1,    1,
};
static const YYINT yydefred[] = {                         0,
    0,    0,    0,    0,  177,    0,  178,    0,    0,    0,
    0,    0,    0,    0,    0,    3,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   15,    0,    0,
    0,   13,  189,    0,    0,    0,  181,  179,    0,   56,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   14,
    0,    0,    0,  195,  196,    0,    2,    4,    5,    6,
    7,    8,    9,   10,   11,   18,   12,   17,   16,    0,
    0,    0,    0,    0,  383,  384,    0,   40,    0,   23,
    0,    0,   25,    0,    0,   27,   48,   47,   29,   32,
   31,   33,   36,   35,    0,   37,  268,  267,   30,    0,
   24,   19,  324,  325,   34,    0,  338,    0,    0,    0,
    0,    0,    0,  346,  347,    0,  345,    0,  335,    0,
  203,    0,    0,  202,  109,    0,   54,   53,   55,    0,
    0,    0,  190,  191,    0,  192,  193,    0,    0,  198,
    0,   21,   22,   39,    0,  375,    0,    0,  378,    0,
    0,   92,  213,    0,   49,  337,  339,  343,  322,  323,
  344,  340,    0,    0,  348,  381,    0,    0,  208,  210,
  209,    0,  206,  219,    0,  218,    0,    0,   83,   79,
  222,  223,    0,  108,    0,    0,   51,    0,    0,  186,
    0,  187,   94,    0,    0,    0,  247,    0,    0,    0,
    0,    0,  217,    0,    0,  336,  211,  205,    0,  220,
    0,    0,   78,    0,    0,    0,    0,    0,    0,  115,
  112,    0,   68,    0,  183,    0,    0,  100,    0,   99,
    0,    0,    0,    0,   45,  377,   26,    0,  380,   28,
    0,    0,    0,  341,    0,  207,    0,    0,   84,    0,
    0,   89,    0,  126,  127,    0,    0,  117,  118,  116,
  114,  111,    0,    0,  188,  103,    0,   98,    0,   46,
   43,    0,    0,    0,  214,    0,  215,    0,  342,   80,
    0,   81,  349,   90,   91,   88,    0,  113,    0,  230,
  231,    0,  225,  229,    0,  232,    0,    0,    0,  101,
    0,    0,    0,    0,    0,   44,  376,  379,    0,  332,
    0,    0,    0,    0,    0,  249,    0,    0,  241,    0,
  250,  128,    0,    0,    0,    0,  255,  256,    0,    0,
    0,    0,    0,  254,    0,    0,  104,  102,    0,  106,
    0,  216,   82,    0,    0,    0,    0,    0,  386,  387,
  389,    0,  385,  388,  390,    0,    0,  246,  270,  278,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  142,
    0,    0,    0,    0,    0,  141,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  162,    0,    0,    0,  150,
    0,    0,    0,    0,    0,  136,  137,  140,  133,    0,
  164,  237,    0,  235,    0,  233,    0,  266,    0,    0,
    0,    0,    0,    0,  257,    0,   64,   67,  125,    0,
    0,  226,    0,  227,    0,  275,    0,    0,    0,  245,
   69,  300,  297,    0,    0,  315,  316,    0,    0,  301,
  317,  318,    0,    0,  303,    0,    0,    0,    0,    0,
    0,    0,  158,  160,  159,  326,  143,  286,  287,    0,
    0,    0,  134,  279,  295,  296,    0,    0,    0,  135,
  288,  319,  320,  139,    0,    0,    0,    0,    0,  169,
  166,    0,  175,  176,  147,  350,    0,  144,  148,  138,
    0,  145,    0,    0,    0,    0,    0,    0,    0,  132,
    0,  243,  105,  261,  260,  262,  269,  265,  264,  107,
    0,    0,    0,    0,    0,    0,  276,  252,  251,    0,
    0,    0,  299,    0,    0,    0,    0,    0,    0,  327,
  328,    0,  354,    0,  157,    0,  374,    0,    0,  284,
    0,    0,  293,  171,  170,  173,    0,  172,  168,    0,
  329,    0,    0,    0,  151,  277,  153,    0,  156,  146,
  161,    0,    0,   58,  123,  122,  228,  271,    0,  272,
    0,  152,   73,    0,    0,    0,   77,   72,    0,  298,
    0,    0,  310,  311,    0,    0,  313,  314,    0,    0,
    0,  359,  353,  367,  368,    0,  370,  371,  372,  373,
  366,    0,    0,    0,  285,    0,    0,  294,    0,  165,
    0,  351,    0,  163,  149,    0,  238,  236,    0,    0,
    0,  253,   74,   75,   76,    0,  302,    0,  305,  304,
    0,  307,  333,    0,  355,    0,  369,  361,  365,  280,
    0,  281,  289,    0,  290,    0,  167,    0,    0,    0,
   59,   65,    0,    0,    0,    0,  273,   71,    0,    0,
  356,    0,    0,    0,    0,  352,    0,  239,   63,   61,
   62,   60,  306,  308,    0,  282,  291,  174,    0,  357,
  155,
};
static const YYINT yydgoto[] = {                          2,
  106,  252,  163,  189,  109,  449,  454,  471,  478,  484,
  311,   87,  153,  495,  347,   66,  193,  111,   89,  171,
  289,  366,  127,   17,  406,  445,   38,  490,  565,  602,
  367,  647,  274,  357,  303,  304,  407,  591,  450,  595,
  455,  145,  148,  307,  329,  308,  416,  330,  331,  459,
  345,  439,  313,  314,  601,  460,  335,  414,  573,  368,
  525,  369,  473,  613,  474,  480,  616,  481,  461,  294,
   99,   41,  112,  139,  370,  408,  540,  128,  129,   58,
  151,   59,  182,  183,  133,  257,  190,  498,  266,  332,
  409,  410,  411,  491,  560,  223,  262,  263,  194,  231,
  232,  298,  267,  531,  588,  589,  203,  240,  241,  545,
  611,  612,  154,   18,   19,   20,   21,   22,   23,   24,
   25,   26,    3,   93,  158,  209,   96,  211,  155,  244,
  177,  245,  630,  428,  521,  532,  224,  204,  195,  333,
  463,  546,
};
static const YYINT yysindex[] = {                       -42,
    0,   17, 1009,  110,    0,  392,    0,   77, -318, 1586,
 -153, -178, -160,  -71,  168,    0,  159,  260,  288,  298,
  320,  329,  422,  451,  473,  477,  486,    0,  538,  547,
  551,    0,    0,  294,  542,  550,    0,    0,  205,    0,
  159, -243,  213,  215,  -90,  -87, -251,  -58,  222, -111,
  134, -253,  247,  150, 1088,  601,  387,  331,  388,    0,
  134, -318,  143,    0,    0, -153,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  354,
  160,  296,  600,  400,    0,    0,  404,    0,    0,    0,
  311,  709,    0,  319,  709,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   46,    0,    0,    0,    0,  247,
    0,    0,    0,    0,    0,  337,    0,  356,  372, -112,
  377,  379,  716,    0,    0,  398,    0,  742,    0,  293,
    0,   60,  190,    0,    0,  485,    0,    0,    0,  143,
  134,  403,    0,    0,  369,    0,    0,  747,    0,    0,
  134,    0,    0,    0,  753,    0,  709,  411,    0,  412,
  709,    0,    0, -250,    0,    0,    0,    0,    0,    0,
    0,    0,  749,  415,    0,    0, 1088,  247,    0,    0,
    0,  386,    0,    0,  426,    0,  709, -250,    0,    0,
    0,    0,    0,    0,  393,  247,    0,  190,  768,    0,
  296,    0,    0, -105,  190,  429,    0,  709,   67,  709,
   79,  780,    0,  434,  758,    0,    0,    0,  293,    0,
   37,  783,    0, -147,  300,  439,  442,  443,  445,    0,
    0,  393,    0,  555,    0,  789,  453,    0,  709,    0,
 -105,  555,  457,  433,    0,    0,    0,  411,    0,    0,
  412,   82,  709,    0,  490,    0,   95,  709,    0,  458,
  459,    0, -147,    0,    0,  508,  742,    0,    0,    0,
    0,    0, -100, -180,    0,    0,   51,    0, -180,    0,
    0,  429,  709,  709,    0,  780,    0,  497,    0,    0,
   37,    0,    0,    0,    0,    0,  300,    0, -102,    0,
    0,  709,    0,    0,  126,    0,    0,  594,  709,    0,
  231,  526,   97,  709,    0,    0,    0,    0,  709,    0,
  709,  537,  300,  300,  310,    0,  773,  709,    0,  578,
    0,    0, 5373,  -80,  596,   70,    0,    0,  491,  425,
  822,  494,  495,    0,  823,  499,    0,    0,   70,    0,
  756,    0,    0,  504,  742,  742,  133,  709,    0,    0,
    0,  709,    0,    0,    0,  143,  503,    0,    0,    0,
   33,  773,  850,   11,  -98,  -62,  609,  613,   45,    0,
   45,   45,   45,  614,  458,    0, 1005, 1133,  318,  255,
  616,  324,  -26,  517,  530,    0,  532,  247,  527,    0,
  633,   45,  190,   45, -132,    0,    0,    0,    0, 5373,
    0,    0,  709,    0,  126,    0,  162,    0,  533,  536,
  540,  879,  860,  541,    0,  709,    0,    0,    0,  584,
  627,    0,  310,    0,  127,    0,  143,  709,  181,    0,
    0,    0,    0,  558,  887,    0,    0,  709,  661,    0,
    0,    0,  709,  662,    0,  904,  904,  709,  526,  665,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  709,
  562,  341,    0,    0,    0,    0,  709,  566,  353,    0,
    0,    0,    0,    0,  318,  -26,  -30,  573, -231,    0,
    0,  904,    0,    0,    0,    0,  581,    0,    0,    0,
  925,    0,  690,  143,    0,  708,    0,  247, -250,    0,
  599,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  976,  300,  300,  709,  188,  709,    0,    0,    0,   33,
  947,  693,    0,  558,  363,  365,  376,  391, 1088,    0,
    0,   35,    0,  143,    0,  782,    0,  153,  341,    0,
  525,  353,    0,    0,    0,    0,  605,    0,    0,  479,
    0,  572,  618,  143,    0,    0,    0,   45,    0,    0,
    0,  709,  351,    0,    0,    0,    0,    0,  127,    0,
  709,    0,    0,  619,  620,  610,    0,    0,  742,    0,
  361,  709,    0,    0,  414,  709,    0,    0,  582,  526,
  441,    0,    0,    0,    0,  247,    0,    0,    0,    0,
    0,  782,  481,  709,    0,  487,  709,    0,  742,    0,
 -231,    0,  623,    0,    0,    0,    0,    0,  599,  975,
  709,    0,    0,    0,    0,  693,    0,  363,    0,    0,
  376,    0,    0,  709,    0,   35,    0,    0,    0,    0,
  153,    0,    0,  525,    0,  625,    0,  956,  738,  709,
    0,    0, 1001, 1008, 1011, 1013,    0,    0,  709,  709,
    0,  526,  709,  709,  983,    0,   45,    0,    0,    0,
    0,    0,    0,    0,  709,    0,    0,    0,    0,    0,
    0,
};
static const YYINT yyrindex[] = {                         5,
    0,    0,    0,    0,    0, 1448,    0,    0, 1572,    0,
  700,    0,    0,    0,    0,    0, 2071,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0, 1699, 1820, 1944,    0,    0,    0,    0,
 2192,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 1018,    0,    0,    0,    0, 2316, 2587,    0, 2707,    0,
  632,  761,    0,    0,    0, 3298,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0, 2827,    0,    0, 1020,    0,  241,    0,
    0,  651,    0,    0,  651,    0,    0,    0,    0,    0,
    0,    0,    0,    0, -233,    0,    0,    0,    0, 1327,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    7,
    0,    0,    0,    0,    0,    0,    0,   86,    0,    0,
    0,    0,  138,    0,    0,    0,    0,    0,    0, 1025,
 3649,    0,    0,    0,  395,    0,    0,    0,   -1,    0,
 3418,    0,    0,    0,    0,    0,  871,    0,    0,    0,
  -25,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  382,    0,    0,    0,    0,   69,    0,    0,    0,
    0,    0,  175,    0,    0,    0,    0, 3889,    0,    0,
    0,    0,    0,    0, 3769,    0,    0,  -33,  653,  -33,
  653, -233,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, 1027,    0, 4129,    0,    0,    0,    0,  217,    0,
 1028, 4009,    0,  653,    0,    0,    0,    0,    0,    0,
    0,   26,   54,    0,  186,    0,  130,  -24,    0,    0,
    0,    0, 1041,    0,    0,  676,  227,    0,    0,    0,
    0,    0,    0, 4489,    0,    0,  277,    0, 4249,    0,
    0,    0,  -33,  -33,    0, -233,    0,  512,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  397,    0,    0,  277,    0, 5272, 4600,  615,    0,
    0,    2,  759,  200, 4840,    0,    0,    0,   54,    0,
  -24,    0,    0,    0,    0,    0,    0,  313,    0, 4369,
    0,    0,  -31,    0, 4950,  277,    0,    0,    0,  339,
    0,    0,    0,    0,  472,    0,    0,    0,  277,    0,
 1042,    0,    0,    0,  548,  723,  395,   55,    0,    0,
    0,  877,    0,    0,    0,    0,  666,    0,    0,    0,
  277,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  786,    0,    0,    0,    0,    0,    0,   12,
    0,    0,  651,    0,  277,    0,  759,    0,    0,    0,
    0,    0,    0,    0,    0,  200,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  118,  519,    0,
    0,    0,    0,    0,    0,    0,    0,  397, 1062,    0,
    0,    0,  397, 1194,    0, 5070, 5070,   76, 2466, 2938,
 3529, 3529,    0,    0,    0,    0,    0,    0,    0,  877,
  798,    0,    0,    0,    0,    0,  877,  930,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, 5070,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0, 3529,    0, 3529,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   55,  893,  111,    0,    0,    0,  277,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  338,
    0,  653,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  -33,  653,    0,    0,    0,    0,    0,    0,    0,
  118,    0,    0,    0,    0,    0,    0,    0,  -20,    0,
  395,   55,    0,    0,  395,   55,    0,    0, 1220,    9,
   84,    0,    0,    0,    0, 3058,    0,    0,    0,    0,
    0, 4720,  893,  111,    0,  893,  111,    0,  678,    0,
    0,    0,    0,    0,    0, 3178,    0,    0,    0,    0,
  111,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   42,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, 5171,  -33,
    0,    0,    0,    0,    0,    0,    0,    0,   55,   55,
    0,    9,  111,  111,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   42,    0,    0,    0, 3529,    0,
    0,
};
static const YYINT yygindex[] = {                         0,
   43,    0, -123, -158,    0,    0,    0, -402, -424,  585,
  -74,    0,    0,    0, -392, 1022, -151,    0,    0,    0,
    0, -349, 1017,    0,    0, -368,    0,    0,  509,    0,
 -283,    0,  833,    0, -288,    0,    0,    0, -495,    0,
 -477,    0,  875,  800,  667,    0,    0, -330, -125, -248,
    0,    0,  750,  734,    0,  264,    0, -487,    0,  713,
    0, -422,    0,    0, -498,    0,    0, -473, -370,  702,
    0, 1034, -106,  949,  -60,    0, -226,  563,  924, 1038,
    0, 1048,    0,  888,    0,    0,   18,  622,  820,  803,
  710,    0,    0, -460,    0,    0,  856,    0,    0,  889,
    0,    0, -267,    0,  488,    0,    0,  881,    0,  238,
  511,    0,    0,  496,    0,   -3,   -2,    0,    0,    0,
    0,  500,    0,  -51,  -93,    0,  -69,    0,    0,    0,
 -138,  843,    0,    0,    0,    0,    0,    0,    0,    0,
  402,    0,
};
#define YYTABLESIZE 5751
static const YYINT yytable[] = {                         30,
   31,  160,  140,  165,    1,  213,  201,  248,   97,  557,
  248,  258,  526,  497,    1,  248,  321,  239,  258,  248,
   70,  130,  302,  572,  448,   97,   16,  184,  559,  222,
  164,  505,   92,  507,  258,   95,  358,  472,  479,  592,
  438,  258,  413,  219,   93,  258,  234,  321,  258,  614,
  321,   85,  258,  242,  553,  355,  356,  444,  382,  596,
  453,  258,  344,  207,   40,  162,  543,  212,  258,  550,
  248,  217,  251,  485,  342,  533,  188,  617,  162,  140,
    1,  248,  436,  162,  342,  248,  248,  305,  253,  233,
  210,  248,  185,  221,  343,   38,  306,  248,  248,  188,
  248,  248,  162,  135,  343,  282,  208,  486,  248,   93,
  176,   56,   57,  286,  246,  248,  249,  487,  291,   32,
  488,   95,  176,  382,  258,  176,  258,  618,  299,  107,
  108,   98,  186,  258,  130,  248,   39,  164,  176,   86,
  176,  660,  669,  382,  524,  277,  615,  221,  260,   93,
  248,  312,  673,  527,  248,  328,  631,  248,  162,  287,
  657,  248,  319,  670,  292,  590,  248,  458,  161,  382,
  248,  248,  248,  309,  349,  310,  176,  248,  248,  248,
  674,  284,  187,  198,   87,  237,  364,  363,  365,  317,
  318,  247,  309,  205,  261,  330,  283,  626,  472,  581,
  323,  479,  100,  250,   60,  176,  285,  644,  325,  103,
  312,  164,  364,  363,  365,  336,  430,  431,  433,  290,
  350,  348,   61,  312,  176,  352,  330,  353,   63,  330,
  541,  176,  248,  508,  371,  248,  119,   62,  258,  248,
  248,  324,  248,  248,  169,  170,  509,  101,  328,  248,
   41,  506,  248,  248,  575,  576,  248,  432,  405,  248,
    1,    1,    1,    1,  434,  561,  382,  382,  435,   67,
  342,  104,    4,    5,    6,    7,  248,  238,  349,  685,
   42,  382,  300,  301,  446,  447,  513,  382,  442,   95,
  343,  502,   91,  600,  489,   94,  382,   68,  258,  258,
  530,  472,  412,  382,  479,  529,  689,   69,  321,  321,
  326,    1,  578,    1,  184,    1,   93,  248,  248,  511,
  451,  452,  248,    8,  248,    9,  321,   10,    1,   70,
    1,  248,  520,   80,   93,  405,   93,  184,   71,  248,
   11,  248,   12,    1,  528,  248,  248,   93,  259,  248,
  571,    1,  248,  556,  535,   13,  496,  248,  248,  537,
  321,  321,  321,   14,  542,  321,  321,  321,  382,  321,
  321,  259,  248,  258,  321,  321,  548,   93,  259,  185,
  258,   95,  259,  551,  258,  258,  579,    1,  382,  321,
   93,  258,  258,  443,  176,  248,  248,  672,  259,   15,
  105,  570,  185,  326,  176,  382,  339,  382,  382,  200,
  327,  248,  176,  248,  248,  248,  339,  340,  341,  186,
   64,  621,   65,  623,  248,  248,  218,  340,  341,  176,
  577,   72,  580,  221,  629,  248,  248,  248,  248,  382,
  382,  382,  186,  566,  382,  382,  382,  248,  382,  382,
  636,  248,  638,  382,  382,  382,  641,  176,  248,  248,
   73,  259,  646,  259,  191,  192,  382,  382,  382,  420,
   85,  419,  382,  281,  651,  628,  176,  654,  627,  221,
  656,  263,   74,  603,  176,  637,   75,  632,  330,  248,
  248,  248,  248,  248,  248,   76,  248,  248,  639,  648,
  248,  248,  642,  566,  263,  330,  359,  360,  361,  137,
  138,  263,  382,  248,  248,  263,   85,  113,  114,  620,
  652,  331,  176,  655,  176,  137,  138,  337,  338,  382,
  176,  263,  359,  360,  361,  468,  469,  667,  640,  330,
  330,  330,  143,  144,  330,  330,  330,   77,  330,  330,
  671,  382,  331,  330,  330,  331,   78,  121,  382,  485,
   79,  178,  119,  119,  119,  645,  678,  119,  330,  179,
  382,  248,  119,   93,   93,  683,  684,  119,  382,  686,
  687,   81,  248,  248,  364,  363,  365,   83,  248,   82,
  248,  690,  180,  486,  263,   88,  263,   90,  259,  248,
  248,  259,  339,  487,  102,  650,  488,  259,  259,  248,
  248,  653,  622,  340,  341,  176,  259,  259,  259,  181,
  259,  259,  643,  259,  259,  176,  664,  665,  259,  110,
  259,  259,  259,  259,  259,  259,  259,  259,  259,  259,
  130,  382,  382,  259,  462,  462,  462,  248,   93,  259,
  382,   56,   33,  134,  248,  132,  259,  142,  382,   93,
   93,  149,   34,   35,   36,  150,  259,  259,  259,  259,
  259,  259,  259,  259,  248,  274,  382,  259,  146,  147,
  259,  382,  264,  265,  248,  124,  382,  259,  259,  382,
  152,  259,  300,  301,  156,  248,  248,   37,  274,  547,
  482,  483,  159,  259,  259,  259,  493,  494,  382,  274,
  259,  259,  259,  259,  259,  259,  259,  259,  157,  124,
  166,  259,  259,  468,  469,  274,  274,  274,  225,  226,
  227,  263,  120,  228,  263,  475,  476,  248,  229,  167,
  263,  263,  567,  230,  569,  446,  447,  593,  594,  263,
  263,  263,  196,  263,  263,  168,  263,  263,  451,  452,
  172,  263,  173,  263,  263,  263,  263,  263,  263,  263,
  263,  263,  263,  597,  598,  174,  263,  382,  382,  248,
  248,  175,  263,  464,  465,  176,  199,  202,  274,  263,
  274,  382,  206,   91,   94,  214,  382,  215,  382,  263,
  263,  263,  263,  263,  263,  263,  263,  283,  235,  220,
  263,  243,  162,  263,  331,  382,  382,  254,  382,  255,
  263,  263,  268,  259,  263,  269,  270,  271,  273,  275,
  283,  331,  364,  363,  365,  276,  263,  263,  263,  280,
  293,  283,  295,  263,  263,  263,  263,  263,  263,  263,
  263,  288,  297,  320,  263,  263,  334,  283,  283,  283,
  346,  354,  372,  659,  415,  331,  331,  331,  421,  424,
  331,  331,  331,  418,  331,  331,  422,  423,  427,  331,
  331,  382,  425,  121,  121,  121,  429,  437,  121,  441,
  382,  382,  456,  121,  331,  362,  457,  466,  121,  492,
  499,  382,  382,  248,  359,  360,  361,  475,  476,  503,
  248,  248,  248,  500,  248,  501,  514,  504,  515,  517,
  283,  518,  283,  516,  519,  274,  691,  522,  274,  523,
  248,  248,  248,  534,  274,  274,  248,  248,  248,  292,
  443,  536,  538,  539,  274,  274,  549,  274,  274,  544,
  552,  274,  382,  382,  382,  274,  558,  274,  274,  274,
  274,  274,  292,  562,  274,  274,  274,  212,  212,  212,
  274,  563,  212,  292,  564,  568,  274,  212,  124,  583,
  584,  412,  212,  274,  661,  574,  248,  582,  619,  292,
  292,  292,  635,  248,  585,  248,  676,  248,  248,  274,
  586,  624,  633,  634,  274,  658,  677,  274,  675,  587,
  679,  124,  124,  124,  274,  274,  124,  680,   28,  124,
  681,  124,  682,  688,  199,  382,  124,  212,   57,   20,
  274,  274,  274,  248,   52,  382,  110,   96,  274,  274,
  274,  274,  274,  274,  274,  274,  274,  274,  274,  274,
   86,   66,  292,  221,  292,  382,  382,  283,  120,  120,
  120,  382,   84,  120,  364,  363,  365,  283,  120,  554,
  115,  309,  625,  120,  279,  236,  283,  283,  315,  283,
  283,  512,  426,  283,  440,  417,  467,  283,  197,  283,
  283,  283,  283,  283,  309,  136,  283,  283,  283,  662,
  216,  599,  283,  141,  131,  309,  256,  555,  283,  604,
  605,  606,  607,  608,  609,  283,  322,  351,  296,  510,
  272,  278,  649,  668,  316,  663,    0,  470,    0,  666,
  382,  283,    0,   29,  610,    0,  283,    0,    0,  283,
    0,  382,  382,    0,    0,    0,  283,  283,  248,    0,
    0,    0,  359,  360,  361,  137,  138,    0,    0,    0,
    0,    0,  283,  283,  283,    0,    0,  248,  248,    0,
  283,  283,  283,  283,  283,  283,  283,  283,  283,  283,
  283,  283,    0,    0,  309,    0,  309,    0,    0,  292,
    0,    0,  364,  363,  365,    0,    0,    0,    0,  292,
    0,    0,    0,  312,    0,    0,    0,    0,  292,  292,
    0,  292,  292,  248,    0,  292,    0,    0,    0,  292,
    0,  292,  292,  292,  292,  292,  312,    0,  292,  292,
  292,    5,    6,    7,  292,    0,    0,  312,    0,    0,
  292,    0,  248,    0,    0,    0,    0,  292,    0,    0,
  248,  248,  248,  248,  248,  477,  248,  248,  248,  248,
  248,    0,    0,  292,   27,    5,    6,    7,  292,    0,
    0,  292,  382,  382,  382,  382,  382,    0,  292,  292,
    0,    8,    0,    9,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  292,  292,  292,    0,    0,    0,
   12,    0,  292,  292,  292,  292,  292,  292,  292,  292,
  292,  292,  292,  292,    0,    0,  312,    9,  312,    0,
    0,  309,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  309,    0,    0,    0,    0,   50,    0,    0,    0,
  309,  309,    0,  309,  309,    0,    0,  309,    0,    0,
    0,  309,    0,  309,  309,  309,  309,  309,    0,   50,
  309,  309,  309,    0,    0,    0,  309,   50,    0,    0,
   50,    0,  309,    0,    0,    0,    0,    0,    0,  309,
    0,    0,    0,    0,  359,  360,  361,  468,  469,    0,
  116,    0,    0,    0,    0,  309,    0,    0,    0,    0,
  309,    0,    0,  309,    0,    0,    0,  117,    0,    0,
  309,  309,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  309,  309,  309,    0,
    0,    0,    0,    0,  309,  309,  309,  309,  309,  309,
  309,  118,  119,  120,  309,  309,  121,  122,  123,   50,
  124,  125,    0,  312,    0,  113,  114,  180,    0,    0,
    0,    0,    0,  312,    0,    0,    0,    0,    0,    0,
  126,    0,  312,  312,    0,  312,  312,    0,    0,  312,
  180,    0,    0,  312,    0,  312,  312,  312,  312,  312,
    0,    0,  312,  312,  312,    0,    0,    0,  312,    0,
    0,    0,    0,    0,  312,    0,    0,    0,    0,    0,
    0,  312,  359,  360,  361,  475,  476,    0,    0,    0,
    0,    0,  382,    0,    0,    0,    0,  312,    0,    0,
    0,    0,  312,    0,    0,  312,    0,    0,    0,  382,
    0,    0,  312,  312,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  312,  312,
  312,    0,    0,    0,    0,    0,  312,  312,  312,  312,
  312,  312,  312,  382,  382,  382,  312,  312,  382,  382,
  382,   57,  382,  382,    0,    0,   50,  382,  382,    0,
    0,    0,    0,    0,    0,   50,   50,    0,    0,    0,
    0,    0,  382,   50,   57,   50,   50,    0,   50,   50,
    0,    0,   50,    0,    0,    0,   50,    0,   50,   50,
   50,   50,   50,    0,    0,   50,   50,   50,    0,    0,
    0,   50,    0,    0,    0,    0,    0,   50,    0,    0,
    0,    0,    0,    0,   50,    0,    0,    0,    0,    0,
    0,    0,    0,   50,   50,   50,   50,   50,   50,   50,
   50,    0,    0,    0,    0,   50,    0,    0,   50,    0,
    0,    0,    0,    0,    0,   50,   50,    0,    0,   50,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   50,   50,   50,   57,    0,    0,    0,    0,   50,
   50,   50,   50,   50,   50,   50,    0,  180,  182,  180,
  180,  180,  180,  180,  180,  180,  180,  180,    0,    0,
    0,  180,  180,  180,  180,    0,  180,  180,    0,  180,
  180,  182,    0,  180,    0,    0,    0,  180,    0,  180,
  180,  180,  180,  180,    0,    0,  180,  180,  180,    0,
    0,    0,  180,    0,    0,    0,    0,    0,  180,    0,
    0,    0,    0,    0,    0,  180,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  180,    0,    0,    0,    0,  180,    0,    0,  180,
    0,    0,    0,    0,    0,    0,  180,  180,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  180,  180,  180,    0,    0,    0,    0,    0,
  180,  180,  180,  180,  180,  180,  180,    0,    0,  184,
    0,   57,    0,   57,   57,   57,    0,   57,   57,   57,
   57,   57,    0,    0,    0,   57,   57,   57,   57,    0,
   57,   57,  184,   57,   57,    0,    0,   57,    0,    0,
    0,   57,    0,   57,   57,   57,   57,   57,    0,    0,
   57,   57,   57,    0,    0,    0,   57,    0,    0,    0,
    0,    0,   57,    0,    0,    0,    0,    0,    0,   57,
    0,    0,    0,   42,    0,   43,    0,   44,   45,   46,
   47,   48,    0,    0,   49,   57,   50,   51,   52,    0,
   57,    0,    0,   57,    0,    0,    0,    0,    0,    0,
   57,   57,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   53,    0,    0,   57,   57,   57,    0,
    0,    0,    0,    0,   57,   57,   57,   57,   57,   57,
   57,    0,    0,  185,    0,   54,   55,    0,  182,    0,
  182,  182,  182,  182,  182,  182,  182,  182,  182,    0,
    0,    0,  182,  182,  182,  182,  185,  182,  182,    0,
  182,  182,    0,    0,  182,    0,    0,    0,  182,    0,
  182,  182,  182,  182,  182,    0,    0,  182,  182,  182,
    0,    0,    0,  182,    0,    0,    0,    0,    0,  182,
    0,    0,    0,    0,    0,    0,  182,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  182,    0,    0,    0,    0,  182,    0,    0,
  182,    0,    0,    0,    0,    0,    0,  182,  182,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  182,  182,  182,    0,    0,    0,    0,
    0,  182,  182,  182,  182,  182,  182,  182,    0,  184,
  194,  184,  184,  184,  184,  184,  184,  184,  184,  184,
    0,    0,    0,  184,  184,  184,  184,    0,  184,  184,
    0,  184,  184,  194,    0,  184,    0,    0,    0,  184,
    0,  184,  184,  184,  184,  184,    0,    0,  184,  184,
  184,    0,    0,    0,  184,    0,    0,    0,    0,    0,
  184,    0,    0,    0,    0,    0,    0,  184,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  184,    0,    0,    0,    0,  184,    0,
    0,  184,    0,    0,    0,    0,    0,    0,  184,  184,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  184,  184,  184,    0,    0,    0,
    0,    0,  184,  184,  184,  184,  184,  184,  184,    0,
    0,  194,    0,  185,    0,  185,  185,  185,  185,  185,
  185,  185,  185,  185,    0,    0,    0,  185,  185,  185,
  185,    0,  185,  185,  194,  185,  185,    0,    0,  185,
    0,    0,    0,  185,    0,  185,  185,  185,  185,  185,
    0,    0,  185,  185,  185,    0,    0,    0,  185,    0,
    0,    0,    0,    0,  185,    0,    0,    0,    0,    0,
    0,  185,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  185,    0,    0,
    0,    0,  185,    0,    0,  185,    0,    0,    0,    0,
    0,    0,  185,  185,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  185,  185,
  185,    0,    0,    0,  194,    0,  185,  185,  185,  185,
  185,  185,  185,    0,    0,  204,    0,    0,    0,    0,
  194,    0,    0,  194,    0,  194,  194,  194,  194,  194,
  194,    0,    0,    0,  194,  194,  194,  194,  204,  194,
  194,    0,  194,  194,    0,    0,  194,    0,    0,    0,
  194,    0,  194,  194,  194,  194,  194,    0,    0,  194,
  194,  194,    0,    0,    0,  194,    0,    0,    0,    0,
    0,  194,    0,    0,    0,    0,    0,    0,  194,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  194,    0,    0,    0,    0,  194,
    0,    0,  194,    0,    0,    0,    0,    0,    0,  194,
  194,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  194,  194,  194,    0,    0,
    0,    0,    0,  194,  194,  194,  194,  194,  194,  194,
    0,  194,    0,    0,  194,    0,    0,  194,  194,  194,
  194,  194,    0,    0,    0,  194,  194,  194,  194,    0,
  194,  194,    0,  194,  194,  258,    0,  194,    0,    0,
    0,  194,    0,  194,  194,  194,  194,  194,    0,    0,
  194,  194,  194,    0,    0,    0,  194,    0,  258,    0,
    0,    0,  194,    0,    0,    0,    0,    0,    0,  194,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  194,    0,    0,    0,    0,
  194,    0,    0,  194,    0,    0,    0,    0,    0,    0,
  194,  194,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  194,  194,  194,    0,
    0,    0,    0,    0,  194,  194,  194,  194,  194,  194,
  194,    0,    0,    0,    0,  204,    0,    0,  204,    0,
    0,  204,  204,  204,  204,  204,    0,    0,  258,  204,
  204,  204,  204,    0,  204,  204,  201,  204,  204,    0,
    0,  204,    0,    0,    0,  204,    0,  204,  204,  204,
  204,  204,    0,    0,  204,  204,  204,    0,    0,  201,
  204,    0,    0,    0,    0,    0,  204,    0,    0,    0,
    0,    0,    0,  204,    0,    0,    0,    0,    0,    0,
  204,    0,    0,    0,    0,    0,    0,    0,    0,  204,
    0,    0,    0,    0,  204,    0,    0,  204,    0,    0,
    0,    0,    0,    0,  204,  204,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  204,  204,  204,    0,    0,    0,    0,    0,  204,  204,
  204,  204,  204,  204,  204,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  200,    0,    0,    0,
    0,    0,    0,    0,    0,  258,    0,    0,    0,    0,
    0,    0,    0,    0,  258,  258,    0,    0,    0,  200,
    0,    0,    0,    0,  258,  258,    0,  258,  258,    0,
  258,  258,    0,    0,    0,  258,    0,  258,  258,  258,
  258,  258,    0,    0,  258,  258,  258,    0,    0,    0,
  258,    0,    0,    0,    0,    0,  258,    0,    0,    0,
    0,    0,    0,  258,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  258,  258,  258,  258,  258,  258,  258,
    0,    0,    0,    0,  258,    0,    0,  258,    0,    0,
    0,    0,    0,    0,  258,  258,    0,    0,  258,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  258,  258,  258,    0,    0,    0,  197,    0,  258,  258,
  258,  258,  258,  258,  258,    0,  201,    0,    0,  201,
    0,    0,    0,  201,  201,  201,  201,    0,    0,  197,
  201,  201,  201,  201,    0,  201,  201,    0,  201,  201,
    0,    0,  201,    0,    0,    0,  201,    0,  201,  201,
  201,  201,  201,    0,    0,  201,  201,  201,    0,    0,
    0,  201,    0,    0,    0,    0,    0,  201,    0,    0,
    0,    0,    0,    0,  201,    0,    0,    0,    0,    0,
    0,  201,    0,    0,    0,    0,    0,    0,    0,    0,
  201,    0,    0,    0,    0,  201,    0,    0,  201,    0,
    0,    0,    0,    0,    0,  201,  201,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  358,    0,  197,
    0,  201,  201,  201,    0,    0,    0,    0,    0,  201,
  201,  201,  201,  201,  201,  201,  200,    0,    0,  200,
  358,    0,    0,  200,  200,  200,  200,    0,    0,    0,
  200,  200,  200,  200,    0,  200,  200,    0,  200,  200,
    0,    0,  200,    0,    0,    0,  200,    0,  200,  200,
  200,  200,  200,    0,    0,  200,  200,  200,    0,    0,
    0,  200,    0,    0,    0,    0,    0,  200,    0,    0,
    0,    0,    0,    0,  200,    0,    0,    0,    0,    0,
    0,  200,    0,    0,    0,    0,    0,    0,    0,    0,
  200,    0,    0,    0,    0,  200,    0,    0,  200,    0,
    0,    0,    0,    0,    0,  200,  200,    0,    0,    0,
  358,    0,    0,    0,    0,    0,    0,  360,    0,    0,
    0,  200,  200,  200,    0,    0,    0,    0,    0,  200,
  200,  200,  200,  200,  200,  200,  197,    0,    0,  197,
  360,    0,    0,  197,  197,  197,  197,    0,    0,    0,
  197,  197,  197,  197,    0,  197,  197,    0,  197,  197,
    0,    0,  197,    0,    0,    0,  197,    0,  197,  197,
  197,  197,  197,    0,    0,  197,  197,  197,    0,    0,
    0,  197,    0,    0,    0,    0,    0,  197,    0,    0,
    0,    0,    0,    0,  197,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  197,    0,    0,    0,    0,  197,    0,    0,  197,    0,
    0,    0,    0,    0,    0,  197,  197,    0,    0,    0,
  360,    0,    0,    0,    0,    0,    0,  364,    0,    0,
    0,  197,  197,  197,    0,    0,    0,  358,    0,  197,
  197,  197,  197,  197,  197,  197,  358,  358,    0,    0,
  364,    0,    0,    0,    0,    0,  358,  358,    0,  358,
  358,    0,    0,  358,    0,    0,    0,  358,    0,  358,
  358,  358,  358,  358,    0,    0,  358,  358,  358,    0,
    0,    0,  358,    0,    0,    0,    0,    0,  358,    0,
    0,    0,    0,    0,    0,  358,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  358,  358,  358,  358,  358,
  358,  358,    0,    0,    0,    0,  358,    0,    0,  358,
    0,    0,    0,    0,    0,    0,  358,  358,    0,    0,
  358,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  364,    0,  358,  358,  358,    0,    0,  199,    0,    0,
  358,  358,  358,  358,  358,  358,  358,  360,    0,    0,
    0,    0,    0,    0,    0,    0,  360,  360,    0,    0,
  199,    0,    0,    0,    0,    0,  360,  360,    0,  360,
  360,    0,    0,  360,    0,    0,    0,  360,    0,  360,
  360,  360,  360,  360,    0,    0,  360,  360,  360,    0,
    0,    0,  360,    0,    0,    0,    0,    0,  360,    0,
    0,    0,    0,    0,    0,  360,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  360,  360,  360,  360,  360,
  360,  360,    0,    0,    0,    0,  360,    0,    0,  360,
    0,    0,    0,    0,    0,    0,  360,  360,    0,    0,
  360,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  360,  360,  360,    0,    0,  212,    0,    0,
  360,  360,  360,  360,  360,  360,  360,  364,    0,    0,
    0,    0,    0,    0,    0,    0,  364,  364,    0,    0,
  212,    0,    0,    0,    0,    0,  364,  364,    0,  364,
  364,    0,    0,  364,    0,    0,    0,  364,    0,  364,
  364,  364,  364,  364,    0,    0,  364,  364,  364,    0,
    0,    0,  364,    0,    0,    0,    0,    0,  364,    0,
    0,    0,    0,    0,    0,  364,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  362,  362,  362,  362,  362,
  362,  364,    0,    0,    0,    0,  364,    0,    0,  364,
    0,    0,    0,    0,    0,    0,  364,  364,    0,    0,
  362,    0,    0,    0,    0,    0,    0,    0,  364,    0,
  212,    0,  364,  364,  364,    0,    0,    0,    0,    0,
  364,  364,  364,  364,  364,  364,  364,  199,    0,    0,
  199,  364,    0,    0,  199,  199,  199,  199,    0,    0,
    0,  199,  199,  199,  199,    0,  199,  199,    0,  199,
  199,    0,    0,  199,    0,    0,    0,  199,    0,  199,
  199,  199,  199,  199,    0,    0,  199,  199,  199,    0,
    0,    0,  199,    0,    0,    0,    0,    0,  199,    0,
    0,    0,    0,    0,    0,  199,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  199,    0,    0,    0,    0,  199,    0,    0,  199,
    0,    0,    0,    0,    0,    0,  199,  199,    0,    0,
    0,  364,    0,    0,    0,    0,    0,    0,  212,    0,
    0,    0,  199,  199,  199,    0,    0,    0,    0,    0,
  199,  199,  199,  199,  199,  199,  199,  212,    0,    0,
  212,  212,    0,    0,    0,  212,  212,  212,    0,    0,
    0,  212,  212,  212,  212,    0,  212,  212,    0,  212,
  212,    0,    0,  212,    0,    0,    0,  212,    0,  212,
  212,  212,  212,  212,    0,    0,  212,  212,  212,    0,
    0,    0,  212,    0,    0,    0,    0,    0,  212,    0,
    0,    0,    0,    0,    0,  212,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  212,    0,    0,    0,    0,  212,    0,    0,  212,
    0,    0,    0,    0,    0,    0,  212,  212,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  221,    0,
    0,    0,  212,  212,  212,    0,    0,    0,  364,    0,
  212,  212,  212,  212,  212,  212,  212,    0,  364,    0,
    0,  221,    0,    0,    0,    0,    0,  364,  364,    0,
  364,  364,    0,    0,  364,    0,    0,    0,  364,    0,
  364,  364,  364,  364,  364,    0,    0,  364,  364,  364,
    0,    0,    0,  364,    0,    0,    0,    0,    0,  364,
    0,    0,    0,    0,    0,    0,  364,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  362,  362,  362,  362,
  362,  362,  364,    0,    0,    0,    0,  364,    0,    0,
  364,    0,    0,    0,    0,    0,    0,  364,  364,    0,
    0,  362,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  221,    0,  364,  364,  364,    0,    0,  221,    0,
    0,  364,  364,  364,  364,  364,  364,  364,  212,    0,
    0,  212,    0,    0,    0,    0,  212,  212,  212,    0,
    0,  221,  212,  212,  212,  212,    0,  212,  212,    0,
  212,  212,    0,    0,  212,    0,    0,    0,  212,    0,
  212,  212,  212,  212,  212,    0,    0,  212,  212,  212,
    0,    0,    0,  212,    0,    0,    0,    0,    0,  212,
    0,    0,    0,    0,    0,    0,  212,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  212,    0,    0,    0,    0,  212,    0,    0,
  212,    0,    0,    0,    0,    0,    0,  212,  212,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  212,  212,  212,    0,    0,  224,    0,
    0,  212,  212,  212,  212,  212,  212,  212,  221,    0,
    0,  221,    0,    0,    0,    0,  221,  221,  221,    0,
    0,  224,  221,    0,    0,  221,    0,  221,  221,    0,
  221,  221,    0,    0,  221,    0,    0,    0,  221,    0,
  221,  221,  221,  221,  221,    0,    0,  221,  221,  221,
    0,    0,    0,  221,    0,    0,    0,    0,    0,  221,
    0,    0,    0,    0,    0,    0,  221,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  221,    0,    0,    0,    0,  221,    0,    0,
  221,    0,    0,    0,    0,    0,    0,  221,  221,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  224,    0,  221,  221,  221,    0,    0,  224,    0,
    0,  221,  221,  221,  221,  221,  221,  221,  221,    0,
    0,  221,    0,    0,    0,    0,  221,  221,  221,    0,
    0,  224,  221,    0,    0,  221,    0,  221,  221,    0,
  221,  221,    0,    0,  221,    0,    0,    0,  221,    0,
  221,  221,  221,  221,  221,    0,    0,  221,  221,  221,
    0,    0,    0,  221,    0,    0,    0,    0,    0,  221,
    0,    0,    0,    0,    0,    0,  221,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  221,    0,    0,    0,    0,  221,    0,    0,
  221,    0,    0,    0,    0,    0,    0,  221,  221,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  221,  221,  221,    0,    0,  240,    0,
    0,  221,  221,  221,  221,  221,  221,  221,  224,    0,
    0,  224,    0,    0,    0,    0,  224,  224,  224,    0,
    0,  240,    0,    0,    0,  224,    0,  224,  224,    0,
  224,  224,    0,    0,  224,    0,    0,    0,  224,    0,
  224,  224,  224,  224,  224,    0,    0,  224,  224,  224,
    0,    0,    0,  224,    0,    0,    0,    0,    0,  224,
    0,    0,    0,    0,    0,    0,  224,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  224,    0,    0,    0,    0,  224,    0,    0,
  224,    0,    0,    0,    0,    0,    0,  224,  224,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  240,    0,  224,  224,  224,    0,    0,  244,    0,
    0,  224,  224,  224,  224,  224,  224,  224,  224,    0,
    0,  224,    0,    0,    0,    0,  224,  224,  224,    0,
    0,  244,    0,    0,    0,  224,    0,  224,  224,    0,
  224,  224,    0,    0,  224,    0,    0,    0,  224,    0,
  224,  224,  224,  224,  224,    0,    0,  224,  224,  224,
    0,    0,    0,  224,    0,    0,    0,    0,    0,  224,
    0,    0,    0,    0,    0,    0,  224,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  224,    0,    0,    0,    0,  224,    0,    0,
  224,    0,    0,    0,    0,    0,    0,  224,  224,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  244,    0,  224,  224,  224,    0,    0,  240,    0,
    0,  224,  224,  224,  224,  224,  224,  224,  240,    0,
    0,  240,    0,    0,    0,    0,    0,  240,  240,    0,
    0,  240,    0,    0,    0,    0,    0,  240,  240,    0,
  240,  240,    0,    0,  240,    0,    0,    0,  240,    0,
  240,  240,  240,  240,  240,    0,    0,  240,  240,  240,
    0,    0,    0,  240,    0,    0,    0,    0,    0,  240,
    0,    0,    0,    0,    0,    0,  240,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  240,    0,    0,    0,    0,  240,    0,    0,
  240,    0,    0,    0,    0,    0,    0,  240,  240,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  234,
    0,    0,    0,  240,  240,  240,    0,    0,    0,    0,
    0,  240,  240,  240,  240,  240,  240,  240,  244,    0,
    0,  244,  234,    0,    0,    0,    0,  244,  244,    0,
    0,    0,    0,    0,    0,    0,    0,  244,  244,    0,
  244,  244,    0,    0,  244,    0,    0,    0,  244,    0,
  244,  244,  244,  244,  244,    0,    0,  244,  244,  244,
    0,    0,    0,  244,    0,    0,    0,    0,    0,  244,
    0,    0,    0,    0,    0,    0,  244,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  244,    0,    0,    0,    0,  244,    0,    0,
  244,    0,    0,    0,    0,    0,    0,  244,  244,    0,
    0,    0,  234,    0,    0,    0,    0,    0,    0,  363,
    0,    0,    0,  244,  244,  244,    0,    0,    0,    0,
    0,  244,  244,  244,  244,  244,  244,  244,  240,    0,
    0,  240,  363,    0,    0,    0,    0,  240,  240,    0,
    0,    0,    0,    0,    0,    0,    0,  240,  240,    0,
  240,  240,    0,    0,  240,    0,    0,    0,  240,    0,
  240,  240,  240,  240,  240,    0,    0,  240,  240,  240,
    0,    0,    0,  240,    0,    0,    0,    0,    0,  240,
    0,    0,    0,    0,    0,    0,  240,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  240,    0,    0,    0,    0,  240,    0,    0,
  240,    0,    0,    0,    0,    0,    0,  240,  240,    0,
    0,    0,  363,    0,    0,    0,    0,    0,    0,  131,
    0,    0,    0,  240,  240,  240,    0,    0,    0,  234,
    0,  240,  240,  240,  240,  240,  240,  240,  234,  234,
    0,    0,  129,    0,    0,    0,    0,    0,  234,  234,
    0,  234,  234,    0,    0,  234,    0,    0,    0,  234,
    0,  234,  234,  234,  234,  234,    0,    0,  234,  234,
  234,    0,    0,    0,  234,    0,    0,    0,    0,    0,
  234,    0,    0,    0,    0,    0,    0,  234,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  234,    0,    0,    0,    0,  234,    0,
    0,  234,    0,    0,    0,    0,    0,    0,  234,  234,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  242,
    0,    0,  131,    0,  234,  234,  234,    0,    0,    0,
    0,    0,  234,  234,  234,  234,  234,  234,  234,  363,
    0,    0,  242,    0,    0,    0,    0,    0,  363,  363,
    0,    0,    0,    0,    0,    0,    0,    0,  363,  363,
    0,  363,  363,    0,    0,  363,    0,    0,    0,  363,
    0,  363,  363,  363,  363,  363,    0,    0,  363,  363,
  363,    0,    0,    0,  363,    0,    0,    0,    0,    0,
  363,    0,    0,    0,    0,    0,    0,  363,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  363,    0,    0,    0,    0,  363,    0,
    0,  363,    0,    0,    0,    0,    0,    0,  363,  363,
    0,    0,  242,    0,    0,    0,    0,    0,    0,  334,
    0,    0,    0,    0,  363,  363,  363,    0,    0,    0,
    0,    0,  363,  363,  363,  363,  363,  363,  363,  129,
    0,    0,  334,    0,    0,    0,    0,    0,    0,  129,
    0,    0,    0,    0,    0,    0,    0,    0,  129,  129,
    0,  129,  129,    0,    0,  129,    0,    0,    0,  129,
    0,  129,  129,  129,  129,  129,    0,    0,  129,  129,
  129,    0,    0,    0,  129,    0,    0,    0,    0,    0,
  129,    0,    0,    0,    0,    0,    0,  129,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  129,    0,    0,    0,    0,  129,    0,
  154,  129,    0,    0,    0,    0,    0,    0,  129,  129,
    0,    0,  334,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  154,  129,  129,  129,    0,    0,  242,
    0,    0,  129,  129,  129,  129,  129,  129,  129,  242,
    0,    0,    0,    0,    0,    0,    0,    0,  242,  242,
    0,  242,  242,    0,    0,  242,    0,    0,    0,  242,
    0,  242,  242,  242,  242,  242,    0,    0,  242,  242,
  242,    0,    0,    0,  242,    0,    0,    0,    0,    0,
  242,    0,    0,    0,    0,    0,    0,  242,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  131,    0,  242,    0,    0,    0,    0,  242,    0,
    0,  242,    0,  154,    0,    0,    0,    0,  242,  242,
    0,    0,    0,    0,  129,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  242,  242,  242,    0,    0,    0,
    0,    0,  242,  242,  242,  242,  242,  242,  242,  334,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  334,
    0,    0,    0,    0,    0,    0,    0,    0,  334,  334,
    0,  334,  334,    0,    0,  334,    0,    0,    0,  334,
    0,  334,  334,  334,  334,  334,    0,    0,  334,  334,
  334,    0,    0,    0,  334,    0,    0,    0,    0,    0,
  334,    0,    0,    0,    0,    0,    0,  334,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  334,    0,  162,    0,    0,  334,    0,
    0,  334,    0,    0,    0,    0,    0,    0,  334,  334,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  154,    0,    0,    0,  334,  334,  334,    0,    0,    0,
  154,    0,  334,  334,  334,  334,  334,  334,  334,  154,
  154,    0,  154,  154,    0,    0,  154,    0,    0,    0,
  154,    0,  154,  154,  154,  154,  154,    0,    0,  154,
  154,  154,    0,    0,    0,  154,    0,    0,    0,    0,
    0,  154,    0,    0,    0,    0,    0,    0,  154,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  154,    0,    0,    0,    0,  154,
    0,    0,  154,    0,    0,    0,    0,    0,    0,  154,
  154,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  129,    0,    0,    0,  154,  154,  154,    0,    0,
    0,  129,    0,  154,  154,  154,  154,  154,  154,  154,
  129,  129,    0,  129,  129,    0,    0,  129,    0,    0,
    0,  129,    0,  129,  129,  129,  129,  129,    0,    0,
  129,  129,  129,    0,    0,    0,  129,    0,    0,    0,
    0,    0,  129,    0,    0,    0,    0,    0,    0,  129,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  129,    0,    0,    0,    0,
  129,    0,    0,  129,    0,    0,    0,    0,    0,    0,
  129,  129,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  373,    0,    0,    0,  129,  129,  129,    0,
    0,    0,  374,    0,  129,  129,  129,  129,  129,  129,
  129,  375,  376,    0,  377,  378,    0,    0,  379,    0,
    0,    0,  380,    0,  381,  382,  383,  384,  385,    0,
    0,  386,  387,  388,    0,    0,    0,  389,    0,    0,
    0,    0,    0,  390,    0,    0,    0,    0,    0,    0,
  391,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  392,    0,    0,    0,
    0,  393,    0,    0,  394,    0,    0,    0,    0,    0,
    0,  395,  396,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  397,    0,  398,
    0,    0,    0,    0,    0,  399,  400,  401,  402,  403,
  404,
};
static const YYINT yycheck[] = {                          3,
    3,   95,   63,  110,    0,  164,  145,   33,   10,   40,
   44,   10,  435,   40,   10,   40,   10,  123,   10,   44,
   41,   10,  123,  511,  123,  277,   10,  278,  489,  188,
  105,  402,  123,  404,   33,  123,  325,  387,  388,  535,
  371,   40,  123,  182,  278,   44,  198,   41,   40,  548,
   44,  295,   44,  205,  479,  323,  324,   47,   33,  537,
  123,   60,  311,  157,  383,   33,  459,  161,   60,  472,
  209,  178,  211,  305,   40,  444,   40,  551,   33,  140,
  123,   40,  366,   33,   40,   44,   33,  268,  212,  196,
  160,  125,  343,  187,   60,   10,  277,   44,   44,   40,
  125,   60,   33,   61,   60,  244,  158,  339,   40,  343,
   44,  265,  266,  252,  208,   40,  210,  349,  257,   10,
  352,  123,   44,   40,  123,   44,  125,  552,  267,  383,
  384,  383,  383,  125,  123,   60,   60,  212,   44,  383,
   44,  629,  638,   60,  433,  239,  549,   10,  296,  383,
   33,  277,  651,  437,   44,  123,  579,   40,   33,  253,
  621,   44,  286,  641,  258,  534,  125,  123,  123,   40,
   60,   61,   62,  123,  313,  125,   44,   60,  125,  125,
  654,  251,  123,  141,   10,  291,   60,   61,   62,  283,
  284,  125,  123,  151,  342,   10,  248,  568,  548,  530,
  303,  551,  261,  125,  383,   44,  125,  600,  302,  321,
  336,  286,   60,   61,   62,  309,  355,  356,  357,  125,
  314,  125,  383,  349,   44,  319,   41,  321,   61,   44,
  457,   44,   33,  366,  328,  125,   10,  309,  221,   40,
  123,  344,  125,   44,  357,  358,  379,  306,  123,   33,
   10,  403,  278,  278,  522,  523,   40,  125,  333,   60,
  256,  257,  258,  259,  358,  492,  287,  288,  362,   10,
   40,  383,  256,  257,  258,  259,   60,  383,  417,  672,
   40,  302,  383,  384,  383,  384,  125,  308,  278,  291,
   60,  398,  383,  542,   40,  383,  317,   10,  297,  298,
  439,  651,  383,  278,  654,  125,  677,   10,  291,  303,
  278,  307,  125,  309,  278,  311,   40,  343,  343,  413,
  383,  384,  123,  307,  125,  309,  320,  311,  324,   10,
  326,  278,  426,   40,  366,  410,   60,  278,   10,  123,
  324,  125,  326,  339,  438,   33,  278,  379,   10,  383,
  509,  347,   40,  384,  448,  339,  383,  383,  383,  453,
  354,  355,  356,  347,  458,  359,  360,  361,  343,  363,
  364,   33,   60,  372,  368,  369,  470,  366,   40,  343,
  372,  383,   44,  477,  383,  384,  525,  383,  303,  383,
  379,  383,  384,  383,   44,  278,  343,  646,   60,  383,
  267,  508,  343,  278,   44,  320,  372,  278,  383,   41,
  285,  343,   44,  372,  297,  298,  372,  383,  384,  383,
  262,  560,  264,  562,  383,  384,   41,  383,  384,   44,
  524,   10,  526,  296,  573,  123,  383,  383,  384,  354,
  355,  356,  383,  504,  359,  360,  361,  372,  363,  364,
  589,  383,  591,  368,  369,  372,  595,   44,  383,  384,
   10,  123,  601,  125,  275,  276,  383,  384,  383,   45,
  296,   47,  343,   41,  613,  125,   44,  616,  572,  342,
  619,   10,   10,  544,   44,  125,   10,  581,  303,  372,
  380,  381,  382,  383,  384,   10,  297,  298,  592,  606,
  383,  384,  596,  564,   33,  320,  380,  381,  382,  383,
  384,   40,  383,  297,  298,   44,  342,  368,  369,   41,
  614,   10,   44,  617,   44,  383,  384,  297,  298,  303,
   44,   60,  380,  381,  382,  383,  384,  631,  125,  354,
  355,  356,  383,  384,  359,  360,  361,   10,  363,  364,
  644,   33,   41,  368,  369,   44,   10,   10,   40,  305,
   10,  269,  336,  337,  338,  125,  660,  341,  383,  277,
  344,  372,  346,  297,  298,  669,  670,  351,   60,  673,
  674,   40,  383,  384,   60,   61,   62,  383,  372,   40,
  278,  685,  300,  339,  123,  383,  125,  383,  260,  383,
  384,  263,  372,  349,  383,  125,  352,  269,  270,  297,
  298,  125,   41,  383,  384,   44,  278,  279,  280,  327,
  282,  283,   41,  285,  286,   44,  630,  630,  290,  383,
  292,  293,  294,  295,  296,  297,  298,  299,  300,  301,
   40,  123,  305,  305,  381,  382,  383,   33,  372,  311,
  269,  265,  261,  266,   40,  325,  318,  304,  277,  383,
  384,   62,  271,  272,  273,  266,  328,  329,  330,  331,
  332,  333,  334,  335,   60,   10,  339,  339,  383,  384,
  342,  300,  383,  384,  372,   10,  349,  349,  350,  352,
  287,  353,  383,  384,  384,  383,  384,  306,   33,  462,
  383,  384,  384,  365,  366,  367,  383,  384,  327,   44,
  372,  373,  374,  375,  376,  377,  378,  379,   10,   44,
  384,  383,  384,  383,  384,   60,   61,   62,  336,  337,
  338,  260,   10,  341,  263,  383,  384,  123,  346,  384,
  269,  270,  505,  351,  507,  383,  384,  383,  384,  278,
  279,  280,  268,  282,  283,  384,  285,  286,  383,  384,
  384,  290,  384,  292,  293,  294,  295,  296,  297,  298,
  299,  300,  301,  383,  384,   60,  305,  383,  384,  383,
  384,  384,  311,  382,  383,   44,  384,   41,  123,  318,
  125,   33,   40,  383,  383,   47,  278,  383,   40,  328,
  329,  330,  331,  332,  333,  334,  335,   10,   41,  384,
  339,  383,   33,  342,  303,  297,  298,  384,   60,   62,
  349,  350,  384,   41,  353,  384,  384,  383,  274,   41,
   33,  320,   60,   61,   62,  383,  365,  366,  367,  383,
  383,   44,  384,  372,  373,  374,  375,  376,  377,  378,
  379,  362,  345,  357,  383,  384,  263,   60,   61,   62,
  335,  325,  285,  626,  269,  354,  355,  356,   47,   47,
  359,  360,  361,  383,  363,  364,  383,  383,  123,  368,
  369,  123,  384,  336,  337,  338,  383,  385,  341,   40,
  372,  344,  284,  346,  383,  123,  284,  284,  351,  284,
  384,  383,  384,   33,  380,  381,  382,  383,  384,  383,
   40,  297,  298,  384,   44,  384,  384,  285,  383,   41,
  123,   62,  125,  384,  384,  260,  689,  344,  263,  303,
   60,   61,   62,   47,  269,  270,   60,   61,   62,   10,
  383,  281,  281,   40,  279,  280,  385,  282,  283,  285,
  385,  286,   60,   61,   62,  290,  384,  292,  293,  294,
  295,  296,   33,  383,  299,  300,  301,  336,  337,  338,
  305,   47,  341,   44,  285,  268,  311,  346,  303,  287,
  288,  383,  351,  318,   10,   10,  372,   41,  384,   60,
   61,   62,  383,  123,  302,  125,   41,  383,  384,  334,
  308,  384,  384,  384,  339,  383,  269,  342,  384,  317,
   10,  336,  337,  338,  349,  350,  341,   10,   10,  344,
   10,  346,   10,   41,  325,  303,  351,   10,  268,   10,
  365,  366,  367,  383,   10,  383,   10,   10,  373,  374,
  375,  376,  377,  378,  379,  380,  381,  382,  383,  384,
   10,   10,  123,  268,  125,  297,  298,  260,  336,  337,
  338,  384,   41,  341,   60,   61,   62,  270,  346,  485,
   54,   10,  564,  351,  242,  201,  279,  280,  279,  282,
  283,  415,  349,  286,  372,  336,  385,  290,  140,  292,
  293,  294,  295,  296,   33,   62,  299,  300,  301,  125,
  177,  539,  305,   66,   57,   44,  219,  486,  311,  328,
  329,  330,  331,  332,  333,  318,  297,  315,  263,  410,
  232,  241,  612,  636,  282,  630,   -1,  123,   -1,  630,
  372,  334,   -1,  125,  353,   -1,  339,   -1,   -1,  342,
   -1,  383,  384,   -1,   -1,   -1,  349,  350,  278,   -1,
   -1,   -1,  380,  381,  382,  383,  384,   -1,   -1,   -1,
   -1,   -1,  365,  366,  367,   -1,   -1,  297,  298,   -1,
  373,  374,  375,  376,  377,  378,  379,  380,  381,  382,
  383,  384,   -1,   -1,  123,   -1,  125,   -1,   -1,  260,
   -1,   -1,   60,   61,   62,   -1,   -1,   -1,   -1,  270,
   -1,   -1,   -1,   10,   -1,   -1,   -1,   -1,  279,  280,
   -1,  282,  283,  343,   -1,  286,   -1,   -1,   -1,  290,
   -1,  292,  293,  294,  295,  296,   33,   -1,  299,  300,
  301,  257,  258,  259,  305,   -1,   -1,   44,   -1,   -1,
  311,   -1,  372,   -1,   -1,   -1,   -1,  318,   -1,   -1,
  380,  381,  382,  383,  384,  123,  380,  381,  382,  383,
  384,   -1,   -1,  334,  256,  257,  258,  259,  339,   -1,
   -1,  342,  380,  381,  382,  383,  384,   -1,  349,  350,
   -1,  307,   -1,  309,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  365,  366,  367,   -1,   -1,   -1,
  326,   -1,  373,  374,  375,  376,  377,  378,  379,  380,
  381,  382,  383,  384,   -1,   -1,  123,  309,  125,   -1,
   -1,  260,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  270,   -1,   -1,   -1,   -1,   10,   -1,   -1,   -1,
  279,  280,   -1,  282,  283,   -1,   -1,  286,   -1,   -1,
   -1,  290,   -1,  292,  293,  294,  295,  296,   -1,   33,
  299,  300,  301,   -1,   -1,   -1,  305,   41,   -1,   -1,
   44,   -1,  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,
   -1,   -1,   -1,   -1,  380,  381,  382,  383,  384,   -1,
  303,   -1,   -1,   -1,   -1,  334,   -1,   -1,   -1,   -1,
  339,   -1,   -1,  342,   -1,   -1,   -1,  320,   -1,   -1,
  349,  350,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  365,  366,  367,   -1,
   -1,   -1,   -1,   -1,  373,  374,  375,  376,  377,  378,
  379,  354,  355,  356,  383,  384,  359,  360,  361,  123,
  363,  364,   -1,  260,   -1,  368,  369,   10,   -1,   -1,
   -1,   -1,   -1,  270,   -1,   -1,   -1,   -1,   -1,   -1,
  383,   -1,  279,  280,   -1,  282,  283,   -1,   -1,  286,
   33,   -1,   -1,  290,   -1,  292,  293,  294,  295,  296,
   -1,   -1,  299,  300,  301,   -1,   -1,   -1,  305,   -1,
   -1,   -1,   -1,   -1,  311,   -1,   -1,   -1,   -1,   -1,
   -1,  318,  380,  381,  382,  383,  384,   -1,   -1,   -1,
   -1,   -1,  303,   -1,   -1,   -1,   -1,  334,   -1,   -1,
   -1,   -1,  339,   -1,   -1,  342,   -1,   -1,   -1,  320,
   -1,   -1,  349,  350,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  365,  366,
  367,   -1,   -1,   -1,   -1,   -1,  373,  374,  375,  376,
  377,  378,  379,  354,  355,  356,  383,  384,  359,  360,
  361,   10,  363,  364,   -1,   -1,  260,  368,  369,   -1,
   -1,   -1,   -1,   -1,   -1,  269,  270,   -1,   -1,   -1,
   -1,   -1,  383,  277,   33,  279,  280,   -1,  282,  283,
   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,  292,  293,
  294,  295,  296,   -1,   -1,  299,  300,  301,   -1,   -1,
   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,   -1,
   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  327,  328,  329,  330,  331,  332,  333,
  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,   -1,
   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,  353,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  365,  366,  367,  123,   -1,   -1,   -1,   -1,  373,
  374,  375,  376,  377,  378,  379,   -1,  260,   10,  262,
  263,  264,  265,  266,  267,  268,  269,  270,   -1,   -1,
   -1,  274,  275,  276,  277,   -1,  279,  280,   -1,  282,
  283,   33,   -1,  286,   -1,   -1,   -1,  290,   -1,  292,
  293,  294,  295,  296,   -1,   -1,  299,  300,  301,   -1,
   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,
   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,
   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  365,  366,  367,   -1,   -1,   -1,   -1,   -1,
  373,  374,  375,  376,  377,  378,  379,   -1,   -1,   10,
   -1,  260,   -1,  262,  263,  264,   -1,  266,  267,  268,
  269,  270,   -1,   -1,   -1,  274,  275,  276,  277,   -1,
  279,  280,   33,  282,  283,   -1,   -1,  286,   -1,   -1,
   -1,  290,   -1,  292,  293,  294,  295,  296,   -1,   -1,
  299,  300,  301,   -1,   -1,   -1,  305,   -1,   -1,   -1,
   -1,   -1,  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,
   -1,   -1,   -1,  308,   -1,  310,   -1,  312,  313,  314,
  315,  316,   -1,   -1,  319,  334,  321,  322,  323,   -1,
  339,   -1,   -1,  342,   -1,   -1,   -1,   -1,   -1,   -1,
  349,  350,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  348,   -1,   -1,  365,  366,  367,   -1,
   -1,   -1,   -1,   -1,  373,  374,  375,  376,  377,  378,
  379,   -1,   -1,   10,   -1,  370,  371,   -1,  260,   -1,
  262,  263,  264,  265,  266,  267,  268,  269,  270,   -1,
   -1,   -1,  274,  275,  276,  277,   33,  279,  280,   -1,
  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,
  292,  293,  294,  295,  296,   -1,   -1,  299,  300,  301,
   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,
   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,
  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  365,  366,  367,   -1,   -1,   -1,   -1,
   -1,  373,  374,  375,  376,  377,  378,  379,   -1,  260,
   10,  262,  263,  264,  265,  266,  267,  268,  269,  270,
   -1,   -1,   -1,  274,  275,  276,  277,   -1,  279,  280,
   -1,  282,  283,   33,   -1,  286,   -1,   -1,   -1,  290,
   -1,  292,  293,  294,  295,  296,   -1,   -1,  299,  300,
  301,   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,
  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,
   -1,  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  365,  366,  367,   -1,   -1,   -1,
   -1,   -1,  373,  374,  375,  376,  377,  378,  379,   -1,
   -1,   10,   -1,  260,   -1,  262,  263,  264,  265,  266,
  267,  268,  269,  270,   -1,   -1,   -1,  274,  275,  276,
  277,   -1,  279,  280,   33,  282,  283,   -1,   -1,  286,
   -1,   -1,   -1,  290,   -1,  292,  293,  294,  295,  296,
   -1,   -1,  299,  300,  301,   -1,   -1,   -1,  305,   -1,
   -1,   -1,   -1,   -1,  311,   -1,   -1,   -1,   -1,   -1,
   -1,  318,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  334,   -1,   -1,
   -1,   -1,  339,   -1,   -1,  342,   -1,   -1,   -1,   -1,
   -1,   -1,  349,  350,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  365,  366,
  367,   -1,   -1,   -1,  123,   -1,  373,  374,  375,  376,
  377,  378,  379,   -1,   -1,   10,   -1,   -1,   -1,   -1,
  260,   -1,   -1,  263,   -1,  265,  266,  267,  268,  269,
  270,   -1,   -1,   -1,  274,  275,  276,  277,   33,  279,
  280,   -1,  282,  283,   -1,   -1,  286,   -1,   -1,   -1,
  290,   -1,  292,  293,  294,  295,  296,   -1,   -1,  299,
  300,  301,   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,
   -1,  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,
   -1,   -1,  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,
  350,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  365,  366,  367,   -1,   -1,
   -1,   -1,   -1,  373,  374,  375,  376,  377,  378,  379,
   -1,  260,   -1,   -1,  263,   -1,   -1,  266,  267,  268,
  269,  270,   -1,   -1,   -1,  274,  275,  276,  277,   -1,
  279,  280,   -1,  282,  283,   10,   -1,  286,   -1,   -1,
   -1,  290,   -1,  292,  293,  294,  295,  296,   -1,   -1,
  299,  300,  301,   -1,   -1,   -1,  305,   -1,   33,   -1,
   -1,   -1,  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  334,   -1,   -1,   -1,   -1,
  339,   -1,   -1,  342,   -1,   -1,   -1,   -1,   -1,   -1,
  349,  350,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  365,  366,  367,   -1,
   -1,   -1,   -1,   -1,  373,  374,  375,  376,  377,  378,
  379,   -1,   -1,   -1,   -1,  260,   -1,   -1,  263,   -1,
   -1,  266,  267,  268,  269,  270,   -1,   -1,  123,  274,
  275,  276,  277,   -1,  279,  280,   10,  282,  283,   -1,
   -1,  286,   -1,   -1,   -1,  290,   -1,  292,  293,  294,
  295,  296,   -1,   -1,  299,  300,  301,   -1,   -1,   33,
  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,   -1,   -1,
   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,   -1,   -1,
  325,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  334,
   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,   -1,   -1,
   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  365,  366,  367,   -1,   -1,   -1,   -1,   -1,  373,  374,
  375,  376,  377,  378,  379,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   10,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  260,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  269,  270,   -1,   -1,   -1,   33,
   -1,   -1,   -1,   -1,  279,  280,   -1,  282,  283,   -1,
  285,  286,   -1,   -1,   -1,  290,   -1,  292,  293,  294,
  295,  296,   -1,   -1,  299,  300,  301,   -1,   -1,   -1,
  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,   -1,   -1,
   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  328,  329,  330,  331,  332,  333,  334,
   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,   -1,   -1,
   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,  353,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  365,  366,  367,   -1,   -1,   -1,   10,   -1,  373,  374,
  375,  376,  377,  378,  379,   -1,  260,   -1,   -1,  263,
   -1,   -1,   -1,  267,  268,  269,  270,   -1,   -1,   33,
  274,  275,  276,  277,   -1,  279,  280,   -1,  282,  283,
   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,  292,  293,
  294,  295,  296,   -1,   -1,  299,  300,  301,   -1,   -1,
   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,   -1,
   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,   -1,
   -1,  325,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,   -1,
   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   10,   -1,  123,
   -1,  365,  366,  367,   -1,   -1,   -1,   -1,   -1,  373,
  374,  375,  376,  377,  378,  379,  260,   -1,   -1,  263,
   33,   -1,   -1,  267,  268,  269,  270,   -1,   -1,   -1,
  274,  275,  276,  277,   -1,  279,  280,   -1,  282,  283,
   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,  292,  293,
  294,  295,  296,   -1,   -1,  299,  300,  301,   -1,   -1,
   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,   -1,
   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,   -1,
   -1,  325,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,   -1,
   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,   -1,
  123,   -1,   -1,   -1,   -1,   -1,   -1,   10,   -1,   -1,
   -1,  365,  366,  367,   -1,   -1,   -1,   -1,   -1,  373,
  374,  375,  376,  377,  378,  379,  260,   -1,   -1,  263,
   33,   -1,   -1,  267,  268,  269,  270,   -1,   -1,   -1,
  274,  275,  276,  277,   -1,  279,  280,   -1,  282,  283,
   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,  292,  293,
  294,  295,  296,   -1,   -1,  299,  300,  301,   -1,   -1,
   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,   -1,
   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,   -1,
   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,   -1,
  123,   -1,   -1,   -1,   -1,   -1,   -1,   10,   -1,   -1,
   -1,  365,  366,  367,   -1,   -1,   -1,  260,   -1,  373,
  374,  375,  376,  377,  378,  379,  269,  270,   -1,   -1,
   33,   -1,   -1,   -1,   -1,   -1,  279,  280,   -1,  282,
  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,  292,
  293,  294,  295,  296,   -1,   -1,  299,  300,  301,   -1,
   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,
   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  328,  329,  330,  331,  332,
  333,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,
   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,
  353,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  123,   -1,  365,  366,  367,   -1,   -1,   10,   -1,   -1,
  373,  374,  375,  376,  377,  378,  379,  260,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  269,  270,   -1,   -1,
   33,   -1,   -1,   -1,   -1,   -1,  279,  280,   -1,  282,
  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,  292,
  293,  294,  295,  296,   -1,   -1,  299,  300,  301,   -1,
   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,
   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  328,  329,  330,  331,  332,
  333,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,
   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,
  353,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  365,  366,  367,   -1,   -1,   10,   -1,   -1,
  373,  374,  375,  376,  377,  378,  379,  260,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  269,  270,   -1,   -1,
   33,   -1,   -1,   -1,   -1,   -1,  279,  280,   -1,  282,
  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,  292,
  293,  294,  295,  296,   -1,   -1,  299,  300,  301,   -1,
   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,
   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  328,  329,  330,  331,  332,
  333,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,
   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,
  353,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   10,   -1,
  123,   -1,  365,  366,  367,   -1,   -1,   -1,   -1,   -1,
  373,  374,  375,  376,  377,  378,  379,  260,   -1,   -1,
  263,   33,   -1,   -1,  267,  268,  269,  270,   -1,   -1,
   -1,  274,  275,  276,  277,   -1,  279,  280,   -1,  282,
  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,  292,
  293,  294,  295,  296,   -1,   -1,  299,  300,  301,   -1,
   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,
   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,
   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,
   -1,  123,   -1,   -1,   -1,   -1,   -1,   -1,   10,   -1,
   -1,   -1,  365,  366,  367,   -1,   -1,   -1,   -1,   -1,
  373,  374,  375,  376,  377,  378,  379,  260,   -1,   -1,
  263,   33,   -1,   -1,   -1,  268,  269,  270,   -1,   -1,
   -1,  274,  275,  276,  277,   -1,  279,  280,   -1,  282,
  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,  292,
  293,  294,  295,  296,   -1,   -1,  299,  300,  301,   -1,
   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,   -1,
   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,  342,
   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   10,   -1,
   -1,   -1,  365,  366,  367,   -1,   -1,   -1,  260,   -1,
  373,  374,  375,  376,  377,  378,  379,   -1,  270,   -1,
   -1,   33,   -1,   -1,   -1,   -1,   -1,  279,  280,   -1,
  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,
  292,  293,  294,  295,  296,   -1,   -1,  299,  300,  301,
   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,
   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  328,  329,  330,  331,
  332,  333,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,
  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,
   -1,  353,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  123,   -1,  365,  366,  367,   -1,   -1,   10,   -1,
   -1,  373,  374,  375,  376,  377,  378,  379,  260,   -1,
   -1,  263,   -1,   -1,   -1,   -1,  268,  269,  270,   -1,
   -1,   33,  274,  275,  276,  277,   -1,  279,  280,   -1,
  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,
  292,  293,  294,  295,  296,   -1,   -1,  299,  300,  301,
   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,
   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,
  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  365,  366,  367,   -1,   -1,   10,   -1,
   -1,  373,  374,  375,  376,  377,  378,  379,  260,   -1,
   -1,  263,   -1,   -1,   -1,   -1,  268,  269,  270,   -1,
   -1,   33,  274,   -1,   -1,  277,   -1,  279,  280,   -1,
  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,
  292,  293,  294,  295,  296,   -1,   -1,  299,  300,  301,
   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,
   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,
  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  123,   -1,  365,  366,  367,   -1,   -1,   10,   -1,
   -1,  373,  374,  375,  376,  377,  378,  379,  260,   -1,
   -1,  263,   -1,   -1,   -1,   -1,  268,  269,  270,   -1,
   -1,   33,  274,   -1,   -1,  277,   -1,  279,  280,   -1,
  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,
  292,  293,  294,  295,  296,   -1,   -1,  299,  300,  301,
   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,
   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,
  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  365,  366,  367,   -1,   -1,   10,   -1,
   -1,  373,  374,  375,  376,  377,  378,  379,  260,   -1,
   -1,  263,   -1,   -1,   -1,   -1,  268,  269,  270,   -1,
   -1,   33,   -1,   -1,   -1,  277,   -1,  279,  280,   -1,
  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,
  292,  293,  294,  295,  296,   -1,   -1,  299,  300,  301,
   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,
   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,
  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  123,   -1,  365,  366,  367,   -1,   -1,   10,   -1,
   -1,  373,  374,  375,  376,  377,  378,  379,  260,   -1,
   -1,  263,   -1,   -1,   -1,   -1,  268,  269,  270,   -1,
   -1,   33,   -1,   -1,   -1,  277,   -1,  279,  280,   -1,
  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,
  292,  293,  294,  295,  296,   -1,   -1,  299,  300,  301,
   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,
   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,
  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  123,   -1,  365,  366,  367,   -1,   -1,   10,   -1,
   -1,  373,  374,  375,  376,  377,  378,  379,  260,   -1,
   -1,  263,   -1,   -1,   -1,   -1,   -1,  269,  270,   -1,
   -1,   33,   -1,   -1,   -1,   -1,   -1,  279,  280,   -1,
  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,
  292,  293,  294,  295,  296,   -1,   -1,  299,  300,  301,
   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,
   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,
  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   10,
   -1,   -1,   -1,  365,  366,  367,   -1,   -1,   -1,   -1,
   -1,  373,  374,  375,  376,  377,  378,  379,  260,   -1,
   -1,  263,   33,   -1,   -1,   -1,   -1,  269,  270,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  279,  280,   -1,
  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,
  292,  293,  294,  295,  296,   -1,   -1,  299,  300,  301,
   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,
   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,
  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,
   -1,   -1,  123,   -1,   -1,   -1,   -1,   -1,   -1,   10,
   -1,   -1,   -1,  365,  366,  367,   -1,   -1,   -1,   -1,
   -1,  373,  374,  375,  376,  377,  378,  379,  260,   -1,
   -1,  263,   33,   -1,   -1,   -1,   -1,  269,  270,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  279,  280,   -1,
  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,   -1,
  292,  293,  294,  295,  296,   -1,   -1,  299,  300,  301,
   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,  311,
   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,   -1,
  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,   -1,
   -1,   -1,  123,   -1,   -1,   -1,   -1,   -1,   -1,   10,
   -1,   -1,   -1,  365,  366,  367,   -1,   -1,   -1,  260,
   -1,  373,  374,  375,  376,  377,  378,  379,  269,  270,
   -1,   -1,   33,   -1,   -1,   -1,   -1,   -1,  279,  280,
   -1,  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,
   -1,  292,  293,  294,  295,  296,   -1,   -1,  299,  300,
  301,   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,
  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,
   -1,  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   10,
   -1,   -1,  123,   -1,  365,  366,  367,   -1,   -1,   -1,
   -1,   -1,  373,  374,  375,  376,  377,  378,  379,  260,
   -1,   -1,   33,   -1,   -1,   -1,   -1,   -1,  269,  270,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  279,  280,
   -1,  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,
   -1,  292,  293,  294,  295,  296,   -1,   -1,  299,  300,
  301,   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,
  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,
   -1,  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,
   -1,   -1,  123,   -1,   -1,   -1,   -1,   -1,   -1,   10,
   -1,   -1,   -1,   -1,  365,  366,  367,   -1,   -1,   -1,
   -1,   -1,  373,  374,  375,  376,  377,  378,  379,  260,
   -1,   -1,   33,   -1,   -1,   -1,   -1,   -1,   -1,  270,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  279,  280,
   -1,  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,
   -1,  292,  293,  294,  295,  296,   -1,   -1,  299,  300,
  301,   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,
  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,
   10,  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,
   -1,   -1,  123,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   33,  365,  366,  367,   -1,   -1,  260,
   -1,   -1,  373,  374,  375,  376,  377,  378,  379,  270,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  279,  280,
   -1,  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,
   -1,  292,  293,  294,  295,  296,   -1,   -1,  299,  300,
  301,   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,
  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   10,   -1,  334,   -1,   -1,   -1,   -1,  339,   -1,
   -1,  342,   -1,  123,   -1,   -1,   -1,   -1,  349,  350,
   -1,   -1,   -1,   -1,   33,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  365,  366,  367,   -1,   -1,   -1,
   -1,   -1,  373,  374,  375,  376,  377,  378,  379,  260,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  270,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  279,  280,
   -1,  282,  283,   -1,   -1,  286,   -1,   -1,   -1,  290,
   -1,  292,  293,  294,  295,  296,   -1,   -1,  299,  300,
  301,   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,   -1,
  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  334,   -1,   33,   -1,   -1,  339,   -1,
   -1,  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,  350,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  260,   -1,   -1,   -1,  365,  366,  367,   -1,   -1,   -1,
  270,   -1,  373,  374,  375,  376,  377,  378,  379,  279,
  280,   -1,  282,  283,   -1,   -1,  286,   -1,   -1,   -1,
  290,   -1,  292,  293,  294,  295,  296,   -1,   -1,  299,
  300,  301,   -1,   -1,   -1,  305,   -1,   -1,   -1,   -1,
   -1,  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  334,   -1,   -1,   -1,   -1,  339,
   -1,   -1,  342,   -1,   -1,   -1,   -1,   -1,   -1,  349,
  350,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  260,   -1,   -1,   -1,  365,  366,  367,   -1,   -1,
   -1,  270,   -1,  373,  374,  375,  376,  377,  378,  379,
  279,  280,   -1,  282,  283,   -1,   -1,  286,   -1,   -1,
   -1,  290,   -1,  292,  293,  294,  295,  296,   -1,   -1,
  299,  300,  301,   -1,   -1,   -1,  305,   -1,   -1,   -1,
   -1,   -1,  311,   -1,   -1,   -1,   -1,   -1,   -1,  318,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  334,   -1,   -1,   -1,   -1,
  339,   -1,   -1,  342,   -1,   -1,   -1,   -1,   -1,   -1,
  349,  350,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  260,   -1,   -1,   -1,  365,  366,  367,   -1,
   -1,   -1,  270,   -1,  373,  374,  375,  376,  377,  378,
  379,  279,  280,   -1,  282,  283,   -1,   -1,  286,   -1,
   -1,   -1,  290,   -1,  292,  293,  294,  295,  296,   -1,
   -1,  299,  300,  301,   -1,   -1,   -1,  305,   -1,   -1,
   -1,   -1,   -1,  311,   -1,   -1,   -1,   -1,   -1,   -1,
  318,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  334,   -1,   -1,   -1,
   -1,  339,   -1,   -1,  342,   -1,   -1,   -1,   -1,   -1,
   -1,  349,  350,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  365,   -1,  367,
   -1,   -1,   -1,   -1,   -1,  373,  374,  375,  376,  377,
  378,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 385
#define YYUNDFTOKEN 530
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,"'\\n'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,"'!'",0,0,0,0,0,0,"'('","')'",0,0,"','","'-'",0,"'/'",0,0,0,0,0,0,0,0,0,0,0,
0,"'<'","'='","'>'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,"PASS","BLOCK","MATCH","SCRUB","RETURN","IN","OS","OUT","LOG",
"QUICK","ON","FROM","TO","FLAGS","RETURNRST","RETURNICMP","RETURNICMP6","PROTO",
"INET","INET6","ALL","ANY","ICMPTYPE","ICMP6TYPE","CODE","KEEP","MODULATE",
"STATE","PORT","BINATTO","NODF","MINTTL","ERROR","ALLOWOPTS","FILENAME",
"ROUTETO","DUPTO","REPLYTO","NO","LABEL","NOROUTE","URPFFAILED","FRAGMENT",
"USER","GROUP","MAXMSS","MAXIMUM","TTL","TOS","DROP","TABLE","REASSEMBLE",
"ANCHOR","SYNCOOKIES","SET","OPTIMIZATION","TIMEOUT","LIMIT","LOGINTERFACE",
"BLOCKPOLICY","RANDOMID","SYNPROXY","FINGERPRINTS","NOSYNC","DEBUG","SKIP",
"HOSTID","ANTISPOOF","FOR","INCLUDE","MATCHES","BITMASK","RANDOM","SOURCEHASH",
"ROUNDROBIN","LEASTSTATES","STATICPORT","PROBABILITY","WEIGHT","BANDWIDTH",
"FLOWS","QUANTUM","QUEUE","PRIORITY","QLIMIT","RTABLE","RDOMAIN","MINIMUM",
"BURST","PARENT","LOAD","RULESET_OPTIMIZATION","PRIO","ONCE","DEFAULT","DELAY",
"STICKYADDRESS","MAXSRCSTATES","MAXSRCNODES","SOURCETRACK","GLOBAL","RULE",
"MAXSRCCONN","MAXSRCCONNRATE","OVERLOAD","FLUSH","SLOPPY","PFLOW","MAXPKTRATE",
"TAGGED","TAG","IFBOUND","FLOATING","STATEPOLICY","STATEDEFAULTS","ROUTE",
"DIVERTTO","DIVERTREPLY","DIVERTPACKET","NATTO","AFTO","RDRTO","RECEIVEDON",
"NE","LE","GE","STRING","NUMBER","PORTBINARY",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,"illegal-symbol",
};
static const char *const yyrule[] = {
"$accept : ruleset",
"ruleset :",
"ruleset : ruleset include '\\n'",
"ruleset : ruleset '\\n'",
"ruleset : ruleset option '\\n'",
"ruleset : ruleset pfrule '\\n'",
"ruleset : ruleset anchorrule '\\n'",
"ruleset : ruleset loadrule '\\n'",
"ruleset : ruleset queuespec '\\n'",
"ruleset : ruleset varset '\\n'",
"ruleset : ruleset antispoof '\\n'",
"ruleset : ruleset tabledef '\\n'",
"ruleset : '{' fakeanchor '}' '\\n'",
"ruleset : ruleset error '\\n'",
"include : INCLUDE STRING",
"fakeanchor : fakeanchor '\\n'",
"fakeanchor : fakeanchor anchorrule '\\n'",
"fakeanchor : fakeanchor pfrule '\\n'",
"fakeanchor : fakeanchor error '\\n'",
"optimizer : string",
"optnodf :",
"optnodf : NODF",
"option : SET REASSEMBLE yesno optnodf",
"option : SET OPTIMIZATION STRING",
"option : SET RULESET_OPTIMIZATION optimizer",
"option : SET TIMEOUT timeout_spec",
"option : SET TIMEOUT '{' optnl timeout_list '}'",
"option : SET LIMIT limit_spec",
"option : SET LIMIT '{' optnl limit_list '}'",
"option : SET LOGINTERFACE stringall",
"option : SET HOSTID number",
"option : SET BLOCKPOLICY DROP",
"option : SET BLOCKPOLICY RETURN",
"option : SET FINGERPRINTS STRING",
"option : SET STATEPOLICY statelock",
"option : SET DEBUG STRING",
"option : SET DEBUG DEBUG",
"option : SET SKIP interface",
"option : SET STATEDEFAULTS state_opt_list",
"option : SET SYNCOOKIES syncookie_val syncookie_opts",
"syncookie_val : STRING",
"syncookie_opts :",
"$$1 :",
"syncookie_opts : $$1 '(' syncookie_opt_l ')'",
"syncookie_opt_l : syncookie_opt_l comma syncookie_opt",
"syncookie_opt_l : syncookie_opt",
"syncookie_opt : STRING STRING",
"stringall : STRING",
"stringall : ALL",
"string : STRING string",
"string : STRING",
"varstring : numberstring varstring",
"varstring : numberstring",
"numberstring : NUMBER",
"numberstring : STRING",
"varset : STRING '=' varstring",
"anchorname : STRING",
"anchorname :",
"pfa_anchorlist :",
"pfa_anchorlist : pfa_anchorlist '\\n'",
"pfa_anchorlist : pfa_anchorlist tabledef '\\n'",
"pfa_anchorlist : pfa_anchorlist pfrule '\\n'",
"pfa_anchorlist : pfa_anchorlist anchorrule '\\n'",
"pfa_anchorlist : pfa_anchorlist include '\\n'",
"$$2 :",
"pfa_anchor : '{' $$2 '\\n' pfa_anchorlist '}'",
"pfa_anchor :",
"anchorrule : ANCHOR anchorname dir quick interface af proto fromto filter_opts pfa_anchor",
"loadrule : LOAD ANCHOR anchorname FROM string",
"$$3 :",
"scrub_opts : $$3 scrub_opts_l",
"scrub_opts_l : scrub_opts_l comma scrub_opt",
"scrub_opts_l : scrub_opt",
"scrub_opt : NODF",
"scrub_opt : MINTTL NUMBER",
"scrub_opt : MAXMSS NUMBER",
"scrub_opt : REASSEMBLE STRING",
"scrub_opt : RANDOMID",
"antispoof : ANTISPOOF logquick antispoof_ifspc af antispoof_opts",
"antispoof_ifspc : FOR antispoof_if",
"antispoof_ifspc : FOR '{' optnl antispoof_iflst '}'",
"antispoof_iflst : antispoof_if optnl",
"antispoof_iflst : antispoof_iflst comma antispoof_if optnl",
"antispoof_if : if_item",
"antispoof_if : '(' if_item ')'",
"$$4 :",
"antispoof_opts : $$4 antispoof_opts_l",
"antispoof_opts :",
"antispoof_opts_l : antispoof_opts_l antispoof_opt",
"antispoof_opts_l : antispoof_opt",
"antispoof_opt : LABEL label",
"antispoof_opt : RTABLE NUMBER",
"not : '!'",
"not :",
"tabledef : TABLE '<' STRING '>' table_opts",
"$$5 :",
"table_opts : $$5 table_opts_l",
"table_opts :",
"table_opts_l : table_opts_l table_opt",
"table_opts_l : table_opt",
"table_opt : STRING",
"table_opt : '{' optnl '}'",
"table_opt : '{' optnl table_host_list '}'",
"table_opt : FILENAME STRING",
"tablespec : xhost optweight",
"tablespec : '{' optnl table_host_list '}'",
"table_host_list : tablespec optnl",
"table_host_list : table_host_list comma tablespec optnl",
"queuespec : QUEUE STRING interface queue_opts",
"$$6 :",
"queue_opts : $$6 queue_opts_l",
"queue_opts_l : queue_opts_l queue_opt",
"queue_opts_l : queue_opt",
"queue_opt : BANDWIDTH scspec optscs",
"queue_opt : PARENT STRING",
"queue_opt : DEFAULT",
"queue_opt : QLIMIT NUMBER",
"queue_opt : FLOWS NUMBER",
"queue_opt : QUANTUM NUMBER",
"optscs :",
"optscs : comma MINIMUM scspec",
"optscs : comma MAXIMUM scspec",
"optscs : comma MINIMUM scspec comma MAXIMUM scspec",
"optscs : comma MAXIMUM scspec comma MINIMUM scspec",
"scspec : bandwidth",
"scspec : bandwidth BURST bandwidth FOR STRING",
"bandwidth : STRING",
"bandwidth : NUMBER",
"pfrule : action dir logquick interface af proto fromto filter_opts",
"$$7 :",
"filter_opts : $$7 filter_opts_l",
"filter_opts :",
"filter_opts_l : filter_opts_l filter_opt",
"filter_opts_l : filter_opt",
"filter_opt : USER uids",
"filter_opt : GROUP gids",
"filter_opt : flags",
"filter_opt : icmpspec",
"filter_opt : PRIO NUMBER",
"filter_opt : TOS tos",
"filter_opt : keep",
"filter_opt : FRAGMENT",
"filter_opt : ALLOWOPTS",
"filter_opt : LABEL label",
"filter_opt : QUEUE qname",
"filter_opt : TAG string",
"filter_opt : not TAGGED string",
"filter_opt : PROBABILITY probability",
"filter_opt : RTABLE NUMBER",
"filter_opt : DIVERTTO STRING PORT portplain",
"filter_opt : DIVERTREPLY",
"filter_opt : DIVERTPACKET PORT portplain",
"filter_opt : SCRUB '(' scrub_opts ')'",
"filter_opt : NATTO redirpool pool_opts",
"filter_opt : AFTO af FROM redirpool pool_opts",
"filter_opt : AFTO af FROM redirpool pool_opts TO redirpool pool_opts",
"filter_opt : RDRTO redirpool pool_opts",
"filter_opt : BINATTO redirpool pool_opts",
"filter_opt : ROUTETO routespec",
"filter_opt : REPLYTO routespec",
"filter_opt : DUPTO routespec",
"filter_opt : not RECEIVEDON if_item",
"filter_opt : ONCE",
"filter_opt : MAXPKTRATE NUMBER '/' NUMBER",
"filter_opt : filter_sets",
"filter_sets : SET '(' filter_sets_l ')'",
"filter_sets : SET filter_set",
"filter_sets_l : filter_sets_l comma filter_set",
"filter_sets_l : filter_set",
"filter_set : prio",
"filter_set : QUEUE qname",
"filter_set : TOS tos",
"filter_set : DELAY NUMBER",
"prio : PRIO NUMBER",
"prio : PRIO '(' NUMBER comma NUMBER ')'",
"probability : STRING",
"probability : NUMBER",
"action : PASS",
"action : MATCH",
"action : BLOCK blockspec",
"blockspec :",
"blockspec : DROP",
"blockspec : RETURNRST",
"blockspec : RETURNRST '(' TTL NUMBER ')'",
"blockspec : RETURNICMP",
"blockspec : RETURNICMP6",
"blockspec : RETURNICMP '(' reticmpspec ')'",
"blockspec : RETURNICMP6 '(' reticmp6spec ')'",
"blockspec : RETURNICMP '(' reticmpspec comma reticmp6spec ')'",
"blockspec : RETURN",
"reticmpspec : STRING",
"reticmpspec : NUMBER",
"reticmp6spec : STRING",
"reticmp6spec : NUMBER",
"dir :",
"dir : IN",
"dir : OUT",
"quick :",
"quick : QUICK",
"logquick :",
"logquick : log",
"logquick : QUICK",
"logquick : log QUICK",
"logquick : QUICK log",
"log : LOG",
"log : LOG '(' logopts ')'",
"logopts : logopt",
"logopts : logopts comma logopt",
"logopt : ALL",
"logopt : MATCHES",
"logopt : USER",
"logopt : TO string",
"interface :",
"interface : ON if_item_not",
"interface : ON '{' optnl if_list '}'",
"if_list : if_item_not optnl",
"if_list : if_list comma if_item_not optnl",
"if_item_not : not if_item",
"if_item : STRING",
"if_item : ANY",
"if_item : RDOMAIN NUMBER",
"af :",
"af : INET",
"af : INET6",
"proto :",
"proto : PROTO proto_item",
"proto : PROTO '{' optnl proto_list '}'",
"proto_list : proto_item optnl",
"proto_list : proto_list comma proto_item optnl",
"proto_item : protoval",
"protoval : STRING",
"protoval : NUMBER",
"fromto : ALL",
"fromto : from os to",
"os :",
"os : OS xos",
"os : OS '{' optnl os_list '}'",
"xos : STRING",
"os_list : xos optnl",
"os_list : os_list comma xos optnl",
"from :",
"from : FROM ipportspec",
"to :",
"to : TO ipportspec",
"ipportspec : ipspec",
"ipportspec : ipspec PORT portspec",
"ipportspec : PORT portspec",
"optnl : '\\n' optnl",
"optnl :",
"ipspec : ANY",
"ipspec : xhost",
"ipspec : '{' optnl host_list '}'",
"host_list : ipspec optnl",
"host_list : host_list comma ipspec optnl",
"xhost : not host",
"xhost : not NOROUTE",
"xhost : not URPFFAILED",
"optweight : WEIGHT NUMBER",
"optweight :",
"host : STRING",
"host : STRING '-' STRING",
"host : STRING '/' NUMBER",
"host : NUMBER '/' NUMBER",
"host : dynaddr",
"host : dynaddr '/' NUMBER",
"host : '<' STRING '>'",
"host : ROUTE STRING",
"number : NUMBER",
"number : STRING",
"dynaddr : '(' STRING ')'",
"portspec : port_item",
"portspec : '{' optnl port_list '}'",
"port_list : port_item optnl",
"port_list : port_list comma port_item optnl",
"port_item : portrange",
"port_item : unaryop portrange",
"port_item : portrange PORTBINARY portrange",
"portplain : numberstring",
"portrange : numberstring",
"uids : uid_item",
"uids : '{' optnl uid_list '}'",
"uid_list : uid_item optnl",
"uid_list : uid_list comma uid_item optnl",
"uid_item : uid",
"uid_item : unaryop uid",
"uid_item : uid PORTBINARY uid",
"uid : STRING",
"uid : NUMBER",
"gids : gid_item",
"gids : '{' optnl gid_list '}'",
"gid_list : gid_item optnl",
"gid_list : gid_list comma gid_item optnl",
"gid_item : gid",
"gid_item : unaryop gid",
"gid_item : gid PORTBINARY gid",
"gid : STRING",
"gid : NUMBER",
"flag : STRING",
"flags : FLAGS flag '/' flag",
"flags : FLAGS '/' flag",
"flags : FLAGS ANY",
"icmpspec : ICMPTYPE icmp_item",
"icmpspec : ICMPTYPE '{' optnl icmp_list '}'",
"icmpspec : ICMP6TYPE icmp6_item",
"icmpspec : ICMP6TYPE '{' optnl icmp6_list '}'",
"icmp_list : icmp_item optnl",
"icmp_list : icmp_list comma icmp_item optnl",
"icmp6_list : icmp6_item optnl",
"icmp6_list : icmp6_list comma icmp6_item optnl",
"icmp_item : icmptype",
"icmp_item : icmptype CODE STRING",
"icmp_item : icmptype CODE NUMBER",
"icmp6_item : icmp6type",
"icmp6_item : icmp6type CODE STRING",
"icmp6_item : icmp6type CODE NUMBER",
"icmptype : STRING",
"icmptype : NUMBER",
"icmp6type : STRING",
"icmp6type : NUMBER",
"tos : STRING",
"tos : NUMBER",
"sourcetrack :",
"sourcetrack : GLOBAL",
"sourcetrack : RULE",
"statelock : IFBOUND",
"statelock : FLOATING",
"keep : NO STATE",
"keep : KEEP STATE state_opt_spec",
"keep : MODULATE STATE state_opt_spec",
"keep : SYNPROXY STATE state_opt_spec",
"flush :",
"flush : FLUSH",
"flush : FLUSH GLOBAL",
"state_opt_spec : '(' state_opt_list ')'",
"state_opt_spec :",
"state_opt_list : state_opt_item",
"state_opt_list : state_opt_list comma state_opt_item",
"state_opt_item : MAXIMUM NUMBER",
"state_opt_item : NOSYNC",
"state_opt_item : MAXSRCSTATES NUMBER",
"state_opt_item : MAXSRCCONN NUMBER",
"state_opt_item : MAXSRCCONNRATE NUMBER '/' NUMBER",
"state_opt_item : OVERLOAD '<' STRING '>' flush",
"state_opt_item : MAXSRCNODES NUMBER",
"state_opt_item : SOURCETRACK sourcetrack",
"state_opt_item : statelock",
"state_opt_item : SLOPPY",
"state_opt_item : PFLOW",
"state_opt_item : STRING NUMBER",
"label : STRING",
"qname : STRING",
"qname : '(' STRING ')'",
"qname : '(' STRING comma STRING ')'",
"portstar : numberstring",
"redirspec : host optweight",
"redirspec : '{' optnl redir_host_list '}'",
"redir_host_list : host optweight optnl",
"redir_host_list : redir_host_list comma host optweight optnl",
"redirpool : redirspec",
"redirpool : redirspec PORT portstar",
"hashkey :",
"hashkey : string",
"$$8 :",
"pool_opts : $$8 pool_opts_l",
"pool_opts :",
"pool_opts_l : pool_opts_l pool_opt",
"pool_opts_l : pool_opt",
"pool_opt : BITMASK",
"pool_opt : RANDOM",
"pool_opt : SOURCEHASH hashkey",
"pool_opt : ROUNDROBIN",
"pool_opt : LEASTSTATES",
"pool_opt : STATICPORT",
"pool_opt : STICKYADDRESS",
"routespec : redirspec pool_opts",
"timeout_spec : STRING NUMBER",
"timeout_list : timeout_list comma timeout_spec optnl",
"timeout_list : timeout_spec optnl",
"limit_spec : STRING NUMBER",
"limit_list : limit_list comma limit_spec optnl",
"limit_list : limit_spec optnl",
"comma : ','",
"comma :",
"yesno : NO",
"yesno : STRING",
"unaryop : '='",
"unaryop : NE",
"unaryop : LE",
"unaryop : '<'",
"unaryop : GE",
"unaryop : '>'",

};
#endif

#if YYDEBUG
int      yydebug;
#endif

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;
int      yynerrs;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#define YYINITSTACKSIZE 200

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;
#line 3787 "parse.y"

int
yyerror(const char *fmt, ...)
{
	va_list		 ap;

	file->errors++;
	va_start(ap, fmt);
	fprintf(stderr, "%s:%d: ", file->name, yylval.lineno);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	return (0);
}

int
validate_range(u_int8_t op, u_int16_t p1, u_int16_t p2)
{
	u_int16_t a = ntohs(p1);
	u_int16_t b = ntohs(p2);

	if ((op == PF_OP_RRG && a > b) ||  /* 34:12,  i.e. none */
	    (op == PF_OP_IRG && a >= b) || /* 34><12, i.e. none */
	    (op == PF_OP_XRG && a > b))    /* 34<>22, i.e. all */
		return 1;
	return 0;
}

int
disallow_table(struct node_host *h, const char *fmt)
{
	for (; h != NULL; h = h->next)
		if (h->addr.type == PF_ADDR_TABLE) {
			yyerror(fmt, h->addr.v.tblname);
			return (1);
		}
	return (0);
}

int
disallow_urpf_failed(struct node_host *h, const char *fmt)
{
	for (; h != NULL; h = h->next)
		if (h->addr.type == PF_ADDR_URPFFAILED) {
			yyerror("%s", fmt);
			return (1);
		}
	return (0);
}

int
disallow_alias(struct node_host *h, const char *fmt)
{
	for (; h != NULL; h = h->next)
		if (DYNIF_MULTIADDR(h->addr)) {
			yyerror(fmt, h->addr.v.tblname);
			return (1);
		}
	return (0);
}

int
rule_consistent(struct pf_rule *r)
{
	int	problems = 0;

	if (r->proto != IPPROTO_TCP && r->os_fingerprint != PF_OSFP_ANY) {
		yyerror("os only applies to tcp");
		problems++;
	}
	if (r->proto != IPPROTO_TCP && r->proto != IPPROTO_UDP &&
	    (r->src.port_op || r->dst.port_op)) {
		yyerror("port only applies to tcp/udp");
		problems++;
	}
	if (r->proto != IPPROTO_TCP && r->proto != IPPROTO_UDP &&
	    r->uid.op) {
		yyerror("user only applies to tcp/udp");
		problems++;
	}
	if (r->proto != IPPROTO_TCP && r->proto != IPPROTO_UDP &&
	    r->gid.op) {
		yyerror("group only applies to tcp/udp");
		problems++;
	}
	if (r->proto != IPPROTO_ICMP && r->proto != IPPROTO_ICMPV6 &&
	    (r->type || r->code)) {
		yyerror("icmp-type/code only applies to icmp");
		problems++;
	}
	if (!r->af && (r->type || r->code)) {
		yyerror("must indicate address family with icmp-type/code");
		problems++;
	}
	if (r->rule_flag & PFRULE_AFTO && r->af == r->naf) {
		yyerror("must indicate different address family with af-to");
		problems++;
	}
	if (r->overload_tblname[0] &&
	    r->max_src_conn == 0 && r->max_src_conn_rate.seconds == 0) {
		yyerror("'overload' requires 'max-src-conn' "
		    "or 'max-src-conn-rate'");
		problems++;
	}
	if ((r->proto == IPPROTO_ICMP && r->af == AF_INET6) ||
	    (r->proto == IPPROTO_ICMPV6 && r->af == AF_INET)) {
		yyerror("proto %s doesn't match address family %s",
		    r->proto == IPPROTO_ICMP ? "icmp" : "icmp6",
		    r->af == AF_INET ? "inet" : "inet6");
		problems++;
	}
	if (r->allow_opts && r->action != PF_PASS) {
		yyerror("allow-opts can only be specified for pass rules");
		problems++;
	}
	if (r->rule_flag & PFRULE_FRAGMENT && (r->src.port_op ||
	    r->dst.port_op || r->flagset || r->type || r->code)) {
		yyerror("fragments can be filtered only on IP header fields");
		problems++;
	}
	if (r->rule_flag & PFRULE_RETURNRST && r->proto != IPPROTO_TCP) {
		yyerror("return-rst can only be applied to TCP rules");
		problems++;
	}
	if (r->max_src_nodes && !(r->rule_flag & PFRULE_RULESRCTRACK)) {
		yyerror("max-src-nodes requires 'source-track rule'");
		problems++;
	}
	if (r->action != PF_PASS && r->keep_state) {
		yyerror("keep state is great, but only for pass rules");
		problems++;
	}
	if (r->rt && !r->keep_state) {
		yyerror("route-to, reply-to and dup-to require keep state");
		problems++;
	}
	if (r->rule_flag & PFRULE_STATESLOPPY &&
	    (r->keep_state == PF_STATE_MODULATE ||
	    r->keep_state == PF_STATE_SYNPROXY)) {
		yyerror("sloppy state matching cannot be used with "
		    "synproxy state or modulate state");
		problems++;
	}

	if ((r->keep_state == PF_STATE_SYNPROXY) && (r->direction != PF_IN))
		fprintf(stderr, "%s:%d: warning: "
		    "synproxy used for inbound rules only, "
		    "ignored for outbound\n", file->name, yylval.lineno);

	if ((r->nat.addr.type != PF_ADDR_NONE ||
	    r->rdr.addr.type != PF_ADDR_NONE) &&
	    r->action != PF_MATCH && !r->keep_state) {
		yyerror("nat-to and rdr-to require keep state");
		problems++;
	}
	if (r->direction == PF_INOUT && (r->nat.addr.type != PF_ADDR_NONE ||
	    r->rdr.addr.type != PF_ADDR_NONE)) {
		yyerror("nat-to and rdr-to require a direction");
		problems++;
	}
	if (r->af == AF_INET6 && (r->scrub_flags &
	    (PFSTATE_NODF|PFSTATE_RANDOMID))) {
		yyerror("address family inet6 does not support scrub options "
		    "no-df, random-id");
		problems++;
	}

	/* Basic rule sanity check. */
	switch (r->action) {
	case PF_MATCH:
		if (r->divert.type != PF_DIVERT_NONE) {
			yyerror("divert is not supported on match rules");
			problems++;
		}
		if (r->rt) {
			yyerror("route-to, reply-to and dup-to "
			   "are not supported on match rules");
			problems++;
		}
		if (r->rule_flag & PFRULE_AFTO) {
			yyerror("af-to is not supported on match rules");
			problems++;
		}
		break;
	case PF_DROP:
		if (r->rt) {
			yyerror("route-to, reply-to and dup-to "
			   "are not supported on block rules");
			problems++;
		}
		break;
	default:;
	}
	return (-problems);
}

int
process_tabledef(char *name, struct table_opts *opts, int popts)
{
	struct pfr_buffer	 ab;
	struct node_tinit	*ti;
	struct pfr_uktable	*ukt;

	bzero(&ab, sizeof(ab));
	ab.pfrb_type = PFRB_ADDRS;
	SIMPLEQ_FOREACH(ti, &opts->init_nodes, entries) {
		if (ti->file)
			if (pfr_buf_load(&ab, ti->file, 0, popts)) {
				if (errno)
					yyerror("cannot load \"%s\": %s",
					    ti->file, strerror(errno));
				else
					yyerror("file \"%s\" contains bad data",
					    ti->file);
				goto _error;
			}
		if (ti->host)
			if (append_addr_host(&ab, ti->host, 0, 0)) {
				yyerror("cannot create address buffer: %s",
				    strerror(errno));
				goto _error;
			}
	}
	if (pf->opts & PF_OPT_VERBOSE)
		print_tabledef(name, opts->flags, opts->init_addr,
		    &opts->init_nodes);
	if (!(pf->opts & PF_OPT_NOACTION) ||
	    (pf->opts & PF_OPT_DUMMYACTION))
		warn_duplicate_tables(name, pf->anchor->path);
	else if (pf->opts & PF_OPT_VERBOSE)
		fprintf(stderr, "%s:%d: skipping duplicate table checks"
		    " for <%s>\n", file->name, yylval.lineno, name);

	/*
	 * postpone definition of non-root tables to moment
	 * when path is fully resolved.
	 */
	if (pf->asd > 0) {
		ukt = calloc(1, sizeof(struct pfr_uktable));
		if (ukt == NULL) {
			DBGPRINT(
			    "%s:%d: not enough memory for <%s>\n", file->name,
			    yylval.lineno, name);
			goto _error;
		}
	} else
		ukt = NULL;

	if (!(pf->opts & PF_OPT_NOACTION) &&
	    pfctl_define_table(name, opts->flags, opts->init_addr,
	    pf->anchor->path, &ab, pf->anchor->ruleset.tticket, ukt)) {
		yyerror("cannot define table %s: %s", name,
		    pf_strerror(errno));
		goto _error;
	}

	if (ukt != NULL) {
		ukt->pfrukt_init_addr = opts->init_addr;
		if (RB_INSERT(pfr_ktablehead, &pfr_ktables,
		    &ukt->pfrukt_kt) != NULL) {
			/*
			 * I think this should not happen, because
			 * pfctl_define_table() above  does the same check
			 * effectively.
			 */
			DBGPRINT(
			    "%s:%d table %s already exists in %s\n",
			    file->name, yylval.lineno,
			    ukt->pfrukt_name, pf->anchor->path);
			free(ukt);
			goto _error;
		}
		DBGPRINT("%s %s@%s inserted to tree\n",
		    __func__, ukt->pfrukt_name, pf->anchor->path);

	} else
		DBGPRINT("%s ukt is null\n", __func__);

	pf->tdirty = 1;
	pfr_buf_clear(&ab);
	return (0);
_error:
	pfr_buf_clear(&ab);
	return (-1);
}

struct keywords {
	const char	*k_name;
	int		 k_val;
};

/* macro gore, but you should've seen the prior indentation nightmare... */

#define FREE_LIST(T,r) \
	do { \
		T *p, *node = r; \
		while (node != NULL) { \
			p = node; \
			node = node->next; \
			free(p); \
		} \
	} while (0)

#define LOOP_THROUGH(T,n,r,C) \
	do { \
		T *n; \
		if (r == NULL) { \
			r = calloc(1, sizeof(T)); \
			if (r == NULL) \
				err(1, "LOOP: calloc"); \
			r->next = NULL; \
		} \
		n = r; \
		while (n != NULL) { \
			do { \
				C; \
			} while (0); \
			n = n->next; \
		} \
	} while (0)

void
expand_label_str(char *label, size_t len, const char *srch, const char *repl)
{
	char *tmp;
	char *p, *q;

	if ((tmp = calloc(1, len)) == NULL)
		err(1, "%s", __func__);
	p = q = label;
	while ((q = strstr(p, srch)) != NULL) {
		*q = '\0';
		if ((strlcat(tmp, p, len) >= len) ||
		    (strlcat(tmp, repl, len) >= len))
			errx(1, "expand_label: label too long");
		q += strlen(srch);
		p = q;
	}
	if (strlcat(tmp, p, len) >= len)
		errx(1, "expand_label: label too long");
	strlcpy(label, tmp, len);	/* always fits */
	free(tmp);
}

void
expand_label_if(const char *name, char *label, size_t len, const char *ifname)
{
	if (strstr(label, name) != NULL) {
		if (!*ifname)
			expand_label_str(label, len, name, "any");
		else
			expand_label_str(label, len, name, ifname);
	}
}

void
expand_label_addr(const char *name, char *label, size_t len, sa_family_t af,
    struct node_host *h)
{
	char tmp[64], tmp_not[66];

	if (strstr(label, name) != NULL) {
		switch (h->addr.type) {
		case PF_ADDR_DYNIFTL:
			snprintf(tmp, sizeof(tmp), "(%s)", h->addr.v.ifname);
			break;
		case PF_ADDR_TABLE:
			snprintf(tmp, sizeof(tmp), "<%s>", h->addr.v.tblname);
			break;
		case PF_ADDR_NOROUTE:
			snprintf(tmp, sizeof(tmp), "no-route");
			break;
		case PF_ADDR_URPFFAILED:
			snprintf(tmp, sizeof(tmp), "urpf-failed");
			break;
		case PF_ADDR_ADDRMASK:
			if (!af || (PF_AZERO(&h->addr.v.a.addr, af) &&
			    PF_AZERO(&h->addr.v.a.mask, af)))
				snprintf(tmp, sizeof(tmp), "any");
			else {
				char	a[48];
				int	bits;

				if (inet_ntop(af, &h->addr.v.a.addr, a,
				    sizeof(a)) == NULL)
					snprintf(tmp, sizeof(tmp), "?");
				else {
					bits = unmask(&h->addr.v.a.mask);
					if ((af == AF_INET && bits < 32) ||
					    (af == AF_INET6 && bits < 128))
						snprintf(tmp, sizeof(tmp),
						    "%s/%d", a, bits);
					else
						snprintf(tmp, sizeof(tmp),
						    "%s", a);
				}
			}
			break;
		default:
			snprintf(tmp, sizeof(tmp), "?");
			break;
		}

		if (h->not) {
			snprintf(tmp_not, sizeof(tmp_not), "! %s", tmp);
			expand_label_str(label, len, name, tmp_not);
		} else
			expand_label_str(label, len, name, tmp);
	}
}

void
expand_label_port(const char *name, char *label, size_t len,
    struct node_port *port)
{
	char	 a1[6], a2[6], op[13] = "";

	if (strstr(label, name) != NULL) {
		snprintf(a1, sizeof(a1), "%u", ntohs(port->port[0]));
		snprintf(a2, sizeof(a2), "%u", ntohs(port->port[1]));
		if (!port->op)
			;
		else if (port->op == PF_OP_IRG)
			snprintf(op, sizeof(op), "%s><%s", a1, a2);
		else if (port->op == PF_OP_XRG)
			snprintf(op, sizeof(op), "%s<>%s", a1, a2);
		else if (port->op == PF_OP_EQ)
			snprintf(op, sizeof(op), "%s", a1);
		else if (port->op == PF_OP_NE)
			snprintf(op, sizeof(op), "!=%s", a1);
		else if (port->op == PF_OP_LT)
			snprintf(op, sizeof(op), "<%s", a1);
		else if (port->op == PF_OP_LE)
			snprintf(op, sizeof(op), "<=%s", a1);
		else if (port->op == PF_OP_GT)
			snprintf(op, sizeof(op), ">%s", a1);
		else if (port->op == PF_OP_GE)
			snprintf(op, sizeof(op), ">=%s", a1);
		expand_label_str(label, len, name, op);
	}
}

void
expand_label_proto(const char *name, char *label, size_t len, u_int8_t proto)
{
	struct protoent *pe;
	char n[4];

	if (strstr(label, name) != NULL) {
		pe = getprotobynumber(proto);
		if (pe != NULL)
			expand_label_str(label, len, name, pe->p_name);
		else {
			snprintf(n, sizeof(n), "%u", proto);
			expand_label_str(label, len, name, n);
		}
	}
}

void
pfctl_expand_label_nr(struct pf_rule *r, unsigned int rno)
{
	char n[11];

	snprintf(n, sizeof(n), "%u", rno);

	if (strstr(r->label, "$nr") != NULL)
		expand_label_str(r->label, PF_RULE_LABEL_SIZE, "$nr", n);

	if (strstr(r->tagname, "$nr") != NULL)
		expand_label_str(r->tagname, PF_TAG_NAME_SIZE, "$nr", n);

	if (strstr(r->match_tagname, "$nr") != NULL)
		expand_label_str(r->match_tagname, PF_TAG_NAME_SIZE, "$nr", n);
}

void
expand_label(char *label, size_t len, const char *ifname, sa_family_t af,
    struct node_host *src_host, struct node_port *src_port,
    struct node_host *dst_host, struct node_port *dst_port,
    u_int8_t proto)
{
	expand_label_if("$if", label, len, ifname);
	expand_label_addr("$srcaddr", label, len, af, src_host);
	expand_label_addr("$dstaddr", label, len, af, dst_host);
	expand_label_port("$srcport", label, len, src_port);
	expand_label_port("$dstport", label, len, dst_port);
	expand_label_proto("$proto", label, len, proto);
	/* rule number, '$nr', gets expanded after optimizer */
}

int
expand_queue(char *qname, struct node_if *interfaces, struct queue_opts *opts)
{
	struct pf_queuespec	qspec;

	LOOP_THROUGH(struct node_if, interface, interfaces,
		bzero(&qspec, sizeof(qspec));
		if (!opts->parent && (opts->marker & QOM_BWSPEC))
			opts->flags |= PFQS_ROOTCLASS;
		if (!(opts->marker & QOM_BWSPEC) &&
		    !(opts->marker & QOM_FLOWS)) {
			yyerror("no bandwidth or flow specification");
			return (1);
		}
		if (strlcpy(qspec.qname, qname, sizeof(qspec.qname)) >=
		    sizeof(qspec.qname)) {
			yyerror("queuename too long");
			return (1);
		}
		if (opts->parent && strlcpy(qspec.parent, opts->parent,
		    sizeof(qspec.parent)) >= sizeof(qspec.parent)) {
			yyerror("parent too long");
			return (1);
		}
		if (strlcpy(qspec.ifname, interface->ifname,
		    sizeof(qspec.ifname)) >= sizeof(qspec.ifname)) {
			yyerror("interface too long");
			return (1);
		}
		qspec.realtime.m1.absolute = opts->realtime.m1.bw_absolute;
		qspec.realtime.m1.percent = opts->realtime.m1.bw_percent;
		qspec.realtime.m2.absolute = opts->realtime.m2.bw_absolute;
		qspec.realtime.m2.percent = opts->realtime.m2.bw_percent;
		qspec.realtime.d = opts->realtime.d;

		qspec.linkshare.m1.absolute = opts->linkshare.m1.bw_absolute;
		qspec.linkshare.m1.percent = opts->linkshare.m1.bw_percent;
		qspec.linkshare.m2.absolute = opts->linkshare.m2.bw_absolute;
		qspec.linkshare.m2.percent = opts->linkshare.m2.bw_percent;
		qspec.linkshare.d = opts->linkshare.d;

		qspec.upperlimit.m1.absolute = opts->upperlimit.m1.bw_absolute;
		qspec.upperlimit.m1.percent = opts->upperlimit.m1.bw_percent;
		qspec.upperlimit.m2.absolute = opts->upperlimit.m2.bw_absolute;
		qspec.upperlimit.m2.percent = opts->upperlimit.m2.bw_percent;
		qspec.upperlimit.d = opts->upperlimit.d;

		qspec.flowqueue.flows = opts->flowqueue.flows;
		qspec.flowqueue.quantum = opts->flowqueue.quantum;
		qspec.flowqueue.interval = opts->flowqueue.interval;
		qspec.flowqueue.target = opts->flowqueue.target;

		qspec.flags = opts->flags;
		qspec.qlimit = opts->qlimit;

		if (pfctl_add_queue(pf, &qspec)) {
			yyerror("cannot add queue");
			return (1);
		}
	);

	FREE_LIST(struct node_if, interfaces);
	return (0);
}

int
expand_divertspec(struct pf_rule *r, struct divertspec *ds)
{
	struct node_host *n;

	switch (ds->type) {
	case PF_DIVERT_NONE:
		return (0);
	case PF_DIVERT_TO:
		if (r->direction == PF_OUT) {
			yyerror("divert-to used with outgoing rule");
			return (1);
		}
		if (r->af) {
			for (n = ds->addr; n != NULL; n = n->next)
				if (n->af == r->af)
					break;
			if (n == NULL) {
				yyerror("divert-to address family mismatch");
				return (1);
			}
			r->divert.addr = n->addr.v.a.addr;
		} else {
			r->af = ds->addr->af;
			r->divert.addr = ds->addr->addr.v.a.addr;
		}
		r->divert.port = ds->port;
		r->divert.type = ds->type;
		return (0);
	case PF_DIVERT_REPLY:
		if (r->direction == PF_IN) {
			yyerror("divert-reply used with incoming rule");
			return (1);
		}
		r->divert.type = ds->type;
		return (0);
	case PF_DIVERT_PACKET:
		r->divert.port = ds->port;
		r->divert.type = ds->type;
		return (0);
	}
	return (1);
}

int
collapse_redirspec(struct pf_pool *rpool, struct pf_rule *r,
    struct redirspec *rs, int routing)
{
	struct pf_opt_tbl *tbl = NULL;
	struct node_host *h, *hprev = NULL;
	struct pf_rule_addr ra;
	int af = 0, naddr = 0;

	if (!rs || !rs->rdr || rs->rdr->host == NULL) {
		rpool->addr.type = PF_ADDR_NONE;
		return (0);
	}

	if (r->rule_flag & PFRULE_AFTO)
		r->naf = rs->af;

	for (h = rs->rdr->host; h != NULL; h = h->next) {
		if (routing) {
			if (h->addr.type == PF_ADDR_DYNIFTL &&
			    h->addr.iflags != PFI_AFLAG_PEER) {
				yyerror("route spec requires :peer with "
				    "dynamic interface addresses");
				return (1);
			}
		}

		/* set rule address family if redirect spec has one */
		if (rs->af && !r->af && !af) {
			/* swap address families for af-to */
			if (r->naf == AF_INET6)
				af = AF_INET;
			else if (r->naf == AF_INET)
				af = AF_INET6;
			else
				af = rs->af;
		}
		if (h->af && !r->naf) {	/* nat-to/rdr-to case */
			/* skip if the rule af doesn't match redirect af */
			if (r->af && r->af != h->af)
				continue;
			/*
			 * fail if the chosen af is not universal for
			 * all addresses in the redirect address pool
			 */
			if (!r->af && af && af != h->af) {
				yyerror("%s spec contains addresses with "
				    "different address families",
				    routing ? "routing" : "translation");
				return (1);
			}
		} else if (h->af) {	/* af-to case */
			/*
			 * fail if the redirect spec af is not universal
			 * for all addresses in the redirect address pool
			 */
			if (rs->af && rs->af != h->af) {
				yyerror("%s spec contains addresses that "
				    "don't match target address family",
				    routing ? "routing" : "translation");
				return (1);
			}
		}
		/* else if (!h->af):
		 * we silently allow any not af-specific host specs,
		 * e.g. (em0) and let the kernel deal with them
		 */

		/* if we haven't selected the rule af yet, now it's time */
		if (!r->af && !af)
			af = h->af;

		if (naddr == 0) {	/* the first host */
			rpool->addr = h->addr;
			if (h->ifname) {
				yyerror("@if not permitted for %s",
				    routing ? "routing" : "translation");
				return (1);
			}
			if (h->ifname && strlcpy(rpool->ifname, h->ifname,
			    sizeof(rpool->ifname)) >= sizeof(rpool->ifname))
				errx(1, "collapse_redirspec: strlcpy");
			hprev = h; /* in case we need to conver to a table */
		} else {		/* multiple hosts */
			if (rs->pool_opts.type &&
			    !PF_POOL_DYNTYPE(rs->pool_opts.type)) {
				yyerror("pool type is not valid for multiple "
				    "translation or routing addresses");
				return (1);
			}
			if ((hprev && hprev->addr.type != PF_ADDR_ADDRMASK) &&
			    (hprev && hprev->addr.type != PF_ADDR_NONE) &&
			    h->addr.type != PF_ADDR_ADDRMASK &&
			    h->addr.type != PF_ADDR_NONE) {
				yyerror("multiple tables or dynamic interfaces "
				    "not supported for translation or routing");
				return (1);
			}
			if (h->ifname) {
				yyerror("@if not permitted for %s",
				    routing ? "routing" : "translation");
				return (1);
			}
			if (hprev) {
				/*
				 * undo some damage and convert the single
				 * host pool to the table
				 */
				memset(&ra, 0, sizeof(ra));
				memset(rpool->ifname, 0, sizeof(rpool->ifname));
				ra.addr = hprev->addr;
				ra.weight = hprev->weight;
				if (add_opt_table(pf, &tbl,
				    hprev->af, &ra, hprev->ifname))
					return (1);
				hprev = NULL;
			}
			memset(&ra, 0, sizeof(ra));
			ra.addr = h->addr;
			ra.weight = h->weight;
			if (add_opt_table(pf, &tbl,
			    h->af, &ra, h->ifname))
				return (1);
		}
		naddr++;
	}
	/* set rule af to the one chosen above */
	if (!r->af && af)
		r->af = af;
	if (!naddr) {
		yyerror("af mismatch in %s spec",
		    routing ? "routing" : "translation");
		return (1);
	}
	if (tbl) {
		if ((pf->opts & PF_OPT_NOACTION) == 0 &&
		    pf_opt_create_table(pf, tbl))
				return (1);

		pf->tdirty = 1;

		if (pf->opts & PF_OPT_VERBOSE)
			print_tabledef(tbl->pt_name,
			    PFR_TFLAG_CONST | tbl->pt_flags,
			    1, &tbl->pt_nodes);

		memset(&rpool->addr, 0, sizeof(rpool->addr));
		rpool->addr.type = PF_ADDR_TABLE;
		strlcpy(rpool->addr.v.tblname, tbl->pt_name,
		    sizeof(rpool->addr.v.tblname));

		pfr_buf_clear(tbl->pt_buf);
		free(tbl->pt_buf);
		tbl->pt_buf = NULL;
		free(tbl);
	}
	return (0);
}


int
apply_redirspec(struct pf_pool *rpool, struct pf_rule *r, struct redirspec *rs,
    int isrdr, struct node_port *np)
{
	if (!rs || !rs->rdr)
		return (0);

	rpool->proxy_port[0] = ntohs(rs->rdr->rport.a);

	if (isrdr) {
		if (!rs->rdr->rport.b && rs->rdr->rport.t) {
			rpool->proxy_port[1] = ntohs(rs->rdr->rport.a) +
			    (ntohs(np->port[1]) - ntohs(np->port[0]));
		} else {
			if (validate_range(rs->rdr->rport.t, rs->rdr->rport.a,
			    rs->rdr->rport.b)) {
				yyerror("invalid rdr-to port range");
				return (1);
			}

			rpool->port_op = rs->rdr->rport.t;
			rpool->proxy_port[1] = ntohs(rs->rdr->rport.b);
		}
	} else {
		rpool->proxy_port[1] = ntohs(rs->rdr->rport.b);
		if (!rpool->proxy_port[0] && !rpool->proxy_port[1]) {
			rpool->proxy_port[0] = PF_NAT_PROXY_PORT_LOW;
			rpool->proxy_port[1] = PF_NAT_PROXY_PORT_HIGH;
		} else if (!rpool->proxy_port[1])
			rpool->proxy_port[1] = rpool->proxy_port[0];
	}

	rpool->opts = rs->pool_opts.type;
	if ((rpool->opts & PF_POOL_TYPEMASK) == PF_POOL_NONE &&
	    (rpool->addr.type == PF_ADDR_TABLE ||
	    DYNIF_MULTIADDR(rpool->addr)))
		rpool->opts |= PF_POOL_ROUNDROBIN;

	if (!PF_POOL_DYNTYPE(rpool->opts) &&
	    (disallow_table(rs->rdr->host,
	    "tables are not supported by pool type") ||
	    disallow_alias(rs->rdr->host,
	    "interface (%s) is not supported by pool type")))
		return (1);

	if (rs->pool_opts.key != NULL)
		memcpy(&rpool->key, rs->pool_opts.key,
		    sizeof(struct pf_poolhashkey));

	if (rs->pool_opts.opts)
		rpool->opts |= rs->pool_opts.opts;

	if (rs->pool_opts.staticport) {
		if (isrdr) {
			yyerror("the 'static-port' option is only valid with "
			    "nat rules");
			return (1);
		}
		if (rpool->proxy_port[0] != PF_NAT_PROXY_PORT_LOW &&
		    rpool->proxy_port[1] != PF_NAT_PROXY_PORT_HIGH) {
			yyerror("the 'static-port' option can't be used when "
			    "specifying a port range");
			return (1);
		}
		rpool->proxy_port[0] = 0;
		rpool->proxy_port[1] = 0;
	}

	return (0);
}


void
expand_rule(struct pf_rule *r, int keeprule, struct node_if *interfaces,
    struct redirspec *nat, struct redirspec *rdr, struct redirspec *rroute,
    struct node_proto *protos, struct node_os *src_oses,
    struct node_host *src_hosts, struct node_port *src_ports,
    struct node_host *dst_hosts, struct node_port *dst_ports,
    struct node_uid *uids, struct node_gid *gids, struct node_if *rcv,
    struct node_icmp *icmp_types)
{
	sa_family_t		 af = r->af;
	int			 added = 0, error = 0;
	char			 ifname[IF_NAMESIZE];
	char			 label[PF_RULE_LABEL_SIZE];
	char			 tagname[PF_TAG_NAME_SIZE];
	char			 match_tagname[PF_TAG_NAME_SIZE];
	u_int8_t		 flags, flagset, keep_state;
	struct node_host	*srch, *dsth, *osrch, *odsth;
	struct redirspec	 binat;
	struct pf_rule		 rb;
	int			 dir = r->direction;

	if (strlcpy(label, r->label, sizeof(label)) >= sizeof(label))
		errx(1, "expand_rule: strlcpy");
	if (strlcpy(tagname, r->tagname, sizeof(tagname)) >= sizeof(tagname))
		errx(1, "expand_rule: strlcpy");
	if (strlcpy(match_tagname, r->match_tagname, sizeof(match_tagname)) >=
	    sizeof(match_tagname))
		errx(1, "expand_rule: strlcpy");
	flags = r->flags;
	flagset = r->flagset;
	keep_state = r->keep_state;

	r->src.addr.type = r->dst.addr.type = PF_ADDR_ADDRMASK;

	LOOP_THROUGH(struct node_if, interface, interfaces,
	LOOP_THROUGH(struct node_proto, proto, protos,
	LOOP_THROUGH(struct node_icmp, icmp_type, icmp_types,
	LOOP_THROUGH(struct node_host, src_host, src_hosts,
	LOOP_THROUGH(struct node_host, dst_host, dst_hosts,
	LOOP_THROUGH(struct node_port, src_port, src_ports,
	LOOP_THROUGH(struct node_port, dst_port, dst_ports,
	LOOP_THROUGH(struct node_os, src_os, src_oses,
	LOOP_THROUGH(struct node_uid, uid, uids,
	LOOP_THROUGH(struct node_gid, gid, gids,

		r->af = af;

		error += collapse_redirspec(&r->rdr, r, rdr, 0);
		error += collapse_redirspec(&r->nat, r, nat, 0);
		error += collapse_redirspec(&r->route, r, rroute, 1);

		/* disallow @if in from or to for the time being */
		if ((src_host->addr.type == PF_ADDR_ADDRMASK &&
		    src_host->ifname) ||
		    (dst_host->addr.type == PF_ADDR_ADDRMASK &&
		    dst_host->ifname)) {
			yyerror("@if syntax not permitted in from or to");
			error++;
		}
		/* for link-local IPv6 address, interface must match up */
		if ((r->af && src_host->af && r->af != src_host->af) ||
		    (r->af && dst_host->af && r->af != dst_host->af) ||
		    (src_host->af && dst_host->af &&
		    src_host->af != dst_host->af) ||
		    (src_host->ifindex && dst_host->ifindex &&
		    src_host->ifindex != dst_host->ifindex) ||
		    (src_host->ifindex && *interface->ifname &&
		    src_host->ifindex != ifa_nametoindex(interface->ifname)) ||
		    (dst_host->ifindex && *interface->ifname &&
		    dst_host->ifindex != ifa_nametoindex(interface->ifname)))
			continue;
		if (!r->af && src_host->af)
			r->af = src_host->af;
		else if (!r->af && dst_host->af)
			r->af = dst_host->af;

		if (*interface->ifname)
			strlcpy(r->ifname, interface->ifname,
			    sizeof(r->ifname));
		else if (ifa_indextoname(src_host->ifindex, ifname))
			strlcpy(r->ifname, ifname, sizeof(r->ifname));
		else if (ifa_indextoname(dst_host->ifindex, ifname))
			strlcpy(r->ifname, ifname, sizeof(r->ifname));
		else
			memset(r->ifname, '\0', sizeof(r->ifname));

		if (interface->use_rdomain)
			r->onrdomain = interface->rdomain;
		else
			r->onrdomain = -1;
		if (strlcpy(r->label, label, sizeof(r->label)) >=
		    sizeof(r->label))
			errx(1, "expand_rule: strlcpy");
		if (strlcpy(r->tagname, tagname, sizeof(r->tagname)) >=
		    sizeof(r->tagname))
			errx(1, "expand_rule: strlcpy");
		if (strlcpy(r->match_tagname, match_tagname,
		    sizeof(r->match_tagname)) >= sizeof(r->match_tagname))
			errx(1, "expand_rule: strlcpy");
		expand_label(r->label, PF_RULE_LABEL_SIZE, r->ifname, r->af,
		    src_host, src_port, dst_host, dst_port, proto->proto);
		expand_label(r->tagname, PF_TAG_NAME_SIZE, r->ifname, r->af,
		    src_host, src_port, dst_host, dst_port, proto->proto);
		expand_label(r->match_tagname, PF_TAG_NAME_SIZE, r->ifname,
		    r->af, src_host, src_port, dst_host, dst_port,
		    proto->proto);

		osrch = odsth = NULL;
		if (src_host->addr.type == PF_ADDR_DYNIFTL) {
			osrch = src_host;
			if ((src_host = gen_dynnode(src_host, r->af)) == NULL)
				err(1, "%s", __func__);
		}
		if (dst_host->addr.type == PF_ADDR_DYNIFTL) {
			odsth = dst_host;
			if ((dst_host = gen_dynnode(dst_host, r->af)) == NULL)
				err(1, "%s", __func__);
		}

		error += check_netmask(src_host, r->af);
		error += check_netmask(dst_host, r->af);

		r->ifnot = interface->not;
		r->proto = proto->proto;
		r->src.addr = src_host->addr;
		r->src.neg = src_host->not;
		r->src.port[0] = src_port->port[0];
		r->src.port[1] = src_port->port[1];
		r->src.port_op = src_port->op;
		r->dst.addr = dst_host->addr;
		r->dst.neg = dst_host->not;
		r->dst.port[0] = dst_port->port[0];
		r->dst.port[1] = dst_port->port[1];
		r->dst.port_op = dst_port->op;
		r->uid.op = uid->op;
		r->uid.uid[0] = uid->uid[0];
		r->uid.uid[1] = uid->uid[1];
		r->gid.op = gid->op;
		r->gid.gid[0] = gid->gid[0];
		r->gid.gid[1] = gid->gid[1];
		if (rcv) {
			strlcpy(r->rcv_ifname, rcv->ifname,
			    sizeof(r->rcv_ifname));
			r->rcvifnot = rcv->not;
		}
		r->type = icmp_type->type;
		r->code = icmp_type->code;

		if ((keep_state == PF_STATE_MODULATE ||
		    keep_state == PF_STATE_SYNPROXY) &&
		    r->proto && r->proto != IPPROTO_TCP)
			r->keep_state = PF_STATE_NORMAL;
		else
			r->keep_state = keep_state;

		if (r->proto && r->proto != IPPROTO_TCP) {
			r->flags = 0;
			r->flagset = 0;
		} else {
			r->flags = flags;
			r->flagset = flagset;
		}
		if (icmp_type->proto && r->proto != icmp_type->proto) {
			yyerror("icmp-type mismatch");
			error++;
		}

		if (src_os && src_os->os) {
			r->os_fingerprint = pfctl_get_fingerprint(src_os->os);
			if ((pf->opts & PF_OPT_VERBOSE2) &&
			    r->os_fingerprint == PF_OSFP_NOMATCH)
				fprintf(stderr,
				    "warning: unknown '%s' OS fingerprint\n",
				    src_os->os);
		} else {
			r->os_fingerprint = PF_OSFP_ANY;
		}

		if (nat && nat->rdr && nat->binat) {
			if (disallow_table(src_host, "invalid use of table "
			    "<%s> as the source address of a binat-to rule") ||
			    disallow_alias(src_host, "invalid use of interface "
			    "(%s) as the source address of a binat-to rule")) {
				error++;
			} else if ((r->src.addr.type != PF_ADDR_ADDRMASK &&
			    r->src.addr.type != PF_ADDR_DYNIFTL) ||
			    (r->nat.addr.type != PF_ADDR_ADDRMASK &&
			    r->nat.addr.type != PF_ADDR_DYNIFTL)) {
				yyerror("binat-to requires a specified "
				    "source and redirect address");
				error++;
			}
			if (DYNIF_MULTIADDR(r->src.addr) ||
			    DYNIF_MULTIADDR(r->nat.addr)) {
				yyerror ("dynamic interfaces must be used with "
				    ":0 in a binat-to rule");
				error++;
			}
			if (PF_AZERO(&r->src.addr.v.a.mask, af) ||
			    PF_AZERO(&r->nat.addr.v.a.mask, af)) {
				yyerror ("source and redir addresess must have "
				    "a matching network mask in binat-rule");
				error++;
			}
			if (r->nat.addr.type == PF_ADDR_TABLE) {
				yyerror ("tables cannot be used as the redirect "
				    "address of a binat-to rule");
				error++;
			}
			if (r->direction != PF_INOUT) {
				yyerror("binat-to cannot be specified "
				    "with a direction");
				error++;
			}

			/* first specify outbound NAT rule */
			r->direction = PF_OUT;
		}

		error += apply_redirspec(&r->nat, r, nat, 0, dst_port);
		error += apply_redirspec(&r->rdr, r, rdr, 1, dst_port);
		error += apply_redirspec(&r->route, r, rroute, 2, dst_port);

		if (rule_consistent(r) < 0 || error)
			yyerror("skipping rule due to errors");
		else {
			r->nr = pf->astack[pf->asd]->match++;
			pfctl_add_rule(pf, r);
			added++;
		}
		r->direction = dir;

		/* Generate binat's matching inbound rule */
		if (!error && nat && nat->rdr && nat->binat) {
			bcopy(r, &rb, sizeof(rb));

			/* now specify inbound rdr rule */
			rb.direction = PF_IN;

			if ((srch = calloc(1, sizeof(*srch))) == NULL)
				err(1, "%s", __func__);
			bcopy(src_host, srch, sizeof(*srch));
			srch->ifname = NULL;
			srch->next = NULL;
			srch->tail = NULL;

			if ((dsth = calloc(1, sizeof(*dsth))) == NULL)
				err(1, "%s", __func__);
			bcopy(&rb.nat.addr, &dsth->addr, sizeof(dsth->addr));
			dsth->ifname = NULL;
			dsth->next = NULL;
			dsth->tail = NULL;

			bzero(&binat, sizeof(binat));
			if ((binat.rdr =
			    calloc(1, sizeof(*binat.rdr))) == NULL)
				err(1, "%s", __func__);
			bcopy(nat->rdr, binat.rdr, sizeof(*binat.rdr));
			bcopy(&nat->pool_opts, &binat.pool_opts,
			    sizeof(binat.pool_opts));
			binat.pool_opts.staticport = 0;
			binat.rdr->host = srch;

			expand_rule(&rb, 1, interface, NULL, &binat, NULL,
			    proto,
			    src_os, dst_host, dst_port, dsth, src_port,
			    uid, gid, rcv, icmp_type);
		}

		if (osrch && src_host->addr.type == PF_ADDR_DYNIFTL) {
			free(src_host);
			src_host = osrch;
		}
		if (odsth && dst_host->addr.type == PF_ADDR_DYNIFTL) {
			free(dst_host);
			dst_host = odsth;
		}
	))))))))));

	if (!keeprule) {
		FREE_LIST(struct node_if, interfaces);
		FREE_LIST(struct node_proto, protos);
		FREE_LIST(struct node_host, src_hosts);
		FREE_LIST(struct node_port, src_ports);
		FREE_LIST(struct node_os, src_oses);
		FREE_LIST(struct node_host, dst_hosts);
		FREE_LIST(struct node_port, dst_ports);
		FREE_LIST(struct node_uid, uids);
		FREE_LIST(struct node_gid, gids);
		FREE_LIST(struct node_icmp, icmp_types);
		if (nat && nat->rdr)
			FREE_LIST(struct node_host, nat->rdr->host);
		if (rdr && rdr->rdr)
			FREE_LIST(struct node_host, rdr->rdr->host);

	}

	if (!added)
		yyerror("rule expands to no valid combination");
}

int
expand_skip_interface(struct node_if *interfaces)
{
	int	errs = 0;

	if (!interfaces || (!interfaces->next && !interfaces->not &&
	    !strcmp(interfaces->ifname, "none"))) {
		if (pf->opts & PF_OPT_VERBOSE)
			printf("set skip on none\n");
		errs = pfctl_set_interface_flags(pf, "", PFI_IFLAG_SKIP, 0);
		return (errs);
	}

	if (pf->opts & PF_OPT_VERBOSE)
		printf("set skip on {");
	LOOP_THROUGH(struct node_if, interface, interfaces,
		if (pf->opts & PF_OPT_VERBOSE)
			printf(" %s", interface->ifname);
		if (interface->not) {
			yyerror("skip on ! <interface> is not supported");
			errs++;
		} else if (interface->use_rdomain) {
			yyerror("skip on rdomain <num> is not supported");
			errs++;
		} else
			errs += pfctl_set_interface_flags(pf,
			    interface->ifname, PFI_IFLAG_SKIP, 1);
	);
	if (pf->opts & PF_OPT_VERBOSE)
		printf(" }\n");

	FREE_LIST(struct node_if, interfaces);

	if (errs)
		return (1);
	else
		return (0);
}

void
freehostlist(struct node_host *h)
{
	struct node_host *n;

	for (n = h; n != NULL; n = n->next)
		if (n->ifname)
			free(n->ifname);
	FREE_LIST(struct node_host, h);
}

#undef FREE_LIST
#undef LOOP_THROUGH

int
kw_cmp(const void *k, const void *e)
{
	return (strcmp(k, ((const struct keywords *)e)->k_name));
}

int
lookup(char *s)
{
	/* this has to be sorted always */
	static const struct keywords keywords[] = {
		{ "af-to",		AFTO},
		{ "all",		ALL},
		{ "allow-opts",		ALLOWOPTS},
		{ "anchor",		ANCHOR},
		{ "antispoof",		ANTISPOOF},
		{ "any",		ANY},
		{ "bandwidth",		BANDWIDTH},
		{ "binat-to",		BINATTO},
		{ "bitmask",		BITMASK},
		{ "block",		BLOCK},
		{ "block-policy",	BLOCKPOLICY},
		{ "burst",		BURST},
		{ "code",		CODE},
		{ "debug",		DEBUG},
		{ "default",		DEFAULT},
		{ "delay",		DELAY},
		{ "divert-packet",	DIVERTPACKET},
		{ "divert-reply",	DIVERTREPLY},
		{ "divert-to",		DIVERTTO},
		{ "drop",		DROP},
		{ "dup-to",		DUPTO},
		{ "file",		FILENAME},
		{ "fingerprints",	FINGERPRINTS},
		{ "flags",		FLAGS},
		{ "floating",		FLOATING},
		{ "flows",		FLOWS},
		{ "flush",		FLUSH},
		{ "for",		FOR},
		{ "fragment",		FRAGMENT},
		{ "from",		FROM},
		{ "global",		GLOBAL},
		{ "group",		GROUP},
		{ "hostid",		HOSTID},
		{ "icmp-type",		ICMPTYPE},
		{ "icmp6-type",		ICMP6TYPE},
		{ "if-bound",		IFBOUND},
		{ "in",			IN},
		{ "include",		INCLUDE},
		{ "inet",		INET},
		{ "inet6",		INET6},
		{ "keep",		KEEP},
		{ "label",		LABEL},
		{ "least-states",	LEASTSTATES},
		{ "limit",		LIMIT},
		{ "load",		LOAD},
		{ "log",		LOG},
		{ "loginterface",	LOGINTERFACE},
		{ "match",		MATCH},
		{ "matches",		MATCHES},
		{ "max",		MAXIMUM},
		{ "max-mss",		MAXMSS},
		{ "max-pkt-rate",	MAXPKTRATE},
		{ "max-src-conn",	MAXSRCCONN},
		{ "max-src-conn-rate",	MAXSRCCONNRATE},
		{ "max-src-nodes",	MAXSRCNODES},
		{ "max-src-states",	MAXSRCSTATES},
		{ "min",		MINIMUM},
		{ "min-ttl",		MINTTL},
		{ "modulate",		MODULATE},
		{ "nat-to",		NATTO},
		{ "no",			NO},
		{ "no-df",		NODF},
		{ "no-route",		NOROUTE},
		{ "no-sync",		NOSYNC},
		{ "on",			ON},
		{ "once",		ONCE},
		{ "optimization",	OPTIMIZATION},
		{ "os",			OS},
		{ "out",		OUT},
		{ "overload",		OVERLOAD},
		{ "parent",		PARENT},
		{ "pass",		PASS},
		{ "pflow",		PFLOW},
		{ "port",		PORT},
		{ "prio",		PRIO},
		{ "probability",	PROBABILITY},
		{ "proto",		PROTO},
		{ "qlimit",		QLIMIT},
		{ "quantum",		QUANTUM},
		{ "queue",		QUEUE},
		{ "quick",		QUICK},
		{ "random",		RANDOM},
		{ "random-id",		RANDOMID},
		{ "rdomain",		RDOMAIN},
		{ "rdr-to",		RDRTO},
		{ "reassemble",		REASSEMBLE},
		{ "received-on",	RECEIVEDON},
		{ "reply-to",		REPLYTO},
		{ "return",		RETURN},
		{ "return-icmp",	RETURNICMP},
		{ "return-icmp6",	RETURNICMP6},
		{ "return-rst",		RETURNRST},
		{ "round-robin",	ROUNDROBIN},
		{ "route",		ROUTE},
		{ "route-to",		ROUTETO},
		{ "rtable",		RTABLE},
		{ "rule",		RULE},
		{ "ruleset-optimization",	RULESET_OPTIMIZATION},
		{ "scrub",		SCRUB},
		{ "set",		SET},
		{ "skip",		SKIP},
		{ "sloppy",		SLOPPY},
		{ "source-hash",	SOURCEHASH},
		{ "source-track",	SOURCETRACK},
		{ "state",		STATE},
		{ "state-defaults",	STATEDEFAULTS},
		{ "state-policy",	STATEPOLICY},
		{ "static-port",	STATICPORT},
		{ "sticky-address",	STICKYADDRESS},
		{ "syncookies",		SYNCOOKIES},
		{ "synproxy",		SYNPROXY},
		{ "table",		TABLE},
		{ "tag",		TAG},
		{ "tagged",		TAGGED},
		{ "timeout",		TIMEOUT},
		{ "to",			TO},
		{ "tos",		TOS},
		{ "ttl",		TTL},
		{ "urpf-failed",	URPFFAILED},
		{ "user",		USER},
		{ "weight",		WEIGHT},
	};
	const struct keywords	*p;

	p = bsearch(s, keywords, sizeof(keywords)/sizeof(keywords[0]),
	    sizeof(keywords[0]), kw_cmp);

	if (p) {
		if (debug > 1)
			fprintf(stderr, "%s: %d\n", s, p->k_val);
		return (p->k_val);
	} else {
		if (debug > 1)
			fprintf(stderr, "string: %s\n", s);
		return (STRING);
	}
}

#define START_EXPAND	1
#define DONE_EXPAND	2

static int expanding;

int
igetc(void)
{
	int c;
	while (1) {
		if (file->ungetpos > 0)
			c = file->ungetbuf[--file->ungetpos];
		else
			c = getc(file->stream);
		if (c == START_EXPAND)
			expanding = 1;
		else if (c == DONE_EXPAND)
			expanding = 0;
		else
			break;
	}
	return (c);
}

int
lgetc(int quotec)
{
	int		c, next;

	if (quotec) {
		if ((c = igetc()) == EOF) {
			yyerror("reached end of file while parsing quoted string");
			if (file == topfile || popfile() == EOF)
				return (EOF);
			return (quotec);
		}
		return (c);
	}

	while ((c = igetc()) == '\\') {
		next = igetc();
		if (next != '\n') {
			c = next;
			break;
		}
		yylval.lineno = file->lineno;
		file->lineno++;
	}

	if (c == EOF) {
		/*
		 * Fake EOL when hit EOF for the first time. This gets line
		 * count right if last line in included file is syntactically
		 * invalid and has no newline.
		 */
		if (file->eof_reached == 0) {
			file->eof_reached = 1;
			return ('\n');
		}
		while (c == EOF) {
			if (file == topfile || popfile() == EOF)
				return (EOF);
			c = igetc();
		}
	}

	return (c);
}

void
lungetc(int c)
{
	if (c == EOF)
		return;
	if (file->ungetpos >= file->ungetsize) {
		void *p = reallocarray(file->ungetbuf, file->ungetsize, 2);
		if (p == NULL)
			err(1, "%s", __func__);
		file->ungetbuf = p;
		file->ungetsize *= 2;
	}
	file->ungetbuf[file->ungetpos++] = c;
}

int
findeol(void)
{
	int	c;

	/* skip to either EOF or the first real EOL */
	while (1) {
		c = lgetc(0);
		if (c == '\n') {
			file->lineno++;
			break;
		}
		if (c == EOF)
			break;
	}
	return (ERROR);
}

int
yylex(void)
{
	char	 buf[8096];
	char	*p, *val;
	int	 quotec, next, c;
	int	 token;

top:
	p = buf;
	while ((c = lgetc(0)) == ' ' || c == '\t')
		; /* nothing */

	yylval.lineno = file->lineno;
	if (c == '#')
		while ((c = lgetc(0)) != '\n' && c != EOF)
			; /* nothing */
	if (c == '$' && !expanding) {
		while (1) {
			if ((c = lgetc(0)) == EOF)
				return (0);

			if (p + 1 >= buf + sizeof(buf) - 1) {
				yyerror("string too long");
				return (findeol());
			}
			if (isalnum(c) || c == '_') {
				*p++ = c;
				continue;
			}
			*p = '\0';
			lungetc(c);
			break;
		}
		val = symget(buf);
		if (val == NULL) {
			yyerror("macro '%s' not defined", buf);
			return (findeol());
		}
		p = val + strlen(val) - 1;
		lungetc(DONE_EXPAND);
		while (p >= val) {
			lungetc((unsigned char)*p);
			p--;
		}
		lungetc(START_EXPAND);
		goto top;
	}

	switch (c) {
	case '\'':
	case '"':
		quotec = c;
		while (1) {
			if ((c = lgetc(quotec)) == EOF)
				return (0);
			if (c == '\n') {
				file->lineno++;
				continue;
			} else if (c == '\\') {
				if ((next = lgetc(quotec)) == EOF)
					return (0);
				if (next == quotec || next == ' ' ||
				    next == '\t')
					c = next;
				else if (next == '\n') {
					file->lineno++;
					continue;
				} else
					lungetc(next);
			} else if (c == quotec) {
				*p = '\0';
				break;
			} else if (c == '\0') {
				yyerror("syntax error");
				return (findeol());
			}
			if (p + 1 >= buf + sizeof(buf) - 1) {
				yyerror("string too long");
				return (findeol());
			}
			*p++ = c;
		}
		yylval.v.string = strdup(buf);
		if (yylval.v.string == NULL)
			err(1, "%s", __func__);
		return (STRING);
	case '!':
		next = lgetc(0);
		if (next == '=')
			return (NE);
		lungetc(next);
		break;
	case '<':
		next = lgetc(0);
		if (next == '>') {
			yylval.v.i = PF_OP_XRG;
			return (PORTBINARY);
		} else if (next == '=')
			return (LE);
		lungetc(next);
		break;
	case '>':
		next = lgetc(0);
		if (next == '<') {
			yylval.v.i = PF_OP_IRG;
			return (PORTBINARY);
		} else if (next == '=')
			return (GE);
		lungetc(next);
		break;
	}

#define allowed_to_end_number(x) \
	(isspace(x) || x == ')' || x ==',' || x == '/' || x == '}' || x == '=')

	if (c == '-' || isdigit(c)) {
		do {
			*p++ = c;
			if ((size_t)(p-buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(0)) != EOF && isdigit(c));
		lungetc(c);
		if (p == buf + 1 && buf[0] == '-')
			goto nodigits;
		if (c == EOF || allowed_to_end_number(c)) {
			const char *errstr = NULL;

			*p = '\0';
			yylval.v.number = strtonum(buf, LLONG_MIN,
			    LLONG_MAX, &errstr);
			if (errstr) {
				yyerror("\"%s\" invalid number: %s",
				    buf, errstr);
				return (findeol());
			}
			return (NUMBER);
		} else {
nodigits:
			while (p > buf + 1)
				lungetc((unsigned char)*--p);
			c = (unsigned char)*--p;
			if (c == '-')
				return (c);
		}
	}

#define allowed_in_string(x) \
	(isalnum(x) || (ispunct(x) && x != '(' && x != ')' && \
	x != '{' && x != '}' && x != '<' && x != '>' && \
	x != '!' && x != '=' && x != '/' && x != '#' && \
	x != ','))

	if (isalnum(c) || c == ':' || c == '_') {
		do {
			*p++ = c;
			if ((size_t)(p-buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(0)) != EOF && (allowed_in_string(c)));
		lungetc(c);
		*p = '\0';
		if ((token = lookup(buf)) == STRING)
			if ((yylval.v.string = strdup(buf)) == NULL)
				err(1, "%s", __func__);
		return (token);
	}
	if (c == '\n') {
		yylval.lineno = file->lineno;
		file->lineno++;
	}
	if (c == EOF)
		return (0);
	return (c);
}

int
check_file_secrecy(int fd, const char *fname)
{
	struct stat	st;

	if (fstat(fd, &st)) {
		warn("cannot stat %s", fname);
		return (-1);
	}
	if (st.st_uid != 0 && st.st_uid != getuid()) {
		warnx("%s: owner not root or current user", fname);
		return (-1);
	}
	if (st.st_mode & (S_IWGRP | S_IXGRP | S_IRWXO)) {
		warnx("%s: group writable or world read/writable", fname);
		return (-1);
	}
	return (0);
}

struct file *
pushfile(const char *name, int secret)
{
	struct file	*nfile;

	if ((nfile = calloc(1, sizeof(struct file))) == NULL ||
	    (nfile->name = strdup(name)) == NULL) {
		warn("%s", __func__);
		if (nfile)
			free(nfile);
		return (NULL);
	}
	if (TAILQ_FIRST(&files) == NULL && strcmp(nfile->name, "-") == 0) {
		nfile->stream = stdin;
		free(nfile->name);
		if ((nfile->name = strdup("stdin")) == NULL) {
			warn("%s", __func__);
			free(nfile);
			return (NULL);
		}
	} else if ((nfile->stream = pfctl_fopen(nfile->name, "r")) == NULL) {
		warn("%s: %s", __func__, nfile->name);
		free(nfile->name);
		free(nfile);
		return (NULL);
	} else if (secret &&
	    check_file_secrecy(fileno(nfile->stream), nfile->name)) {
		fclose(nfile->stream);
		free(nfile->name);
		free(nfile);
		return (NULL);
	}
	nfile->lineno = TAILQ_EMPTY(&files) ? 1 : 0;
	nfile->ungetsize = 16;
	nfile->ungetbuf = malloc(nfile->ungetsize);
	if (nfile->ungetbuf == NULL) {
		warn("%s", __func__);
		fclose(nfile->stream);
		free(nfile->name);
		free(nfile);
		return (NULL);
	}
	TAILQ_INSERT_TAIL(&files, nfile, entry);
	return (nfile);
}

int
popfile(void)
{
	struct file	*prev;

	if ((prev = TAILQ_PREV(file, files, entry)) != NULL)
		prev->errors += file->errors;

	TAILQ_REMOVE(&files, file, entry);
	fclose(file->stream);
	free(file->name);
	free(file->ungetbuf);
	free(file);
	file = prev;

	return (file ? 0 : EOF);
}

int
parse_config(char *filename, struct pfctl *xpf)
{
	int		 errors = 0;
	struct sym	*sym;

	pf = xpf;
	returnicmpdefault = (ICMP_UNREACH << 8) | ICMP_UNREACH_PORT;
	returnicmp6default =
	    (ICMP6_DST_UNREACH << 8) | ICMP6_DST_UNREACH_NOPORT;
	blockpolicy = PFRULE_DROP;

	if ((file = pushfile(filename, 0)) == NULL) {
		warn("cannot open the main config file!");
		return (-1);
	}
	topfile = file;

	yyparse();
	errors = file->errors;
	popfile();

	/* Free macros and check which have not been used. */
	while ((sym = TAILQ_FIRST(&symhead))) {
		if ((pf->opts & PF_OPT_VERBOSE2) && !sym->used)
			fprintf(stderr, "warning: macro '%s' not "
			    "used\n", sym->nam);
		free(sym->nam);
		free(sym->val);
		TAILQ_REMOVE(&symhead, sym, entry);
		free(sym);
	}

	return (errors ? -1 : 0);
}

int
symset(const char *nam, const char *val, int persist)
{
	struct sym	*sym;

	TAILQ_FOREACH(sym, &symhead, entry) {
		if (strcmp(nam, sym->nam) == 0)
			break;
	}

	if (sym != NULL) {
		if (sym->persist == 1)
			return (0);
		else {
			free(sym->nam);
			free(sym->val);
			TAILQ_REMOVE(&symhead, sym, entry);
			free(sym);
		}
	}
	if ((sym = calloc(1, sizeof(*sym))) == NULL)
		return (-1);

	sym->nam = strdup(nam);
	if (sym->nam == NULL) {
		free(sym);
		return (-1);
	}
	sym->val = strdup(val);
	if (sym->val == NULL) {
		free(sym->nam);
		free(sym);
		return (-1);
	}
	sym->used = 0;
	sym->persist = persist;
	TAILQ_INSERT_TAIL(&symhead, sym, entry);
	return (0);
}

int
pfctl_cmdline_symset(char *s)
{
	char	*sym, *val;
	int	 ret;

	if ((val = strrchr(s, '=')) == NULL)
		return (-1);

	sym = strndup(s, val - s);
	if (sym == NULL)
		err(1, "%s", __func__);
	ret = symset(sym, val + 1, 1);
	free(sym);

	return (ret);
}

char *
symget(const char *nam)
{
	struct sym	*sym;

	TAILQ_FOREACH(sym, &symhead, entry) {
		if (strcmp(nam, sym->nam) == 0) {
			sym->used = 1;
			return (sym->val);
		}
	}
	return (NULL);
}

void
mv_rules(struct pf_ruleset *src, struct pf_ruleset *dst)
{
	struct pf_rule *r;

	TAILQ_FOREACH(r, src->rules.active.ptr, entries)
		dst->anchor->match++;
	TAILQ_CONCAT(dst->rules.active.ptr, src->rules.active.ptr, entries);
	src->anchor->match = 0;
	TAILQ_CONCAT(dst->rules.inactive.ptr, src->rules.inactive.ptr, entries);
}

void
mv_tables(struct pfctl *pf, struct pfr_ktablehead *ktables,
    struct pf_anchor *a, struct pf_anchor *alast)
{

	struct pfr_ktable *kt, *kt_safe;
	char new_path[PF_ANCHOR_MAXPATH];
	char *path_cut;
	int sz;
	struct pfr_uktable *ukt;
	SLIST_HEAD(, pfr_uktable) ukt_list;

	/*
	 * Here we need to rename anchor path from temporal names such as
	 * _1/_2/foo to _1/bar/foo etc.
	 *
	 * This also means we need to remove and insert table to ktables
	 * tree as anchor path is being updated.
	 */
	SLIST_INIT(&ukt_list);
	DBGPRINT("%s [ %s ] (%s)\n", __func__, a->path, alast->path);
	RB_FOREACH_SAFE(kt, pfr_ktablehead, ktables, kt_safe) {
		path_cut = strstr(kt->pfrkt_anchor, alast->path);
		if (path_cut != NULL) {
			path_cut += strlen(alast->path);
			if (*path_cut)
				sz = snprintf(new_path, sizeof (new_path),
				    "%s%s", a->path, path_cut);
			else
				sz = snprintf(new_path, sizeof (new_path),
				    "%s", a->path);
			if (sz >= sizeof (new_path))
				errx(1, "new path is too long for %s@%s\n",
				    kt->pfrkt_name, kt->pfrkt_anchor);

			DBGPRINT("%s %s@%s -> %s@%s\n", __func__,
			    kt->pfrkt_name, kt->pfrkt_anchor,
			    kt->pfrkt_name, new_path);
			RB_REMOVE(pfr_ktablehead, ktables, kt);
			strlcpy(kt->pfrkt_anchor, new_path,
			    sizeof(kt->pfrkt_anchor));
			SLIST_INSERT_HEAD(&ukt_list, (struct pfr_uktable *)kt,
			    pfrukt_entry);
		}
	}

	while ((ukt = SLIST_FIRST(&ukt_list)) != NULL) {
		SLIST_REMOVE_HEAD(&ukt_list, pfrukt_entry);
		if (RB_INSERT(pfr_ktablehead, ktables,
		    (struct pfr_ktable *)ukt) != NULL)
			errx(1, "%s@%s exists already\n",
			    ukt->pfrukt_name,
			    ukt->pfrukt_anchor);
	}
}

void
decide_address_family(struct node_host *n, sa_family_t *af)
{
	if (*af != 0 || n == NULL)
		return;
	*af = n->af;
	while ((n = n->next) != NULL) {
		if (n->af != *af) {
			*af = 0;
			return;
		}
	}
}

int
invalid_redirect(struct node_host *nh, sa_family_t af)
{
	if (!af) {
		struct node_host *n;

		/* tables and dyniftl are ok without an address family */
		for (n = nh; n != NULL; n = n->next) {
			if (n->addr.type != PF_ADDR_TABLE &&
			    n->addr.type != PF_ADDR_DYNIFTL) {
				yyerror("address family not given and "
				    "translation address expands to multiple "
				    "address families");
				return (1);
			}
		}
	}
	if (nh == NULL) {
		yyerror("no translation address with matching address family "
		    "found.");
		return (1);
	}
	return (0);
}

int
atoul(char *s, u_long *ulvalp)
{
	u_long	 ulval;
	char	*ep;

	errno = 0;
	ulval = strtoul(s, &ep, 0);
	if (s[0] == '\0' || *ep != '\0')
		return (-1);
	if (errno == ERANGE && ulval == ULONG_MAX)
		return (-1);
	*ulvalp = ulval;
	return (0);
}

int
getservice(char *n)
{
	struct servent	*s;
	u_long		 ulval;

	if (atoul(n, &ulval) == 0) {
		if (ulval > 65535) {
			yyerror("illegal port value %lu", ulval);
			return (-1);
		}
		return (htons(ulval));
	} else {
		s = getservbyname(n, "tcp");
		if (s == NULL)
			s = getservbyname(n, "udp");
		if (s == NULL) {
			yyerror("unknown port %s", n);
			return (-1);
		}
		return (s->s_port);
	}
}

int
rule_label(struct pf_rule *r, char *s)
{
	if (s) {
		if (strlcpy(r->label, s, sizeof(r->label)) >=
		    sizeof(r->label)) {
			yyerror("rule label too long (max %zu chars)",
			    sizeof(r->label)-1);
			return (-1);
		}
	}
	return (0);
}

u_int16_t
parseicmpspec(char *w, sa_family_t af)
{
	const struct icmpcodeent	*p;
	u_long				 ulval;
	u_int8_t			 icmptype;

	if (af == AF_INET)
		icmptype = returnicmpdefault >> 8;
	else
		icmptype = returnicmp6default >> 8;

	if (atoul(w, &ulval) == -1) {
		if ((p = geticmpcodebyname(icmptype, w, af)) == NULL) {
			yyerror("unknown icmp code %s", w);
			return (0);
		}
		ulval = p->code;
	}
	if (ulval > 255) {
		yyerror("invalid icmp code %lu", ulval);
		return (0);
	}
	return (icmptype << 8 | ulval);
}

int
parseport(char *port, struct range *r, int extensions)
{
	char	*p = strchr(port, ':');

	if (p == NULL) {
		if ((r->a = getservice(port)) == -1)
			return (-1);
		r->b = 0;
		r->t = PF_OP_NONE;
		return (0);
	}
	if ((extensions & PPORT_STAR) && !strcmp(p+1, "*")) {
		*p = 0;
		if ((r->a = getservice(port)) == -1)
			return (-1);
		r->b = 0;
		r->t = PF_OP_IRG;
		return (0);
	}
	if ((extensions & PPORT_RANGE)) {
		*p++ = 0;
		if ((r->a = getservice(port)) == -1 ||
		    (r->b = getservice(p)) == -1)
			return (-1);
		if (r->a == r->b) {
			r->b = 0;
			r->t = PF_OP_NONE;
		} else
			r->t = PF_OP_RRG;
		return (0);
	}
	yyerror("port is invalid: %s", port);
	return (-1);
}

int
pfctl_load_anchors(int dev, struct pfctl *pf)
{
	struct loadanchors	*la;

	TAILQ_FOREACH(la, &loadanchorshead, entries) {
		if (pf->opts & PF_OPT_VERBOSE)
			fprintf(stderr, "\nLoading anchor %s from %s\n",
			    la->anchorname, la->filename);
		if (pfctl_rules(dev, la->filename, pf->opts, pf->optimize,
		    la->anchorname, pf->trans) == -1)
			return (-1);
	}

	return (0);
}

int
kw_casecmp(const void *k, const void *e)
{
	return (strcasecmp(k, ((const struct keywords *)e)->k_name));
}

int
map_tos(char *s, int *val)
{
	/* DiffServ Codepoints and other TOS mappings */
	const struct keywords	 toswords[] = {
		{ "af11",		IPTOS_DSCP_AF11 },
		{ "af12",		IPTOS_DSCP_AF12 },
		{ "af13",		IPTOS_DSCP_AF13 },
		{ "af21",		IPTOS_DSCP_AF21 },
		{ "af22",		IPTOS_DSCP_AF22 },
		{ "af23",		IPTOS_DSCP_AF23 },
		{ "af31",		IPTOS_DSCP_AF31 },
		{ "af32",		IPTOS_DSCP_AF32 },
		{ "af33",		IPTOS_DSCP_AF33 },
		{ "af41",		IPTOS_DSCP_AF41 },
		{ "af42",		IPTOS_DSCP_AF42 },
		{ "af43",		IPTOS_DSCP_AF43 },
		{ "critical",		IPTOS_PREC_CRITIC_ECP },
		{ "cs0",		IPTOS_DSCP_CS0 },
		{ "cs1",		IPTOS_DSCP_CS1 },
		{ "cs2",		IPTOS_DSCP_CS2 },
		{ "cs3",		IPTOS_DSCP_CS3 },
		{ "cs4",		IPTOS_DSCP_CS4 },
		{ "cs5",		IPTOS_DSCP_CS5 },
		{ "cs6",		IPTOS_DSCP_CS6 },
		{ "cs7",		IPTOS_DSCP_CS7 },
		{ "ef",			IPTOS_DSCP_EF },
		{ "inetcontrol",	IPTOS_PREC_INTERNETCONTROL },
		{ "lowdelay",		IPTOS_LOWDELAY },
		{ "netcontrol",		IPTOS_PREC_NETCONTROL },
		{ "reliability",	IPTOS_RELIABILITY },
		{ "throughput",		IPTOS_THROUGHPUT }
	};
	const struct keywords	*p;

	p = bsearch(s, toswords, sizeof(toswords)/sizeof(toswords[0]),
	    sizeof(toswords[0]), kw_casecmp);

	if (p) {
		*val = p->k_val;
		return (1);
	}
	return (0);
}

int
lookup_rtable(u_int rtableid)
{
	size_t			 len;
	struct rt_tableinfo	 info;
	int			 mib[6];
	static u_int		 found[RT_TABLEID_MAX+1];

	if (found[rtableid])
		return found[rtableid];

	mib[0] = CTL_NET;
	mib[1] = PF_ROUTE;
	mib[2] = 0;
	mib[3] = 0;
	mib[4] = NET_RT_TABLE;
	mib[5] = rtableid;

	len = sizeof(info);
	if (sysctl(mib, 6, &info, &len, NULL, 0) == -1) {
		if (errno == ENOENT) {
			/* table nonexistent */
			found[rtableid] = 0;
			return 0;
		}
		err(1, "%s", __func__);
	}
	found[rtableid] = 1;
	return 1;
}

int
filteropts_to_rule(struct pf_rule *r, struct filter_opts *opts)
{
	if (opts->marker & FOM_ONCE) {
		if ((r->action != PF_PASS && r->action != PF_DROP) || r->anchor) {
			yyerror("'once' only applies to pass/block rules");
			return (1);
		}
		r->rule_flag |= PFRULE_ONCE;
	}

	r->keep_state = opts->keep.action;
	r->pktrate.limit = opts->pktrate.limit;
	r->pktrate.seconds = opts->pktrate.seconds;
	r->prob = opts->prob;
	r->rtableid = opts->rtableid;
	r->tos = opts->tos;

	if (opts->nodf)
		r->scrub_flags |= PFSTATE_NODF;
	if (opts->randomid)
		r->scrub_flags |= PFSTATE_RANDOMID;
	if (opts->minttl)
		r->min_ttl = opts->minttl;
	if (opts->max_mss)
		r->max_mss = opts->max_mss;

	if (opts->tag)
		if (strlcpy(r->tagname, opts->tag,
		    PF_TAG_NAME_SIZE) >= PF_TAG_NAME_SIZE) {
			yyerror("tag too long, max %u chars",
			    PF_TAG_NAME_SIZE - 1);
			return (1);
		}
	if (opts->match_tag)
		if (strlcpy(r->match_tagname, opts->match_tag,
		    PF_TAG_NAME_SIZE) >= PF_TAG_NAME_SIZE) {
			yyerror("tag too long, max %u chars",
			    PF_TAG_NAME_SIZE - 1);
			return (1);
		}
	r->match_tag_not = opts->match_tag_not;

	if (rule_label(r, opts->label))
		return (1);
	free(opts->label);

	if (opts->marker & FOM_AFTO)
		r->rule_flag |= PFRULE_AFTO;
	if ((opts->marker & FOM_AFTO) && r->direction != PF_IN) {
		yyerror("af-to can only be used with direction in");
		return (1);
	}
	if ((opts->marker & FOM_AFTO) && opts->rt) {
		yyerror("af-to cannot be used together with "
		    "route-to, reply-to, dup-to");
		return (1);
	}
	if (opts->marker & FOM_SCRUB_TCP)
		r->scrub_flags |= PFSTATE_SCRUB_TCP;
	if (opts->marker & FOM_SETDELAY) {
		r->delay = opts->delay;
		r->rule_flag |= PFRULE_SETDELAY;
	}
	if (opts->marker & FOM_SETPRIO) {
		r->set_prio[0] = opts->set_prio[0];
		r->set_prio[1] = opts->set_prio[1];
		r->scrub_flags |= PFSTATE_SETPRIO;
	}
	if (opts->marker & FOM_SETTOS) {
		r->scrub_flags |= PFSTATE_SETTOS;
		r->set_tos = opts->settos;
	}
	if (opts->marker & FOM_PRIO)
		r->prio = opts->prio ? opts->prio : PF_PRIO_ZERO;
	if (opts->marker & FOM_SETPRIO) {
		r->set_prio[0] = opts->set_prio[0];
		r->set_prio[1] = opts->set_prio[1];
		r->scrub_flags |= PFSTATE_SETPRIO;
	}

	r->flags = opts->flags.b1;
	r->flagset = opts->flags.b2;
	if ((opts->flags.b1 & opts->flags.b2) != opts->flags.b1) {
		yyerror("flags always false");
		return (1);
	}

	if (opts->queues.qname != NULL) {
		if (strlcpy(r->qname, opts->queues.qname,
		    sizeof(r->qname)) >= sizeof(r->qname)) {
			yyerror("rule qname too long (max "
			    "%zu chars)", sizeof(r->qname)-1);
			return (1);
		}
		free(opts->queues.qname);
	}
	if (opts->queues.pqname != NULL) {
		if (strlcpy(r->pqname, opts->queues.pqname,
		    sizeof(r->pqname)) >= sizeof(r->pqname)) {
			yyerror("rule pqname too long (max "
			    "%zu chars)", sizeof(r->pqname)-1);
			return (1);
		}
		free(opts->queues.pqname);
	}

	if (opts->fragment)
		r->rule_flag |= PFRULE_FRAGMENT;
	r->allow_opts = opts->allowopts;

	return (0);
}
#line 4834 "parse.c"

#if YYDEBUG
#include <stdio.h>	/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == NULL)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == NULL)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != NULL)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    /* yym is set below */
    /* yyn is set below */
    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        yychar = YYLEX;
        if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if (((yyn = yysindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if (((yyn = yyrindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag != 0) goto yyinrecovery;

    YYERROR_CALL("syntax error");

    goto yyerrlab; /* redundant goto avoids 'unused label' warning */
yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if (((yyn = yysindex[*yystack.s_mark]) != 0) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym > 0)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);

    switch (yyn)
    {
case 13:
#line 551 "parse.y"
	{ file->errors++; }
#line 5036 "parse.c"
break;
case 14:
#line 554 "parse.y"
	{
			struct file	*nfile;

			if ((nfile = pushfile(yystack.l_mark[0].v.string, 0)) == NULL) {
				yyerror("failed to include file %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);

			file = nfile;
			lungetc('\n');
		}
#line 5053 "parse.c"
break;
case 19:
#line 579 "parse.y"
	{
			if (!strcmp(yystack.l_mark[0].v.string, "none"))
				yyval.v.i = 0;
			else if (!strcmp(yystack.l_mark[0].v.string, "basic"))
				yyval.v.i = PF_OPTIMIZE_BASIC;
			else if (!strcmp(yystack.l_mark[0].v.string, "profile"))
				yyval.v.i = PF_OPTIMIZE_BASIC | PF_OPTIMIZE_PROFILE;
			else {
				yyerror("unknown ruleset-optimization %s", yystack.l_mark[0].v.string);
				YYERROR;
			}
		}
#line 5069 "parse.c"
break;
case 20:
#line 593 "parse.y"
	{ yyval.v.number = 0; }
#line 5074 "parse.c"
break;
case 21:
#line 594 "parse.y"
	{ yyval.v.number = 1; }
#line 5079 "parse.c"
break;
case 22:
#line 597 "parse.y"
	{
			pfctl_set_reassembly(pf, yystack.l_mark[-1].v.number, yystack.l_mark[0].v.number);
		}
#line 5086 "parse.c"
break;
case 23:
#line 600 "parse.y"
	{
			if (pfctl_set_optimization(pf, yystack.l_mark[0].v.string) != 0) {
				yyerror("unknown optimization %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 5098 "parse.c"
break;
case 24:
#line 608 "parse.y"
	{
			if (!(pf->opts & PF_OPT_OPTIMIZE)) {
				pf->opts |= PF_OPT_OPTIMIZE;
				pf->optimize = yystack.l_mark[0].v.i;
			}
		}
#line 5108 "parse.c"
break;
case 29:
#line 618 "parse.y"
	{
			if (pfctl_set_logif(pf, yystack.l_mark[0].v.string) != 0) {
				yyerror("error setting loginterface %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 5120 "parse.c"
break;
case 30:
#line 626 "parse.y"
	{
			if (yystack.l_mark[0].v.number == 0 || yystack.l_mark[0].v.number > UINT_MAX) {
				yyerror("hostid must be non-zero");
				YYERROR;
			}
			pfctl_set_hostid(pf, yystack.l_mark[0].v.number);
		}
#line 5131 "parse.c"
break;
case 31:
#line 633 "parse.y"
	{
			if (pf->opts & PF_OPT_VERBOSE)
				printf("set block-policy drop\n");
			blockpolicy = PFRULE_DROP;
		}
#line 5140 "parse.c"
break;
case 32:
#line 638 "parse.y"
	{
			if (pf->opts & PF_OPT_VERBOSE)
				printf("set block-policy return\n");
			blockpolicy = PFRULE_RETURN;
		}
#line 5149 "parse.c"
break;
case 33:
#line 643 "parse.y"
	{
			if (pf->opts & PF_OPT_VERBOSE)
				printf("set fingerprints \"%s\"\n", yystack.l_mark[0].v.string);
			if (!pf->anchor->name[0]) {
				if (pfctl_file_fingerprints(pf->dev,
				    pf->opts, yystack.l_mark[0].v.string)) {
					yyerror("error loading "
					    "fingerprints %s", yystack.l_mark[0].v.string);
					free(yystack.l_mark[0].v.string);
					YYERROR;
				}
			}
			free(yystack.l_mark[0].v.string);
		}
#line 5167 "parse.c"
break;
case 34:
#line 657 "parse.y"
	{
			if (pf->opts & PF_OPT_VERBOSE)
				switch (yystack.l_mark[0].v.i) {
				case 0:
					printf("set state-policy floating\n");
					break;
				case PFRULE_IFBOUND:
					printf("set state-policy if-bound\n");
					break;
				}
			default_statelock = yystack.l_mark[0].v.i;
		}
#line 5183 "parse.c"
break;
case 35:
#line 669 "parse.y"
	{
			if (pfctl_set_debug(pf, yystack.l_mark[0].v.string) != 0) {
				yyerror("error setting debuglevel %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 5195 "parse.c"
break;
case 36:
#line 677 "parse.y"
	{
			if (pfctl_set_debug(pf, "debug") != 0) {
				yyerror("error setting debuglevel %s", "debug");
				YYERROR;
			}
		}
#line 5205 "parse.c"
break;
case 37:
#line 683 "parse.y"
	{
			if (expand_skip_interface(yystack.l_mark[0].v.interface) != 0) {
				yyerror("error setting skip interface(s)");
				YYERROR;
			}
		}
#line 5215 "parse.c"
break;
case 38:
#line 689 "parse.y"
	{
			if (keep_state_defaults != NULL) {
				yyerror("cannot redefine state-defaults");
				YYERROR;
			}
			keep_state_defaults = yystack.l_mark[0].v.state_opt;
		}
#line 5226 "parse.c"
break;
case 39:
#line 696 "parse.y"
	{
			if (pfctl_set_syncookies(pf, yystack.l_mark[-1].v.i, yystack.l_mark[0].v.watermarks)) {
				yyerror("error setting syncookies");
				YYERROR;
			}
		}
#line 5236 "parse.c"
break;
case 40:
#line 704 "parse.y"
	{
			if (!strcmp(yystack.l_mark[0].v.string, "never"))
				yyval.v.i = PF_SYNCOOKIES_NEVER;
			else if (!strcmp(yystack.l_mark[0].v.string, "adaptive"))
				yyval.v.i = PF_SYNCOOKIES_ADAPTIVE;
			else if (!strcmp(yystack.l_mark[0].v.string, "always"))
				yyval.v.i = PF_SYNCOOKIES_ALWAYS;
			else {
				yyerror("illegal value for syncookies");
				YYERROR;
			}
		}
#line 5252 "parse.c"
break;
case 41:
#line 718 "parse.y"
	{ yyval.v.watermarks = NULL; }
#line 5257 "parse.c"
break;
case 42:
#line 719 "parse.y"
	{
			memset(&syncookie_opts, 0, sizeof(syncookie_opts));
		  }
#line 5264 "parse.c"
break;
case 43:
#line 721 "parse.y"
	{ yyval.v.watermarks = &syncookie_opts; }
#line 5269 "parse.c"
break;
case 46:
#line 728 "parse.y"
	{
			double	 val;
			char	*cp;

			val = strtod(yystack.l_mark[0].v.string, &cp);
			if (cp == NULL || strcmp(cp, "%"))
				YYERROR;
			if (val <= 0 || val > 100) {
				yyerror("illegal percentage value");
				YYERROR;
			}
			if (!strcmp(yystack.l_mark[-1].v.string, "start")) {
				syncookie_opts.hi = val;
			} else if (!strcmp(yystack.l_mark[-1].v.string, "end")) {
				syncookie_opts.lo = val;
			} else {
				yyerror("illegal syncookie option");
				YYERROR;
			}
		}
#line 5293 "parse.c"
break;
case 47:
#line 750 "parse.y"
	{ yyval.v.string = yystack.l_mark[0].v.string; }
#line 5298 "parse.c"
break;
case 48:
#line 751 "parse.y"
	{
			if ((yyval.v.string = strdup("all")) == NULL) {
				err(1, "stringall: strdup");
			}
		}
#line 5307 "parse.c"
break;
case 49:
#line 758 "parse.y"
	{
			if (asprintf(&yyval.v.string, "%s %s", yystack.l_mark[-1].v.string, yystack.l_mark[0].v.string) == -1)
				err(1, "string: asprintf");
			free(yystack.l_mark[-1].v.string);
			free(yystack.l_mark[0].v.string);
		}
#line 5317 "parse.c"
break;
case 51:
#line 767 "parse.y"
	{
			if (asprintf(&yyval.v.string, "%s %s", yystack.l_mark[-1].v.string, yystack.l_mark[0].v.string) == -1)
				err(1, "string: asprintf");
			free(yystack.l_mark[-1].v.string);
			free(yystack.l_mark[0].v.string);
		}
#line 5327 "parse.c"
break;
case 53:
#line 776 "parse.y"
	{
			char	*s;
			if (asprintf(&s, "%lld", yystack.l_mark[0].v.number) == -1) {
				yyerror("string: asprintf");
				YYERROR;
			}
			yyval.v.string = s;
		}
#line 5339 "parse.c"
break;
case 55:
#line 787 "parse.y"
	{
			char *s = yystack.l_mark[-2].v.string;
			if (pf->opts & PF_OPT_VERBOSE)
				printf("%s = \"%s\"\n", yystack.l_mark[-2].v.string, yystack.l_mark[0].v.string);
			while (*s++) {
				if (isspace((unsigned char)*s)) {
					yyerror("macro name cannot contain "
					    "whitespace");
					free(yystack.l_mark[-2].v.string);
					free(yystack.l_mark[0].v.string);
					YYERROR;
				}
			}
			if (symset(yystack.l_mark[-2].v.string, yystack.l_mark[0].v.string, 0) == -1)
				err(1, "cannot store variable %s", yystack.l_mark[-2].v.string);
			free(yystack.l_mark[-2].v.string);
			free(yystack.l_mark[0].v.string);
		}
#line 5361 "parse.c"
break;
case 56:
#line 807 "parse.y"
	{
			if (yystack.l_mark[0].v.string[0] == '\0') {
				free(yystack.l_mark[0].v.string);
				yyerror("anchor name must not be empty");
				YYERROR;
			}
			if (strlen(pf->anchor->path) + 1 +
			    strlen(yystack.l_mark[0].v.string) >= PATH_MAX) {
				free(yystack.l_mark[0].v.string);
				yyerror("anchor name is longer than %u",
				    PATH_MAX - 1);
				YYERROR;
			}
			if (yystack.l_mark[0].v.string[0] == '_' || strstr(yystack.l_mark[0].v.string, "/_") != NULL) {
				free(yystack.l_mark[0].v.string);
				yyerror("anchor names beginning with '_' "
				    "are reserved for internal use");
				YYERROR;
			}
			yyval.v.string = yystack.l_mark[0].v.string;
		}
#line 5386 "parse.c"
break;
case 57:
#line 828 "parse.y"
	{ yyval.v.string = NULL; }
#line 5391 "parse.c"
break;
case 64:
#line 840 "parse.y"
	{
			char ta[PF_ANCHOR_NAME_SIZE];
			struct pf_ruleset *rs;

			/* steping into a brace anchor */
			pf->asd++;
			if (pf->asd >= PFCTL_ANCHOR_STACK_DEPTH)
				errx(1, "pfa_anchor: anchors too deep");
			pf->bn++;
			pf->brace = 1;

			/*
			 * Anchor contents are parsed before the anchor rule
			 * production completes, so we don't know the real
			 * location yet. Create a holding ruleset in the root;
			 * contents will be moved afterwards.
			 */
			snprintf(ta, PF_ANCHOR_NAME_SIZE, "_%d", pf->bn);
			rs = pf_find_or_create_ruleset(ta);
			if (rs == NULL)
				err(1, "pfa_anchor: pf_find_or_create_ruleset (%s)", ta);
			pf->astack[pf->asd] = rs->anchor;
			pf->anchor = rs->anchor;
		}
#line 5419 "parse.c"
break;
case 65:
#line 864 "parse.y"
	{
			pf->alast = pf->anchor;
			pf->asd--;
			pf->anchor = pf->astack[pf->asd];
		}
#line 5428 "parse.c"
break;
case 67:
#line 874 "parse.y"
	{
			struct pf_rule	r;
			struct node_proto	*proto;
			char	*p;

			memset(&r, 0, sizeof(r));
			if (pf->astack[pf->asd + 1]) {
				if (yystack.l_mark[-8].v.string && strchr(yystack.l_mark[-8].v.string, '/') != NULL) {
					free(yystack.l_mark[-8].v.string);
					yyerror("anchor paths containing '/' "
					    "cannot be used for inline anchors.");
					YYERROR;
				}

				/* Move inline rules into relative location. */
				pf_anchor_setup(&r,
				    &pf->astack[pf->asd]->ruleset,
				    yystack.l_mark[-8].v.string ? yystack.l_mark[-8].v.string : pf->alast->name);

				if (r.anchor == NULL)
					err(1, "anchorrule: unable to "
					    "create ruleset");

				if (pf->alast != r.anchor) {
					if (r.anchor->match) {
						yyerror("inline anchor '%s' "
						    "already exists",
						    r.anchor->name);
						YYERROR;
					}
					mv_rules(&pf->alast->ruleset,
					    &r.anchor->ruleset);
					mv_tables(pf, &pfr_ktables, r.anchor, pf->alast);
				}
				pf_remove_if_empty_ruleset(&pf->alast->ruleset);
				pf->alast = r.anchor;
			} else {
				if (!yystack.l_mark[-8].v.string) {
					yyerror("anchors without explicit "
					    "rules must specify a name");
					YYERROR;
				}

				/*
				 * Don't make non-brace anchors part of the main anchor pool.
				 */
				if ((r.anchor = calloc(1, sizeof(*r.anchor))) == NULL) {
					err(1, "anchorrule: calloc");
				}
				pf_init_ruleset(&r.anchor->ruleset);
				r.anchor->ruleset.anchor = r.anchor;
				if (strlcpy(r.anchor->path, yystack.l_mark[-8].v.string,
				    sizeof(r.anchor->path)) >= sizeof(r.anchor->path)) {
					errx(1, "anchorrule: strlcpy");
				}
				if ((p = strrchr(yystack.l_mark[-8].v.string, '/')) != NULL) {
					if (strlen(p) == 1) {
						yyerror("anchorrule: bad anchor name %s",
						    yystack.l_mark[-8].v.string);
						YYERROR;
					}
				} else
					p = yystack.l_mark[-8].v.string;
				if (strlcpy(r.anchor->name, p,
				    sizeof(r.anchor->name)) >= sizeof(r.anchor->name)) {
					errx(1, "anchorrule: strlcpy");
				}
			}

			r.direction = yystack.l_mark[-7].v.i;
			r.quick = yystack.l_mark[-6].v.logquick.quick;
			r.af = yystack.l_mark[-4].v.i;

			if (yystack.l_mark[-1].v.filter_opts.flags.b1 || yystack.l_mark[-1].v.filter_opts.flags.b2 || yystack.l_mark[-2].v.fromto.src_os) {
				for (proto = yystack.l_mark[-3].v.proto; proto != NULL &&
				    proto->proto != IPPROTO_TCP;
				    proto = proto->next)
					;	/* nothing */
				if (proto == NULL && yystack.l_mark[-3].v.proto != NULL) {
					if (yystack.l_mark[-1].v.filter_opts.flags.b1 || yystack.l_mark[-1].v.filter_opts.flags.b2)
						yyerror(
						    "flags only apply to tcp");
					if (yystack.l_mark[-2].v.fromto.src_os)
						yyerror(
						    "OS fingerprinting only "
						    "applies to tcp");
					YYERROR;
				}
			}

			if (filteropts_to_rule(&r, &yystack.l_mark[-1].v.filter_opts))
				YYERROR;

			if (yystack.l_mark[-1].v.filter_opts.keep.action) {
				yyerror("cannot specify state handling "
				    "on anchors");
				YYERROR;
			}

			if (yystack.l_mark[-1].v.filter_opts.rt) {
				yyerror("cannot specify route handling "
				    "on anchors");
				YYERROR;
			}

			decide_address_family(yystack.l_mark[-2].v.fromto.src.host, &r.af);
			decide_address_family(yystack.l_mark[-2].v.fromto.dst.host, &r.af);

			expand_rule(&r, 0, yystack.l_mark[-5].v.interface, NULL, NULL, NULL, yystack.l_mark[-3].v.proto, yystack.l_mark[-2].v.fromto.src_os,
			    yystack.l_mark[-2].v.fromto.src.host, yystack.l_mark[-2].v.fromto.src.port, yystack.l_mark[-2].v.fromto.dst.host, yystack.l_mark[-2].v.fromto.dst.port,
			    yystack.l_mark[-1].v.filter_opts.uid, yystack.l_mark[-1].v.filter_opts.gid, yystack.l_mark[-1].v.filter_opts.rcv, yystack.l_mark[-1].v.filter_opts.icmpspec);
			free(yystack.l_mark[-8].v.string);
			pf->astack[pf->asd + 1] = NULL;
		}
#line 5546 "parse.c"
break;
case 68:
#line 990 "parse.y"
	{
			struct loadanchors	*loadanchor;

			if (yystack.l_mark[-2].v.string == NULL) {
				yyerror("anchor name is missing");
				YYERROR;
			}
			loadanchor = calloc(1, sizeof(struct loadanchors));
			if (loadanchor == NULL)
				err(1, "loadrule: calloc");
			if ((loadanchor->anchorname = malloc(PATH_MAX)) ==
			    NULL)
				err(1, "loadrule: malloc");
			if (pf->anchor->name[0])
				snprintf(loadanchor->anchorname, PATH_MAX,
				    "%s/%s", pf->anchor->path, yystack.l_mark[-2].v.string);
			else
				strlcpy(loadanchor->anchorname, yystack.l_mark[-2].v.string, PATH_MAX);
			if ((loadanchor->filename = strdup(yystack.l_mark[0].v.string)) == NULL)
				err(1, "loadrule: strdup");

			TAILQ_INSERT_TAIL(&loadanchorshead, loadanchor,
			    entries);

			free(yystack.l_mark[-2].v.string);
			free(yystack.l_mark[0].v.string);
		}
#line 5577 "parse.c"
break;
case 69:
#line 1018 "parse.y"
	{
				bzero(&scrub_opts, sizeof scrub_opts);
			}
#line 5584 "parse.c"
break;
case 70:
#line 1022 "parse.y"
	{ yyval.v.scrub_opts = scrub_opts; }
#line 5589 "parse.c"
break;
case 73:
#line 1029 "parse.y"
	{
			if (scrub_opts.nodf) {
				yyerror("no-df cannot be respecified");
				YYERROR;
			}
			scrub_opts.nodf = 1;
		}
#line 5600 "parse.c"
break;
case 74:
#line 1036 "parse.y"
	{
			if (scrub_opts.marker & FOM_MINTTL) {
				yyerror("min-ttl cannot be respecified");
				YYERROR;
			}
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 255) {
				yyerror("illegal min-ttl value %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			scrub_opts.marker |= FOM_MINTTL;
			scrub_opts.minttl = yystack.l_mark[0].v.number;
		}
#line 5616 "parse.c"
break;
case 75:
#line 1048 "parse.y"
	{
			if (scrub_opts.marker & FOM_MAXMSS) {
				yyerror("max-mss cannot be respecified");
				YYERROR;
			}
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 65535) {
				yyerror("illegal max-mss value %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			scrub_opts.marker |= FOM_MAXMSS;
			scrub_opts.maxmss = yystack.l_mark[0].v.number;
		}
#line 5632 "parse.c"
break;
case 76:
#line 1060 "parse.y"
	{
			if (strcasecmp(yystack.l_mark[0].v.string, "tcp") != 0) {
				yyerror("scrub reassemble supports only tcp, "
				    "not '%s'", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
			if (scrub_opts.reassemble_tcp) {
				yyerror("reassemble tcp cannot be respecified");
				YYERROR;
			}
			scrub_opts.reassemble_tcp = 1;
		}
#line 5650 "parse.c"
break;
case 77:
#line 1074 "parse.y"
	{
			if (scrub_opts.randomid) {
				yyerror("random-id cannot be respecified");
				YYERROR;
			}
			scrub_opts.randomid = 1;
		}
#line 5661 "parse.c"
break;
case 78:
#line 1083 "parse.y"
	{
			struct pf_rule		 r;
			struct node_host	*h = NULL, *hh;
			struct node_if		*i, *j;

			for (i = yystack.l_mark[-2].v.interface; i; i = i->next) {
				bzero(&r, sizeof(r));

				r.action = PF_DROP;
				r.direction = PF_IN;
				r.log = yystack.l_mark[-3].v.logquick.log;
				r.logif = yystack.l_mark[-3].v.logquick.logif;
				r.quick = yystack.l_mark[-3].v.logquick.quick;
				r.af = yystack.l_mark[-1].v.i;
				if (rule_label(&r, yystack.l_mark[0].v.antispoof_opts.label))
					YYERROR;
				r.rtableid = yystack.l_mark[0].v.antispoof_opts.rtableid;
				j = calloc(1, sizeof(struct node_if));
				if (j == NULL)
					err(1, "antispoof: calloc");
				if (strlcpy(j->ifname, i->ifname,
				    sizeof(j->ifname)) >= sizeof(j->ifname)) {
					free(j);
					yyerror("interface name too long");
					YYERROR;
				}
				j->not = 1;
				if (i->dynamic) {
					h = calloc(1, sizeof(*h));
					if (h == NULL)
						err(1, "address: calloc");
					h->addr.type = PF_ADDR_DYNIFTL;
					set_ipmask(h, 128);
					if (strlcpy(h->addr.v.ifname, i->ifname,
					    sizeof(h->addr.v.ifname)) >=
					    sizeof(h->addr.v.ifname)) {
						free(h);
						yyerror(
						    "interface name too long");
						YYERROR;
					}
					hh = malloc(sizeof(*hh));
					if (hh == NULL)
						 err(1, "address: malloc");
					bcopy(h, hh, sizeof(*hh));
					h->addr.iflags = PFI_AFLAG_NETWORK;
				} else {
					h = ifa_lookup(j->ifname,
					    PFI_AFLAG_NETWORK);
					hh = NULL;
				}

				if (h != NULL)
					expand_rule(&r, 0, j, NULL, NULL, NULL,
					    NULL, NULL, h, NULL, NULL, NULL,
					    NULL, NULL, NULL, NULL);

				if ((i->ifa_flags & IFF_LOOPBACK) == 0) {
					bzero(&r, sizeof(r));

					r.action = PF_DROP;
					r.direction = PF_IN;
					r.log = yystack.l_mark[-3].v.logquick.log;
					r.logif = yystack.l_mark[-3].v.logquick.logif;
					r.quick = yystack.l_mark[-3].v.logquick.quick;
					r.af = yystack.l_mark[-1].v.i;
					if (rule_label(&r, yystack.l_mark[0].v.antispoof_opts.label))
						YYERROR;
					r.rtableid = yystack.l_mark[0].v.antispoof_opts.rtableid;
					if (hh != NULL)
						h = hh;
					else
						h = ifa_lookup(i->ifname, 0);
					if (h != NULL)
						expand_rule(&r, 0, NULL, NULL,
						    NULL, NULL, NULL, NULL, h,
						    NULL, NULL, NULL, NULL,
						    NULL, NULL, NULL);
				} else
					free(hh);
			}
			free(yystack.l_mark[0].v.antispoof_opts.label);
		}
#line 5748 "parse.c"
break;
case 79:
#line 1168 "parse.y"
	{ yyval.v.interface = yystack.l_mark[0].v.interface; }
#line 5753 "parse.c"
break;
case 80:
#line 1169 "parse.y"
	{ yyval.v.interface = yystack.l_mark[-1].v.interface; }
#line 5758 "parse.c"
break;
case 81:
#line 1172 "parse.y"
	{ yyval.v.interface = yystack.l_mark[-1].v.interface; }
#line 5763 "parse.c"
break;
case 82:
#line 1173 "parse.y"
	{
			yystack.l_mark[-3].v.interface->tail->next = yystack.l_mark[-1].v.interface;
			yystack.l_mark[-3].v.interface->tail = yystack.l_mark[-1].v.interface;
			yyval.v.interface = yystack.l_mark[-3].v.interface;
		}
#line 5772 "parse.c"
break;
case 83:
#line 1180 "parse.y"
	{ yyval.v.interface = yystack.l_mark[0].v.interface; }
#line 5777 "parse.c"
break;
case 84:
#line 1181 "parse.y"
	{
			yystack.l_mark[-1].v.interface->dynamic = 1;
			yyval.v.interface = yystack.l_mark[-1].v.interface;
		}
#line 5785 "parse.c"
break;
case 85:
#line 1187 "parse.y"
	{
				bzero(&antispoof_opts, sizeof antispoof_opts);
				antispoof_opts.rtableid = -1;
			}
#line 5793 "parse.c"
break;
case 86:
#line 1192 "parse.y"
	{ yyval.v.antispoof_opts = antispoof_opts; }
#line 5798 "parse.c"
break;
case 87:
#line 1193 "parse.y"
	{
			bzero(&antispoof_opts, sizeof antispoof_opts);
			antispoof_opts.rtableid = -1;
			yyval.v.antispoof_opts = antispoof_opts;
		}
#line 5807 "parse.c"
break;
case 90:
#line 1204 "parse.y"
	{
			if (antispoof_opts.label) {
				yyerror("label cannot be redefined");
				YYERROR;
			}
			antispoof_opts.label = yystack.l_mark[0].v.string;
		}
#line 5818 "parse.c"
break;
case 91:
#line 1211 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > RT_TABLEID_MAX) {
				yyerror("invalid rtable id");
				YYERROR;
			} else if (!lookup_rtable(yystack.l_mark[0].v.number)) {
				yyerror("rtable %lld does not exist", yystack.l_mark[0].v.number);
				YYERROR;
			}
			antispoof_opts.rtableid = yystack.l_mark[0].v.number;
		}
#line 5832 "parse.c"
break;
case 92:
#line 1223 "parse.y"
	{ yyval.v.number = 1; }
#line 5837 "parse.c"
break;
case 93:
#line 1224 "parse.y"
	{ yyval.v.number = 0; }
#line 5842 "parse.c"
break;
case 94:
#line 1227 "parse.y"
	{
			struct node_host	 *h, *nh;
			struct node_tinit	 *ti, *nti;

			if (strlen(yystack.l_mark[-2].v.string) >= PF_TABLE_NAME_SIZE) {
				yyerror("table name too long, max %d chars",
				    PF_TABLE_NAME_SIZE - 1);
				free(yystack.l_mark[-2].v.string);
				YYERROR;
			}
			if (process_tabledef(yystack.l_mark[-2].v.string, &yystack.l_mark[0].v.table_opts, pf->opts)) {
				free(yystack.l_mark[-2].v.string);
				YYERROR;
			}
			free(yystack.l_mark[-2].v.string);
			for (ti = SIMPLEQ_FIRST(&yystack.l_mark[0].v.table_opts.init_nodes); ti != NULL;
			    ti = nti) {
				if (ti->file)
					free(ti->file);
				for (h = ti->host; h != NULL; h = nh) {
					nh = h->next;
					free(h);
				}
				nti = SIMPLEQ_NEXT(ti, entries);
				free(ti);
			}
		}
#line 5873 "parse.c"
break;
case 95:
#line 1256 "parse.y"
	{
			bzero(&table_opts, sizeof table_opts);
			SIMPLEQ_INIT(&table_opts.init_nodes);
		}
#line 5881 "parse.c"
break;
case 96:
#line 1261 "parse.y"
	{ yyval.v.table_opts = table_opts; }
#line 5886 "parse.c"
break;
case 97:
#line 1263 "parse.y"
	{
			bzero(&table_opts, sizeof table_opts);
			SIMPLEQ_INIT(&table_opts.init_nodes);
			yyval.v.table_opts = table_opts;
		}
#line 5895 "parse.c"
break;
case 100:
#line 1274 "parse.y"
	{
			if (!strcmp(yystack.l_mark[0].v.string, "const"))
				table_opts.flags |= PFR_TFLAG_CONST;
			else if (!strcmp(yystack.l_mark[0].v.string, "persist"))
				table_opts.flags |= PFR_TFLAG_PERSIST;
			else if (!strcmp(yystack.l_mark[0].v.string, "counters"))
				table_opts.flags |= PFR_TFLAG_COUNTERS;
			else {
				yyerror("invalid table option '%s'", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 5913 "parse.c"
break;
case 101:
#line 1288 "parse.y"
	{ table_opts.init_addr = 1; }
#line 5918 "parse.c"
break;
case 102:
#line 1289 "parse.y"
	{
			struct node_host	*n;
			struct node_tinit	*ti;

			for (n = yystack.l_mark[-1].v.host; n != NULL; n = n->next) {
				switch (n->addr.type) {
				case PF_ADDR_ADDRMASK:
					continue; /* ok */
				case PF_ADDR_RANGE:
					yyerror("address ranges are not "
					    "permitted inside tables");
					break;
				case PF_ADDR_DYNIFTL:
					yyerror("dynamic addresses are not "
					    "permitted inside tables");
					break;
				case PF_ADDR_TABLE:
					yyerror("tables cannot contain tables");
					break;
				case PF_ADDR_NOROUTE:
					yyerror("\"no-route\" is not permitted "
					    "inside tables");
					break;
				case PF_ADDR_URPFFAILED:
					yyerror("\"urpf-failed\" is not "
					    "permitted inside tables");
					break;
				default:
					yyerror("unknown address type %d",
					    n->addr.type);
				}
				YYERROR;
			}
			if (!(ti = calloc(1, sizeof(*ti))))
				err(1, "table_opt: calloc");
			ti->host = yystack.l_mark[-1].v.host;
			SIMPLEQ_INSERT_TAIL(&table_opts.init_nodes, ti,
			    entries);
			table_opts.init_addr = 1;
		}
#line 5962 "parse.c"
break;
case 103:
#line 1329 "parse.y"
	{
			struct node_tinit	*ti;

			if (!(ti = calloc(1, sizeof(*ti))))
				err(1, "table_opt: calloc");
			ti->file = yystack.l_mark[0].v.string;
			SIMPLEQ_INSERT_TAIL(&table_opts.init_nodes, ti,
			    entries);
			table_opts.init_addr = 1;
		}
#line 5976 "parse.c"
break;
case 104:
#line 1341 "parse.y"
	{
			if (yystack.l_mark[0].v.weight > 0) {
				struct node_host	*n;
				for (n = yystack.l_mark[-1].v.host; n != NULL; n = n->next)
					n->weight = yystack.l_mark[0].v.weight;
			}
			yyval.v.host = yystack.l_mark[-1].v.host;
		}
#line 5988 "parse.c"
break;
case 105:
#line 1349 "parse.y"
	{ yyval.v.host = yystack.l_mark[-1].v.host; }
#line 5993 "parse.c"
break;
case 106:
#line 1352 "parse.y"
	{ yyval.v.host = yystack.l_mark[-1].v.host; }
#line 5998 "parse.c"
break;
case 107:
#line 1353 "parse.y"
	{
			yystack.l_mark[-3].v.host->tail->next = yystack.l_mark[-1].v.host;
			yystack.l_mark[-3].v.host->tail = yystack.l_mark[-1].v.host->tail;
			yyval.v.host = yystack.l_mark[-3].v.host;
		}
#line 6007 "parse.c"
break;
case 108:
#line 1360 "parse.y"
	{
			struct node_host	*n;

			if (yystack.l_mark[-1].v.interface == NULL && yystack.l_mark[0].v.queue_opts.parent == NULL) {
				yyerror("root queue without interface");
				YYERROR;
			}
			if (yystack.l_mark[-1].v.interface != NULL &&
			    ((n = ifa_exists(yystack.l_mark[-1].v.interface->ifname)) == NULL ||
			     n->af != AF_LINK)) {
				yyerror("not an interface");
				YYERROR;
			}

			expand_queue(yystack.l_mark[-2].v.string, yystack.l_mark[-1].v.interface, &yystack.l_mark[0].v.queue_opts);
		}
#line 6027 "parse.c"
break;
case 109:
#line 1378 "parse.y"
	{
			bzero(&queue_opts, sizeof queue_opts);
		}
#line 6034 "parse.c"
break;
case 110:
#line 1382 "parse.y"
	{ yyval.v.queue_opts = queue_opts; }
#line 6039 "parse.c"
break;
case 113:
#line 1389 "parse.y"
	{
			if (queue_opts.marker & QOM_BWSPEC) {
				yyerror("bandwidth cannot be respecified");
				YYERROR;
			}
			queue_opts.marker |= QOM_BWSPEC;
			queue_opts.linkshare = yystack.l_mark[-1].v.sc;
			queue_opts.realtime= yystack.l_mark[0].v.queue_opts.realtime;
			queue_opts.upperlimit = yystack.l_mark[0].v.queue_opts.upperlimit;
		}
#line 6053 "parse.c"
break;
case 114:
#line 1399 "parse.y"
	{
			if (queue_opts.marker & QOM_PARENT) {
				yyerror("parent cannot be respecified");
				YYERROR;
			}
			queue_opts.marker |= QOM_PARENT;
			queue_opts.parent = yystack.l_mark[0].v.string;
		}
#line 6065 "parse.c"
break;
case 115:
#line 1407 "parse.y"
	{
			if (queue_opts.marker & QOM_DEFAULT) {
				yyerror("default cannot be respecified");
				YYERROR;
			}
			queue_opts.marker |= QOM_DEFAULT;
			queue_opts.flags |= PFQS_DEFAULT;
		}
#line 6077 "parse.c"
break;
case 116:
#line 1415 "parse.y"
	{
			if (queue_opts.marker & QOM_QLIMIT) {
				yyerror("qlimit cannot be respecified");
				YYERROR;
			}
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 65535) {
				yyerror("qlimit out of range: max 65535");
				YYERROR;
			}
			queue_opts.marker |= QOM_QLIMIT;
			queue_opts.qlimit = yystack.l_mark[0].v.number;
		}
#line 6093 "parse.c"
break;
case 117:
#line 1427 "parse.y"
	{
			if (queue_opts.marker & QOM_FLOWS) {
				yyerror("number of flows cannot be respecified");
				YYERROR;
			}
			if (yystack.l_mark[0].v.number < 1 || yystack.l_mark[0].v.number > 32767) {
				yyerror("number of flows out of range: "
				    "max 32767");
				YYERROR;
			}
			queue_opts.marker |= QOM_FLOWS;
			queue_opts.flags |= PFQS_FLOWQUEUE;
			queue_opts.flowqueue.flows = yystack.l_mark[0].v.number;
		}
#line 6111 "parse.c"
break;
case 118:
#line 1441 "parse.y"
	{
			if (queue_opts.marker & QOM_QUANTUM) {
				yyerror("quantum cannot be respecified");
				YYERROR;
			}
			if (yystack.l_mark[0].v.number < 1 || yystack.l_mark[0].v.number > 65535) {
				yyerror("quantum out of range: max 65535");
				YYERROR;
			}
			queue_opts.marker |= QOM_QUANTUM;
			queue_opts.flowqueue.quantum = yystack.l_mark[0].v.number;
		}
#line 6127 "parse.c"
break;
case 119:
#line 1455 "parse.y"
	{

		}
#line 6134 "parse.c"
break;
case 120:
#line 1458 "parse.y"
	{
			yyval.v.queue_opts.realtime = yystack.l_mark[0].v.sc;
		}
#line 6141 "parse.c"
break;
case 121:
#line 1461 "parse.y"
	{
			yyval.v.queue_opts.upperlimit = yystack.l_mark[0].v.sc;
		}
#line 6148 "parse.c"
break;
case 122:
#line 1464 "parse.y"
	{
			yyval.v.queue_opts.realtime = yystack.l_mark[-3].v.sc;
			yyval.v.queue_opts.upperlimit = yystack.l_mark[0].v.sc;
		}
#line 6156 "parse.c"
break;
case 123:
#line 1468 "parse.y"
	{
			yyval.v.queue_opts.realtime = yystack.l_mark[0].v.sc;
			yyval.v.queue_opts.upperlimit = yystack.l_mark[-3].v.sc;
		}
#line 6164 "parse.c"
break;
case 124:
#line 1474 "parse.y"
	{
			yyval.v.sc.m2 = yystack.l_mark[0].v.queue_bwspec;
			yyval.v.sc.d = 0;
			if (yyval.v.sc.m2.bw_percent) {
				yyerror("no bandwidth in %% yet");
				YYERROR;
			}
		}
#line 6176 "parse.c"
break;
case 125:
#line 1482 "parse.y"
	{
			u_long	 ul;
			char	*cp;

			ul = strtoul(yystack.l_mark[0].v.string, &cp, 10);
			if (cp == NULL || strcmp(cp, "ms")) {
				yyerror("time in scspec must be in ms");
				YYERROR;
			}

			yyval.v.sc.m1 = yystack.l_mark[-2].v.queue_bwspec;
			yyval.v.sc.d = ul;
			yyval.v.sc.m2 = yystack.l_mark[-4].v.queue_bwspec;

			if (yyval.v.sc.m1.bw_percent || yyval.v.sc.m2.bw_percent) {
				yyerror("no bandwidth in %% yet");
				YYERROR;
			}
		}
#line 6199 "parse.c"
break;
case 126:
#line 1503 "parse.y"
	{
			double	 bps;
			char	*cp;

			yyval.v.queue_bwspec.bw_percent = 0;

			bps = strtod(yystack.l_mark[0].v.string, &cp);
			if (cp != NULL) {
				if (strlen(cp) > 1) {
					char *cu = cp + 1;
					if (!strcmp(cu, "Bit") ||
					    !strcmp(cu, "B") ||
					    !strcmp(cu, "bit") ||
					    !strcmp(cu, "b")) {
						*cu = 0;
					}
				}
				if (!strcmp(cp, "b"))
					; /* nothing */
				else if (!strcmp(cp, "K"))
					bps *= 1000;
				else if (!strcmp(cp, "M"))
					bps *= 1000 * 1000;
				else if (!strcmp(cp, "G"))
					bps *= 1000 * 1000 * 1000;
				else if (!strcmp(cp, "%")) {
					if (bps < 0 || bps > 100) {
						yyerror("bandwidth spec "
						    "out of range");
						free(yystack.l_mark[0].v.string);
						YYERROR;
					}
					yyval.v.queue_bwspec.bw_percent = bps;
					bps = 0;
				} else {
					yyerror("unknown unit \"%s\"", cp);
					free(yystack.l_mark[0].v.string);
					YYERROR;
				}
			}
			free(yystack.l_mark[0].v.string);
			if (bps < 0 || bps > (double)LLONG_MAX) {
				yyerror("bandwidth number too big");
				YYERROR;
			}
			yyval.v.queue_bwspec.bw_absolute = (u_int64_t)bps;
		}
#line 6250 "parse.c"
break;
case 127:
#line 1550 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > LLONG_MAX) {
				yyerror("bandwidth number too big");
				YYERROR;
			}
			yyval.v.queue_bwspec.bw_percent = 0;
			yyval.v.queue_bwspec.bw_absolute = yystack.l_mark[0].v.number;
		}
#line 6262 "parse.c"
break;
case 128:
#line 1562 "parse.y"
	{
			struct pf_rule		 r;
			struct node_state_opt	*o;
			struct node_proto	*proto;
			int			 srctrack = 0;
			int			 statelock = 0;
			int			 adaptive = 0;
			int			 defaults = 0;

			memset(&r, 0, sizeof(r));
			r.action = yystack.l_mark[-7].v.b.b1;
			switch (yystack.l_mark[-7].v.b.b2) {
			case PFRULE_RETURNRST:
				r.rule_flag |= PFRULE_RETURNRST;
				r.return_ttl = yystack.l_mark[-7].v.b.w;
				break;
			case PFRULE_RETURNICMP:
				r.rule_flag |= PFRULE_RETURNICMP;
				r.return_icmp = yystack.l_mark[-7].v.b.w;
				r.return_icmp6 = yystack.l_mark[-7].v.b.w2;
				break;
			case PFRULE_RETURN:
				r.rule_flag |= PFRULE_RETURN;
				r.return_icmp = yystack.l_mark[-7].v.b.w;
				r.return_icmp6 = yystack.l_mark[-7].v.b.w2;
				break;
			}
			r.direction = yystack.l_mark[-6].v.i;
			r.log = yystack.l_mark[-5].v.logquick.log;
			r.logif = yystack.l_mark[-5].v.logquick.logif;
			r.quick = yystack.l_mark[-5].v.logquick.quick;
			r.af = yystack.l_mark[-3].v.i;

			if (filteropts_to_rule(&r, &yystack.l_mark[0].v.filter_opts))
				YYERROR;

			if (yystack.l_mark[0].v.filter_opts.flags.b1 || yystack.l_mark[0].v.filter_opts.flags.b2 || yystack.l_mark[-1].v.fromto.src_os) {
				for (proto = yystack.l_mark[-2].v.proto; proto != NULL &&
				    proto->proto != IPPROTO_TCP;
				    proto = proto->next)
					;	/* nothing */
				if (proto == NULL && yystack.l_mark[-2].v.proto != NULL) {
					if (yystack.l_mark[0].v.filter_opts.flags.b1 || yystack.l_mark[0].v.filter_opts.flags.b2)
						yyerror(
						    "flags only apply to tcp");
					if (yystack.l_mark[-1].v.fromto.src_os)
						yyerror(
						    "OS fingerprinting only "
						    "apply to tcp");
					YYERROR;
				}
			}

			r.keep_state = yystack.l_mark[0].v.filter_opts.keep.action;
			o = yystack.l_mark[0].v.filter_opts.keep.options;

			/* 'keep state' by default on pass rules. */
			if (!r.keep_state && !r.action &&
			    !(yystack.l_mark[0].v.filter_opts.marker & FOM_KEEP)) {
				r.keep_state = PF_STATE_NORMAL;
				o = keep_state_defaults;
				defaults = 1;
			}

			while (o) {
				struct node_state_opt	*p = o;

				switch (o->type) {
				case PF_STATE_OPT_MAX:
					if (r.max_states) {
						yyerror("state option 'max' "
						    "multiple definitions");
						YYERROR;
					}
					r.max_states = o->data.max_states;
					break;
				case PF_STATE_OPT_NOSYNC:
					if (r.rule_flag & PFRULE_NOSYNC) {
						yyerror("state option 'sync' "
						    "multiple definitions");
						YYERROR;
					}
					r.rule_flag |= PFRULE_NOSYNC;
					break;
				case PF_STATE_OPT_SRCTRACK:
					if (srctrack) {
						yyerror("state option "
						    "'source-track' "
						    "multiple definitions");
						YYERROR;
					}
					srctrack =  o->data.src_track;
					r.rule_flag |= PFRULE_SRCTRACK;
					break;
				case PF_STATE_OPT_MAX_SRC_STATES:
					if (r.max_src_states) {
						yyerror("state option "
						    "'max-src-states' "
						    "multiple definitions");
						YYERROR;
					}
					if (o->data.max_src_states == 0) {
						yyerror("'max-src-states' must "
						    "be > 0");
						YYERROR;
					}
					r.max_src_states =
					    o->data.max_src_states;
					r.rule_flag |= PFRULE_SRCTRACK;
					break;
				case PF_STATE_OPT_OVERLOAD:
					if (r.overload_tblname[0]) {
						yyerror("multiple 'overload' "
						    "table definitions");
						YYERROR;
					}
					if (strlcpy(r.overload_tblname,
					    o->data.overload.tblname,
					    PF_TABLE_NAME_SIZE) >=
					    PF_TABLE_NAME_SIZE) {
						yyerror("state option: "
						    "strlcpy");
						YYERROR;
					}
					r.flush = o->data.overload.flush;
					break;
				case PF_STATE_OPT_MAX_SRC_CONN:
					if (r.max_src_conn) {
						yyerror("state option "
						    "'max-src-conn' "
						    "multiple definitions");
						YYERROR;
					}
					if (o->data.max_src_conn == 0) {
						yyerror("'max-src-conn' "
						    "must be > 0");
						YYERROR;
					}
					r.max_src_conn =
					    o->data.max_src_conn;
					r.rule_flag |= PFRULE_SRCTRACK |
					    PFRULE_RULESRCTRACK;
					break;
				case PF_STATE_OPT_MAX_SRC_CONN_RATE:
					if (r.max_src_conn_rate.limit) {
						yyerror("state option "
						    "'max-src-conn-rate' "
						    "multiple definitions");
						YYERROR;
					}
					if (!o->data.max_src_conn_rate.limit ||
					    !o->data.max_src_conn_rate.seconds) {
						yyerror("'max-src-conn-rate' "
						    "values must be > 0");
						YYERROR;
					}
					if (o->data.max_src_conn_rate.limit >
					    PF_THRESHOLD_MAX) {
						yyerror("'max-src-conn-rate' "
						    "maximum rate must be < %u",
						    PF_THRESHOLD_MAX);
						YYERROR;
					}
					r.max_src_conn_rate.limit =
					    o->data.max_src_conn_rate.limit;
					r.max_src_conn_rate.seconds =
					    o->data.max_src_conn_rate.seconds;
					r.rule_flag |= PFRULE_SRCTRACK |
					    PFRULE_RULESRCTRACK;
					break;
				case PF_STATE_OPT_MAX_SRC_NODES:
					if (r.max_src_nodes) {
						yyerror("state option "
						    "'max-src-nodes' "
						    "multiple definitions");
						YYERROR;
					}
					if (o->data.max_src_nodes == 0) {
						yyerror("'max-src-nodes' must "
						    "be > 0");
						YYERROR;
					}
					r.max_src_nodes =
					    o->data.max_src_nodes;
					r.rule_flag |= PFRULE_SRCTRACK |
					    PFRULE_RULESRCTRACK;
					break;
				case PF_STATE_OPT_STATELOCK:
					if (statelock) {
						yyerror("state locking option: "
						    "multiple definitions");
						YYERROR;
					}
					statelock = 1;
					r.rule_flag |= o->data.statelock;
					break;
				case PF_STATE_OPT_SLOPPY:
					if (r.rule_flag & PFRULE_STATESLOPPY) {
						yyerror("state sloppy option: "
						    "multiple definitions");
						YYERROR;
					}
					r.rule_flag |= PFRULE_STATESLOPPY;
					break;
				case PF_STATE_OPT_PFLOW:
					if (r.rule_flag & PFRULE_PFLOW) {
						yyerror("state pflow "
						    "option: multiple "
						    "definitions");
						YYERROR;
					}
					r.rule_flag |= PFRULE_PFLOW;
					break;
				case PF_STATE_OPT_TIMEOUT:
					if (o->data.timeout.number ==
					    PFTM_ADAPTIVE_START ||
					    o->data.timeout.number ==
					    PFTM_ADAPTIVE_END)
						adaptive = 1;
					if (r.timeout[o->data.timeout.number]) {
						yyerror("state timeout %s "
						    "multiple definitions",
						    pf_timeouts[o->data.
						    timeout.number].name);
						YYERROR;
					}
					r.timeout[o->data.timeout.number] =
					    o->data.timeout.seconds;
				}
				o = o->next;
				if (!defaults)
					free(p);
			}

			/* 'flags S/SA' by default on stateful rules */
			if (!r.action && !r.flags && !r.flagset &&
			    !yystack.l_mark[0].v.filter_opts.fragment && !(yystack.l_mark[0].v.filter_opts.marker & FOM_FLAGS) &&
			    r.keep_state) {
				r.flags = parse_flags("S");
				r.flagset =  parse_flags("SA");
			}
			if (!adaptive && r.max_states) {
				r.timeout[PFTM_ADAPTIVE_START] =
				    (r.max_states / 10) * 6;
				r.timeout[PFTM_ADAPTIVE_END] =
				    (r.max_states / 10) * 12;
			}
			if (r.rule_flag & PFRULE_SRCTRACK) {
				if (srctrack == PF_SRCTRACK_GLOBAL &&
				    r.max_src_nodes) {
					yyerror("'max-src-nodes' is "
					    "incompatible with "
					    "'source-track global'");
					YYERROR;
				}
				if (srctrack == PF_SRCTRACK_GLOBAL &&
				    r.max_src_conn) {
					yyerror("'max-src-conn' is "
					    "incompatible with "
					    "'source-track global'");
					YYERROR;
				}
				if (srctrack == PF_SRCTRACK_GLOBAL &&
				    r.max_src_conn_rate.seconds) {
					yyerror("'max-src-conn-rate' is "
					    "incompatible with "
					    "'source-track global'");
					YYERROR;
				}
				if (r.timeout[PFTM_SRC_NODE] <
				    r.max_src_conn_rate.seconds)
					r.timeout[PFTM_SRC_NODE] =
					    r.max_src_conn_rate.seconds;
				r.rule_flag |= PFRULE_SRCTRACK;
				if (srctrack == PF_SRCTRACK_RULE)
					r.rule_flag |= PFRULE_RULESRCTRACK;
			}
			if (r.keep_state && !statelock)
				r.rule_flag |= default_statelock;

			decide_address_family(yystack.l_mark[-1].v.fromto.src.host, &r.af);
			decide_address_family(yystack.l_mark[-1].v.fromto.dst.host, &r.af);

			if (yystack.l_mark[0].v.filter_opts.rt) {
				if (yystack.l_mark[0].v.filter_opts.rt != PF_DUPTO && !r.direction) {
					yyerror("direction must be explicit "
					    "with rules that specify routing");
					YYERROR;
				}
				r.rt = yystack.l_mark[0].v.filter_opts.rt;
			}

			if (expand_divertspec(&r, &yystack.l_mark[0].v.filter_opts.divert))
				YYERROR;

			expand_rule(&r, 0, yystack.l_mark[-4].v.interface, &yystack.l_mark[0].v.filter_opts.nat, &yystack.l_mark[0].v.filter_opts.rdr, &yystack.l_mark[0].v.filter_opts.rroute, yystack.l_mark[-2].v.proto,
			    yystack.l_mark[-1].v.fromto.src_os,
			    yystack.l_mark[-1].v.fromto.src.host, yystack.l_mark[-1].v.fromto.src.port, yystack.l_mark[-1].v.fromto.dst.host, yystack.l_mark[-1].v.fromto.dst.port,
			    yystack.l_mark[0].v.filter_opts.uid, yystack.l_mark[0].v.filter_opts.gid, yystack.l_mark[0].v.filter_opts.rcv, yystack.l_mark[0].v.filter_opts.icmpspec);
		}
#line 6566 "parse.c"
break;
case 129:
#line 1864 "parse.y"
	{
				bzero(&filter_opts, sizeof filter_opts);
				filter_opts.rtableid = -1;
			}
#line 6574 "parse.c"
break;
case 130:
#line 1869 "parse.y"
	{ yyval.v.filter_opts = filter_opts; }
#line 6579 "parse.c"
break;
case 131:
#line 1870 "parse.y"
	{
			bzero(&filter_opts, sizeof filter_opts);
			filter_opts.rtableid = -1;
			yyval.v.filter_opts = filter_opts;
		}
#line 6588 "parse.c"
break;
case 134:
#line 1881 "parse.y"
	{
			if (filter_opts.uid)
				yystack.l_mark[0].v.uid->tail->next = filter_opts.uid;
			filter_opts.uid = yystack.l_mark[0].v.uid;
		}
#line 6597 "parse.c"
break;
case 135:
#line 1886 "parse.y"
	{
			if (filter_opts.gid)
				yystack.l_mark[0].v.gid->tail->next = filter_opts.gid;
			filter_opts.gid = yystack.l_mark[0].v.gid;
		}
#line 6606 "parse.c"
break;
case 136:
#line 1891 "parse.y"
	{
			if (filter_opts.marker & FOM_FLAGS) {
				yyerror("flags cannot be redefined");
				YYERROR;
			}
			filter_opts.marker |= FOM_FLAGS;
			filter_opts.flags.b1 |= yystack.l_mark[0].v.b.b1;
			filter_opts.flags.b2 |= yystack.l_mark[0].v.b.b2;
			filter_opts.flags.w |= yystack.l_mark[0].v.b.w;
			filter_opts.flags.w2 |= yystack.l_mark[0].v.b.w2;
		}
#line 6621 "parse.c"
break;
case 137:
#line 1902 "parse.y"
	{
			if (filter_opts.marker & FOM_ICMP) {
				yyerror("icmp-type cannot be redefined");
				YYERROR;
			}
			filter_opts.marker |= FOM_ICMP;
			filter_opts.icmpspec = yystack.l_mark[0].v.icmp;
		}
#line 6633 "parse.c"
break;
case 138:
#line 1910 "parse.y"
	{
			if (filter_opts.marker & FOM_PRIO) {
				yyerror("prio cannot be redefined");
				YYERROR;
			}
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > IFQ_MAXPRIO) {
				yyerror("prio must be 0 - %u", IFQ_MAXPRIO);
				YYERROR;
			}
			filter_opts.marker |= FOM_PRIO;
			filter_opts.prio = yystack.l_mark[0].v.number;
		}
#line 6649 "parse.c"
break;
case 139:
#line 1922 "parse.y"
	{
			if (filter_opts.marker & FOM_TOS) {
				yyerror("tos cannot be redefined");
				YYERROR;
			}
			filter_opts.marker |= FOM_TOS;
			filter_opts.tos = yystack.l_mark[0].v.number;
		}
#line 6661 "parse.c"
break;
case 140:
#line 1930 "parse.y"
	{
			if (filter_opts.marker & FOM_KEEP) {
				yyerror("modulate or keep cannot be redefined");
				YYERROR;
			}
			filter_opts.marker |= FOM_KEEP;
			filter_opts.keep.action = yystack.l_mark[0].v.keep_state.action;
			filter_opts.keep.options = yystack.l_mark[0].v.keep_state.options;
		}
#line 6674 "parse.c"
break;
case 141:
#line 1939 "parse.y"
	{
			filter_opts.fragment = 1;
		}
#line 6681 "parse.c"
break;
case 142:
#line 1942 "parse.y"
	{
			filter_opts.allowopts = 1;
		}
#line 6688 "parse.c"
break;
case 143:
#line 1945 "parse.y"
	{
			if (filter_opts.label) {
				yyerror("label cannot be redefined");
				YYERROR;
			}
			filter_opts.label = yystack.l_mark[0].v.string;
		}
#line 6699 "parse.c"
break;
case 144:
#line 1952 "parse.y"
	{
			if (filter_opts.queues.qname) {
				yyerror("queue cannot be redefined");
				YYERROR;
			}
			filter_opts.queues = yystack.l_mark[0].v.qassign;
		}
#line 6710 "parse.c"
break;
case 145:
#line 1959 "parse.y"
	{
			filter_opts.tag = yystack.l_mark[0].v.string;
		}
#line 6717 "parse.c"
break;
case 146:
#line 1962 "parse.y"
	{
			filter_opts.match_tag = yystack.l_mark[0].v.string;
			filter_opts.match_tag_not = yystack.l_mark[-2].v.number;
		}
#line 6725 "parse.c"
break;
case 147:
#line 1966 "parse.y"
	{
			double	p;

			p = floor(yystack.l_mark[0].v.probability * UINT_MAX + 0.5);
			if (p < 0.0 || p > UINT_MAX) {
				yyerror("invalid probability: %g%%", yystack.l_mark[0].v.probability * 100);
				YYERROR;
			}
			filter_opts.prob = (u_int32_t)p;
			if (filter_opts.prob == 0)
				filter_opts.prob = 1;
		}
#line 6741 "parse.c"
break;
case 148:
#line 1978 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > RT_TABLEID_MAX) {
				yyerror("invalid rtable id");
				YYERROR;
			} else if (!lookup_rtable(yystack.l_mark[0].v.number)) {
				yyerror("rtable %lld does not exist", yystack.l_mark[0].v.number);
				YYERROR;
			}
			filter_opts.rtableid = yystack.l_mark[0].v.number;
		}
#line 6755 "parse.c"
break;
case 149:
#line 1988 "parse.y"
	{
			if (filter_opts.divert.type != PF_DIVERT_NONE) {
				yyerror("more than one divert option");
				YYERROR;
			}
			filter_opts.divert.type = PF_DIVERT_TO;
			if ((filter_opts.divert.addr = host(yystack.l_mark[-2].v.string, pf->opts)) == NULL) {
				yyerror("could not parse divert address: %s",
				    yystack.l_mark[-2].v.string);
				free(yystack.l_mark[-2].v.string);
				YYERROR;
			}
			free(yystack.l_mark[-2].v.string);
			filter_opts.divert.port = yystack.l_mark[0].v.range.a;
			if (!filter_opts.divert.port) {
				yyerror("invalid divert port: %u", ntohs(yystack.l_mark[0].v.range.a));
				YYERROR;
			}
		}
#line 6778 "parse.c"
break;
case 150:
#line 2007 "parse.y"
	{
			if (filter_opts.divert.type != PF_DIVERT_NONE) {
				yyerror("more than one divert option");
				YYERROR;
			}
			filter_opts.divert.type = PF_DIVERT_REPLY;
		}
#line 6789 "parse.c"
break;
case 151:
#line 2014 "parse.y"
	{
			if (filter_opts.divert.type != PF_DIVERT_NONE) {
				yyerror("more than one divert option");
				YYERROR;
			}
			filter_opts.divert.type = PF_DIVERT_PACKET;
			/*
			 * If IP reassembly was not turned off, also
			 * forcibly enable TCP reassembly by default.
			 */
			if (pf->reassemble & PF_REASS_ENABLED)
				filter_opts.marker |= FOM_SCRUB_TCP;

			filter_opts.divert.port = yystack.l_mark[0].v.range.a;
			if (!filter_opts.divert.port) {
				yyerror("invalid divert port: %u", ntohs(yystack.l_mark[0].v.range.a));
				YYERROR;
			}
		}
#line 6812 "parse.c"
break;
case 152:
#line 2033 "parse.y"
	{
			filter_opts.nodf = yystack.l_mark[-1].v.scrub_opts.nodf;
			filter_opts.minttl = yystack.l_mark[-1].v.scrub_opts.minttl;
			filter_opts.randomid = yystack.l_mark[-1].v.scrub_opts.randomid;
			filter_opts.max_mss = yystack.l_mark[-1].v.scrub_opts.maxmss;
			if (yystack.l_mark[-1].v.scrub_opts.reassemble_tcp)
				filter_opts.marker |= FOM_SCRUB_TCP;
			filter_opts.marker |= yystack.l_mark[-1].v.scrub_opts.marker;
		}
#line 6825 "parse.c"
break;
case 153:
#line 2042 "parse.y"
	{
			if (filter_opts.nat.rdr) {
				yyerror("cannot respecify nat-to/binat-to");
				YYERROR;
			}
			filter_opts.nat.rdr = yystack.l_mark[-1].v.redirection;
			memcpy(&filter_opts.nat.pool_opts, &yystack.l_mark[0].v.pool_opts,
			    sizeof(filter_opts.nat.pool_opts));
		}
#line 6838 "parse.c"
break;
case 154:
#line 2051 "parse.y"
	{
			if (filter_opts.nat.rdr) {
				yyerror("cannot respecify af-to");
				YYERROR;
			}
			if (yystack.l_mark[-3].v.i == 0) {
				yyerror("no target address family specified");
				YYERROR;
			}
			filter_opts.nat.af = yystack.l_mark[-3].v.i;
			filter_opts.nat.rdr = yystack.l_mark[-1].v.redirection;
			memcpy(&filter_opts.nat.pool_opts, &yystack.l_mark[0].v.pool_opts,
			    sizeof(filter_opts.nat.pool_opts));
			filter_opts.rdr.rdr =
			    calloc(1, sizeof(struct redirection));
			bzero(&filter_opts.rdr.pool_opts,
			    sizeof(filter_opts.rdr.pool_opts));
			filter_opts.marker |= FOM_AFTO;
		}
#line 6861 "parse.c"
break;
case 155:
#line 2070 "parse.y"
	{
			if (filter_opts.nat.rdr) {
				yyerror("cannot respecify af-to");
				YYERROR;
			}
			if (yystack.l_mark[-6].v.i == 0) {
				yyerror("no address family specified");
				YYERROR;
			}
			if ((yystack.l_mark[-4].v.redirection->host->af && yystack.l_mark[-4].v.redirection->host->af != yystack.l_mark[-6].v.i) ||
			    (yystack.l_mark[-1].v.redirection->host->af && yystack.l_mark[-1].v.redirection->host->af != yystack.l_mark[-6].v.i)) {
				yyerror("af-to addresses must be in the "
				    "target address family");
				YYERROR;
			}
			filter_opts.nat.af = yystack.l_mark[-6].v.i;
			filter_opts.nat.rdr = yystack.l_mark[-4].v.redirection;
			memcpy(&filter_opts.nat.pool_opts, &yystack.l_mark[-3].v.pool_opts,
			    sizeof(filter_opts.nat.pool_opts));
			filter_opts.rdr.af = yystack.l_mark[-6].v.i;
			filter_opts.rdr.rdr = yystack.l_mark[-1].v.redirection;
			memcpy(&filter_opts.nat.pool_opts, &yystack.l_mark[0].v.pool_opts,
			    sizeof(filter_opts.nat.pool_opts));
			filter_opts.marker |= FOM_AFTO;
		}
#line 6890 "parse.c"
break;
case 156:
#line 2095 "parse.y"
	{
			if (filter_opts.rdr.rdr) {
				yyerror("cannot respecify rdr-to");
				YYERROR;
			}
			filter_opts.rdr.rdr = yystack.l_mark[-1].v.redirection;
			memcpy(&filter_opts.rdr.pool_opts, &yystack.l_mark[0].v.pool_opts,
			    sizeof(filter_opts.rdr.pool_opts));
		}
#line 6903 "parse.c"
break;
case 157:
#line 2104 "parse.y"
	{
			if (filter_opts.nat.rdr) {
				yyerror("cannot respecify nat-to/binat-to");
				YYERROR;
			}
			filter_opts.nat.rdr = yystack.l_mark[-1].v.redirection;
			filter_opts.nat.binat = 1;
			memcpy(&filter_opts.nat.pool_opts, &yystack.l_mark[0].v.pool_opts,
			    sizeof(filter_opts.nat.pool_opts));
			filter_opts.nat.pool_opts.staticport = 1;
		}
#line 6918 "parse.c"
break;
case 158:
#line 2115 "parse.y"
	{
			filter_opts.rt = PF_ROUTETO;
		}
#line 6925 "parse.c"
break;
case 159:
#line 2118 "parse.y"
	{
			filter_opts.rt = PF_REPLYTO;
		}
#line 6932 "parse.c"
break;
case 160:
#line 2121 "parse.y"
	{
			filter_opts.rt = PF_DUPTO;
		}
#line 6939 "parse.c"
break;
case 161:
#line 2124 "parse.y"
	{
			if (filter_opts.rcv) {
				yyerror("cannot respecify received-on");
				YYERROR;
			}
			filter_opts.rcv = yystack.l_mark[0].v.interface;
			filter_opts.rcv->not = yystack.l_mark[-2].v.number;
		}
#line 6951 "parse.c"
break;
case 162:
#line 2132 "parse.y"
	{
			filter_opts.marker |= FOM_ONCE;
		}
#line 6958 "parse.c"
break;
case 163:
#line 2135 "parse.y"
	{
			if (yystack.l_mark[-2].v.number < 0 || yystack.l_mark[-2].v.number > UINT_MAX ||
			    yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > UINT_MAX) {
				yyerror("only positive values permitted");
				YYERROR;
			}
			if (filter_opts.pktrate.limit) {
				yyerror("cannot respecify max-pkt-rate");
				YYERROR;
			}
			filter_opts.pktrate.limit = yystack.l_mark[-2].v.number;
			filter_opts.pktrate.seconds = yystack.l_mark[0].v.number;
		}
#line 6975 "parse.c"
break;
case 165:
#line 2151 "parse.y"
	{ yyval.v.filter_opts = filter_opts; }
#line 6980 "parse.c"
break;
case 166:
#line 2152 "parse.y"
	{ yyval.v.filter_opts = filter_opts; }
#line 6985 "parse.c"
break;
case 169:
#line 2159 "parse.y"
	{
			if (filter_opts.marker & FOM_SETPRIO) {
				yyerror("prio cannot be redefined");
				YYERROR;
			}
			filter_opts.marker |= FOM_SETPRIO;
			filter_opts.set_prio[0] = yystack.l_mark[0].v.b.b1;
			filter_opts.set_prio[1] = yystack.l_mark[0].v.b.b2;
		}
#line 6998 "parse.c"
break;
case 170:
#line 2168 "parse.y"
	{
			if (filter_opts.queues.qname) {
				yyerror("queue cannot be redefined");
				YYERROR;
			}
			filter_opts.queues = yystack.l_mark[0].v.qassign;
		}
#line 7009 "parse.c"
break;
case 171:
#line 2175 "parse.y"
	{
			if (filter_opts.marker & FOM_SETTOS) {
				yyerror("tos cannot be respecified");
				YYERROR;
			}
			filter_opts.marker |= FOM_SETTOS;
			filter_opts.settos = yystack.l_mark[0].v.number;
		}
#line 7021 "parse.c"
break;
case 172:
#line 2183 "parse.y"
	{
			if (filter_opts.delay) {
				yyerror("delay cannot be respecified");
				YYERROR;
			}
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 0xffff) {
				yyerror("illegal delay value %lld (0-%u)", yystack.l_mark[0].v.number,
				    0xffff);
				YYERROR;
			}
			filter_opts.marker |= FOM_SETDELAY;
			filter_opts.delay = yystack.l_mark[0].v.number;
		}
#line 7038 "parse.c"
break;
case 173:
#line 2198 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > IFQ_MAXPRIO) {
				yyerror("prio must be 0 - %u", IFQ_MAXPRIO);
				YYERROR;
			}
			yyval.v.b.b1 = yyval.v.b.b2 = yystack.l_mark[0].v.number;
		}
#line 7049 "parse.c"
break;
case 174:
#line 2205 "parse.y"
	{
			if (yystack.l_mark[-3].v.number < 0 || yystack.l_mark[-3].v.number > IFQ_MAXPRIO ||
			    yystack.l_mark[-1].v.number < 0 || yystack.l_mark[-1].v.number > IFQ_MAXPRIO) {
				yyerror("prio must be 0 - %u", IFQ_MAXPRIO);
				YYERROR;
			}
			yyval.v.b.b1 = yystack.l_mark[-3].v.number;
			yyval.v.b.b2 = yystack.l_mark[-1].v.number;
		}
#line 7062 "parse.c"
break;
case 175:
#line 2216 "parse.y"
	{
			char	*e;
			double	 p = strtod(yystack.l_mark[0].v.string, &e);

			if (*e == '%') {
				p *= 0.01;
				e++;
			}
			if (*e) {
				yyerror("invalid probability: %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
			yyval.v.probability = p;
		}
#line 7082 "parse.c"
break;
case 176:
#line 2232 "parse.y"
	{
			yyval.v.probability = (double)yystack.l_mark[0].v.number;
		}
#line 7089 "parse.c"
break;
case 177:
#line 2238 "parse.y"
	{ yyval.v.b.b1 = PF_PASS; yyval.v.b.b2 = yyval.v.b.w = 0; }
#line 7094 "parse.c"
break;
case 178:
#line 2239 "parse.y"
	{ yyval.v.b.b1 = PF_MATCH; yyval.v.b.b2 = yyval.v.b.w = 0; }
#line 7099 "parse.c"
break;
case 179:
#line 2240 "parse.y"
	{ yyval.v.b = yystack.l_mark[0].v.b; yyval.v.b.b1 = PF_DROP; }
#line 7104 "parse.c"
break;
case 180:
#line 2243 "parse.y"
	{
			yyval.v.b.b2 = blockpolicy;
			yyval.v.b.w = returnicmpdefault;
			yyval.v.b.w2 = returnicmp6default;
		}
#line 7113 "parse.c"
break;
case 181:
#line 2248 "parse.y"
	{
			yyval.v.b.b2 = PFRULE_DROP;
			yyval.v.b.w = 0;
			yyval.v.b.w2 = 0;
		}
#line 7122 "parse.c"
break;
case 182:
#line 2253 "parse.y"
	{
			yyval.v.b.b2 = PFRULE_RETURNRST;
			yyval.v.b.w = 0;
			yyval.v.b.w2 = 0;
		}
#line 7131 "parse.c"
break;
case 183:
#line 2258 "parse.y"
	{
			if (yystack.l_mark[-1].v.number < 0 || yystack.l_mark[-1].v.number > 255) {
				yyerror("illegal ttl value %lld", yystack.l_mark[-1].v.number);
				YYERROR;
			}
			yyval.v.b.b2 = PFRULE_RETURNRST;
			yyval.v.b.w = yystack.l_mark[-1].v.number;
			yyval.v.b.w2 = 0;
		}
#line 7144 "parse.c"
break;
case 184:
#line 2267 "parse.y"
	{
			yyval.v.b.b2 = PFRULE_RETURNICMP;
			yyval.v.b.w = returnicmpdefault;
			yyval.v.b.w2 = returnicmp6default;
		}
#line 7153 "parse.c"
break;
case 185:
#line 2272 "parse.y"
	{
			yyval.v.b.b2 = PFRULE_RETURNICMP;
			yyval.v.b.w = returnicmpdefault;
			yyval.v.b.w2 = returnicmp6default;
		}
#line 7162 "parse.c"
break;
case 186:
#line 2277 "parse.y"
	{
			yyval.v.b.b2 = PFRULE_RETURNICMP;
			yyval.v.b.w = yystack.l_mark[-1].v.number;
			yyval.v.b.w2 = returnicmpdefault;
		}
#line 7171 "parse.c"
break;
case 187:
#line 2282 "parse.y"
	{
			yyval.v.b.b2 = PFRULE_RETURNICMP;
			yyval.v.b.w = returnicmpdefault;
			yyval.v.b.w2 = yystack.l_mark[-1].v.number;
		}
#line 7180 "parse.c"
break;
case 188:
#line 2287 "parse.y"
	{
			yyval.v.b.b2 = PFRULE_RETURNICMP;
			yyval.v.b.w = yystack.l_mark[-3].v.number;
			yyval.v.b.w2 = yystack.l_mark[-1].v.number;
		}
#line 7189 "parse.c"
break;
case 189:
#line 2292 "parse.y"
	{
			yyval.v.b.b2 = PFRULE_RETURN;
			yyval.v.b.w = returnicmpdefault;
			yyval.v.b.w2 = returnicmp6default;
		}
#line 7198 "parse.c"
break;
case 190:
#line 2299 "parse.y"
	{
			if (!(yyval.v.number = parseicmpspec(yystack.l_mark[0].v.string, AF_INET))) {
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 7209 "parse.c"
break;
case 191:
#line 2306 "parse.y"
	{
			u_int8_t		icmptype;

			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 255) {
				yyerror("invalid icmp code %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			icmptype = returnicmpdefault >> 8;
			yyval.v.number = (icmptype << 8 | yystack.l_mark[0].v.number);
		}
#line 7223 "parse.c"
break;
case 192:
#line 2318 "parse.y"
	{
			if (!(yyval.v.number = parseicmpspec(yystack.l_mark[0].v.string, AF_INET6))) {
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 7234 "parse.c"
break;
case 193:
#line 2325 "parse.y"
	{
			u_int8_t		icmptype;

			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 255) {
				yyerror("invalid icmp code %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			icmptype = returnicmp6default >> 8;
			yyval.v.number = (icmptype << 8 | yystack.l_mark[0].v.number);
		}
#line 7248 "parse.c"
break;
case 194:
#line 2337 "parse.y"
	{ yyval.v.i = PF_INOUT; }
#line 7253 "parse.c"
break;
case 195:
#line 2338 "parse.y"
	{ yyval.v.i = PF_IN; }
#line 7258 "parse.c"
break;
case 196:
#line 2339 "parse.y"
	{ yyval.v.i = PF_OUT; }
#line 7263 "parse.c"
break;
case 197:
#line 2342 "parse.y"
	{ yyval.v.logquick.quick = 0; }
#line 7268 "parse.c"
break;
case 198:
#line 2343 "parse.y"
	{ yyval.v.logquick.quick = 1; }
#line 7273 "parse.c"
break;
case 199:
#line 2346 "parse.y"
	{ yyval.v.logquick.log = 0; yyval.v.logquick.quick = 0; yyval.v.logquick.logif = 0; }
#line 7278 "parse.c"
break;
case 200:
#line 2347 "parse.y"
	{ yyval.v.logquick = yystack.l_mark[0].v.logquick; yyval.v.logquick.quick = 0; }
#line 7283 "parse.c"
break;
case 201:
#line 2348 "parse.y"
	{ yyval.v.logquick.quick = 1; yyval.v.logquick.log = 0; yyval.v.logquick.logif = 0; }
#line 7288 "parse.c"
break;
case 202:
#line 2349 "parse.y"
	{ yyval.v.logquick = yystack.l_mark[-1].v.logquick; yyval.v.logquick.quick = 1; }
#line 7293 "parse.c"
break;
case 203:
#line 2350 "parse.y"
	{ yyval.v.logquick = yystack.l_mark[0].v.logquick; yyval.v.logquick.quick = 1; }
#line 7298 "parse.c"
break;
case 204:
#line 2353 "parse.y"
	{ yyval.v.logquick.log = PF_LOG; yyval.v.logquick.logif = 0; }
#line 7303 "parse.c"
break;
case 205:
#line 2354 "parse.y"
	{
			yyval.v.logquick.log = PF_LOG | yystack.l_mark[-1].v.logquick.log;
			yyval.v.logquick.logif = yystack.l_mark[-1].v.logquick.logif;
		}
#line 7311 "parse.c"
break;
case 206:
#line 2360 "parse.y"
	{ yyval.v.logquick = yystack.l_mark[0].v.logquick; }
#line 7316 "parse.c"
break;
case 207:
#line 2361 "parse.y"
	{
			yyval.v.logquick.log = yystack.l_mark[-2].v.logquick.log | yystack.l_mark[0].v.logquick.log;
			yyval.v.logquick.logif = yystack.l_mark[0].v.logquick.logif;
			if (yyval.v.logquick.logif == 0)
				yyval.v.logquick.logif = yystack.l_mark[-2].v.logquick.logif;
		}
#line 7326 "parse.c"
break;
case 208:
#line 2369 "parse.y"
	{ yyval.v.logquick.log = PF_LOG_ALL; yyval.v.logquick.logif = 0; }
#line 7331 "parse.c"
break;
case 209:
#line 2370 "parse.y"
	{ yyval.v.logquick.log = PF_LOG_MATCHES; yyval.v.logquick.logif = 0; }
#line 7336 "parse.c"
break;
case 210:
#line 2371 "parse.y"
	{ yyval.v.logquick.log = PF_LOG_USER; yyval.v.logquick.logif = 0; }
#line 7341 "parse.c"
break;
case 211:
#line 2372 "parse.y"
	{
			const char	*errstr;
			u_int		 i;

			yyval.v.logquick.log = 0;
			if (strncmp(yystack.l_mark[0].v.string, "pflog", 5)) {
				yyerror("%s: should be a pflog interface", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			i = strtonum(yystack.l_mark[0].v.string + 5, 0, 255, &errstr);
			if (errstr) {
				yyerror("%s: %s", yystack.l_mark[0].v.string, errstr);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
			yyval.v.logquick.logif = i;
		}
#line 7364 "parse.c"
break;
case 212:
#line 2393 "parse.y"
	{ yyval.v.interface = NULL; }
#line 7369 "parse.c"
break;
case 213:
#line 2394 "parse.y"
	{ yyval.v.interface = yystack.l_mark[0].v.interface; }
#line 7374 "parse.c"
break;
case 214:
#line 2395 "parse.y"
	{ yyval.v.interface = yystack.l_mark[-1].v.interface; }
#line 7379 "parse.c"
break;
case 215:
#line 2398 "parse.y"
	{ yyval.v.interface = yystack.l_mark[-1].v.interface; }
#line 7384 "parse.c"
break;
case 216:
#line 2399 "parse.y"
	{
			yystack.l_mark[-3].v.interface->tail->next = yystack.l_mark[-1].v.interface;
			yystack.l_mark[-3].v.interface->tail = yystack.l_mark[-1].v.interface;
			yyval.v.interface = yystack.l_mark[-3].v.interface;
		}
#line 7393 "parse.c"
break;
case 217:
#line 2406 "parse.y"
	{ yyval.v.interface = yystack.l_mark[0].v.interface; yyval.v.interface->not = yystack.l_mark[-1].v.number; }
#line 7398 "parse.c"
break;
case 218:
#line 2409 "parse.y"
	{
			struct node_host	*n;

			yyval.v.interface = calloc(1, sizeof(struct node_if));
			if (yyval.v.interface == NULL)
				err(1, "if_item: calloc");
			if (strlcpy(yyval.v.interface->ifname, yystack.l_mark[0].v.string, sizeof(yyval.v.interface->ifname)) >=
			    sizeof(yyval.v.interface->ifname)) {
				free(yystack.l_mark[0].v.string);
				free(yyval.v.interface);
				yyerror("interface name too long");
				YYERROR;
			}

			if ((n = ifa_exists(yystack.l_mark[0].v.string)) != NULL)
				yyval.v.interface->ifa_flags = n->ifa_flags;

			free(yystack.l_mark[0].v.string);
			yyval.v.interface->not = 0;
			yyval.v.interface->next = NULL;
			yyval.v.interface->tail = yyval.v.interface;
		}
#line 7424 "parse.c"
break;
case 219:
#line 2431 "parse.y"
	{
			yyval.v.interface = calloc(1, sizeof(struct node_if));
			if (yyval.v.interface == NULL)
				err(1, "if_item: calloc");
			strlcpy(yyval.v.interface->ifname, "any", sizeof(yyval.v.interface->ifname));
			yyval.v.interface->not = 0;
			yyval.v.interface->next = NULL;
			yyval.v.interface->tail = yyval.v.interface;
		}
#line 7437 "parse.c"
break;
case 220:
#line 2440 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > RT_TABLEID_MAX)
				yyerror("rdomain %lld outside range", yystack.l_mark[0].v.number);

			yyval.v.interface = calloc(1, sizeof(struct node_if));
			if (yyval.v.interface == NULL)
				err(1, "if_item: calloc");
			yyval.v.interface->not = 0;
			yyval.v.interface->use_rdomain = 1;
			yyval.v.interface->rdomain = yystack.l_mark[0].v.number;
			yyval.v.interface->next = NULL;
			yyval.v.interface->tail = yyval.v.interface;
		}
#line 7454 "parse.c"
break;
case 221:
#line 2455 "parse.y"
	{ yyval.v.i = 0; }
#line 7459 "parse.c"
break;
case 222:
#line 2456 "parse.y"
	{ yyval.v.i = AF_INET; }
#line 7464 "parse.c"
break;
case 223:
#line 2457 "parse.y"
	{ yyval.v.i = AF_INET6; }
#line 7469 "parse.c"
break;
case 224:
#line 2460 "parse.y"
	{ yyval.v.proto = NULL; }
#line 7474 "parse.c"
break;
case 225:
#line 2461 "parse.y"
	{ yyval.v.proto = yystack.l_mark[0].v.proto; }
#line 7479 "parse.c"
break;
case 226:
#line 2462 "parse.y"
	{ yyval.v.proto = yystack.l_mark[-1].v.proto; }
#line 7484 "parse.c"
break;
case 227:
#line 2465 "parse.y"
	{ yyval.v.proto = yystack.l_mark[-1].v.proto; }
#line 7489 "parse.c"
break;
case 228:
#line 2466 "parse.y"
	{
			yystack.l_mark[-3].v.proto->tail->next = yystack.l_mark[-1].v.proto;
			yystack.l_mark[-3].v.proto->tail = yystack.l_mark[-1].v.proto;
			yyval.v.proto = yystack.l_mark[-3].v.proto;
		}
#line 7498 "parse.c"
break;
case 229:
#line 2473 "parse.y"
	{
			u_int8_t	pr;

			pr = (u_int8_t)yystack.l_mark[0].v.number;
			if (pr == 0) {
				yyerror("proto 0 cannot be used");
				YYERROR;
			}
			yyval.v.proto = calloc(1, sizeof(struct node_proto));
			if (yyval.v.proto == NULL)
				err(1, "proto_item: calloc");
			yyval.v.proto->proto = pr;
			yyval.v.proto->next = NULL;
			yyval.v.proto->tail = yyval.v.proto;
		}
#line 7517 "parse.c"
break;
case 230:
#line 2490 "parse.y"
	{
			struct protoent	*p;

			p = getprotobyname(yystack.l_mark[0].v.string);
			if (p == NULL) {
				yyerror("unknown protocol %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			yyval.v.number = p->p_proto;
			free(yystack.l_mark[0].v.string);
		}
#line 7533 "parse.c"
break;
case 231:
#line 2502 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 255) {
				yyerror("protocol outside range");
				YYERROR;
			}
		}
#line 7543 "parse.c"
break;
case 232:
#line 2510 "parse.y"
	{
			yyval.v.fromto.src.host = NULL;
			yyval.v.fromto.src.port = NULL;
			yyval.v.fromto.dst.host = NULL;
			yyval.v.fromto.dst.port = NULL;
			yyval.v.fromto.src_os = NULL;
		}
#line 7554 "parse.c"
break;
case 233:
#line 2517 "parse.y"
	{
			yyval.v.fromto.src = yystack.l_mark[-2].v.peer;
			yyval.v.fromto.src_os = yystack.l_mark[-1].v.os;
			yyval.v.fromto.dst = yystack.l_mark[0].v.peer;
		}
#line 7563 "parse.c"
break;
case 234:
#line 2524 "parse.y"
	{ yyval.v.os = NULL; }
#line 7568 "parse.c"
break;
case 235:
#line 2525 "parse.y"
	{ yyval.v.os = yystack.l_mark[0].v.os; }
#line 7573 "parse.c"
break;
case 236:
#line 2526 "parse.y"
	{ yyval.v.os = yystack.l_mark[-1].v.os; }
#line 7578 "parse.c"
break;
case 237:
#line 2529 "parse.y"
	{
			yyval.v.os = calloc(1, sizeof(struct node_os));
			if (yyval.v.os == NULL)
				err(1, "os: calloc");
			yyval.v.os->os = yystack.l_mark[0].v.string;
			yyval.v.os->tail = yyval.v.os;
		}
#line 7589 "parse.c"
break;
case 238:
#line 2538 "parse.y"
	{ yyval.v.os = yystack.l_mark[-1].v.os; }
#line 7594 "parse.c"
break;
case 239:
#line 2539 "parse.y"
	{
			yystack.l_mark[-3].v.os->tail->next = yystack.l_mark[-1].v.os;
			yystack.l_mark[-3].v.os->tail = yystack.l_mark[-1].v.os;
			yyval.v.os = yystack.l_mark[-3].v.os;
		}
#line 7603 "parse.c"
break;
case 240:
#line 2546 "parse.y"
	{
			yyval.v.peer.host = NULL;
			yyval.v.peer.port = NULL;
		}
#line 7611 "parse.c"
break;
case 241:
#line 2550 "parse.y"
	{
			yyval.v.peer = yystack.l_mark[0].v.peer;
		}
#line 7618 "parse.c"
break;
case 242:
#line 2555 "parse.y"
	{
			yyval.v.peer.host = NULL;
			yyval.v.peer.port = NULL;
		}
#line 7626 "parse.c"
break;
case 243:
#line 2559 "parse.y"
	{
			if (disallow_urpf_failed(yystack.l_mark[0].v.peer.host, "\"urpf-failed\" is "
			    "not permitted in a destination address"))
				YYERROR;
			yyval.v.peer = yystack.l_mark[0].v.peer;
		}
#line 7636 "parse.c"
break;
case 244:
#line 2567 "parse.y"
	{
			yyval.v.peer.host = yystack.l_mark[0].v.host;
			yyval.v.peer.port = NULL;
		}
#line 7644 "parse.c"
break;
case 245:
#line 2571 "parse.y"
	{
			yyval.v.peer.host = yystack.l_mark[-2].v.host;
			yyval.v.peer.port = yystack.l_mark[0].v.port;
		}
#line 7652 "parse.c"
break;
case 246:
#line 2575 "parse.y"
	{
			yyval.v.peer.host = NULL;
			yyval.v.peer.port = yystack.l_mark[0].v.port;
		}
#line 7660 "parse.c"
break;
case 249:
#line 2585 "parse.y"
	{ yyval.v.host = NULL; }
#line 7665 "parse.c"
break;
case 250:
#line 2586 "parse.y"
	{ yyval.v.host = yystack.l_mark[0].v.host; }
#line 7670 "parse.c"
break;
case 251:
#line 2587 "parse.y"
	{ yyval.v.host = yystack.l_mark[-1].v.host; }
#line 7675 "parse.c"
break;
case 252:
#line 2591 "parse.y"
	{ yyval.v.host = yystack.l_mark[-1].v.host; }
#line 7680 "parse.c"
break;
case 253:
#line 2592 "parse.y"
	{
			if (yystack.l_mark[-3].v.host == NULL) {
				freehostlist(yystack.l_mark[-1].v.host);
				yyval.v.host = yystack.l_mark[-3].v.host;
			} else if (yystack.l_mark[-1].v.host == NULL) {
				freehostlist(yystack.l_mark[-3].v.host);
				yyval.v.host = yystack.l_mark[-1].v.host;
			} else {
				yystack.l_mark[-3].v.host->tail->next = yystack.l_mark[-1].v.host;
				yystack.l_mark[-3].v.host->tail = yystack.l_mark[-1].v.host->tail;
				yyval.v.host = yystack.l_mark[-3].v.host;
			}
		}
#line 7697 "parse.c"
break;
case 254:
#line 2607 "parse.y"
	{
			struct node_host	*n;

			for (n = yystack.l_mark[0].v.host; n != NULL; n = n->next)
				n->not = yystack.l_mark[-1].v.number;
			yyval.v.host = yystack.l_mark[0].v.host;
		}
#line 7708 "parse.c"
break;
case 255:
#line 2614 "parse.y"
	{
			yyval.v.host = calloc(1, sizeof(struct node_host));
			if (yyval.v.host == NULL)
				err(1, "xhost: calloc");
			yyval.v.host->addr.type = PF_ADDR_NOROUTE;
			yyval.v.host->next = NULL;
			yyval.v.host->not = yystack.l_mark[-1].v.number;
			yyval.v.host->tail = yyval.v.host;
		}
#line 7721 "parse.c"
break;
case 256:
#line 2623 "parse.y"
	{
			yyval.v.host = calloc(1, sizeof(struct node_host));
			if (yyval.v.host == NULL)
				err(1, "xhost: calloc");
			yyval.v.host->addr.type = PF_ADDR_URPFFAILED;
			yyval.v.host->next = NULL;
			yyval.v.host->not = yystack.l_mark[-1].v.number;
			yyval.v.host->tail = yyval.v.host;
		}
#line 7734 "parse.c"
break;
case 257:
#line 2634 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 1 || yystack.l_mark[0].v.number > USHRT_MAX) {
				yyerror("weight out of range");
				YYERROR;
			}
			yyval.v.weight = yystack.l_mark[0].v.number;
		}
#line 7745 "parse.c"
break;
case 258:
#line 2641 "parse.y"
	{ yyval.v.weight = 0; }
#line 7750 "parse.c"
break;
case 259:
#line 2644 "parse.y"
	{
			if ((yyval.v.host = host(yystack.l_mark[0].v.string, pf->opts)) == NULL)	{
				/* error. "any" is handled elsewhere */
				free(yystack.l_mark[0].v.string);
				yyerror("could not parse host specification");
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);

		}
#line 7764 "parse.c"
break;
case 260:
#line 2654 "parse.y"
	{
			struct node_host *b, *e;

			if ((b = host(yystack.l_mark[-2].v.string, pf->opts)) == NULL ||
			    (e = host(yystack.l_mark[0].v.string, pf->opts)) == NULL) {
				free(yystack.l_mark[-2].v.string);
				free(yystack.l_mark[0].v.string);
				yyerror("could not parse host specification");
				YYERROR;
			}
			if (b->af != e->af ||
			    b->addr.type != PF_ADDR_ADDRMASK ||
			    e->addr.type != PF_ADDR_ADDRMASK ||
			    unmask(&b->addr.v.a.mask) !=
			    (b->af == AF_INET ? 32 : 128) ||
			    unmask(&e->addr.v.a.mask) !=
			    (e->af == AF_INET ? 32 : 128) ||
			    b->next != NULL || b->not ||
			    e->next != NULL || e->not) {
				free(b);
				free(e);
				free(yystack.l_mark[-2].v.string);
				free(yystack.l_mark[0].v.string);
				yyerror("invalid address range");
				YYERROR;
			}
			memcpy(&b->addr.v.a.mask, &e->addr.v.a.addr,
			    sizeof(b->addr.v.a.mask));
			b->addr.type = PF_ADDR_RANGE;
			yyval.v.host = b;
			free(e);
			free(yystack.l_mark[-2].v.string);
			free(yystack.l_mark[0].v.string);
		}
#line 7802 "parse.c"
break;
case 261:
#line 2688 "parse.y"
	{
			char	*buf;

			if (asprintf(&buf, "%s/%lld", yystack.l_mark[-2].v.string, yystack.l_mark[0].v.number) == -1)
				err(1, "host: asprintf");
			free(yystack.l_mark[-2].v.string);
			if ((yyval.v.host = host(buf, pf->opts)) == NULL)	{
				/* error. "any" is handled elsewhere */
				free(buf);
				yyerror("could not parse host specification");
				YYERROR;
			}
			free(buf);
		}
#line 7820 "parse.c"
break;
case 262:
#line 2702 "parse.y"
	{
			char	*buf;

			/* ie. for 10/8 parsing */
			if (asprintf(&buf, "%lld/%lld", yystack.l_mark[-2].v.number, yystack.l_mark[0].v.number) == -1)
				err(1, "host: asprintf");
			if ((yyval.v.host = host(buf, pf->opts)) == NULL)	{
				/* error. "any" is handled elsewhere */
				free(buf);
				yyerror("could not parse host specification");
				YYERROR;
			}
			free(buf);
		}
#line 7838 "parse.c"
break;
case 264:
#line 2717 "parse.y"
	{
			struct node_host	*n;

			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 128) {
				yyerror("bit number too big");
				YYERROR;
			}
			yyval.v.host = yystack.l_mark[-2].v.host;
			for (n = yystack.l_mark[-2].v.host; n != NULL; n = n->next)
				set_ipmask(n, yystack.l_mark[0].v.number);
		}
#line 7853 "parse.c"
break;
case 265:
#line 2728 "parse.y"
	{
			if (strlen(yystack.l_mark[-1].v.string) >= PF_TABLE_NAME_SIZE) {
				yyerror("table name '%s' too long", yystack.l_mark[-1].v.string);
				free(yystack.l_mark[-1].v.string);
				YYERROR;
			}
			yyval.v.host = calloc(1, sizeof(struct node_host));
			if (yyval.v.host == NULL)
				err(1, "host: calloc");
			yyval.v.host->addr.type = PF_ADDR_TABLE;
			if (strlcpy(yyval.v.host->addr.v.tblname, yystack.l_mark[-1].v.string,
			    sizeof(yyval.v.host->addr.v.tblname)) >=
			    sizeof(yyval.v.host->addr.v.tblname))
				errx(1, "host: strlcpy");
			free(yystack.l_mark[-1].v.string);
			yyval.v.host->next = NULL;
			yyval.v.host->tail = yyval.v.host;
		}
#line 7875 "parse.c"
break;
case 266:
#line 2746 "parse.y"
	{
			yyval.v.host = calloc(1, sizeof(struct node_host));
			if (yyval.v.host == NULL) {
				free(yystack.l_mark[0].v.string);
				err(1, "host: calloc");
			}
			yyval.v.host->addr.type = PF_ADDR_RTLABEL;
			if (strlcpy(yyval.v.host->addr.v.rtlabelname, yystack.l_mark[0].v.string,
			    sizeof(yyval.v.host->addr.v.rtlabelname)) >=
			    sizeof(yyval.v.host->addr.v.rtlabelname)) {
				yyerror("route label too long, max %zu chars",
				    sizeof(yyval.v.host->addr.v.rtlabelname) - 1);
				free(yystack.l_mark[0].v.string);
				free(yyval.v.host);
				YYERROR;
			}
			yyval.v.host->next = NULL;
			yyval.v.host->tail = yyval.v.host;
			free(yystack.l_mark[0].v.string);
		}
#line 7899 "parse.c"
break;
case 268:
#line 2769 "parse.y"
	{
			u_long	ulval;

			if (atoul(yystack.l_mark[0].v.string, &ulval) == -1) {
				yyerror("%s is not a number", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			} else
				yyval.v.number = ulval;
			free(yystack.l_mark[0].v.string);
		}
#line 7914 "parse.c"
break;
case 269:
#line 2782 "parse.y"
	{
			int	 flags = 0;
			char	*p, *op;

			op = yystack.l_mark[-1].v.string;
			if (!isalpha((unsigned char)op[0])) {
				yyerror("invalid interface name '%s'", op);
				free(op);
				YYERROR;
			}
			while ((p = strrchr(yystack.l_mark[-1].v.string, ':')) != NULL) {
				if (!strcmp(p+1, "network"))
					flags |= PFI_AFLAG_NETWORK;
				else if (!strcmp(p+1, "broadcast"))
					flags |= PFI_AFLAG_BROADCAST;
				else if (!strcmp(p+1, "peer"))
					flags |= PFI_AFLAG_PEER;
				else if (!strcmp(p+1, "0"))
					flags |= PFI_AFLAG_NOALIAS;
				else {
					yyerror("interface %s has bad modifier",
					    yystack.l_mark[-1].v.string);
					free(op);
					YYERROR;
				}
				*p = '\0';
			}
			if (flags & (flags - 1) & PFI_AFLAG_MODEMASK) {
				free(op);
				yyerror("illegal combination of "
				    "interface modifiers");
				YYERROR;
			}
			yyval.v.host = calloc(1, sizeof(struct node_host));
			if (yyval.v.host == NULL)
				err(1, "address: calloc");
			yyval.v.host->af = 0;
			set_ipmask(yyval.v.host, 128);
			yyval.v.host->addr.type = PF_ADDR_DYNIFTL;
			yyval.v.host->addr.iflags = flags;
			if (strlcpy(yyval.v.host->addr.v.ifname, yystack.l_mark[-1].v.string,
			    sizeof(yyval.v.host->addr.v.ifname)) >=
			    sizeof(yyval.v.host->addr.v.ifname)) {
				free(op);
				free(yyval.v.host);
				yyerror("interface name too long");
				YYERROR;
			}
			free(op);
			yyval.v.host->next = NULL;
			yyval.v.host->tail = yyval.v.host;
		}
#line 7970 "parse.c"
break;
case 270:
#line 2836 "parse.y"
	{ yyval.v.port = yystack.l_mark[0].v.port; }
#line 7975 "parse.c"
break;
case 271:
#line 2837 "parse.y"
	{ yyval.v.port = yystack.l_mark[-1].v.port; }
#line 7980 "parse.c"
break;
case 272:
#line 2840 "parse.y"
	{ yyval.v.port = yystack.l_mark[-1].v.port; }
#line 7985 "parse.c"
break;
case 273:
#line 2841 "parse.y"
	{
			yystack.l_mark[-3].v.port->tail->next = yystack.l_mark[-1].v.port;
			yystack.l_mark[-3].v.port->tail = yystack.l_mark[-1].v.port;
			yyval.v.port = yystack.l_mark[-3].v.port;
		}
#line 7994 "parse.c"
break;
case 274:
#line 2848 "parse.y"
	{
			yyval.v.port = calloc(1, sizeof(struct node_port));
			if (yyval.v.port == NULL)
				err(1, "port_item: calloc");
			yyval.v.port->port[0] = yystack.l_mark[0].v.range.a;
			yyval.v.port->port[1] = yystack.l_mark[0].v.range.b;
			if (yystack.l_mark[0].v.range.t) {
				yyval.v.port->op = PF_OP_RRG;

				if (validate_range(yyval.v.port->op, yyval.v.port->port[0],
				    yyval.v.port->port[1])) {
					yyerror("invalid port range");
					YYERROR;
				}
			} else
				yyval.v.port->op = PF_OP_EQ;
			yyval.v.port->next = NULL;
			yyval.v.port->tail = yyval.v.port;
		}
#line 8017 "parse.c"
break;
case 275:
#line 2867 "parse.y"
	{
			if (yystack.l_mark[0].v.range.t) {
				yyerror("':' cannot be used with an other "
				    "port operator");
				YYERROR;
			}
			yyval.v.port = calloc(1, sizeof(struct node_port));
			if (yyval.v.port == NULL)
				err(1, "port_item: calloc");
			yyval.v.port->port[0] = yystack.l_mark[0].v.range.a;
			yyval.v.port->port[1] = yystack.l_mark[0].v.range.b;
			yyval.v.port->op = yystack.l_mark[-1].v.i;
			yyval.v.port->next = NULL;
			yyval.v.port->tail = yyval.v.port;
		}
#line 8036 "parse.c"
break;
case 276:
#line 2882 "parse.y"
	{
			if (yystack.l_mark[-2].v.range.t || yystack.l_mark[0].v.range.t) {
				yyerror("':' cannot be used with an other "
				    "port operator");
				YYERROR;
			}
			yyval.v.port = calloc(1, sizeof(struct node_port));
			if (yyval.v.port == NULL)
				err(1, "port_item: calloc");
			yyval.v.port->port[0] = yystack.l_mark[-2].v.range.a;
			yyval.v.port->port[1] = yystack.l_mark[0].v.range.a;
			yyval.v.port->op = yystack.l_mark[-1].v.i;
			if (validate_range(yyval.v.port->op, yyval.v.port->port[0], yyval.v.port->port[1])) {
				yyerror("invalid port range");
				YYERROR;
			}
			yyval.v.port->next = NULL;
			yyval.v.port->tail = yyval.v.port;
		}
#line 8059 "parse.c"
break;
case 277:
#line 2903 "parse.y"
	{
			if (parseport(yystack.l_mark[0].v.string, &yyval.v.range, 0) == -1) {
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 8070 "parse.c"
break;
case 278:
#line 2912 "parse.y"
	{
			if (parseport(yystack.l_mark[0].v.string, &yyval.v.range, PPORT_RANGE) == -1) {
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 8081 "parse.c"
break;
case 279:
#line 2921 "parse.y"
	{ yyval.v.uid = yystack.l_mark[0].v.uid; }
#line 8086 "parse.c"
break;
case 280:
#line 2922 "parse.y"
	{ yyval.v.uid = yystack.l_mark[-1].v.uid; }
#line 8091 "parse.c"
break;
case 281:
#line 2925 "parse.y"
	{ yyval.v.uid = yystack.l_mark[-1].v.uid; }
#line 8096 "parse.c"
break;
case 282:
#line 2926 "parse.y"
	{
			yystack.l_mark[-3].v.uid->tail->next = yystack.l_mark[-1].v.uid;
			yystack.l_mark[-3].v.uid->tail = yystack.l_mark[-1].v.uid;
			yyval.v.uid = yystack.l_mark[-3].v.uid;
		}
#line 8105 "parse.c"
break;
case 283:
#line 2933 "parse.y"
	{
			yyval.v.uid = calloc(1, sizeof(struct node_uid));
			if (yyval.v.uid == NULL)
				err(1, "uid_item: calloc");
			yyval.v.uid->uid[0] = yystack.l_mark[0].v.number;
			yyval.v.uid->uid[1] = yystack.l_mark[0].v.number;
			yyval.v.uid->op = PF_OP_EQ;
			yyval.v.uid->next = NULL;
			yyval.v.uid->tail = yyval.v.uid;
		}
#line 8119 "parse.c"
break;
case 284:
#line 2943 "parse.y"
	{
			if (yystack.l_mark[0].v.number == -1 && yystack.l_mark[-1].v.i != PF_OP_EQ && yystack.l_mark[-1].v.i != PF_OP_NE) {
				yyerror("user unknown requires operator = or "
				    "!=");
				YYERROR;
			}
			yyval.v.uid = calloc(1, sizeof(struct node_uid));
			if (yyval.v.uid == NULL)
				err(1, "uid_item: calloc");
			yyval.v.uid->uid[0] = yystack.l_mark[0].v.number;
			yyval.v.uid->uid[1] = yystack.l_mark[0].v.number;
			yyval.v.uid->op = yystack.l_mark[-1].v.i;
			yyval.v.uid->next = NULL;
			yyval.v.uid->tail = yyval.v.uid;
		}
#line 8138 "parse.c"
break;
case 285:
#line 2958 "parse.y"
	{
			if (yystack.l_mark[-2].v.number == -1 || yystack.l_mark[0].v.number == -1) {
				yyerror("user unknown requires operator = or "
				    "!=");
				YYERROR;
			}
			yyval.v.uid = calloc(1, sizeof(struct node_uid));
			if (yyval.v.uid == NULL)
				err(1, "uid_item: calloc");
			yyval.v.uid->uid[0] = yystack.l_mark[-2].v.number;
			yyval.v.uid->uid[1] = yystack.l_mark[0].v.number;
			yyval.v.uid->op = yystack.l_mark[-1].v.i;
			yyval.v.uid->next = NULL;
			yyval.v.uid->tail = yyval.v.uid;
		}
#line 8157 "parse.c"
break;
case 286:
#line 2975 "parse.y"
	{
			if (!strcmp(yystack.l_mark[0].v.string, "unknown"))
				yyval.v.number = -1;
			else {
				uid_t uid;

				if (uid_from_user(yystack.l_mark[0].v.string, &uid) == -1) {
					yyerror("unknown user %s", yystack.l_mark[0].v.string);
					free(yystack.l_mark[0].v.string);
					YYERROR;
				}
				yyval.v.number = uid;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 8176 "parse.c"
break;
case 287:
#line 2990 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number >= UID_MAX) {
				yyerror("illegal uid value %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			yyval.v.number = yystack.l_mark[0].v.number;
		}
#line 8187 "parse.c"
break;
case 288:
#line 2999 "parse.y"
	{ yyval.v.gid = yystack.l_mark[0].v.gid; }
#line 8192 "parse.c"
break;
case 289:
#line 3000 "parse.y"
	{ yyval.v.gid = yystack.l_mark[-1].v.gid; }
#line 8197 "parse.c"
break;
case 290:
#line 3003 "parse.y"
	{ yyval.v.gid = yystack.l_mark[-1].v.gid; }
#line 8202 "parse.c"
break;
case 291:
#line 3004 "parse.y"
	{
			yystack.l_mark[-3].v.gid->tail->next = yystack.l_mark[-1].v.gid;
			yystack.l_mark[-3].v.gid->tail = yystack.l_mark[-1].v.gid;
			yyval.v.gid = yystack.l_mark[-3].v.gid;
		}
#line 8211 "parse.c"
break;
case 292:
#line 3011 "parse.y"
	{
			yyval.v.gid = calloc(1, sizeof(struct node_gid));
			if (yyval.v.gid == NULL)
				err(1, "gid_item: calloc");
			yyval.v.gid->gid[0] = yystack.l_mark[0].v.number;
			yyval.v.gid->gid[1] = yystack.l_mark[0].v.number;
			yyval.v.gid->op = PF_OP_EQ;
			yyval.v.gid->next = NULL;
			yyval.v.gid->tail = yyval.v.gid;
		}
#line 8225 "parse.c"
break;
case 293:
#line 3021 "parse.y"
	{
			if (yystack.l_mark[0].v.number == -1 && yystack.l_mark[-1].v.i != PF_OP_EQ && yystack.l_mark[-1].v.i != PF_OP_NE) {
				yyerror("group unknown requires operator = or "
				    "!=");
				YYERROR;
			}
			yyval.v.gid = calloc(1, sizeof(struct node_gid));
			if (yyval.v.gid == NULL)
				err(1, "gid_item: calloc");
			yyval.v.gid->gid[0] = yystack.l_mark[0].v.number;
			yyval.v.gid->gid[1] = yystack.l_mark[0].v.number;
			yyval.v.gid->op = yystack.l_mark[-1].v.i;
			yyval.v.gid->next = NULL;
			yyval.v.gid->tail = yyval.v.gid;
		}
#line 8244 "parse.c"
break;
case 294:
#line 3036 "parse.y"
	{
			if (yystack.l_mark[-2].v.number == -1 || yystack.l_mark[0].v.number == -1) {
				yyerror("group unknown requires operator = or "
				    "!=");
				YYERROR;
			}
			yyval.v.gid = calloc(1, sizeof(struct node_gid));
			if (yyval.v.gid == NULL)
				err(1, "gid_item: calloc");
			yyval.v.gid->gid[0] = yystack.l_mark[-2].v.number;
			yyval.v.gid->gid[1] = yystack.l_mark[0].v.number;
			yyval.v.gid->op = yystack.l_mark[-1].v.i;
			yyval.v.gid->next = NULL;
			yyval.v.gid->tail = yyval.v.gid;
		}
#line 8263 "parse.c"
break;
case 295:
#line 3053 "parse.y"
	{
			if (!strcmp(yystack.l_mark[0].v.string, "unknown"))
				yyval.v.number = -1;
			else {
				gid_t gid;

				if (gid_from_group(yystack.l_mark[0].v.string, &gid) == -1) {
					yyerror("unknown group %s", yystack.l_mark[0].v.string);
					free(yystack.l_mark[0].v.string);
					YYERROR;
				}
				yyval.v.number = gid;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 8282 "parse.c"
break;
case 296:
#line 3068 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number >= GID_MAX) {
				yyerror("illegal gid value %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			yyval.v.number = yystack.l_mark[0].v.number;
		}
#line 8293 "parse.c"
break;
case 297:
#line 3077 "parse.y"
	{
			int	f;

			if ((f = parse_flags(yystack.l_mark[0].v.string)) < 0) {
				yyerror("bad flags %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
			yyval.v.b.b1 = f;
		}
#line 8308 "parse.c"
break;
case 298:
#line 3090 "parse.y"
	{ yyval.v.b.b1 = yystack.l_mark[-2].v.b.b1; yyval.v.b.b2 = yystack.l_mark[0].v.b.b1; }
#line 8313 "parse.c"
break;
case 299:
#line 3091 "parse.y"
	{ yyval.v.b.b1 = 0; yyval.v.b.b2 = yystack.l_mark[0].v.b.b1; }
#line 8318 "parse.c"
break;
case 300:
#line 3092 "parse.y"
	{ yyval.v.b.b1 = 0; yyval.v.b.b2 = 0; }
#line 8323 "parse.c"
break;
case 301:
#line 3095 "parse.y"
	{ yyval.v.icmp = yystack.l_mark[0].v.icmp; }
#line 8328 "parse.c"
break;
case 302:
#line 3096 "parse.y"
	{ yyval.v.icmp = yystack.l_mark[-1].v.icmp; }
#line 8333 "parse.c"
break;
case 303:
#line 3097 "parse.y"
	{ yyval.v.icmp = yystack.l_mark[0].v.icmp; }
#line 8338 "parse.c"
break;
case 304:
#line 3098 "parse.y"
	{ yyval.v.icmp = yystack.l_mark[-1].v.icmp; }
#line 8343 "parse.c"
break;
case 305:
#line 3101 "parse.y"
	{ yyval.v.icmp = yystack.l_mark[-1].v.icmp; }
#line 8348 "parse.c"
break;
case 306:
#line 3102 "parse.y"
	{
			yystack.l_mark[-3].v.icmp->tail->next = yystack.l_mark[-1].v.icmp;
			yystack.l_mark[-3].v.icmp->tail = yystack.l_mark[-1].v.icmp;
			yyval.v.icmp = yystack.l_mark[-3].v.icmp;
		}
#line 8357 "parse.c"
break;
case 307:
#line 3109 "parse.y"
	{ yyval.v.icmp = yystack.l_mark[-1].v.icmp; }
#line 8362 "parse.c"
break;
case 308:
#line 3110 "parse.y"
	{
			yystack.l_mark[-3].v.icmp->tail->next = yystack.l_mark[-1].v.icmp;
			yystack.l_mark[-3].v.icmp->tail = yystack.l_mark[-1].v.icmp;
			yyval.v.icmp = yystack.l_mark[-3].v.icmp;
		}
#line 8371 "parse.c"
break;
case 309:
#line 3117 "parse.y"
	{
			yyval.v.icmp = calloc(1, sizeof(struct node_icmp));
			if (yyval.v.icmp == NULL)
				err(1, "icmp_item: calloc");
			yyval.v.icmp->type = yystack.l_mark[0].v.number;
			yyval.v.icmp->code = 0;
			yyval.v.icmp->proto = IPPROTO_ICMP;
			yyval.v.icmp->next = NULL;
			yyval.v.icmp->tail = yyval.v.icmp;
		}
#line 8385 "parse.c"
break;
case 310:
#line 3127 "parse.y"
	{
			const struct icmpcodeent	*p;

			if ((p = geticmpcodebyname(yystack.l_mark[-2].v.number-1, yystack.l_mark[0].v.string, AF_INET)) == NULL) {
				yyerror("unknown icmp-code %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}

			free(yystack.l_mark[0].v.string);
			yyval.v.icmp = calloc(1, sizeof(struct node_icmp));
			if (yyval.v.icmp == NULL)
				err(1, "icmp_item: calloc");
			yyval.v.icmp->type = yystack.l_mark[-2].v.number;
			yyval.v.icmp->code = p->code + 1;
			yyval.v.icmp->proto = IPPROTO_ICMP;
			yyval.v.icmp->next = NULL;
			yyval.v.icmp->tail = yyval.v.icmp;
		}
#line 8408 "parse.c"
break;
case 311:
#line 3146 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 255) {
				yyerror("illegal icmp-code %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			yyval.v.icmp = calloc(1, sizeof(struct node_icmp));
			if (yyval.v.icmp == NULL)
				err(1, "icmp_item: calloc");
			yyval.v.icmp->type = yystack.l_mark[-2].v.number;
			yyval.v.icmp->code = yystack.l_mark[0].v.number + 1;
			yyval.v.icmp->proto = IPPROTO_ICMP;
			yyval.v.icmp->next = NULL;
			yyval.v.icmp->tail = yyval.v.icmp;
		}
#line 8426 "parse.c"
break;
case 312:
#line 3162 "parse.y"
	{
			yyval.v.icmp = calloc(1, sizeof(struct node_icmp));
			if (yyval.v.icmp == NULL)
				err(1, "icmp_item: calloc");
			yyval.v.icmp->type = yystack.l_mark[0].v.number;
			yyval.v.icmp->code = 0;
			yyval.v.icmp->proto = IPPROTO_ICMPV6;
			yyval.v.icmp->next = NULL;
			yyval.v.icmp->tail = yyval.v.icmp;
		}
#line 8440 "parse.c"
break;
case 313:
#line 3172 "parse.y"
	{
			const struct icmpcodeent	*p;

			if ((p = geticmpcodebyname(yystack.l_mark[-2].v.number-1, yystack.l_mark[0].v.string, AF_INET6)) == NULL) {
				yyerror("unknown icmp6-code %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);

			yyval.v.icmp = calloc(1, sizeof(struct node_icmp));
			if (yyval.v.icmp == NULL)
				err(1, "icmp_item: calloc");
			yyval.v.icmp->type = yystack.l_mark[-2].v.number;
			yyval.v.icmp->code = p->code + 1;
			yyval.v.icmp->proto = IPPROTO_ICMPV6;
			yyval.v.icmp->next = NULL;
			yyval.v.icmp->tail = yyval.v.icmp;
		}
#line 8463 "parse.c"
break;
case 314:
#line 3191 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 255) {
				yyerror("illegal icmp-code %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			yyval.v.icmp = calloc(1, sizeof(struct node_icmp));
			if (yyval.v.icmp == NULL)
				err(1, "icmp_item: calloc");
			yyval.v.icmp->type = yystack.l_mark[-2].v.number;
			yyval.v.icmp->code = yystack.l_mark[0].v.number + 1;
			yyval.v.icmp->proto = IPPROTO_ICMPV6;
			yyval.v.icmp->next = NULL;
			yyval.v.icmp->tail = yyval.v.icmp;
		}
#line 8481 "parse.c"
break;
case 315:
#line 3207 "parse.y"
	{
			const struct icmptypeent	*p;

			if ((p = geticmptypebyname(yystack.l_mark[0].v.string, AF_INET)) == NULL) {
				yyerror("unknown icmp-type %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			yyval.v.number = p->type + 1;
			free(yystack.l_mark[0].v.string);
		}
#line 8496 "parse.c"
break;
case 316:
#line 3218 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 255) {
				yyerror("illegal icmp-type %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			yyval.v.number = yystack.l_mark[0].v.number + 1;
		}
#line 8507 "parse.c"
break;
case 317:
#line 3227 "parse.y"
	{
			const struct icmptypeent	*p;

			if ((p = geticmptypebyname(yystack.l_mark[0].v.string, AF_INET6)) ==
			    NULL) {
				yyerror("unknown icmp6-type %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			yyval.v.number = p->type + 1;
			free(yystack.l_mark[0].v.string);
		}
#line 8523 "parse.c"
break;
case 318:
#line 3239 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 255) {
				yyerror("illegal icmp6-type %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			yyval.v.number = yystack.l_mark[0].v.number + 1;
		}
#line 8534 "parse.c"
break;
case 319:
#line 3248 "parse.y"
	{
			int val;
			char *end;

			if (map_tos(yystack.l_mark[0].v.string, &val))
				yyval.v.number = val;
			else if (yystack.l_mark[0].v.string[0] == '0' && yystack.l_mark[0].v.string[1] == 'x') {
				errno = 0;
				yyval.v.number = strtoul(yystack.l_mark[0].v.string, &end, 16);
				if (errno || *end != '\0')
					yyval.v.number = 256;
			} else
				yyval.v.number = 256;		/* flag bad argument */
			if (yyval.v.number < 0 || yyval.v.number > 255) {
				yyerror("illegal tos value %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 8558 "parse.c"
break;
case 320:
#line 3268 "parse.y"
	{
			yyval.v.number = yystack.l_mark[0].v.number;
			if (yyval.v.number < 0 || yyval.v.number > 255) {
				yyerror("illegal tos value %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
		}
#line 8569 "parse.c"
break;
case 321:
#line 3277 "parse.y"
	{ yyval.v.i = PF_SRCTRACK; }
#line 8574 "parse.c"
break;
case 322:
#line 3278 "parse.y"
	{ yyval.v.i = PF_SRCTRACK_GLOBAL; }
#line 8579 "parse.c"
break;
case 323:
#line 3279 "parse.y"
	{ yyval.v.i = PF_SRCTRACK_RULE; }
#line 8584 "parse.c"
break;
case 324:
#line 3282 "parse.y"
	{
			yyval.v.i = PFRULE_IFBOUND;
		}
#line 8591 "parse.c"
break;
case 325:
#line 3285 "parse.y"
	{
			yyval.v.i = 0;
		}
#line 8598 "parse.c"
break;
case 326:
#line 3290 "parse.y"
	{
			yyval.v.keep_state.action = 0;
			yyval.v.keep_state.options = NULL;
		}
#line 8606 "parse.c"
break;
case 327:
#line 3294 "parse.y"
	{
			yyval.v.keep_state.action = PF_STATE_NORMAL;
			yyval.v.keep_state.options = yystack.l_mark[0].v.state_opt;
		}
#line 8614 "parse.c"
break;
case 328:
#line 3298 "parse.y"
	{
			yyval.v.keep_state.action = PF_STATE_MODULATE;
			yyval.v.keep_state.options = yystack.l_mark[0].v.state_opt;
		}
#line 8622 "parse.c"
break;
case 329:
#line 3302 "parse.y"
	{
			yyval.v.keep_state.action = PF_STATE_SYNPROXY;
			yyval.v.keep_state.options = yystack.l_mark[0].v.state_opt;
		}
#line 8630 "parse.c"
break;
case 330:
#line 3308 "parse.y"
	{ yyval.v.i = 0; }
#line 8635 "parse.c"
break;
case 331:
#line 3309 "parse.y"
	{ yyval.v.i = PF_FLUSH; }
#line 8640 "parse.c"
break;
case 332:
#line 3310 "parse.y"
	{
			yyval.v.i = PF_FLUSH | PF_FLUSH_GLOBAL;
		}
#line 8647 "parse.c"
break;
case 333:
#line 3315 "parse.y"
	{ yyval.v.state_opt = yystack.l_mark[-1].v.state_opt; }
#line 8652 "parse.c"
break;
case 334:
#line 3316 "parse.y"
	{ yyval.v.state_opt = NULL; }
#line 8657 "parse.c"
break;
case 335:
#line 3319 "parse.y"
	{ yyval.v.state_opt = yystack.l_mark[0].v.state_opt; }
#line 8662 "parse.c"
break;
case 336:
#line 3320 "parse.y"
	{
			yystack.l_mark[-2].v.state_opt->tail->next = yystack.l_mark[0].v.state_opt;
			yystack.l_mark[-2].v.state_opt->tail = yystack.l_mark[0].v.state_opt;
			yyval.v.state_opt = yystack.l_mark[-2].v.state_opt;
		}
#line 8671 "parse.c"
break;
case 337:
#line 3327 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > UINT_MAX) {
				yyerror("only positive values permitted");
				YYERROR;
			}
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			yyval.v.state_opt->type = PF_STATE_OPT_MAX;
			yyval.v.state_opt->data.max_states = yystack.l_mark[0].v.number;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8688 "parse.c"
break;
case 338:
#line 3340 "parse.y"
	{
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			yyval.v.state_opt->type = PF_STATE_OPT_NOSYNC;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8700 "parse.c"
break;
case 339:
#line 3348 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > UINT_MAX) {
				yyerror("only positive values permitted");
				YYERROR;
			}
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			yyval.v.state_opt->type = PF_STATE_OPT_MAX_SRC_STATES;
			yyval.v.state_opt->data.max_src_states = yystack.l_mark[0].v.number;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8717 "parse.c"
break;
case 340:
#line 3361 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > UINT_MAX) {
				yyerror("only positive values permitted");
				YYERROR;
			}
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			yyval.v.state_opt->type = PF_STATE_OPT_MAX_SRC_CONN;
			yyval.v.state_opt->data.max_src_conn = yystack.l_mark[0].v.number;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8734 "parse.c"
break;
case 341:
#line 3374 "parse.y"
	{
			if (yystack.l_mark[-2].v.number < 0 || yystack.l_mark[-2].v.number > UINT_MAX ||
			    yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > UINT_MAX) {
				yyerror("only positive values permitted");
				YYERROR;
			}
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			yyval.v.state_opt->type = PF_STATE_OPT_MAX_SRC_CONN_RATE;
			yyval.v.state_opt->data.max_src_conn_rate.limit = yystack.l_mark[-2].v.number;
			yyval.v.state_opt->data.max_src_conn_rate.seconds = yystack.l_mark[0].v.number;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8753 "parse.c"
break;
case 342:
#line 3389 "parse.y"
	{
			if (strlen(yystack.l_mark[-2].v.string) >= PF_TABLE_NAME_SIZE) {
				yyerror("table name '%s' too long", yystack.l_mark[-2].v.string);
				free(yystack.l_mark[-2].v.string);
				YYERROR;
			}
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			if (strlcpy(yyval.v.state_opt->data.overload.tblname, yystack.l_mark[-2].v.string,
			    PF_TABLE_NAME_SIZE) >= PF_TABLE_NAME_SIZE)
				errx(1, "state_opt_item: strlcpy");
			free(yystack.l_mark[-2].v.string);
			yyval.v.state_opt->type = PF_STATE_OPT_OVERLOAD;
			yyval.v.state_opt->data.overload.flush = yystack.l_mark[0].v.i;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8775 "parse.c"
break;
case 343:
#line 3407 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > UINT_MAX) {
				yyerror("only positive values permitted");
				YYERROR;
			}
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			yyval.v.state_opt->type = PF_STATE_OPT_MAX_SRC_NODES;
			yyval.v.state_opt->data.max_src_nodes = yystack.l_mark[0].v.number;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8792 "parse.c"
break;
case 344:
#line 3420 "parse.y"
	{
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			yyval.v.state_opt->type = PF_STATE_OPT_SRCTRACK;
			yyval.v.state_opt->data.src_track = yystack.l_mark[0].v.i;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8805 "parse.c"
break;
case 345:
#line 3429 "parse.y"
	{
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			yyval.v.state_opt->type = PF_STATE_OPT_STATELOCK;
			yyval.v.state_opt->data.statelock = yystack.l_mark[0].v.i;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8818 "parse.c"
break;
case 346:
#line 3438 "parse.y"
	{
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			yyval.v.state_opt->type = PF_STATE_OPT_SLOPPY;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8830 "parse.c"
break;
case 347:
#line 3446 "parse.y"
	{
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			yyval.v.state_opt->type = PF_STATE_OPT_PFLOW;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8842 "parse.c"
break;
case 348:
#line 3454 "parse.y"
	{
			int	i;

			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > UINT_MAX) {
				yyerror("only positive values permitted");
				YYERROR;
			}
			for (i = 0; pf_timeouts[i].name &&
			    strcmp(pf_timeouts[i].name, yystack.l_mark[-1].v.string); ++i)
				;	/* nothing */
			if (!pf_timeouts[i].name) {
				yyerror("illegal timeout name %s", yystack.l_mark[-1].v.string);
				free(yystack.l_mark[-1].v.string);
				YYERROR;
			}
			if (strchr(pf_timeouts[i].name, '.') == NULL) {
				yyerror("illegal state timeout %s", yystack.l_mark[-1].v.string);
				free(yystack.l_mark[-1].v.string);
				YYERROR;
			}
			free(yystack.l_mark[-1].v.string);
			yyval.v.state_opt = calloc(1, sizeof(struct node_state_opt));
			if (yyval.v.state_opt == NULL)
				err(1, "state_opt_item: calloc");
			yyval.v.state_opt->type = PF_STATE_OPT_TIMEOUT;
			yyval.v.state_opt->data.timeout.number = pf_timeouts[i].timeout;
			yyval.v.state_opt->data.timeout.seconds = yystack.l_mark[0].v.number;
			yyval.v.state_opt->next = NULL;
			yyval.v.state_opt->tail = yyval.v.state_opt;
		}
#line 8876 "parse.c"
break;
case 349:
#line 3486 "parse.y"
	{
			yyval.v.string = yystack.l_mark[0].v.string;
		}
#line 8883 "parse.c"
break;
case 350:
#line 3491 "parse.y"
	{
			struct pfctl_qsitem *qsi;

			if ((qsi = pfctl_find_queue(yystack.l_mark[0].v.string, &qspecs)) == NULL) {
				yyerror("queue %s is not defined", yystack.l_mark[0].v.string);
				YYERROR;
			}
			yyval.v.qassign.qname = yystack.l_mark[0].v.string;
			yyval.v.qassign.pqname = NULL;
		}
#line 8897 "parse.c"
break;
case 351:
#line 3501 "parse.y"
	{
			struct pfctl_qsitem *qsi;

			if ((qsi = pfctl_find_queue(yystack.l_mark[-1].v.string, &qspecs)) == NULL) {
				yyerror("queue %s is not defined", yystack.l_mark[-1].v.string);
				YYERROR;
			}
			yyval.v.qassign.qname = yystack.l_mark[-1].v.string;
			yyval.v.qassign.pqname = NULL;
		}
#line 8911 "parse.c"
break;
case 352:
#line 3511 "parse.y"
	{
			struct pfctl_qsitem *qsi, *pqsi;

			if ((qsi = pfctl_find_queue(yystack.l_mark[-3].v.string, &qspecs)) == NULL) {
				yyerror("queue %s is not defined", yystack.l_mark[-3].v.string);
				YYERROR;
			}
			if ((pqsi = pfctl_find_queue(yystack.l_mark[-1].v.string, &qspecs)) == NULL) {
				yyerror("queue %s is not defined", yystack.l_mark[-1].v.string);
				YYERROR;
			}
			yyval.v.qassign.qname = yystack.l_mark[-3].v.string;
			yyval.v.qassign.pqname = yystack.l_mark[-1].v.string;
		}
#line 8929 "parse.c"
break;
case 353:
#line 3527 "parse.y"
	{
			if (parseport(yystack.l_mark[0].v.string, &yyval.v.range, PPORT_RANGE|PPORT_STAR) == -1) {
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 8940 "parse.c"
break;
case 354:
#line 3536 "parse.y"
	{
			if (yystack.l_mark[0].v.weight > 0) {
				struct node_host	*n;
				for (n = yystack.l_mark[-1].v.host; n != NULL; n = n->next)
					n->weight = yystack.l_mark[0].v.weight;
			}
			yyval.v.host = yystack.l_mark[-1].v.host;
		}
#line 8952 "parse.c"
break;
case 355:
#line 3544 "parse.y"
	{ yyval.v.host = yystack.l_mark[-1].v.host; }
#line 8957 "parse.c"
break;
case 356:
#line 3547 "parse.y"
	{
			if (yystack.l_mark[-2].v.host->addr.type != PF_ADDR_ADDRMASK) {
				free(yystack.l_mark[-2].v.host);
				yyerror("only addresses can be listed for "
				    "redirection pools ");
				YYERROR;
			}
			if (yystack.l_mark[-1].v.weight > 0) {
				struct node_host	*n;
				for (n = yystack.l_mark[-2].v.host; n != NULL; n = n->next)
					n->weight = yystack.l_mark[-1].v.weight;
			}
			yyval.v.host = yystack.l_mark[-2].v.host;
		}
#line 8975 "parse.c"
break;
case 357:
#line 3561 "parse.y"
	{
			yystack.l_mark[-4].v.host->tail->next = yystack.l_mark[-2].v.host;
			yystack.l_mark[-4].v.host->tail = yystack.l_mark[-2].v.host->tail;
			if (yystack.l_mark[-1].v.weight > 0) {
				struct node_host	*n;
				for (n = yystack.l_mark[-2].v.host; n != NULL; n = n->next)
					n->weight = yystack.l_mark[-1].v.weight;
			}
			yyval.v.host = yystack.l_mark[-4].v.host;
		}
#line 8989 "parse.c"
break;
case 358:
#line 3573 "parse.y"
	{
			yyval.v.redirection = calloc(1, sizeof(struct redirection));
			if (yyval.v.redirection == NULL)
				err(1, "redirection: calloc");
			yyval.v.redirection->host = yystack.l_mark[0].v.host;
			yyval.v.redirection->rport.a = yyval.v.redirection->rport.b = yyval.v.redirection->rport.t = 0;
		}
#line 9000 "parse.c"
break;
case 359:
#line 3580 "parse.y"
	{
			yyval.v.redirection = calloc(1, sizeof(struct redirection));
			if (yyval.v.redirection == NULL)
				err(1, "redirection: calloc");
			yyval.v.redirection->host = yystack.l_mark[-2].v.host;
			yyval.v.redirection->rport = yystack.l_mark[0].v.range;
		}
#line 9011 "parse.c"
break;
case 360:
#line 3590 "parse.y"
	{
			yyval.v.hashkey = calloc(1, sizeof(struct pf_poolhashkey));
			if (yyval.v.hashkey == NULL)
				err(1, "hashkey: calloc");
			yyval.v.hashkey->key32[0] = arc4random();
			yyval.v.hashkey->key32[1] = arc4random();
			yyval.v.hashkey->key32[2] = arc4random();
			yyval.v.hashkey->key32[3] = arc4random();
		}
#line 9024 "parse.c"
break;
case 361:
#line 3600 "parse.y"
	{
			if (!strncmp(yystack.l_mark[0].v.string, "0x", 2)) {
				if (strlen(yystack.l_mark[0].v.string) != 34) {
					free(yystack.l_mark[0].v.string);
					yyerror("hex key must be 128 bits "
						"(32 hex digits) long");
					YYERROR;
				}
				yyval.v.hashkey = calloc(1, sizeof(struct pf_poolhashkey));
				if (yyval.v.hashkey == NULL)
					err(1, "hashkey: calloc");

				if (sscanf(yystack.l_mark[0].v.string, "0x%8x%8x%8x%8x",
				    &yyval.v.hashkey->key32[0], &yyval.v.hashkey->key32[1],
				    &yyval.v.hashkey->key32[2], &yyval.v.hashkey->key32[3]) != 4) {
					free(yyval.v.hashkey);
					free(yystack.l_mark[0].v.string);
					yyerror("invalid hex key");
					YYERROR;
				}
			} else {
				MD5_CTX	context;

				yyval.v.hashkey = calloc(1, sizeof(struct pf_poolhashkey));
				if (yyval.v.hashkey == NULL)
					err(1, "hashkey: calloc");
				MD5Init(&context);
				MD5Update(&context, (unsigned char *)yystack.l_mark[0].v.string,
				    strlen(yystack.l_mark[0].v.string));
				MD5Final((unsigned char *)yyval.v.hashkey, &context);
				HTONL(yyval.v.hashkey->key32[0]);
				HTONL(yyval.v.hashkey->key32[1]);
				HTONL(yyval.v.hashkey->key32[2]);
				HTONL(yyval.v.hashkey->key32[3]);
			}
			free(yystack.l_mark[0].v.string);
		}
#line 9065 "parse.c"
break;
case 362:
#line 3639 "parse.y"
	{ bzero(&pool_opts, sizeof pool_opts); }
#line 9070 "parse.c"
break;
case 363:
#line 3641 "parse.y"
	{ yyval.v.pool_opts = pool_opts; }
#line 9075 "parse.c"
break;
case 364:
#line 3642 "parse.y"
	{
			bzero(&pool_opts, sizeof pool_opts);
			yyval.v.pool_opts = pool_opts;
		}
#line 9083 "parse.c"
break;
case 367:
#line 3652 "parse.y"
	{
			if (pool_opts.type) {
				yyerror("pool type cannot be redefined");
				YYERROR;
			}
			pool_opts.type =  PF_POOL_BITMASK;
		}
#line 9094 "parse.c"
break;
case 368:
#line 3659 "parse.y"
	{
			if (pool_opts.type) {
				yyerror("pool type cannot be redefined");
				YYERROR;
			}
			pool_opts.type = PF_POOL_RANDOM;
		}
#line 9105 "parse.c"
break;
case 369:
#line 3666 "parse.y"
	{
			if (pool_opts.type) {
				yyerror("pool type cannot be redefined");
				YYERROR;
			}
			pool_opts.type = PF_POOL_SRCHASH;
			pool_opts.key = yystack.l_mark[0].v.hashkey;
		}
#line 9117 "parse.c"
break;
case 370:
#line 3674 "parse.y"
	{
			if (pool_opts.type) {
				yyerror("pool type cannot be redefined");
				YYERROR;
			}
			pool_opts.type = PF_POOL_ROUNDROBIN;
		}
#line 9128 "parse.c"
break;
case 371:
#line 3681 "parse.y"
	{
			if (pool_opts.type) {
				yyerror("pool type cannot be redefined");
				YYERROR;
			}
			pool_opts.type = PF_POOL_LEASTSTATES;
		}
#line 9139 "parse.c"
break;
case 372:
#line 3688 "parse.y"
	{
			if (pool_opts.staticport) {
				yyerror("static-port cannot be redefined");
				YYERROR;
			}
			pool_opts.staticport = 1;
		}
#line 9150 "parse.c"
break;
case 373:
#line 3695 "parse.y"
	{
			if (pool_opts.marker & POM_STICKYADDRESS) {
				yyerror("sticky-address cannot be redefined");
				YYERROR;
			}
			pool_opts.marker |= POM_STICKYADDRESS;
			pool_opts.opts |= PF_POOL_STICKYADDR;
		}
#line 9162 "parse.c"
break;
case 374:
#line 3705 "parse.y"
	{
			struct redirection *redir;
			if (filter_opts.rt != PF_NOPFROUTE) {
				yyerror("cannot respecify "
				    "route-to/reply-to/dup-to");
				YYERROR;
			}
			redir = calloc(1, sizeof(*redir));
			if (redir == NULL)
				err(1, "routespec calloc");
			redir->host = yystack.l_mark[-1].v.host;
			filter_opts.rroute.rdr = redir;
			memcpy(&filter_opts.rroute.pool_opts, &yystack.l_mark[0].v.pool_opts,
			    sizeof(filter_opts.rroute.pool_opts));
		}
#line 9181 "parse.c"
break;
case 375:
#line 3723 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > UINT_MAX) {
				yyerror("only positive values permitted");
				YYERROR;
			}
			if (pfctl_set_timeout(pf, yystack.l_mark[-1].v.string, yystack.l_mark[0].v.number, 0) != 0) {
				yyerror("unknown timeout %s", yystack.l_mark[-1].v.string);
				free(yystack.l_mark[-1].v.string);
				YYERROR;
			}
			free(yystack.l_mark[-1].v.string);
		}
#line 9197 "parse.c"
break;
case 378:
#line 3742 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > UINT_MAX) {
				yyerror("only positive values permitted");
				YYERROR;
			}
			if (pfctl_set_limit(pf, yystack.l_mark[-1].v.string, yystack.l_mark[0].v.number) != 0) {
				yyerror("unable to set limit %s %lld", yystack.l_mark[-1].v.string, yystack.l_mark[0].v.number);
				free(yystack.l_mark[-1].v.string);
				YYERROR;
			}
			free(yystack.l_mark[-1].v.string);
		}
#line 9213 "parse.c"
break;
case 383:
#line 3764 "parse.y"
	{ yyval.v.number = 0; }
#line 9218 "parse.c"
break;
case 384:
#line 3765 "parse.y"
	{
			if (!strcmp(yystack.l_mark[0].v.string, "yes"))
				yyval.v.number = 1;
			else {
				yyerror("invalid value '%s', expected 'yes' "
				    "or 'no'", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 9233 "parse.c"
break;
case 385:
#line 3778 "parse.y"
	{ yyval.v.i = PF_OP_EQ; }
#line 9238 "parse.c"
break;
case 386:
#line 3779 "parse.y"
	{ yyval.v.i = PF_OP_NE; }
#line 9243 "parse.c"
break;
case 387:
#line 3780 "parse.y"
	{ yyval.v.i = PF_OP_LE; }
#line 9248 "parse.c"
break;
case 388:
#line 3781 "parse.y"
	{ yyval.v.i = PF_OP_LT; }
#line 9253 "parse.c"
break;
case 389:
#line 3782 "parse.y"
	{ yyval.v.i = PF_OP_GE; }
#line 9258 "parse.c"
break;
case 390:
#line 3783 "parse.y"
	{ yyval.v.i = PF_OP_GT; }
#line 9263 "parse.c"
break;
#line 9265 "parse.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            yychar = YYLEX;
            if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if (((yyn = yygindex[yym]) != 0) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    YYERROR_CALL("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
