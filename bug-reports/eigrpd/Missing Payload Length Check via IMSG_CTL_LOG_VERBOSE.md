## Email

Dear OpenBSD Security Team,

This email reports a privsep interface bug in eigrpd where the privileged
parent (running as root) does not validate the payload length of an
IMSG_CTL_LOG_VERBOSE message received from the unprivileged eigrpe engine,
which runs as a non-root user. We also provide a patch that fixes this issue
and four pointer leaks previously reported in eigrpd.

In the privileged handler for IMSG_CTL_LOG_VERBOSE, the privileged parent
copies a four-byte verbose value from imsg.data without first checking the
payload length. Although the request normally comes from eigrpctl and is
validated by the eigrpe engine before being forwarded, the unprivileged engine
compartment can still send an invalid message directly to the privileged
compartment.

[Proof-of-concept details removed.]

The patch contains two fixes: 1) validating the payload length before copying
the verbose value, and 2) sanitizing the pointer-bearing messages sent by
main_imsg_send_config() for struct eigrpd_conf, struct eigrp, struct iface,
and struct eigrp_iface.

Happy to answer any questions!

Best regards,
Researcher A, Researcher B
Research Group X
