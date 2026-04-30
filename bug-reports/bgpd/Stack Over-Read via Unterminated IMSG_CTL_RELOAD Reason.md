## Email

Dear OpenBSD security team,

We are researchers at the Institution X, studying
vulnerabilities around the privsep compartmentalization boundary. We
found several vulnerabilities involving three different userspace
software (relayd, OpenBGPd and radiusd), and would like to hear
your opinion.

This email discusses the vulnerability in *OpenBGPd* while the others
are reported in separate emails to facilitate tracking.

*OpenBGPd* Potential Vulnerability
----------------------------------

We found that OpenBGPd's privileged compartment does not properly
validate whether strings received from the unprivileged compartment are
correctly null-terminated. This allows a compromised unprivileged
compartment to trigger stack buffer over-read behaviors in the privileged
compartment, and in some cases, print the over-read stack data to the log.
We believe this design does not adhere to the least-privilege design
principle and partially compromises the protection that
compartmentalization should provide.

[Proof-of-concept details removed.]

Similar Vulnerabilities
-----------------------

We also observed similar vulnerabilities in other message handlers of
the bgpd
privileged compartment. For example, the handler for IMSG_DEMOTE also does
not check for string terminators.

Impact
------

This vulnerability *seems* relatively low impact: the default configuration
only allows wheel users to read the log file that could contain leaked
stack data,
and DoS if over-read triggers memory faults. However, we still consider the
vulnerability should be patched considering that bgpd is running in many
internet-critical hardware.

Thank you for maintaining such a fantastic security-focused OS. It's been
a pleasure studying OpenBSD's security measures and design.

Looking forward to hearing your opinion.

Regards,
Researcher A & Researcher B & Researcher C, from the Research Group X
