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
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
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
extern YYSTYPE pcap_yylval;
