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

#ifndef yyparse
#define yyparse    pcap_yyparse
#endif /* yyparse */

#ifndef yylex
#define yylex      pcap_yylex
#endif /* yylex */

#ifndef yyerror
#define yyerror    pcap_yyerror
#endif /* yyerror */

#ifndef yychar
#define yychar     pcap_yychar
#endif /* yychar */

#ifndef yyval
#define yyval      pcap_yyval
#endif /* yyval */

#ifndef yylval
#define yylval     pcap_yylval
#endif /* yylval */

#ifndef yydebug
#define yydebug    pcap_yydebug
#endif /* yydebug */

#ifndef yynerrs
#define yynerrs    pcap_yynerrs
#endif /* yynerrs */

#ifndef yyerrflag
#define yyerrflag  pcap_yyerrflag
#endif /* yyerrflag */

#ifndef yylhs
#define yylhs      pcap_yylhs
#endif /* yylhs */

#ifndef yylen
#define yylen      pcap_yylen
#endif /* yylen */

#ifndef yydefred
#define yydefred   pcap_yydefred
#endif /* yydefred */

#ifndef yydgoto
#define yydgoto    pcap_yydgoto
#endif /* yydgoto */

#ifndef yysindex
#define yysindex   pcap_yysindex
#endif /* yysindex */

#ifndef yyrindex
#define yyrindex   pcap_yyrindex
#endif /* yyrindex */

#ifndef yygindex
#define yygindex   pcap_yygindex
#endif /* yygindex */

#ifndef yytable
#define yytable    pcap_yytable
#endif /* yytable */

#ifndef yycheck
#define yycheck    pcap_yycheck
#endif /* yycheck */

#ifndef yyname
#define yyname     pcap_yyname
#endif /* yyname */

#ifndef yyrule
#define yyrule     pcap_yyrule
#endif /* yyrule */
#define YYPREFIX "pcap_yy"

#define YYPURE 0

#line 2 "grammar.y"
/*	$OpenBSD: grammar.y,v 1.24 2024/04/08 02:51:14 jsg Exp $	*/

/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <net/if.h>

#include <netinet/in.h>
#include <netinet/if_ether.h>

#include <net/pfvar.h>

#include <net80211/ieee80211.h>

#include <stdio.h>
#include <string.h>

#include "pcap-int.h"

#include "gencode.h"
#include <pcap-namedb.h>

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif

#define QSET(q, p, d, a) (q).proto = (p),\
			 (q).dir = (d),\
			 (q).addr = (a)

int n_errors = 0;

static struct qual qerr = { Q_UNDEF, Q_UNDEF, Q_UNDEF, Q_UNDEF };

static void
yyerror(char *msg)
{
	++n_errors;
	bpf_error("%s", msg);
	/* NOTREACHED */
}

#ifndef YYBISON
int yyparse(void);

int
pcap_parse(void)
{
	return (yyparse());
}
#endif

#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#line 79 "grammar.y"
typedef union {
	int i;
	bpf_u_int32 h;
	u_char *e;
	char *s;
	struct stmt *stmt;
	struct arth *a;
	struct {
		struct qual q;
		struct block *b;
	} blk;
	struct block *rblk;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
#line 198 "grammar.c"

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

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();

#define DST 257
#define SRC 258
#define HOST 259
#define GATEWAY 260
#define NET 261
#define MASK 262
#define PORT 263
#define LESS 264
#define GREATER 265
#define PROTO 266
#define PROTOCHAIN 267
#define BYTE 268
#define ARP 269
#define RARP 270
#define IP 271
#define TCP 272
#define UDP 273
#define ICMP 274
#define IGMP 275
#define IGRP 276
#define PIM 277
#define ATALK 278
#define DECNET 279
#define LAT 280
#define SCA 281
#define MOPRC 282
#define MOPDL 283
#define STP 284
#define TK_BROADCAST 285
#define TK_MULTICAST 286
#define NUM 287
#define INBOUND 288
#define OUTBOUND 289
#define PF_IFNAME 290
#define PF_RSET 291
#define PF_RNR 292
#define PF_SRNR 293
#define PF_REASON 294
#define PF_ACTION 295
#define TYPE 296
#define SUBTYPE 297
#define DIR 298
#define ADDR1 299
#define ADDR2 300
#define ADDR3 301
#define ADDR4 302
#define LINK 303
#define GEQ 304
#define LEQ 305
#define NEQ 306
#define ID 307
#define EID 308
#define HID 309
#define HID6 310
#define LSH 311
#define RSH 312
#define LEN 313
#define RND 314
#define SAMPLE 315
#define IPV6 316
#define ICMPV6 317
#define AH 318
#define ESP 319
#define VLAN 320
#define MPLS 321
#define OR 322
#define AND 323
#define UMINUS 324
#define YYERRCODE 256
typedef int YYINT;
static const YYINT pcap_yylhs[] = {                      -1,
    0,    0,   24,    1,    1,    1,    1,    1,   20,   21,
    2,    2,    2,    3,    3,    3,    3,    3,    3,    3,
    3,   23,   22,    4,    4,    4,    7,    7,    5,    5,
    8,    8,    8,    8,    8,    8,    6,    6,    6,    6,
    6,    6,    9,    9,   10,   10,   10,   10,   10,   10,
   10,   10,   10,   10,   11,   11,   11,   12,   16,   16,
   16,   16,   16,   16,   16,   16,   16,   16,   16,   16,
   16,   16,   16,   16,   16,   16,   16,   16,   16,   25,
   25,   25,   25,   25,   25,   25,   25,   25,   25,   25,
   25,   25,   25,   26,   26,   26,   26,   26,   26,   29,
   29,   28,   27,   27,   27,   27,   30,   30,   31,   31,
   32,   32,   18,   18,   18,   19,   19,   19,   13,   13,
   14,   14,   14,   14,   14,   14,   14,   14,   14,   14,
   14,   14,   14,   14,   15,   15,   15,   15,   15,   17,
   17,
};
static const YYINT pcap_yylen[] = {                       2,
    2,    1,    0,    1,    3,    3,    3,    3,    1,    1,
    1,    1,    3,    1,    3,    3,    1,    3,    1,    1,
    2,    1,    1,    1,    3,    3,    1,    1,    1,    2,
    3,    2,    2,    2,    2,    2,    2,    3,    1,    3,
    3,    1,    1,    0,    1,    1,    3,    3,    3,    3,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    2,
    2,    2,    2,    4,    1,    1,    2,    1,    2,    1,
    1,    2,    2,    2,    2,    2,    2,    2,    2,    1,
    1,    1,    4,    2,    2,    2,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    4,    6,    3,    3,    3,    3,    3,    3,    3,    3,
    2,    3,    1,    1,    1,    1,    1,    1,    1,    1,
    3,
};
static const YYINT pcap_yydefred[] = {                    3,
    0,    0,    0,    0,    0,   61,   62,   60,   63,   64,
   65,   66,   67,   68,   69,   70,   71,   72,   74,   73,
   79,  140,   85,   86,    0,    0,    0,    0,    0,    0,
   59,  133,  134,    0,   75,   76,   77,   78,    0,    0,
   22,    0,   23,    0,    4,   29,    0,    0,    0,  120,
    0,  119,    0,    0,   42,   91,   82,   83,    0,   94,
   95,   96,   97,  100,  101,   98,  102,   99,   93,   87,
    0,   89,  131,    0,    0,   10,    9,    0,    0,   14,
   20,    0,    0,   37,   11,   12,    0,    0,    0,    0,
   55,   58,   56,   57,   34,   35,   80,   81,    0,    0,
    0,   51,   52,   53,   54,    0,   33,   36,   92,  114,
  116,  118,    0,    0,    0,    0,    0,    0,    0,    0,
  113,  115,  117,    0,    0,    0,    0,    0,    0,   30,
  136,  135,  138,  139,  137,    0,    0,    0,    6,    5,
    0,    0,    0,    8,    7,    0,    0,    0,   24,    0,
    0,    0,   21,    0,    0,    0,    0,  107,  108,    0,
  109,  110,  105,  111,  112,  106,   31,    0,    0,    0,
    0,    0,    0,  125,  126,    0,    0,    0,   38,  132,
  141,   84,    0,   16,   15,   18,   13,    0,    0,   48,
   50,   47,   49,    0,  121,    0,   25,   26,  103,    0,
  122,
};
static const YYINT pcap_yydgoto[] = {                     1,
  127,  153,   85,  150,   45,   46,  151,   47,   48,  106,
  107,  108,   49,   50,  136,   74,   52,  124,  125,   78,
   79,   75,   88,    2,   55,   56,  109,   68,   66,  160,
  163,  166,
};
static const YYINT pcap_yysindex[] = {                    0,
    0,  248, -273, -251, -232,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0, -290, -247, -207, -205, -275, -221,
    0,    0,    0, -194,    0,    0,    0,    0,  -36,  -36,
    0,  318,    0, -298,    0,    0,  -18,  526,  566,    0,
    9,    0,  248,  248,    0,    0,    0,    0,   77,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  -36,    0,    0,    9,  318,    0,    0,  189,  189,    0,
    0,  -42,   63,    0,    0,    0,  -18,  -18, -270, -227,
    0,    0,    0,    0,    0,    0,    0,    0, -268, -267,
 -240,    0,    0,    0,    0, -174,    0,    0,    0,    0,
    0,    0,  318,  318,  318,  318,  318,  318,  318,  318,
    0,    0,    0,  318,  318,  318,  -39,   71,   78,    0,
    0,    0,    0,    0,    0, -162,   78,  448,    0,    0,
    0,  189,  189,    0,    0, -183, -158, -154,    0,   93,
 -298,   78,    0, -123, -116, -111, -106,    0,    0, -145,
    0,    0,    0,    0,    0,    0,    0,  123,  123,  144,
  -16,  -24,  -24,    0,    0,  448,  448,  608,    0,    0,
    0,    0,   78,    0,    0,    0,    0,  -18,  -18,    0,
    0,    0,    0, -267,    0, -134,    0,    0,    0,   64,
    0,
};
static const YYINT pcap_yyrindex[] = {                    0,
    0,  150,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    8,   10,
    0,    0,    0,  156,    0,    0,    0,    0,    0,    0,
    1,    0,  589,  589,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  589,  589,    0,
    0,   21,   28,    0,    0,    0,    0,    0,  -33,  351,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  271,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  602,  627,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    3,  589,  589,    0,    0,    0,    0,    0,    0, -216,
    0, -201,    0,    0,    0,    0,    0,    0,    0,   58,
    0,    0,    0,    0,    0,    0,    0,   43,   56,   83,
   70,   16,   30,    0,    0,   72,   79,    0,    0,    0,
    0,    0,  117,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,
};
static const YYINT pcap_yygindex[] = {                    0,
  159,  -41,  -76,    0,  -45,    0,    0,    0,    0,    0,
   52,    0,  649,  -40,    0,  170,  656,    0,    0,   18,
   20,  628,  639,    0,    0,    0,    0,    0,    0,    0,
  -21,    0,
};
#define YYTABLESIZE 939
static const YYINT pcap_yytable[] = {                    46,
   39,  179,   12,   43,  147,   84,   46,   88,  130,   90,
  149,   64,  128,   57,   41,  123,   60,  119,  158,  161,
   17,   43,  120,   76,   77,  119,  117,   19,  118,  124,
  120,   65,  140,  145,  128,   58,  139,  144,  159,  162,
  119,   39,  129,   12,  119,  119,  164,  119,   88,  119,
   90,  154,  155,  123,   59,  130,  123,  104,  123,   61,
  123,   17,  119,  119,  119,  149,  165,  124,   19,  127,
  124,   40,  124,  123,  124,  123,  123,  123,   41,   62,
  129,   63,  128,  129,   91,   67,   93,  124,   94,  124,
  124,  124,   69,  130,  156,  157,  130,  130,  104,  126,
  129,  128,  129,  129,  129,   28,   28,  127,  123,  148,
  127,  180,   40,  130,  132,  130,  130,  130,  181,   41,
   27,   27,  124,  128,  182,  184,  119,  127,  185,  127,
  127,  127,  186,  187,  190,  129,  135,  134,  133,  123,
  128,  191,  128,  128,  128,  192,  197,  198,  130,    2,
  193,  194,  200,  124,  119,    1,  201,  167,  119,  119,
   44,  119,  127,  119,  119,  117,  129,  118,  188,  120,
  189,   51,  199,    0,    0,  128,  119,  119,  119,  130,
    0,  116,    0,    0,    0,  119,  117,    0,  118,    0,
  120,    0,    0,  127,    0,    0,    0,    0,    0,    0,
  131,    0,    0,    0,    0,    0,  128,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  146,
    0,   41,   51,   51,    0,   46,    0,   46,   43,   46,
    0,    0,    0,   42,    0,    0,    0,    0,    0,    0,
  119,    0,    0,    0,    0,    0,    0,   51,   51,    0,
   22,    0,    0,   46,    0,    0,    0,   43,   43,   43,
   43,   43,    0,   43,    0,    0,   43,   43,   22,    0,
    0,    0,    0,   46,   46,   46,   46,    0,    0,    0,
   41,    0,   76,   77,    0,   43,   43,   43,   80,   81,
   82,   83,   42,    0,  113,  114,   43,   43,   43,   43,
   43,   43,   43,   32,    0,    0,  119,  119,  119,    0,
   32,   51,   51,  119,  119,    0,    0,    0,    0,  123,
  123,  123,   39,   39,   12,   12,  123,  123,    0,   88,
   88,   90,   90,  124,  124,  124,    0,  123,  123,    0,
  124,  124,   17,   17,    0,    0,  129,  129,  129,   19,
   19,  124,  124,  129,  129,    0,    0,   43,    0,  130,
  130,  130,   42,    0,  129,  129,  130,  130,    0,    0,
    0,    0,    0,  127,  127,  127,    0,  130,  130,  104,
  104,    0,    0,   45,    0,    0,  128,  128,  128,    0,
   45,  127,  127,   40,   40,    0,    0,    0,    0,    0,
   41,   41,    0,    0,  128,  128,   44,   44,   44,   44,
   44,    0,   44,    0,    0,   44,   44,    0,    0,    0,
  119,  119,  119,    0,    0,    0,    0,  119,  119,    0,
    0,    0,    0,    0,   44,   44,    0,    0,   27,   27,
    0,    0,    0,    0,    0,   44,   44,   44,   44,   44,
   44,   44,    3,    4,  113,  114,    5,    6,    7,    8,
    9,   10,   11,   12,   13,   14,   15,   16,   17,   18,
   19,   20,   21,    0,    0,   22,   23,   24,   25,   26,
   27,   28,   29,   30,    0,  116,    0,    0,    0,  119,
  117,   31,  118,    0,  120,   80,   81,   82,   83,    0,
    0,   32,   33,   34,   35,   36,   37,   38,   39,   40,
    0,    3,    4,    0,    0,    5,    6,    7,    8,    9,
   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
   20,   21,    0,    0,   22,   23,   24,   25,   26,   27,
   28,   29,   30,    0,    0,    0,    0,    0,    0,    0,
   31,    0,    0,    0,    0,    0,    0,   32,    0,    0,
   32,   33,   34,   35,   36,   37,   38,   39,   40,    0,
    0,  115,    0,    0,    0,    0,    0,   32,   32,   32,
   32,    0,    0,    0,    0,    0,    6,    7,    8,    9,
   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
   20,   21,    0,  116,   22,    0,    0,  119,  117,   45,
  118,   45,  120,   45,    0,    0,    0,    0,    0,    0,
   31,    0,    0,    0,    0,  123,  122,  121,    0,   53,
   32,   33,    0,   35,   36,   37,   38,   45,    0,  120,
   54,    0,    0,  120,  120,  116,  120,    0,  120,  119,
  117,    0,  118,    0,  120,    0,    0,   45,   45,   45,
   45,  120,  120,  120,  119,  196,   71,   71,  119,  119,
    0,  119,    0,  119,   87,    0,    0,    0,    0,    0,
   53,   53,    0,    0,    0,    0,  119,  119,  119,  115,
   73,   54,   54,    0,   70,   72,    0,    0,   71,    0,
  195,    0,   86,    0,    0,  142,  142,    0,  129,    0,
    0,    0,    0,    0,   71,   87,  143,  143,    0,    0,
    0,    0,    0,  138,    0,  120,  137,    0,    0,    0,
  129,  115,    0,  141,  141,    0,    0,    0,    0,    0,
    0,    0,  152,   86,    0,    0,    0,    0,    0,    0,
  119,    0,    0,    0,    0,    0,    0,    0,  113,  114,
    0,  168,  169,  170,  171,  172,  173,  174,  175,   53,
  142,    0,  176,  177,  178,    0,    0,    0,    0,    0,
  143,  143,   89,   90,   91,   92,   93,    0,   94,    0,
    0,   95,   96,    0,    0,    0,    0,  183,  141,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   97,   98,    0,    0,    0,   87,   87,    0,    0,    0,
    0,   99,  100,  101,  102,  103,  104,  105,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   86,   86,   44,   44,   44,   44,   44,
    0,   44,    0,    0,   44,   44,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  110,
  111,  112,    0,   44,   44,    0,  113,  114,    0,    0,
    0,    0,    0,    0,   44,   44,   44,   44,   44,   44,
   44,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  120,  120,  120,    0,    0,
    0,    0,  120,  120,    0,    0,    0,    0,  113,  114,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  119,  119,  119,    0,    0,    0,    0,  119,  119,
};
static const YYINT pcap_yycheck[] = {                    33,
    0,   41,    0,   40,   47,   47,   40,    0,   54,    0,
   87,  287,   53,  287,   33,    0,  307,   42,  287,  287,
    0,   40,   47,  322,  323,   42,   43,    0,   45,    0,
   47,  307,   78,   79,   75,  287,   78,   79,  307,  307,
   38,   41,    0,   41,   42,   43,  287,   45,   41,   47,
   41,  322,  323,   38,  287,    0,   41,    0,   43,  307,
   45,   41,   60,   61,   62,  142,  307,   38,   41,    0,
   41,    0,   43,   58,   45,   60,   61,   62,    0,  287,
   38,  287,    0,   41,  259,  307,  261,   58,  263,   60,
   61,   62,  287,   38,  322,  323,   41,  143,   41,   91,
   58,  142,   60,   61,   62,  322,  323,   38,   93,   47,
   41,   41,   41,   58,   38,   60,   61,   62,   41,   41,
  322,  323,   93,   41,  287,  309,  124,   58,  287,   60,
   61,   62,  287,   41,  258,   93,   60,   61,   62,  124,
   58,  258,   60,   61,   62,  257,  188,  189,   93,    0,
  257,  297,  287,  124,   38,    0,   93,  106,   42,   43,
    2,   45,   93,   47,   42,   43,  124,   45,  151,   47,
  151,    2,  194,   -1,   -1,   93,   60,   61,   62,  124,
   -1,   38,   -1,   -1,   -1,   42,   43,   -1,   45,   -1,
   47,   -1,   -1,  124,   -1,   -1,   -1,   -1,   -1,   -1,
  124,   -1,   -1,   -1,   -1,   -1,  124,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  262,
   -1,   33,   53,   54,   -1,  259,   -1,  261,   40,  263,
   -1,   -1,   -1,   45,   -1,   -1,   -1,   -1,   -1,   -1,
  124,   -1,   -1,   -1,   -1,   -1,   -1,   78,   79,   -1,
  287,   -1,   -1,  287,   -1,   -1,   -1,  257,  258,  259,
  260,  261,   -1,  263,   -1,   -1,  266,  267,  287,   -1,
   -1,   -1,   -1,  307,  308,  309,  310,   -1,   -1,   -1,
   33,   -1,  322,  323,   -1,  285,  286,   40,  307,  308,
  309,  310,   45,   -1,  311,  312,  296,  297,  298,  299,
  300,  301,  302,   33,   -1,   -1,  304,  305,  306,   -1,
   40,  142,  143,  311,  312,   -1,   -1,   -1,   -1,  304,
  305,  306,  322,  323,  322,  323,  311,  312,   -1,  322,
  323,  322,  323,  304,  305,  306,   -1,  322,  323,   -1,
  311,  312,  322,  323,   -1,   -1,  304,  305,  306,  322,
  323,  322,  323,  311,  312,   -1,   -1,   40,   -1,  304,
  305,  306,   45,   -1,  322,  323,  311,  312,   -1,   -1,
   -1,   -1,   -1,  304,  305,  306,   -1,  322,  323,  322,
  323,   -1,   -1,   33,   -1,   -1,  304,  305,  306,   -1,
   40,  322,  323,  322,  323,   -1,   -1,   -1,   -1,   -1,
  322,  323,   -1,   -1,  322,  323,  257,  258,  259,  260,
  261,   -1,  263,   -1,   -1,  266,  267,   -1,   -1,   -1,
  304,  305,  306,   -1,   -1,   -1,   -1,  311,  312,   -1,
   -1,   -1,   -1,   -1,  285,  286,   -1,   -1,  322,  323,
   -1,   -1,   -1,   -1,   -1,  296,  297,  298,  299,  300,
  301,  302,  264,  265,  311,  312,  268,  269,  270,  271,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  284,   -1,   -1,  287,  288,  289,  290,  291,
  292,  293,  294,  295,   -1,   38,   -1,   -1,   -1,   42,
   43,  303,   45,   -1,   47,  307,  308,  309,  310,   -1,
   -1,  313,  314,  315,  316,  317,  318,  319,  320,  321,
   -1,  264,  265,   -1,   -1,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   -1,   -1,  287,  288,  289,  290,  291,  292,
  293,  294,  295,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  303,   -1,   -1,   -1,   -1,   -1,   -1,  287,   -1,   -1,
  313,  314,  315,  316,  317,  318,  319,  320,  321,   -1,
   -1,  124,   -1,   -1,   -1,   -1,   -1,  307,  308,  309,
  310,   -1,   -1,   -1,   -1,   -1,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   -1,   38,  287,   -1,   -1,   42,   43,  259,
   45,  261,   47,  263,   -1,   -1,   -1,   -1,   -1,   -1,
  303,   -1,   -1,   -1,   -1,   60,   61,   62,   -1,    2,
  313,  314,   -1,  316,  317,  318,  319,  287,   -1,   38,
    2,   -1,   -1,   42,   43,   38,   45,   -1,   47,   42,
   43,   -1,   45,   -1,   47,   -1,   -1,  307,  308,  309,
  310,   60,   61,   62,   38,   58,   39,   40,   42,   43,
   -1,   45,   -1,   47,   47,   -1,   -1,   -1,   -1,   -1,
   53,   54,   -1,   -1,   -1,   -1,   60,   61,   62,  124,
   42,   53,   54,   -1,   39,   40,   -1,   -1,   71,   -1,
   93,   -1,   47,   -1,   -1,   78,   79,   -1,   53,   -1,
   -1,   -1,   -1,   -1,   87,   88,   78,   79,   -1,   -1,
   -1,   -1,   -1,   75,   -1,  124,   71,   -1,   -1,   -1,
   75,  124,   -1,   78,   79,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   87,   88,   -1,   -1,   -1,   -1,   -1,   -1,
  124,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  311,  312,
   -1,  113,  114,  115,  116,  117,  118,  119,  120,  142,
  143,   -1,  124,  125,  126,   -1,   -1,   -1,   -1,   -1,
  142,  143,  257,  258,  259,  260,  261,   -1,  263,   -1,
   -1,  266,  267,   -1,   -1,   -1,   -1,  142,  143,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  285,  286,   -1,   -1,   -1,  188,  189,   -1,   -1,   -1,
   -1,  296,  297,  298,  299,  300,  301,  302,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  188,  189,  257,  258,  259,  260,  261,
   -1,  263,   -1,   -1,  266,  267,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  304,
  305,  306,   -1,  285,  286,   -1,  311,  312,   -1,   -1,
   -1,   -1,   -1,   -1,  296,  297,  298,  299,  300,  301,
  302,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  304,  305,  306,   -1,   -1,
   -1,   -1,  311,  312,   -1,   -1,   -1,   -1,  311,  312,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  304,  305,  306,   -1,   -1,   -1,   -1,  311,  312,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 324
#define YYUNDFTOKEN 359
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const pcap_yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'!'",0,0,0,0,"'&'",0,"'('","')'","'*'","'+'",0,"'-'",0,"'/'",0,0,0,0,0,0,0,0,0,
0,"':'",0,"'<'","'='","'>'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,"'['",0,"']'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'|'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"DST","SRC","HOST","GATEWAY","NET","MASK","PORT",
"LESS","GREATER","PROTO","PROTOCHAIN","BYTE","ARP","RARP","IP","TCP","UDP",
"ICMP","IGMP","IGRP","PIM","ATALK","DECNET","LAT","SCA","MOPRC","MOPDL","STP",
"TK_BROADCAST","TK_MULTICAST","NUM","INBOUND","OUTBOUND","PF_IFNAME","PF_RSET",
"PF_RNR","PF_SRNR","PF_REASON","PF_ACTION","TYPE","SUBTYPE","DIR","ADDR1",
"ADDR2","ADDR3","ADDR4","LINK","GEQ","LEQ","NEQ","ID","EID","HID","HID6","LSH",
"RSH","LEN","RND","SAMPLE","IPV6","ICMPV6","AH","ESP","VLAN","MPLS","OR","AND",
"UMINUS",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"illegal-symbol",
};
static const char *const pcap_yyrule[] = {
"$accept : prog",
"prog : null expr",
"prog : null",
"null :",
"expr : term",
"expr : expr and term",
"expr : expr and id",
"expr : expr or term",
"expr : expr or id",
"and : AND",
"or : OR",
"id : nid",
"id : pnum",
"id : paren pid ')'",
"nid : ID",
"nid : HID '/' NUM",
"nid : HID MASK HID",
"nid : HID",
"nid : HID6 '/' NUM",
"nid : HID6",
"nid : EID",
"nid : not id",
"not : '!'",
"paren : '('",
"pid : nid",
"pid : qid and id",
"pid : qid or id",
"qid : pnum",
"qid : pid",
"term : rterm",
"term : not term",
"head : pqual dqual aqual",
"head : pqual dqual",
"head : pqual aqual",
"head : pqual PROTO",
"head : pqual PROTOCHAIN",
"head : pqual ndaqual",
"rterm : head id",
"rterm : paren expr ')'",
"rterm : pname",
"rterm : arth relop arth",
"rterm : arth irelop arth",
"rterm : other",
"pqual : pname",
"pqual :",
"dqual : SRC",
"dqual : DST",
"dqual : SRC OR DST",
"dqual : DST OR SRC",
"dqual : SRC AND DST",
"dqual : DST AND SRC",
"dqual : ADDR1",
"dqual : ADDR2",
"dqual : ADDR3",
"dqual : ADDR4",
"aqual : HOST",
"aqual : NET",
"aqual : PORT",
"ndaqual : GATEWAY",
"pname : LINK",
"pname : IP",
"pname : ARP",
"pname : RARP",
"pname : TCP",
"pname : UDP",
"pname : ICMP",
"pname : IGMP",
"pname : IGRP",
"pname : PIM",
"pname : ATALK",
"pname : DECNET",
"pname : LAT",
"pname : SCA",
"pname : MOPDL",
"pname : MOPRC",
"pname : IPV6",
"pname : ICMPV6",
"pname : AH",
"pname : ESP",
"pname : STP",
"other : pqual TK_BROADCAST",
"other : pqual TK_MULTICAST",
"other : LESS NUM",
"other : GREATER NUM",
"other : BYTE NUM byteop NUM",
"other : INBOUND",
"other : OUTBOUND",
"other : VLAN pnum",
"other : VLAN",
"other : MPLS pnum",
"other : MPLS",
"other : pfvar",
"other : pqual p80211",
"other : SAMPLE NUM",
"pfvar : PF_IFNAME ID",
"pfvar : PF_RSET ID",
"pfvar : PF_RNR NUM",
"pfvar : PF_SRNR NUM",
"pfvar : PF_REASON reason",
"pfvar : PF_ACTION action",
"reason : NUM",
"reason : ID",
"action : ID",
"p80211 : TYPE type SUBTYPE subtype",
"p80211 : TYPE type",
"p80211 : SUBTYPE subtype",
"p80211 : DIR dir",
"type : NUM",
"type : ID",
"subtype : NUM",
"subtype : ID",
"dir : NUM",
"dir : ID",
"relop : '>'",
"relop : GEQ",
"relop : '='",
"irelop : LEQ",
"irelop : '<'",
"irelop : NEQ",
"arth : pnum",
"arth : narth",
"narth : pname '[' arth ']'",
"narth : pname '[' arth ':' NUM ']'",
"narth : arth '+' arth",
"narth : arth '-' arth",
"narth : arth '*' arth",
"narth : arth '/' arth",
"narth : arth '&' arth",
"narth : arth '|' arth",
"narth : arth LSH arth",
"narth : arth RSH arth",
"narth : '-' arth",
"narth : paren narth ')'",
"narth : LEN",
"narth : RND",
"byteop : '&'",
"byteop : '|'",
"byteop : '<'",
"byteop : '>'",
"byteop : '='",
"pnum : NUM",
"pnum : paren pnum ')'",

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
case 1:
#line 132 "grammar.y"
	{
	finish_parse(yystack.l_mark[0].blk.b);
}
#line 1021 "grammar.c"
break;
case 3:
#line 137 "grammar.y"
	{ yyval.blk.q = qerr; }
#line 1026 "grammar.c"
break;
case 5:
#line 140 "grammar.y"
	{ gen_and(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 1031 "grammar.c"
break;
case 6:
#line 141 "grammar.y"
	{ gen_and(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 1036 "grammar.c"
break;
case 7:
#line 142 "grammar.y"
	{ gen_or(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 1041 "grammar.c"
break;
case 8:
#line 143 "grammar.y"
	{ gen_or(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 1046 "grammar.c"
break;
case 9:
#line 145 "grammar.y"
	{ yyval.blk = yystack.l_mark[-1].blk; }
#line 1051 "grammar.c"
break;
case 10:
#line 147 "grammar.y"
	{ yyval.blk = yystack.l_mark[-1].blk; }
#line 1056 "grammar.c"
break;
case 12:
#line 150 "grammar.y"
	{ yyval.blk.b = gen_ncode(NULL, (bpf_u_int32)yystack.l_mark[0].i,
						   yyval.blk.q = yystack.l_mark[-1].blk.q); }
#line 1062 "grammar.c"
break;
case 13:
#line 152 "grammar.y"
	{ yyval.blk = yystack.l_mark[-1].blk; }
#line 1067 "grammar.c"
break;
case 14:
#line 154 "grammar.y"
	{ yyval.blk.b = gen_scode(yystack.l_mark[0].s, yyval.blk.q = yystack.l_mark[-1].blk.q); }
#line 1072 "grammar.c"
break;
case 15:
#line 155 "grammar.y"
	{ yyval.blk.b = gen_mcode(yystack.l_mark[-2].s, NULL, yystack.l_mark[0].i,
				    yyval.blk.q = yystack.l_mark[-3].blk.q); }
#line 1078 "grammar.c"
break;
case 16:
#line 157 "grammar.y"
	{ yyval.blk.b = gen_mcode(yystack.l_mark[-2].s, yystack.l_mark[0].s, 0,
				    yyval.blk.q = yystack.l_mark[-3].blk.q); }
#line 1084 "grammar.c"
break;
case 17:
#line 159 "grammar.y"
	{
				  /* Decide how to parse HID based on proto */
				  yyval.blk.q = yystack.l_mark[-1].blk.q;
				  switch (yyval.blk.q.proto) {
				  case Q_DECNET:
					yyval.blk.b = gen_ncode(yystack.l_mark[0].s, 0, yyval.blk.q);
					break;
				  default:
					yyval.blk.b = gen_ncode(yystack.l_mark[0].s, 0, yyval.blk.q);
					break;
				  }
				}
#line 1100 "grammar.c"
break;
case 18:
#line 171 "grammar.y"
	{
#ifdef INET6
				  yyval.blk.b = gen_mcode6(yystack.l_mark[-2].s, NULL, yystack.l_mark[0].i,
				    yyval.blk.q = yystack.l_mark[-3].blk.q);
#else
				  bpf_error("'ip6addr/prefixlen' not supported "
					"in this configuration");
#endif /*INET6*/
				}
#line 1113 "grammar.c"
break;
case 19:
#line 180 "grammar.y"
	{
#ifdef INET6
				  yyval.blk.b = gen_mcode6(yystack.l_mark[0].s, 0, 128,
				    yyval.blk.q = yystack.l_mark[-1].blk.q);
#else
				  bpf_error("'ip6addr' not supported "
					"in this configuration");
#endif /*INET6*/
				}
#line 1126 "grammar.c"
break;
case 20:
#line 189 "grammar.y"
	{ yyval.blk.b = gen_ecode(yystack.l_mark[0].e, yyval.blk.q = yystack.l_mark[-1].blk.q); }
#line 1131 "grammar.c"
break;
case 21:
#line 190 "grammar.y"
	{ gen_not(yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 1136 "grammar.c"
break;
case 22:
#line 192 "grammar.y"
	{ yyval.blk = yystack.l_mark[-1].blk; }
#line 1141 "grammar.c"
break;
case 23:
#line 194 "grammar.y"
	{ yyval.blk = yystack.l_mark[-1].blk; }
#line 1146 "grammar.c"
break;
case 25:
#line 197 "grammar.y"
	{ gen_and(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 1151 "grammar.c"
break;
case 26:
#line 198 "grammar.y"
	{ gen_or(yystack.l_mark[-2].blk.b, yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 1156 "grammar.c"
break;
case 27:
#line 200 "grammar.y"
	{ yyval.blk.b = gen_ncode(NULL, (bpf_u_int32)yystack.l_mark[0].i,
						   yyval.blk.q = yystack.l_mark[-1].blk.q); }
#line 1162 "grammar.c"
break;
case 30:
#line 205 "grammar.y"
	{ gen_not(yystack.l_mark[0].blk.b); yyval.blk = yystack.l_mark[0].blk; }
#line 1167 "grammar.c"
break;
case 31:
#line 207 "grammar.y"
	{ QSET(yyval.blk.q, yystack.l_mark[-2].i, yystack.l_mark[-1].i, yystack.l_mark[0].i); }
#line 1172 "grammar.c"
break;
case 32:
#line 208 "grammar.y"
	{ QSET(yyval.blk.q, yystack.l_mark[-1].i, yystack.l_mark[0].i, Q_DEFAULT); }
#line 1177 "grammar.c"
break;
case 33:
#line 209 "grammar.y"
	{ QSET(yyval.blk.q, yystack.l_mark[-1].i, Q_DEFAULT, yystack.l_mark[0].i); }
#line 1182 "grammar.c"
break;
case 34:
#line 210 "grammar.y"
	{ QSET(yyval.blk.q, yystack.l_mark[-1].i, Q_DEFAULT, Q_PROTO); }
#line 1187 "grammar.c"
break;
case 35:
#line 211 "grammar.y"
	{ QSET(yyval.blk.q, yystack.l_mark[-1].i, Q_DEFAULT, Q_PROTOCHAIN); }
#line 1192 "grammar.c"
break;
case 36:
#line 212 "grammar.y"
	{ QSET(yyval.blk.q, yystack.l_mark[-1].i, Q_DEFAULT, yystack.l_mark[0].i); }
#line 1197 "grammar.c"
break;
case 37:
#line 214 "grammar.y"
	{ yyval.blk = yystack.l_mark[0].blk; }
#line 1202 "grammar.c"
break;
case 38:
#line 215 "grammar.y"
	{ yyval.blk.b = yystack.l_mark[-1].blk.b; yyval.blk.q = yystack.l_mark[-2].blk.q; }
#line 1207 "grammar.c"
break;
case 39:
#line 216 "grammar.y"
	{ yyval.blk.b = gen_proto_abbrev(yystack.l_mark[0].i); yyval.blk.q = qerr; }
#line 1212 "grammar.c"
break;
case 40:
#line 217 "grammar.y"
	{ yyval.blk.b = gen_relation(yystack.l_mark[-1].i, yystack.l_mark[-2].a, yystack.l_mark[0].a, 0);
				  yyval.blk.q = qerr; }
#line 1218 "grammar.c"
break;
case 41:
#line 219 "grammar.y"
	{ yyval.blk.b = gen_relation(yystack.l_mark[-1].i, yystack.l_mark[-2].a, yystack.l_mark[0].a, 1);
				  yyval.blk.q = qerr; }
#line 1224 "grammar.c"
break;
case 42:
#line 221 "grammar.y"
	{ yyval.blk.b = yystack.l_mark[0].rblk; yyval.blk.q = qerr; }
#line 1229 "grammar.c"
break;
case 44:
#line 225 "grammar.y"
	{ yyval.i = Q_DEFAULT; }
#line 1234 "grammar.c"
break;
case 45:
#line 228 "grammar.y"
	{ yyval.i = Q_SRC; }
#line 1239 "grammar.c"
break;
case 46:
#line 229 "grammar.y"
	{ yyval.i = Q_DST; }
#line 1244 "grammar.c"
break;
case 47:
#line 230 "grammar.y"
	{ yyval.i = Q_OR; }
#line 1249 "grammar.c"
break;
case 48:
#line 231 "grammar.y"
	{ yyval.i = Q_OR; }
#line 1254 "grammar.c"
break;
case 49:
#line 232 "grammar.y"
	{ yyval.i = Q_AND; }
#line 1259 "grammar.c"
break;
case 50:
#line 233 "grammar.y"
	{ yyval.i = Q_AND; }
#line 1264 "grammar.c"
break;
case 51:
#line 234 "grammar.y"
	{ yyval.i = Q_ADDR1; }
#line 1269 "grammar.c"
break;
case 52:
#line 235 "grammar.y"
	{ yyval.i = Q_ADDR2; }
#line 1274 "grammar.c"
break;
case 53:
#line 236 "grammar.y"
	{ yyval.i = Q_ADDR3; }
#line 1279 "grammar.c"
break;
case 54:
#line 237 "grammar.y"
	{ yyval.i = Q_ADDR4; }
#line 1284 "grammar.c"
break;
case 55:
#line 241 "grammar.y"
	{ yyval.i = Q_HOST; }
#line 1289 "grammar.c"
break;
case 56:
#line 242 "grammar.y"
	{ yyval.i = Q_NET; }
#line 1294 "grammar.c"
break;
case 57:
#line 243 "grammar.y"
	{ yyval.i = Q_PORT; }
#line 1299 "grammar.c"
break;
case 58:
#line 246 "grammar.y"
	{ yyval.i = Q_GATEWAY; }
#line 1304 "grammar.c"
break;
case 59:
#line 248 "grammar.y"
	{ yyval.i = Q_LINK; }
#line 1309 "grammar.c"
break;
case 60:
#line 249 "grammar.y"
	{ yyval.i = Q_IP; }
#line 1314 "grammar.c"
break;
case 61:
#line 250 "grammar.y"
	{ yyval.i = Q_ARP; }
#line 1319 "grammar.c"
break;
case 62:
#line 251 "grammar.y"
	{ yyval.i = Q_RARP; }
#line 1324 "grammar.c"
break;
case 63:
#line 252 "grammar.y"
	{ yyval.i = Q_TCP; }
#line 1329 "grammar.c"
break;
case 64:
#line 253 "grammar.y"
	{ yyval.i = Q_UDP; }
#line 1334 "grammar.c"
break;
case 65:
#line 254 "grammar.y"
	{ yyval.i = Q_ICMP; }
#line 1339 "grammar.c"
break;
case 66:
#line 255 "grammar.y"
	{ yyval.i = Q_IGMP; }
#line 1344 "grammar.c"
break;
case 67:
#line 256 "grammar.y"
	{ yyval.i = Q_IGRP; }
#line 1349 "grammar.c"
break;
case 68:
#line 257 "grammar.y"
	{ yyval.i = Q_PIM; }
#line 1354 "grammar.c"
break;
case 69:
#line 258 "grammar.y"
	{ yyval.i = Q_ATALK; }
#line 1359 "grammar.c"
break;
case 70:
#line 259 "grammar.y"
	{ yyval.i = Q_DECNET; }
#line 1364 "grammar.c"
break;
case 71:
#line 260 "grammar.y"
	{ yyval.i = Q_LAT; }
#line 1369 "grammar.c"
break;
case 72:
#line 261 "grammar.y"
	{ yyval.i = Q_SCA; }
#line 1374 "grammar.c"
break;
case 73:
#line 262 "grammar.y"
	{ yyval.i = Q_MOPDL; }
#line 1379 "grammar.c"
break;
case 74:
#line 263 "grammar.y"
	{ yyval.i = Q_MOPRC; }
#line 1384 "grammar.c"
break;
case 75:
#line 264 "grammar.y"
	{ yyval.i = Q_IPV6; }
#line 1389 "grammar.c"
break;
case 76:
#line 265 "grammar.y"
	{ yyval.i = Q_ICMPV6; }
#line 1394 "grammar.c"
break;
case 77:
#line 266 "grammar.y"
	{ yyval.i = Q_AH; }
#line 1399 "grammar.c"
break;
case 78:
#line 267 "grammar.y"
	{ yyval.i = Q_ESP; }
#line 1404 "grammar.c"
break;
case 79:
#line 268 "grammar.y"
	{ yyval.i = Q_STP; }
#line 1409 "grammar.c"
break;
case 80:
#line 270 "grammar.y"
	{ yyval.rblk = gen_broadcast(yystack.l_mark[-1].i); }
#line 1414 "grammar.c"
break;
case 81:
#line 271 "grammar.y"
	{ yyval.rblk = gen_multicast(yystack.l_mark[-1].i); }
#line 1419 "grammar.c"
break;
case 82:
#line 272 "grammar.y"
	{ yyval.rblk = gen_less(yystack.l_mark[0].i); }
#line 1424 "grammar.c"
break;
case 83:
#line 273 "grammar.y"
	{ yyval.rblk = gen_greater(yystack.l_mark[0].i); }
#line 1429 "grammar.c"
break;
case 84:
#line 274 "grammar.y"
	{ yyval.rblk = gen_byteop(yystack.l_mark[-1].i, yystack.l_mark[-2].i, yystack.l_mark[0].i); }
#line 1434 "grammar.c"
break;
case 85:
#line 275 "grammar.y"
	{ yyval.rblk = gen_inbound(0); }
#line 1439 "grammar.c"
break;
case 86:
#line 276 "grammar.y"
	{ yyval.rblk = gen_inbound(1); }
#line 1444 "grammar.c"
break;
case 87:
#line 277 "grammar.y"
	{ yyval.rblk = gen_vlan(yystack.l_mark[0].i); }
#line 1449 "grammar.c"
break;
case 88:
#line 278 "grammar.y"
	{ yyval.rblk = gen_vlan(-1); }
#line 1454 "grammar.c"
break;
case 89:
#line 279 "grammar.y"
	{ yyval.rblk = gen_mpls(yystack.l_mark[0].i); }
#line 1459 "grammar.c"
break;
case 90:
#line 280 "grammar.y"
	{ yyval.rblk = gen_mpls(-1); }
#line 1464 "grammar.c"
break;
case 91:
#line 281 "grammar.y"
	{ yyval.rblk = yystack.l_mark[0].rblk; }
#line 1469 "grammar.c"
break;
case 92:
#line 282 "grammar.y"
	{ yyval.rblk = yystack.l_mark[0].rblk; }
#line 1474 "grammar.c"
break;
case 93:
#line 283 "grammar.y"
	{ yyval.rblk = gen_sample(yystack.l_mark[0].i); }
#line 1479 "grammar.c"
break;
case 94:
#line 286 "grammar.y"
	{ yyval.rblk = gen_pf_ifname(yystack.l_mark[0].s); }
#line 1484 "grammar.c"
break;
case 95:
#line 287 "grammar.y"
	{ yyval.rblk = gen_pf_ruleset(yystack.l_mark[0].s); }
#line 1489 "grammar.c"
break;
case 96:
#line 288 "grammar.y"
	{ yyval.rblk = gen_pf_rnr(yystack.l_mark[0].i); }
#line 1494 "grammar.c"
break;
case 97:
#line 289 "grammar.y"
	{ yyval.rblk = gen_pf_srnr(yystack.l_mark[0].i); }
#line 1499 "grammar.c"
break;
case 98:
#line 290 "grammar.y"
	{ yyval.rblk = gen_pf_reason(yystack.l_mark[0].i); }
#line 1504 "grammar.c"
break;
case 99:
#line 291 "grammar.y"
	{ yyval.rblk = gen_pf_action(yystack.l_mark[0].i); }
#line 1509 "grammar.c"
break;
case 100:
#line 294 "grammar.y"
	{ yyval.i = yystack.l_mark[0].i; }
#line 1514 "grammar.c"
break;
case 101:
#line 295 "grammar.y"
	{ const char *reasons[] = PFRES_NAMES;
				  int i;
				  for (i = 0; reasons[i]; i++) {
					  if (strcasecmp(yystack.l_mark[0].s, reasons[i]) == 0) {
						  yyval.i = i;
						  break;
					  }
				  }
				  if (reasons[i] == NULL)
					  bpf_error("unknown PF reason");
				}
#line 1529 "grammar.c"
break;
case 102:
#line 308 "grammar.y"
	{ if (strcasecmp(yystack.l_mark[0].s, "pass") == 0 ||
				      strcasecmp(yystack.l_mark[0].s, "accept") == 0)
					yyval.i = PF_PASS;
				  else if (strcasecmp(yystack.l_mark[0].s, "drop") == 0 ||
				      strcasecmp(yystack.l_mark[0].s, "block") == 0)
					yyval.i = PF_DROP;
				  else if (strcasecmp(yystack.l_mark[0].s, "match") == 0)
					yyval.i = PF_MATCH;
				  else if (strcasecmp(yystack.l_mark[0].s, "rdr") == 0)
				  	yyval.i = PF_RDR;
				  else if (strcasecmp(yystack.l_mark[0].s, "nat") == 0)
				  	yyval.i = PF_NAT;
				  else if (strcasecmp(yystack.l_mark[0].s, "binat") == 0)
				  	yyval.i = PF_BINAT;
				  else if (strcasecmp(yystack.l_mark[0].s, "scrub") == 0)
				  	yyval.i = PF_SCRUB;
				  else
					  bpf_error("unknown PF action");
				}
#line 1552 "grammar.c"
break;
case 103:
#line 330 "grammar.y"
	{ yyval.rblk = gen_p80211_type(yystack.l_mark[-2].i | yystack.l_mark[0].i,
					IEEE80211_FC0_TYPE_MASK |
					IEEE80211_FC0_SUBTYPE_MASK);
				}
#line 1560 "grammar.c"
break;
case 104:
#line 334 "grammar.y"
	{ yyval.rblk = gen_p80211_type(yystack.l_mark[0].i,
					IEEE80211_FC0_TYPE_MASK); }
#line 1566 "grammar.c"
break;
case 105:
#line 336 "grammar.y"
	{ yyval.rblk = gen_p80211_type(yystack.l_mark[0].i,
					IEEE80211_FC0_SUBTYPE_MASK); }
#line 1572 "grammar.c"
break;
case 106:
#line 338 "grammar.y"
	{ yyval.rblk = gen_p80211_fcdir(yystack.l_mark[0].i); }
#line 1577 "grammar.c"
break;
case 108:
#line 342 "grammar.y"
	{ if (strcasecmp(yystack.l_mark[0].s, "data") == 0)
					yyval.i = IEEE80211_FC0_TYPE_DATA;
				  else if (strcasecmp(yystack.l_mark[0].s, "mgt") == 0 ||
					strcasecmp(yystack.l_mark[0].s, "management") == 0)
					yyval.i = IEEE80211_FC0_TYPE_MGT;
				  else if (strcasecmp(yystack.l_mark[0].s, "ctl") == 0 ||
					strcasecmp(yystack.l_mark[0].s, "control") == 0)
					yyval.i = IEEE80211_FC0_TYPE_CTL;
				  else
					  bpf_error("unknown 802.11 type");
				}
#line 1592 "grammar.c"
break;
case 110:
#line 356 "grammar.y"
	{ if (strcasecmp(yystack.l_mark[0].s, "assocreq") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_ASSOC_REQ;
				  else if (strcasecmp(yystack.l_mark[0].s, "assocresp") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_ASSOC_RESP;
				  else if (strcasecmp(yystack.l_mark[0].s, "reassocreq") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_REASSOC_REQ;
				  else if (strcasecmp(yystack.l_mark[0].s, "reassocresp") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_REASSOC_RESP;
				  else if (strcasecmp(yystack.l_mark[0].s, "probereq") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_PROBE_REQ;
				  else if (strcasecmp(yystack.l_mark[0].s, "proberesp") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_PROBE_RESP;
				  else if (strcasecmp(yystack.l_mark[0].s, "beacon") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_BEACON;
				  else if (strcasecmp(yystack.l_mark[0].s, "atim") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_ATIM;
				  else if (strcasecmp(yystack.l_mark[0].s, "disassoc") == 0 ||
				      strcasecmp(yystack.l_mark[0].s, "disassociation") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_DISASSOC;
				  else if (strcasecmp(yystack.l_mark[0].s, "auth") == 0 ||
				      strcasecmp(yystack.l_mark[0].s, "authentication") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_AUTH;
				  else if (strcasecmp(yystack.l_mark[0].s, "deauth") == 0 ||
				      strcasecmp(yystack.l_mark[0].s, "deauthentication") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_DEAUTH;
				  else if (strcasecmp(yystack.l_mark[0].s, "data") == 0)
					yyval.i = IEEE80211_FC0_SUBTYPE_DATA;
				  else
					  bpf_error("unknown 802.11 subtype");
				}
#line 1626 "grammar.c"
break;
case 112:
#line 389 "grammar.y"
	{ if (strcasecmp(yystack.l_mark[0].s, "nods") == 0)
					yyval.i = IEEE80211_FC1_DIR_NODS;
				  else if (strcasecmp(yystack.l_mark[0].s, "tods") == 0)
					yyval.i = IEEE80211_FC1_DIR_TODS;
				  else if (strcasecmp(yystack.l_mark[0].s, "fromds") == 0)
					yyval.i = IEEE80211_FC1_DIR_FROMDS;
				  else if (strcasecmp(yystack.l_mark[0].s, "dstods") == 0)
					yyval.i = IEEE80211_FC1_DIR_DSTODS;
				  else
					bpf_error("unknown 802.11 direction");
				}
#line 1641 "grammar.c"
break;
case 113:
#line 402 "grammar.y"
	{ yyval.i = BPF_JGT; }
#line 1646 "grammar.c"
break;
case 114:
#line 403 "grammar.y"
	{ yyval.i = BPF_JGE; }
#line 1651 "grammar.c"
break;
case 115:
#line 404 "grammar.y"
	{ yyval.i = BPF_JEQ; }
#line 1656 "grammar.c"
break;
case 116:
#line 406 "grammar.y"
	{ yyval.i = BPF_JGT; }
#line 1661 "grammar.c"
break;
case 117:
#line 407 "grammar.y"
	{ yyval.i = BPF_JGE; }
#line 1666 "grammar.c"
break;
case 118:
#line 408 "grammar.y"
	{ yyval.i = BPF_JEQ; }
#line 1671 "grammar.c"
break;
case 119:
#line 410 "grammar.y"
	{ yyval.a = gen_loadi(yystack.l_mark[0].i); }
#line 1676 "grammar.c"
break;
case 121:
#line 413 "grammar.y"
	{ yyval.a = gen_load(yystack.l_mark[-3].i, yystack.l_mark[-1].a, 1); }
#line 1681 "grammar.c"
break;
case 122:
#line 414 "grammar.y"
	{ yyval.a = gen_load(yystack.l_mark[-5].i, yystack.l_mark[-3].a, yystack.l_mark[-1].i); }
#line 1686 "grammar.c"
break;
case 123:
#line 415 "grammar.y"
	{ yyval.a = gen_arth(BPF_ADD, yystack.l_mark[-2].a, yystack.l_mark[0].a); }
#line 1691 "grammar.c"
break;
case 124:
#line 416 "grammar.y"
	{ yyval.a = gen_arth(BPF_SUB, yystack.l_mark[-2].a, yystack.l_mark[0].a); }
#line 1696 "grammar.c"
break;
case 125:
#line 417 "grammar.y"
	{ yyval.a = gen_arth(BPF_MUL, yystack.l_mark[-2].a, yystack.l_mark[0].a); }
#line 1701 "grammar.c"
break;
case 126:
#line 418 "grammar.y"
	{ yyval.a = gen_arth(BPF_DIV, yystack.l_mark[-2].a, yystack.l_mark[0].a); }
#line 1706 "grammar.c"
break;
case 127:
#line 419 "grammar.y"
	{ yyval.a = gen_arth(BPF_AND, yystack.l_mark[-2].a, yystack.l_mark[0].a); }
#line 1711 "grammar.c"
break;
case 128:
#line 420 "grammar.y"
	{ yyval.a = gen_arth(BPF_OR, yystack.l_mark[-2].a, yystack.l_mark[0].a); }
#line 1716 "grammar.c"
break;
case 129:
#line 421 "grammar.y"
	{ yyval.a = gen_arth(BPF_LSH, yystack.l_mark[-2].a, yystack.l_mark[0].a); }
#line 1721 "grammar.c"
break;
case 130:
#line 422 "grammar.y"
	{ yyval.a = gen_arth(BPF_RSH, yystack.l_mark[-2].a, yystack.l_mark[0].a); }
#line 1726 "grammar.c"
break;
case 131:
#line 423 "grammar.y"
	{ yyval.a = gen_neg(yystack.l_mark[0].a); }
#line 1731 "grammar.c"
break;
case 132:
#line 424 "grammar.y"
	{ yyval.a = yystack.l_mark[-1].a; }
#line 1736 "grammar.c"
break;
case 133:
#line 425 "grammar.y"
	{ yyval.a = gen_loadlen(); }
#line 1741 "grammar.c"
break;
case 134:
#line 426 "grammar.y"
	{ yyval.a = gen_loadrnd(); }
#line 1746 "grammar.c"
break;
case 135:
#line 428 "grammar.y"
	{ yyval.i = '&'; }
#line 1751 "grammar.c"
break;
case 136:
#line 429 "grammar.y"
	{ yyval.i = '|'; }
#line 1756 "grammar.c"
break;
case 137:
#line 430 "grammar.y"
	{ yyval.i = '<'; }
#line 1761 "grammar.c"
break;
case 138:
#line 431 "grammar.y"
	{ yyval.i = '>'; }
#line 1766 "grammar.c"
break;
case 139:
#line 432 "grammar.y"
	{ yyval.i = '='; }
#line 1771 "grammar.c"
break;
case 141:
#line 435 "grammar.y"
	{ yyval.i = yystack.l_mark[-1].i; }
#line 1776 "grammar.c"
break;
#line 1778 "grammar.c"
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
