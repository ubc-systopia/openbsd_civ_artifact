## Email

Dear OpenBSD security team,

This email reports a common pattern we found in OpenBSD
privilege-separated programs where the privileged
compartment leaks pointers and uninitialized memory to
unprivileged compartments.  This occurs when unsanitized
structures (i.e., do not explicitly remove sensitive data
such as pointers and uninitialized memory) are sent
directly through IPC, leaking internal pointers and
uninitialized memory in both unpopulated fields and
compiler padding.  These leaks could be used by an
attacker who already controls the unprivileged
compartment to gain insight into the privileged
compartment's secrets, such as the memory layout hidden
by `fork+exec` as well as potentially sensitive data
contained in uninitialized memory.  We have previously
sent individual bug reports regarding `httpd` and `bgpd`,
but we are now finding that this pattern appears to be
prevalent across programs, and we think it might be
helpful to send a general report discussing this.

We found this is mostly seen in messages that require
the privileged compartment to send complex data to
unprivileged compartments.  One class of examples is
configuration passing : instead of creating a pointer-free
payload, the privileged compartment directly sends its
internal config structs, which contain pointers, to the
unprivileged compartments.  We have seen an `XXX` marking
in OpenSSH acknowledging this when the privileged monitor
sends ServerOptions to less-privileged compartments
(monitor.c:782), and most of our findings fall into this
class.  We also found non-configuration messages with
the same issue.  For example, tcpdump's privileged
compartment proxies localtime(3) and gmtime(3) calls on
behalf of the unprivileged compartment via PRIV_LOCALTIME,
and sends back the raw `struct tm`, which contains a
pointer (tm_zone).

In total, we have observed the following places where the
privileged compartment leaks sensitive information.
Considering its prevalent nature, we suspect there might
be more such places.

- sshd leaking pointer-containing `struct ServerOptions`
  (monitor.c:783)
- tcpdump leaking pointer-containing `struct tm`
  (privsep.c:601)
- bgpd leaking pointer-containing `struct filter_rule`
  (bgpd.c:781)
- iked leaking pointer-containing `struct iked_policy`
  (config.c:812)
- eigrpd leaking pointer-containing `struct eigrp_iface`
  (eigrpd.c:601)
- dvmrpd leaking uninitialized memory when sending
  stack-allocated `struct route_report` and `struct mfc`
  due to unpopulated fields and compiler padding
  (kroute.c:55 and kroute.c:121).
- dhcpleased leaking pointer-containing `struct iface_conf`
  (dhcpleased.c:770)

We found that the leaked pointers are
heap-allocated, either explicitly allocated by malloc or
by BSD queue macros (e.g., TAILQ_ENTRY).  Fortunately,
these pointers cannot be used to infer the address of
code segments.  For many structures that leak pointers,
despite having compiler padding and being only partially
filled, many of them are explicitly zeroed during
initialization and therefore do not leak uninitialized
memory.

Happy to answer any questions!

Best regards,
Researcher A, Researcher B
Research Group X

```c
monitor.c (ssh)
776 void
777 mm_encode_server_options(struct sshbuf *m)
778 {
779         int r;
780         u_int i;
781
782         /* XXX this leaks raw pointers to the unpriv child processes */
783         if ((r = sshbuf_put_string(m, &options, sizeof(options))) != 0)
784                 fatal_fr(r, "assemble options");
...
801 }
```

```c
bgpd.c (bgpd)
590 int
591 send_config(struct bgpd_config *conf)
592 {
...
777         while ((r = TAILQ_FIRST(conf->filters)) != NULL) {
778                 TAILQ_REMOVE(conf->filters, r, entry);
779                 if (imsg_send_filterset(ibuf_rde, &r->set) == -1)
780                         return (-1);
781                 if (imsg_compose(ibuf_rde, IMSG_RECONF_FILTER, 0, 0, -1,
782                     r, sizeof(struct filter_rule)) == -1)
783                         return (-1);
784                 filterset_free(&r->set);
785                 free(r);
786         }
...
588 }
```

```c
config.c (iked)
773 int
774 config_setpolicy(struct iked *env, struct iked_policy *pol,
775     enum privsep_procid id)
776 {
...
792         iov[c].iov_base = pol;
793         iov[c++].iov_len = sizeof(*pol);
...
812         if (proc_composev(&env->sc_ps, id, IMSG_CFG_POLICY, iov,
813             iovcnt) == -1) {
814                 log_debug("%s: proc_composev failed", __func__);
815                 return (-1);
...
818         return (0);
819 }
```

```c
eigrpd.c (eigrpd)
582 static int
583 main_imsg_send_config(struct eigrpd_conf *xconf)
584 {
585         struct eigrp            *eigrp;
586         struct eigrp_iface      *ei;
587
588         if (eigrp_sendboth(IMSG_RECONF_CONF, xconf, sizeof(*xconf)) == -1)
589                 return (-1);
590
591         TAILQ_FOREACH(eigrp, &xconf->instances, entry) {
592                 if (eigrp_sendboth(IMSG_RECONF_INSTANCE, eigrp,
593                     sizeof(*eigrp)) == -1)
594                         return (-1);
595
596                 TAILQ_FOREACH(ei, &eigrp->ei_list, e_entry) {
597                         if (eigrp_sendboth(IMSG_RECONF_IFACE, ei->iface,
598                             sizeof(struct iface)) == -1)
599                                 return (-1);
600
601                         if (eigrp_sendboth(IMSG_RECONF_EIGRP_IFACE, ei,
602                             sizeof(*ei)) == -1)
603                                 return (-1);
604                 }
605         }
606
607         if (eigrp_sendboth(IMSG_RECONF_END, NULL, 0) == -1)
608                 return (-1);
609
610         return (0);
611 }
```

```c
privsep.c (tcpdump)
583 static void
584 impl_localtime(int fd)
585 {
586         struct tm *lt, *gt;
587         time_t t;
588
589         logmsg(LOG_DEBUG, "[priv]: msg PRIV_LOCALTIME received");
590
591         must_read(fd, &t, sizeof(time_t));
592
593         /* this must be done separately, since they apparently use the
594          * same local buffer */
595         if ((lt = localtime(&t)) == NULL)
596                 errx(1, "localtime()");
597         must_write(fd, lt, sizeof(*lt));
598
599         if ((gt = gmtime(&t)) == NULL)
600                 errx(1, "gmtime()");
601         must_write(fd, gt, sizeof(*gt));
602
603         if (lt->tm_zone == NULL)
604                 write_zero(fd);
605         else
606                 write_string(fd, lt->tm_zone);
607 }
```

```c
dhcpleased.c (dhcpleased)
760 int
761 main_imsg_send_config(struct dhcpleased_conf *xconf)
762 {
763         struct iface_conf       *iface_conf;
764
765         main_imsg_compose_frontend(IMSG_RECONF_CONF, -1, NULL, 0);
766         main_imsg_compose_engine(IMSG_RECONF_CONF, -1, NULL, 0);
767
768         /* Send the interface list to the frontend & engine. */
769         SIMPLEQ_FOREACH(iface_conf, &xconf->iface_list, entry) {
770                 main_imsg_compose_frontend(IMSG_RECONF_IFACE, -1, iface_conf,
771                     sizeof(*iface_conf));
772                 main_imsg_compose_engine(IMSG_RECONF_IFACE, -1, iface_conf,
773                     sizeof(*iface_conf));
...
789         }
790
791         /* Config is now complete. */
792         main_imsg_compose_frontend(IMSG_RECONF_END, -1, NULL, 0);
793         main_imsg_compose_engine(IMSG_RECONF_END, -1, NULL, 0);
794
795         return (0);
796 }
```

```c
kroute.c (dvmrpd)
 41 int
 42 kmr_init(int fd)
 43 {
 44         struct iface            *iface;
 45         struct route_report      rr;
 46
 47         LIST_FOREACH(iface, &conf->iface_list, entry) {
 48                 log_debug("kmr_init: interface %s", iface->name);
 49
 50                 rr.net.s_addr = iface->addr.s_addr & iface->mask.s_addr;
 51                 rr.mask = iface->mask;
 52                 rr.nexthop.s_addr = 0;
 53                 rr.metric = iface->metric;
 54                 rr.ifindex = iface->ifindex;
 55                 main_imsg_compose_rde(IMSG_ROUTE_REPORT, -1, &rr, sizeof(rr));
 56
 57                 mrt_add_vif(conf->mroute_socket, iface);
 58         }
 59
 60         if ((mroute_ptr = calloc(1, IBUF_READ_SIZE)) == NULL)
 61                 fatal("kmr_init");
 62
 63         return (0);
 64 }
...
 83 void
 84 kmr_recv_msg(int fd, short event, void *bula)
 85 {
...
110         case IGMPMSG_NOCACHE:
...
117                 /* send MFC entry to RDE */
118                 mfc.origin = kernel_msg.im_src;
119                 mfc.group = kernel_msg.im_dst;
120                 mfc.ifindex = kernel_msg.im_vif;
121                 main_imsg_compose_rde(IMSG_MFC_ADD, 0, &mfc, sizeof(mfc));
122                 break;
...
130 }
```
