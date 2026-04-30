## Email

Dear OpenBSD Security Team,

This email reports a privsep interface bug in httpd, allowing a less-
privileged logger compartment to trigger a heap over-read and a
subsequent leak of the over-read data inside the privileged
compartment.

The bug lies in the message handler for IMSG_LOG_OPEN (logger.c:158),
where it expects the message to be a uint32_t value followed by a
string. However, the only check is that the message length is at least
as long as uint32_t. If the message sent by the unprivileged
compartment either contains a non-NULL-terminated string or consists
only of a single uint32_t value, it will cause an over-read of the
pointer `p` when performing snprintf (logger.c:171). The over-read
content will first be stored in array `name` (logger.c:171), then
concatenated with another string (logger.c:180), and finally logged
(logger.c:186), leaking the over-read data.

[Proof-of-concept details removed.]

Thank you for your efforts in maintaining and securing OpenBSD!

Regards,
Researcher A, Researcher C, Researcher B
Research Group X

```c
logger.c
157 int
158 logger_open_priv(struct imsg *imsg)
159 {
160         char                     path[PATH_MAX];
161         char                     name[PATH_MAX], *p;
162         uint32_t                 id;
163         size_t                   len;
164         int                      fd;
165
166         /* called from the privileged process */
167         IMSG_SIZE_CHECK(imsg, &id);
168         memcpy(&id, imsg->data, sizeof(id));
169         p = (char *)imsg->data + sizeof(id);
170
171         if ((size_t)snprintf(name, sizeof(name), "/%s", p) >= sizeof(name))
172                 return (-1);
173         if ((len = strlcpy(path, httpd_env->sc_logdir, sizeof(path)))
174             >= sizeof(path))
175                 return (-1);
176
177         p = path + len;
178         len = sizeof(path) - len;
179
180         if (canonicalize_path(name, p, len) == NULL) {
181                 log_warnx("invalid log name");
182                 return (-1);
183         }
184
185         if ((fd = open(path, O_WRONLY|O_APPEND|O_CREAT, 0644)) == -1) {
186                 log_warn("failed to open %s", path);
187                 return (-1);
188         }
```
