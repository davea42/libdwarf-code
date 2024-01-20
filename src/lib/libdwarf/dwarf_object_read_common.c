/*
Copyright (c) 2018, David Anderson
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
#include <stddef.h> /* size_t */
#include <stdio.h>  /* SEEK_END SEEK_SET */

#ifdef _WIN32
#ifdef HAVE_STDAFX_H
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */
#include <io.h> /* off_t */
#elif defined HAVE_UNISTD_H
#include <unistd.h> /* off_t */
#endif /* _WIN32*/

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "dwarf_base_types.h"
#include "dwarf_opaque.h"
#include "dwarf_safe_strcpy.h"
#include "dwarf_object_read_common.h"

/*  Neither off_t nor ssize_t is in C90.
    However, both are in Posix:
    IEEE Std 1003.1-1990, aka
    ISO/IEC 9954-1:1990. 
    This gets asked to read large sections sometimes. 
    The Linux kernel allows at most 0x7ffff000
    bytes in a read()*/
int
_dwarf_object_read_random(int fd, char *buf, off_t loc,
    size_t size, off_t filesize, int *errc)
{
    off_t scode = 0;
    ssize_t rcode = 0;
    off_t endpoint = 0;
    size_t max_single_read = 0x1ffff000;
    size_t remaining_bytes = 0;

    if (loc >= filesize) {
        /*  Seek can seek off the end. Lets not allow that.
            The object is corrupt. */
        *errc = DW_DLE_SEEK_OFF_END;
        return DW_DLV_ERROR;
    }
    endpoint = loc+size;
    if (endpoint > filesize) {
        /*  Let us -not- try to read past end of object.
            The object is corrupt. */
        *errc = DW_DLE_READ_OFF_END;
        return DW_DLV_ERROR;
    }
#ifdef _WIN32
    scode = (off_t)lseek(fd,(long)loc,SEEK_SET);
#else
    scode = lseek(fd,loc,SEEK_SET);
#endif
    if (scode == (off_t)-1) {
        *errc = DW_DLE_SEEK_ERROR;
        return DW_DLV_ERROR;
    }
    remaining_bytes = size;
    while(remaining_bytes > 0) { 
        if (remaining_bytes >= max_single_read) {
            size = max_single_read;
        }
#ifdef _WIN32
        rcode = (ssize_t)read(fd,buf,(unsigned const)size);
#else
        rcode = read(fd,buf,size);
#endif
        if (rcode == (ssize_t)-1 ||
            (size_t)rcode != size) {
            *errc = DW_DLE_READ_ERROR;
            return DW_DLV_ERROR;
        }
        remaining_bytes -= size;
        buf += size;
        size = remaining_bytes;
    }
    return DW_DLV_OK;
}
