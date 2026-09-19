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
#define yyparse    mibparse
#endif /* yyparse */

#ifndef yylex
#define yylex      miblex
#endif /* yylex */

#ifndef yyerror
#define yyerror    miberror
#endif /* yyerror */

#ifndef yychar
#define yychar     mibchar
#endif /* yychar */

#ifndef yyval
#define yyval      mibval
#endif /* yyval */

#ifndef yylval
#define yylval     miblval
#endif /* yylval */

#ifndef yydebug
#define yydebug    mibdebug
#endif /* yydebug */

#ifndef yynerrs
#define yynerrs    mibnerrs
#endif /* yynerrs */

#ifndef yyerrflag
#define yyerrflag  miberrflag
#endif /* yyerrflag */

#ifndef yylhs
#define yylhs      miblhs
#endif /* yylhs */

#ifndef yylen
#define yylen      miblen
#endif /* yylen */

#ifndef yydefred
#define yydefred   mibdefred
#endif /* yydefred */

#ifndef yydgoto
#define yydgoto    mibdgoto
#endif /* yydgoto */

#ifndef yysindex
#define yysindex   mibsindex
#endif /* yysindex */

#ifndef yyrindex
#define yyrindex   mibrindex
#endif /* yyrindex */

#ifndef yygindex
#define yygindex   mibgindex
#endif /* yygindex */

#ifndef yytable
#define yytable    mibtable
#endif /* yytable */

#ifndef yycheck
#define yycheck    mibcheck
#endif /* yycheck */

#ifndef yyname
#define yyname     mibname
#endif /* yyname */

#ifndef yyrule
#define yyrule     mibrule
#endif /* yyrule */
#define YYPREFIX "mib"

#define YYPURE 0

#line 20 "mib.y"

#include <sys/tree.h>

#include <assert.h>
#include <ber.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>
#include <time.h>

#include "log.h"
#include "mib.h"

/* RFC2578 section 3.1 */
#define DESCRIPTOR_MAX 64

/* Values from real life testing, could be adjusted */
#define ITEM_MAX DESCRIPTOR_MAX
#define MODULENAME_MAX 64
#define SYMBOLS_MAX 256
#define IMPORTS_MAX 16
#define TEXT_MAX 16384

#ifndef nitems
#define nitems(_a) (sizeof((_a)) / sizeof((_a)[0]))
#endif

struct objidcomponent {
	enum {
		OCT_DESCRIPTOR,
		OCT_NUMBER,
		OCT_NAMEANDNUMBER
	}			 type;
	uint32_t		 number;
	char			 name[DESCRIPTOR_MAX + 1];
};

struct oid_unresolved {
	/* Unusual to have long lists of unresolved components */
	struct objidcomponent	 bo_id[16];
	size_t			 bo_n;
};

struct oid_resolved {
	uint32_t		*bo_id;
	size_t			 bo_n;
};

enum status {
	CURRENT,
	DEPRECATED,
	OBSOLETE
};

enum access {
	NOTACCESSIBLE,
	ACCESSIBLEFORNOTIFY,
	READONLY,
	READWRITE,
	READCREATE
};

struct objectidentity {
	enum status		 status;
	char			*description;
	char			*reference;
};

struct objecttype {
	void			*syntax;
	char			*units;
	enum access		 maxaccess;
	enum status		 status;
	char			*description;
	char			*reference;
	void			*index;
	void			*defval;
};

struct notificationtype {
	void			*objects;
	enum status		 status;
	char			*description;
	char			*reference;
};

struct textualconvention {
	char			*displayhint;
	enum status		 status;
	char			*description;
	char			*reference;
	void			*syntax;
};

struct item {
	char			 name[DESCRIPTOR_MAX + 1];
	enum item_type {
		IT_OID,
		IT_MACRO,
		IT_MODULE_IDENTITY,
		IT_OBJECT_IDENTITY,
		IT_APPLICATIONSYNTAX,
		IT_OBJECT_TYPE,
		IT_NOTIFICATION_TYPE,
		IT_TEXTUAL_CONVENTION,
		IT_OBJECT_GROUP,
		IT_NOTIFICATION_GROUP,
		IT_MODULE_COMPLIANCE,
		IT_AGENT_CAPABITIES
	}			 type;
	int			 resolved;
	struct module		*module;

	union {
		struct oid_unresolved	*oid_unresolved;
		struct oid_resolved	 oid;
	};

	union {
		struct objectidentity	 objectidentity;
		struct objecttype	 objecttype;
		struct notificationtype	 notificationtype;
		struct textualconvention textualconvention;
	};

	/* Global case insensitive */
	RB_ENTRY(item)		 entrygci;
	/* Module case insensitive */
	RB_ENTRY(item)		 entryci;
	/* Module case sensitive */
	RB_ENTRY(item)		 entrycs;
	/* Global oid */
	RB_ENTRY(item)		 entry;
};

struct import_symbol {
	char			 name[DESCRIPTOR_MAX + 1];
	struct item		*item;
};

struct import {
	char			 name[MODULENAME_MAX + 1];
	struct module		*module;
	struct import_symbol	 symbols[SYMBOLS_MAX];
	size_t			 nsymbols;
};

static struct module {
	char			 name[MODULENAME_MAX + 1];
	int8_t			 resolved;

	time_t			 lastupdated;

	struct import		*imports;

	RB_HEAD(itemscs, item) itemscs;
	RB_HEAD(itemsci, item) itemsci;

	RB_ENTRY(module) entryci;
	RB_ENTRY(module) entrycs;
} *module;

int		 yylex(void);
static void	 yyerror(const char *, ...)
    __attribute__((__format__ (printf, 1, 2)));
void		 mib_defaults(void);
void		 mib_modulefree(struct module *);
int		 mib_imports_add(char *, char **);
int		 mib_oid_append(struct oid_unresolved *,
		    const struct objidcomponent *);
int		 mib_macro(const char *);
int		 mib_oid_concat(struct oid_unresolved *,
		    const struct oid_unresolved *);
struct item	*mib_item(const char *, enum item_type);
int		 mib_item_oid(struct item *,
		    const struct oid_unresolved *);
int		 mib_macro(const char *);
int		 mib_applicationsyntax(const char *);
struct item	*mib_oid(const char *, const struct oid_unresolved *);
int		 mib_moduleidentity(const char *, time_t, const char *,
		    const char *, const char *, const struct oid_unresolved *);
int		 mib_objectidentity(const char *, enum status, const char *,
		    const char *, const struct oid_unresolved *);
int		 mib_objecttype(const char *, void  *, const char *,
		    enum access, enum status, const char *, const char *,
		    void *, void *, const struct oid_unresolved *);
int		 mib_notificationtype(const char *, void *, enum status,
		    const char *, const char *, const struct oid_unresolved *);
int		 mib_textualconvetion(const char *, const char *, enum status,
		    const char *, const char *, void *);
int		 mib_objectgroup(const char *, void *, enum status,
		    const char *, const char *, const struct oid_unresolved *);
int		 mib_notificationgroup(const char *, void *, enum status,
		    const char *, const char *, const struct oid_unresolved *);
int		 mib_modulecompliance(const char *, enum status, const char *,
		    const char *, void *, const struct oid_unresolved *);
struct item	*mib_item_find(struct item *, const char *);
struct item	*mib_item_parent(struct ber_oid *);
int		  mib_resolve_oid(struct oid_resolved *,
		    struct oid_unresolved *, struct item *);
int		 mib_resolve_item(struct item *);
int		 mib_resolve_module(struct module *);
int		 module_cmp_cs(struct module *, struct module *);
int		 module_cmp_ci(struct module *, struct module *);
int		 item_cmp_cs(struct item *, struct item *);
int		 item_cmp_ci(struct item *, struct item *);
int		 item_cmp_oid(struct item *, struct item *);

RB_HEAD(modulesci, module) modulesci = RB_INITIALIZER(&modulesci);
RB_HEAD(modulescs, module) modulescs = RB_INITIALIZER(&modulescs);
RB_HEAD(items, item) items = RB_INITIALIZER(&items);
RB_HEAD(itemsgci, item) itemsci = RB_INITIALIZER(&itemsci);
/*
 * Use case sensitive matching internally (for resolving IMPORTS) and
 * case sensitive matching, followed by case insensitive matching
 * for end-user resolving (e.g. mib_string2oid()).
 * It shouldn't happen there's case-based overlap in module/item names,
 * but allow all to be resolved in case there is.
 */
RB_PROTOTYPE_STATIC(modulesci, module, entryci, module_cmp_ci);
RB_PROTOTYPE_STATIC(modulescs, module, entrycs, module_cmp_cs);
/*
 * mib_string2oid() should match case insensitive on:
 * <module>::<descriptor>
 * <descriptor>
 */
RB_PROTOTYPE_STATIC(itemsgci, item, entrygci, item_cmp_ci);
RB_PROTOTYPE_STATIC(itemsci, item, entryci, item_cmp_ci);
RB_PROTOTYPE_STATIC(itemscs, item, entrycs, item_cmp_cs);
RB_PROTOTYPE_STATIC(items, item, entry, item_cmp_oid);

struct file {
	FILE		*stream;
	const char	*name;
	size_t		 lineno;
	enum {
		FILE_UNDEFINED,
		FILE_ASN1,
		FILE_SMI2
	}		 state;
} file;

typedef union {
	char			 string[TEXT_MAX];
	unsigned long long	 number;
	long long		 signednumber;
	char			 symbollist[SYMBOLS_MAX][DESCRIPTOR_MAX + 1];
	struct objidcomponent	 objidcomponent;
	struct oid_unresolved	 oid;
	time_t			 time;
	enum status		 status;
	enum access		 access;
} YYSTYPE;

#line 362 "mib.c"

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

#define ERROR 257
#define HSTRING 258
#define BSTRING 259
#define ABSENT 260
#define ACCESS 261
#define AGENTCAPABILITIES 262
#define ANY 263
#define APPLICATION 264
#define AUGMENTS 265
#define BEGIN 266
#define BIT 267
#define BITS 268
#define BOOLEAN 269
#define BY 270
#define CHOICE 271
#define COMPONENT 272
#define COMPONENTS 273
#define CONTACTINFO 274
#define CREATIONREQUIRES 275
#define Counter32 276
#define Counter64 277
#define DEFAULT 278
#define DEFINED 279
#define DEFINITIONS 280
#define DEFVAL 281
#define DESCRIPTION 282
#define DISPLAYHINT 283
#define END 284
#define ENUMERATED 285
#define ENTERPRISE 286
#define EXPLICIT 287
#define EXPORTS 288
#define EXTERNAL 289
#define FALSE 290
#define FROM 291
#define GROUP 292
#define Gauge32 293
#define IDENTIFIER 294
#define IMPLICIT 295
#define IMPLIED 296
#define IMPORTS 297
#define INCLUDES 298
#define INDEX 299
#define INTEGER 300
#define Integer32 301
#define IpAddress 302
#define LASTUPDATED 303
#define MANDATORYGROUPS 304
#define MAX 305
#define MAXACCESS 306
#define MIN 307
#define MINACCESS 308
#define MINUSINFINITY 309
#define MODULE 310
#define MODULECOMPLIANCE 311
#define MODULEIDENTITY 312
#define NOTIFICATIONGROUP 313
#define NOTIFICATIONTYPE 314
#define NOTIFICATIONS 315
#define ASNNULL 316
#define OBJECT 317
#define OBJECTGROUP 318
#define OBJECTIDENTITY 319
#define OBJECTTYPE 320
#define OBJECTS 321
#define OCTET 322
#define OF 323
#define OPTIONAL 324
#define ORGANIZATION 325
#define Opaque 326
#define PLUSINFINITY 327
#define PRESENT 328
#define PRIVATE 329
#define PRODUCTRELEASE 330
#define REAL 331
#define REFERENCE 332
#define REVISION 333
#define SEQUENCE 334
#define SET 335
#define SIZE 336
#define STATUS 337
#define STRING 338
#define SUPPORTS 339
#define SYNTAX 340
#define TAGS 341
#define TEXTUALCONVENTION 342
#define TRAPTYPE 343
#define TRUE 344
#define TimeTicks 345
#define UNITS 346
#define UNIVERSAL 347
#define Unsigned32 348
#define VARIABLES 349
#define VARIATION 350
#define WITH 351
#define WRITESYNTAX 352
#define SNMPv2SMI 353
#define SNMPv2CONF 354
#define SNMPv2TC 355
#define PRODUCTION 356
#define RANGESEPARATOR 357
#define typereference 358
#define identifier 359
#define TEXT 360
#define NUMBER 361
#define SIGNEDNUMBER 362
#define YYERRCODE 256
typedef int YYINT;
static const YYINT miblhs[] = {                          -1,
    0,    0,   19,   17,   17,    1,    2,    2,    2,   21,
   18,   23,   23,    3,    3,    4,    4,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    5,   20,   14,   24,
   24,   25,   25,   26,   22,   22,   27,   27,   27,   27,
   27,   27,   27,   27,   27,   27,   34,   35,   35,   36,
   31,   31,   37,   37,   38,   28,   28,   28,   28,   28,
   28,   13,   13,   12,   12,   29,   29,   29,   44,   44,
   45,   45,   46,   30,   30,   47,   47,   47,   47,   47,
   47,   47,   48,   49,   49,   49,   50,   11,   11,   32,
   32,   51,   33,   52,   52,   53,   54,   54,   55,   55,
   57,   57,   58,   58,   56,   56,   59,   59,   60,   60,
   61,   62,   64,   64,   63,   63,   65,   65,   66,   66,
   67,   67,   10,    9,    9,    7,    7,    8,    8,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   39,   40,   40,   40,   40,   40,   41,   41,   41,
   43,   69,   69,   70,   42,   42,   71,   71,   72,   72,
   68,   68,   73,   73,   74,   74,   74,   74,   16,   15,
};
static const YYINT miblen[] = {                           2,
    0,    2,    0,    9,    1,    1,    1,    1,    1,    1,
    3,    2,    0,    3,    3,    3,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,   12,    2,    1,
    0,    1,    2,    4,    2,    0,    5,    9,   16,   10,
    6,   11,   10,   10,   10,    3,    4,    1,    3,    1,
    4,    0,    1,    3,    1,    1,    3,    1,    1,    1,
    1,    2,    0,    2,    0,    4,    4,    0,    1,    3,
    2,    1,    1,    4,    0,    1,    1,    1,    1,    1,
    1,    1,    3,    3,    1,    0,    1,    2,    0,    3,
    1,    2,    1,    1,    2,    4,    1,    0,    4,    0,
    1,    3,    1,    1,    1,    0,    1,    2,    1,    1,
    4,    7,    1,    1,    1,    1,    2,    0,    2,    0,
    2,    0,    5,    2,    0,    1,    1,    1,    4,    1,
    1,    2,    2,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    4,    4,    4,    4,    4,    8,    7,    7,
    4,    1,    3,    4,    4,    4,    1,    3,    4,    4,
    3,    1,    1,    3,    1,    1,    1,    1,    1,    1,
};
static const YYINT mibdefred[] = {                        1,
    0,    7,    8,    9,    6,    0,    5,    2,    0,    0,
    3,    0,   13,    0,    0,   37,    0,    0,   36,   24,
   29,   25,   22,   23,   35,   20,   34,   31,   33,   21,
   30,   28,   32,   27,   26,   18,   11,   12,    0,   19,
   17,    0,    0,    0,    0,   10,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    4,
   45,   14,   15,   16,   39,    0,    0,  146,  151,    0,
    0,    0,  145,    0,    0,    0,    0,    0,  149,    0,
    0,   56,   66,   68,   69,   70,   71,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  142,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  180,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  162,  177,  178,  176,
  175,    0,    0,    0,    0,    0,  167,    0,    0,    0,
    0,   67,    0,    0,    0,   98,    0,    0,    0,    0,
    0,    0,   60,    0,    0,    0,   65,    0,   63,    0,
    0,   47,    0,    0,    0,    0,    0,    0,    0,  161,
  156,    0,    0,    0,    0,  165,  153,  154,    0,    0,
  102,   51,    0,    0,  155,  166,    0,  157,    0,   57,
    0,    0,    0,   61,    0,  138,    0,    0,  137,    0,
    0,   72,    0,    0,    0,  163,  171,  174,    0,    0,
  168,    0,    0,  100,    0,    0,    0,    0,   59,    0,
   64,    0,    0,    0,    0,    0,    0,  179,    0,    0,
  164,  169,  170,    0,    0,    0,    0,   74,    0,    0,
    0,  104,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   42,    0,  159,    0,  160,  107,    0,
    0,  105,    0,    0,  139,  134,  133,    0,   48,    0,
    0,    0,   43,  158,    0,    0,    0,   53,   55,   50,
   54,    0,    0,   38,   52,    0,    0,    0,  106,    0,
  117,  119,  120,    0,   44,  114,  113,    0,  111,  126,
  125,    0,  124,  123,    0,  118,    0,    0,  109,    0,
    0,    0,    0,    0,    0,  112,  121,  127,    0,    0,
    0,    0,    0,    0,  129,    0,    0,    0,    0,   83,
    0,   79,   82,    0,    0,  131,    0,   77,   81,    0,
   76,   89,   90,   87,   88,    0,   86,   92,    0,   91,
   49,  122,   80,    0,    0,   95,   84,    0,   93,   97,
   94,
};
static const YYINT mibdgoto[] = {                         1,
    6,    7,   38,   39,  113,   41,  198,  199,  247,  297,
  109,  218,  166,   51,  114,  229,    8,   14,   12,   18,
   45,   46,   15,  252,  253,  254,   47,   82,  315,  324,
   92,  144,  240,   90,  154,  155,  158,  159,   83,   84,
   85,   86,   87,  331,  332,  333,  349,  350,  355,  356,
  145,  241,  242,  260,  277,  289,  298,  299,  290,  291,
  292,  293,  302,  305,  312,  320,  327,  132,  126,  127,
  136,  137,  133,  134,
};
static const YYINT mibsindex[] = {                        0,
 -157,    0,    0,    0,    0, -219,    0,    0, -304, -192,
    0, -216,    0, -273,   86,    0, -218, -261,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  -36,    0,
    0, -215, -257,  -94, -179,    0, -261, -157,  139, -250,
 -209, -127, -231, -185, -190, -159, -190, -200, -201,    0,
    0,    0,    0,    0,    0, -220,   25,    0,    0,  111,
  -23,  112,    0, -141, -183,  119, -118, -123,    0,  122,
  -15,    0,    0,    0,    0,    0,    0, -273,   40, -173,
   45, -166, -184, -161, -273,  146,  -96, -176, -237, -174,
 -237, -237,    0,  142, -149, -170, -273, -169, -144, -237,
 -174, -240,    0,  -88, -273, -273, -273, -273,   77, -273,
  -78, -120, -140, -152,  169,  -34,    0,    0,    0,    0,
    0,  171,   89, -147,  174,  -10,    0,  175,  188, -103,
  194,    0,  146,  110,  192,    0, -273,  196,    2,  198,
  199, -121,    0,  117,  200,  -32,    0,    7,    0,  -31,
 -288,    0,  -29, -104, -102,  -49,  -22,  -99, -176,    0,
    0, -237, -237, -293, -174,    0,    0,    0,  219, -237,
    0,    0, -273,  -18,    0,    0, -237,    0,  -62,    0,
 -273,  -85, -273,    0,  -77,    0,  241, -288,    0,  -76,
  -62,    0, -273,  -74,  247,    0,    0,    0,  248,  250,
    0, -237,  255,    0,  -63,  257,  -61,   -8,    0,  -62,
    0,  -62,  -52,  241, -288,  -62,  -56,    0,  -27,  -20,
    0,    0,    0,  273,  278,  -62,  283,    0,  -24,   -4,
   -8,    0,   -1,    3,  286, -288,  210,    4,   77, -273,
    1,    8,  -20,    0,  299,    0,   26,    0,    0,   54,
   77,    0,   77,   77,    0,    0,    0,   77,    0,   83,
   85,   77,    0,    0,  146,  245, -232,    0,    0,    0,
    0,   10,   12,    0,    0, -110, -110, -110,    0, -232,
    0,    0,    0,  -62,    0,    0,    0,    9,    0,    0,
    0,   87,    0,    0,   31,    0, -224, -110,    0,   14,
  146,   24,  254,  258,   97,    0,    0,    0,  146,   72,
 -273, -267,  259,   27,    0, -273,  102,  260, -273,    0,
   11,    0,    0, -114,   77,    0,   29,    0,    0, -267,
    0,    0,    0,    0,    0, -282,    0,    0,  261,    0,
    0,    0,    0,    0,   13,    0,    0,   32,    0,    0,
    0,
};
static const YYINT mibrindex[] = {                        0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  106,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  106,    0,    0,    0,
    0,    0,    0,    0,   55,    0,   55,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  -41,    0,    0,  -30,
  -21,  -16,    0,    0,    0,  -13,    0,   56,    0,   -5,
   -2,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   67,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   88,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  354,  -11,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  271,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  277,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   93,    0,
    0,    0,    0,    0,    0,    0, -281,    0,    0,    0,
   51,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   51,
    0,   51,    0,    0,  284,   51,    0,    0,    0,   52,
    0,    0,    0,    0,    0,   70,    0,    0, -228,    0,
   61,    0,    0,    0,    0,  284,    0,    0,    0,    0,
    0,    0,   62,    0,    0,    0,    0,    0,    0, -227,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -290,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -284,
    0,    0,    0, -249,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0, -238,    0, -254,    0,    0,    0,
    0, -245,    0,    0,   64,    0,    0,    0,    0,  129,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   18,    0,    0,    0,    0,
    0,    0,    0,  -38,    0,    0,    0,    0,    0,    0,
    0,
};
static const YYINT mibgindex[] = {                        0,
  -37,  373,    0,    0,  -14,  375,    0,  -79,  181, -107,
    0, -177,    0,    0,  -80,  103,    0,    0,    0,    0,
    0,  383,    0,    0,    0,  180,    0,  -89,    0,    0,
  378,  253,    0,    0,  246,    0,    0,  249,    0,    0,
    0,    0,    0,    0,   98,  114,    0,    0,    0,   91,
    0,    0,  213,    0,    0,    0,    0,  147,    0,  166,
    0,    0,    0,    0,    0,    0,    0,  -54,    0,  291,
  350,  287,    0,  293,
};
#define YYTABLESIZE 504
static const YYINT mibtable[] = {                        17,
   40,   37,  152,   44,  107,   97,  123,   49,  346,  169,
   62,  162,  161,  147,  121,   75,  101,  128,  129,  116,
  128,  129,  141,  227,  112,  115,   78,  144,  329,  173,
  150,   75,   44,  175,   40,  156,  130,  160,  148,  163,
  313,  140,  243,  128,  244,  175,  138,  139,  248,   75,
  193,   10,  308,  181,  340,  148,  358,  151,  257,  287,
    9,   96,  130,  108,  110,  116,  184,  209,  210,  128,
   16,  115,  196,   11,  314,  108,  354,  136,  196,  136,
   13,  108,  110,  152,  288,   16,   97,   50,  108,  110,
  170,   16,  143,   42,  147,  150,   43,   16,   52,  100,
  153,   78,  157,  141,   60,   88,   75,  111,  144,   65,
  143,  150,  173,  128,  176,   66,  307,  207,  225,  148,
  130,  131,  140,  130,  131,  213,  186,  108,  110,   89,
   91,  194,  216,  309,   93,  341,   95,  359,   96,   97,
   67,  269,   96,  342,   37,  246,  197,   98,   68,   69,
   99,  102,  103,  278,  104,  279,  280,  234,  105,  108,
  281,  110,  115,  116,  284,   70,  246,  117,  143,  270,
  118,  119,   71,   72,   73,  120,  153,  124,  157,  301,
  304,  140,  125,  224,  135,  285,  141,  142,  228,   74,
  146,  143,  147,  152,   75,    2,    3,    4,   76,  161,
    5,  259,  106,  164,  106,  165,   77,  167,  168,  173,
  224,  171,  172,  174,   78,  177,   53,   79,   54,   55,
   80,  318,   56,   57,   58,   59,  348,  351,  178,  325,
   81,  224,  179,  180,  182,  183,  185,  187,  189,  188,
  152,  190,  152,  191,   16,  343,  344,  345,   16,  192,
  195,  147,  200,  147,   48,  201,  203,  202,  212,  204,
  141,  205,  141,  215,  152,  144,  152,  144,  150,  217,
  150,  296,  300,  303,  220,  147,  148,  147,  148,  140,
  223,  140,  222,  226,  141,  230,  141,  231,  232,  144,
  233,  144,  150,  296,  150,  235,  236,  237,  238,  249,
  148,  239,  148,  140,  152,  140,  328,  330,  245,  250,
  152,  228,  251,  255,  330,  147,  152,  152,  256,  347,
   37,  147,   37,  258,  141,  330,  265,  147,  147,  144,
  141,  197,  150,    5,  267,  144,  141,  141,  150,  274,
  148,  144,  144,  140,  150,  150,  148,   19,  143,  140,
  143,  261,  148,  148,  263,  140,  140,  276,  264,  268,
  271,   20,   21,  272,  282,  275,  283,  286,  310,  294,
  311,  295,  143,  317,  143,  319,  321,  323,   22,  326,
  322,  334,  335,  337,  338,  357,   23,   24,  352,   46,
  360,   62,   99,   73,  172,  101,   25,   26,   27,   28,
   19,   58,   75,   29,   30,   31,   75,   41,  135,   75,
  132,   32,  143,   67,   20,   21,  103,   40,  143,   85,
   63,   68,   69,   64,  143,  143,  266,   33,  336,   61,
   34,   22,  273,   35,   94,  214,  219,  353,   70,   23,
   24,  221,  339,   36,   16,   71,   72,   73,  361,   25,
   26,   27,   28,  262,  316,  306,   29,   30,   31,  206,
  149,  211,   74,    0,   32,  208,    0,   75,    0,    0,
    0,   76,    0,    0,    0,    0,    0,    0,    0,  122,
   33,    0,    0,   34,    0,    0,   35,    0,    0,    0,
   79,    0,    0,   80,    0,    0,   36,   16,    0,    0,
    0,    0,    0,   81,
};
static const YYINT mibcheck[] = {                        14,
   15,   40,   44,   18,  123,   44,   96,   44,  123,   44,
   48,  119,  123,   44,   95,  265,   40,  258,  259,  310,
  258,  259,   44,  201,   40,  310,  281,   44,  296,   41,
   44,  281,   47,   44,   49,  116,  282,  118,   44,  120,
  265,   44,  220,  282,  222,   44,  101,  102,  226,  299,
   44,  356,   44,  143,   44,  110,   44,  112,  236,  292,
  280,   44,  308,  292,  292,  356,  147,  361,  362,  308,
  359,  356,  361,  266,  299,  304,  359,  359,  361,  361,
  297,  310,  310,  125,  317,  359,  125,  303,  317,  317,
  125,  359,  107,  312,  125,  336,  358,  359,  356,  123,
  115,  356,  117,  125,  284,  337,  356,  123,  125,  360,
   44,  125,  124,  352,  125,  325,  294,  172,  198,  125,
  361,  362,  125,  361,  362,  180,  125,  356,  356,  315,
  321,  125,  187,  125,  294,  125,  337,  125,  340,  360,
  268,  249,  125,  258,   59,  225,  161,  123,  276,  277,
   40,   40,  294,  261,  338,  263,  264,  212,   40,  283,
  268,   40,  123,  337,  272,  293,  246,  123,  183,  250,
  337,  356,  300,  301,  302,  337,  191,  274,  193,  287,
  288,   40,  359,  198,  359,  275,  336,  358,  203,  317,
  360,  125,  337,  282,  322,  353,  354,  355,  326,  123,
  358,  239,  323,  282,  323,  346,  334,  360,   40,  357,
  225,   41,  124,   40,  342,   41,  311,  345,  313,  314,
  348,  311,  317,  318,  319,  320,  334,  335,   41,  319,
  358,  246,  336,   40,  125,   44,   41,   40,  360,   41,
  282,  125,  284,   44,  359,  360,  361,  362,  359,  282,
  282,  282,  282,  284,  291,  360,  306,  360,   40,  282,
  282,  361,  284,  282,  306,  282,  308,  284,  282,  332,
  284,  286,  287,  288,  360,  306,  282,  308,  284,  282,
   40,  284,  360,  360,  306,  360,  308,   41,   41,  306,
   41,  308,  306,  308,  308,   41,  360,   41,  360,  356,
  306,  310,  308,  306,  346,  308,  321,  322,  361,  337,
  352,  326,  333,   41,  329,  346,  358,  359,   41,  334,
  359,  352,  361,   41,  346,  340,   41,  358,  359,  346,
  352,  346,  346,  358,  125,  352,  358,  359,  352,   41,
  346,  358,  359,  346,  358,  359,  352,  262,  282,  352,
  284,  356,  358,  359,  356,  358,  359,  304,  356,  356,
  360,  276,  277,  356,  282,  340,  282,  123,  282,  360,
  340,  360,  306,  360,  308,  352,  123,  281,  293,  308,
  123,  123,  356,  282,  125,  125,  301,  302,  360,  284,
  359,  337,  337,  306,   41,  125,  311,  312,  313,  314,
  262,  125,  310,  318,  319,  320,  356,  356,  125,  340,
  282,  326,  346,  268,  276,  277,  356,  356,  352,  356,
   48,  276,  277,   49,  358,  359,  246,  342,  326,   47,
  345,  293,  253,  348,   57,  183,  191,  340,  293,  301,
  302,  193,  329,  358,  359,  300,  301,  302,  358,  311,
  312,  313,  314,  241,  308,  290,  318,  319,  320,  169,
  111,  175,  317,   -1,  326,  173,   -1,  322,   -1,   -1,
   -1,  326,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  334,
  342,   -1,   -1,  345,   -1,   -1,  348,   -1,   -1,   -1,
  345,   -1,   -1,  348,   -1,   -1,  358,  359,   -1,   -1,
   -1,   -1,   -1,  358,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 362
#define YYUNDFTOKEN 439
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const mibname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,0,"','",0,0,0,0,0,0,0,0,0,0,0,0,0,0,"';'",0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'","'|'","'}'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"ERROR","HSTRING","BSTRING","ABSENT","ACCESS","AGENTCAPABILITIES","ANY",
"APPLICATION","AUGMENTS","BEGIN","BIT","BITS","BOOLEAN","BY","CHOICE",
"COMPONENT","COMPONENTS","CONTACTINFO","CREATIONREQUIRES","Counter32",
"Counter64","DEFAULT","DEFINED","DEFINITIONS","DEFVAL","DESCRIPTION",
"DISPLAYHINT","END","ENUMERATED","ENTERPRISE","EXPLICIT","EXPORTS","EXTERNAL",
"FALSE","FROM","GROUP","Gauge32","IDENTIFIER","IMPLICIT","IMPLIED","IMPORTS",
"INCLUDES","INDEX","INTEGER","Integer32","IpAddress","LASTUPDATED",
"MANDATORYGROUPS","MAX","MAXACCESS","MIN","MINACCESS","MINUSINFINITY","MODULE",
"MODULECOMPLIANCE","MODULEIDENTITY","NOTIFICATIONGROUP","NOTIFICATIONTYPE",
"NOTIFICATIONS","ASNNULL","OBJECT","OBJECTGROUP","OBJECTIDENTITY","OBJECTTYPE",
"OBJECTS","OCTET","OF","OPTIONAL","ORGANIZATION","Opaque","PLUSINFINITY",
"PRESENT","PRIVATE","PRODUCTRELEASE","REAL","REFERENCE","REVISION","SEQUENCE",
"SET","SIZE","STATUS","STRING","SUPPORTS","SYNTAX","TAGS","TEXTUALCONVENTION",
"TRAPTYPE","TRUE","TimeTicks","UNITS","UNIVERSAL","Unsigned32","VARIABLES",
"VARIATION","WITH","WRITESYNTAX","SNMPv2SMI","SNMPv2CONF","SNMPv2TC",
"PRODUCTION","RANGESEPARATOR","typereference","identifier","TEXT","NUMBER",
"SIGNEDNUMBER",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,"illegal-symbol",
};
static const char *const mibrule[] = {
"$accept : grammar",
"grammar :",
"grammar : grammar module",
"$$1 :",
"module : moduleidentifier DEFINITIONS PRODUCTION BEGIN $$1 imports moduleidentity modulebody END",
"module : smiv2moduleidentifier",
"moduleidentifier : typereference",
"smiv2moduleidentifier : SNMPv2SMI",
"smiv2moduleidentifier : SNMPv2CONF",
"smiv2moduleidentifier : SNMPv2TC",
"modulebody : assignmentlist",
"imports : IMPORTS importlist ';'",
"importlist : importlist symbolsfrom",
"importlist :",
"symbolsfrom : symbollist FROM moduleidentifier",
"symbolsfrom : symbollist FROM smiv2moduleidentifier",
"symbollist : symbollist ',' symbol",
"symbollist : symbol",
"symbol : typereference",
"symbol : descriptor",
"symbol : MODULEIDENTITY",
"symbol : OBJECTIDENTITY",
"symbol : Integer32",
"symbol : IpAddress",
"symbol : Counter32",
"symbol : Gauge32",
"symbol : Unsigned32",
"symbol : TimeTicks",
"symbol : Opaque",
"symbol : Counter64",
"symbol : OBJECTTYPE",
"symbol : NOTIFICATIONTYPE",
"symbol : TEXTUALCONVENTION",
"symbol : OBJECTGROUP",
"symbol : NOTIFICATIONGROUP",
"symbol : MODULECOMPLIANCE",
"symbol : AGENTCAPABILITIES",
"descriptor : identifier",
"moduleidentity : descriptor MODULEIDENTITY lastupdated ORGANIZATION TEXT CONTACTINFO TEXT DESCRIPTION TEXT revisionpart PRODUCTION objectidentifiervalue",
"lastupdated : LASTUPDATED TEXT",
"revisionpart : revisions",
"revisionpart :",
"revisions : revision",
"revisions : revisions revision",
"revision : REVISION TEXT DESCRIPTION TEXT",
"assignmentlist : assignment assignmentlist",
"assignmentlist :",
"assignment : descriptor OBJECT IDENTIFIER PRODUCTION objectidentifiervalue",
"assignment : descriptor OBJECTIDENTITY STATUS status DESCRIPTION TEXT referpart PRODUCTION objectidentifiervalue",
"assignment : descriptor OBJECTTYPE SYNTAX syntax unitspart MAXACCESS access STATUS status DESCRIPTION TEXT referpart indexpart defvalpart PRODUCTION objectidentifiervalue",
"assignment : descriptor NOTIFICATIONTYPE objectspart STATUS status DESCRIPTION TEXT referpart PRODUCTION objectidentifiervalue",
"assignment : typereference PRODUCTION SEQUENCE '{' entries '}'",
"assignment : typereference PRODUCTION TEXTUALCONVENTION displaypart STATUS status DESCRIPTION TEXT referpart SYNTAX syntax",
"assignment : descriptor MODULECOMPLIANCE STATUS status DESCRIPTION TEXT referpart compliancemodulepart PRODUCTION objectidentifiervalue",
"assignment : descriptor OBJECTGROUP objectspart STATUS status DESCRIPTION TEXT referpart PRODUCTION objectidentifiervalue",
"assignment : descriptor NOTIFICATIONGROUP notificationspart STATUS status DESCRIPTION TEXT referpart PRODUCTION objectidentifiervalue",
"assignment : typereference PRODUCTION syntax",
"notificationspart : NOTIFICATIONS '{' notifications '}'",
"notifications : notification",
"notifications : notification ',' notifications",
"notification : descriptor",
"objectspart : OBJECTS '{' objects '}'",
"objectspart :",
"objects : object",
"objects : objects ',' object",
"object : descriptor",
"syntax : type",
"syntax : SEQUENCE OF typereference",
"syntax : integersubtype",
"syntax : octetstringsubtype",
"syntax : enumeration",
"syntax : bits",
"unitspart : UNITS TEXT",
"unitspart :",
"referpart : REFERENCE TEXT",
"referpart :",
"indexpart : INDEX '{' indextypes '}'",
"indexpart : AUGMENTS '{' descriptor '}'",
"indexpart :",
"indextypes : indextype",
"indextypes : indextypes ',' indextype",
"indextype : IMPLIED index",
"indextype : index",
"index : descriptor",
"defvalpart : DEFVAL '{' defvalue '}'",
"defvalpart :",
"defvalue : descriptor",
"defvalue : NUMBER",
"defvalue : SIGNEDNUMBER",
"defvalue : HSTRING",
"defvalue : TEXT",
"defvalue : bitsvalue",
"defvalue : objectidentifiervalue",
"bitsvalue : '{' bitscomponentlist '}'",
"bitscomponentlist : bitscomponentlist ',' bitscomponent",
"bitscomponentlist : bitscomponent",
"bitscomponentlist :",
"bitscomponent : identifier",
"displaypart : DISPLAYHINT TEXT",
"displaypart :",
"entries : entry ',' entries",
"entries : entry",
"entry : descriptor syntax",
"compliancemodulepart : compliancemodules",
"compliancemodules : compliancemodule",
"compliancemodules : compliancemodules compliancemodule",
"compliancemodule : MODULE modulename modulemandatorypart modulecompliancepart",
"modulename : moduleidentifier",
"modulename :",
"modulemandatorypart : MANDATORYGROUPS '{' modulegroups '}'",
"modulemandatorypart :",
"modulegroups : modulegroup",
"modulegroups : modulegroups ',' modulegroup",
"modulegroup : objectidentifiervalue",
"modulegroup : descriptor",
"modulecompliancepart : modulecompliances",
"modulecompliancepart :",
"modulecompliances : modulecompliance",
"modulecompliances : modulecompliances modulecompliance",
"modulecompliance : modulecompliancegroup",
"modulecompliance : moduleobject",
"modulecompliancegroup : GROUP modulegroupobjectname DESCRIPTION TEXT",
"moduleobject : OBJECT moduleobjectname modulesyntaxpart writesyntaxpart moduleaccesspart DESCRIPTION TEXT",
"moduleobjectname : objectidentifiervalue",
"moduleobjectname : descriptor",
"modulegroupobjectname : objectidentifiervalue",
"modulegroupobjectname : descriptor",
"modulesyntaxpart : SYNTAX syntax",
"modulesyntaxpart :",
"writesyntaxpart : WRITESYNTAX syntax",
"writesyntaxpart :",
"moduleaccesspart : MINACCESS access",
"moduleaccesspart :",
"objectidentifiervalue : '{' objidcomponentfirst objidcomponent objidcomponentlist '}'",
"objidcomponentlist : objidcomponent objidcomponentlist",
"objidcomponentlist :",
"objidcomponentfirst : descriptor",
"objidcomponentfirst : objidcomponent",
"objidcomponent : NUMBER",
"objidcomponent : descriptor '(' NUMBER ')'",
"type : typereference",
"type : INTEGER",
"type : OBJECT IDENTIFIER",
"type : OCTET STRING",
"type : Integer32",
"type : IpAddress",
"type : Counter32",
"type : Gauge32",
"type : Unsigned32",
"type : TimeTicks",
"type : Opaque",
"type : Counter64",
"type : BITS",
"integersubtype : INTEGER '(' ranges ')'",
"integersubtype : Integer32 '(' ranges ')'",
"integersubtype : Unsigned32 '(' ranges ')'",
"integersubtype : Gauge32 '(' ranges ')'",
"integersubtype : typereference '(' ranges ')'",
"octetstringsubtype : OCTET STRING '(' SIZE '(' ranges ')' ')'",
"octetstringsubtype : Opaque '(' SIZE '(' ranges ')' ')'",
"octetstringsubtype : typereference '(' SIZE '(' ranges ')' ')'",
"bits : BITS '{' namedbits '}'",
"namedbits : namedbit",
"namedbits : namedbits ',' namedbit",
"namedbit : identifier '(' NUMBER ')'",
"enumeration : INTEGER '{' namednumbers '}'",
"enumeration : typereference '{' namednumbers '}'",
"namednumbers : namednumber",
"namednumbers : namednumbers ',' namednumber",
"namednumber : identifier '(' NUMBER ')'",
"namednumber : identifier '(' SIGNEDNUMBER ')'",
"ranges : range '|' ranges",
"ranges : range",
"range : value",
"range : value RANGESEPARATOR value",
"value : SIGNEDNUMBER",
"value : NUMBER",
"value : HSTRING",
"value : BSTRING",
"access : descriptor",
"status : descriptor",

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
#line 968 "mib.y"

void
yyerror(const char *fmt, ...)
{
	va_list		 ap;
	char		 msg[1024] = "";

	if (file.state == FILE_UNDEFINED) {
		log_debug("%s: not an ASN.1 file: skipping", file.name);
		return;
	}
	if (file.state == FILE_ASN1) {
		if (strcmp(fmt, "syntax error") == 0) {
			log_debug("%s: not an SMIv2 file: skipping", file.name);
			return;
		}
	}
	if (fmt != NULL) {
		va_start(ap, fmt);
		vsnprintf(msg, sizeof(msg), fmt, ap);
		va_end(ap);
	}
	log_warnx("%s:%zu: %s", file.name, file.lineno, msg);
}

/*
 * X.208
 */
int
yylex(void)
{
	const struct {
		const char *name;
		int token;
	} keywords[] = {
		/* RFC2578 section 3.7 */
		{ "ABSENT",		ABSENT },
		{ "ACCESS",		ACCESS },
		{ "AGENT-CAPABILITIES",	AGENTCAPABILITIES },
		{ "ANY",		ANY },
		{ "APPLICATION",	APPLICATION },
		{ "AUGMENTS",		AUGMENTS },
		{ "BEGIN",		BEGIN },
		{ "BIT",		BIT },
		{ "BITS",		BITS },
		{ "BOOLEAN",		BOOLEAN },
		{ "BY",			BY },
		{ "CHOICE",		CHOICE },
		{ "COMPONENT",		COMPONENT },
		{ "COMPONENTS",		COMPONENTS },
		{ "CONTACT-INFO",	CONTACTINFO },
		{ "CREATION-REQUIRES",	CREATIONREQUIRES },
		{ "Counter32",		Counter32 },
		{ "Counter64",		Counter64 },
		{ "DEFAULT",		DEFAULT },
		{ "DEFINED",		DEFINED },
		{ "DEFINITIONS",	DEFINITIONS },
		{ "DEFVAL",		DEFVAL },
		{ "DESCRIPTION",	DESCRIPTION },
		{ "DISPLAY-HINT",	DISPLAYHINT },
		{ "END",		END },
		{ "ENUMERATED",		ENUMERATED },
		{ "ENTERPRISE",		ENTERPRISE },
		{ "EXPLICIT",		EXPLICIT },
		{ "EXPORTS",		EXPORTS },
		{ "EXTERNAL",		EXTERNAL },
		{ "FALSE",		FALSE },
		{ "FROM",		FROM },
		{ "GROUP",		GROUP },
		{ "Gauge32",		Gauge32 },
		{ "IDENTIFIER",		IDENTIFIER },
		{ "IMPLICIT",		IMPLICIT },
		{ "IMPLIED",		IMPLIED },
		{ "IMPORTS",		IMPORTS },
		{ "INCLUDES",		INCLUDES },
		{ "INDEX",		INDEX },
		{ "INTEGER",		INTEGER },
		{ "Integer32",		Integer32 },
		{ "IpAddress",		IpAddress },
		{ "LAST-UPDATED",	LASTUPDATED },
		{ "MANDATORY-GROUPS",	MANDATORYGROUPS },
		{ "MAX",		MAX },
		{ "MAX-ACCESS",		MAXACCESS },
		{ "MIN",		MIN },
		{ "MIN-ACCESS",		MINACCESS },
		{ "MINUS-INFINITY",	MINUSINFINITY },
		{ "MODULE",		MODULE },
		{ "MODULE-COMPLIANCE",	MODULECOMPLIANCE },
		{ "MODULE-IDENTITY",	MODULEIDENTITY },
		{ "NOTIFICATION-GROUP",	NOTIFICATIONGROUP },
		{ "NOTIFICATION-TYPE",	NOTIFICATIONTYPE },
		{ "NOTIFICATIONS",	NOTIFICATIONS },
		{ "NULL",		ASNNULL },
		{ "OBJECT",		OBJECT },
		{ "OBJECT-GROUP",	OBJECTGROUP },
		{ "OBJECT-IDENTITY",	OBJECTIDENTITY },
		{ "OBJECT-TYPE",	OBJECTTYPE },
		{ "OBJECTS",		OBJECTS },
		{ "OCTET",		OCTET },
		{ "OF",			OF },
		{ "OPTIONAL",		OPTIONAL },
		{ "ORGANIZATION",	ORGANIZATION },
		{ "Opaque",		Opaque },
		{ "PLUS-INFINITY",	PLUSINFINITY },
		{ "PRESENT",		PRESENT },
		{ "PRIVATE",		PRIVATE },
		{ "PRODUCT-RELEASE",	PRODUCTRELEASE },
		{ "REAL",		REAL },
		{ "REFERENCE",		REFERENCE },
		{ "REVISION",		REVISION },
		{ "SEQUENCE",		SEQUENCE },
		{ "SET",		SET },
		{ "SIZE",		SIZE },
		{ "STATUS",		STATUS },
		{ "STRING",		STRING },
		{ "SUPPORTS",		SUPPORTS },
		{ "SYNTAX",		SYNTAX },
		{ "TAGS",		TAGS },
		{ "TEXTUAL-CONVENTION",	TEXTUALCONVENTION },
		{ "TRAP-TYPE",		TRAPTYPE },
		{ "TRUE",		TRUE },
		{ "TimeTicks",		TimeTicks },
		{ "UNITS",		UNITS },
		{ "UNIVERSAL",		UNIVERSAL },
		{ "Unsigned32",		Unsigned32 },
		{ "VARIABLES",		VARIABLES },
		{ "VARIATION",		VARIATION },
		{ "WITH",		WITH },
		{ "WRITE-SYNTAX",	WRITESYNTAX },

		/* ASN.1 */
		{ "::=",		PRODUCTION },
		{ "..",			RANGESEPARATOR },

		/* SMIv2 */
		{ "SNMPv2-SMI",		SNMPv2SMI },
		{ "SNMPv2-CONF",	SNMPv2CONF },
		{ "SNMPv2-TC",		SNMPv2TC },

		{}
	};
	char buf[TEXT_MAX];
	size_t i = 0, j;
	int c, comment = 0;
	const char *errstr;
	char *endptr;

	while ((c = fgetc(file.stream)) != EOF) {
		if (i == sizeof(buf)) {
			yyerror("token too large");
			return ERROR;
		}
		if (i > 0 && buf[0] == '"') {
			if (c == '"') {
				buf[i] = '\0';
				(void)strlcpy(yylval.string, buf + 1,
				    sizeof(yylval.string));
				return TEXT;
			}
			if (c == '\n')
				file.lineno++;
			buf[i++] = c;
			continue;
		}
		if (comment) {
			if (c == '-') {
				if (++comment == 3)
					comment = 0;
			} else if (c == '\n') {
				file.lineno++;
				comment = 0;
			} else
				comment = 1;
			continue;
		}
		if (c == '\n') {
			if (i != 0) {
				if (buf[i - 1] == '\r') {
					i--;
					if (i == 0) {
						file.lineno++;
						continue;
					}
				}
				ungetc(c, file.stream);
				goto token;
			}
			file.lineno++;
			continue;
		}
		if (c == ' ' || c == '\t') {
			if (i == 0)
				continue;
			goto token;
		}
		if (c == '.' || c == ':') {
			if (i > 0 && buf[0] != '.' && buf[0] != ':') {
				ungetc(c, file.stream);
				goto token;
			}
		}
		if (i > 0 && (buf[0] == '.' || buf[0] == ':')) {
			if (c != '.' && c != ':' && c != '=') {
				ungetc(c, file.stream);
				goto token;
			}
		}
		if (c == ',' || c == ';' || c == '{' || c == '}' ||
		    c == '(' || c == ')' || c == '[' || c == ']' || c == '|') {
			if (i == 0)
				return c;
			ungetc(c, file.stream);
			goto token;
		}
		buf[i++] = c;
		if (i >= 2) {
			if (buf[i - 2] == '-' && buf[i - 1] == '-') {
				if (i > 2) {
					ungetc('-', file.stream);
					ungetc('-', file.stream);
					i -= 2;
					goto token;
				}
				comment = 1;
				i = 0;
				continue;
			}
		}
	}

	if (ferror(file.stream)) {
		yyerror(NULL);
		return ERROR;
	}
	if (i == 0)
		return 0;

 token:
	buf[i] = '\0';

	for (i = 0; keywords[i].name != NULL; i++) {
		if (strcmp(keywords[i].name, buf) == 0)
			return keywords[i].token;
	}

	if (isupper(buf[0])) {
		for (i = 1; buf[i] != '\0'; i++) {
			if (!isalnum(buf[i]) && buf[i] != '-')
				break;
		}
		if (buf[i] == '\0' && buf[i - 1] != '-') {
			strlcpy(yylval.string, buf, sizeof(yylval.string));
			return typereference;
		}
	}
	if (islower(buf[0])) {
		for (i = 1; buf[i] != '\0'; i++) {
			if (!isalnum(buf[i]) && buf[i] != '-')
				break;
		}
		if (buf[i] == '\0'&& buf[i - 1] != '-') {
			strlcpy(yylval.string, buf, sizeof(yylval.string));
			return identifier;
		}
	}

	if (buf[0] == '\'') {
		for (i = 1; buf[i] != '\0'; i++)
			continue;
		if (i < 3 || buf[i - 2] != '\'') {
			yyerror("incomplete binary or hexadecimal string");
			return ERROR;
		}
		if (tolower(buf[i - 1]) == 'b') {
			for (j = 1; j < i - 2; j++) {
				if (buf[j] != '0' && buf[j] != '1') {
					yyerror("invalid character in bstring");
					return ERROR;
				}
			}
			strlcpy(yylval.string, buf + 1, sizeof(yylval.string));
			yylval.string[i - 2] = '\0';
			return BSTRING;
		} else if (tolower(buf[i - 1]) == 'h') {
			for (j = 1; j < i - 2; j++) {
				if (!isxdigit(buf[j])) {
					yyerror("invalid character in hstring");
					return ERROR;
				}
			}
			strlcpy(yylval.string, buf + 1, sizeof(yylval.string));
			yylval.string[i - 2] = '\0';
			return HSTRING;
		}
		yyerror("no valid binary or hexadecimal string");
		return ERROR;
	}
	for (i = 0; buf[i] != '\0'; i++) {
		if (i == 0 && buf[i] == '-')
			continue;
		if (!isdigit(buf[i]))
			break;
	}
	if ((i == 1 && isdigit(buf[0])) || i > 1) {
		yylval.signednumber =
		    strtonum(buf, LLONG_MIN, LLONG_MAX, &errstr);
		if (errstr != NULL) {
			if (errno == ERANGE && isdigit(buf[0])) {
				errno = 0;
				yylval.number = strtoull(buf, &endptr, 10);
				if (errno == 0)
					return NUMBER;
			}
			yyerror("invalid number: %s: %s", buf, errstr);
			return ERROR;
		}
		if (buf[0] == '-')
			return SIGNEDNUMBER;
		yylval.number = yylval.signednumber;
		return NUMBER;
	}

	yyerror("unknown token: %s", buf);
	return ERROR;
}

void
mib_clear(void)
{
	struct module *m;
	struct item *iso;

	while ((m = RB_ROOT(&modulesci)) != NULL)
		mib_modulefree(m);

	/* iso */
	iso = RB_ROOT(&items);
	assert(strcmp(iso->name, "iso") == 0);
	RB_REMOVE(itemsgci, &itemsci, iso);
	RB_REMOVE(items, &items, iso);
	free(iso->oid.bo_id);
	free(iso);
	assert(RB_EMPTY(&modulesci));
	assert(RB_EMPTY(&modulescs));
	assert(RB_EMPTY(&items));
	assert(RB_EMPTY(&itemsci));
}

void
mib_modulefree(struct module *m)
{
	struct item *item;

	if (m == NULL)
		return;

	if (RB_FIND(modulesci, &modulesci, m) == m)
		RB_REMOVE(modulesci, &modulesci, m);
	if (RB_FIND(modulescs, &modulescs, m) == m)
		RB_REMOVE(modulescs, &modulescs, m);
	free(m->imports);

	while ((item = RB_ROOT(&m->itemscs)) != NULL) {
		RB_REMOVE(itemscs, &m->itemscs, item);
		if (RB_FIND(itemsci, &m->itemsci, item) == item)
			RB_REMOVE(itemsci, &m->itemsci, item);
		if (RB_FIND(items, &items, item) == item)
			RB_REMOVE(items, &items, item);
		if (RB_FIND(itemsgci, &itemsci, item) == item)
			RB_REMOVE(itemsgci, &itemsci, item);
		if (!item->resolved)
			free(item->oid_unresolved);
		else
			free(item->oid.bo_id);
		free(item);
	}

	free(m);
}

int
mib_imports_add(char *name, char **symbols)
{
	size_t im, ism, isi;
	struct import *import;

	for (im = 0; module->imports != NULL &&
	    module->imports[im].name[0] != '\0'; im++) {
		if (strcmp(module->imports[im].name, name) == 0)
			break;
	}
	if (module->imports == NULL || module->imports[im].name[0] == '\0') {
		if ((import = reallocarray(module->imports, im + 2,
		    sizeof(*module->imports))) == NULL) {
			yyerror("malloc");
			return -1;
		}
		module->imports = import;
		strlcpy(module->imports[im].name, name,
		    sizeof(module->imports[im].name));
		module->imports[im].nsymbols = 0;

		module->imports[im + 1].name[0] = '\0';
	}

	import = &module->imports[im];
	for (isi = 0; symbols[isi] != NULL; isi++) {
		for (ism = 0; ism < import->nsymbols; ism++) {
			if (strcmp(symbols[isi],
			    import->symbols[ism].name) == 0) {
				yyerror("symbol %s already imported",
				    symbols[isi]);
				break;
			}
		}
		if (ism != import->nsymbols)
			continue;

		if (import->nsymbols == nitems(import->symbols)) {
			yyerror("Too many symbols imported");
			return -1;
		}
		strlcpy(import->symbols[ism].name, symbols[isi],
		    sizeof(import->symbols[ism].name));
		import->symbols[ism].item = NULL;
		import->nsymbols++;
	}
	return 0;
}

int
mib_oid_append(struct oid_unresolved *oid, const struct objidcomponent *subid)
{
	if (oid->bo_n == nitems(oid->bo_id)) {
		yyerror("oid too long");
		return -1;
	}

	switch (oid->bo_id[oid->bo_n].type = subid->type) {
	case OCT_DESCRIPTOR:
		strlcpy(oid->bo_id[oid->bo_n].name, subid->name,
		    sizeof(oid->bo_id[oid->bo_n].name));
		break;
	case OCT_NUMBER:
		oid->bo_id[oid->bo_n].number = subid->number;
		break;
	case OCT_NAMEANDNUMBER:
		oid->bo_id[oid->bo_n].number = subid->number;
		strlcpy(oid->bo_id[oid->bo_n].name, subid->name,
		    sizeof(oid->bo_id[oid->bo_n].name));
	}
	oid->bo_n++;
	return 0;
}

int
mib_oid_concat(struct oid_unresolved *dst, const struct oid_unresolved *src)
{
	size_t i;

	for (i = 0; i < src->bo_n; i++)
		if (mib_oid_append(dst, &src->bo_id[i]) == -1)
			return -1;
	return 0;
}

struct item *
mib_item(const char *name, enum item_type type)
{
	struct item *item;

	if ((item = calloc(1, sizeof(*item))) == NULL) {
		log_warn("malloc");
		return NULL;
	}

	item->type = type;
	item->resolved = 0;
	item->module = module;
	(void)strlcpy(item->name, name, sizeof(item->name));

	if (RB_INSERT(itemscs, &module->itemscs, item) != NULL) {
		yyerror("duplicate item %s", name);
		free(item);
		return NULL;
	}

	return item;
}

int
mib_item_oid(struct item *item, const struct oid_unresolved *oid)
{
	if ((item->oid_unresolved = calloc(1,
	    sizeof(*item->oid_unresolved))) == NULL) {
		yyerror("malloc");
		return -1;
	}

	*item->oid_unresolved = *oid;
	return 0;
}

int
mib_macro(const char *name)
{
	return mib_item(name, IT_MACRO) == NULL ? -1 : 0;
}

struct item *
mib_oid(const char *name, const struct oid_unresolved *oid)
{
	struct item *item;

	if ((item = mib_item(name, IT_OID)) == NULL)
		return NULL;

	if (mib_item_oid(item, oid) == -1)
		return NULL;
	return item;
}

int
mib_applicationsyntax(const char *name)
{
	return mib_item(name, IT_APPLICATIONSYNTAX) == NULL ? -1 : 0;
}

int
mib_moduleidentity(const char *name, time_t lastupdated,
    const char *organization, const char *contactinfo, const char *description,
    const struct oid_unresolved *oid)
{
	struct item *item;

	if ((item = mib_item(name, IT_MODULE_IDENTITY)) == NULL)
		return -1;

	if (mib_item_oid(item, oid) == -1)
		return -1;

	module->lastupdated = lastupdated;
	return 0;
}

int
mib_objectidentity(const char *name, enum status status,
    const char *description, const char *reference,
    const struct oid_unresolved *oid)
{
	struct item *item;

	if ((item = mib_item(name, IT_OBJECT_IDENTITY)) == NULL)
		return -1;

	item->objectidentity.status = status;
	if (mib_item_oid(item, oid) == -1)
		return -1;

	return 0;
}

int
mib_objecttype(const char *name, void *syntax, const char *units,
    enum access maxaccess, enum status status, const char *description,
    const char *reference, void *index, void *defval,
    const struct oid_unresolved *oid)
{
	struct item *item;

	if ((item = mib_item(name, IT_OBJECT_TYPE)) == NULL)
		return -1;

	item->objecttype.maxaccess = maxaccess;
	item->objecttype.status = status;
	if (mib_item_oid(item, oid) == -1)
		return -1;
	return 0;
}

int
mib_notificationtype(const char *name, void *objects, enum status status,
    const char *description, const char *reference,
    const struct oid_unresolved *oid)
{
	struct item *item;

	if ((item = mib_item(name, IT_NOTIFICATION_TYPE)) == NULL)
		return -1;

	item->notificationtype.status = status;
	if (mib_item_oid(item, oid) == -1)
		return -1;
	return 0;
}

int
mib_textualconvetion(const char *name, const char *displayhint,
    enum status status, const char *description, const char *reference,
    void *syntax)
{
	struct item *item;

	if ((item = mib_item(name, IT_TEXTUAL_CONVENTION)) == NULL)
		return -1;
	item->textualconvention.status = status;
	return 0;
}

int
mib_objectgroup(const char *name, void *objects, enum status status,
    const char *description, const char *reference,
    const struct oid_unresolved *oid)
{
	struct item *item;

	if ((item = mib_item(name, IT_OBJECT_GROUP)) == NULL)
		return -1;

	if (mib_item_oid(item, oid) == -1)
		return -1;
	return 0;
}

int
mib_notificationgroup(const char *name, void *notifications, enum status status,
    const char *description, const char *reference,
    const struct oid_unresolved *oid)
{
	struct item *item;

	if ((item = mib_item(name, IT_NOTIFICATION_GROUP)) == NULL)
		return -1;

	if (mib_item_oid(item, oid) == -1)
		return -1;
	return 0;
}

int
mib_modulecompliance(const char *name, enum status status,
    const char *description, const char *reference, void *mods,
    const struct oid_unresolved *oid)
{
	struct item *item;

	if ((item = mib_item(name, IT_MODULE_COMPLIANCE)) == NULL)
		return -1;

	if (mib_item_oid(item, oid) == -1)
		return -1;
	return 0;
}

void
mib_parsefile(const char *path)
{
	mib_defaults();

	log_debug("mib parsing %s", path);
	if ((file.stream = fopen(path, "r")) == NULL) {
		log_warn("fopen");
		return;
	}
	file.lineno = 1;
	file.name = path;
	file.state = FILE_UNDEFINED;

	if (yyparse() != 0) {
		mib_modulefree(module);
		module = NULL;
	}

	fclose(file.stream);
}

void
mib_parsedir(const char *path)
{
	DIR *dir;
	struct dirent *dirent;
	char mibfile[PATH_MAX];

	if ((dir = opendir(path)) == NULL) {
		log_warn("opendir(%s)", path);
		return;
	}

	errno = 0;
	while ((dirent = readdir(dir)) != NULL) {
		if (dirent->d_name[0] == '.')
			continue;
		if (snprintf(mibfile, sizeof(mibfile), "%s/%s",
		    path, dirent->d_name) >= (int)sizeof(mibfile))
			continue;
		if (dirent->d_type == DT_DIR) {
			mib_parsedir(mibfile);
			continue;
		}
		if (dirent->d_type != DT_REG)
			continue;
		mib_parsefile(mibfile);
	}

	closedir(dir);
}

struct item *
mib_item_parent(struct ber_oid *oid)
{
	struct item *item, search;

	search.oid.bo_n = oid->bo_n;
	search.oid.bo_id = oid->bo_id;

	while (search.oid.bo_n > 0) {
		if ((item = RB_FIND(items, &items, &search)) != NULL)
			return item;
		search.oid.bo_n--;
	}
	return NULL;
}

const char *
mib_string2oid(const char *str, struct ber_oid *oid)
{
	char mname[512], *descriptor, *digits;
	struct module *m = NULL, msearch;
	struct item *item = NULL, isearch;
	struct ber_oid oidbuf;
	size_t i;

	oid->bo_n = 0;

	if (isdigit(str[0])) {
		if (ober_string2oid(str, oid) == -1)
			return "invalid OID";
		return NULL;
	}

	if (strlcpy(mname, str, sizeof(mname)) >= sizeof(mname))
		return "OID name too long";

	if ((descriptor = strchr(mname, ':')) != NULL) {
		descriptor[0] = '\0';
		if (descriptor[1] != ':')
			return "module and descriptor must be separated by "
			    "double colon";
		descriptor += 2;
		if (strlcpy(msearch.name, mname, sizeof(msearch.name)) >=
		    sizeof(msearch.name))
			return "module not found";
		if ((m = RB_FIND(modulescs, &modulescs, &msearch)) == NULL) {
			m = RB_FIND(modulesci, &modulesci, &msearch);
			if (m == NULL)
				return "module not found";
		}
	} else
		descriptor = mname;

	if ((digits = strchr(descriptor, '.')) != NULL) {
		digits++[0] = '\0';
		if (ober_string2oid(digits, &oidbuf) == -1)
			return "invalid OID";
	} else
		oidbuf.bo_n = 0;

	if (strlcpy(isearch.name, descriptor, sizeof(isearch.name)) >=
	    sizeof(isearch.name))
		return "descriptor not found";

	if (m != NULL) {
		item = RB_FIND(itemscs, &m->itemscs, &isearch);
		if (item == NULL)
			item = RB_FIND(itemsci, &m->itemsci, &isearch);
	} else
		item = RB_FIND(itemsgci, &itemsci, &isearch);
	if (item == NULL)
		return "descriptor not found";

	if (item->oid.bo_n + oidbuf.bo_n > nitems(oid->bo_id))
		return "OID too long";

	for (i = 0; i < item->oid.bo_n; i++)
		oid->bo_id[oid->bo_n++] = item->oid.bo_id[i];
	for (i = 0; i < oidbuf.bo_n; i++)
		oid->bo_id[oid->bo_n++] = oidbuf.bo_id[i];

	return NULL;
}

char *
mib_oid2string(struct ber_oid *oid, char *buf, size_t buflen,
    enum mib_oidfmt fmt)
{
	struct item *item;
	char digit[11];
	size_t i = 0;

	buf[0] = '\0';
	if (fmt == MIB_OIDSYMBOLIC && (item = mib_item_parent(oid)) != NULL) {
		snprintf(buf, buflen, "%s::%s", item->module->name,
		    item->name);
		i = item->oid.bo_n;
	}

	for (; i < oid->bo_n; i++) {
		if (i != 0)
			strlcat(buf, ".", buflen);
		snprintf(digit, sizeof(digit), "%"PRIu32, oid->bo_id[i]);
		strlcat(buf, digit, buflen);
	}

	return buf;
}

void
mib_defaults(void)
{
	struct oid_unresolved oid;
	struct item *iso;

	if (!RB_EMPTY(&modulesci))
		return;

	/* ASN.1 constant, not part of a module */
	if ((iso = calloc(1, sizeof(*iso))) == NULL)
		fatal("malloc");
	iso->type = IT_OID;
	iso->resolved = 1;
	iso->module = NULL;
	strlcpy(iso->name, "iso", sizeof(iso->name));
	if ((iso->oid.bo_id = calloc(1, sizeof(*iso->oid.bo_id))) == NULL)
		fatal("malloc");
	iso->oid.bo_id[0] = 1;
	iso->oid.bo_n = 1;
	RB_INSERT(items, &items, iso);
	RB_INSERT(itemsgci, &itemsci, iso);

	file.state = FILE_SMI2;
	file.lineno = 0;

	if ((module = calloc(1, sizeof(*module))) == NULL)
		fatal("malloc");
	RB_INIT(&module->itemscs);
	RB_INIT(&module->itemsci);
	module->resolved = 0;

	strlcpy(module->name, "SNMPv2-SMI", sizeof(module->name));
	file.name = module->name;
	oid.bo_id[0].type = oid.bo_id[1].type = OCT_NUMBER;
	oid.bo_n = 2;

	oid.bo_id[0].number = 1;
	oid.bo_id[1].number = 3;
	if (mib_oid("org", &oid) == NULL)
		exit(1);

	oid.bo_id[0].number = oid.bo_id[1].number = 0;
	if (mib_oid("zeroDotZero", &oid) == NULL)
		exit(1);

	oid.bo_id[0].type = OCT_DESCRIPTOR;
	strlcpy(oid.bo_id[0].name, "org", sizeof(oid.bo_id[0].name));
	oid.bo_id[1].number = 6;
	if (mib_oid("dod", &oid) == NULL)
		exit(1);

	strlcpy(oid.bo_id[0].name, "dod", sizeof(oid.bo_id[0].name));
	oid.bo_id[1].number = 1;
	if (mib_oid("internet", &oid) == NULL)
		exit(1);

	strlcpy(oid.bo_id[0].name, "internet", sizeof(oid.bo_id[0].name));
	oid.bo_id[1].number = 1;
	if (mib_oid("directory", &oid) == NULL)
		exit(1);

	oid.bo_id[1].number = 2;
	if (mib_oid("mgmt", &oid) == NULL)
		exit(1);

	strlcpy(oid.bo_id[0].name, "mgmt", sizeof(oid.bo_id[0].name));
	oid.bo_id[1].number = 1;
	if (mib_oid("mib-2", &oid) == NULL)
		exit(1);

	strlcpy(oid.bo_id[0].name, "mib-2", sizeof(oid.bo_id[0].name));
	oid.bo_id[1].number = 10;
	if (mib_oid("transmission", &oid) == NULL)
		exit(1);

	strlcpy(oid.bo_id[0].name, "internet", sizeof(oid.bo_id[0].name));
	oid.bo_id[1].number = 3;
	if (mib_oid("experimental", &oid) == NULL)
		exit(1);

	oid.bo_id[1].number = 4;
	if (mib_oid("private", &oid) == NULL)
		exit(1);

	oid.bo_id[1].number = 5;
	if (mib_oid("security", &oid) == NULL)
		exit(1);

	oid.bo_id[1].number = 6;
	if (mib_oid("snmpV2", &oid) == NULL)
		exit(1);

	strlcpy(oid.bo_id[0].name, "private", sizeof(oid.bo_id[0].name));
	oid.bo_id[1].number = 1;
	if (mib_oid("enterprises", &oid) == NULL)
		exit(1);

	strlcpy(oid.bo_id[0].name, "snmpV2", sizeof(oid.bo_id[0].name));
	oid.bo_id[1].number = 1;
	if (mib_oid("snmpDomains", &oid) == NULL)
		exit(1);

	oid.bo_id[1].number = 2;
	if (mib_oid("snmpProxys", &oid) == NULL)
		exit(1);

	oid.bo_id[1].number = 3;
	if (mib_oid("snmpModules", &oid) == NULL)
		exit(1);

	if (mib_macro("MODULE-IDENTITY") == -1 ||
	    mib_macro("OBJECT-IDENTITY") == -1 ||
	    mib_macro("OBJECT-TYPE") == -1 ||
	    mib_macro("NOTIFICATION-TYPE") == -1)
		exit(1);

	if (mib_applicationsyntax("Integer32") == -1 ||
	    mib_applicationsyntax("IpAddress") == -1 ||
	    mib_applicationsyntax("Counter32") == -1 ||
	    mib_applicationsyntax("Gauge32") == -1 ||
	    mib_applicationsyntax("Unsigned32") == -1 ||
	    mib_applicationsyntax("TimeTicks") == -1 ||
	    mib_applicationsyntax("Opaque") == -1 ||
	    mib_applicationsyntax("Counter64") == -1)
		exit(1);

	RB_INSERT(modulesci, &modulesci, module);
	RB_INSERT(modulescs, &modulescs, module);

	if ((module = calloc(1, sizeof(*module))) == NULL)
		fatal("malloc");
	RB_INIT(&module->itemscs);
	RB_INIT(&module->itemsci);
	module->resolved = 0;

	strlcpy(module->name, "SNMPv2-TC", sizeof(module->name));
	file.name = module->name;

	if (mib_macro("TEXTUAL-CONVENTION") == -1)
		exit(1);

	if (mib_textualconvetion(
	    "DisplayString", "255a", CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "PhysAddress", "1x:", CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "MacAddress", "1x:", CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "TruthValue", NULL, CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "TestAndIncr", NULL, CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "AutonomousType", NULL, CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "InstancePointer", NULL, OBSOLETE, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "VariablePointer", NULL, CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "RowPointer", NULL, CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "RowStatus", NULL, CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "TimeStamp", NULL, CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "TimeInterval", NULL, CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "DateAndTime", NULL, CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "StorageType", NULL, CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "TDomain", NULL, CURRENT, "", NULL, NULL) == -1 ||
	    mib_textualconvetion(
	    "TAddress", NULL, CURRENT, "", NULL, NULL) == -1)
		exit(1);

	RB_INSERT(modulesci, &modulesci, module);
	RB_INSERT(modulescs, &modulescs, module);

	if ((module = calloc(1, sizeof(*module))) == NULL)
		fatal("malloc");
	RB_INIT(&module->itemscs);
	RB_INIT(&module->itemsci);
	module->resolved = 0;

	strlcpy(module->name, "SNMPv2-CONF", sizeof(module->name));
	file.name = module->name;

	if (mib_macro("OBJECT-GROUP") == -1 ||
	    mib_macro("NOTIFICATION-GROUP") == -1 ||
	    mib_macro("MODULE-COMPLIANCE") == -1 ||
	    mib_macro("AGENT-CAPABILITIES") == -1)
		exit(1);

	RB_INSERT(modulesci, &modulesci, module);
	RB_INSERT(modulescs, &modulescs, module);

	module = NULL;
}

/*
 * Used only for resolving phase
 */
struct item *
mib_item_find(struct item *orig, const char *name)
{
	struct module *m = orig->module;
	struct item *item, search;
	struct import *import;
	size_t i, j;

	strlcpy(search.name, name, sizeof(search.name));
	if ((item = RB_FIND(itemscs, &m->itemscs, &search)) != NULL) {
		if (mib_resolve_item(item) == -1)
			return NULL;
		return item;
	}

	for (i = 0; m->imports != NULL && m->imports[i].name[0] != '\0'; i++) {
		import = &m->imports[i];
		for (j = 0; j < import->nsymbols; j++) {
			if (strcmp(name, import->symbols[j].name) == 0)
				return import->symbols[j].item;
		}
	}

	log_warnx("%s::%s: item %s not found: disabling",
	    m->name, orig->name, name);

	return NULL;
}

int
mib_resolve_oid(struct oid_resolved *dst, struct oid_unresolved *src,
    struct item *item)
{
	struct module *m = item->module;
	struct item *reference, search;
	struct ber_oid oid;
	size_t i, j, bo_n;

	oid.bo_n = 0;
	for (i = 0; i < src->bo_n; i++) {
		switch (src->bo_id[i].type) {
		case OCT_DESCRIPTOR:
			if ((reference = mib_item_find(item,
			    src->bo_id[i].name)) == NULL)
				return -1;
			for (j = 0; j < reference->oid.bo_n; j++)
				oid.bo_id[oid.bo_n++] =
				    reference->oid.bo_id[j];
			break;
		case OCT_NUMBER:
			if (oid.bo_n == nitems(oid.bo_id)) {
				log_warnx("%s::%s: OID too long: disabling",
				    m->name, item->name);
				return -1;
			}
			oid.bo_id[oid.bo_n++] = src->bo_id[i].number;
			break;
		case OCT_NAMEANDNUMBER:
			if (oid.bo_n == nitems(oid.bo_id)) {
				log_warnx("%s::%s: OID too long: disabling",
				    m->name, item->name);
				return -1;
			}
			oid.bo_id[oid.bo_n++] = src->bo_id[i].number;
			if (i == src->bo_n - 1) {
				if (strcmp(src->bo_id[i].name, item->name) != 0) {
					log_warnx("%s::%s: last OBJECT "
					    "IDENTIFIER component name doesn't "
					    "match item: disabling", m->name,
					    item->name);
					return -1;
				}
				break;
			}
			strlcpy(search.name, src->bo_id[i].name,
			    sizeof(search.name));
			reference = RB_FIND(itemscs, &m->itemscs, &search);
			if (reference != NULL) {
				search.oid.bo_n = oid.bo_n;
				search.oid.bo_id = oid.bo_id;
				if (item_cmp_oid(reference, &search) != 0) {
					log_warnx("%s::%s: two different OIDs "
					    "for same descriptor: disabling",
					    m->name, item->name);
					return -1;
				}
				break;
			}

			bo_n = src->bo_n;
			src->bo_n = i + 1;
			module = m;
			reference = mib_oid(src->bo_id[i].name, src);
			module = NULL;
			if (reference == NULL)
				return -1;
			if (mib_resolve_item(reference) == -1)
				return -1;
			src->bo_n = bo_n;
			break;
		}
	}

	dst->bo_n = oid.bo_n;
	if ((dst->bo_id = calloc(dst->bo_n, sizeof(*dst->bo_id))) == NULL) {
		log_warn("malloc");
		return -1;
	}
	for (i = 0; i < oid.bo_n; i++)
		dst->bo_id[i] = oid.bo_id[i];

	return 0;
}

/*
 * No recursion protection. Assume MIBs do the right thing
 */
int
mib_resolve_item(struct item *item)
{
	struct item *prev;
	struct oid_resolved oid;

	if (item->resolved)
		return 0;

	item->resolved = 1;

	if (item->type == IT_MACRO ||
	    item->type == IT_APPLICATIONSYNTAX ||
	    item->type == IT_TEXTUAL_CONVENTION)
		return 0;

	if (mib_resolve_oid(&oid, item->oid_unresolved, item) == -1)
		return -1;
	free(item->oid_unresolved);
	item->oid = oid;

	if ((prev = RB_INSERT(items, &items, item)) != NULL) {
		/* Prioritize an OID derived from a MACRO over a plain OID */
		if (prev->type == IT_OID && item->type != IT_OID) {
			RB_REMOVE(items, &items, prev);
			RB_INSERT(items, &items, item);
		}
	}
	RB_INSERT(itemsgci, &itemsci, item);
	RB_INSERT(itemsci, &item->module->itemsci, item);

	return 0;
}

int
mib_resolve_module(struct module *m)
{
	struct module msearch;
	struct import *import;
	struct import_symbol *symbol;
	struct item *item, isearch;
	size_t i, j;

	if (m->resolved)
		return 0;

	m->resolved = 1;

	for (i = 0; m->imports != NULL && m->imports[i].name[0] != '\0'; i++) {
		import = &m->imports[i];
		strlcpy(msearch.name, import->name, sizeof(msearch.name));
		import->module = RB_FIND(modulescs, &modulescs, &msearch);
		if (import->module == NULL ||
		    mib_resolve_module(import->module) == -1) {
			log_warnx("%s: import %s not found: disabling",
			    m->name, import->name);
			goto fail;
		}

		for (j = 0; j < import->nsymbols; j++) {
			symbol = &import->symbols[j];
			strlcpy(isearch.name, symbol->name,
			    sizeof(isearch.name));
			symbol->item = RB_FIND(itemscs,
			    &import->module->itemscs, &isearch);
			if (symbol->item == NULL) {
				log_warnx("%s: symbol %s not found in %s: "
				    "disabling", m->name, symbol->name,
				    import->name);
				goto fail;
			}
		}
	}

	RB_FOREACH(item, itemscs, &m->itemscs) {
		if (mib_resolve_item(item) == -1)
			goto fail;
	}

	free(m->imports);
	m->imports = NULL;

	return 0;
 fail:
	mib_modulefree(m);
	return -1;
}

void
mib_resolve(void)
{
	struct module *m;

	mib_defaults();
 next:
	RB_FOREACH(m, modulescs, &modulescs) {
		if (mib_resolve_module(m) == -1) {
			/*
			 * mib_resolve_module can recurse and remove,
			 * for which RB_FOEACH_SAFE doesn't protect.
			 */
			goto next;
		}
	}
}

int
module_cmp_cs(struct module *m1, struct module *m2)
{
	return strcmp(m1->name, m2->name);
}

int
module_cmp_ci(struct module *m1, struct module *m2)
{
	return strcasecmp(m1->name, m2->name);
}

int
item_cmp_cs(struct item *d1, struct item *d2)
{
	return strcmp(d1->name, d2->name);
}

int
item_cmp_ci(struct item *d1, struct item *d2)
{
	return strcasecmp(d1->name, d2->name);
}

int
item_cmp_oid(struct item *i1, struct item *i2)
{
	size_t   i, min;

	min = i1->oid.bo_n < i2->oid.bo_n ? i1->oid.bo_n : i2->oid.bo_n;
	for (i = 0; i < min; i++) {
		if (i1->oid.bo_id[i] < i2->oid.bo_id[i])
			return (-1);
		if (i1->oid.bo_id[i] > i2->oid.bo_id[i])
			return (1);
	}
	/* i1 is parent of i2 */
	if (i1->oid.bo_n < i2->oid.bo_n)
		return (-2);
	/* i1 is child of i2 */
	if (i1->oid.bo_n > i2->oid.bo_n)
		return 2;
	return (0);
}

RB_GENERATE_STATIC(modulesci, module, entryci, module_cmp_ci);
RB_GENERATE_STATIC(modulescs, module, entrycs, module_cmp_cs);
RB_GENERATE_STATIC(itemsgci, item, entrygci, item_cmp_ci);
RB_GENERATE_STATIC(itemsci, item, entryci, item_cmp_ci);
RB_GENERATE_STATIC(itemscs, item, entrycs, item_cmp_cs);
RB_GENERATE_STATIC(items, item, entry, item_cmp_oid);
#line 2339 "mib.c"

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
case 3:
#line 328 "mib.y"
	{
				file.state = FILE_ASN1;
				module = calloc(1, sizeof(*module));
				if (module == NULL) {
					yyerror("malloc");
					YYERROR;
				}
				RB_INIT(&module->itemscs);
				RB_INIT(&module->itemsci);
				module->resolved = 0;
				if (strlcpy(module->name, yystack.l_mark[-3].string,
				    sizeof(module->name)) >=
				    sizeof(module->name)) {
					yyerror("module name too long");
					free(module);
					YYERROR;
				}
			}
#line 2558 "mib.c"
break;
case 4:
#line 345 "mib.y"
	{
				struct module *mprev;

				if ((mprev = RB_INSERT(modulescs, &modulescs,
				    module)) != NULL) {
					if (module->lastupdated >
					    mprev->lastupdated) {
						mib_modulefree(mprev);
						RB_INSERT(modulescs, &modulescs,
						    module);
						RB_INSERT(modulesci, &modulesci,
						    module);
					} else
						mib_modulefree(module);
				} else 
					RB_INSERT(modulesci, &modulesci, module);
				module = NULL;
			}
#line 2580 "mib.c"
break;
case 5:
#line 363 "mib.y"
	{
				log_debug("%s: SMIv2 definitions: skipping",
				   file.name);
				YYACCEPT;
			}
#line 2589 "mib.c"
break;
case 6:
#line 370 "mib.y"
	{ strlcpy(yyval.string, yystack.l_mark[0].string, sizeof(yyval.string)); }
#line 2594 "mib.c"
break;
case 7:
#line 373 "mib.y"
	{ strlcpy(yyval.string, "SNMPv2-SMI", sizeof(yyval.string)); }
#line 2599 "mib.c"
break;
case 8:
#line 374 "mib.y"
	{ strlcpy(yyval.string, "SNMPv2-CONF", sizeof(yyval.string)); }
#line 2604 "mib.c"
break;
case 9:
#line 375 "mib.y"
	{ strlcpy(yyval.string, "SNMPv2-TC", sizeof(yyval.string)); }
#line 2609 "mib.c"
break;
case 14:
#line 388 "mib.y"
	{
				size_t i;
				char *symbols[SYMBOLS_MAX];

				for (i = 0; yystack.l_mark[-2].symbollist[i][0] != '\0'; i++)
					symbols[i] = yystack.l_mark[-2].symbollist[i];
				symbols[i] = NULL;
					
				if (mib_imports_add(yystack.l_mark[0].string, symbols) == -1)
					YYERROR;
			}
#line 2624 "mib.c"
break;
case 15:
#line 399 "mib.y"
	{
				size_t i;
				char *symbols[SYMBOLS_MAX];

				for (i = 0; yystack.l_mark[-2].symbollist[i][0] != '\0'; i++)
					symbols[i] = yystack.l_mark[-2].symbollist[i];
				symbols[i] = NULL;
					
				if (mib_imports_add(yystack.l_mark[0].string, symbols) == -1)
					YYERROR;

				file.state = FILE_SMI2;
			}
#line 2641 "mib.c"
break;
case 16:
#line 414 "mib.y"
	{
				size_t i;
				for (i = 0; yystack.l_mark[-2].symbollist[i][0] != '\0'; i++)
					strlcpy(yyval.symbollist[i], yystack.l_mark[-2].symbollist[i], sizeof(yyval.symbollist[i]));
				if (i + 1 == nitems(yyval.symbollist)) {
					yyerror("too many symbols from module");
					YYERROR;
				}
				if (strlcpy(yyval.symbollist[i], yystack.l_mark[0].string, sizeof(yyval.symbollist[i])) >=
				    sizeof(yyval.symbollist[i])) {
					yyerror("symbol too long");
					YYERROR;
				}
				yyval.symbollist[i + 1][0] = '\0';
			}
#line 2660 "mib.c"
break;
case 17:
#line 429 "mib.y"
	{
				if (strlcpy(yyval.symbollist[0], yystack.l_mark[0].string, sizeof(yyval.symbollist[0])) >=
				    sizeof(yyval.symbollist[0])) {
					yyerror("symbol too long");
					YYERROR;
				}
				yyval.symbollist[1][0] = '\0';
			}
#line 2672 "mib.c"
break;
case 18:
#line 439 "mib.y"
	{ strlcpy(yyval.string, yystack.l_mark[0].string, sizeof(yyval.string)); }
#line 2677 "mib.c"
break;
case 19:
#line 440 "mib.y"
	{ strlcpy(yyval.string, yystack.l_mark[0].string, sizeof(yyval.string)); }
#line 2682 "mib.c"
break;
case 20:
#line 442 "mib.y"
	{
				strlcpy(yyval.string, "MODULE-IDENTITY", sizeof(yyval.string));
			}
#line 2689 "mib.c"
break;
case 21:
#line 445 "mib.y"
	{
				strlcpy(yyval.string, "OBJECT-IDENTITY", sizeof(yyval.string));
			}
#line 2696 "mib.c"
break;
case 22:
#line 448 "mib.y"
	{ strlcpy(yyval.string, "Integer32", sizeof(yyval.string)); }
#line 2701 "mib.c"
break;
case 23:
#line 449 "mib.y"
	{ strlcpy(yyval.string, "IpAddress", sizeof(yyval.string)); }
#line 2706 "mib.c"
break;
case 24:
#line 450 "mib.y"
	{ strlcpy(yyval.string, "Counter32", sizeof(yyval.string)); }
#line 2711 "mib.c"
break;
case 25:
#line 451 "mib.y"
	{ strlcpy(yyval.string, "Gauge32", sizeof(yyval.string)); }
#line 2716 "mib.c"
break;
case 26:
#line 452 "mib.y"
	{ strlcpy(yyval.string, "Unsigned32", sizeof(yyval.string)); }
#line 2721 "mib.c"
break;
case 27:
#line 453 "mib.y"
	{ strlcpy(yyval.string, "TimeTicks", sizeof(yyval.string)); }
#line 2726 "mib.c"
break;
case 28:
#line 454 "mib.y"
	{ strlcpy(yyval.string, "Opaque", sizeof(yyval.string)); }
#line 2731 "mib.c"
break;
case 29:
#line 455 "mib.y"
	{ strlcpy(yyval.string, "Counter64", sizeof(yyval.string)); }
#line 2736 "mib.c"
break;
case 30:
#line 456 "mib.y"
	{ strlcpy(yyval.string, "OBJECT-TYPE", sizeof(yyval.string)); }
#line 2741 "mib.c"
break;
case 31:
#line 457 "mib.y"
	{
				strlcpy(yyval.string, "NOTIFICATION-TYPE", sizeof(yyval.string));
			}
#line 2748 "mib.c"
break;
case 32:
#line 461 "mib.y"
	{
				strlcpy(yyval.string, "TEXTUAL-CONVENTION", sizeof(yyval.string));
			}
#line 2755 "mib.c"
break;
case 33:
#line 465 "mib.y"
	{
				strlcpy(yyval.string, "OBJECT-GROUP", sizeof(yyval.string));
			}
#line 2762 "mib.c"
break;
case 34:
#line 468 "mib.y"
	{
				strlcpy(yyval.string, "NOTIFICATION-GROUP", sizeof(yyval.string));
			}
#line 2769 "mib.c"
break;
case 35:
#line 471 "mib.y"
	{
				strlcpy(yyval.string, "MODULE-COMPLIANCE", sizeof(yyval.string));
			}
#line 2776 "mib.c"
break;
case 36:
#line 474 "mib.y"
	{
				strlcpy(yyval.string, "AGENT-CAPABILITIES", sizeof(yyval.string));
			}
#line 2783 "mib.c"
break;
case 37:
#line 479 "mib.y"
	{
				if (strlen(yystack.l_mark[0].string) > DESCRIPTOR_MAX) {
					yyerror("descriptor too long");
					YYERROR;
				}
				strlcpy(yyval.string, yystack.l_mark[0].string, sizeof(yyval.string));
			}
#line 2794 "mib.c"
break;
case 38:
#line 490 "mib.y"
	{
				if (mib_moduleidentity(
				    yystack.l_mark[-11].string, yystack.l_mark[-9].time, yystack.l_mark[-7].string, yystack.l_mark[-5].string, yystack.l_mark[-3].string, &yystack.l_mark[0].oid) == -1)
					YYERROR;
			}
#line 2803 "mib.c"
break;
case 39:
#line 497 "mib.y"
	{
				char timebuf[14] = "";
				struct tm tm = {};
				size_t len;

				if ((len = strlen(yystack.l_mark[0].string)) == 11)
					snprintf(timebuf, sizeof(timebuf),
					    "19%s", yystack.l_mark[0].string);
				else if (len == 13)
					strlcpy(timebuf, yystack.l_mark[0].string, sizeof(timebuf));
				else {
					yyerror("Invalid LAST-UPDATED: %s", yystack.l_mark[0].string);
					YYERROR;
				}

				if (strptime(timebuf, "%Y%m%d%H%MZ", &tm) == NULL) {
					yyerror("Invalid LAST-UPDATED: %s", yystack.l_mark[0].string);
					YYERROR;
				}

				if ((yyval.time = mktime(&tm)) == -1) {
					yyerror("Invalid LAST-UPDATED: %s", yystack.l_mark[0].string);
					YYERROR;
				}
			}
#line 2832 "mib.c"
break;
case 47:
#line 540 "mib.y"
	{
				if (mib_oid(yystack.l_mark[-4].string, &yystack.l_mark[0].oid) == NULL)
					YYERROR;
			}
#line 2840 "mib.c"
break;
case 48:
#line 546 "mib.y"
	{
				const char *reference;

				reference = yystack.l_mark[-2].string[0] == '\0' ? NULL : yystack.l_mark[-2].string;

				if (mib_objectidentity(yystack.l_mark[-8].string, yystack.l_mark[-5].status, yystack.l_mark[-3].string, reference,
				    &yystack.l_mark[0].oid) == -1)
					YYERROR;
			}
#line 2853 "mib.c"
break;
case 49:
#line 558 "mib.y"
	{
				const char *units, *reference;

				units = yystack.l_mark[-11].string[0] == '\0' ? NULL : yystack.l_mark[-11].string;
				reference = yystack.l_mark[-4].string[0] == '\0' ? NULL : yystack.l_mark[-4].string;

				if (mib_objecttype(yystack.l_mark[-15].string, NULL, units, yystack.l_mark[-9].access, yystack.l_mark[-7].status, yystack.l_mark[-5].string,
				    reference, NULL, NULL, &yystack.l_mark[0].oid) == -1)
					YYERROR;
			}
#line 2867 "mib.c"
break;
case 50:
#line 570 "mib.y"
	{
				const char *reference;

				reference = yystack.l_mark[-2].string[0] == '\0' ? NULL : yystack.l_mark[-2].string;

				if (mib_notificationtype(yystack.l_mark[-9].string, NULL, yystack.l_mark[-5].status, yystack.l_mark[-3].string,
				    reference, &yystack.l_mark[0].oid) == -1)
					YYERROR;
			}
#line 2880 "mib.c"
break;
case 51:
#line 579 "mib.y"
	{
				/* Table entry, ignore for now */
			}
#line 2887 "mib.c"
break;
case 52:
#line 583 "mib.y"
	{
				const char *displayhint, *reference;

				displayhint = yystack.l_mark[-7].string[0] == '\0' ? NULL : yystack.l_mark[-7].string;
				reference = yystack.l_mark[-2].string[0] == '\0' ? NULL : yystack.l_mark[-2].string;

				if (mib_textualconvetion(yystack.l_mark[-10].string, displayhint, yystack.l_mark[-5].status,
				    yystack.l_mark[-3].string, reference, NULL) == -1)
					YYERROR;
			}
#line 2901 "mib.c"
break;
case 53:
#line 595 "mib.y"
	{
				const char *reference;

				reference = yystack.l_mark[-3].string[0] == '\0' ? NULL : yystack.l_mark[-3].string;

				if (mib_modulecompliance(yystack.l_mark[-9].string, yystack.l_mark[-6].status, yystack.l_mark[-4].string, reference,
				    NULL, &yystack.l_mark[0].oid) == -1)
					YYERROR;
			}
#line 2914 "mib.c"
break;
case 54:
#line 606 "mib.y"
	{
				const char *reference;

				reference = yystack.l_mark[-2].string[0] == '\0' ? NULL : yystack.l_mark[-2].string;

				if (mib_objectgroup(yystack.l_mark[-9].string, NULL, yystack.l_mark[-5].status, yystack.l_mark[-3].string, reference,
				    &yystack.l_mark[0].oid) == -1)
					YYERROR;
			}
#line 2927 "mib.c"
break;
case 55:
#line 617 "mib.y"
	{
				const char *reference;

				reference = yystack.l_mark[-2].string[0] == '\0' ? NULL : yystack.l_mark[-2].string;

				if (mib_notificationgroup(yystack.l_mark[-9].string, NULL, yystack.l_mark[-5].status, yystack.l_mark[-3].string, reference,
				    &yystack.l_mark[0].oid) == -1)
					YYERROR;
			}
#line 2940 "mib.c"
break;
case 72:
#line 658 "mib.y"
	{
				strlcpy(yyval.string, yystack.l_mark[0].string, sizeof(yyval.string));
			}
#line 2947 "mib.c"
break;
case 73:
#line 661 "mib.y"
	{
				yyval.string[0] = '\0';
			}
#line 2954 "mib.c"
break;
case 74:
#line 666 "mib.y"
	{
				strlcpy(yyval.string, yystack.l_mark[0].string, sizeof(yyval.string));
			}
#line 2961 "mib.c"
break;
case 75:
#line 669 "mib.y"
	{
				yyval.string[0] = '\0';
			}
#line 2968 "mib.c"
break;
case 98:
#line 715 "mib.y"
	{
				strlcpy(yyval.string, yystack.l_mark[0].string, sizeof(yyval.string));
			}
#line 2975 "mib.c"
break;
case 99:
#line 718 "mib.y"
	{
				yyval.string[0] = '\0';
			}
#line 2982 "mib.c"
break;
case 133:
#line 802 "mib.y"
	{
				yyval.oid.bo_n = 0;

				if (yystack.l_mark[-2].objidcomponent.type == OCT_DESCRIPTOR) {
					yyerror("only first component of "
					    "OBJECT IDENTIFIER can be a "
					    "descriptor");
					YYERROR;
				}
				if (mib_oid_append(&yyval.oid, &yystack.l_mark[-3].objidcomponent) == -1)
					YYERROR;
				if (mib_oid_append(&yyval.oid, &yystack.l_mark[-2].objidcomponent) == -1)
					YYERROR;
				if (mib_oid_concat(&yyval.oid, &yystack.l_mark[-1].oid) == -1)
					YYERROR;
			}
#line 3002 "mib.c"
break;
case 134:
#line 820 "mib.y"
	{
				if (yystack.l_mark[-1].objidcomponent.type == OCT_DESCRIPTOR) {
					yyerror("only first component of "
					    "OBJECT IDENTIFIER can be a "
					    "descriptor");
					YYERROR;
				}

				yyval.oid.bo_n = 0;
				if (mib_oid_append(&yyval.oid, &yystack.l_mark[-1].objidcomponent) == -1)
					YYERROR;
				if (mib_oid_concat(&yyval.oid, &yystack.l_mark[0].oid) == -1)
					YYERROR;
			}
#line 3020 "mib.c"
break;
case 135:
#line 834 "mib.y"
	{
				yyval.oid.bo_n = 0;
			}
#line 3027 "mib.c"
break;
case 136:
#line 839 "mib.y"
	{
				yyval.objidcomponent.type = OCT_DESCRIPTOR;
				strlcpy(yyval.objidcomponent.name, yystack.l_mark[0].string, sizeof(yyval.objidcomponent.name));
			}
#line 3035 "mib.c"
break;
case 137:
#line 843 "mib.y"
	{
				yyval.objidcomponent.type = yystack.l_mark[0].objidcomponent.type;
				yyval.objidcomponent.number = yystack.l_mark[0].objidcomponent.number;
				strlcpy(yyval.objidcomponent.name, yystack.l_mark[0].objidcomponent.name, sizeof(yyval.objidcomponent.name));
			}
#line 3044 "mib.c"
break;
case 138:
#line 850 "mib.y"
	{
				yyval.objidcomponent.type = OCT_NUMBER;
				if (yystack.l_mark[0].number > UINT32_MAX) {
					yyerror("OBJECT IDENTIFIER number "
					    "too large");
					YYERROR;
				}
				yyval.objidcomponent.number = yystack.l_mark[0].number;
				yyval.objidcomponent.name[0] = '\0';
			}
#line 3058 "mib.c"
break;
case 139:
#line 860 "mib.y"
	{
				yyval.objidcomponent.type = OCT_NAMEANDNUMBER;
				if (yystack.l_mark[-1].number > UINT32_MAX) {
					yyerror("OBJECT IDENTIFIER number "
					    "too large");
					YYERROR;
				}
				yyval.objidcomponent.number = yystack.l_mark[-1].number;
				strlcpy(yyval.objidcomponent.name, yystack.l_mark[-3].string, sizeof(yyval.objidcomponent.name));
			}
#line 3072 "mib.c"
break;
case 179:
#line 935 "mib.y"
	{
				if (strcmp(yystack.l_mark[0].string, "not-accessible") == 0)
					yyval.access = NOTACCESSIBLE;
				else if (
				    strcmp(yystack.l_mark[0].string, "accessible-for-notify") == 0)
					yyval.access = ACCESSIBLEFORNOTIFY;
				else if (strcmp(yystack.l_mark[0].string, "read-only") == 0)
					yyval.access = READONLY;
				else if (strcmp(yystack.l_mark[0].string, "read-write") == 0)
					yyval.access = READWRITE;
				else if (strcmp(yystack.l_mark[0].string, "read-create") == 0)
					yyval.access = READCREATE;
				else {
					yyerror("invalid access");
					YYERROR;
				}
			}
#line 3093 "mib.c"
break;
case 180:
#line 954 "mib.y"
	{
				if (strcmp(yystack.l_mark[0].string, "current") == 0)
					yyval.status = CURRENT;
				else if (strcmp(yystack.l_mark[0].string, "deprecated") == 0)
					yyval.status = DEPRECATED;
				else if (strcmp(yystack.l_mark[0].string, "obsolete") == 0)
					yyval.status = OBSOLETE;
				else {
					yyerror("invalid status");
					YYERROR;
				}
			}
#line 3109 "mib.c"
break;
#line 3111 "mib.c"
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
