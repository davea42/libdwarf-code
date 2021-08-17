#ifndef REGEX_H
#define REGEX_H

extern char *re_comp(char *);
extern int re_exec(char *);
extern void re_modw(char *);
extern int re_subs(char *, char *);

#endif /* REGEX_H */
