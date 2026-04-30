## Email

Dear OpenBSD Security Team,

This email reports four privsep interface bugs in httpd, involving
two categories:

1) The privileged compartment sends pointers to unprivileged
compartments, breaking the ASLR protection between compartment
processes set up by the fork+exec technique.

2) The privileged compartment sends uninitialized stack data to
unprivileged compartments, possibly leaking sensitive data.

We provide all four proofs of concept in a single patch.
Each can be reproduced by defining its corresponding macro,
which are listed below in the same order the bugs are reported:

1) PTR_CFG_SERVER
2) PTR_CFG_MEDIA
3) LEAK_CFG_MEDIA
4) LEAK_CFG_DONE

1. Two pointer-leaking bugs

Both pointer-leaking bugs share a root cause-the privileged
compartment sends pointer-containing C structures to unprivileged
compartments.

The first bug is in function `config_setserver`, which places a
pointer-containing struct server_config into an iovec (config.c:191)
and then sends that iovec to unprivileged compartments
(config.c:228).

The second bug is in function `config_setmedia`, which sends a
pointer-containing `struct media_type` to the unprivileged
compartment (config.c:834).

[Proof-of-concept details removed.]

Function `config_setmedia` also leaks uninitialized stack data in
addition to pointers because the message it sends-`struct
media_type`-stores strings in fixed-size arrays (httpd.h:441),
leaving any unused bytes uninitialized. In `config_setmedia`, the
`media_type` instance is being passed in as a function parameter,
and it contains these uninitialized stack bytes. It is initially
allocated as a stack variable in parse.y:1896.

In parent_configure, the privileged compartment sends a
stack-allocated struct ctl_flags to the unprivileged compartment.
Alignment padding between entries cf_opts (httpd.h:121) and cf_flags
(httpd.h:122) contains uninitialized stack data and is included
in the message.

[Proof-of-concept details removed.]

Best regards,
Researcher C, Researcher A, Researcher B,
Research Group X

```c
config.c
161 int
162 config_setserver(struct httpd *env, struct server *srv)
163 {
164         struct privsep          *ps = env->sc_ps;
165         struct server_config     s;
166         int                      id;
167         int                      fd, n, m;
168         struct iovec             iov[6];
169         size_t                   c;
170         unsigned int             what;
171
...
190             c = 0;
191             iov[c].iov_base = &s;
192             iov[c++].iov_len = sizeof(s);
...
227             } else {
228                 if (proc_composev(ps, id, IMSG_CFG_SERVER,
229                     iov, c) != 0) {
230                     log_warn("%s: failed to compose "
231                         "IMSG_CFG_SERVER imsg for `%s'",
232                         __func__, srv->srv_conf.name);
233                     return (-1);
234                 }
235
236                 /* Configure FCGI parameters if necessary. */
237                 config_setserver_fcgiparams(env, srv);
238             }
...
818 int
819 config_setmedia(struct httpd *env, struct media_type *media)
820 {
821     struct privsep          *ps = env->sc_ps;
822     int                      id;
823     unsigned int             what;
824
825     for (id = 0; id < PROC_MAX; id++) {
826         what = ps->ps_what[id];
827
828         if ((what & CONFIG_MEDIA) == 0 || id == privsep_process)
829             continue;
830
831         DPRINTF("%s: sending media \"%s\" to %s", __func__,
832             media->media_name, ps->ps_title[id]);
833
834         proc_compose(ps, id, IMSG_CFG_MEDIA, media, sizeof(*media));
835     }
836
837     return (0);
838 }
```

```c
httpd.c
271 int
272 parent_configure(struct httpd *env)
273 {
274     int                      id;
275     struct ctl_flags         cf;
276     int                      ret = -1;
277     struct server           *srv;
278     struct media_type       *media;
279     struct auth             *auth;
...
313     for (id = 0; id < PROC_MAX; id++) {
314         if (id == privsep_process)
315             continue;
316         cf.cf_opts = env->sc_opts;
317         cf.cf_flags = env->sc_flags;
318         memcpy(cf.cf_tls_sid, env->sc_tls_sid, sizeof(cf.cf_tls_sid));
319
320         proc_compose(env->sc_ps, id, IMSG_CFG_DONE, &cf, sizeof(cf));
321     }
```

```c
httpd.h
120 struct ctl_flags {
121     uint8_t          cf_opts;
122     uint32_t         cf_flags;
123     uint8_t          cf_tls_sid[TLS_MAX_SESSION_ID_LENGTH];
124 };
...
441 struct media_type {
442     char                     media_name[MEDIATYPE_NAMEMAX];
443     char                     media_type[MEDIATYPE_TYPEMAX];
444     char                     media_subtype[MEDIATYPE_TYPEMAX];
445     char                    *media_encoding;
446     RB_ENTRY(media_type)     media_entry;
447 };
```

```c
parse.y
1891 int
1892 load_config(const char *filename, struct httpd *x_conf)
1893 {
1894     struct sym              *sym, *next;
1895     struct http_mediatype    mediatypes[] = MEDIA_TYPES;
1896     struct media_type        m;
1897     int                      i;
1898
```

```c
httpd.c
114 int
115 main(int argc, char *argv[])
116 {
117     int                      c;
118     unsigned int             proc;
119     int                      debug = 0, verbose = 0;
120     uint32_t                 opts = 0;
121     struct httpd            *env;
...
257     /* initialize the TLS session id to a random key for all procs */
258     arc4random_buf(env->sc_tls_sid, sizeof(env->sc_tls_sid));
259
260     if (parent_configure(env) == -1)
261         fatalx("configuration failed");
```
