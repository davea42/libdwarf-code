/*
  Copyright 2016-2016 David Anderson. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 51
  Franklin Street - Fifth Floor, Boston MA 02110-1301, USA.

*/

#include "globals.h"
#include "naming.h"
#include "dwconf.h"
#include "esb.h"

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
    Othewise you will get bogus results and confusion. */

/* ASCII control codes:
We leave newline as is, NUL is end of string,
the others are translated.
NUL Null             0  00              Ctrl-@ ^@
SOH Start of heading 1  01      Alt-1   Ctrl-A ^A
STX Start of text    2  02      Alt-2   Ctrl-B ^B
ETX End of text	     3  03      Alt-3   Ctrl-C ^C
EOT End of transmission	4 04    Alt-4   Ctrl-D ^D
ENQ Enquiry          5    05    Alt-5   Ctrl-E ^E
ACK Acknowledge	     6    06    Alt-6   Ctrl-F ^F
BEL Bell             7    07    Alt-7   Ctrl-G ^G
BS  Backspace        8    08    Alt-8   Ctrl-H ^H
HT  Horizontal tab   9    09    Alt-9   Ctrl-I ^I
LF  Line feed       10    0A    Alt-10  Ctrl-J ^J
VT  Vertical tab    11    0B    Alt-11  Ctrl-K ^K
FF  Form feed       12    0C    Alt-12  Ctrl-L ^L
CR  Carriage return 13    0D    Alt-13  Ctrl-M ^M
SO  Shift out       14    0E    Alt-14  Ctrl-N ^N
SI  Shift in        15    0F    Alt-15	Ctrl-O ^O
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
*/


static struct esb_s localesb;

boolean no_sanitize_string_garbage;

static  char tmpbuf[4];

static const char *
as_number(int c)
{
    snprintf(tmpbuf,sizeof(tmpbuf),"%%%02x",c & 0xff);
    return tmpbuf;
}

static void
do_sanity_insert( const char *s,struct esb_s *mesb)
{
    const char *cp = s;

    for( ; *cp; cp++) {
        int c = *cp & 0xff ;
        if (c >= 0x20 && c <=0x7e) {
            /* Usual case */
            esb_appendn(mesb,cp,1);
            continue;
        }
#ifdef _WIN32
        if (c == 0x0D) {
            esb_appendn(mesb,cp,1);
            continue;
        }
#endif /* _WIN32 */
        if (c == 0x0A || c == 0x09 ) {
            esb_appendn(mesb,cp,1);
            continue;
        }
        if (c < 0x20) {
            esb_append(mesb,as_number(c));
            /* esb_appendn(mesb,"?",1); */
            continue;
        }
        if (c == 0x7f) {
            esb_append(mesb,as_number(c));
            /* esb_appendn(mesb,"?",1); */
            continue;
        }
        esb_appendn(mesb,cp,1);
    }
}

const char *
sanitized(const char *s)
{

    if (no_sanitize_string_garbage) {
        return s;
    }

    /* destructor reconstructs the struct as empty! */
    esb_destructor(&localesb);
    do_sanity_insert(s,&localesb);
    return esb_get_string(&localesb);
}
