
/* Copyright (c) 2023, David Anderson
All rights reserved.

Redistribution and use in source and binary forms, with
or without modification, are permitted provided that the
following conditions are met:

    Redistributions of source code must retain the above
    copyright notice, this list of conditions and the following
    disclaimer.

    Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <config.h>

#include <stdlib.h> /* calloc() free() */
#include <string.h> /* memset() strcmp() strncmp() strlen() */
#include <stdio.h> /* debugging */

#if defined(_WIN32) && defined(HAVE_STDAFX_H)
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */

#if 0
#include "dwarf.h"
#include "libdwarf.h"
#endif
#include "libdwarf_private.h" /* for TRUE FALSE */
#include "dwarf_secname_ck.h"

/*  Mainly important for Elf */

int
_dwarf_startswith(const char * input,const char* ckfor)
{
    size_t cklen = strlen(ckfor);

    if (! strncmp(input,ckfor,cklen)) {
        return TRUE;
    }
    return FALSE;
}

static const char *nonsec[] = { 
".bss",
".comment",
".data",
".fini",
".fini_array",
".got",
".init",
".interp",
".jcr",
".plt",
".rela.data",
".rela.got",
".rela.plt",
".rela.text",
".rel.data",
".rel.got",
".rel.plt",
".rel.text",
".sbss",
".text",
0
};

/*  These help us ignore some sections that are
    irrelevant to libdwarf.  Maybe should use a hash
    table or binary search instead of sequential search? */
int
_dwarf_ignorethissection(const char *scn_name)
{
#if 0
    int tablen = sizeof(nonsec)/sizeof(const char *);
    const char *cp = 0;
#endif
    int i = 0;

    for ( ; nonsec[i]; ++i) {
        const char *s = nonsec[i];
        if (_dwarf_startswith(scn_name,s)) {
            return TRUE;
        }
    }
    return FALSE;
}
