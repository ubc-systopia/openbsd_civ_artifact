## Email

Dear OpenBSD Security Team,

This email reports a mid-step vector found while fuzzing bgpd.

A stack *over-read* can be triggered in the privileged bgpd compartment
when an `IMSG_PFTABLE_ADD` or `IMSG_PFTABLE_REMOVE` message is sent from
the unprivileged rde process.  This occurs when `pftable_msg.pftable` is
missing a null terminator.

```c
bgpd.h
 872 struct pftable_msg {
 873         struct bgpd_addr        addr;
 874         char                    pftable[PFTABLE_LEN];
 875         uint8_t                 len;
 876 };
```

Given that `pftable` is never checked for a null terminator, a
compromised unprivileged rde process would be able to trigger the stack
*over-read* in the privileged compartment.

```c
bgpd.c
 921         case IMSG_PFTABLE_ADD:
 922             if (idx != PFD_PIPE_RDE)
 923                 log_warnx("pftable request not from RDE");
 924             else if (imsg_get_data(&imsg, &pfmsg, sizeof(pfmsg)) ==
 925                 -1)
 926                 log_warn("wrong imsg len");
 927             else if (pftable_addr_add(&pfmsg) != 0)
 928                 rv = -1;
 929             break;
 930         case IMSG_PFTABLE_REMOVE:
 931             if (idx != PFD_PIPE_RDE)
 932                 log_warnx("pftable request not from RDE");
 933             else if (imsg_get_data(&imsg, &pfmsg, sizeof(pfmsg)) ==
 934                 -1)
 935                 log_warn("wrong imsg len");
 936             else if (pftable_addr_remove(&pfmsg) != 0)
 937                 rv = -1;
 938             break;
 ```

Furthermore, since both the `pftable_addr_add` and `pftable_addr_remove`
(lines 927, 936) eventually call `log_warn`, the over-read data past
`struct pftable_msg pfmsg` would be exposed by those logs.

[Proof-of-concept details removed.]

Regards,
Researcher C, Researcher A, Researcher B
Research Group X
