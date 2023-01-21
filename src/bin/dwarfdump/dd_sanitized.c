/*
Copyright 2016-2018 David Anderson. All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of version 2 of the GNU General
  Public License as published by the Free Software Foundation.

  This program is distributed in the hope that it would be
  useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  Further, this software is distributed without any warranty
  that it is free of the rightful claim of any third person
  regarding infringement or the like.  Any license provided
  herein, whether implied or otherwise, applies only to this
  software file.  Patent licenses, if any, provided herein
  do not apply to combinations of this program with other
  software, or any other product whatsoever.

  You should have received a copy of the GNU General Public
  License along with this program; if not, write the Free
  Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
  Boston MA 02110-1301, USA.

*/

#include <config.h>

#include "dwarf.h"
#include "libdwarf.h"
#include "dd_globals.h"
#include "dd_esb.h"
#ifndef TESTING
#include "dd_glflags.h"
#endif
#include "dd_sanitized.h"

/*  This does a uri-style conversion of control characters.
    So  SOH prints as %01 for example.
    Which stops corrupted or crafted strings from
    doing things to the terminal the string is routed to.

    We do not translate an input % to %% (as in real uri)
    as that would be a bit confusing for most readers.

    The conversion makes it possble to print UTF-8 strings
    reproducibly, sort of (not showing the
    real glyph!).

    Only call this in a printf or sprintf, and
    only call it once in any single printf/sprintf.
    Otherwise you will get bogus results and confusion. */

/* ASCII control codes:
We leave newline as is, NUL is end of string,
the others are translated.
NUL Null             0  00              Ctrl-@ ^@
SOH Start of heading 1  01      Alt-1   Ctrl-A ^A
STX Start of text    2  02      Alt-2   Ctrl-B ^B
ETX End of text      3  03      Alt-3   Ctrl-C ^C
EOT End of transmission 4 04    Alt-4   Ctrl-D ^D
ENQ Enquiry          5    05    Alt-5   Ctrl-E ^E
ACK Acknowledge      6    06    Alt-6   Ctrl-F ^F
BEL Bell             7    07    Alt-7   Ctrl-G ^G
BS  Backspace        8    08    Alt-8   Ctrl-H ^H
HT  Horizontal tab   9    09    Alt-9   Ctrl-I ^I
LF  Line feed       10    0A    Alt-10  Ctrl-J ^J
VT  Vertical tab    11    0B    Alt-11  Ctrl-K ^K
FF  Form feed       12    0C    Alt-12  Ctrl-L ^L
CR  Carriage return 13    0D    Alt-13  Ctrl-M ^M
SO  Shift out       14    0E    Alt-14  Ctrl-N ^N
SI  Shift in        15    0F    Alt-15  Ctrl-O ^O
DLE Data line escape 16   10    Alt-16  Ctrl-P ^P
DC1 Device control 1 17   11    Alt-17  Ctrl-Q ^Q
DC2 Device control 2 18   12    Alt-18  Ctrl-R ^R
DC3 Device control 3 19   13    Alt-19  Ctrl-S ^S
DC4 Device control 4 20   14    Alt-20  Ctrl-T ^T
NAK Negative acknowledge 21 15  Alt-21  Ctrl-U ^U
SYN Synchronous idle 22   16    Alt-22  Ctrl-V ^V
ETB End transmission block 23 17 Alt-23 Ctrl-W ^W
CAN Cancel              24 18   Alt-24  Ctrl-X ^X
EM  End of medium       25 19   Alt-25  Ctrl-Y ^Y
SU  Substitute          26 1A   Alt-26  Ctrl-Z ^Z
ES  Escape              27 1B   Alt-27  Ctrl-[ ^[
FS  File separator      28 1C   Alt-28  Ctrl-\ ^\
GS  Group separator     29 1D   Alt-29  Ctrl-] ^]
RS  Record separator    30 1E   Alt-30  Ctrl-^ ^^
US  Unit separator      31 1F   Alt-31  Ctrl-_ ^_

In addition,  characters decimal 141, 157, 127,128, 129
143,144,157
appear to be questionable too.
Not in iso-8859-1 nor in html character entities list.

We translate all strings with a % to do sanitizing and
we change a literal ASCII '%' char to %27 so readers
know any % is a sanitized char. We could double up
a % into %% on output, but switching to %27 is simpler
and for readers and prevents ambiguity.

Since we do not handle utf-8 properly nor detect it
we turn all non-ASCII to %xx below.
*/
#define SANBUF_SIZE 400
/*  Allocates as esb_constructor_fixed() would,
    so below SANBUF_SIZE bytes no malloc needed. */
static char sanbuf[SANBUF_SIZE];
static struct esb_s localesb = {sanbuf,
    SANBUF_SIZE,0,1,0};

/* dwarfdump-sanitize table */
char dwarfdump_sanitize_table[256] = {
0 /*0*/,3 /*0x1*/,3 /*0x2*/,3 /*0x3*/,
3 /*0x4*/,3 /*0x5*/,3 /*0x6*/,3 /*0x7*/,
3 /*0x8*/,1 /*0x9*/,1 /*0xa*/,3 /*0xb*/,
3 /*0xc*/,
#ifdef _WIN32
1 /*0x0d*/,
#else
3 /*0x0d*/,
#endif
3 /*0xe*/,3 /*0xf*/,
3 /*0x10*/,3 /*0x11*/,3 /*0x12*/,3 /*0x13*/,
3 /*0x14*/,3 /*0x15*/,3 /*0x16*/,3 /*0x17*/,
3 /*0x18*/,3 /*0x19*/,3 /*0x1a*/,3 /*0x1b*/,
3 /*0x1c*/,3 /*0x1d*/,3 /*0x1e*/,3 /*0x1f*/,
1 /*   */,1 /* ! */,1 /* " */,1 /* # */,
1 /* $ */,3 /* % */,1 /* & */,1 /* ' */,
1 /* ( */,1 /* ) */,1 /* * */,1 /* + */,
1 /* , */,1 /* - */,1 /* . */,1 /* / */,
1 /* 0 */,1 /* 1 */,1 /* 2 */,1 /* 3 */,
1 /* 4 */,1 /* 5 */,1 /* 6 */,1 /* 7 */,
1 /* 8 */,1 /* 9 */,1 /* : */,1 /* ; */,
1 /* < */,1 /* = */,1 /* > */,1 /* ? */,
1 /* @ */,1 /* A */,1 /* B */,1 /* C */,
1 /* D */,1 /* E */,1 /* F */,1 /* G */,
1 /* H */,1 /* I */,1 /* J */,1 /* K */,
1 /* L */,1 /* M */,1 /* N */,1 /* O */,
1 /* P */,1 /* Q */,1 /* R */,1 /* S */,
1 /* T */,1 /* U */,1 /* V */,1 /* W */,
1 /* X */,1 /* Y */,1 /* Z */,1 /* [ */,
1 /* \ */,1 /* ] */,1 /* ^ */,1 /* _ */,
1 /* ` */,1 /* a */,1 /* b */,1 /* c */,
1 /* d */,1 /* e */,1 /* f */,1 /* g */,
1 /* h */,1 /* i */,1 /* j */,1 /* k */,
1 /* l */,1 /* m */,1 /* n */,1 /* o */,
1 /* p */,1 /* q */,1 /* r */,1 /* s */,
1 /* t */,1 /* u */,1 /* v */,1 /* w */,
1 /* x */,1 /* y */,1 /* z */,1 /* { */,
1 /* | */,1 /* } */,1 /* ~ */,3 /*0x7f*/,
3 /*0x80*/,3 /*0x81*/,3 /*0x82*/,3 /*0x83*/,
3 /*0x84*/,3 /*0x85*/,3 /*0x86*/,3 /*0x87*/,
3 /*0x88*/,3 /*0x89*/,3 /*0x8a*/,3 /*0x8b*/,
3 /*0x8c*/,3 /*0x8d*/,3 /*0x8e*/,3 /*0x8f*/,
3 /*0x90*/,3 /*0x91*/,3 /*0x92*/,3 /*0x93*/,
3 /*0x94*/,3 /*0x95*/,3 /*0x96*/,3 /*0x97*/,
3 /*0x98*/,3 /*0x99*/,3 /*0x9a*/,3 /*0x9b*/,
3 /*0x9c*/,3 /*0x9d*/,3 /*0x9e*/,3 /*0x9f*/,
3 /*0xa0*/,3 /*0xa1*/,3 /*0xa2*/,3 /*0xa3*/,
3 /*0xa4*/,3 /*0xa5*/,3 /*0xa6*/,3 /*0xa7*/,
3 /*0xa8*/,3 /*0xa9*/,3 /*0xaa*/,3 /*0xab*/,
3 /*0xac*/,3 /*0xad*/,3 /*0xae*/,3 /*0xaf*/,
3 /*0xb0*/,3 /*0xb1*/,3 /*0xb2*/,3 /*0xb3*/,
3 /*0xb4*/,3 /*0xb5*/,3 /*0xb6*/,3 /*0xb7*/,
3 /*0xb8*/,3 /*0xb9*/,3 /*0xba*/,3 /*0xbb*/,
3 /*0xbc*/,3 /*0xbd*/,3 /*0xbe*/,3 /*0xbf*/,
3 /*0xc0*/,3 /*0xc1*/,3 /*0xc2*/,3 /*0xc3*/,
3 /*0xc4*/,3 /*0xc5*/,3 /*0xc6*/,3 /*0xc7*/,
3 /*0xc8*/,3 /*0xc9*/,3 /*0xca*/,3 /*0xcb*/,
3 /*0xcc*/,3 /*0xcd*/,3 /*0xce*/,3 /*0xcf*/,
3 /*0xd0*/,3 /*0xd1*/,3 /*0xd2*/,3 /*0xd3*/,
3 /*0xd4*/,3 /*0xd5*/,3 /*0xd6*/,3 /*0xd7*/,
3 /*0xd8*/,3 /*0xd9*/,3 /*0xda*/,3 /*0xdb*/,
3 /*0xdc*/,3 /*0xdd*/,3 /*0xde*/,3 /*0xdf*/,
3 /*0xe0*/,3 /*0xe1*/,3 /*0xe2*/,3 /*0xe3*/,
3 /*0xe4*/,3 /*0xe5*/,3 /*0xe6*/,3 /*0xe7*/,
3 /*0xe8*/,3 /*0xe9*/,3 /*0xea*/,3 /*0xeb*/,
3 /*0xec*/,3 /*0xed*/,3 /*0xee*/,3 /*0xef*/,
3 /*0xf0*/,3 /*0xf1*/,3 /*0xf2*/,3 /*0xf3*/,
3 /*0xf4*/,3 /*0xf5*/,3 /*0xf6*/,3 /*0xf7*/,
3 /*0xf8*/,3 /*0xf9*/,3 /*0xfa*/,3 /*0xfb*/,
3 /*0xfc*/,3 /*0xfd*/,3 /*0xfe*/,3 /*0xff*/,};

/*  do_sanity_insert() and no_questionable_chars()
    absolutely must have the same idea of
    questionable characters.  Be Careful.
    Until 0.5.0 the two did NOT agree on
    handling of newline or tab or carriage return
    which was... a bug.
    no_questionable_chars() said those three ok
    so do_sanity_insert was never called, making
    an inconsistency vs this code as it was
    before 0.5.0.
    */
static void
do_sanity_insert( const char *s,struct esb_s *mesb)
{
    const char *cp = s;

    for ( ; *cp; cp++) {
        unsigned c = *cp & 0xff ;
        int t = dwarfdump_sanitize_table[c];

        if (t == 1) {
            esb_appendn(mesb,cp,1);
            continue;
        }
        esb_appendn(mesb, "%",1);
        esb_append_printf_u(mesb, "%02x",c & 0xff);
    }
}

/*  This routine improves overall dwarfdump
    run times a lot by separating strings
    that might print badly from strings that
    will print fine.
    In one large test case it reduces run time
    from 140 seconds to 13 seconds. */
static int
no_questionable_chars(const char *s) {
    const char *cp = s;

    for ( ; *cp; cp++) {
        unsigned c = *cp & 0xff ;
        int t = dwarfdump_sanitize_table[c];

        if (t == 1) {
            continue;
        }
        return FALSE;
    }
    return TRUE;
}

void
sanitized_string_destructor(void)
{
    esb_destructor(&localesb);
}

const char *
sanitized(const char *s)
{
    const char *sout = 0;

#ifndef TESTING
    if (glflags.gf_no_sanitize_strings) {
        return s;
    }
#endif
    if (no_questionable_chars(s)) {
        /*  The original string is safe as is. */
        return s;
    }
    /*  Using esb_destructor is quite expensive in cpu time
        when we build the next sanitized string
        so we just empty the localesb.
        One reason it's expensive is that we do the appends
        in such small batches in do_sanity-insert().
        */
    esb_empty_string(&localesb);
    do_sanity_insert(s,&localesb);
    sout = esb_get_string(&localesb);
    return sout;
}
