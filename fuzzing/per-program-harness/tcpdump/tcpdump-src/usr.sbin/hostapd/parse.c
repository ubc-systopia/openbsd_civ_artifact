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

#line 24 "parse.y"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/stat.h>

#include <net/if.h>
#include <net/if_media.h>
#include <net/if_arp.h>
#include <net/if_llc.h>
#include <net/bpf.h>

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>

#include <net80211/ieee80211.h>
#include <net80211/ieee80211_radiotap.h>

#include <ctype.h>
#include <errno.h>
#include <event.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <err.h>

#include "hostapd.h"

TAILQ_HEAD(files, file)		 files = TAILQ_HEAD_INITIALIZER(files);
static struct file {
	TAILQ_ENTRY(file)	 entry;
	FILE			*stream;
	char			*name;
	size_t	 		 ungetpos;
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

extern struct hostapd_config hostapd_cfg;

typedef struct {
	union {
		struct {
			u_int8_t		lladdr[IEEE80211_ADDR_LEN];
			struct hostapd_table	*table;
			u_int32_t		flags;
		} reflladdr;
		struct {
			u_int16_t		alg;
			u_int16_t		transaction;
		} authalg;
		struct in_addr		in;
		char			*string;
		int64_t			number;
		u_int16_t		reason;
		enum hostapd_op		op;
		struct timeval		timeout;
	} v;
	int lineno;
} YYSTYPE;

struct hostapd_apme *apme;
struct hostapd_table *table;
struct hostapd_entry *entry;
struct hostapd_frame frame, *frame_ptr;
struct hostapd_ieee80211_frame *frame_ieee80211;

#define HOSTAPD_MATCH(_m, _not)	{					\
	frame.f_flags |= (_not) ?					\
	    HOSTAPD_FRAME_F_##_m##_N : HOSTAPD_FRAME_F_##_m;		\
}
#define HOSTAPD_MATCH_TABLE(_m, _not)	{				\
	frame.f_flags |= HOSTAPD_FRAME_F_##_m##_TABLE | ((_not) ?	\
	    HOSTAPD_FRAME_F_##_m##_N : HOSTAPD_FRAME_F_##_m);		\
}
#define HOSTAPD_MATCH_RADIOTAP(_x) {					\
	if (hostapd_cfg.c_apme_dlt == DLT_IEEE802_11 ||			\
	    (hostapd_cfg.c_apme_dlt == 0 &&				\
	    HOSTAPD_DLT == DLT_IEEE802_11)) {				\
		yyerror("option %s requires radiotap headers", #_x);	\
		YYERROR;						\
	}								\
	frame.f_radiotap |= HOSTAPD_RADIOTAP_F(RSSI);			\
	frame.f_flags |= HOSTAPD_FRAME_F_##_x;				\
}
#define HOSTAPD_IAPP_FLAG(_f, _not) {					\
	if (_not)							\
		hostapd_cfg.c_iapp.i_flags &= ~(HOSTAPD_IAPP_F_##_f);	\
	else								\
		hostapd_cfg.c_iapp.i_flags |= (HOSTAPD_IAPP_F_##_f);	\
}

#line 149 "parse.c"

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

#define MODE 257
#define INTERFACE 258
#define IAPP 259
#define HOSTAP 260
#define MULTICAST 261
#define BROADCAST 262
#define SET 263
#define SEC 264
#define USEC 265
#define HANDLE 266
#define TYPE 267
#define SUBTYPE 268
#define FROM 269
#define TO 270
#define BSSID 271
#define WITH 272
#define FRAME 273
#define RADIOTAP 274
#define NWID 275
#define PASSIVE 276
#define MANAGEMENT 277
#define DATA 278
#define PROBE 279
#define BEACON 280
#define ATIM 281
#define ANY 282
#define DS 283
#define NO 284
#define DIR 285
#define RESEND 286
#define RANDOM 287
#define AUTH 288
#define DEAUTH 289
#define ASSOC 290
#define DISASSOC 291
#define REASSOC 292
#define REQUEST 293
#define RESPONSE 294
#define PCAP 295
#define RATE 296
#define ERROR 297
#define CONST 298
#define TABLE 299
#define NODE 300
#define DELETE 301
#define ADD 302
#define LOG 303
#define VERBOSE 304
#define LIMIT 305
#define QUICK 306
#define SKIP 307
#define REASON 308
#define UNSPECIFIED 309
#define EXPIRE 310
#define LEAVE 311
#define TOOMANY 312
#define NOT 313
#define AUTHED 314
#define ASSOCED 315
#define RESERVED 316
#define RSN 317
#define REQUIRED 318
#define INCONSISTENT 319
#define IE 320
#define INVALID 321
#define MIC 322
#define FAILURE 323
#define OPEN 324
#define ADDRESS 325
#define PORT 326
#define ON 327
#define NOTIFY 328
#define TTL 329
#define INCLUDE 330
#define ROUTE 331
#define ROAMING 332
#define RSSI 333
#define TXRATE 334
#define FREQ 335
#define HOPPER 336
#define DELAY 337
#define NE 338
#define LE 339
#define GE 340
#define ARROW 341
#define STRING 342
#define NUMBER 343
#define YYERRCODE 256
typedef int YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,    0,    0,    0,    0,    0,    0,   17,   19,
   19,   19,   19,   19,   19,   19,   19,   19,   26,   26,
   28,   28,   29,   29,   30,   30,   24,   24,   22,   22,
   32,   32,   33,   23,   23,   35,   35,   36,   37,   37,
   39,   42,   20,   27,   27,   45,   45,   46,   46,   46,
   46,   38,   38,   38,   41,   41,   41,   41,   41,   41,
   47,   47,   49,   50,   50,   40,   40,   51,   51,   48,
   43,   43,   43,   44,   44,   52,   52,   52,   52,   63,
   63,   63,   64,   64,   64,   64,   64,   64,   64,   64,
   64,   64,   64,   65,   65,   68,   68,   69,   66,   66,
   10,   10,   67,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,   53,   53,
   53,   70,   70,   70,   70,   54,   54,   54,   55,   55,
   55,   56,   56,   56,   57,   57,   71,   71,   72,   72,
   72,    6,    6,   58,   58,   73,   59,   59,   60,   61,
   62,    5,    5,    5,    8,   75,   18,   74,   74,   76,
   76,   77,   77,   77,    9,    9,   21,    2,    2,    2,
   78,   78,   81,   79,   80,   80,   80,    1,   83,   83,
    3,    4,   25,   25,   82,   84,   34,   34,   31,   31,
   15,   15,   15,   11,   11,   11,   11,   11,   11,   11,
   11,   11,   12,   13,   14,   16,
};
static const YYINT yylen[] = {                            2,
    0,    2,    3,    3,    3,    3,    3,    3,    2,    4,
    5,    5,    4,    5,    4,    6,    6,    5,    4,    2,
    0,    2,    0,    2,    0,    2,    1,    1,    5,    1,
    1,    3,    1,    5,    1,    1,    3,    1,    0,    3,
    0,    0,   10,    5,    1,    1,    3,    3,    2,    3,
    3,    0,    1,    1,    0,    3,    3,    3,    4,    2,
    0,    1,    3,    1,    1,    1,    7,    0,    1,    5,
    0,    3,    3,    0,    5,    0,    2,    3,    4,    0,
    2,    3,    3,    3,    2,    1,    2,    2,    2,    2,
    2,    2,    2,    0,    1,    2,    1,    3,    0,    1,
    2,    2,    1,    0,    2,    3,    3,    3,    3,    3,
    3,    3,    4,    2,    3,    3,    3,    3,    0,    2,
    3,    2,    2,    2,    3,    0,    2,    3,    0,    2,
    3,    0,    2,    3,    0,    1,    2,    1,    3,    3,
    3,    1,    1,    2,    3,    2,    0,    2,    2,    2,
    2,    1,    1,    1,    3,    0,    4,    0,    1,    2,
    1,    1,    3,    5,    2,    1,    3,    2,    2,    2,
    1,    3,    0,    3,    0,    3,    2,    1,    0,    2,
    1,    1,    0,    1,    1,    1,    0,    2,    0,    1,
    0,    1,    1,    0,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,
};
static const YYINT yydefred[] = {                         1,
    0,    0,    0,    0,    0,    0,    0,    2,    0,    0,
    0,    0,    0,    8,   41,    0,    0,    0,  156,    9,
    0,    3,    4,    5,    6,    7,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  166,    0,   53,
   54,    0,    0,    0,   15,    0,    0,    0,    0,   27,
   28,   13,   33,    0,   10,   30,    0,    0,  155,  162,
    0,  157,    0,  161,  165,    0,    0,    0,    0,    0,
   20,  184,   14,  193,    0,  192,    0,   18,   45,    0,
    0,  190,    0,   38,    0,   11,   35,  206,   12,    0,
  160,    0,   69,   66,   42,    0,  178,   22,    0,   24,
    0,   49,    0,    0,    0,   16,   17,    0,   31,    0,
  181,  163,  173,    0,  171,   40,    0,    0,    0,    0,
   19,    0,   46,   48,   51,   50,    0,    0,    0,    0,
   36,    0,    0,    0,    0,    0,   77,    0,    0,    0,
   26,    0,    0,  188,   29,   32,    0,    0,  185,  186,
  174,    0,    0,  164,  172,    0,    0,   60,    0,    0,
    0,    0,    0,   78,  120,    0,    0,    0,   44,   47,
   34,   37,    0,  177,    0,   58,    0,   57,    0,   64,
   65,    0,   62,   56,    0,    0,   43,    0,   79,    0,
    0,    0,    0,  121,  127,    0,    0,    0,    0,  176,
    0,    0,  144,    0,    0,  182,    0,  154,  152,  153,
   59,   72,   73,    0,   81,    0,  124,  123,    0,  122,
  143,  128,  142,  130,    0,    0,    0,  180,   63,    0,
  145,  148,    0,    0,  168,  169,  170,    0,    0,    0,
   86,    0,    0,    0,    0,    0,   82,  125,  131,  133,
    0,    0,    0,    0,   67,    0,  138,  146,  149,    0,
    0,    0,    0,    0,    0,   85,    0,   97,    0,  100,
   87,    0,  103,   88,   89,   91,   90,   92,   93,  134,
  198,  199,  201,  200,  202,  195,  197,  196,    0,    0,
    0,  137,  150,    0,   70,   75,   83,   84,    0,   96,
  101,  102,    0,    0,  105,    0,  114,    0,    0,    0,
  203,  139,  204,  140,  205,  141,  151,   98,  106,  107,
  108,  112,  109,    0,  110,  111,  115,  116,  117,  118,
  113,
};
static const YYINT yydgoto[] = {                          1,
   98,  208,  209,  210,  211,  222,  273,  223,   39,  270,
  289,  312,  314,  316,   77,   89,    9,   10,   11,   12,
   13,   55,   86,   52,   73,   45,   78,   69,   71,  121,
   83,  108,   56,  129,  130,   87,   67,   42,   27,   95,
  136,  117,  162,  187,  122,   79,  184,  178,  176,  182,
   96,  119,  140,  168,  198,  227,  255,  179,  205,  234,
  261,  295,  189,  247,  266,  271,  274,  267,  268,  194,
  256,  257,  231,   62,   37,   63,   64,  114,  115,  151,
  132,  152,  200,  153,
};
static const YYINT yysindex[] = {                         0,
  -10,   37, -217, -208,    5, -288,    1,    0,   60,   88,
   95,  100,  108,    0,    0, -229, -236, -241,    0,    0,
 -214,    0,    0,    0,    0,    0, -218, -141, -207, -131,
 -185, -170, -226, -115, -231,  109, -103,    0, -169,    0,
    0, -140, -136, -122,    0,  -57,  -19,  -97,  -78,    0,
    0,    0,    0,  210,    0,    0, -112, -121,    0,    0,
  210,    0, -103,    0,    0,  -18, -188, -116, -122, -114,
    0,    0,    0,    0,  210,    0, -249,    0,    0,    5,
    5,    0, -111,    0,  210,    0,    0,    0,    0, -108,
    0, -110,    0,    0,    0,  -44,    0,    0, -101,    0,
  -18,    0, -104,  -99,  -95,    0,    0,   30,    0, -102,
    0,    0,    0,   30,    0,    0,  -34,  -20,  -46, -100,
    0,   30,    0,    0,    0,    0,  210,  116, -111,   30,
    0,    3,  117,  -94, -213,  -61,    0, -146,   78,  -24,
    0,  122,  -18,    0,    0,    0,  126, -102,    0,    0,
    0, -116,  -94,    0,    0,  -15,  -13,    0, -160,  -55,
  -88,  -39,  -12,    0,    0, -144,   89,  -11,    0,    0,
    0,    0,  213,    0,   -4,    0, -133,    0,  -17,    0,
    0,  -29,    0,    0, -105,  -66,    0,   90,    0,   -2,
   -1,   10,   13,    0,    0,  -54,   91,   12,  -58,    0,
  -55,   18,    0, -144,   43,    0, -178,    0,    0,    0,
    0,    0,    0,  250,    0,  -98,    0,    0,   63,    0,
    0,    0,    0,    0,  -54,  132, -221,    0,    0,  -98,
    0,    0,  -29,   77,    0,    0,    0,    9, -126,   73,
    0,   29,   47, -124,   47, -119,    0,    0,    0,    0,
  -54,  -30,  -30,  -30,    0, -221,    0,    0,    0,  -29,
   85,   97,   73,   73,  -18,    0,   73,    0, -117,    0,
    0, -245,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   15,   24,
   40,    0,    0,  -29,    0,    0,    0,    0,   42,    0,
    0,    0, -132, -158,    0, -118,    0, -109,   64,   74,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   56,    0,    0,    0,    0,    0,    0,
    0,
};
static const YYINT yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   -6,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  409,    0,  412,    0,
    0,    2,   14,   88,    0,  415, -195,    0,    0,    0,
    0,    0,    0,   92,    0,    0,    0,    0,    0,    0,
 -107,    0,  420,    0,    0,   94,   34,    0,   -3,    0,
    0,    0,    0,    0,  180,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   92,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   46,    0,    0,  100,    0,
 -195,    0,    0,    0,    0,    0,    0, -106,    0,    0,
    0,    0,    0, -106,    0,    0,   45,  -79,  146,    0,
    0,  175,    0,    0,    0,    0,  170,    0,    0, -106,
    0,   -9,    0,    0,    0,   -5,    0,    0, -120,  141,
    0,    0, -195,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   49,
    0,  421,   53,    0,    0,    0,  -50,  128,    0,    0,
    0,    0,   -8,    0,    0,    0,    0,    0,  166,    0,
    0,    0,    0,    0,    0,    0,    0,  -74,    0,    0,
    0,    0,    0,    0,    0,    0,  -50,  151,    0,    0,
   49,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  -50,    6,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   93,
    0,  105,  124,    0,  124,    0,    0,    0,    0,    0,
    0,  101,  101,  101,    0,   87,    0,    0,    0,    0,
    0,    0,   93,   93,   94,    0,  136,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,
};
static const YYINT yygindex[] = {                         0,
  296,    0,  -67,    0, -199, -168,    0,   28,    0,    0,
  -53,    0,    0,    0,  -40,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  381,    0,
   58,    0,  -45,  -64,    0,  -68,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  -62,  251,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  223,  -52,    0,  215,    0,  188,  260,
    0,  209,    0,    0,    0,    0,  403,    0,  333,    0,
    0,    0,    0,    0,
};
#define YYTABLESIZE 512
static const YYINT yytable[] = {                          8,
  175,  179,  287,   52,   71,   18,   23,   54,  207,  191,
   85,   39,   76,   76,   76,  135,  112,  189,  189,   61,
   33,   34,  113,   21,  102,   92,   57,   28,   29,  284,
  286,  285,   19,  259,  175,  179,   30,  109,  123,   82,
  150,  131,  303,   68,  304,  156,   14,   50,   15,  134,
   16,   17,  103,   20,   55,   76,  249,  143,   61,  157,
  293,   21,   80,  305,   18,  148,  113,  306,   51,   22,
  307,  308,  158,  127,  309,  104,  310,  138,  191,  172,
  170,  105,  280,  146,   93,  174,  159,   40,   41,  160,
  235,  236,  237,   94,  317,   31,  136,   23,  166,   35,
   36,   32,   94,   75,   24,   58,  191,  106,  107,   25,
   76,  252,  253,  254,   99,  175,  179,   26,   90,   43,
   44,   76,   76,   76,  190,  191,  196,   38,  221,  191,
  163,  164,  101,  104,   46,  191,   47,  129,  192,  193,
  180,  181,  110,  202,  203,   95,   48,  216,  191,  191,
  126,  321,  322,  323,  324,  119,  225,  221,  212,  213,
  132,   49,  191,  191,   76,  128,  263,  264,  275,  276,
   59,  133,   65,  278,  279,  301,  302,  319,  320,  142,
  239,  240,  241,  221,  144,  251,   66,  147,   68,  242,
  243,  244,  245,  246,   60,  325,  326,  191,  191,  290,
  291,   80,  189,   70,  191,  191,  191,  187,  327,  328,
  297,  298,  189,  191,  191,  191,  191,  191,   72,   82,
   81,   88,  118,  124,  299,   97,   53,  120,  100,   84,
   53,  116,  125,  111,  189,  187,  126,  135,  139,   84,
  145,  154,  141,  161,  167,    2,  169,  111,  183,    3,
  171,  175,    4,  177,  185,  188,  186,  206,  197,  199,
   52,  137,   52,   52,   52,   52,   52,  204,   39,  201,
   39,   39,   39,   39,   39,   52,  214,  135,   52,  219,
  217,  218,  226,   39,  228,  230,   39,  111,    5,   52,
   71,  191,   74,   74,   74,  220,  238,   39,   52,  189,
   68,  135,   68,   68,   68,   68,   39,  281,  282,  283,
  135,  233,  111,  288,   76,   76,   76,   76,   68,    6,
   52,   80,   80,   80,   80,   23,   52,   52,   52,   68,
   76,    7,  175,  179,   39,   39,   39,   80,   68,   21,
   55,   76,   21,  149,   61,  248,  260,  265,   80,   55,
   76,  262,  269,   61,  272,  294,  311,   80,  136,  165,
  296,   94,   94,   94,   94,  313,   68,   68,   68,  331,
  195,  215,  224,   99,   99,   99,   99,   94,   76,   76,
   76,  315,  136,  318,  329,   80,   80,   80,   94,   99,
   74,  136,  104,  104,  104,  104,  330,   94,  129,  129,
   99,   74,   74,   74,   95,   95,   95,   95,  104,   99,
  126,  126,  126,  250,  119,  119,  119,  119,  158,  104,
   95,  167,  132,  129,  183,   94,   94,   94,  104,  159,
   74,   95,  129,  189,  147,  191,  126,   99,   99,   99,
   95,  119,  194,  189,   74,  126,  132,  173,  187,   99,
  119,  229,  258,  189,  300,  132,  104,  104,  104,  277,
  129,  129,  129,  232,  292,   91,  155,    0,   95,   95,
   95,  189,    0,  126,  126,  126,  187,    0,  119,  119,
  119,  189,  189,  132,  132,  132,    0,  187,    0,    0,
    0,    0,  189,    0,  189,    0,    0,    0,    0,  187,
  189,    0,    0,    0,  189,  187,    0,    0,    0,    0,
  189,  189,
};
static const YYINT yycheck[] = {                         10,
   10,   10,   33,   10,   10,   60,   10,  123,   38,   60,
  123,   10,   33,   33,   33,   10,  125,  125,  125,  123,
  257,  258,   90,   10,  274,   66,  258,  257,  258,   60,
   61,   62,    5,  233,   44,   44,  266,   83,  101,   10,
   38,  110,  288,   10,  290,  259,   10,  274,  266,  114,
  259,  260,  302,  342,   10,   10,  225,  122,   10,  273,
  260,   61,   10,  309,   60,  130,  134,  313,  295,   10,
  316,  317,  286,   44,  320,  325,  322,  118,  274,  148,
  143,  331,  251,  129,  273,  153,  300,  306,  307,  303,
  269,  270,  271,  282,  294,  325,   10,   10,  139,  336,
  342,  331,   10,  123,   10,  337,  302,   80,   81,   10,
   33,  333,  334,  335,   10,  125,  125,   10,   61,  261,
  262,   33,   33,   33,  269,  270,  167,  342,  196,  325,
  277,  278,   75,   10,  342,  331,  268,   10,  283,  284,
  301,  302,   85,  277,  278,   10,  332,  188,  269,  270,
   10,  310,  311,  312,  313,   10,  197,  225,  264,  265,
   10,  332,  283,  284,   33,  108,  293,  294,  293,  294,
   62,  114,  342,  293,  294,  293,  294,  310,  311,  122,
  279,  280,  281,  251,  127,  226,  327,  130,  325,  288,
  289,  290,  291,  292,  298,  314,  315,  277,  278,  253,
  254,  299,   33,  326,  279,  280,  281,   33,  318,  319,
  263,  264,   33,  288,  289,  290,  291,  292,  276,   10,
  299,  343,  267,  328,  265,  342,  342,  329,  343,  342,
  342,  342,  332,  342,  342,  342,  332,  272,  285,  342,
  125,  125,  343,  305,  269,  256,  125,  342,  304,  260,
  125,  267,  263,  267,  343,  268,  296,  287,  270,   47,
  267,  282,  269,  270,  271,  272,  273,  285,  267,  274,
  269,  270,  271,  272,  273,  282,  343,  272,  285,  270,
  283,  283,  271,  282,  343,  268,  285,  342,  299,  296,
  296,  342,  313,  313,  313,  283,   47,  296,  305,  125,
  267,  296,  269,  270,  271,  272,  305,  338,  339,  340,
  305,  269,  342,  344,  269,  270,  271,  272,  285,  330,
  327,  269,  270,  271,  272,  329,  333,  334,  335,  296,
  285,  342,  342,  342,  333,  334,  335,  285,  305,  326,
  296,  296,  329,  341,  296,  283,  270,  275,  296,  305,
  305,  343,  324,  305,  308,  271,  342,  305,  272,  282,
  264,  269,  270,  271,  272,  342,  333,  334,  335,  314,
  282,  282,  282,  269,  270,  271,  272,  285,  333,  334,
  335,  342,  296,  342,  321,  333,  334,  335,  296,  285,
  313,  305,  269,  270,  271,  272,  323,  305,  271,  272,
  296,  313,  313,  313,  269,  270,  271,  272,  285,  305,
  270,  271,  272,  282,  269,  270,  271,  272,   10,  296,
  285,   10,  272,  296,   10,  333,  334,  335,  305,   10,
   10,  296,  305,  342,  269,  342,  296,  333,  334,  335,
  305,  296,  342,  274,  313,  305,  296,  152,  274,   69,
  305,  201,  230,  274,  267,  305,  333,  334,  335,  245,
  333,  334,  335,  204,  256,   63,  134,   -1,  333,  334,
  335,  302,   -1,  333,  334,  335,  302,   -1,  333,  334,
  335,  302,  313,  333,  334,  335,   -1,  313,   -1,   -1,
   -1,   -1,  313,   -1,  325,   -1,   -1,   -1,   -1,  325,
  331,   -1,   -1,   -1,  325,  331,   -1,   -1,   -1,   -1,
  331,  342,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 344
#define YYUNDFTOKEN 431
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,"'\\n'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,"'!'",0,0,0,0,"'&'",0,0,0,0,0,"','",0,0,"'/'",0,0,0,0,0,0,0,0,0,0,0,0,"'<'",
"'='","'>'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,"MODE","INTERFACE","IAPP","HOSTAP","MULTICAST","BROADCAST","SET","SEC",
"USEC","HANDLE","TYPE","SUBTYPE","FROM","TO","BSSID","WITH","FRAME","RADIOTAP",
"NWID","PASSIVE","MANAGEMENT","DATA","PROBE","BEACON","ATIM","ANY","DS","NO",
"DIR","RESEND","RANDOM","AUTH","DEAUTH","ASSOC","DISASSOC","REASSOC","REQUEST",
"RESPONSE","PCAP","RATE","ERROR","CONST","TABLE","NODE","DELETE","ADD","LOG",
"VERBOSE","LIMIT","QUICK","SKIP","REASON","UNSPECIFIED","EXPIRE","LEAVE",
"TOOMANY","NOT","AUTHED","ASSOCED","RESERVED","RSN","REQUIRED","INCONSISTENT",
"IE","INVALID","MIC","FAILURE","OPEN","ADDRESS","PORT","ON","NOTIFY","TTL",
"INCLUDE","ROUTE","ROAMING","RSSI","TXRATE","FREQ","HOPPER","DELAY","NE","LE",
"GE","ARROW","STRING","NUMBER","\"==\"",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"illegal-symbol",
};
static const char *const yyrule[] = {
"$accept : grammar",
"grammar :",
"grammar : grammar '\\n'",
"grammar : grammar include '\\n'",
"grammar : grammar tabledef '\\n'",
"grammar : grammar option '\\n'",
"grammar : grammar event '\\n'",
"grammar : grammar varset '\\n'",
"grammar : grammar error '\\n'",
"include : INCLUDE STRING",
"option : SET HOSTAP INTERFACE hostapifaces",
"option : SET HOSTAP HOPPER INTERFACE hopperifaces",
"option : SET HOSTAP HOPPER DELAY timeout",
"option : SET HOSTAP MODE hostapmode",
"option : SET IAPP INTERFACE STRING passive",
"option : SET IAPP MODE iappmode",
"option : SET IAPP ADDRESS ROAMING TABLE table",
"option : SET IAPP ROUTE ROAMING TABLE table",
"option : SET IAPP HANDLE SUBTYPE iappsubtypes",
"iappmode : MULTICAST iappmodeaddr iappmodeport iappmodettl",
"iappmode : BROADCAST iappmodeport",
"iappmodeaddr :",
"iappmodeaddr : ADDRESS ipv4addr",
"iappmodeport :",
"iappmodeport : PORT NUMBER",
"iappmodettl :",
"iappmodettl : TTL NUMBER",
"hostapmode : RADIOTAP",
"hostapmode : PCAP",
"hostapifaces : '{' optnl hostapifacelist optnl '}'",
"hostapifaces : hostapiface",
"hostapifacelist : hostapiface",
"hostapifacelist : hostapifacelist comma hostapiface",
"hostapiface : STRING",
"hopperifaces : '{' optnl hopperifacelist optnl '}'",
"hopperifaces : hopperiface",
"hopperifacelist : hopperiface",
"hopperifacelist : hopperifacelist comma hopperiface",
"hopperiface : STRING",
"hostapmatch :",
"hostapmatch : ON not STRING",
"$$1 :",
"$$2 :",
"event : HOSTAP HANDLE $$1 eventopt hostapmatch frmmatch $$2 action limit rate",
"iappsubtypes : '{' optnl iappsubtypelist optnl '}'",
"iappsubtypes : iappsubtype",
"iappsubtypelist : iappsubtype",
"iappsubtypelist : iappsubtypelist comma iappsubtype",
"iappsubtype : not ADD NOTIFY",
"iappsubtype : not RADIOTAP",
"iappsubtype : not ROUTE ROAMING",
"iappsubtype : not ADDRESS ROAMING",
"eventopt :",
"eventopt : QUICK",
"eventopt : SKIP",
"action :",
"action : WITH LOG verbose",
"action : WITH FRAME frmaction",
"action : WITH IAPP iapp",
"action : WITH NODE nodeopt frmactionaddr",
"action : WITH RESEND",
"verbose :",
"verbose : VERBOSE",
"iapp : TYPE RADIOTAP verbose",
"nodeopt : DELETE",
"nodeopt : ADD",
"frmmatch : ANY",
"frmmatch : frm frmmatchtype frmmatchdir frmmatchfrom frmmatchto frmmatchbssid frmmatchrtap",
"frm :",
"frm : FRAME",
"frmaction : frmactiontype frmactiondir frmactionfrom frmactionto frmactionbssid",
"limit :",
"limit : LIMIT NUMBER SEC",
"limit : LIMIT NUMBER USEC",
"rate :",
"rate : RATE NUMBER '/' NUMBER SEC",
"frmmatchtype :",
"frmmatchtype : TYPE ANY",
"frmmatchtype : TYPE not DATA",
"frmmatchtype : TYPE not MANAGEMENT frmmatchmgmt",
"frmmatchmgmt :",
"frmmatchmgmt : SUBTYPE ANY",
"frmmatchmgmt : SUBTYPE not frmsubtype",
"frmsubtype : PROBE REQUEST frmelems",
"frmsubtype : PROBE RESPONSE frmelems",
"frmsubtype : BEACON frmelems",
"frmsubtype : ATIM",
"frmsubtype : AUTH frmauth",
"frmsubtype : DEAUTH frmreason",
"frmsubtype : ASSOC REQUEST",
"frmsubtype : DISASSOC frmreason",
"frmsubtype : ASSOC RESPONSE",
"frmsubtype : REASSOC REQUEST",
"frmsubtype : REASSOC RESPONSE",
"frmelems :",
"frmelems : frmelems_l",
"frmelems_l : frmelems_l frmelem",
"frmelems_l : frmelem",
"frmelem : NWID not STRING",
"frmauth :",
"frmauth : authalg",
"authalg : OPEN REQUEST",
"authalg : OPEN RESPONSE",
"frmreason : frmreason_l",
"frmreason_l :",
"frmreason_l : REASON UNSPECIFIED",
"frmreason_l : REASON AUTH EXPIRE",
"frmreason_l : REASON AUTH LEAVE",
"frmreason_l : REASON ASSOC EXPIRE",
"frmreason_l : REASON ASSOC TOOMANY",
"frmreason_l : REASON NOT AUTHED",
"frmreason_l : REASON NOT ASSOCED",
"frmreason_l : REASON ASSOC LEAVE",
"frmreason_l : REASON ASSOC NOT AUTHED",
"frmreason_l : REASON RESERVED",
"frmreason_l : REASON RSN REQUIRED",
"frmreason_l : REASON RSN INCONSISTENT",
"frmreason_l : REASON IE INVALID",
"frmreason_l : REASON MIC FAILURE",
"frmmatchdir :",
"frmmatchdir : DIR ANY",
"frmmatchdir : DIR not frmdir",
"frmdir : NO DS",
"frmdir : TO DS",
"frmdir : FROM DS",
"frmdir : DS TO DS",
"frmmatchfrom :",
"frmmatchfrom : FROM ANY",
"frmmatchfrom : FROM not frmmatchaddr",
"frmmatchto :",
"frmmatchto : TO ANY",
"frmmatchto : TO not frmmatchaddr",
"frmmatchbssid :",
"frmmatchbssid : BSSID ANY",
"frmmatchbssid : BSSID not frmmatchaddr",
"frmmatchrtap :",
"frmmatchrtap : frmmatchrtap_l",
"frmmatchrtap_l : frmmatchrtap_l frmmatchrtapopt",
"frmmatchrtap_l : frmmatchrtapopt",
"frmmatchrtapopt : RSSI unaryop percent",
"frmmatchrtapopt : TXRATE unaryop txrate",
"frmmatchrtapopt : FREQ unaryop freq",
"frmmatchaddr : table",
"frmmatchaddr : lladdr",
"frmactiontype : TYPE DATA",
"frmactiontype : TYPE MANAGEMENT frmactionmgmt",
"frmactionmgmt : SUBTYPE frmsubtype",
"frmactiondir :",
"frmactiondir : DIR frmdir",
"frmactionfrom : FROM frmactionaddr",
"frmactionto : TO frmactionaddr",
"frmactionbssid : BSSID frmactionaddr",
"frmactionaddr : lladdr",
"frmactionaddr : randaddr",
"frmactionaddr : refaddr",
"table : '<' STRING '>'",
"$$3 :",
"tabledef : TABLE table $$3 tableopts",
"tableopts :",
"tableopts : tableopts_l",
"tableopts_l : tableopts_l tableopt",
"tableopts_l : tableopt",
"tableopt : CONST",
"tableopt : '{' optnl '}'",
"tableopt : '{' optnl tableaddrlist optnl '}'",
"string : string STRING",
"string : STRING",
"varset : STRING '=' string",
"refaddr : '&' FROM",
"refaddr : '&' TO",
"refaddr : '&' BSSID",
"tableaddrlist : tableaddrentry",
"tableaddrlist : tableaddrlist comma tableaddrentry",
"$$4 :",
"tableaddrentry : lladdr $$4 tableaddropt",
"tableaddropt :",
"tableaddropt : assign ipv4addr ipv4netmask",
"tableaddropt : mask lladdr",
"ipv4addr : STRING",
"ipv4netmask :",
"ipv4netmask : '/' NUMBER",
"lladdr : STRING",
"randaddr : RANDOM",
"passive :",
"passive : PASSIVE",
"assign : ARROW",
"mask : '&'",
"comma :",
"comma : ',' optnl",
"optnl :",
"optnl : '\\n'",
"not :",
"not : '!'",
"not : NOT",
"unaryop :",
"unaryop : '='",
"unaryop : \"==\"",
"unaryop : '!'",
"unaryop : NE",
"unaryop : LE",
"unaryop : '<'",
"unaryop : GE",
"unaryop : '>'",
"percent : STRING",
"txrate : STRING",
"freq : STRING",
"timeout : NUMBER",

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
#line 1232 "parse.y"

/*
 * Parser and lexer
 */

struct keywords {
	char *k_name;
	int k_val;
};

int
kw_cmp(const void *a, const void *b)
{
	return strcmp(a, ((const struct keywords *)b)->k_name);
}

int
lookup(char *token)
{
	/* Keep this list sorted */
	static const struct keywords keywords[] = {
		{ "add",		ADD },
		{ "address",		ADDRESS },
		{ "any",		ANY },
		{ "assoc",		ASSOC },
		{ "assoced",		ASSOCED },
		{ "atim",		ATIM },
		{ "auth",		AUTH },
		{ "authed",		AUTHED },
		{ "beacon",		BEACON },
		{ "broadcast",		BROADCAST },
		{ "bssid",		BSSID },
		{ "const",		CONST },
		{ "data",		DATA },
		{ "deauth",		DEAUTH },
		{ "delay",		DELAY },
		{ "delete",		DELETE },
		{ "dir",		DIR },
		{ "disassoc",		DISASSOC },
		{ "ds",			DS },
		{ "expire",		EXPIRE },
		{ "failure",		FAILURE },
		{ "frame",		FRAME },
		{ "freq",		FREQ },
		{ "from",		FROM },
		{ "handle",		HANDLE },
		{ "hopper",		HOPPER },
		{ "hostap",		HOSTAP },
		{ "iapp",		IAPP },
		{ "ie",			IE },
		{ "include",		INCLUDE },
		{ "inconsistent",	INCONSISTENT },
		{ "interface",		INTERFACE },
		{ "invalid",		INVALID },
		{ "leave",		LEAVE },
		{ "limit",		LIMIT },
		{ "log",		LOG },
		{ "management",		MANAGEMENT },
		{ "mic",		MIC },
		{ "mode",		MODE },
		{ "multicast",		MULTICAST },
		{ "no",			NO },
		{ "node",		NODE },
		{ "not",		NOT },
		{ "notify",		NOTIFY },
		{ "nwid",		NWID },
		{ "on",			ON },
		{ "open",		OPEN },
		{ "passive",		PASSIVE },
		{ "pcap",		PCAP },
		{ "port",		PORT },
		{ "probe",		PROBE },
		{ "quick",		QUICK },
		{ "radiotap",		RADIOTAP },
		{ "random",		RANDOM },
		{ "rate",		RATE },
		{ "reason",		REASON },
		{ "reassoc",		REASSOC },
		{ "request",		REQUEST },
		{ "required",		REQUIRED },
		{ "resend",		RESEND },
		{ "reserved",		RESERVED },
		{ "response",		RESPONSE },
		{ "roaming",		ROAMING },
		{ "route",		ROUTE },
		{ "rsn",		RSN },
		{ "sec",		SEC },
		{ "set",		SET },
		{ "signal",		RSSI },
		{ "skip",		SKIP },
		{ "subtype",		SUBTYPE },
		{ "table",		TABLE },
		{ "to",			TO },
		{ "toomany",		TOOMANY },
		{ "ttl",		TTL },
		{ "txrate",		TXRATE },
		{ "type",		TYPE },
		{ "unspecified",	UNSPECIFIED },
		{ "usec",		USEC },
		{ "verbose",		VERBOSE },
		{ "with",		WITH }
	};
	const struct keywords *p;

	p = bsearch(token, keywords, sizeof(keywords) / sizeof(keywords[0]),
	    sizeof(keywords[0]), kw_cmp);

	return (p == NULL ? STRING : p->k_val);
}

#define	START_EXPAND	1
#define	DONE_EXPAND	2

static int	expanding;

int
igetc(void)
{
	int	c;

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
			yyerror("reached end of file while parsing "
			    "quoted string");
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
			yyerror("macro \"%s\" not defined", buf);
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
			hostapd_fatal("yylex: strdup");
		return (STRING);
	case '-':
		next = lgetc(0);
		if (next == '>')
			return (ARROW);
		lungetc(next);
		break;
	case '!':
		next = lgetc(0);
		if (next == '=')
			return (NE);
		lungetc(next);
		break;		
	case '<':
		next = lgetc(0);
		if (next == '=')
			return (LE);
		lungetc(next);
		break;
	case '>':
		next = lgetc(0);
		if (next == '=')
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

	if (isalnum(c) || c == ':' || c == '_' || c == '*') {
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
				hostapd_fatal("yylex: strdup");
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

	hostapd_log(HOSTAPD_LOG_DEBUG, "%s = \"%s\"", sym->nam, sym->val);

	return (0);
}

int
hostapd_parse_symset(char *s)
{
	char	*sym, *val;
	int	ret;
	size_t	len;

	if ((val = strrchr(s, '=')) == NULL)
		return (-1);

	len = strlen(s) - strlen(val) + 1;
	if ((sym = malloc(len)) == NULL)
		hostapd_fatal("cmdline_symset: malloc");

	(void)strlcpy(sym, s, len);

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

	if ((nfile = calloc(1, sizeof(struct file))) == NULL) {
		warn("%s", __func__);
		return (NULL);
	}
	if ((nfile->name = strdup(name)) == NULL) {
		warn("%s", __func__);
		free(nfile);
		return (NULL);
	}
	if ((nfile->stream = fopen(nfile->name, "r")) == NULL) {
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
hostapd_parse_file(struct hostapd_config *cfg)
{
	struct sym *sym, *next;
	int errors = 0;
	int ret;

	if ((file = pushfile(cfg->c_config, 1)) == NULL)
		hostapd_fatal("failed to open the main config file: %s\n",
		    cfg->c_config);
	topfile = file;

	/* Init tables and data structures */
	TAILQ_INIT(&cfg->c_apmes);
	TAILQ_INIT(&cfg->c_tables);
	TAILQ_INIT(&cfg->c_frames);
	cfg->c_iapp.i_multicast.sin_addr.s_addr = INADDR_ANY;
	cfg->c_iapp.i_flags = HOSTAPD_IAPP_F_DEFAULT;
	cfg->c_iapp.i_ttl = IP_DEFAULT_MULTICAST_TTL;
	cfg->c_apme_hopdelay.tv_sec = HOSTAPD_HOPPER_MDELAY / 1000;
	cfg->c_apme_hopdelay.tv_usec = (HOSTAPD_HOPPER_MDELAY % 1000) * 1000;

	ret = yyparse();
	errors = file->errors;
	popfile();

	/* Free macros and check which have not been used. */
	TAILQ_FOREACH_SAFE(sym, &symhead, entry, next) {
		if (!sym->used)
			hostapd_log(HOSTAPD_LOG_VERBOSE,
			    "warning: macro '%s' not used", sym->nam);
		if (!sym->persist) {
			free(sym->nam);
			free(sym->val);
			TAILQ_REMOVE(&symhead, sym, entry);
			free(sym);
		}
	}

	return (errors ? EINVAL : ret);
}

int
yyerror(const char *fmt, ...)
{
	va_list		 ap;
	char		*msg;

	file->errors++;

	va_start(ap, fmt);
	if (vasprintf(&msg, fmt, ap) == -1)
		hostapd_fatal("yyerror vasprintf");
	va_end(ap);
	fprintf(stderr, "%s:%d: %s\n", file->name, yylval.lineno, msg);
	fflush(stderr);
	free(msg);

	return (0);
}
#line 1449 "parse.c"

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
case 8:
#line 190 "parse.y"
	{ file->errors++; }
#line 1651 "parse.c"
break;
case 9:
#line 194 "parse.y"
	{
			struct file *nfile;

			if ((nfile = pushfile(yystack.l_mark[0].v.string, 1)) == NULL) {
				yyerror("failed to include file %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);

			file = nfile;
			lungetc('\n');
		}
#line 1668 "parse.c"
break;
case 10:
#line 209 "parse.y"
	{
			if (!TAILQ_EMPTY(&hostapd_cfg.c_apmes))
				hostapd_cfg.c_flags |= HOSTAPD_CFG_F_APME;
		}
#line 1676 "parse.c"
break;
case 12:
#line 215 "parse.y"
	{
			bcopy(&yystack.l_mark[0].v.timeout, &hostapd_cfg.c_apme_hopdelay,
			    sizeof(struct timeval));
		}
#line 1684 "parse.c"
break;
case 14:
#line 221 "parse.y"
	{
			if (strlcpy(hostapd_cfg.c_iapp.i_iface, yystack.l_mark[-1].v.string,
			    sizeof(hostapd_cfg.c_iapp.i_iface)) >=
			    sizeof(hostapd_cfg.c_iapp.i_iface)) {
				yyerror("invalid interface %s", yystack.l_mark[-1].v.string);
				free(yystack.l_mark[-1].v.string);
				YYERROR;
			}

			hostapd_cfg.c_flags |= HOSTAPD_CFG_F_IAPP;

			hostapd_log(HOSTAPD_LOG_DEBUG,
			    "%s: IAPP interface added", yystack.l_mark[-1].v.string);

			free(yystack.l_mark[-1].v.string);
		}
#line 1704 "parse.c"
break;
case 16:
#line 239 "parse.y"
	{
			if ((hostapd_cfg.c_iapp.i_addr_tbl =
			    hostapd_table_lookup(&hostapd_cfg, yystack.l_mark[0].v.string)) == NULL) {
				yyerror("undefined table <%s>", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 1717 "parse.c"
break;
case 17:
#line 249 "parse.y"
	{
			if ((hostapd_cfg.c_iapp.i_route_tbl =
			    hostapd_table_lookup(&hostapd_cfg, yystack.l_mark[0].v.string)) == NULL) {
				yyerror("undefined table <%s>", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 1730 "parse.c"
break;
case 19:
#line 262 "parse.y"
	{
			hostapd_cfg.c_flags &= ~HOSTAPD_CFG_F_BRDCAST;
		}
#line 1737 "parse.c"
break;
case 20:
#line 266 "parse.y"
	{
			hostapd_cfg.c_flags |= HOSTAPD_CFG_F_BRDCAST;
		}
#line 1744 "parse.c"
break;
case 22:
#line 273 "parse.y"
	{
			bcopy(&yystack.l_mark[0].v.in, &hostapd_cfg.c_iapp.i_multicast.sin_addr,
			    sizeof(struct in_addr));
		}
#line 1752 "parse.c"
break;
case 24:
#line 281 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > UINT16_MAX) {
				yyerror("port out of range: %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			hostapd_cfg.c_iapp.i_addr.sin_port = htons(yystack.l_mark[0].v.number);
		}
#line 1763 "parse.c"
break;
case 26:
#line 292 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 1 || yystack.l_mark[0].v.number > UINT8_MAX) {
				yyerror("ttl out of range: %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			hostapd_cfg.c_iapp.i_ttl = yystack.l_mark[0].v.number;
		}
#line 1774 "parse.c"
break;
case 27:
#line 302 "parse.y"
	{
			hostapd_cfg.c_apme_dlt = DLT_IEEE802_11_RADIO;
		}
#line 1781 "parse.c"
break;
case 28:
#line 306 "parse.y"
	{
			hostapd_cfg.c_apme_dlt = DLT_IEEE802_11;
		}
#line 1788 "parse.c"
break;
case 33:
#line 320 "parse.y"
	{
			if (hostapd_apme_add(&hostapd_cfg, yystack.l_mark[0].v.string) != 0) {
				yyerror("failed to add hostap interface");
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 1799 "parse.c"
break;
case 38:
#line 338 "parse.y"
	{
			if ((apme = hostapd_apme_addhopper(&hostapd_cfg,
			    yystack.l_mark[0].v.string)) == NULL) {
				yyerror("failed to add hopper %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 1812 "parse.c"
break;
case 40:
#line 351 "parse.y"
	{
			if ((frame.f_apme =
			    hostapd_apme_lookup(&hostapd_cfg, yystack.l_mark[0].v.string)) == NULL) {
				yyerror("undefined hostap interface");
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);

			HOSTAPD_MATCH(APME, yystack.l_mark[-1].v.number);
		}
#line 1827 "parse.c"
break;
case 41:
#line 365 "parse.y"
	{
			bzero(&frame, sizeof(struct hostapd_frame));
			/* IEEE 802.11 frame to match */
			frame_ieee80211 = &frame.f_frame;
		}
#line 1836 "parse.c"
break;
case 42:
#line 369 "parse.y"
	{
			/* IEEE 802.11 raw frame to send as an action */
			frame_ieee80211 = &frame.f_action_data.a_frame;
		}
#line 1844 "parse.c"
break;
case 43:
#line 372 "parse.y"
	{
			if ((frame_ptr = calloc(1, sizeof(struct hostapd_frame)))
			    == NULL) {
				yyerror("calloc");
				YYERROR;
			}

			if (gettimeofday(&frame.f_last, NULL) == -1)
				hostapd_fatal("gettimeofday");
			timeradd(&frame.f_last, &frame.f_limit, &frame.f_then);

			bcopy(&frame, frame_ptr, sizeof(struct hostapd_frame));
			TAILQ_INSERT_TAIL(&hostapd_cfg.c_frames,
			    frame_ptr, f_entries);
		}
#line 1863 "parse.c"
break;
case 48:
#line 398 "parse.y"
	{
			HOSTAPD_IAPP_FLAG(ADD_NOTIFY, yystack.l_mark[-2].v.number);
		}
#line 1870 "parse.c"
break;
case 49:
#line 402 "parse.y"
	{
			HOSTAPD_IAPP_FLAG(RADIOTAP, yystack.l_mark[-1].v.number);
		}
#line 1877 "parse.c"
break;
case 50:
#line 406 "parse.y"
	{
			HOSTAPD_IAPP_FLAG(ROAMING_ROUTE, yystack.l_mark[-2].v.number);
		}
#line 1884 "parse.c"
break;
case 51:
#line 410 "parse.y"
	{
			HOSTAPD_IAPP_FLAG(ROAMING_ADDRESS, yystack.l_mark[-2].v.number);
		}
#line 1891 "parse.c"
break;
case 52:
#line 416 "parse.y"
	{
			frame.f_flags |= HOSTAPD_FRAME_F_RET_OK;
		}
#line 1898 "parse.c"
break;
case 53:
#line 420 "parse.y"
	{
			frame.f_flags |= HOSTAPD_FRAME_F_RET_QUICK;
		}
#line 1905 "parse.c"
break;
case 54:
#line 424 "parse.y"
	{
			frame.f_flags |= HOSTAPD_FRAME_F_RET_SKIP;
		}
#line 1912 "parse.c"
break;
case 55:
#line 430 "parse.y"
	{
			frame.f_action = HOSTAPD_ACTION_NONE;
		}
#line 1919 "parse.c"
break;
case 56:
#line 434 "parse.y"
	{
			frame.f_action = HOSTAPD_ACTION_LOG;
		}
#line 1926 "parse.c"
break;
case 57:
#line 438 "parse.y"
	{
			frame.f_action = HOSTAPD_ACTION_FRAME;
		}
#line 1933 "parse.c"
break;
case 59:
#line 443 "parse.y"
	{
			if ((yystack.l_mark[0].v.reflladdr.flags & HOSTAPD_ACTION_F_REF_M) == 0) {
				bcopy(yystack.l_mark[0].v.reflladdr.lladdr, frame.f_action_data.a_lladdr,
				    IEEE80211_ADDR_LEN);
			} else
				frame.f_action_data.a_flags |= yystack.l_mark[0].v.reflladdr.flags;
		}
#line 1944 "parse.c"
break;
case 60:
#line 451 "parse.y"
	{
			frame.f_action = HOSTAPD_ACTION_RESEND;
		}
#line 1951 "parse.c"
break;
case 62:
#line 458 "parse.y"
	{
			frame.f_action_flags |= HOSTAPD_ACTION_VERBOSE;
		}
#line 1958 "parse.c"
break;
case 63:
#line 464 "parse.y"
	{
			frame.f_action = HOSTAPD_ACTION_RADIOTAP;
		}
#line 1965 "parse.c"
break;
case 64:
#line 470 "parse.y"
	{
			frame.f_action = HOSTAPD_ACTION_DELNODE;
		}
#line 1972 "parse.c"
break;
case 65:
#line 474 "parse.y"
	{
			frame.f_action = HOSTAPD_ACTION_ADDNODE;
		}
#line 1979 "parse.c"
break;
case 72:
#line 493 "parse.y"
	{
			if (yystack.l_mark[-1].v.number < 0 || yystack.l_mark[-1].v.number > LONG_MAX) {
				yyerror("limit out of range: %lld sec", yystack.l_mark[-1].v.number);
				YYERROR;
			}
			frame.f_limit.tv_sec = yystack.l_mark[-1].v.number;
		}
#line 1990 "parse.c"
break;
case 73:
#line 501 "parse.y"
	{
			if (yystack.l_mark[-1].v.number < 0 || yystack.l_mark[-1].v.number > LONG_MAX) {
				yyerror("limit out of range: %lld usec", yystack.l_mark[-1].v.number);
				YYERROR;
			}
			frame.f_limit.tv_sec = yystack.l_mark[-1].v.number / 1000000;
			frame.f_limit.tv_usec = yystack.l_mark[-1].v.number % 1000000;
		}
#line 2002 "parse.c"
break;
case 75:
#line 513 "parse.y"
	{
			if ((yystack.l_mark[-3].v.number < 1 || yystack.l_mark[-3].v.number > LONG_MAX) ||
			    (yystack.l_mark[-1].v.number < 1 || yystack.l_mark[-1].v.number > LONG_MAX)) {
				yyerror("rate out of range: %lld/%lld sec",
				    yystack.l_mark[-3].v.number, yystack.l_mark[-1].v.number);
				YYERROR;
			}

			if (!(yystack.l_mark[-3].v.number && yystack.l_mark[-1].v.number)) {
				yyerror("invalid rate");
				YYERROR;
			}

			frame.f_rate = yystack.l_mark[-3].v.number;
			frame.f_rate_intval = yystack.l_mark[-1].v.number;
		}
#line 2022 "parse.c"
break;
case 78:
#line 534 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_TYPE_DATA;
			HOSTAPD_MATCH(TYPE, yystack.l_mark[-1].v.number);
		}
#line 2031 "parse.c"
break;
case 79:
#line 540 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_TYPE_MGT;
			HOSTAPD_MATCH(TYPE, yystack.l_mark[-2].v.number);
		}
#line 2040 "parse.c"
break;
case 82:
#line 550 "parse.y"
	{
			HOSTAPD_MATCH(SUBTYPE, yystack.l_mark[-1].v.number);
		}
#line 2047 "parse.c"
break;
case 83:
#line 556 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_SUBTYPE_PROBE_REQ;
		}
#line 2055 "parse.c"
break;
case 84:
#line 561 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_SUBTYPE_PROBE_RESP;
		}
#line 2063 "parse.c"
break;
case 85:
#line 566 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_SUBTYPE_BEACON;
		}
#line 2071 "parse.c"
break;
case 86:
#line 571 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_SUBTYPE_ATIM;
		}
#line 2079 "parse.c"
break;
case 87:
#line 576 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_SUBTYPE_AUTH;
		}
#line 2087 "parse.c"
break;
case 88:
#line 581 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_SUBTYPE_DEAUTH;
		}
#line 2095 "parse.c"
break;
case 89:
#line 586 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_SUBTYPE_ASSOC_REQ;
		}
#line 2103 "parse.c"
break;
case 90:
#line 591 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_SUBTYPE_DISASSOC;
		}
#line 2111 "parse.c"
break;
case 91:
#line 596 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_SUBTYPE_ASSOC_RESP;
		}
#line 2119 "parse.c"
break;
case 92:
#line 601 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_SUBTYPE_REASSOC_REQ;
		}
#line 2127 "parse.c"
break;
case 93:
#line 606 "parse.y"
	{
			frame_ieee80211->i_fc[0] |=
			    IEEE80211_FC0_SUBTYPE_REASSOC_RESP;
		}
#line 2135 "parse.c"
break;
case 100:
#line 625 "parse.y"
	{
			if ((frame_ieee80211->i_data = malloc(6)) == NULL) {
				yyerror("failed to allocate auth");
				YYERROR;
			}
			((u_int16_t *)frame_ieee80211->i_data)[0] =
				yystack.l_mark[0].v.authalg.alg;
			((u_int16_t *)frame_ieee80211->i_data)[1] =
				yystack.l_mark[0].v.authalg.transaction;
			((u_int16_t *)frame_ieee80211->i_data)[0] = 0;
			frame_ieee80211->i_data_len = 6;
		}
#line 2151 "parse.c"
break;
case 101:
#line 640 "parse.y"
	{
			yyval.v.authalg.alg = htole16(IEEE80211_AUTH_ALG_OPEN);
			yyval.v.authalg.transaction = htole16(IEEE80211_AUTH_OPEN_REQUEST);
		}
#line 2159 "parse.c"
break;
case 102:
#line 645 "parse.y"
	{
			yyval.v.authalg.alg = htole16(IEEE80211_AUTH_ALG_OPEN);
			yyval.v.authalg.transaction = htole16(IEEE80211_AUTH_OPEN_RESPONSE);
		}
#line 2167 "parse.c"
break;
case 103:
#line 652 "parse.y"
	{
			if (yystack.l_mark[0].v.reason != 0) {
				if ((frame_ieee80211->i_data =
				    malloc(sizeof(u_int16_t))) == NULL) {
					yyerror("failed to allocate "
					    "reason code %u", yystack.l_mark[0].v.reason);
					YYERROR;
				}
				*(u_int16_t *)frame_ieee80211->i_data =
				    htole16(yystack.l_mark[0].v.reason);
				frame_ieee80211->i_data_len = sizeof(u_int16_t);
			}
		}
#line 2184 "parse.c"
break;
case 104:
#line 668 "parse.y"
	{
			yyval.v.reason = 0;
		}
#line 2191 "parse.c"
break;
case 105:
#line 672 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_UNSPECIFIED;
		}
#line 2198 "parse.c"
break;
case 106:
#line 676 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_AUTH_EXPIRE;
		}
#line 2205 "parse.c"
break;
case 107:
#line 680 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_AUTH_LEAVE;
		}
#line 2212 "parse.c"
break;
case 108:
#line 684 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_ASSOC_EXPIRE;
		}
#line 2219 "parse.c"
break;
case 109:
#line 688 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_ASSOC_TOOMANY;
		}
#line 2226 "parse.c"
break;
case 110:
#line 692 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_NOT_AUTHED;
		}
#line 2233 "parse.c"
break;
case 111:
#line 696 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_NOT_ASSOCED;
		}
#line 2240 "parse.c"
break;
case 112:
#line 700 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_ASSOC_LEAVE;
		}
#line 2247 "parse.c"
break;
case 113:
#line 704 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_NOT_AUTHED;
		}
#line 2254 "parse.c"
break;
case 114:
#line 708 "parse.y"
	{
			yyval.v.reason = 10;	/* XXX unknown */
		}
#line 2261 "parse.c"
break;
case 115:
#line 712 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_RSN_REQUIRED;
		}
#line 2268 "parse.c"
break;
case 116:
#line 716 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_RSN_INCONSISTENT;
		}
#line 2275 "parse.c"
break;
case 117:
#line 720 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_IE_INVALID;
		}
#line 2282 "parse.c"
break;
case 118:
#line 724 "parse.y"
	{
			yyval.v.reason = IEEE80211_REASON_MIC_FAILURE;
		}
#line 2289 "parse.c"
break;
case 121:
#line 732 "parse.y"
	{
			HOSTAPD_MATCH(DIR, yystack.l_mark[-1].v.number);
		}
#line 2296 "parse.c"
break;
case 122:
#line 738 "parse.y"
	{
			frame_ieee80211->i_fc[1] |= IEEE80211_FC1_DIR_NODS;
		}
#line 2303 "parse.c"
break;
case 123:
#line 742 "parse.y"
	{
			frame_ieee80211->i_fc[1] |= IEEE80211_FC1_DIR_TODS;
		}
#line 2310 "parse.c"
break;
case 124:
#line 746 "parse.y"
	{
			frame_ieee80211->i_fc[1] |= IEEE80211_FC1_DIR_FROMDS;
		}
#line 2317 "parse.c"
break;
case 125:
#line 750 "parse.y"
	{
			frame_ieee80211->i_fc[1] |= IEEE80211_FC1_DIR_DSTODS;
		}
#line 2324 "parse.c"
break;
case 128:
#line 758 "parse.y"
	{
			if ((yystack.l_mark[0].v.reflladdr.flags & HOSTAPD_ACTION_F_OPT_TABLE) == 0) {
				bcopy(yystack.l_mark[0].v.reflladdr.lladdr, &frame_ieee80211->i_from,
				    IEEE80211_ADDR_LEN);
				HOSTAPD_MATCH(FROM, yystack.l_mark[-1].v.number);
			} else {
				frame.f_from = yystack.l_mark[0].v.reflladdr.table;
				HOSTAPD_MATCH_TABLE(FROM, yystack.l_mark[-1].v.number);
			}
		}
#line 2338 "parse.c"
break;
case 131:
#line 773 "parse.y"
	{
			if ((yystack.l_mark[0].v.reflladdr.flags & HOSTAPD_ACTION_F_OPT_TABLE) == 0) {
				bcopy(yystack.l_mark[0].v.reflladdr.lladdr, &frame_ieee80211->i_to,
				    IEEE80211_ADDR_LEN);
				HOSTAPD_MATCH(TO, yystack.l_mark[-1].v.number);
			} else {
				frame.f_to = yystack.l_mark[0].v.reflladdr.table;
				HOSTAPD_MATCH_TABLE(TO, yystack.l_mark[-1].v.number);
			}
		}
#line 2352 "parse.c"
break;
case 134:
#line 788 "parse.y"
	{
			if ((yystack.l_mark[0].v.reflladdr.flags & HOSTAPD_ACTION_F_OPT_TABLE) == 0) {
				bcopy(yystack.l_mark[0].v.reflladdr.lladdr, &frame_ieee80211->i_bssid,
				    IEEE80211_ADDR_LEN);
				HOSTAPD_MATCH(BSSID, yystack.l_mark[-1].v.number);
			} else {
				frame.f_bssid = yystack.l_mark[0].v.reflladdr.table;
				HOSTAPD_MATCH_TABLE(BSSID, yystack.l_mark[-1].v.number);
			}
		}
#line 2366 "parse.c"
break;
case 139:
#line 809 "parse.y"
	{
			if ((yystack.l_mark[-1].v.op == HOSTAPD_OP_GT && yystack.l_mark[0].v.number == 100) ||
			    (yystack.l_mark[-1].v.op == HOSTAPD_OP_LE && yystack.l_mark[0].v.number == 100) ||
			    (yystack.l_mark[-1].v.op == HOSTAPD_OP_LT && yystack.l_mark[0].v.number == 0) ||
			    (yystack.l_mark[-1].v.op == HOSTAPD_OP_GE && yystack.l_mark[0].v.number == 0)) {
				yyerror("absurd unary comparison");
				YYERROR;
			}

			frame.f_rssi_op = yystack.l_mark[-1].v.op;
			frame.f_rssi = yystack.l_mark[0].v.number;
			HOSTAPD_MATCH_RADIOTAP(RSSI);
		}
#line 2383 "parse.c"
break;
case 140:
#line 823 "parse.y"
	{
			frame.f_txrate_op = yystack.l_mark[-1].v.op;
			frame.f_txrate = yystack.l_mark[0].v.number;
			HOSTAPD_MATCH_RADIOTAP(RATE);
		}
#line 2392 "parse.c"
break;
case 141:
#line 829 "parse.y"
	{
			frame.f_chan_op = yystack.l_mark[-1].v.op;
			frame.f_chan = yystack.l_mark[0].v.number;
			HOSTAPD_MATCH_RADIOTAP(CHANNEL);
		}
#line 2401 "parse.c"
break;
case 142:
#line 837 "parse.y"
	{
			if ((yyval.v.reflladdr.table =
			    hostapd_table_lookup(&hostapd_cfg, yystack.l_mark[0].v.string)) == NULL) {
				yyerror("undefined table <%s>", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			yyval.v.reflladdr.flags = HOSTAPD_ACTION_F_OPT_TABLE;
			free(yystack.l_mark[0].v.string);
		}
#line 2415 "parse.c"
break;
case 143:
#line 848 "parse.y"
	{
			bcopy(yystack.l_mark[0].v.reflladdr.lladdr, yyval.v.reflladdr.lladdr, IEEE80211_ADDR_LEN);
			yyval.v.reflladdr.flags = HOSTAPD_ACTION_F_OPT_LLADDR;
		}
#line 2423 "parse.c"
break;
case 144:
#line 855 "parse.y"
	{
			frame_ieee80211->i_fc[0] |= IEEE80211_FC0_TYPE_DATA;
		}
#line 2430 "parse.c"
break;
case 145:
#line 859 "parse.y"
	{
			frame_ieee80211->i_fc[0] |= IEEE80211_FC0_TYPE_MGT;
		}
#line 2437 "parse.c"
break;
case 147:
#line 868 "parse.y"
	{
			frame.f_action_data.a_flags |=
			    HOSTAPD_ACTION_F_OPT_DIR_AUTO;
		}
#line 2445 "parse.c"
break;
case 149:
#line 876 "parse.y"
	{
			if ((yystack.l_mark[0].v.reflladdr.flags & HOSTAPD_ACTION_F_REF_M) == 0) {
				bcopy(yystack.l_mark[0].v.reflladdr.lladdr, frame_ieee80211->i_from,
				    IEEE80211_ADDR_LEN);
			} else
				frame.f_action_data.a_flags |=
				    (yystack.l_mark[0].v.reflladdr.flags << HOSTAPD_ACTION_F_REF_FROM_S);
		}
#line 2457 "parse.c"
break;
case 150:
#line 887 "parse.y"
	{
			if ((yystack.l_mark[0].v.reflladdr.flags & HOSTAPD_ACTION_F_REF_M) == 0) {
				bcopy(yystack.l_mark[0].v.reflladdr.lladdr, frame_ieee80211->i_to,
				    IEEE80211_ADDR_LEN);
			} else
				frame.f_action_data.a_flags |=
				    (yystack.l_mark[0].v.reflladdr.flags << HOSTAPD_ACTION_F_REF_TO_S);
		}
#line 2469 "parse.c"
break;
case 151:
#line 898 "parse.y"
	{
			if ((yystack.l_mark[0].v.reflladdr.flags & HOSTAPD_ACTION_F_REF_M) == 0) {
				bcopy(yystack.l_mark[0].v.reflladdr.lladdr, frame_ieee80211->i_bssid,
				    IEEE80211_ADDR_LEN);
			} else
				frame.f_action_data.a_flags |=
				    (yystack.l_mark[0].v.reflladdr.flags << HOSTAPD_ACTION_F_REF_BSSID_S);
		}
#line 2481 "parse.c"
break;
case 152:
#line 909 "parse.y"
	{
			bcopy(yystack.l_mark[0].v.reflladdr.lladdr, yyval.v.reflladdr.lladdr, IEEE80211_ADDR_LEN);
			yyval.v.reflladdr.flags = yystack.l_mark[0].v.reflladdr.flags;
		}
#line 2489 "parse.c"
break;
case 153:
#line 914 "parse.y"
	{
			yyval.v.reflladdr.flags = yystack.l_mark[0].v.reflladdr.flags;
		}
#line 2496 "parse.c"
break;
case 154:
#line 918 "parse.y"
	{
			yyval.v.reflladdr.flags = yystack.l_mark[0].v.reflladdr.flags;
		}
#line 2503 "parse.c"
break;
case 155:
#line 923 "parse.y"
	{
			if (strlen(yystack.l_mark[-1].v.string) >= HOSTAPD_TABLE_NAMELEN) {
				yyerror("table name %s too long, max %u",
				    yystack.l_mark[-1].v.string, HOSTAPD_TABLE_NAMELEN - 1);
				free(yystack.l_mark[-1].v.string);
				YYERROR;
			}
			yyval.v.string = yystack.l_mark[-1].v.string;
		}
#line 2516 "parse.c"
break;
case 156:
#line 934 "parse.y"
	{
			if ((table =
			    hostapd_table_add(&hostapd_cfg, yystack.l_mark[0].v.string)) == NULL) {
				yyerror("failed to add table: %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 2529 "parse.c"
break;
case 157:
#line 942 "parse.y"
	{
			table = NULL;
		}
#line 2536 "parse.c"
break;
case 162:
#line 955 "parse.y"
	{
			if (table->t_flags & HOSTAPD_TABLE_F_CONST) {
				yyerror("option already specified");
				YYERROR;
			}
			table->t_flags |= HOSTAPD_TABLE_F_CONST;
		}
#line 2547 "parse.c"
break;
case 165:
#line 967 "parse.y"
	{
			if (asprintf(&yyval.v.string, "%s %s", yystack.l_mark[-1].v.string, yystack.l_mark[0].v.string) == -1)
				hostapd_fatal("string: asprintf");
			free(yystack.l_mark[-1].v.string);
			free(yystack.l_mark[0].v.string);
		}
#line 2557 "parse.c"
break;
case 167:
#line 977 "parse.y"
	{
			char *s = yystack.l_mark[-2].v.string;
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
				hostapd_fatal("cannot store variable");
			free(yystack.l_mark[-2].v.string);
			free(yystack.l_mark[0].v.string);
		}
#line 2577 "parse.c"
break;
case 168:
#line 996 "parse.y"
	{
			yyval.v.reflladdr.flags |= HOSTAPD_ACTION_F_REF_FROM;
		}
#line 2584 "parse.c"
break;
case 169:
#line 1000 "parse.y"
	{
			yyval.v.reflladdr.flags |= HOSTAPD_ACTION_F_REF_TO;
		}
#line 2591 "parse.c"
break;
case 170:
#line 1004 "parse.y"
	{
			yyval.v.reflladdr.flags |= HOSTAPD_ACTION_F_REF_BSSID;
		}
#line 2598 "parse.c"
break;
case 173:
#line 1014 "parse.y"
	{
			if ((entry = hostapd_entry_add(table,
			    yystack.l_mark[0].v.reflladdr.lladdr)) == NULL) {
				yyerror("failed to add entry: %s",
				    etheraddr_string(yystack.l_mark[0].v.reflladdr.lladdr));
				YYERROR;
			}
		}
#line 2610 "parse.c"
break;
case 174:
#line 1021 "parse.y"
	{
			entry = NULL;
		}
#line 2617 "parse.c"
break;
case 176:
#line 1028 "parse.y"
	{
			entry->e_flags |= HOSTAPD_ENTRY_F_INADDR;
			entry->e_inaddr.in_af = AF_INET;
			bcopy(&yystack.l_mark[-1].v.in, &entry->e_inaddr.in_v4,
			    sizeof(struct in_addr));
		}
#line 2627 "parse.c"
break;
case 177:
#line 1035 "parse.y"
	{
			entry->e_flags |= HOSTAPD_ENTRY_F_MASK;
			bcopy(yystack.l_mark[0].v.reflladdr.lladdr, entry->e_mask, IEEE80211_ADDR_LEN);

			/* Update entry position in the table */
			hostapd_entry_update(table, entry);
		}
#line 2638 "parse.c"
break;
case 178:
#line 1045 "parse.y"
	{
			if (inet_net_pton(AF_INET, yystack.l_mark[0].v.string, &yyval.v.in, sizeof(yyval.v.in)) == -1) {
				yyerror("invalid address: %s\n", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 2650 "parse.c"
break;
case 179:
#line 1056 "parse.y"
	{
			entry->e_inaddr.in_netmask = -1;
		}
#line 2657 "parse.c"
break;
case 180:
#line 1060 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > 32) {
				yyerror("netmask out of range: %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			entry->e_inaddr.in_netmask = yystack.l_mark[0].v.number;
		}
#line 2668 "parse.c"
break;
case 181:
#line 1070 "parse.y"
	{
			struct ether_addr *ea;

			if ((ea = ether_aton(yystack.l_mark[0].v.string)) == NULL) {
				yyerror("invalid address: %s\n", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);

			bcopy(ea, yyval.v.reflladdr.lladdr, IEEE80211_ADDR_LEN);
			yyval.v.reflladdr.flags = HOSTAPD_ACTION_F_OPT_LLADDR;
		}
#line 2685 "parse.c"
break;
case 182:
#line 1086 "parse.y"
	{
			yyval.v.reflladdr.flags |= HOSTAPD_ACTION_F_REF_RANDOM;
		}
#line 2692 "parse.c"
break;
case 184:
#line 1093 "parse.y"
	{
			hostapd_cfg.c_flags |= HOSTAPD_CFG_F_IAPP_PASSIVE;
		}
#line 2699 "parse.c"
break;
case 191:
#line 1113 "parse.y"
	{
			yyval.v.number = 0;
		}
#line 2706 "parse.c"
break;
case 192:
#line 1117 "parse.y"
	{
			yyval.v.number = 1;
		}
#line 2713 "parse.c"
break;
case 193:
#line 1121 "parse.y"
	{
			yyval.v.number = 1;
		}
#line 2720 "parse.c"
break;
case 194:
#line 1127 "parse.y"
	{
			yyval.v.op = HOSTAPD_OP_EQ;
		}
#line 2727 "parse.c"
break;
case 195:
#line 1131 "parse.y"
	{
			yyval.v.op = HOSTAPD_OP_EQ;
		}
#line 2734 "parse.c"
break;
case 196:
#line 1135 "parse.y"
	{
			yyval.v.op = HOSTAPD_OP_EQ;
		}
#line 2741 "parse.c"
break;
case 197:
#line 1139 "parse.y"
	{
			yyval.v.op = HOSTAPD_OP_NE;
		}
#line 2748 "parse.c"
break;
case 198:
#line 1143 "parse.y"
	{
			yyval.v.op = HOSTAPD_OP_NE;
		}
#line 2755 "parse.c"
break;
case 199:
#line 1147 "parse.y"
	{
			yyval.v.op = HOSTAPD_OP_LE;
		}
#line 2762 "parse.c"
break;
case 200:
#line 1151 "parse.y"
	{
			yyval.v.op = HOSTAPD_OP_LT;
		}
#line 2769 "parse.c"
break;
case 201:
#line 1155 "parse.y"
	{
			yyval.v.op = HOSTAPD_OP_GE;
		}
#line 2776 "parse.c"
break;
case 202:
#line 1159 "parse.y"
	{
			yyval.v.op = HOSTAPD_OP_GT;
		}
#line 2783 "parse.c"
break;
case 203:
#line 1165 "parse.y"
	{
			double val;
			char *cp;

			val = strtod(yystack.l_mark[0].v.string, &cp);
			if (cp == NULL || strcmp(cp, "%") != 0 ||
			    val < 0 || val > 100) {
				yyerror("invalid percentage: %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
			yyval.v.number = val;
		}
#line 2801 "parse.c"
break;
case 204:
#line 1182 "parse.y"
	{
			double val;
			char *cp;

			val = strtod(yystack.l_mark[0].v.string, &cp) * 2;
			if (cp == NULL || strcasecmp(cp, "mb") != 0 ||
			    val != (int)val) {
				yyerror("invalid rate: %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
			yyval.v.number = val;
		}
#line 2819 "parse.c"
break;
case 205:
#line 1199 "parse.y"
	{
			double val;
			char *cp;

			val = strtod(yystack.l_mark[0].v.string, &cp);
			if (cp != NULL) {
				if (strcasecmp(cp, "ghz") == 0) {
					yyval.v.number = val * 1000;
				} else if (strcasecmp(cp, "mhz") == 0) {
					yyval.v.number = val;
				} else
					cp = NULL;
			}
			if (cp == NULL) {
				yyerror("invalid frequency: %s", yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			free(yystack.l_mark[0].v.string);
		}
#line 2843 "parse.c"
break;
case 206:
#line 1222 "parse.y"
	{
			if (yystack.l_mark[0].v.number < 1 || yystack.l_mark[0].v.number > LONG_MAX) {
				yyerror("timeout out of range: %lld", yystack.l_mark[0].v.number);
				YYERROR;
			}
			yyval.v.timeout.tv_sec = yystack.l_mark[0].v.number / 1000;
			yyval.v.timeout.tv_usec = (yystack.l_mark[0].v.number % 1000) * 1000;
		}
#line 2855 "parse.c"
break;
#line 2857 "parse.c"
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
