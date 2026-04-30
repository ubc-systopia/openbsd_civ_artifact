## Title

OpenBSD: Heap Over-Read
FreeBSD: Stack/Heap Over-Read and Over-Write Due to insifficient verification on the size of struct sockaddr

## Email

Dear OpenBSD Security Team,

This email reports several privsep interface bugs found in `iked`, all
of which are caused by the improper validation of the sender-provided
`sa_len` field in `struct sockaddr`.  On OpenBSD, these bugs can trigger
a stack/heap over-read in the privileged parent compartment. The same
bugs exist in OpenIKED-Portable, but can also trigger a stack/heap
over-write in addition to the over-reads.  This was confirmed on FreeBSD.

These bugs manifest themselves in the following 4 functions:
`vroute_getaddr`
`vroute_getdns`
`vroute_getroute`
`vroute_getcloneroute`

In other words, any of the accepted `imsg` types in
`parent_dispatch_ikev2` (iked.c:489) will be able to trigger the bug.

The common issue across all four problematic functions lies in the fact
that there’s no check to ensure `sa_len` accurately reflects the actual
size of the `struct sockaddr` as determined by the socket's protocol
family. This allows an attacker to manipulate both the message length
and `sa_len`, leading to buffer over-reads and buffer over-writes.

We use `vroute_getdns` (vroute.c:342) to demonstrate the *over-read*
privsep interface bug:

The program performs the following checks:
Check 1)
The received message is at least `sizeof(struct sockaddr)` bytes long
(vroute.c:353)
Check 2)
The message is at least `sa_len` bytes, where `sa_len` is the length
declared in the `sockaddr` structure (vroute.c:357)
Check 3)
The total message size is `sa_len + sizeof(ifidx)` (vroute.c:362)

Then, the pointer to the `sockaddr` is cast to a specific `struct
sockaddr_*` based on its `sa_family` (util.c:255 for `AF_INET6`) and
used in a `memcpy` to access fields specific to that family (util.c:258).

Now, consider a maliciously crafted message:
1) The message size is exactly `sizeof(struct sockaddr)` (16 bytes),
passing check (1)
2) Interpret the entire message as a struct sockaddr instance, where
`sa_len == sizeof(struct sockaddr) - sizeof(ifidx)` to pass checks (2)
and (3)
3) The `sa_family` is set to `AF_INET6` to trigger over-read

All three checks pass. However, because `sa_family` is `AF_INET6`, the
16-byte message is being cast to `struct sockaddr_in6` (util.c:255), and
later accesses 16 bytes starting from `&a6->sin6_addr.s6_addr`
(util.c:258, a6 is the casted pointer to AF_INT6). This exceeds the end
of the message buffer, resulting in a buffer over-read. This could be
prevented by verifying that `sa_len` accurately reflects the expected
size of the `struct sockaddr_*` based on its `sa_family`.

[Proof-of-concept details removed.]

For example, in `vroute_getcloneroute` (vroute.c:647), if `sa_len` is
set to a value larger than the size of the destination buffer (a struct
sockaddr_storage), and the message length is also at least `sa_len +
sizeof(rdomain)`, all the checks pass, leading to a buffer over-write
upon memcpy (vroute.c:678).

This exact issue was observed when running OpenIKED-Portable on FreeBSD.
(Note: `vroute_getdns` is only available on OpenBSD.) Specifically:
- By setting a larger than expected `sa_len` and calling
`vroute_getaddr`, a subsequent call to `vroute_insertaddr`
(vroute.c:317) can trigger a heap over-write (vroute.c:481).
- By setting a larger than expected `sa_len` and calling
`vroute_getcloneroute`, a stack over-write occurs (vroute.c:690).
- By setting a larger than expected `sa_len` and calling
`vroute_getroute`, a subsequent call to `vroute_insertroute`
(vroute.c:651) can trigger a heap over-write (vroute.c:395).

However, we could not reproduce these over-writes on OpenBSD because
`sizeof(struct sockaddr_storage)` is 256 on OpenBSD, while the maximum
value of `sa_len` is 255 since it is only 1 byte.  Since in all these
cases we are writing into a struct that is at least `sizeof(struct
sockaddr_storage)`, we cannot over-write it even with the maximum
`sa_len` value.  On FreeBSD, `sizeof(struct sockaddr_storage)` is only
128 bytes, thus we can cause the over-write.

Interestingly, OpenIKED-Portable does define a `SA_LEN` macro
(openbsd-compat.h:112-120) that helps determine the actual size of
`struct sockaddr` according to its socket protocol family.

Lastly, while all these bugs occur due to an invalid `sa_len` value, it
should be noted that other message fields provided by the sender could
also be interpreted as an invalid length value.  We only saw this in
FreeBSD, where the `struct sockaddr` was cast to a `struct sockaddr_dl`
in the `getnameinfo_link` function provided by FreeBSD’s `libc`
(getnameinfo.c:445-446).  This then causes a stack over-read when
calling `memcpy` using the larger-than-expected `sdl_nlen` value from
`struct sockaddr_dl` (getnameinfo.c:468).  The `sdl_nlen` value
corresponds to a single byte with an offset different from `sa_len`
within the sender’s message.  This illustrates how difficult it is to
determine which message fields require validation and how they are used
in later function calls.

Thanks for reading through this long email. Please do not hesitate to
reach out for any questions. Thank you!

Regards,

Researcher C, Researcher A, Researcher B
Research Group X

```c
iked.c
488 int
489 parent_dispatch_ikev2(int fd, struct privsep_proc *p, struct imsg *imsg)
490 {
491     struct iked *env = iked_env;
492
493     switch (imsg->hdr.type) {
494     case IMSG_IF_ADDADDR:
495     case IMSG_IF_DELADDR:
496         return (vroute_getaddr(env, imsg));
497     case IMSG_VDNS_ADD:
498     case IMSG_VDNS_DEL:
499         return (vroute_getdns(env, imsg));
500     case IMSG_VROUTE_ADD:
501     case IMSG_VROUTE_DEL:
502         return (vroute_getroute(env, imsg));
503     case IMSG_VROUTE_CLONE:
504         return (vroute_getcloneroute(env, imsg));
505     default:
506         return (-1);
507     }
508
509     return (0);
510 }

vroute.c
341 int
342 vroute_getdns(struct iked *env, struct imsg *imsg)
343 {
344     struct sockaddr     *dns;
345     uint8_t             *ptr;
346     size_t              left;
347     int                 add;
348     unsigned int        ifidx;
349
350     ptr = imsg->data;
351     left = IMSG_DATA_SIZE(imsg);
352
353     if (left < sizeof(*dns))
354         fatalx("bad length imsg received");
355
356     dns = (struct sockaddr *) ptr;
357     if (left < dns->sa_len)
358         fatalx("bad length imsg received");
359     ptr += dns->sa_len;
360     left -= dns->sa_len;
361
362     if (left != sizeof(ifidx))
363         fatalx("bad length imsg received");
364     memcpy(&ifidx, ptr, sizeof(ifidx));
365     ptr += sizeof(ifidx);
366     left -= sizeof(ifidx);
367
368     add = (imsg->hdr.type == IMSG_VDNS_ADD);
369     if (add) {
370         vroute_insertdns(env, ifidx, dns);
371     } else {
372         vroute_removedns(env, ifidx, dns);
373     }
374
375     return (vroute_dodns(env, dns, add, ifidx));
376 }
...
646 int
647 vroute_getcloneroute(struct iked *env, struct imsg *imsg)
648 {
649     struct sockaddr     *dst;
650     struct sockaddr_storage  dest;
651     struct sockaddr_storage  mask;
652     struct sockaddr_storage  addr;
653     uint8_t         *ptr;
654     size_t           left;
655     uint32_t         rdomain;
656     int          flags;
657     int          addrs;
658     int          need_gw;
659
660     ptr = (uint8_t *)imsg->data;
661     left = IMSG_DATA_SIZE(imsg);
662
663     if (left < sizeof(rdomain))
664         return (-1);
665     rdomain = *ptr;
666     ptr += sizeof(rdomain);
667     left -= sizeof(rdomain);
668
669     bzero(&dest, sizeof(dest));
670     bzero(&mask, sizeof(mask));
671     bzero(&addr, sizeof(addr));
672
673     if (left < sizeof(struct sockaddr))
674         return (-1);
675     dst = (struct sockaddr *)ptr;
676     if (left < dst->sa_len)
677         return (-1);
678     memcpy(&dest, dst, dst->sa_len);

util.c
254     case AF_INET6:
255         a6 = (struct sockaddr_in6 *)a;
256         b6 = (struct sockaddr_in6 *)b;
257
258         memcpy(&av, &a6->sin6_addr.s6_addr, 16);
259         memcpy(&bv, &b6->sin6_addr.s6_addr, 16);

openbsd-compat.h
112 #define SA_LEN(sa)                                              \
113     ((sa->sa_family == AF_INET)  ? sizeof(struct sockaddr_in) : \
114     (sa->sa_family == AF_INET6) ? sizeof(struct sockaddr_in6) : \
115     sizeof(struct sockaddr))
116 #define SS_LEN(ss)                                      \
117     ((ss.ss_family == AF_INET)  ? sizeof(struct sockaddr_in) :  \
118     (ss.ss_family == AF_INET6) ? sizeof(struct sockaddr_in6) :  \
119     sizeof(struct sockaddr_storage))
120 #endif

freebsd-src/lib/libc/net/getnameinfo.c
440 static int
441 getnameinfo_link(const struct afd *afd,
442     const struct sockaddr *sa, socklen_t salen,
443     char *host, size_t hostlen, char *serv, size_t servlen, int flags)
444 {
445     const struct sockaddr_dl *sdl =
446         (const struct sockaddr_dl *)(const void *)sa;
...
468         memcpy(host, sdl->sdl_data, sdl->sdl_nlen);
```
