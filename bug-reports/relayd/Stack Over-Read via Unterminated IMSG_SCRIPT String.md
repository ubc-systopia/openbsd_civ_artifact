## Email

Dear OpenBSD security team,

We are researchers at the Institution X, studying
vulnerabilities around the privsep compartmentalization boundary. We
discover several vulnerabilities involving three different userspace
software (relayd, OpenBGPd, and radiusd) and would like to hear your
opinion.

This email discusses the vulnerability in *relayd* while the others
are reported in separate emails to facilitate tracking.

*relayd* Potential Vulnerability
--------------------------------

We discover that relayd's privileged compartment fails to ensure the
string received from the unprivileged compartment is properly NULL-
terminated. This allows a compromised less-privileged compartment to
trigger a stack buffer over-read in the privileged compartment or even
print the over-read stack content into the log. We believe this design
does not adhere to the least-privilege design principle and partially
compromises the protection that compartmentalization should provide.

[Proof-of-concept details removed.]

(A Minor) Compilation Issue
---------------------------

The code does not compile if DEBUG is set to be greater than 1 due to
an erroneous expression in config.c. We comment that out in the patch.

Similar Vulnerabilities
-----------------------

Other message handlers in the privileged compartment exhibit similar
vulnerabilities, which could potentially make extracting the over-read
data even easier. For instance, the handler for IMSG_DEMOTE does not
validate proper string termination. This handler invokes the
`carp_demote_set` function, which logs the over-read data into the log
file without requiring verbose printing and DEBUG > 1.

Impact
------

This vulnerability *seems* relatively low impact: the log is likely
only readable by wheel users, and a DoS could occur if the over-read
triggers memory errors. However, we still think this should be patched
considering relayd is an internet-facing software and could be subject
to more frequent attacks.

Thank you for maintaining such a fantastic security-focused OS. It's
a pleasure studying OpenBSD's security measures and design.

Looking forward to hearing your opinion.

Regards,
Researcher A & Researcher B, from the Research Group X
