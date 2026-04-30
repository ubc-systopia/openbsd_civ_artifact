## Email

Dear OpenBSD security team,

We found two more mid-step vectors similar to the ones we reported earlier.
Which are in OpenSMTPd and httpd.

This report discusses only *httpd*. the OpenSMTPd one will be sent to
opensmtpd-security@.

In httpd, the privileged privsep process sends a data structure
containing its
own pointer to unprivileged privsep processes in message IMSG_CFG_AUTH.
If the
unprivileged privsep process is compromised, this leaked pointer could
be used
to break the ASLR provided by fork+exec.  We believe this does not adhere to
the privsep design and could be used as a mid-step attack vector.

The affected code resides in functions `config_setauth` and
`config_getauth` in
config.c. The former is used to send the entire `struct auth` by the
privileged
parent as payload, and the latter is called by the unprivileged children to
receive that payload. Because `struct auth` is defined as a node of
TAILQ, it
will contain two pointers pointing to the previous and next in the
linked list,
and those were sent along with useful data.

[Proof-of-concept details removed.]

Regards,
Researcher A and Researcher B, from the Research Group X

[0] https://www.openbsdhandbook.com/services/webserver/basic_webserver/
