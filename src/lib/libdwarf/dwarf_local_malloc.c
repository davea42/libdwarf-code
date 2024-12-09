/*
  Copyright (C) 2024 David Anderson. All Rights Reserved.

  This program is free software; you can redistribute it
  and/or modify it under the terms of version 2.1 of the
  GNU Lesser General Public License as published by the Free
  Software Foundation.

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

  You should have received a copy of the GNU Lesser General
  Public License along with this program; if not, write the
  Free Software Foundation, Inc., 51 Franklin Street - Fifth
  Floor, Boston MA 02110-1301, USA.

*/

#include <config.h>

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#ifdef LIBDWARF_MALLOC
#define LOCAL_UTIL_MALLOC 1
#else
/* so LIBDWARF_MALLOC works and normal works*/
#define xfree free
#endif /* LIBDWARF_MALLOC */

#ifdef LOCAL_UTIL_MALLOC
static unsigned long total_alloc;
static unsigned long alloc_count;
static unsigned long largest_alloc;
static unsigned long free_count;
#define xmalloc    malloc
#define xrealloc   realloc
#define xcalloc    calloc
#define xfree      free
#endif /* LOCAL_UTIL_MALLOC */

#ifdef LOCAL_UTIL_MALLOC
void * _libdwarf_malloc(size_t s)
{
    ++alloc_count;
    if ((unsigned long)s > largest_alloc) {
        largest_alloc = s;
        printf("dadebug line %d largest_alloc %lx\n",
            __LINE__,
            largest_alloc);
        fflush(stdout);
    }
    return xmalloc(s);
}
void * _libdwarf_calloc(size_t n, size_t s)
{
    ++alloc_count;
    if (n  > largest_alloc ) {
        largest_alloc = n;
        printf("dadebug line %d largest_alloc %lx\n",
            __LINE__,
            largest_alloc);
        fflush(stdout);
    }
    if (s  > largest_alloc ) {
        largest_alloc = s;
        printf("dadebug line %d largest_alloc %lx\n",
            __LINE__,
            largest_alloc);
        fflush(stdout);
    }
    if ((n * s) > largest_alloc) {
        largest_alloc = s;
        printf("dadebug line %d largest_alloc %lx\n",
            __LINE__,
            largest_alloc);
        fflush(stdout);
    }
    return xcalloc(n,s);
}
void * _libdwarf_realloc(void * p, size_t s)
{
    ++alloc_count;
    if ((s)  > largest_alloc) {
        largest_alloc = s;
        printf("dadebug line %d largest_alloc %lx\n",
            __LINE__,
            largest_alloc);
        fflush(stdout);
    }
    return xrealloc(p,s);
}
void _libdwarf_free(void *p)
{
    free_count++;
    xfree(p);
}
void _libdwarf_finish(void)
{
     printf("dadebug at finish   total alloc %lu\n",total_alloc);
     printf("dadebug at finish largext alloc %lu\n",largest_alloc);
     printf("dadebug at finish alloc count   %lu\n",alloc_count);
     printf("dadebug at finish    free count %lu\n",free_count);
     fflush(stdout);
}
#endif /* LOCAL_UTIL_MALLOC */

