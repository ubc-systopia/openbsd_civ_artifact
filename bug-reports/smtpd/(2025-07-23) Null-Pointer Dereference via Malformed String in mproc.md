## Email

Dear OpenBSD Security Team,

This email reports a privsep interface bug found by a fuzzer in
OpenSMTPd's `mproc` framework, where an unprivileged
compartment can trigger a NULL-pointer access in the privileged
compartment.

The bug lies in the `m_get_string` function in the `mproc` framework. In
the commit [0], error-handling logic was added such that if the first
character of the string passed from the unprivileged compartment is `\0`,
the returned pointer to the retrieved string is set to
NULL (line 529). However, callers of this function were not updated to
handle scenarios where the string pointer is NULL, causing NULL-pointer
dereferences when the error-handling branch is triggered.

This allows an unprivileged compartment to craft a message containing
strings that start with `\0`, triggering segmentation faults in the
privileged compartment. Fortunately, we did not find cases where this
erronous path could be triggered without compromising one
low-privileged compartment.

[Proof-of-concept details removed.]

Besides adding correct handling logic for receiving NULL pointers, we also
suggest modifying the `m_add_string` function such that if the variable
`v` is NULL (line 418), the function appends "s" before the string
terminator to maintain consistent string formatting.

Regards,
Researcher A, Researcher C, Researcher B
Research Group X

[0]
https://github.com/openbsd/src/commit/d6a7182727895a3a007b7c422af1128dd67d5849

```c
mproc.c
410 void
411 m_add_string(struct mproc *m, const char *v)
412 {
413         if (v) {
414                 m_add(m, "s", 1);
415                 m_add(m, v, strlen(v) + 1);
416         }
417         else
418                 m_add(m, "\0", 1);
419 };


mproc.c
518 void
519 m_get_string(struct msg *m, const char **s)
520 {
521         uint8_t *end;
522         char c;
523
524         if (m->pos >= m->end)
525                 m_error("msg too short");
526
527         c = *m->pos++;
528         if (c == '\0') {
529                 *s = NULL;
530                 return;
531         }
532
533         if (m->pos >= m->end)
534                 m_error("msg too short");
535         end = memchr(m->pos, 0, m->end - m->pos);
536         if (end == NULL)
537                 m_error("unterminated string");
538
539         *s = m->pos;
540         m->pos = end + 1;
541 }
```
