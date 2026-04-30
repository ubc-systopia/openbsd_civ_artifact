## Email

Dear OpenBSD security team,

We are researchers at the Institution X, studying
vulnerabilities around the privsep compartmentalization boundary. We
found several vulnerabilities involving three different userspace
software (relayd, OpenBGPd and radiusd), and would like to hear
your opinion.

This email discusses the vulnerability in *radiusd* while the others
are reported in separate emails to facilitate tracking.

*radiusd* Potential Vulnerability
---------------------------------

We observed the mechanism in the privileged compartment to ensure the
string received from the unprivileged compartment is NULL-terminated
can be bypassed if the unprivileged compartment crafts the message in
a specific way.

The vulnerable code is located in the handler for
IMSG_BSDAUTH_USERCHECK in radiusd_bsdauth.c. If the message sent by
the unprivileged compartment sets args->userlen or args->passlen to
*0*, the manually inserted \0 in the privileged compartment is placed
one byte before the start of the string. This allows a compromised
unprivileged compartment to trigger buffer over-read behaviors in the
privileged compartment. We believe this design does not adhere to the
least-privilege principle and partially compromises the protection
that compartmentalization should provide.

[Proof-of-concept details removed.]

Impact
------

This vulnerability *appears* to have relatively low impact, resulting in
Denial of Service (DoS) if the unprivileged compartment is
compromised. However, it demonstrates that the manually added \0 can
be bypassed and we believe this should be patched in future releases of
radiusd.

Thank you for maintaining such a fantastic security-focused OS. It's been
a pleasure studying OpenBSD's security measures and design.

We look forward to hearing your opinion.

Regards,
Researcher A & Researcher B
Research Group X
