/*
Copyright (c) 2019-2019, David Anderson
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

/*  A lighly generalized data buffer.
    Works for more than just strings,
    but has features (such as ensuring
    data always has a NUL byte following
    the data area used) most useful for C strings.
*/


#include <stdio.h> /* for malloc */
#include <stdlib.h> /* for malloc */
#include <string.h> /* for strlen */
#ifdef HAVE_MALLOC_H
/* Useful include for some Windows compilers. */
#include <malloc.h>
#endif /* HAVE_MALLOC_H */
#include "dwarfstring.h"
#ifndef TRUE
#define TRUE 1
#endif /* TRUE */
#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

static unsigned long minimumnewlen = 30;
/*
struct dwarfstring_s {
   char *        s_data;
   unsigned long s_size;
   unsigned long s_avail;
   unsigned char s_malloc;
};
*/

int
dwarfstring_constructor(struct dwarfstring_s *g)
{
    g->s_data = "";
    g->s_size = 0;
    g->s_avail = 0;
    g->s_malloc = FALSE;
    return TRUE;
}

static int
dwarfstring_resize_to(struct dwarfstring_s *g,unsigned long newlen)
{
    char *b = 0;
    unsigned long lastpos =
        g->s_size - g->s_avail;
    unsigned long malloclen = newlen+1;

    if(malloclen < minimumnewlen) {
        malloclen = minimumnewlen;
    }
    b = malloc(malloclen);
    if (!b) {
        return FALSE;
    }
    if (lastpos > 0) {
        memcpy(b,g->s_data,lastpos);
    }
    if (g->s_malloc) {
        free(g->s_data);
        g->s_data = 0;
    }
    g->s_data = b;
    g->s_data[lastpos] = 0;
    g->s_size = newlen;
    g->s_avail = newlen - lastpos;
    g->s_malloc = TRUE;
    return TRUE;
}

int
dwarfstring_reset(struct dwarfstring_s *g)
{
    if (!g->s_size) {
        /* In initial condition, nothing to do. */
        return TRUE;
    }
    g->s_avail = g->s_size;
    g->s_data[0] = 0;
    return TRUE;
}

int
dwarfstring_constructor_fixed(struct dwarfstring_s *g,unsigned long len)
{
    int r = FALSE;

    dwarfstring_constructor(g);
    if (len == 0) {
        return TRUE;
    }
    r = dwarfstring_resize_to(g,len);
    if (!r) {
        return FALSE;
    }
    return TRUE;
}

int
dwarfstring_constructor_static(struct dwarfstring_s *g,
    char * space,
    unsigned long len)
{
    dwarfstring_constructor(g);
    g->s_data = space;
    g->s_data[0] = 0;
    g->s_size = len;
    g->s_avail = len;
    g->s_malloc = FALSE;
    return TRUE;
}

void
dwarfstring_destructor(struct dwarfstring_s *g)
{
    if (g->s_malloc) {
        free(g->s_data);
        g->s_data = 0;
    }
    dwarfstring_constructor(g);
}

/*  For the case where one wants just the first 'len'
    characters of 'str'. NUL terminator provided
    for you in s_data.
*/
int
dwarfstring_append_length(struct dwarfstring_s *g,char *str,
    unsigned long slen)
{
    unsigned long lastpos = g->s_size - g->s_avail;
    int r = 0;

    if (!str  || slen ==0) {
        return TRUE;
    }
    if (slen >= g->s_avail) {
        unsigned long newlen = 0;

        newlen = g->s_size + slen+2;
        r = dwarfstring_resize_to(g,newlen);
        if (!r) {
            return FALSE;
        }
    }
    memcpy(g->s_data + lastpos,str,slen);
    g->s_avail -= slen;
    g->s_data[g->s_size - g->s_avail] = 0;
    return TRUE;
}

int
dwarfstring_append(struct dwarfstring_s *g,char *str)
{
    unsigned long dlen = 0;

    if(!str) {
        return TRUE;
    }
    dlen = strlen(str);
    return dwarfstring_append_length(g,str,dlen);
}

char *
dwarfstring_string(struct dwarfstring_s *g)
{
    return g->s_data;
}

unsigned long
dwarfstring_strlen(struct dwarfstring_s *g)
{
    return g->s_size - g->s_avail;
}
