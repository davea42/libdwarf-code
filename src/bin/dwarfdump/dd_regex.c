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
 *                      Only ASCII characters allowed in this call.
 *            char *re_comp(s)
 *            char *s;
 *
 *      re_exec:        execute the NFA to match a pattern.
 *                      ASCII and also 8 bit characters allowed,
 *                      including UTF-9 and iso_8859.
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
#include "libdwarf_private.h" /* TRUE/FALSE */
#include "dd_regex.h"

#ifndef DW_DLV_OK 
/*  Makes testing easier */
#define DW_DLV_NO_ENTRY -1
#define DW_DLV_OK        0
#define DW_DLV_ERROR     1
#endif /* DW_DLV_OK */


#define MAXNFA  1024
#define MAXTAG  10

/* status (sta) flags */
#define OKP     1
#define NOP     0

/* nfa values */
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

#define END     12

#ifdef DEBUG
static int recurlevel = 0;
#define RECURLEVEL(x) recurlevel += (x)
/*  Max two pachar in one printf line allowed */
static char charstra[10];
static char charstrb[10];
static char *pachar(unsigned char uc)
{
    static int last = 0;
    char *out = last?charstrb:charstra;
    if (uc < ' ') {
        sprintf(out,"0x%02x",uc);
    } else {
        out[0] = uc;
        out[1] = 0;
    }
    last = !last;
    return out;
}
static const char *naming(int i)
{
    if (i > 12) {
        return "";
    }
    switch(i) {
    case 0: return "zero";
    case 1: return "CHR";
    case 2: return "ANY";
    case 3: return "CCL";
    case 4: return "BOL";
    case 5: return "EOL";
    case 6: return "BOT";
    case 7: return "EOT";
    case 10: return "REF";
    case 11: return "CLO";
    case 12: return "END";
    }
    return "";
}
#else /* !DEBUG */
#define RECURLEVEL(x)
#endif /* ! DEBUG */

/*
 * The following defines are not meant to be changeable.
 * They are for readability only.
 */
#define MAXCHR    128
#define CHRBIT    8
#define BITBLK    MAXCHR/CHRBIT
#define BLKIND    0170
#define BITIND    07

typedef unsigned char CHAR;

static int  tagstk[MAXTAG];  /* subpat tag stack..*/
static CHAR nfa[MAXNFA];     /* automaton..       */
static int  sta = NOP;       /* status of lastpat */

static CHAR bittab[BITBLK];  /* bit table for CCL */
                    /* pre-set bits...   */
static CHAR bitarr[] = {1,2,4,8,16,32,64,128};


#ifdef DEBUG
static void nfadump(CHAR *);
static void symbolic(const char *);
#endif

static void
chset(CHAR c)
{
    CHAR blk = 0;
    CHAR ind = 0;
    CHAR bi = 0;

    blk = ((c) & BLKIND) >> 3;
    bi = (c) & BITIND;
    ind = bitarr[bi];
    bittab[blk] |= ind;
}

#define badpat    (*nfa = END)
/*  Stores into nfa */
#define store(x) {  *mp++ = (x) ;\
printf("dadebug store 0x%x %s at 0x%lx\n",(x),naming(x),(unsigned long)(mp-1));}

static void
resetfornewpattern(void)
{
     int j = 0;
     for (j = 0; j < BITBLK; ++j) {
         bittab[j] = 0;
     }
     for (j = 0; j < MAXTAG; ++j) {
         tagstk[j] =0;
     }
     for (j = 0; j < MAXNFA; ++j) {
         nfa[j] =0;
     }
}

static int
all_ascii (const char *pat)
{
    const unsigned char *cp = (const unsigned char*)pat;
    for( ; *cp; ++cp) {
        if (*cp >= 0x7f) {
            return FALSE;
        }
    }
    return TRUE;
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

    if (!all_ascii(pat)) {
        return DW_DLV_ERROR;
    }
    resetfornewpattern();
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
printf("dadebug switch pattern %s mp 0x%lx sp 0x%lx lp 0x%lx\n",pachar(*p),(unsigned long)mp,(unsigned long)sp,(unsigned long)lp);
        switch(*p) {
        case '.':               /* match any char..  */
            store(ANY);
            break;

        case '^':               /* match beginning.. */
            if (p == pat) {
                store(BOL);
            } else {
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
            while (*p && (*p != ']')) {
                if (*p == '-' && *(p+1) && *(p+1) != ']') {
                    p++;
                    c1 = *(p-2) + 1;
                    c2 = *p++;
                    while (c1 <= c2) {
                        chset((CHAR)c1++);
                    }
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
            for (n = 0; n < BITBLK; n++) {
printf("dadebug CCL val %d\n",n);
                CHAR maskval = mask ^ bittab[n];
                store(maskval);
                bittab[n] = 0;
            }
printf("dadebug CCL END line %d\n",__LINE__);
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
printf("dadebug break on CLO line %d\n",__LINE__);
                break;
            }
            switch(*lp) {

            case BOL:
            case BOT:
            case EOT:

#ifdef  ROBPIKEextension  
            case BOW:
            case EOW:
#endif
            case REF:
                badpat;
                printf("Regular expression %s has illegal "
                    "* + closure\n",pat);
                return DW_DLV_ERROR;
            default:
                break;
            }

            if (*p == '+') {
                int i = 0;
                for (sp = mp; lp < sp; lp++) {
printf("dadebug store another set entry (%d) for closure\n",i++);
                    store(*lp);
                }
            }
            store(END);
            store(END);
printf("dadebug mp to sp \n");
            sp = mp;
            while (--mp > lp) {
                *mp = mp[-1];
            }
            store(CLO);
printf("dadebug restore mp, done with * - closure \n");
            mp = sp;
            break;

        case '\\':              /* tags, backrefs .. */
            switch(*++p) {
            case '(':
                if (tagc < MAXTAG) {
                    tagstk[++tagi] = tagc;
                    store(BOT);
                    store(tagc);
                    ++tagc;
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
                    store(tagstk[tagi]);
                    tagi--;
                } else {
                    badpat;
                    printf("Regular expression has "
                        "unmatched \\)\n");
                    return DW_DLV_ERROR;
                }
                break;
#ifdef  ROBPIKEextension  
            This matches an expression to the beginning of a word.
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
#endif  /* ROBPIKEextension  */
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
printf("dadebug end switch pattern %s mp 0x%lx sp 0x%lx lp 0x%lx\n",pachar(*p),(unsigned long)mp,(unsigned long)sp,(unsigned long)lp);
        sp = lp;
    }
    if (tagi > 0) {
        badpat;
        printf("Regular expression has "
            "unmatched \\(\n");
        return DW_DLV_ERROR;
    }
printf("dadebug store END line %d\n",__LINE__);
    store(END);
    sta = OKP;
#ifdef DEBUG
    symbolic(pat);
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

printf("dadebug re_exec entry line %d\n",__LINE__);
    switch(*ap) {
    case BOL:            /* anchored: match from BOL only */
        RECURLEVEL(1);
        res = dd_pmatch(lp,ap,&ep);
        RECURLEVEL(-1);
printf("dadebug dd_pmatch ret %d now recurlev %d line %d\n",res,recurlevel,__LINE__);
        if (res != DW_DLV_OK) {
            return res;
        }
        break;
    case CHR:            /* ordinary char: locate it fast */
        c = *(ap+1);
        while (*lp && (*lp != c)) {
            lp++;
        }
        if (!*lp) {      /* if EOS, fail, else fall thru. */
            return DW_DLV_NO_ENTRY;
        }
        goto regmatch; /* GO TO avoids a fall-through warning from gcc */
    default:    {        /* regular matching all the way. */
        regmatch:
        do {
printf("dadebug now  dd_pmatch lp 0x%lx ap  0x%lx *lp 0x%x %s ret %d recurlevel %d line %d\n",(unsigned long)lp,(unsigned long)ap,*lp,pachar(*lp),res,recurlevel,__LINE__);
            RECURLEVEL(1);
            res = dd_pmatch(lp,ap,&ep);
            RECURLEVEL(-1);
printf("dadebug dd_pmatch ret %d now recurlev %d line %d\n",res,recurlevel,__LINE__);
            if (res == DW_DLV_ERROR) {
printf("dadebug exec ret DW_DLV_ERROR line %d\n",__LINE__);
                return res;
            }
            if (res == DW_DLV_OK) {
printf("dadebug exec want DW_DLV_OK line %d\n",__LINE__);
                break;
            }
            lp++;
        } while (*lp);
printf("dadebug exec ret %d line %d\n",res,__LINE__);
        if (res == DW_DLV_NO_ENTRY) {
             /* Never found */
             return res;
        }
        break;
    }
    case END:            /* munged automaton. fail always */
        badpat;
        printf("Error in in regex automaton. "
            "END out of place\n");
        return DW_DLV_ERROR;
    } /* end switch on *ap */
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
 *    TRUE for 0-9 A-Z a-z _ and to support things
 *    like iso-8859-1 and UTF8 all the bytes with the upper
 *    bit on are allowed too. We cannot handle UTF-8
 *    multi-byte codes in dw_re_comp call, but can in a de_re_exec
 *    call.
 */
#ifdef  ROBPIKEextension  
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
    1, 1, 1, 1, 1, 1, 0, 0,
};

#define inascii(x)    (0177&(x)) 
#define iswordc(x)    chrtyp[inascii(x)]
#endif /*  ROBPIKEextension */

#define isinset(x,y)  ((x)[((y)&BLKIND)>>3] & bitarr[(y)&BITIND])

/*
 * skip values for CLO XXX to skip past the closure
 */

#define ANYSKIP    2    /* [CLO] ANY END ...         */
#define CHRSKIP    3    /* [CLO] CHR chr END ...     */
#define CCLSKIP   18    /* [CLO] CCL 32bytes END ... */

static int
dd_pmatch(const char *lp, CHAR *ap,char **end_ptr)
{
    int op    = 0; 
    int c    = 0;
    int n    = 0;
    char *bp = 0;   /* beginning of subpat.. */
    char *ep = 0;   /* ending of subpat..     */
    char *are = 0;  /* to save the line ptr. */
    char *e  = 0;    /* extra pointer for CLO */
    int res  = 0;

printf("dadebug dd_pmatch lp 0x%lx ap 0x%lx recurlevel %d line %d\n",(unsigned long)lp,(unsigned long)ap,recurlevel,__LINE__);
    while ((op = *ap++) != END) {
printf("dadebug dd_pmatch lp 0x%lx 0x%x %s op 0x%x  %s ap 0x%lx line %d\n",
(unsigned long)lp,(*lp),pachar(*lp),op,naming(op),(unsigned long)ap,__LINE__);
if (*lp == 'i') {
printf("dadebug at i recurlevel %d line %d\n",recurlevel,__LINE__);
}
        switch(op) {
        case CHR:
            if (*lp++ != *ap++) {
printf("dadebug CHR NO ENTRY line %d\n",__LINE__);
                return DW_DLV_NO_ENTRY;
            }
printf("dadebug CHR MATCH line %d\n",__LINE__);
            break;
        case ANY:
            if (!*lp++) {
printf("dadebug ANY MATCH now fail line %d\n",__LINE__);
                return DW_DLV_NO_ENTRY;
            }
printf("dadebug ANY MATCH line %d\n",__LINE__);
            break;
        case CCL:
            c = *lp++;
printf("dadebug CCL  check for char 0x%x %s in CCL line %d\n",c,pachar(c),__LINE__);
            if (!isinset(ap,c)) {
printf("dadebug CCL NO ENTRY line %d\n",__LINE__);
                return DW_DLV_NO_ENTRY;
            }
printf("dadebug CCL MATCH passed: line %d\n",__LINE__);
            ap += BITBLK;
            break;
        case BOL:
            if (lp != bol) {
                return DW_DLV_NO_ENTRY;
            }
printf("dadebug BOL MATCH line %d\n",__LINE__);
            break;
        case EOL:
            if (*lp) {
                return DW_DLV_NO_ENTRY;
            }
printf("dadebug EOL MATCH line %d\n",__LINE__);
            break;
        case BOT:
            bopat[*ap++] = (char *)lp;
            break;
        case EOT:
            eopat[*ap++] = (char *)lp;
            break;
#ifdef  ROBPIKEextension  
        case BOW:
            if ((lp!=bol && iswordc(lp[-1])) || !iswordc(*lp)) {
                return DW_DLV_NO_ENTRY;
            }
            break;
        case EOW:
            if (lp==bol || !iswordc(lp[-1]) || iswordc(*lp)) {
                return DW_DLV_NO_ENTRY;
            }
            break;
#endif /* ROBPIKEextension */
        case REF:
            n = *ap++;
            bp = bopat[n];
            ep = eopat[n];
            while (bp < ep) {
                if (*bp++ != *lp++) {
                    return DW_DLV_NO_ENTRY;
                }
printf("dadebug MATCH line %d\n",__LINE__);
            }
printf("dadebug MATCH line %d\n",__LINE__);
            break;
        case CLO:
            are = (char *)lp;
printf("dadebug CLO inner type 0x%x  %s line %d\n",*ap,naming(*ap),__LINE__);
            switch(*ap) {
            case ANY:
printf("dadebug now CLO/ANY MATCH c 0x%x %s %s line %d\n",c,pachar(c),naming(c),__LINE__);
                while (*lp) {
                    lp++;
                }
                n = ANYSKIP;
                break;
            case CHR:
                c = *(ap+1);
printf("dadebug now CLO/CHR MATCH c 0x%x %s %s line %d\n",c,
pachar(c),naming(c),__LINE__);
printf("dadebug now CLO/CHR MATCH 0x%x %s line %d\n",*lp,pachar(*lp),__LINE__);
                while (*lp && c == *lp) {
printf("dadebug MATCH c 0x%x %s %s line %d\n",c,pachar(c),naming(c),__LINE__);
                    lp++;
                }
                n = CHRSKIP;
                break;
            case CCL:
printf("dadebug now CLO/CCL MATCH s 0x%x %s %s lp 0x%lx %s line %d\n",c,pachar(c),naming(c),
-(unsigned long)lp,pachar(*lp),__LINE__);
                while ((c = *lp) && isinset(ap+1,c)) {
printf("dadebug MATCH c 0x%x %s lp 0x%lx %s line %d\n",c,pachar(c),
(unsigned long)lp,pachar(*lp),__LINE__);
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
printf("dadebug old ap 0x%lx curlevel %d char at 0x%lx %s line %d\n",(unsigned long)ap,recurlevel,(unsigned long)lp,pachar(*lp),__LINE__);
            ap += n;
printf("dadebug new ap 0x%lx curlevel %d char at 0x%lx %s line %d\n",(unsigned long)ap,recurlevel,(unsigned long)lp,pachar(*lp),__LINE__);
            /*  We likely are past where this CLO applied
                so back up so lp points into the CLO/CCL we are finished with. */
            while (lp >= are) {
printf("dadebug ARE RECURSE INTO dd_pmatch, find matching lp 0x%lx *lp %s.line %d\n",(unsigned long) lp,pachar(*lp),__LINE__);
                RECURLEVEL(1);
                res = dd_pmatch(lp, ap,&e);
printf("dadebug ARE END RECURSION of level %d dd_pmatch res %d line %d\n",recurlevel,res,__LINE__);
                RECURLEVEL(-1);
printf("dadebug ARE RECURSION now %d dd_pmatch  line %d\n",recurlevel,__LINE__);
                if (res == DW_DLV_ERROR) {
                   return res;
                }
                if (res == DW_DLV_OK) {
                    *end_ptr = e;
                    return res; 
                }
#if 0
                If the failure is on a CHR or CCL it is too late. 
#endif
                /* NO_ENTRY so far */
                --lp;
printf("dadebug ARE NO ENTRY backup one character lp now 0x%lx %s line %d\n",(unsigned long)lp,pachar(*lp),__LINE__);
            }
printf("dadebug NO ENTRY line %d\n",__LINE__);
            return DW_DLV_NO_ENTRY;
        default:
            badpat;
            printf("Regular expression has illegal "
                "dd_re_exec: bad nfa.\n");
            return DW_DLV_ERROR;
        }
    }
printf("dadebug OK end ptr set 0x%lx line %d\n",(unsigned long)lp,__LINE__);
    *end_ptr = (char *)lp;
    return DW_DLV_OK;
}

#ifdef DEBUG
/*
 * symbolic - produce a symbolic dump of the nfa
 */
static void
symbolic(const char *s) 
{
    printf("pattern: %s\n", s);
    printf("nfacode:\n");
    nfadump(nfa);
}

static void   
nfadump(CHAR *ap)
{
    register int n;

    while (*ap != END) {
printf("dadebug next nfa 0x%lx\n",(unsigned long)ap);
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
printf("dadebug end CLOSURE next nfa 0x%lx\n",(unsigned long)ap);
            break;
        case CHR:
            printf("\tCHR %s\n",pachar(*ap));
            ++ap;
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
#ifdef  ROBPIKEextension  
        case BOW:
            printf("BOW\n");
            break;
        case EOW:
            printf("EOW\n");
            break;
#endif
        case REF:
            printf("REF: %d\n",*ap++);
            break;
        case CCL:
            printf("\tCCL [");
            for (n = 0; n < MAXCHR; n++)
                if (isinset(ap,(CHAR)n)) {
                    printf("%s", pachar(n));
                }
            printf("] \n");
            ap += BITBLK;
printf("dadebug CCL next nfa 0x%lx\n",(unsigned long)ap);
            break;
        default:
            printf("bad nfa. opcode %o\n", ap[-1]);
            exit(1);
            break;
        }
    }
    printf("dadebug nfa end at 0x%lx\n",(unsigned long)ap);
}
#endif  /* DEBUG */
