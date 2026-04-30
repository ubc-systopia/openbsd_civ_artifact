## Email

Dear OpenBSD Security Team,

This email reports a privsep interface bug in `rpki-client` discovered
by a fuzzer, which allows a less-privileged parser compartment to
trigger a NULL pointer access in the more privileged main compartment.

The bug lies in the parent compartment's message handler for the parser
compartment. At the beginning of the handler, it reads an ID
(main.c:587) and a string (main.c:589) from the IPC message controlled
by the less-privileged parser compartment. The ID is then used to look
up the `struct repo` (main.c:601). Next, both the string pointer and the
repo pointer are passed to the function `repostats_new_files_inc`
(main.c:603). However, the less-privileged compartment can craft a
malicious message to make either the `rp` pointer or `file` pointer
passed to the `repostats_new_files_inc` function NULL, triggering a NULL
pointer access in the more privileged compartment.

In the function `io_read_str`, if it reads a string length of zero, it
sets pointer parameter `res` to NULL (io.c:140). Therefore, if the
less-privileged compartment sends a `file` string with a length of zero,
the `file` pointer after the `io_read_str` call (main.c:589) will be
NULL. Similarly, in the function `repo_byid`, if no repo is found for
the given ID, it returns a NULL pointer (repo.c:1276). Thus,
the less-privileged compartment can make the `rp` pointer NULL by
sending an invalid ID. Despite these possibilities, there is no check
either before or inside the function call `repostats_new_files_inc`
(main.c:603) to validate these pointers, allowing a potential NULL
pointer access induced by the less-privileged compartment.

[Proof-of-concept details removed.]

We acknowledge that both the main and parser compartments run as the
user `_rpki-client`. However, we consider the main compartment to be
more privileged than the parser because it has access to more system
calls after `pledge`.

Regards,
Researcher A, Researcher C, Researcher B
Research Group X

```c
main.c
564 entity_process(struct ibuf *b, struct validation_data *vd, struct
stats *st)
565 {
566         enum rtype       type;
567         struct tal      *tal;
568         struct cert     *cert;
569         struct mft      *mft;
570         struct roa      *roa;
571         struct aspa     *aspa;
572         struct spl      *spl;
573         struct repo     *rp;
574         char            *file;
575         time_t           mtime;
576         unsigned int     id;
577         int              talid;
578         int              ok = 1;
...
586         io_read_buf(b, &type, sizeof(type));
587         io_read_buf(b, &id, sizeof(id));
588         io_read_buf(b, &talid, sizeof(talid));
589         io_read_str(b, &file);
590         io_read_buf(b, &mtime, sizeof(mtime));
591
592         /* in filemode messages can be ignored, only the accounting
matters */
593         if (filemode)
594                 goto done;
595
596         if (filepath_valid(&fpt, file, talid)) {
597                 warnx("%s: File already visited", file);
598                 goto done;
599         }
600
601         rp = repo_byid(id);
602         repo_stat_inc(rp, talid, type, STYPE_OK);
603         repostats_new_files_inc(rp, file);

io.c
133 void
134 io_read_str(struct ibuf *b, char **res)
135 {
136         size_t   sz;
137
138         io_read_buf(b, &sz, sizeof(sz));
139         if (sz == 0) {
140                 *res = NULL;
141                 return;
142         }
143         if ((*res = calloc(sz + 1, 1)) == NULL)
144                 err(1, NULL);
145         io_read_buf(b, *res, sz);
146 }

repo.c
1267 struct repo *
1268 repo_byid(unsigned int id)
1269 {
1270         struct repo     *rp;
1271
1272         SLIST_FOREACH(rp, &repos, entry) {
1273                 if (rp->id == id)
1274                         return rp;
1275         }
1276         return NULL;
1277 }
```
