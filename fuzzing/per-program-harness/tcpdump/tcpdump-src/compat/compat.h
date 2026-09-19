#include <stdint.h>    // for uint32_t
#include <arpa/inet.h> // for htonl()
#include <endian.h>
#define __dead __attribute__((noreturn))
#define	HW_PHYSMEM64		19
#define	RT_TABLEID_MAX		255
#define	KERN_MAXCLUSTERS	67
#define	NET_RT_TABLE	5
#define freezero(ptr, size) free(ptr)
#define betoh64(x) be64toh(x)
#define HOST_NAME_MAX 255
#define ether_addr_octet octet
#define _NSIG	33
#define ETHERTYPE_NSH 0x894f
#define	ETHERTYPE_EAPOL		0x888E
#define ETHERTYPE_NHRP 0x2001
#define MAXHOSTNAMELEN 256
#define AF_MPLS         33 

#define M_AUTH		0x0800
#define M_CONF		0x0400
#define	IEEE80211_FC0_SUBTYPE_QOS		0x80
#define NAMESERVER_PORT	53

#define pledge(...) (0)
#define unveil(...) (0)

#ifndef letoh16
#define letoh16(x) (le16toh(x))
#endif

#ifndef letoh32
#define letoh32(x) (le32toh(x))
#endif

#ifndef letoh64
#define letoh64(x) (le64toh(x))
#endif

/* Provide missing macros for big-endian to host conversions */
#ifndef betoh16
#define betoh16(x) (be16toh(x))
#endif

#ifndef betoh32
#define betoh32(x) (be32toh(x))
#endif

#ifndef betoh64
#define betoh64(x) (be64toh(x))
#endif

/* Define swap macros using built-in byte swap if not already defined */
#ifndef swap16
#define swap16(x) __builtin_bswap16(x)
#endif

#ifndef swap32
#define swap32(x) __builtin_bswap32(x)
#endif

#ifndef swap64
#define swap64(x) __builtin_bswap64(x)
#endif

#define SA_LEN(x) ((x)->sa_len)

/*
#define IPV6_MINHOPCOUNT 65
#define TCP_SACK_ENABLE         0x08
#define PATH_MAX 1024

#define DEF_WEAK(x)
#define WRAP(x)(...)



#include <stdlib.h>
#include <sys/queue.h>
#include <imsg.h>

#include "util.h"

#define pledge(...) (0)
#define unveil(...) (0)

int getdtablecount(void);
int
fmt_scaled(long long number, char *result);

int
bcrypt_checkpass(const char *pass, const char *goodhash);
int
bcrypt_newhash(const char *pass, int log_rounds, char *hash, size_t hashlen);
int
_bcrypt_autorounds(void);
int
crypt_checkpass(const char *pass, const char *goodhash);
int
crypt_newhash(const char *pass, const char *pref, char *hash, size_t hashlen);
char *
bcrypt(const char *pass, const char *salt);
*/
