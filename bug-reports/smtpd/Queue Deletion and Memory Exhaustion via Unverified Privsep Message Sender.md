## Email

Dear OpenSMTPd team,

We are researchers at the Institution X, inspecting
OpenSMTPd
for mid-step attack vectors that could be used to bypass privsep boundaries.
We have identified potential mid-step vectors in OpenSMTPd and would like to
hear your opinion. We previously reported other vectors of this kind in
other
daemons to security@openbsd.org.

Mid-step vector #1
------------------

OpenSMTPd does not verify the sender of messages sent across privsep
processes,
allowing a compromised process to send messages that perform operations
beyond
its originally designed capabilities. We believe this design does not
adhere to
the least-privilege design principle and partially compromises the
protection
that privsep should provide.

A potential use of this mid-step vector involves a privsep process that does
not manage the queue, such as lka. If compromised, it allows an attacker to
send queue-deletion messages to the queue privsep process and delete the
entire
queue. This scenario is described as something that should not happen on
slide
25 of this presentation [0].

Fortunately, the attacker needs to guess the randomly generated ID to
perform a
deletion. However, since the ID is only a 32-bit integer, the attacker could
still delete all messages by brute-forcing the entire range.

[Proof-of-concept details removed.]

The lka privsep process will begin attempting to delete messages after
running the
"smtpctl show routes" command.

Mid-step vector #2
------------------

[Proof-of-concept details removed.]

This shows that a compromised privsep process can cause memory exhaustion in
other privsep processes. We think that this may constitute another mid-step
vector - not sure if this is something you would be interested in fixing.

Impact
------

Both mid-step vectors seem low impact: DoS assuming a low-importance privsep
process is compromised. Still, mid-step vector 1 seems like a relatively
dangerous primitive that probably should be patched in future releases of
OpenSMTPd?

Thank you again for this great piece of software. It is a pleasure reading
OpenSMTPd code.

Regards,
Researcher A & Researcher B, from the Research Group X

[0]
https://www.opensmtpd.org/presentations/eurobsdcon2017-smtpd/eurobsdcon2017-opensmtpd.pdf
