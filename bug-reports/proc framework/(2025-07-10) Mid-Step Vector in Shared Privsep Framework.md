Dear OpenBSD Security Team,

This email reports a mid-step vector in the common privsep framework used by
relayd, httpd, iked, snmpd, and vmd.

In the privsep framework, the IMSG_CTL_PROCFD message handler uses array
indices inside the message payload (packed as `struct privsep_fd`, used as
`dst` and `n` variables in the code below) to access an array called
`ps_ievs` inside `struct privsep`. However, there is no validation of the
range of these indices. This allows the sender to trigger out-of-bounds
array accesses in the message receiver compartment.

We understand that IMSG_CTL_PROCFD is designed to be sent from the
privileged compartment to the unprivileged compartments. However, we found
that the privileged compartment also accepts and processes this message,
as it is a generic message and is handled in a dispatcher shared across
all compartments. Therefore, a compromised unprivileged compartment could
exploit this mid-step vector to trigger out-of-bounds array accesses in the
privileged compartment.

[Proof-of-concept details removed.]

The `proc_accept` function in `proc.c`, part of the IMSG_CTL_PROCFD message
handler:

```c
 >  /* `dst` and `n` can be arbitrary integers whose values are
controlled by  */
 >  /* the compromised unprivileged message sender */
 >  iev = &ps->ps_ievs[dst][n]; // possibly out-of-bound
 >  if (imsgbuf_init(&iev->ibuf, fd) == -1)
 >      fatal("imsgbuf_init");
 >  imsgbuf_allow_fdpass(&iev->ibuf);
 >  /* `iev->handler` could be data read from out-of-bounds access */
 >  event_set(&iev->ev, iev->ibuf.fd, iev->events, iev->handler, iev->data);
 >  event_add(&iev->ev, NULL);
 ```

Fortunately, to mount such an attack, the attacker would need knowledge of
the memory layout of the privileged compartment, which is mitigated by the
per-compartment ASLR provided by fork+exec. However, we still believe this
issue is worth fixing. We suggest adding boundary checks and entirely
blocking the privileged compartment from receiving and processing this
message to resolve the problem.

We discovered this mid-step vector using a fuzzer that we are currently
developing within Institution X specifically to target privsep interfaces. We have
carefully reviewed this bug to ensure that it is not a false positive.

Thank you for maintaining such an awesome security-focused OS!

Regards,
Researcher A, Researcher C, Researcher B
Research Group X
