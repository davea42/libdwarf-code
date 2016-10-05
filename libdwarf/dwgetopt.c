/* $NetBSD: getopt.c,v 1.1 2009/03/22 22:33:13 joerg Exp $*/
/*  Modified by David Anderson to work with GNU/Linux and freebsd.
    Added {} for clarity.
    Switched to standard dwarfdump formatting.
    Treatment of : modified so that :: gets dwoptarg NULL
    if space follows the letter
    (the dwoptarg is set to null).
    renamed to make it clear this is a private version.
*/
/*
* Copyright (c) 1987, 1993, 1994
* The Regents of the University of California.  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. Neither the name of the University nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/

/*  This does not presently handle the option string
    leading + or leading - features. Such are not used
    by by libdwarfdump.  Nor does it understand the
    GNU Env var POSIXLY_CORRECT .
    It does know of the leading ":" in the option string.
    See BADCH below.
    */

#include <stdio.h>
#include <stdlib.h> /* For exit() */
#include <string.h> /* For strchr */
#include "dwgetopt.h"

#define STRIP_OFF_CONSTNESS(a)  ((void *)(size_t)(const void *)(a))

int dwopterr = 1,     /* if error message should be printed */
    dwoptind = 1,    /* index into parent argv vector */
    dwoptopt,        /* character checked for validity */
    dwoptreset;      /* reset getopt */
char *dwoptarg;      /* argument associated with option */

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""

#if 0 /* FOR DEBUGGING ONLY */
/*  Use for testing dwgetopt only.
    Not a standard function. */
void
dwgetoptresetfortestingonly(void)
{
   dwopterr   = 1;
   dwoptind   = 1;
   dwoptopt   = 0;
   dwoptreset = 0;
   dwoptarg   = 0;
}
#endif /* FOR DEBUGGING ONLY */

/*
    * getopt --
    * Parse argc/argv argument vector.
    * a: means
    *     -afoo
    *     -a foo
    *     and 'foo' is returned in dwoptarg
    *  b:: means
    *     -b
    *        and dwoptarg is null
    *     -bother
    *        and dwoptarg is 'other'
    */
int
dwgetopt(int nargc, char * const nargv[], const char *ostr)
{
    static const char *place = EMSG;/* option letter processing */
    char *oli;                      /* option letter list index */

    if (dwoptreset || *place == 0) { /* update scanning pointer */
        dwoptreset = 0;
        place = nargv[dwoptind];
        if (dwoptind >= nargc || *place++ != '-') {
            /* Argument is absent or is not an option */
            place = EMSG;
            return (-1);
        }
        dwoptopt = *place++;
        if (dwoptopt == '-' && *place == 0) {
            /* "--" => end of options */
            ++dwoptind;
            place = EMSG;
            return (-1);
        }
        if (dwoptopt == 0) {
            /* Solitary '-', treat as a '-' option
                if the program (eg su) is looking for it. */
            place = EMSG;
            if (strchr(ostr, '-') == NULL) {
                return -1;
            }
            dwoptopt = '-';
        }
    } else {
        dwoptopt = *place++;
    }
    /* See if option letter is one the caller wanted... */
    if (dwoptopt == ':' || (oli = strchr(ostr, dwoptopt)) == NULL) {
        if (*place == 0) {
            ++dwoptind;
        }
        if (dwopterr && *ostr != ':') {
            (void)fprintf(stderr,
                "%s: invalid option -- '%c'\n",
                nargv[0]?nargv[0]:"",
                dwoptopt);
        }
        return (BADCH);
    }

    /* Does this option need an argument? */
    if (oli[1] != ':') {
        /* don't need argument */
        dwoptarg = NULL;
        if (*place == 0) {
            ++dwoptind;
        }
    } else {
        int reqnextarg = 1;
        if (oli[1] && (oli[2] == ':')) {
            /* Pair of :: means special treatment of dwoptarg */
            reqnextarg = 0;
        }
        /* Option-argument is either the rest of this argument or the
        entire next argument. */
        if (*place ) {
            /* Whether : or :: */
            dwoptarg = STRIP_OFF_CONSTNESS(place);
        } else if (reqnextarg) {
            /* ! *place */
            if (nargc > (++dwoptind)) {
                dwoptarg = nargv[dwoptind];
            } else {
                place=EMSG;
                /*  Next arg required, but is missing */
                if (*ostr == ':') {
                    /* Leading : in ostr calls for BADARG return. */
                    return (BADARG);
                }
                if (dwopterr) {
                    (void)fprintf(stderr,
                        "%s: option requires an argument. -- '%c'\n",
                        nargv[0]?nargv[0]:"",
                        dwoptopt);
                }
                return (BADCH);
            }
        } else {
            /* ! *place */
            /* The key part of :: treatment. */
            dwoptarg = NULL;
        }
        place = EMSG;
        ++dwoptind;
    }
    return (dwoptopt);  /* return option letter */
}
