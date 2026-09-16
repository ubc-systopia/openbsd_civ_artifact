## Email

Dear OpenBSD Security Team,

This email reports a privsep interface bug in isakmpd where the privileged
monitor (running as root) does not validate the address length of a
MONITOR_BIND message from the unprivileged child, leading to a buffer
over-read.

The privileged MONITOR_BIND handler first receives a socket descriptor
(monitor.c:599). It then reads a four-byte payload length into `namelen`
(monitor.c:605), reads that many bytes from the monitor socket (monitor.c:612),
and treats the resulting buffer as a `struct sockaddr` (monitor.c:614).

However, `m_priv_check_bind` does not first check the size of the payload
before accessing it as a `struct sockaddr` (it applies `SA_LEN` to the buffer
(monitor.c:788) and later accesses `sa_family` (monitor.c:793)). This can cause
a buffer over-read when `namelen` and the payload is very small.

[Proof-of-concept details removed.]

Our fix rejects addresses shorter than `struct sockaddr`.

Happy to answer any questions!

Best regards,
Researcher A, Researcher B
Research Group X

```c
monitor.c
591 /* Privileged: called by monitor_loop.  */
592 static void
593 m_priv_bind(void)
594 {
595         int              sock, v, err = 0;
596         struct sockaddr *name = 0;
597         socklen_t        namelen;
598
599         sock = mm_receive_fd(m_state.s);
...
604
605         must_read(&namelen, sizeof namelen);
606         name = malloc(namelen);
...
612         must_read((char *)name, namelen);
613
614         if (m_priv_check_bind(name, namelen) != 0) {

monitor.c
778 /* Check bind */
779 static int
780 m_priv_check_bind(const struct sockaddr *sa, socklen_t salen)
781 {
782         in_port_t       port;
783
784         if (sa == NULL) {
785                 log_print("NULL address");
786                 return 1;
787         }
788         if (SA_LEN(sa) != salen) {
789                 log_print("Length mismatch: %lu %lu",
790                     (unsigned long)sa->sa_len, (unsigned long)salen);
791                 return 1;
792         }
793         switch (sa->sa_family) {
```
