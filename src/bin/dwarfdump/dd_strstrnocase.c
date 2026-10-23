/*
Copyright (C) 2009-2016 David Anderson.  All Rights Reserved.
Portions Copyright 2012 SN Systems Ltd. All rights reserved.

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

/*
This tries to find the string 'contained' in the
string 'container'.  it returns true if found, else false.
The comparisons are independent  of case.

Regrettably there is no generally accepted version that
does this job, though GNU Linux has  strcasestr() which
does what we need.   Our code here do not behave like
strstr or strcasestr in the case of
an empty 'contained' argument: we return FALSE (this
case is not interesting for dwarfdump).

There is a public domain stristr().
But given that dwarfdump is GPL,
it would seem (IANAL) that we cannot mix public domain code
into the release.

The software here is independently written and indeed trivial.

The POSIX function tolower() is only properly
defined on unsigned char, hence
the ugly casts.

strstrnocase.c

*/

#include <config.h>

#include <ctype.h> /* tolower() */

#include "dwarf.h"
#include "libdwarf.h"
#include "dd_defined_types.h"
#include "dd_checkutil.h"
#include "dd_glflags.h"
#include "dd_globals.h"

/*  Supposed to return TRUE if the string contained
    appears anywhere in container.
    This now has the fix for github issue #335 . */
Dwarf_Bool
is_strstrnocase(const char * container, const char * contained)
{
    const unsigned char *ct = (const unsigned char *)container;
    for (; *ct; ++ct )
    {
        const unsigned char * cntnd =
            (const unsigned char *)contained;
        const unsigned char * ctp =
            (const unsigned char *)ct;

        /* Do not update ct in this loop. */
        for (; *cntnd && *ct ; ++cntnd,++ctp)
        {
            unsigned char lct = tolower(*ctp);
            unsigned char tlc = tolower(*cntnd);
            if (lct != tlc) {
                break;
            }
        }
        if (!*cntnd) {
            /* We matched all the way to end of contained  */
            /* ASSERT: innerwrong = FALSE  */
            return TRUE;
        }
        if (!*ctp) {
            /*  Ran out of container before contained,
                so no future match of contained
                is possible. */
            return FALSE;

        }
        /*  No match so far.
            See if there is more in container to check. */
    }
    return FALSE;
}
