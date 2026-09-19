## Email

Dear OpenBSD Security Team,

This email reports a privsep interface bug in dvmrpd. The root parent
copies a four-byte `int` from an `IMSG_CTL_LOG_VERBOSE` message without
checking its payload length (dvmrpd.c:351-353). Although this verbose
value comes from the control socket and is checked by `dvmrpe`,
a misbehaving `dvmrpe` can send a malformed message directly to the
parent, causing memory errors.

This is similar to the issues we previously reported in eigrpd and
ospfd.

Our fix adds the missing payload-length check.

[Proof-of-concept details removed.]

Happy to answer any questions!

Best regards,
Shibo, Hugo
Systopia Team

```c
dvmrpd.c
351         case IMSG_CTL_LOG_VERBOSE:
352             /* already checked by dvmrpe */
353             memcpy(&verbose, imsg.data, sizeof(verbose));
```
