/*  Copyright (c) 2021 David Anderson

*/

/*
    Encouraged to write my own by the work of
    Ozan S. Yigit (oz),Dept. of Computer Science,
    York University (Canada). His code is...very clever...
    and dense, not my style. 
    He placed is work in the Public Domain.
*/

/* the input data, exp_in. All characters restricted to ASCII.
^ starting postion in exp_in
. position in exp_in
[ ]  BBK...EBK matching single character(s) in []
[^] BBK...EBK matching single character(s) not in []
    not contained in brackets
$   matches end of line.
*   matches previous zero or mor times.
+   matches previous one or more times.
*/
#include "config.h"
#include "stdio.h"
#include "dd_regex.h"
#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"


#define BOL  1  /*^*/
#define EOL  2  /*$*/
#define BBK  3  /*[*/
#define EBK  4  /*]*/
#define CLO  5  /* match prev 0 or more times */
#define ANY  6  /* match any char */
#define CHR  7  /* match a single char char */
#define END  8  /* End of a BBK sequence or end of a CLO */
/* No need to have + the emitted automaton eliminates that,
    creating [x] CLO [x] */

#define MAXBUF 1024

typedef unsigned char UC;

static UC re_in[MAXBUF];

/* The created automaton.  The +2 enables savestrcpy. */
static UC   fa[MAXBUF+2];
static int  facount = 0;

/* Building automaton */
static UC   tmpfa[MAXBUF+2];
static int  tmpfacount = 0;
static UC   tmpfab[MAXBUF+2];
static int  tmpfabcount = 0;
#define isascii(c) ((c) >= ' ' && (c) < 0x7f)

/* These defines mean not having to use tests
on every store to return ERROR. */
#define STORE(c)                \
do {                            \
    if (facount < MAXBUF) {  \
        fa[facount] = c;     \
        ++facount;           \
printf("dadebug facount %d %u\n",facount,c); \
    } else {                    \
        return DW_DLV_ERROR;    \
    }                           \
} while (0)
#define STORETMP(c)             \
do {                            \
    if (tmpfacount < MAXBUF) {  \
        tmpfa[facount] = c;     \
        ++tmpfacount;           \
printf("dadebug tmpfacount %d %u\n",tmpfacount,c); \
    } else {                    \
        return DW_DLV_ERROR;    \
    }                           \
} while (0)
#define STORETMPB(c)            \
do {                            \
    if (tmpfabcount < MAXBUF) { \
        tmpfab[facount] = c;    \
printf("dadebug tmpfabcount %d %u\n",tmpfabcount,c); \
        ++tmpfabcount;          \
    } else {                    \
        return DW_DLV_ERROR;    \
    }                           \
} while (0)

const char *prefix = "        ";
#ifdef DEBUG
static void
fadump(UC *au, int len,const char *name,int line)
{
    int i = 0;

    printf("Dump of %d %s finite automaton line %d\n",
        len,name,line);
    for ( ; i < len; ++i) {
        UC c = au[i];
        switch(c) {
        case ANY:
            puts(prefix);
            printf("ANY .");
            printf("\n");
            break;
        case CHR:
            puts(prefix);
            printf("CHR ");
            ++i;
            c = fa[i];
            printf("%c",c);
            printf("\n");
            break;
        case BBK: {
            int j = 0;
            UC  count = 0;
            puts(prefix);
            printf("BBK [");
            ++i;
            count = fa[i];
            printf(" count: %u",count);
            ++i;
            c = fa[i];
            while(c != EBK) {
                printf("%c",c);
                ++j;
            }
            if (j != count) {
                printf("ERROR, internal error BBK/EBK count\n");
                return;
            }
            printf(" ] EBK");
            printf("\n");
            }
            break;
        case BOL:
            puts(prefix);
            printf("BOL ^");
            printf("\n");
            break; 
        case EOL:
            puts(prefix);
            printf("EOL $ ");
            printf("\n");
            break; 
        case CLO: {
            UC ci  = 0;

            ++i;
            ci = fa[i]; 
            switch(ci) {
            case ANY:
                puts("CLOSURE ");
                printf("ANY .");
                printf("\n");
                break;
            case CHR:
                puts("CLOSURE ");
                printf("CHR ");
                ++i;
                ci = fa[i];
                printf("%c",ci);
                printf("\n");
                break;
            case BBK: {
                int j = 0;
                UC count = 0;

                puts("CLOSURE ");
                printf("BBK [ ");
                ++i;
                count = fa[i];
                printf(" count: %u",c);
                ++i;
                ci = fa[i];
                while(c != EBK) {
                    printf("%c",ci);
                    ++j;
                }
                if (j != count) {
                    printf("ERROR, internal error BBK/EBK count\n");
                    return;
                }
                printf(" ] EBK");
                printf("\n");
                }
                break;
            default:
                printf("Error unexpected fa element CLO at byte %d\n",i);
                return;
            }
        } /* end CLO */
        default:
            printf("%s '%c'",prefix,c);
            printf("\n");
        } /* end switch */
    }
}
#endif

void 
safestrcpy(UC *targ,int targlen,UC *src, int srclen)
{
    int u = 0;
    UC *in = src;
    UC *out = targ;
    for (; *in ; ++u,++in,++out) {
        if (u < srclen && u < targlen) {
            *out = *in;
            continue;
        }
        break;
    }
    *out = 0;
}
static void
resetfornewpattern(void)
{
    unsigned u = 0;
    for (; u < MAXBUF; ++u) {
        fa[u]     = 0;
        tmpfa[u] = 0;
    }
    facount = 0;
    tmpfacount = 0;
}

static int
all_ascii (const char *pat)
{
    UC c = 0;
    const unsigned char *cp = (const unsigned char*)pat;

    for( ; *cp; ++cp) {
        c = *cp;
        if (!isascii(c)) {
            return FALSE;
        }
    }
    return TRUE;
}

static int
inset(UC cx,UC *known,int kcount)
{
    int i = 0;
    for(i = 0; i < kcount; ++i) {
        if (cx == known[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

static void
copytmptofinal(char x)
{
    if (!tmpfacount) {
        return;
    }
    safestrcpy(fa+facount,MAXBUF-facount,tmpfa,tmpfacount);
    facount += tmpfacount;
printf("Dadebug copytafinal now facount %d line %d\n",facount,__LINE__);
    if (x == 'e') {
printf("Dadebug emptytmpfa line %d\n",__LINE__);
        tmpfacount = 0;
        tmpfa[0] = 0;
    } 
}
static void
copytmpbtotmp(void)
{
    safestrcpy(tmpfa+tmpfacount,MAXBUF-tmpfacount,
        tmpfab,tmpfabcount);
}
static void
emptytmp(void)
{
    tmpfacount = 0;
    tmpfa[0] = 0;
    tmpfabcount = 0;
    tmpfab[0] = 0;
}

/*  *pat points to '[' */
static int
fill_in_bracket_chars(UC *pat,int *absorbcount)
{
    int count = 0;
    int negateall = FALSE;
    UC *cp = pat;

    for ( cp = pat+1; *cp; ++cp,++count) { 
        if ( *cp == '^' && !count) {
            negateall = TRUE;
            continue;
        }
        if (*cp == ']') {
            if (!tmpfabcount) {
                return DW_DLV_ERROR;
            }
            break;
        }
        STORETMPB(*cp);
    }
    if (*cp != ']') {
        return DW_DLV_ERROR;
    }
    if (!negateall) {
printf("dadebug tmpfacount %d line %d\n",tmpfacount,__LINE__);
        STORETMP(BBK);
        STORETMP(tmpfabcount);
        copytmpbtotmp();
        tmpfabcount = 0;
        tmpfab[0] = 0;
        STORETMP(EBK);
printf("dadebug tmpfacount %d line %d\n",tmpfacount,__LINE__);
        return DW_DLV_OK;
    }
    {
        UC tmpfac[MAXBUF];
        int tmpfaccount = 0;
        int i = 0;
        UC cx = 0;


        STORETMP(BBK);
        for (cx = ' '; cx > 0x7f; ++cx) {
           if (inset(cx,tmpfab,tmpfabcount)) {
               continue;
           }
           tmpfac[tmpfaccount] = cx;
           ++tmpfaccount;
        }
        STORETMP(tmpfaccount);
        for(i = 0; i < tmpfaccount; ++i) {
            STORETMP(tmpfac[i]);
        }
        tmpfabcount = 0;
        tmpfab[0] = 0;
        STORETMP(EBK);
    }
    return DW_DLV_OK;
}
static int
previsCLOANY(void)
{
    if (facount < 2) {
        return FALSE;
    }
    if (fa[facount-1] != ANY) {
        return FALSE;
    }
    if (fa[facount-2] != CLO) {
        return FALSE;
    }
    return TRUE;
}
static void
trimfinals(void)
{
    int changed=TRUE;
    while(facount > 0 && changed) {
        changed = FALSE;
        if (facount > 1 && previsCLOANY()) {
            changed = TRUE;
            facount -= 2;
            continue;
        }
        if (facount == 1) {
            if (fa[0] == EOL) {
                facount = 0;
            }
        }
        if (facount == 1) {
            if (fa[0] == BOL) {
                facount = 0;
            }
        }
    }
}

int
dd_re_comp(const char *re_arg)
{
    UC *pat = 0;
    if (!all_ascii(re_arg)) {
        return DW_DLV_ERROR;
    }
    if (!re_arg || !re_arg[0]) {
        return DW_DLV_ERROR;
    }
    resetfornewpattern();
    safestrcpy(re_in,MAXBUF,(UC *)re_arg,MAXBUF);
    for(pat = re_in; *pat; ++pat) { 
printf("dadebug pattern char %c\n",*pat);
        switch(*pat) {
        case '.':
            copytmptofinal('e');
            if (!previsCLOANY()) {
                STORETMP(ANY);
fadump(tmpfa,tmpfacount,"fmpfa");
            } else {
printf("Dadebug emptytmp line %d\n",__LINE__);
                emptytmp();
            }
            break;
        case '^':
            if (pat != re_in) {
                /* Oops must be at start */ 
                return DW_DLV_ERROR;
            }
            STORE(BOL);
            break;
        case '$':
            if (*(pat+1)) {
                /* Oops must be at end */ 
                return DW_DLV_ERROR;
            }
            STORE(EOL);
            break;
        case '[': {
            int bbkcount = 0;
            int res = 0;

            res = fill_in_bracket_chars(pat,&bbkcount);
            if (res != DW_DLV_OK) {
                return res;
            }
            pat += (bbkcount-1);
            }
            break;
        case '*':
            if (!tmpfacount) {
                return DW_DLV_ERROR;
            }
            if ((tmpfacount != 1  ||
               tmpfa[0] != ANY) ||
               !previsCLOANY()) {
               STORE(CLO);
               copytmptofinal('e');
               STORE(END);
fadump(fa,facount,"fa",__LINE__);
fadump(tmpfa,tmpfacount,"fmpfa"__LINE__);
            } else {
printf("Dadebug emptytmp line %d\n",__LINE__);
                emptytmp();
            }
            break;
        case '+':
            if (!tmpfacount) {
                return DW_DLV_ERROR;
            }
            if ((tmpfacount != 1  ||  
                tmpfa[0] != ANY) ||
                !previsCLOANY()) {
                copytmptofinal('k');
                STORE(CLO);
                copytmptofinal('e');
                STORE(END);
            } else {
printf("Dadebug emptytmp line %d\n",__LINE__);
                emptytmp();
            }
            break;
        default:
            STORETMP(CHR);
            STORETMP(*pat);
            break;
        }
    }
    copytmptofinal('e');
    trimfinals(); 
#ifdef DEBUG
    fadump(fa,facount,"Full fa",__LINE__);
#endif
    if (!facount) {
        return DW_DLV_ERROR;
    }
    return DW_DLV_OK;
}

int
dd_re_exec(const char *exp_in)
{
    return DW_DLV_ERROR;
}
