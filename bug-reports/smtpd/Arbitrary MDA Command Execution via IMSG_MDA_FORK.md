## Email

Dear OpenSMTPD Security Team,

This email reports a privilege separation interface bug in
OpenSMTPD that allows an unprivileged privsep process to execute
arbitrary shell commands under any arbitrary non-root user. This
affects OpenSMTPD when configured with at least one local
dispatcher for all users (for clarity: this is configured with
the `action` keyword in smtpd.conf) which we believe is a common
deployment scenario.

We assume that one of OpenSMTPD's unprivileged processes has been
compromised (e.g., lka). This issue allows the compromised
process, running under the de-privileged `_smtpd` user, to
escape the pledge sandbox, escape (filesystem) access control,
and other protection mechanisms.

The issue lies in the privileged handler for the
`IMSG_MDA_FORK` message. The handler receives a `struct deliver`
containing a string called `mda_exec` from unprivileged privsep
processes. It first checks whether the dispatcher name in the
`deliver` struct matches a known dispatcher (`smtpd.c:1419`).
Then, if the dispatcher is configured for all users, it uses the
user identities from the attacker-controlled `deliver` to
execute `mda_exec` as a shell command (reading attacker-
controlled credentials at `smtpd.c:1446--1449`, switching to
that user at `smtpd.c:1528--1530`, and executing the shell
command at `mda_unpriv.c:91`). This should be straightforward to
do for a compromised privsep process like lka, because
(1) dispatcher information is shared among all privsep
processes, so the malicious privsep process does not need to
guess the correct dispatcher, and (2) a multi-user local
dispatcher configuration is common.

This API seems inherently unsafe: the ability to execute
arbitrary commands as a system user should probably not be
exposed to unprivileged privsep processes at all. The absence of
a check to verify which privsep process called this endpoint
amplifies the issue, as any privsep process capable of sending
messages to the privileged parent can invoke this handler, even
if it is not designed to send this message (in practice we
believe that only dispatchers should be able to call this
endpoint).

[Proof-of-concept details removed.]

Happy to answer any questions.

Best regards,
Researcher A, Researcher B
Research Group X

```c
smtpd.c
1405 static void
1406 forkmda(struct mproc *p, uint64_t id, struct deliver *deliver)
1407 {
1408         char             ebuf[128], sfn[32];
1409         struct dispatcher       *dsp;
1410         struct child    *child;
1411         pid_t            pid;
1412         int              allout, pipefd[2];
1413         struct passwd   *pw;
1414         const char      *pw_name;
1415         uid_t   pw_uid;
1416         gid_t   pw_gid;
1417         const char      *pw_dir;
1418
1419         dsp = dict_xget(env->sc_dispatchers, deliver->dispatcher);
1420         if (dsp->type != DISPATCHER_LOCAL)
1421                 fatalx("non-local dispatcher called from forkmda()");
1422
...
1446                 pw_name = deliver->userinfo.username;
1447                 pw_uid = deliver->userinfo.uid;
1448                 pw_gid = deliver->userinfo.gid;
1449                 pw_dir = deliver->userinfo.directory;
...
1550         if (dsp->u.local.is_mbox &&
1551             dsp->u.local.mda_wrapper == NULL &&
1552             deliver->mda_exec[0] == '\0')
1553                 mda_mbox(deliver);
1554         else
1555                 mda_unpriv(dsp, deliver, pw_name, pw_dir);
1556 }
```

```c
mda_unpriv.c
 26 void
 27 mda_unpriv(struct dispatcher *dsp, struct deliver *deliver,
 28     const char *pw_name, const char *pw_dir)
 29 {
...
 37         if (deliver->mda_exec[0])
 38                 mda_command = deliver->mda_exec;
 39         else
 40                 mda_command = dsp->u.local.command;
...
 91         execle("/bin/sh", "/bin/sh", "-c", mda_command, (char *)NULL,
 92             mda_environ);
 93
 94         perror("execle");
 95         _exit(1);
 96 }
```
