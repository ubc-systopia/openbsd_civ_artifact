#define __dead __attribute__((noreturn))
#define pledge(...) (0)
#define unveil(...) (0)
#define XDR_CAST(proc) ((xdrproc_t)(uintptr_t)(proc))
#define freezero(ptr, size) free(ptr)

#define DEF_WEAK(svc_getreq_poll)
#define howmany(x, y)  (((x) + ((y) - 1)) / (y))

#define MOUNT_FFS       "ffs"           /* UNIX "Fast" Filesystem */
#define MOUNT_UFS       MOUNT_FFS       /* for compatibility */
#define MOUNT_NFS       "nfs"           /* Network Filesystem */
#define MOUNT_MFS       "mfs"           /* Memory Filesystem */
#define MOUNT_MSDOS     "msdos"         /* MSDOS Filesystem */
#define MOUNT_AFS       "afs"           /* Andrew Filesystem */
#define MOUNT_CD9660    "cd9660"        /* ISO9660 (aka CDROM) Filesystem */
#define MOUNT_EXT2FS    "ext2fs"        /* Second Extended Filesystem */
#define MOUNT_NCPFS     "ncpfs"         /* NetWare Network File System */
#define MOUNT_NTFS      "ntfs"          /* NTFS */
#define MOUNT_UDF       "udf"           /* UDF */
#define MOUNT_TMPFS     "tmpfs"         /* tmpfs */
#define MOUNT_FUSEFS    "fuse"          /* FUSE */

#define export_info export

#if 0
struct xucred {
	u_int   cr_version;             /* structure layout version */
	uid_t   cr_uid;                 /* effective user id */
	short   cr_ngroups;             /* number of groups */
	gid_t   cr_groups[XU_NGROUPS];  /* groups */
	union {
		void    *_cr_unused1;   /* compatibility with old ucred */
		pid_t   cr_pid;
	};
};

struct oexport_args {
	int     ex_flags;               /* export related flags */
	uid_t   ex_root;                /* mapping for root uid */
	struct  xucred ex_anon;         /* mapping for anonymous user */
	struct  sockaddr *ex_addr;      /* net address to which exported */
	u_char  ex_addrlen;             /* and the net address length */
	struct  sockaddr *ex_mask;      /* mask of valid bits in saddr */
	u_char  ex_masklen;             /* and the smask length */
	char    *ex_indexfile;          /* index file for WebNFS URLs */
};
#endif



#if 0
struct ufs_args {
	char    *fspec;                 /* block special device to mount */
	struct  oexport_args export_info;    /* network export information */
};
#endif


// typedef bool_t (*resultproc_t)(caddr_t, struct sockaddr_in *);



/*
 *
#define LOG_MAXLINE	8192	
#define _PATH_LOGPID    "/var/run/syslog.pid"
#define _NSIG	33
#define	IPPROTO_UDP		17
#define IPPROTO_TCP	6
#define _PATH_LOGCONF   "/etc/syslog.conf"
#define HOST_NAME_MAX 255
#define _PATH_KLOG      "/dev/klog"
#define	LIOCSFD		_IOW('l', 127, int)
#define	KERN_MSGBUFSIZE		38
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
 * */
