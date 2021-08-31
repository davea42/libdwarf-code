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
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#include "dd_regex.h"
#include "dwarf.h"
#include "libdwarf.h" /* for DW_DLV names */
#include "libdwarf_private.h" /* for TRUE FALSE */

#define DEBUG 1

#define ERROR  0  /* Never should be used */
#define BOL  1  /*^*/
#define EOL  2  /*$*/
#define BBK  3  /*[*/
#define EBK  4  /*]*/
#define CLO  5  /* match prev 0 or more times */
#define CLE  6  /* end of CLO */
#define ANY  7  /* match any char */
#define CHR  8  /* match a single char char */
#define MAXPREDEF 8
/* No need to have + the emitted automaton eliminates that,
    creating [x] CLO [x] */

#define MAXBUF 1024

typedef unsigned char UC;

static UC re_in[MAXBUF+2];

/* The created automaton.  The +2 enables savestrcpy. */
static UC   fa[MAXBUF+2];
static int  facount = 0;

/* Building automaton */
static UC   tmpfaa[MAXBUF+2];
static int  tmpfaacount = 0;
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
printf("dadebug facount %d %u  line %d\n",facount,c,__LINE__); \
    } else {                    \
        return DW_DLV_ERROR;    \
    }                           \
} while (0)
#define STORETMP(c)             \
do {                            \
    if (tmpfaacount < MAXBUF) {  \
        tmpfaa[tmpfaacount] = c;     \
        ++tmpfaacount;           \
printf("dadebug tmpfaacount %d %u line %d\n",tmpfaacount,c,__LINE__); \
    } else {                    \
printf("dadebug ERROR line %d\n",__LINE__); \
        return DW_DLV_ERROR;    \
    }                           \
} while (0)
#define STORETMPB(c)            \
do {                            \
    if (tmpfabcount < MAXBUF) { \
        tmpfab[tmpfabcount] = c;    \
printf("dadebug tmpfabcount %d %u line %d\n",tmpfabcount,c,__LINE__); \
        ++tmpfabcount;          \
    } else {                    \
        return DW_DLV_ERROR;    \
    }                           \
} while (0)

static char *
naming(UC c)
{
    static int use1;
    /*  Two result arrays so we can allow naming called twice
        in one printf. Turns any UC into a useful string.
        Array index must match the #define value for each name.*/
    static char nam1[10];
    static char nam2[10];
    static char * n[] = {
    "ERROR",
    "BOL",
    "EOL",
    "BBK",
    "EBK",
    "CLO",
    "CLE",
    "ANY",
    "CHR"
    };
    char *outtab= 0;

    if (use1) {
        outtab = nam1; 
    } else {
        outtab = nam2; 
    }
    outtab[0] = 0;
    if (c && c <= MAXPREDEF) {
        strcpy(outtab,n[c]);
    } else {
        sprintf(outtab,"0x%x\n",c);
    }
    use1 = !use1;
    return outtab;
}

const char *prefix = "    ";
#ifdef DEBUG
static void
fadump(UC *au, int len,const char *name,int line)
{
    int i = 0;

    printf("Dump of %d %s finite automaton line %d\n",
        len,name,line);
    for ( ; i < len; ++i) {
        UC c = au[i];
        printf("[%2d] ",i);
        switch(c) {
        case ANY:
            printf("%s",prefix);
            printf("ANY");
            break;
        case CHR:
            printf("%s",prefix);
            printf("CHR ");
            ++i;
            c = au[i];
            printf("%c",c);
            break;
        case BBK: {
            int j = 0;
            UC  count = 0;
            printf("%s",prefix);
            printf("BBK [");
            ++i;
            count = au[i];
            printf(" (count: %u) ",count);
            ++i;
            c = au[i];
            while(c != EBK) {
                printf("%c",c);
                ++j;
                ++i;
                c = au[i];
            }
            if (j != count) {
                printf("ERROR, internal error BBK/EBK count j %d count %d\n",
                    j,count);
            }
            printf(" ] EBK");
            }
            break;
        case BOL:
            printf("%s",prefix);
            printf("BOL ^");
            break; 
        case EOL:
            printf("%s",prefix);
            printf("EOL $ ");
            break; 
        case CLO: {
            UC ci  = 0;

            ++i;
            ci = au[i]; 
            switch(ci) {
            case ANY:
                printf("%s","CLO ");
                printf("ANY");
                ++i;
                ci = au[i];
                if (ci != CLE) {
                    printf(" unexpectd: 0x%x %s",ci,naming(ci));
                } else {
                    printf(" CLE");
                }
                break;
            case CHR:
                printf("%s","CLO ");
                printf("CHR ");
                ++i;
                ci = au[i];
                printf("%c",ci);
                ++i;
                ci = au[i];
                if (ci != CLE) {
                    printf(" unexpectd: 0x%x %s",ci,naming(ci));
                } else {
                    printf(" CLE");
                }
                break;
            case BBK: {
                int j = 0;
                UC count = 0;

                printf("%s","CLO ");
                printf("BBK [");
                ++i;
                count = au[i];
                printf(" (count: %u) ",count);
                ++i;
                ci = au[i];
                for(  ;ci != EBK;  ++i,++j,ci=au[i]) {
                    printf("%c",ci);
                }
                if (j != count) {
                    printf("ERROR, internal error BBK/EBK count j %d count %d\n",
                       j,count);
                }
                printf(" ] EBK");
                ++i;
                ci = au[i];
                if (ci != CLE) {
                    printf(" unexpectd: 0x%x %s",ci,naming(ci));
                } else {
                    printf(" CLE");
                }
                }
                break;
            default:
                printf("Error unexpected  element CLO at byte %d\n",i);
                return;
            } /* end switch */
            } /* end CLO */
            break;
        case CLE: {
            printf("%s CLE (out of place!) ",prefix);
            break;
            }
        default:
            if (!isascii(c)) {
                printf("%s 0x%x %s",prefix,c,naming(c));
            } else {
                printf("%s 0x%x %c",prefix,c,c);
            }
            break;
        } /* end switch */
        printf("\n");
    } /* End for loop */
}
#endif

static int
safestrcpy(UC *targ,int targlen,UC *src, int srclen)
{
    int u = 0;
    UC *in = src;
    UC *out = targ;
    for ( ; ; ++u,++in,++out) {
        if (u < srclen && u < targlen) {
            *out = *in;
            continue;
        }
        break;
    }
    /*  This is only relevant for the string copy, not for
        other uses. But harmless everywhere as we never
        step backwards in creating fa's. and our targets
        a a couple bytes bigger than MAXBUF */
    *out = 0;
    return u;
}
static void
resetfornewpattern(void)
{
    unsigned u = 0;
    for (; u < MAXBUF; ++u) {
        fa[u]     = 0;
        tmpfaa[u] = 0;
        tmpfab[u] = 0;
    }
    facount = 0;
    tmpfaacount = 0;
    tmpfabcount = 0;
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
    if (!tmpfaacount) {
printf("Empty tmpfaa\n");
        return;
    }
printf("dadebug facount %d line %d\n",facount,__LINE__);
printf("dadebug tmpfaacount %d line %d\n",tmpfaacount,__LINE__);
    facount += safestrcpy(fa+facount,MAXBUF-facount,
        tmpfaa,tmpfaacount);
printf("dadebug facount %d line %d\n",facount,__LINE__);
printf("dadebug tmpfaacount %d line %d\n",tmpfaacount,__LINE__);
    if (x == 'e') {
        tmpfaacount = 0;
        tmpfaa[0] = 0;
    } 
printf("dadebug facount %d line %d\n",facount,__LINE__);
printf("dadebug tmpfaacount %d line %d\n",tmpfaacount,__LINE__);
}

static void
copytmpabtotmpaa(void)
{
    if (!tmpfabcount) {
        return;
    }
printf("dadebug tmpfaacount %d line %d\n",tmpfaacount,__LINE__);
printf("dadebug tmpfabcount %d line %d\n",tmpfabcount,__LINE__);
    tmpfaacount += safestrcpy(tmpfaa+tmpfaacount,MAXBUF-tmpfaacount,
        tmpfab,tmpfabcount);
printf("dadebug tmpfaacount %d line %d\n",tmpfaacount,__LINE__);
    tmpfabcount = 0;
    tmpfab[0] = 0;
}

static void
emptytmp(void)
{
    tmpfaacount = 0;
    tmpfaa[0] = 0;
    tmpfabcount = 0;
    tmpfab[0] = 0;
}

/*  *pat points to '[' */
static int
fill_in_bracket_chars(UC *pat,int *absorbcount)
{
    int count = 0;
    int negate_all= FALSE;
    UC *cp = pat;
    int insequence=FALSE;
    UC lastchar = 0;


    /* Passing over the [ */
    ++count;
    tmpfabcount = 0;
    for ( cp = pat+1; *cp; ++cp,++count) { 
        UC c = *cp;
        if ( c == '^' && count == 1) {
            negate_all = TRUE;
            continue;
        }
        if (c == ']') {
            if (insequence) {
                return DW_DLV_ERROR;
            }
            if (!tmpfabcount) {
                return DW_DLV_ERROR;
            }
            break;
        }
        if (insequence) {
           UC cb = lastchar +1;
           if (cb > lastchar) {
               for ( ; cb <= c; ++cb) {
                    STORETMPB(cb);
               }
            } else {
                return DW_DLV_ERROR;
            }
            insequence = FALSE;
            continue;
        }
        if (c == '-') {
printf("dadebug insequence now, line %d\n",__LINE__);
             insequence = TRUE;
             continue;
        }
        STORETMPB(c);
        lastchar = c;
    }
    if (*cp != ']') {
        return DW_DLV_ERROR;
    }
    if (!negate_all) {
printf("dadebug tmpfaacount %d line %d\n",tmpfaacount,__LINE__);
printf("dadebug tmpfabcount %d line %d\n",tmpfabcount,__LINE__);
        STORETMP(BBK);
        STORETMP(tmpfabcount);
printf("dadebug tmpfaacount %d line %d\n",tmpfaacount,__LINE__);
printf("dadebug tmpfabcount %d line %d\n",tmpfabcount,__LINE__);
fadump(tmpfab,tmpfabcount," in [] tmpfab ",__LINE__);
        copytmpabtotmpaa();
printf("dadebug tmpfaacount %d line %d\n",tmpfaacount,__LINE__);
printf("dadebug tmpfabcount %d line %d\n",tmpfabcount,__LINE__);
        STORETMP(EBK);
        *absorbcount = count;
fadump(tmpfaa,tmpfaacount," in [] tmpfaa ",__LINE__);
printf("dadebug count in pat %d line %d\n",count,__LINE__);
printf("dadebug tmpfaacount %d line %d\n",tmpfaacount,__LINE__);
printf("dadebug tmpfabcount %d line %d\n",tmpfabcount,__LINE__);
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
    *absorbcount = count;
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
    safestrcpy(re_in,MAXBUF,(UC *)re_arg,strlen(re_arg));
    resetfornewpattern();
    for(pat = re_in; *pat; ++pat) { 
        UC pchar = *pat;
printf("dadebug pattern char %c pat 0x%lx\n",pchar,(unsigned long)pat);
        switch(pchar) {
        case '.':
            copytmptofinal('e');
            if (!previsCLOANY()) {
                STORETMP(ANY);
fadump(tmpfaa,tmpfaacount,"tmpfaa",__LINE__);
            } else {
printf("Dadebug emptytmp line dup CLO ANY %d\n",__LINE__);
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
            copytmptofinal('e');
            STORE(EOL);
            break;
        case '[': {
            int bbkcount = 0;
            int res = 0;
            copytmptofinal('e');
            res = fill_in_bracket_chars(pat,&bbkcount);
            if (res != DW_DLV_OK) {
                return res;
            }
            pat += bbkcount;
printf("dadebug bbkcount %d pat 0x%lx %s %d\n",bbkcount,(unsigned long)pat,naming(*pat),__LINE__);
            }
            break;
        case '*':
            if (!tmpfaacount) {
                return DW_DLV_ERROR;
            }
fadump(tmpfaa,tmpfaacount,"tmpfaa in * for *closure ",__LINE__);
            if ((tmpfaacount != 1  ||
               tmpfaa[0] != ANY) ||
               !previsCLOANY()) {
               STORE(CLO);
               copytmptofinal('e');
               STORE(CLE);
fadump(tmpfaa,tmpfaacount,"tmpfaa for *closure",__LINE__);
fadump(fa,facount,"fa for *closure",__LINE__);
            } else {
printf("Dadebug emptytmp line DUP CLO ANY %d\n",__LINE__);
                emptytmp();
            }
            break;
        case '+':
            if (!tmpfaacount) {
                return DW_DLV_ERROR;
            }
            if ((tmpfaacount != 1  ||  
                tmpfaa[0] != ANY) ||
                !previsCLOANY()) {
                copytmptofinal('k');
                STORE(CLO);
                copytmptofinal('e');
                STORE(CLE);
            } else {
printf("Dadebug emptytmp line DUP CLO ANY %d\n",__LINE__);
                emptytmp();
            }
            break;
        default:
            STORETMP(CHR);
            STORETMP(pchar);
            break;
        }
    }
fadump(fa,facount,"befoe finaltmp",__LINE__);
    copytmptofinal('e');
fadump(fa,facount,"post final tmp",__LINE__);
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
dd_re_exec(const char * x UNUSEDARG )
{
    return DW_DLV_ERROR;
}
