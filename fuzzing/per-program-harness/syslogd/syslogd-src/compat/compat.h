
#define LOG_MAXLINE	8192	
#define __dead __attribute__((noreturn))
#define pledge(...) (0)
#define unveil(...) (0)
#define _PATH_LOGPID    "/var/run/syslog.pid"
#define _NSIG	33
#define	IPPROTO_UDP		17
#define IPPROTO_TCP	6
#define _PATH_LOGCONF   "/etc/syslog.conf"
#define HOST_NAME_MAX 255
#define _PATH_KLOG      "/dev/klog"
#define	LIOCSFD		_IOW('l', 127, int)
#define	KERN_MSGBUFSIZE		38

int		getdtablecount(void);

/*
#define freezero(ptr, size) free(ptr)
#define DEF_WEAK(x)
#define WRAP(x)(...)



#define IPV6_MINHOPCOUNT 65
#define	TCP_SACK_ENABLE		0x08
#define PATH_MAX 1024
#include <stdlib.h>
#include <sys/queue.h>
#include <imsg.h>

#include "util.h"


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
