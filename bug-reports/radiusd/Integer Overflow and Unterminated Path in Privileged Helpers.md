## Email

Dear OpenBSD Security Team,

This email reports two privsep interface bugs in radiusd that allow
unprivileged `_radiusd` module processes to trigger memory-safety
violations in helpers running as root.

### Issue 1: BSDAUTH message-length integer overflow

The root BSDAUTH helper receives `IMSG_BSDAUTH_USERCHECK` messages
containing two sender-controlled `size_t` fields, `userlen` and
`passlen`. It first checks that the fixed argument header is present
(radiusd_bsdauth.c:138). It then adds the header size and both
untrusted lengths to check whether the strings fit in the received
payload (radiusd_bsdauth.c:145). If an attacker supplies large values
for `userlen` and `passlen`, this addition can overflow to a small
value and allow a short message to pass the check. The handler then
uses the original large lengths as offsets when writing NUL
terminators (radiusd_bsdauth.c:155 and radiusd_bsdauth.c:157), causing
out-of-bounds writes in the root helper.
`IMSG_BSDAUTH_GROUPCHECK` has the same issue.

Our fix checks each length individually before performing the existing
combined length check, preventing the addition from overflowing.

### Issue 2: unterminated path in IMSG_RADIUSD_FILE_PARAMS

The root FILE helper receives `IMSG_RADIUSD_FILE_PARAMS` from its
unprivileged `_radiusd` process. The message contains a fixed-size
`path` array (radiusd_file.c:47). The helper verifies that the
parameter structure is present (radiusd_file.c:141), but it does not
ensure that `path` is NUL-terminated before passing it to `strlcpy`
and `unveil` (radiusd_file.c:146 and radiusd_file.c:148).

Our fix writes a NUL terminator to the final byte of the path array
before any string operation.

[Proof-of-concept details removed.]

Happy to answer any questions!

Best regards,
Researcher A, Researcher B
Research Group X

```c
radiusd_bsdauth.c
138                 if (datalen < sizeof(
139                     struct auth_usercheck_args)) {
149                         syslog(LOG_ERR, "Short message");
141                         break;
142                 }
...
145                 if (datalen < sizeof(struct auth_usercheck_args)
146                     + args->userlen + args->passlen) {
147                         syslog(LOG_ERR, "Short message");
148                         break;
149                 }
...
154                 user = (char *)(args + 1);
155                 user[args->userlen - 1] = '\0';
156                 pass = user + args->userlen;
157                 pass[args->passlen - 1] = '\0';
...
185                 if (datalen <
186                     sizeof(struct auth_groupcheck_args) +
187                     args->userlen + args->grouplen) {
188                         syslog(LOG_ERR, "Short message");
189                         break;
190                 }
...
195                 user = (char *)(args + 1);
196                 user[args->userlen - 1] = '\0';
197                 group = user + args->userlen;
198                 group[args->grouplen - 1] = '\0';

radiusd_file.c
45 struct module_file_params {
46         int                      debug;
47         char                     path[PATH_MAX];
48 };
...
141         if (datalen < sizeof(params))
...
144         paramsp = imsg.data;
145         if (paramsp->path[0] != '\0') {
146                 strlcpy(pathdb, paramsp->path, sizeof(pathdb));
147                 strlcat(pathdb, ".db", sizeof(pathdb));
148                 if (unveil(paramsp->path, "r") == -1 ||
```
