#define bpf_timeval timeval
#define MPLS_LABEL_MAX		((1 << 20) - 1)
#define __dead __attribute__((noreturn))
#define		BPF_RND		0xc0
#define MPLS_LABEL_MASK		__MADDR(0xfffff000U)
#define BPF_DIRECTION_OUT	(1 << 1)
#define BPF_DIRECTION_IN	(1 << 0)
#define BIOCSDIRFILT	_IOW('B',125, u_int)
#define __MADDR(x) ((u_int32_t)(x))
#define SA_LEN(x) ((x)->sa_len)
