/*
  Copyright (C) 2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2013-2018 David Anderson. All Rights Reserved.
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

/*  esb.c
    extensible string buffer.

    A simple means (vaguely like a C++ class) that
    enables safely saving strings of arbitrary length built up
    in small pieces.

    We really do allow only C strings here. NUL bytes
    in a string result in adding only up to the NUL (and
    in the case of certain interfaces here a warning
    to stderr).

    Do selftest as follows:
        gcc -DSELFTEST esb.c
        ./a.out
        valgrind --leak-check=full ./a.out

    The functions assume that
    pointer arguments of all kinds are not NULL.
*/

#ifdef SELFTEST
typedef char * string; /* SELFTEST */
#endif
#include "esb.h"
#define TRUE 1

/*  INITIAL_ALLOC value takes no account of space for a trailing NUL,
    the NUL is accounted for in init_esb_string
    and in later tests against esb_allocated_size. */
#undef MALLOC_COUNT
#ifdef SELFTEST
#define INITIAL_ALLOC 1  /* SELFTEST */
#define MALLOC_COUNT 1
#else
/*  There is nothing magic about this size.
    It is just big enough to avoid most resizing. */
#define INITIAL_ALLOC 100
#endif
/*  Allow for final NUL */
static size_t alloc_size = INITIAL_ALLOC;

/* NULL device used when printing formatted strings */
static FILE *null_device_handle = 0;
#if _WIN32
#define NULL_DEVICE_NAME "NUL"
#else
#define NULL_DEVICE_NAME "/dev/null"
#endif /* _WIN32 */


#ifdef MALLOC_COUNT
long malloc_count = 0;
long malloc_size = 0;
#endif

/* Open the null device used during formatting printing */
FILE *esb_open_null_device(void)
{
    if (!null_device_handle) {
        null_device_handle = fopen(NULL_DEVICE_NAME,"w");
    }
    return null_device_handle;
}

/* Close the null device used during formatting printing */
void esb_close_null_device(void)
{
    if (null_device_handle) {
        fclose(null_device_handle);
        null_device_handle = 0;
    }
}

/*  min_len is overall space wanted for initial alloc.
    ASSERT: esb_allocated_size == 0 */
static void
init_esb_string(struct esb_s *data, size_t min_len)
{
    char* d;

    if (data->esb_allocated_size > 0) {
        return;
    }
    /*  Only esb_constructor applied so far.
        Now Allow for string space. */
    if (min_len <= alloc_size) {
        min_len = alloc_size;
    } else  {
        min_len++ ; /* Allow for NUL at end */
    }
    d = malloc(min_len);
#ifdef MALLOC_COUNT
    ++malloc_count;
    malloc_size += min_len;
#endif
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

/*  Make more room. Leaving  contents unchanged, effectively.
    The NUL byte at end has room and this preserves that room.
*/
static void
esb_allocate_more(struct esb_s *data, size_t len)
{
    size_t new_size = 0;
    char* newd = 0;

    if (data->esb_rigid) {
        return;
    }
    if (data->esb_allocated_size == 0) {
        init_esb_string(data, alloc_size);
    }
    new_size = data->esb_allocated_size + len;
    if (new_size < alloc_size) {
        new_size = alloc_size;
    }
    if (data->esb_fixed) {
        size_t copylen = data->esb_used_bytes;
        if ((new_size-1) < copylen) {
            copylen = new_size -1;
        }
        newd = malloc(new_size);
#ifdef MALLOC_COUNT
        ++malloc_count;
        malloc_size += len;
#endif
        if (newd) {
            strncpy(newd,data->esb_string,copylen);
            newd[copylen] = 0;
        }
    } else {
        newd = realloc(data->esb_string, new_size);
#ifdef MALLOC_COUNT
        ++malloc_count;
        malloc_size += len;
#endif
    }
    if (!newd) {
        fprintf(stderr, "dwarfdump is out of memory allocating "
            "%lu bytes\n", (unsigned long) new_size);
        exit(5);
    }
    /*  If the area was reallocated by realloc() the earlier
        space was free()d by realloc(). */
    data->esb_string = newd;
    data->esb_allocated_size = new_size;
    data->esb_fixed = 0;
}

/*  Ensure that the total buffer length is large enough that
    at least minlen bytes are available, unused,
    in the allocation. */
void
esb_force_allocation(struct esb_s *data, size_t minlen)
{
    size_t target_len = 0;

    if (data->esb_rigid) {
        return;
    }
    if (data->esb_allocated_size == 0) {
        init_esb_string(data, alloc_size);
    }
    target_len = data->esb_used_bytes + minlen;
    if (data->esb_allocated_size < target_len) {
        size_t needed = target_len - data->esb_allocated_size;
        esb_allocate_more(data,needed);
    }
}

/*  The 'len' is believed. Do not pass in strings < len bytes long.
    For strlen(in_string) > len bytes we take the initial len bytes.
    len does not include the trailing NUL. */
static void
esb_appendn_internal(struct esb_s *data, const char * in_string, size_t len)
{
    size_t remaining = 0;
    size_t needed = len;

    if (data->esb_allocated_size == 0) {
        size_t maxlen = (len > alloc_size)? (len):alloc_size;

        init_esb_string(data, maxlen);
    }
    /*  ASSERT: data->esb_allocated_size > data->esb_used_bytes  */
    remaining = data->esb_allocated_size - data->esb_used_bytes - 1;
    if (remaining <= needed) {
        if (data->esb_rigid && len > remaining) {
            len = remaining;
        } else {
            size_t alloc_amt = needed - remaining;
            esb_allocate_more(data,alloc_amt);
        }
    }
    if (len ==  0) {
        /* No room for anything more, or no more requested. */
        return;
    }
    strncpy(&data->esb_string[data->esb_used_bytes], in_string, len);
    data->esb_used_bytes += len;
    /* Insist on explicit NUL terminator */
    data->esb_string[data->esb_used_bytes] = 0;
}

/* len >= strlen(in_string) */
void
esb_appendn(struct esb_s *data, const char * in_string, size_t len)
{
    size_t full_len = strlen(in_string);

    if (full_len < len) {
        fprintf(stderr, "dwarfdump esb internal error, bad string length "
            " %lu  < %lu \n",
            (unsigned long) full_len, (unsigned long) len);
        len = full_len;
    }

    esb_appendn_internal(data, in_string, len);
}

/*  The length is gotten from the in_string itself, this
    is the usual way to add string data.. */
void
esb_append(struct esb_s *data, const char * in_string)
{
    size_t len = 0;
    if(in_string) {
        len = strlen(in_string);
        if (len) {
            esb_appendn_internal(data, in_string, len);
        }
    }
}


/*  Always returns an empty string or a non-empty string. Never 0. */


char*
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

/*  *data is presumed to contain garbage, not values, and
    is properly initialized here. */
void
esb_constructor(struct esb_s *data)
{
    memset(data, 0, sizeof(*data));
}

#if 0
void
esb_constructor_rigid(struct esb_s *data,char *buf,size_t buflen)
{
    memset(data, 0, sizeof(*data));
    data->esb_string = buf;
    data->esb_string[0] = 0;
    data->esb_allocated_size = buflen;
    data->esb_used_bytes = 0;
    data->esb_rigid = 1;
    data->esb_fixed = 1;
}
#endif

/*  ASSERT: buflen > 0 */
void
esb_constructor_fixed(struct esb_s *data,char *buf,size_t buflen)
{
    memset(data, 0, sizeof(*data));
    if  (buflen < 1) {
        return;
    }
    data->esb_string = buf;
    data->esb_string[0] = 0;
    data->esb_allocated_size = buflen;
    data->esb_used_bytes = 0;
    data->esb_rigid = 0;
    data->esb_fixed = 1;
}


/*  The string is freed, contents of *data set to zeroes. */
void
esb_destructor(struct esb_s *data)
{
    if(data->esb_fixed) {
        data->esb_allocated_size = 0;
        data->esb_used_bytes = 0;
        data->esb_string = 0;
        data->esb_rigid = 0;
        data->esb_fixed = 0;
        return;
    }
    if (data->esb_string) {
        free(data->esb_string);
        data->esb_string = 0;
    }
    esb_constructor(data);
}


/*  To get all paths in the code tested, this sets the
    initial allocation/reallocation file-static
    which can be quite small but must not be zero
    The alloc_size variable  is used for initializations. */
void
esb_alloc_size(size_t size)
{
    if (size < 1) {
        size = 1;
    }
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
    va_list ap;
    int len = 0;
    int len2 = 0;
    int remaining = 0;

    if (!null_device_handle) {
        if(!esb_open_null_device()) {
            esb_append(data," Unable to open null printf device on:");
            esb_append(data,in_string);
            return;
        }
    }
    va_start(ap,in_string);
    len = vfprintf(null_device_handle,in_string,ap);
    va_end(ap);

    if (data->esb_allocated_size == 0) {
        init_esb_string(data, alloc_size);
    }
    remaining = data->esb_allocated_size - data->esb_used_bytes -1;
    if (remaining < len) {
        if (data->esb_rigid) {
            /* No room, give up. */
            return;
        } else {
            esb_allocate_more(data, len);
        }
    }
    va_start(ap,in_string);
#ifdef HAVE_VSNPRINTF
    len2 = vsnprintf(&data->esb_string[data->esb_used_bytes],
        data->esb_allocated_size,
#else
    len2 = vsprintf(&data->esb_string[data->esb_used_bytes],
#endif
        in_string,ap);

    va_end(ap);
    data->esb_used_bytes += len2;
    if (len2 >  len) {
        /*  We are in big trouble, this should be impossible.
            We have trashed something in memory. */
        fprintf(stderr,
            "dwarfdump esb internal error, vsprintf botch "
            " %lu  < %lu \n",
            (unsigned long) len2, (unsigned long) len);
        exit(5);
    }
    return;
}

/*  Get a copy of the internal data buffer.
    It is up to the code calling this
    to free() the string using the
    pointer returned here. */
char*
esb_get_copy(struct esb_s *data)
{
    char* copy = NULL;

    /* is ok as is if esb_allocated_size is 0 */
    size_t len = esb_string_len(data);
    if (len) {
        copy = (char*)malloc(len + 1);
#ifdef MALLOC_COUNT
        ++malloc_count;
        malloc_size += len+1;
#endif
        strcpy(copy,esb_get_string(data));
    }
    return copy;
}


#ifdef SELFTEST
static int failcount = 0;
void
validate_esb(int instance,
   struct esb_s* d,
   size_t explen,
   size_t expalloc,
   const char *expout,
   int line )
{
    if (esb_string_len(d) != explen) {
        ++failcount;
        printf("  FAIL instance %d  esb_string_len() %u explen %u line %d\n",
            instance,(unsigned)esb_string_len(d),(unsigned)explen,line);
    }
    if (d->esb_allocated_size != expalloc) {
        ++failcount;
        printf("  FAIL instance %d  esb_allocated_size  %u expalloc %u line %d\n",
            instance,(unsigned)d->esb_allocated_size,(unsigned)expalloc,line);
    }
    if(strcmp(esb_get_string(d),expout)) {
        ++failcount;
        printf("  FAIL instance %d esb_get_stringr %s expstr %s line %d\n",
            instance,esb_get_string(d),expout,line);
    }
}



int main()
{
#ifdef _WIN32
    /* Open the null device used during formatting printing */
    if (!esb_open_null_device())
    {
        fprintf(stderr, "esb: Unable to open null device.\n");
        exit(1);
    }
#endif /* _WIN32 */

   {
        struct esb_s d;
        esb_constructor(&d);
        esb_append(&d,"a");
        validate_esb(1,&d,1,2,"a",__LINE__);
        esb_append(&d,"b");
        validate_esb(2,&d,2,3,"ab",__LINE__);
        esb_append(&d,"c");
        validate_esb(3,&d,3,4,"abc",__LINE__);
        esb_empty_string(&d);
        validate_esb(4,&d,0,4,"",__LINE__);
        esb_destructor(&d);
    }
    {
        struct esb_s d;

        esb_constructor(&d);
        esb_append(&d,"aa");
        validate_esb(6,&d,2,3,"aa",__LINE__);
        esb_append(&d,"bbb");
        validate_esb(7,&d,5,6,"aabbb",__LINE__);
        esb_append(&d,"c");
        validate_esb(8,&d,6,7,"aabbbc",__LINE__);
        esb_empty_string(&d);
        validate_esb(9,&d,0,7,"",__LINE__);
        esb_destructor(&d);
    }
    {
        struct esb_s d;
        static char oddarray[7] = {'a','b',0,'c','c','d',0};

        esb_constructor(&d);
        fprintf(stderr,"  esb_appendn call error(intentional). Expect msg on stderr\n");
        /* This provokes a msg on stderr. Bad input. */
        esb_appendn(&d,oddarray,6);
        validate_esb(10,&d,2,3,"ab",__LINE__);
        esb_appendn(&d,"cc",1);
        validate_esb(11,&d,3,4,"abc",__LINE__);
        esb_empty_string(&d);
        validate_esb(12,&d,0,4,"",__LINE__);
        esb_destructor(&d);
    }
    {
        struct esb_s d;

        esb_constructor(&d);
        esb_force_allocation(&d,7);
        esb_append(&d,"aaaa i");
        validate_esb(13,&d,6,7,"aaaa i",__LINE__);
        esb_destructor(&d);
    }
    {
        struct esb_s d5;
        const char * s = "insert me %d";

        esb_constructor(&d5);
        esb_force_allocation(&d5,50);
        esb_append(&d5,"aaa ");
        esb_append_printf(&d5,s,1);
        esb_append(&d5,"zzz");
        validate_esb(14,&d5,18,50,"aaa insert me 1zzz",__LINE__);
        esb_destructor(&d5);
    }
    {
        struct esb_s d;
        struct esb_s e;
        esb_constructor(&d);
        esb_constructor(&e);

        char* result = NULL;
        esb_append(&d,"abcde fghij klmno pqrst");
        validate_esb(15,&d,23,24,"abcde fghij klmno pqrst",__LINE__);

        result = esb_get_copy(&d);
        esb_append(&e,result);
        validate_esb(16,&e,23,24,"abcde fghij klmno pqrst",__LINE__);
        esb_destructor(&d);
        esb_destructor(&e);
    }
    {
        struct esb_s d5;
        char bufs[4];
        char bufl[60];
        const char * s = "insert me %d";

        esb_constructor_fixed(&d5,bufs,sizeof(bufs));
        esb_append(&d5,"aaa ");
        esb_append_printf(&d5,s,1);
        esb_append(&d5,"zzz");
        validate_esb(17,&d5,18,19,"aaa insert me 1zzz",__LINE__);
        esb_destructor(&d5);

        esb_constructor_fixed(&d5,bufl,sizeof(bufl));
        esb_append(&d5,"aaa ");
        esb_append_printf(&d5,s,1);
        esb_append(&d5,"zzz");
        validate_esb(17,&d5,18,60,"aaa insert me 1zzz",__LINE__);
        esb_destructor(&d5);

    }
#ifdef _WIN32
    /* Close the null device used during formatting printing */
    esb_close_null_device();
#endif /* _WIN32 */
    if (failcount) {
        printf("FAIL esb test\n");
        exit(1);
    }
    printf("PASS esb test\n");
    exit(0);
}
#endif /* SELFTEST */
