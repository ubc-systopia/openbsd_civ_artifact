## Email

Dear OpenBSD Security Team,

This email reports three privsep interface bugs in ospfd where the privileged
parent, running as root, does not validate the payload lengths of messages
received from unprivileged compartments.

### Issue 1: IMSG_CTL_LOG_VERBOSE

The privileged compartment copies a four-byte verbosity value from imsg.data
without first confirming that the payload is exactly four bytes (ospfd.c:422):

```c
ospfd.c
420         case IMSG_CTL_LOG_VERBOSE:
421                 /* already checked by ospfe */
422                 memcpy(&verbose, imsg.data, sizeof(verbose));
423                 log_setverbose(verbose);
424                 break;
```

This is analogous to the IMSG_CTL_LOG_VERBOSE issue we previously reported in
eigrpd. Our patch fixes this by requiring the payload length to equal
sizeof(verbose) before copying the value.

### Issue 2: IMSG_KROUTE_CHANGE

The privileged handler does not check that the payload contains at least one
complete struct kroute before calling kr_change() (ospfd.c:476), causing
potential NULL ptr access (if the message has no data) or over-read (if the
message is shorter than sizeof(struct kroute)) (kroute.c:295).

```c
ospfd.c
473         case IMSG_KROUTE_CHANGE:
474                 count = (imsg.hdr.len - IMSG_HEADER_SIZE) /
475                     sizeof(struct kroute);
476                 if (kr_change(imsg.data, count))
477                         log_warn("main_dispatch_rde: error changing "
478                             "route");
479                 break;

kroute.c
289 int
290 kr_change(struct kroute *kroute, int krcount)
291 {
292         struct kroute_node      *kr;
293         int                      action = RTM_ADD;
294
295         kroute->rtlabel = rtlabel_tag2id(kroute->ext_tag);
```

Our patch fixes this by requiring a nonempty payload whose length is an exact
multiple of sizeof(struct kroute) before calling kr_change().

### Issue 3: IMSG_KROUTE_DELETE

The privileged handler for IMSG_KROUTE_DELETE does not validate that the
payload contains a complete struct kroute before passing it to kr_delete()
(ospfd.c:481), causing a potential NULL pointer access (if the message has no
data) or over-read (if the data is shorter than sizeof(struct kroute))
(kroute.c:327).

```c
ospfd.c
480         case IMSG_KROUTE_DELETE:
481                 if (kr_delete(imsg.data))
482                         log_warn("main_dispatch_rde: error deleting "
483                             "route");
484                 break;

kroute.c
322 int
323 kr_delete(struct kroute *kroute)
324 {
325         struct kroute_node      *kr, *nkr;
326
327         if ((kr = kroute_find(kroute->prefix.s_addr, kroute->prefixlen,
328             kr_state.fib_prio)) == NULL)
329                 return (0);
```

Our patch fixes this by requiring the payload length to equal sizeof(struct
kroute) before calling kr_delete().

[Proof-of-concept details removed.]

Happy to answer any questions!

Best regards,
Shibo, Hugo
Systopia Team
