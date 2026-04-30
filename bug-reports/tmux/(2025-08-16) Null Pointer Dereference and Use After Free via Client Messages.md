## Email

Dear OpenBSD Security Team,

This email reports two privsep interface bugs in `tmux` which allow a
client to trigger bugs in the server: the first allows the client
compartment to trigger a NULL pointer dereference, and the second a
use after free. We know that client and server both run under the same
user (albeit with different pledge profiles), and so suspect that
these bugs may have no security impact. However, we went for
security@ as we may not fully understand the impact.

The first privsep interface bug lies in the definition and use of
`struct client` (tmux.h:1874). Many pointers in `struct client` are
NULL at the very beginning, and the more-privileged server
compartment gradually assigns values to those pointers as it
interacts with the client. However, this design relies on the client
sending messages in the correct order. We found in multiple places
where the client can trigger a NULL pointer dereference in the server
compartment by sending a message that tricks the server into
accessing a pointer before it becomes valid.

[Proof-of-concept details removed.]

Regards,
Researcher A, Researcher C, Researcher B
Research Group X

```c
tmux.h
1874 struct client {
1875         const char              *name;
1876         struct tmuxpeer         *peer;
1877         struct cmdq_list        *queue;
...
1907         char                    *ttyname;

server-client.c
3443 static void
3444 server_client_dispatch_command(struct client *c, struct imsg *imsg)
3445 {
3446         struct msg_command        data;
3447         char                     *buf;
3448         size_t                    len;
3449         int                       argc;
3450         char                    **argv, *cause;
3451         struct cmd_parse_result  *pr;
3452         struct args_value        *values;
3453         struct cmdq_item         *new_item;
3454         struct cmd_list          *cmdlist;
3455
3456         if (c->flags & CLIENT_EXIT)
3457                 return;
3458
3459         if (imsg->hdr.len - IMSG_HEADER_SIZE < sizeof data)
3460                 fatalx("bad MSG_COMMAND size");
3461         memcpy(&data, imsg->data, sizeof data);
3462
3463         buf = (char *)imsg->data + sizeof data;
3464         len = imsg->hdr.len  - IMSG_HEADER_SIZE - sizeof data;
3465         if (len > 0 && buf[len - 1] != '\0')
3466                 fatalx("bad MSG_COMMAND string");
3467
3468         argc = data.argc;
3469         if (cmd_unpack_argv(buf, len, argc, &argv) != 0) {
3470                 cause = xstrdup("command too long");
3471                 goto error;
3472         }
3473
...
3504 error:
3505         cmd_free_argv(argc, argv);
3506
3507         cmdq_append(c, cmdq_get_error(cause));
3508         free(cause);
3509
3510         c->flags |= CLIENT_EXIT;
3511 }
...
3514 static void
3515 server_client_dispatch_identify(struct client *c, struct imsg *imsg)
3516 {
...
3529         switch (imsg->hdr.type) {
...
3570         case MSG_IDENTIFY_TTYNAME:
3571                 if (datalen == 0 || data[datalen - 1] != '\0')
3572                         fatalx("bad MSG_IDENTIFY_TTYNAME string");
3573                 c->ttyname = xstrdup(data);
3574                 log_debug("client %p IDENTIFY_TTYNAME %s", c, data);
3575                 break;
...
3616         if (imsg->hdr.type != MSG_IDENTIFY_DONE)
3617                 return;
3618         c->flags |= CLIENT_IDENTIFIED;
3619
3620         if (*c->ttyname != '\0')
3621                 name = xstrdup(c->ttyname);

cmd.c
298 int
299 cmd_unpack_argv(char *buf, size_t len, int argc, char ***argv)
300 {
301         int     i;
302         size_t  arglen;
303
304         if (argc == 0)
305                 return (0);
306         *argv = xcalloc(argc, sizeof **argv);
307
308         buf[len - 1] = '\0';
309         for (i = 0; i < argc; i++) {
310                 if (len == 0) {
311                         cmd_free_argv(argc, *argv);
312                         return (-1);
313                 }
314
315                 arglen = strlen(buf) + 1;
316                 (*argv)[i] = xstrdup(buf);
317
318                 buf += arglen;
319                 len -= arglen;
320         }
321         cmd_log_argv(argc, *argv, "%s", __func__);
322
323         return (0);
324 }
…
344 void
345 cmd_free_argv(int argc, char **argv)
346 {
347         int     i;
348
349         if (argc == 0)
350                 return;
351         for (i = 0; i < argc; i++)
352                 free(argv[i]);
353         free(argv);
354 }
```
