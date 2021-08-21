/*
 * regex - Regular expression pattern matching  and replacement
 *
 * By:  Ozan S. Yigit (oz)
 *      Dept. of Computer Science
 *      York University
 *
 * These routines are the PUBLIC DOMAIN equivalents of regex
 * routines as found in 4.nBSD UN*X, with minor extensions.
 *
 * These routines are derived from various implementations found
 * in software tools books, and Conroy's grep. They are NOT derived
 * from licensed/restricted software.
 * For more interesting/academic/complicated implementations,
 * see Henry Spencer's regexp routines, or GNU Emacs pattern
 * matching module.
 *
 * Modification history:
 *
 * $Log: regex.c,v $
 * Revision 1.4  1991/10/17  03:56:42  oz
 * miscellaneous changes, small cleanups etc.
 *
 * Revision 1.3  1989/04/01  14:18:09  oz
 * Change all references to a dfa: this is actually an nfa.
 *
 * Revision 1.2  88/08/28  15:36:04  oz
 * Use a complement bitmap to represent NCL.
 * This removes the need to have seperate
 * code in the pmatch case block - it is 
 * just CCL code now.
 * 
 * Use the actual CCL code in the CLO
 * section of ematch. No need for a recursive
 * pmatch call.
 * 
 * Use a bitmap table to set char bits in an
 * 8-bit chunk.
 * 
 * Interfaces:
 *      re_comp:        compile a regular expression into a NFA.
 *
 *            char *re_comp(s)
 *            char *s;
 *
 *      re_exec:        execute the NFA to match a pattern.
 *
 *            int re_exec(s)
 *            char *s;
 *
 *    re_modw        change re_exec's understanding of what a "word"
 *            looks like (for \< and \>) by adding into the
 *            hidden word-syntax table.
 *
 *            void re_modw(s)
 *            char *s;
 *
 *      re_subs:    substitute the matched portions in a new string.
 *
 *            int re_subs(src, dst)
 *            char *src;
 *            char *dst;
 *
 *    re_fail:    failure routine for re_exec.
 *
 *            void re_fail(msg, op)
 *            char *msg;
 *            char op;
 *  
 * Regular Expressions:
 *
 *      [1]     char    matches itself, unless it is a special
 *                      character (metachar): . \ [ ] * + ^ $
 *
 *      [2]     .       matches any character.
 *
 *      [3]     \       matches the character following it, except
 *            when followed by a left or right round bracket,
 *            a digit 1 to 9 or a left or right angle bracket. 
 *            (see [7], [8] and [9])
 *            It is used as an escape character for all 
 *            other meta-characters, and itself. When used
 *            in a set ([4]), it is treated as an ordinary
 *            character.
 *
 *      [4]     [set]   matches one of the characters in the set.
 *                      If the first character in the set is "^",
 *                      it matches a character NOT in the set, i.e. 
 *            complements the set. A shorthand S-E is 
 *            used to specify a set of characters S upto 
 *            E, inclusive. The special characters "]" and 
 *            "-" have no special meaning if they appear 
 *            as the first chars in the set.
 *                      examples:        match:
 *
 *                              [a-z]    any lowercase alpha
 *
 *                              [^]-]    any char except ] and -
 *
 *                              [^A-Z]   any char except uppercase
 *                                       alpha
 *
 *                              [a-zA-Z] any alpha
 *
 *      [5]     *       any regular expression form [1] to [4], followed by
 *                      closure char (*) matches zero or more matches of
 *                      that form.
 *
 *      [6]     +       same as [5], except it matches one or more.
 *
 *      [7]             a regular expression in the form [1] to [10], enclosed
 *                      as \(form\) matches what form matches. The enclosure
 *                      creates a set of tags, used for [8] and for
 *                      pattern substution. The tagged forms are numbered
 *            starting from 1.
 *
 *      [8]             a \ followed by a digit 1 to 9 matches whatever a
 *                      previously tagged regular expression ([7]) matched.
 *
 *    [9]    \<    a regular expression starting with a \< construct
 *        \>    and/or ending with a \> construct, restricts the
 *            pattern matching to the beginning of a word, and/or
 *            the end of a word. A word is defined to be a character
 *            string beginning and/or ending with the characters
 *            A-Z a-z 0-9 and _. It must also be preceded and/or
 *            followed by any character outside those mentioned.
 *
 *      [10]            a composite regular expression xy where x and y
 *                      are in the form [1] to [10] matches the longest
 *                      match of x followed by a match for y.
 *
 *      [11]    ^    a regular expression starting with a ^ character
 *        $    and/or ending with a $ character, restricts the
 *                      pattern matching to the beginning of the line,
 *                      or the end of line. [anchors] Elsewhere in the
 *            pattern, ^ and $ are treated as ordinary characters.
 *
 *
 * Acknowledgements:
 *
 *    HCR's Hugh Redelmeier has been most helpful in various
 *    stages of development. He convinced me to include BOW
 *    and EOW constructs, originally invented by Rob Pike at
 *    the University of Toronto.
 *
 * References:
 *              Software tools            Kernighan & Plauger
 *              Software tools in Pascal        Kernighan & Plauger
 *              Grep [rsx-11 C dist]            David Conroy
 *        ed - text editor        Un*x Programmer's Manual
 *        Advanced editing on Un*x    B. W. Kernighan
 *        RegExp routines            Henry Spencer
 *
 * Notes:
 *
 *    This implementation uses a bit-set representation for character
 *    classes for speed and compactness. Each character is represented 
 *    by one bit in a 128-bit block. Thus, CCL always takes a 
 *    constant 16 bytes in the internal nfa, and re_exec does a single
 *    bit comparison to locate the character in the set.
 *
 * Examples:
 *
 *    pattern:    foo*.*
 *    compile:    CHR f CHR o CLO CHR o END CLO ANY END END
 *    matches:    fo foo fooo foobar fobar foxx ...
 *
 *    pattern:    fo[ob]a[rz]    
 *    compile:    CHR f CHR o CCL bitset CHR a CCL bitset END
 *    matches:    fobar fooar fobaz fooaz
 *
 *    pattern:    foo\\+
 *    compile:    CHR f CHR o CHR o CHR \ CLO CHR \ END END
 *    matches:    foo\ foo\\ foo\\\  ...
 *
 *    pattern:    \(foo\)[1-3]\1    (same as foo[1-3]foo)
 *    compile:    BOT 1 CHR f CHR o CHR o EOT 1 CCL bitset REF 1 END
 *    matches:    foo1foo foo2foo foo3foo
 *
 *    pattern:    \(fo.*\)-\1
 *    compile:    BOT 1 CHR f CHR o CLO ANY END EOT 1 CHR - REF 1 END
 *    matches:    foo-foo fo-fo fob-fob foobar-foobar ...
 */
#include "config.h"
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include "dwarf.h"
#include "libdwarf.h" /* for DW_DLV_ names */
#include "dd_regex.h"

#ifndef DW_DLV_OK 
/*  Makes testing easier */
#define DW_DLV_NO_ENTRY -1
#define DW_DLV_OK        0
#define DW_DLV_ERROR     1
#endif /* DW_DLV_OK */


#define MAXNFA  1024
#define MAXTAG  10

#define OKP     1
#define NOP     0

#define CHR     1
#define ANY     2
#define CCL     3 /* character class [ */
#define BOL     4 /* beginning of line, ^ */
#define EOL     5 /* end of line, 5 */
#define BOT     6 /* beginning of tag, ( */
#define EOT     7 /* end of tag, ) */
#define BOW     8 /* nonstandard beginning of ? < */
#define EOW     9 /* nonstandard end  of ? > */
#define REF     10
#define CLO     11

#define END     0

/*
 * The following defines are not meant to be changeable.
 * They are for readability only.
 */
#define MAXCHR    128
#define BITTABMAX 4
#define CHRBIT    8
#define BITBLK    MAXCHR/CHRBIT
#define BLKIND    0170
#define BITIND    07

#define ASCIIB    0177

#ifdef NO_UCHAR
typedef char CHAR;
#else
typedef unsigned char CHAR;
#endif

static int  tagstk[MAXTAG];  /* subpat tag stack..*/
static CHAR nfa[MAXNFA];     /* automaton..       */
static int  sta = NOP;       /* status of lastpat */

static CHAR bittab[BITBLK];  /* bit table for CCL */
                    /* pre-set bits...   */
static CHAR bitarr[] = {1,2,4,8,16,32,64,128};


#ifdef DEBUG
static void nfadump(CHAR *);
static void symbolic(char *);
#endif

static void
chset(CHAR c)
{
    bittab[(CHAR) ((c) & BLKIND) >> 3] |= bitarr[(c) & BITIND];
}

#define badpat    (*nfa = END)
#define store(x)    *mp++ = x

static void
resetbittab(void)
{
     int i = 0;
     int j = 0;
     for (j = 0; j < BITBLK; ++j) {
         bittab[j] = 0;
     }
}
 
int
dd_re_comp(const char *pat)
{
    const char *p  = 0;           /* pattern pointer   */
    CHAR *mp = nfa;
    CHAR *lp = 0;          /* saved pointer..   */
    CHAR *sp = nfa;      /* another one..     */
    int tagi = 0;          /* tag stack index   */
    int tagc = 1;          /* actual tag count  */
    int n    = 0;
    CHAR mask = 0;        /* xor mask -CCL/NCL */
    int c1   = 0;
    int c2   = 0;

    resetbittab();
    if (!pat || !*pat) {
        if (sta) {
            return DW_DLV_NO_ENTRY;
        } else {
            /* Seems impossible to get here */
            badpat;
            printf("Regular expression has "
                "no previous regular expression\n");
            return DW_DLV_ERROR;
        }
    }
    sta = NOP;

    for (p = pat; *p; p++) {
        lp = mp;
        switch(*p) {
        case '.':               /* match any char..  */
            store(ANY);
            break;

        case '^':               /* match beginning.. */
            if (p == pat)
                store(BOL);
            else {
                store(CHR);
                store(*p);
            }
            break;

        case '$':               /* match endofline.. */
            if (!*(p+1)) {
                store(EOL);
            } else {
                store(CHR);
                store(*p);
            }
            break;

        case '[':               /* match char class..*/
            store(CCL);
            if (*++p == '^') {
                mask = 0377;    
                p++;
            } else {
                mask = 0;
            }
            if (*p == '-')  {       /* real dash */
                chset(*p++);
            }
            if (*p == ']') {       /* real brac */
                chset(*p++);
            }
            while (*p && *p != ']') {
                if (*p == '-' && *(p+1) && *(p+1) != ']') {
                    p++;
                    c1 = *(p-2) + 1;
                    c2 = *p++;
                    while (c1 <= c2)
                        chset((CHAR)c1++);
                }
#ifdef EXTEND
                else if (*p == '\\' && *(p+1)) {
                    p++;
                    chset(*p++);
                }
#endif /* EXTEND */
                else {
                    chset(*p++);
                }
            }
            if (!*p) {
                badpat;
                printf("Regular expression %s missing ]\n",pat);
                return DW_DLV_ERROR;
            }
            /* Storing the bitmask into nfa  */
            for (n = 0; n < BITBLK; bittab[n++] =
                (char) 0) {
                store(mask ^ bittab[n]);
            }
            resetbittab();
            break;

        case '*':               /* match 0 or more.. */
        case '+':               /* match 1 or more.. */
            if (p == pat) {
                badpat;
                printf("Regular expression %s has empty * +  "
                    "Closure\n",pat);
                return DW_DLV_ERROR;
            }
            lp = sp;        /* previous opcode */
            if (*lp == CLO)  {      /* equivalence..   */
                break;
            }
            switch(*lp) {

            case BOL:
            case BOT:
            case EOT:
            case BOW:
            case EOW:
            case REF:
                badpat;
                printf("Regular expression %s has illegal "
                    "* + closure\n",pat);
                return DW_DLV_ERROR;
            default:
                break;
            }

            if (*p == '+') {
                for (sp = mp; lp < sp; lp++)
                    store(*lp);
            }

            store(END);
            store(END);
            sp = mp;
            while (--mp > lp) {
                *mp = mp[-1];
            }
            store(CLO);
            mp = sp;
            break;

        case '\\':              /* tags, backrefs .. */
            switch(*++p) {

            case '(':
                if (tagc < MAXTAG) {
                    tagstk[++tagi] = tagc;
                    store(BOT);
                    store(tagc++);
                }
                else {
                    badpat;
                    printf("Regular expression %s has "
                        "too many \\(\\) pairs (%d)\n",pat,
                        tagc);
                    return DW_DLV_ERROR;
                }
                break;
            case ')':
                if (*sp == BOT) {
                    badpat;
                    printf("Regular expression has "
                        "null pattern inside \\(\\)\n");
                    return DW_DLV_ERROR;
                }
                if (tagi > 0) {
                    store(EOT);
                    store(tagstk[tagi--]);
                } else {
                    badpat;
                    printf("Regular expression has "
                        "unmatched \\)\n");
                    return DW_DLV_ERROR;
                }
                break;
            case '<':
                store(BOW);
                break;
            case '>':
                if (*sp == BOW) {
                    badpat;
                    printf("Regular expression has "
                        "null pattern inside \\<\\>\n");
                    return DW_DLV_ERROR;
                }
                store(EOW);
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                n = *p-'0';
                if (tagi > 0 && tagstk[tagi] == n)
                    badpat;
                    printf("Regular expression has "
                        "Cyclical reference\n");
                    return DW_DLV_ERROR;
                if (tagc > n) {
                    store(REF);
                    store(n);
                } else {
                    badpat;
                    printf("Regular expression has "
                        "undetermined reference");;
                    return DW_DLV_ERROR;
                }
                break;
#ifdef EXTEND
            case 'b':
                store(CHR);
                store('\b');
                break;
            case 'n':
                store(CHR);
                store('\n');
                break;
            case 'f':
                store(CHR);
                store('\f');
                break;
            case 'r':
                store(CHR);
                store('\r');
                break;
            case 't':
                store(CHR);
                store('\t');
                break;
#endif /* EXTEND */
            default:
                store(CHR);
                store(*p);
            }
            break;

        default :               /* an ordinary char  */
            store(CHR);
            store(*p);
            break;
        }
        sp = lp;
    }
    if (tagi > 0) {
        badpat;
        printf("Regular expression has "
            "unmatched \\(\n");
        return DW_DLV_ERROR;
    }
    store(END);
    sta = OKP;
#ifdef DEBUG
    nfadump(nfa);
#endif /* DEBUG */
    return DW_DLV_OK;
}


static char *bol;
static char *bopat[MAXTAG];
static char *eopat[MAXTAG];
static int dd_pmatch(const char *, CHAR *,char **str_out);

/*
 * re_exec:
 *     execute nfa to find a match.
 *
 *    special cases: (nfa[0])    
 *        BOL
 *            Match only once, starting from the
 *            beginning.
 *        CHR
 *            First locate the character without
 *            calling pmatch, and if found, call
 *            pmatch for the remaining string.
 *        END
 *            re_comp failed, poor luser did not
 *            check for it. Fail fast.
 *
 *    If a match is found, bopat[0] and eopat[0] are set
 *    to the beginning and the end of the matched fragment,
 *    respectively.
 *
 *    return DW_DLV_OK, DW_DLV_NO_ENTRY or DW_DLV_ERROR
 */

int
dd_re_exec(char *lp)
{
    CHAR c   = 0;
    char *ep = 0;
    CHAR *ap = nfa;
    int res  = 0;

    bol = lp;
    bopat[0] = 0;
    bopat[1] = 0;
    bopat[2] = 0;
    bopat[3] = 0;
    bopat[4] = 0;
    bopat[5] = 0;
    bopat[6] = 0;
    bopat[7] = 0;
    bopat[8] = 0;
    bopat[9] = 0;

    switch(*ap) {
    case BOL:            /* anchored: match from BOL only */
        res = dd_pmatch(lp,ap,&ep);
        if (res != DW_DLV_OK) {
            return res;
        }
        break;
    case CHR:            /* ordinary char: locate it fast */
        c = *(ap+1);
        while (*lp && *lp != c) {
            lp++;
        }
        if (!*lp) {      /* if EOS, fail, else fall thru. */
            return DW_DLV_NO_ENTRY;
        }
    default:            /* regular matching all the way. */
#ifdef OLD
        while (*lp) {
            res =  dd_pmatch(lp,ap,&ep)))
            if (res == DW_DLV_ERROR) {
                return res;
            }
            if (res == DW_DLV_OK) {
                break;
            }
            lp++;
        }
#else    /* match null string */
        do {
            res = dd_pmatch(lp,ap,&ep);
            if (res == DW_DLV_ERROR) {
                return res;
            }
            if (res == DW_DLV_OK) {
                break;
            }
            lp++;
        } while (*lp);
#endif /* OLD */
        break;
    case END:            /* munged automaton. fail always */
        badpat;
        printf("Error in in regex automaton. "
            "END out of place\n");
        return DW_DLV_ERROR;
    }
    if (res != DW_DLV_OK) {
        return res;
    }
    bopat[0] = lp;
    eopat[0] = ep;

    return DW_DLV_OK;
}

/* 
 * dd_pmatch: internal routine for the hard part
 *
 *     This code is partly snarfed from an early grep written by
 *    David Conroy. The backref and tag stuff, and various other
 *    innovations are by oz.
 *
 *    special case optimizations: (nfa[n], nfa[n+1])
 *        CLO ANY
 *            We KNOW .* will match everything upto the
 *            end of line. Thus, directly go to the end of
 *            line, without recursive pmatch calls. As in
 *            the other closure cases, the remaining pattern
 *            must be matched by moving backwards on the
 *            string recursively, to find a match for xy
 *            (x is ".*" and y is the remaining pattern)
 *            where the match satisfies the LONGEST match for
 *            x followed by a match for y.
 *        CLO CHR
 *            We can again scan the string forward for the
 *            single char and at the point of failure, we
 *            execute the remaining nfa recursively, same as
 *            above.
 *
 *    At the end of a successful match, bopat[n] and eopat[n]
 *    are set to the beginning and end of subpatterns matched
 *    by tagged expressions (n = 1 to 9).    
 *
 *      returns DW_DLV_OK for a match, and sets the ep pointer
 *          to point into the input at the next char to check
 *      DW_DLV_NO_ENTRY for a non-match
 *      DW_DLV_NO_ERROR for an internal error
 */

/*
 * character classification table for word boundary operators BOW
 * and EOW. the reason for not using ctype macros is that we can
 * let the user add into our own table. see re_modw. This table
 * is not in the bitset form, since we may wish to extend it in the
 * future for other character classifications. 
 *
 *    TRUE for 0-9 A-Z a-z _
 */
static CHAR chrtyp[MAXCHR] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 0, 0, 0, 0, 0
};

#define inascii(x)    (0177&(x))
#define iswordc(x)     chrtyp[inascii(x)]
#define isinset(x,y)     ((x)[((y)&BLKIND)>>3] & bitarr[(y)&BITIND])

/*
 * skip values for CLO XXX to skip past the closure
 */

#define ANYSKIP    2    /* [CLO] ANY END ...         */
#define CHRSKIP    3    /* [CLO] CHR chr END ...     */
#define CCLSKIP   18    /* [CLO] CCL 16bytes END ... */

static int
dd_pmatch(const char *lp, CHAR *ap,char **end_ptr)
{
    int op    = 0; 
    int c    = 0;
    int n    = 0;
    char *e  = 0;    /* extra pointer for CLO */
    char *bp = 0;   /* beginning of subpat.. */
    char *ep = 0;   /* ending of subpat..     */
    char *are = 0;           /* to save the line ptr. */
    int res  = 0;

    while ((op = *ap++) != END) {
        switch(op) {
        case CHR:
            if (*lp++ != *ap++) {
                return DW_DLV_NO_ENTRY;
            }
            break;
        case ANY:
            if (!*lp++) {
                return DW_DLV_NO_ENTRY;
            }
            break;
        case CCL:
            c = *lp++;
            if (!isinset(ap,c)) {
                return DW_DLV_NO_ENTRY;
            }

            ap += BITBLK;
            break;
        case BOL:
            if (lp != bol) {
                return DW_DLV_NO_ENTRY;
            }
            break;
        case EOL:
            if (*lp) {
                return DW_DLV_NO_ENTRY;
            }
            break;
        case BOT:
            bopat[*ap++] = (char *)lp;
            break;
        case EOT:
            eopat[*ap++] = (char *)lp;
            break;
        case BOW:
            if (lp!=bol && iswordc(lp[-1]) || !iswordc(*lp)) {
                return DW_DLV_NO_ENTRY;
            }
            break;
        case EOW:
            if (lp==bol || !iswordc(lp[-1]) || iswordc(*lp)) {
                return DW_DLV_NO_ENTRY;
            }
            break;
        case REF:
            n = *ap++;
            bp = bopat[n];
            ep = eopat[n];
            while (bp < ep) {
                if (*bp++ != *lp++) {
                    return DW_DLV_NO_ENTRY;
                }
            }
            break;
        case CLO:
            are = (char *)lp;
            switch(*ap) {
            case ANY:
                while (*lp) {
                    lp++;
                }
                n = ANYSKIP;
                break;
            case CHR:
                c = *(ap+1);
                while (*lp && c == *lp) {
                    lp++;
                }
                n = CHRSKIP;
                break;
            case CCL:
                while ((c = *lp) && isinset(ap+1,c)) {
                    lp++;
                }
                n = CCLSKIP;
                break;
            default:
                badpat;
                printf("Regular expression has illegal "
                    "closure: bad nfa\n");
                return DW_DLV_ERROR;
            }
            ap += n;
            while (lp >= are) {
                res = dd_pmatch(lp, ap,&e);
                if (res == DW_DLV_ERROR) {
                   return res;
                }
                if (res == DW_DLV_OK) {
                    *end_ptr = e;
                    return res; 
                }
                /* NO_ENTRY so far */
                --lp;
            }
            return DW_DLV_NO_ENTRY;
        default:
            badpat;
            printf("Regular expression has illegal "
                "dd_re_exec: bad nfa.\n");
            return DW_DLV_ERROR;
        }
    }
    *end_ptr = (char *)lp;
    return DW_DLV_OK;
}

#ifdef DEBUG
/*
 * symbolic - produce a symbolic dump of the nfa
 */
static void
symbolic(char *s) 
{
    printf("pattern: %s\n", s);
    printf("nfacode:\n");
    nfadump(nfa);
}

static void   
nfadump(CHAR *ap)
{
    register int n;

    while (*ap != END)
        switch(*ap++) {
        case CLO:
            printf("CLOSURE");
            nfadump(ap);
            switch(*ap) {
            case CHR:
                n = CHRSKIP;
                break;
            case ANY:
                n = ANYSKIP;
                break;
            case CCL:
                n = CCLSKIP;
                break;
            }
            ap += n;
            break;
        case CHR:
            printf("\tCHR %c\n",*ap++);
            break;
        case ANY:
            printf("\tANY .\n");
            break;
        case BOL:
            printf("\tBOL -\n");
            break;
        case EOL:
            printf("\tEOL -\n");
            break;
        case BOT:
            printf("BOT: %d\n",*ap++);
            break;
        case EOT:
            printf("EOT: %d\n",*ap++);
            break;
        case BOW:
            printf("BOW\n");
            break;
        case EOW:
            printf("EOW\n");
            break;
        case REF:
            printf("REF: %d\n",*ap++);
            break;
        case CCL:
            printf("\tCCL [");
            for (n = 0; n < MAXCHR; n++)
                if (isinset(ap,(CHAR)n)) {
                    if (n < ' ')
                        printf("^%c", n ^ 0x040);
                    else
                        printf("%c", n);
                }
            printf("]\n");
            ap += BITBLK;
            break;
        default:
            printf("bad nfa. opcode %o\n", ap[-1]);
            exit(1);
            break;
        }
}
#endif  /* DEBUG */
