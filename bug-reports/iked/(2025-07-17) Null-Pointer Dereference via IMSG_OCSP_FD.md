## Email

Dear OpenBSD Security Team,

This email reports a mid-step vector found in `iked`.  We suspect that
this bug could also be triggered from the network.

A null-pointer dereference can be triggered in the privileged parent
process when an `IMSG_OCSP_FD` message containing an invalid URL is sent
from the CA process.  Upon receiving the `IMSG_OCSP_FD` message, the
`ocsp_connect` function is called (shown below).  The `url` variable is
set by the message data sent by CA (line 101). However, if the URL is
invalid, the `OCSP_parse_url` function (line 108) would fail and cause a
jump to the `done` label (line 111).  This jump would skip the `calloc`
(line 123), leaving the `oc` variable as `NULL`.  Notably, subsequent
`if` statements can also trigger this in a similar manner (lines 113-
117).  For example, even with a valid URL, this bug can still be
triggered if OCSP over SSL is used (line 113).

The `ocsp_connect_finish` function is subsequently called with the `oc`
argument (line 183).  In the `ocsp_connect_finish` function (shown
below), `iov[0].iov_base` is then also set to `NULL` based on the value
of the `oc` argument (line 229).    The `iovcnt` variable is then
incremented to 1 (line 231).  Finally, a call is made to
`proc_composev_imsg` with `iov` and `iovcnt` as its arguments. Since
`iovcnt` is set to 1 and `iov[0].iov_len` is greater than 0,
`iov[0].iov_base` will eventually be accessed during later calls to the
`imsg` API, thus causing a null-pointer dereference.

[Proof-of-concept details removed.]

This bug was found while fuzzing the IPC interface of the parent process
using AFL++.  We specifically targeted the parent process given its
higher privilege, but this approach could also be used to fuzz the CA
and IKEv2 processes with minimal effort.

We noticed that OpenIKED-Portable already incorporates fuzzing into its
continuous integration workflow. We are wondering if this could be
extended to include our privsep interface fuzzing. If you are
interested, we would be happy to help on improving the existing fuzzer
with our approach.

[Proof-of-concept details removed.]

Regards,

Researcher C, Researcher A, Researcher B
Research Group X

```c
ocsp.c
  78 ocsp_connect(struct iked *env, struct imsg *imsg)
  79 {
  80     struct ocsp_connect *oc = NULL;
  81     struct iked_sahdr    sh;
  ...
  84     uint8_t         *ptr;
  85     size_t           len;
  ...
  88     int         use_ssl, fd = -1, ret = -1, error;
  89
  90     IMSG_SIZE_CHECK(imsg, &sh);
  91
  92     ptr = (uint8_t *)imsg->data;
  93     len = IMSG_DATA_SIZE(imsg);
  94
  95     memcpy(&sh, ptr, sizeof(sh));
  96
  97     ptr += sizeof(sh);
  98     len -= sizeof(sh);
  99
100     if (len > 0)
101         url = freeme = get_string(ptr, len);
102     else if (env->sc_ocsp_url)
103         url = env->sc_ocsp_url;
104     else {
105         log_warnx("%s: no ocsp url", SPI_SH(&sh, __func__));
106         goto done;
107     }
108     if (!OCSP_parse_url(url, &host, &port, &path, &use_ssl)) {
109         log_warnx("%s: error parsing OCSP-request-URL: %s",
110             SPI_SH(&sh, __func__), url);
111         goto done;
112     }
113     if (use_ssl) {
114         log_warnx("%s: OCSP over SSL not supported: %s",
115             SPI_SH(&sh, __func__), url);
116         goto done;
117     }
...
123         if ((oc = calloc(1, sizeof(*oc))) == NULL) {
124                 log_debug("%s: calloc failed", __func__);
125                 goto done;
126         }
...
175  done:
...
182     if (ret == -1) {
183         ocsp_connect_finish(env, -1, oc);

ocsp.c
222 /* send FD+path or error back to CA process */
223 int
224 ocsp_connect_finish(struct iked *env, int fd, struct ocsp_connect *oc)
225 {
226     struct iovec         iov[2];
227     int          iovcnt = 0, ret;
228
229     iov[iovcnt].iov_base = &oc->oc_sh;
230     iov[iovcnt].iov_len = sizeof(oc->oc_sh);
231     iovcnt++;
232
...
247         ret = proc_composev_imsg(&env->sc_ps, PROC_CERT, -1,
248             IMSG_OCSP_FD, -1, -1, iov, iovcnt);
```
