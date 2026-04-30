## Email

Dear OpenBSD Security Team,

This email reports a privsep interface bug in relayd that allows the
unprivileged hce compartment to trigger an out-of-bounds array access
and segmentation fault in the privileged parent compartment.

[Proof-of-concept details removed.]

Regards,
Researcher A, Researcher C, Researcher B
Research Group X
