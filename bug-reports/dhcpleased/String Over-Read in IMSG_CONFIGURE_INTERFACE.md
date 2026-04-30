## Email

Dear OpenBSD Security Team,

This email reports a mid-step vector in *dhcpleased*.

In *dhcpleased*, multiple handlers in the `main_dispatch_engine` method
of the privileged privsep process receive a `struct
imsg_configure_interface`
from the unprivileged privsep process. However, none of these handlers
validate the terminators in the struct's strings, including those that
later read the string fields.

[Proof-of-concept details removed.]

     -------------<sender>-------------
     > As sender, my pid is 32770, uid is 77

     -------------<receiver>-------------
     > As receiver, my pid is 72741, my uid is 0
     > Maximum strlen for hostname is 4097
     > strlen for hostname is 4098

The results demonstrate that the received string length exceeds the
maximum allowed length, indicating a buffer over-read. Note that the
received `strlen` is only one character longer because the first
over-read byte is zero, which acts as a string terminator.

[Proof-of-concept details removed.]

Regards,
Researcher A and Researcher B,
Research Group X
