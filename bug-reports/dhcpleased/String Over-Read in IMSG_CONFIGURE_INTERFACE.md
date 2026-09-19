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

Regards,
Shibo and Hugo,
UBC Systopia Lab
