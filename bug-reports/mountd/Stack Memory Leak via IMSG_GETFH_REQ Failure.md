## Email

Dear OpenBSD Security Team,

This email reports mid-step vectors in *mountd*. While both privsep
processes run as root, this may still be worth addressing as one
of them is de-privileged with pledge.

[Proof-of-concept details removed.]

The privileged privsep process also misses some checks to ensure
strings from the unprivileged privsep process are null-terminated.

Regards,
Researcher A and Researcher B,
Research Group X
