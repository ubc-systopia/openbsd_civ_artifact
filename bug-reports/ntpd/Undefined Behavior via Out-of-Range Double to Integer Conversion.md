## Email

Dear OpenBSD Team,

We have found several privsep interface bugs in `ntpd`, where the
unprivileged `ntp` compartment can trigger undefined behavior inside
the privileged compartment. We think that these bugs have *no security
impact* but are still worth fixing, thus sending this report to
bugs@.

The privileged compartment accepts multiple messages from the
unprivileged `ntp` compartment, where the payload is a `double`
arithmetic value. We identified several cases where the privileged
parent performs a lossy conversion from `double` to `long` to extract
the integer part of the `double`. This conversion is well-defined only
when the `double` value is between `LONG_MIN` and `LONG_MAX`.
However, the privileged compartment does not verify whether the
`double` is in this range before the conversion to a `long`, allowing
the unprivileged `ntp` compartment to send a value outside this range,
thus triggering undefined behavior inside the privileged compartment.

[Proof-of-concept details removed.]

We believe these bugs are not exploitable to escalate privileges, as
with most reasonable compilers they will only result in corrupting the
`double` value sent by the unprivileged compartment, which the
unprivileged compartment already controls. Nonetheless, we consider
this issue worth addressing as we think that unprivileged compartments
should not be able to trigger undefined behavior in privileged
parents.

For context, we assume that the unprivileged compartment has been
compromised and can send arbitrary messages to the privileged
compartment to trigger interface bugs that would allow it to escalate
privileges. We found this bug while looking for such interface bugs.

Regards,
Researcher A, Researcher C, Researcher B
Research Group X

```c
ntpd.c
395  double                   d;
...
408  case IMSG_ADJTIME:
409          if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(d))
410                  fatalx("invalid IMSG_ADJTIME received");
411          memcpy(&d, imsg.data, sizeof(d));
412          n = ntpd_adjtime(d);
413          imsg_compose(ibuf, IMSG_ADJTIME, 0, 0, -1,
414               &n, sizeof(n));
415          break;
...
465 ntpd_adjtime(double d)
466 {
467         struct timeval  tv, olddelta;
468         int             synced = 0;
469         static int      firstadj = 1;
470
471         d += getoffset();
472         if (d >= (double)LOG_NEGLIGIBLE_ADJTIME / 1000 ||
473             d <= -1 * (double)LOG_NEGLIGIBLE_ADJTIME / 1000)
474                 log_info("adjusting local clock by %fs", d);
475         else
476                 log_debug("adjusting local clock by %fs", d);
477         d_to_tv(d, &tv);
...
486 void
487 ntpd_adjfreq(double relfreq, int wrlog)
488 {
489         int64_t curfreq;
490         double ppmfreq;
491         int r;
492
493         if (adjfreq(NULL, &curfreq) == -1) {
494                 log_warn("adjfreq failed");
495                 return;
496         }
497
498         /*
499          * adjfreq's unit is ns/s shifted left 32; convert relfreq to
500          * that unit before adding. We log values in part per million.
501          */
502         curfreq += relfreq * 1e9 * (1LL << 32);
```

```c
util.c
76 void
77 d_to_tv(double d, struct timeval *tv)
78 {
79         tv->tv_sec = d;
80         tv->tv_usec = (d - tv->tv_sec) * 1000000;
81         while (tv->tv_usec < 0) {
82                 tv->tv_usec += 1000000;
83                 tv->tv_sec -= 1;
84         }
85 }
```
