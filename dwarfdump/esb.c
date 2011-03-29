/*
  Copyright (C) 2005 Silicon Graphics, Inc.  All Rights Reserved.
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

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan


$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/esb.c,v 1.1 2005/08/04 05:09:37 davea Exp $ */

/* esb.c 
   extensible string buffer.

   A simple means (vaguely like a C++ class) that
   enables safely saving strings of arbitrary length built up
   in small pieces.

*/

#include "globals.h"
#include <stdarg.h>  /* For va_start etc. */
#include "esb.h"

#define INITIAL_ALLOC 1024
static size_t alloc_size = INITIAL_ALLOC;


static void
init_esb_string(struct esb_s *data, size_t min_len)
{
    string d;

    if (data->esb_allocated_size > 0) {
        return;
    }
    if (min_len < alloc_size) {
        min_len = alloc_size;
    }
    d = malloc(min_len);
    if (!d) {
        fprintf(stderr,
            "dwarfdump is out of memory allocating %lu bytes\n",
            (unsigned long) min_len);
        exit(5);
    }
    data->esb_string = d;
    data->esb_allocated_size = min_len;
    data->esb_string[0] = 0;
    data->esb_used_bytes = 0;
}

/* Make more room. Leaving  contents unchanged, effectively.
*/
static void
allocate_more(struct esb_s *data, size_t len)
{
    size_t new_size = data->esb_allocated_size + len;
    string newd = 0;

    if (new_size < alloc_size)
        new_size = alloc_size;
    newd = realloc(data->esb_string, new_size);
    if (!newd) {
        fprintf(stderr, "dwarfdump is out of memory re-allocating "
            "%lu bytes\n", (unsigned long) new_size);
        exit(5);
    }
    data->esb_string = newd;
    data->esb_allocated_size = new_size;
}

static void
esb_appendn_internal(struct esb_s *data, const char * in_string, size_t len);

void
esb_appendn(struct esb_s *data, const char * in_string, size_t len)
{
    size_t full_len = strlen(in_string);

    if (full_len < len) {
        fprintf(stderr, "dwarfdump internal error, bad string length "
            " %lu  < %lu \n",
            (unsigned long) full_len, (unsigned long) len);
        len = full_len;
    }

    esb_appendn_internal(data, in_string, len);
}

/*  The length is gotten from the in_string itself. */
void
esb_append(struct esb_s *data, const char * in_string)
{
    size_t len = strlen(in_string);

    esb_appendn_internal(data, in_string, len);
}

/*  The 'len' is believed. Do not pass in strings < len bytes long. */
static void
esb_appendn_internal(struct esb_s *data, const char * in_string, size_t len)
{
    size_t remaining = 0;
    size_t needed = len + 1;

    if (data->esb_allocated_size == 0) {
        size_t maxlen = (len > alloc_size) ? len : alloc_size;

        init_esb_string(data, maxlen);
    }
    remaining = data->esb_allocated_size - data->esb_used_bytes;
    if (remaining < needed) {
        allocate_more(data, needed);
    }
    strncpy(&data->esb_string[data->esb_used_bytes], in_string, len);
    data->esb_used_bytes += len;
    /* Insist on explicit NUL terminator */
    data->esb_string[data->esb_used_bytes] = 0;
}

/*  Always returns an empty string or a non-empty string. Never 0. */
string
esb_get_string(struct esb_s *data)
{
    if (data->esb_allocated_size == 0) {
        init_esb_string(data, alloc_size);
    }
    return data->esb_string;
}


/*  Sets esb_used_bytes to zero. The string is not freed and
    esb_allocated_size is unchanged.  */
void
esb_empty_string(struct esb_s *data)
{
    if (data->esb_allocated_size == 0) {
        init_esb_string(data, alloc_size);
    }
    data->esb_used_bytes = 0;
    data->esb_string[0] = 0;

}


/*  Return esb_used_bytes. */
size_t
esb_string_len(struct esb_s *data)
{
    return data->esb_used_bytes;
}


/*  The following are for testing esb, not use by dwarfdump. */

/*  *data is presumed to contain garbage, not values, and
    is properly initialized. */
void
esb_constructor(struct esb_s *data)
{
    memset(data, 0, sizeof(*data));
}

/*  The string is freed, contents of *data set to zeroes. */
void
esb_destructor(struct esb_s *data)
{
    if (data->esb_string) {
        free(data->esb_string);
    }
    esb_constructor(data);
}


/*  To get all paths in the code tested, this sets the
    allocation/reallocation to the given value, which can be quite small
    but must not be zero. */
void
esb_alloc_size(size_t size)
{
    alloc_size = size;
}

size_t
esb_get_allocated_size(struct esb_s *data)
{
    return data->esb_allocated_size;
}

/*  Append a formatted string */
void
esb_append_printf(struct esb_s *data,const char *in_string, ...)
{
#if WIN32
    #define NULL_DEVICE_FILE "NUL"
#else
    #define NULL_DEVICE_FILE "/dev/null"
#endif /* WIN32 */

    static FILE *null_file = NULL;

    int needed_size = 0;
    int length = 0;
    va_list ap;
    va_start(ap,in_string);
    if (null_file == NULL) {
        null_file = fopen(NULL_DEVICE_FILE,"w");
    }
    length = vfprintf(null_file,in_string,ap);

    /* Check if we require allocate more space */
    needed_size = data->esb_used_bytes + length;
    if (needed_size > data->esb_allocated_size) {
        allocate_more(data,length);
    }
    vsprintf(&data->esb_string[data->esb_used_bytes],in_string,ap);
    data->esb_used_bytes += length;
    va_end(ap);
}
