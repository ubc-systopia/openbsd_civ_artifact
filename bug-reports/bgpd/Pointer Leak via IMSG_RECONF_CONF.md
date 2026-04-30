## Email

Dear OpenBSD Security Team,

This email reports a privilege separation interface bug we discovered
in bgpd, where the privileged compartment leaks pointers to
unprivileged compartments, along with a proposed fix.

[Proof-of-concept details removed.]

We understand that all pointer fields reference heap-allocated
objects and therefore the security impact is limited, but we still
recommend fixing this as a best practice.

We observed that all compartments receiving `struct bgpd_config`
call copy_config() (config.c:77) to copy the configuration struct,
excluding all pointer fields. Therefore our fix applies
copy_config() to sanitize the struct before sending, ensuring no
pointers are transmitted across compartment boundaries.

Thank you for the amazing privsep work!

Best regards,
Researcher A, Researcher C, Researcher B,
Research Group X
