## Email

Dear OpenBSD Security Team,

This report describes a privsep interface bug in `mountd` that allows
the unprivileged parent to export arbitrary files with arbitrary
permissions via NFS. This occurs because the privileged child performs
no verification on export requests sent by the parent, allowing the
parent to fully control what gets exported. Although the unprivileged
parent also runs as root, it is heavily constrained by pledge and
therefore cannot perform mount operations on its own. This bug allows
the unprivileged parent to partially break the sandbox, which does not
align with the least-privilege design principle.

The bug lies in the two privileged handlers for export-related
requests. `IMSG_EXPORT_REQ` is responsible for adding a path to the
kernel export list, and `IMSG_GETFH_REQ` is responsible for creating
the file handle for the incoming connection. However, neither message
verifies that the path it receives from the unprivileged parent is
specified in `/etc/exports` or that it matches the permissions
configured there. As a result, the unprivileged parent can craft
arbitrary parameters and gain control over what gets exported and with
what permissions.

[Proof-of-concept details removed.]
