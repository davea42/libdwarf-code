/*
   Copyright (C) 2020 David Anderson. All Rights Reserved.

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
   Public License along with this program; if not, write
   the Free Software Foundation, Inc., 51 Franklin Street -
   Fifth Floor, Boston MA 02110-1301, USA.
*/


#include "config.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for free() */
#endif /* HAVE_STDLIB_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h> /* open(), off_t, size_t, ssize_t */
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h> /* off_t ssize_t */
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */
#include <fcntl.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#elif defined(_WIN32) && defined(_MSC_VER)
/*  Want to have SEEK_CUR and SEEK_SET defined. */
#include <io.h>
#include <basetsd.h>
/*  Should we have include <windows.h> instead? */
typedef SSIZE_T ssize_t; /* MSVC does not have POSIX ssize_t */
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_MALLOC_H
/* Useful include for some Windows compilers. */
#include <malloc.h>
#endif /* HAVE_MALLOC_H */

#include "dwarf_incl.h"
#include "dwarf_error.h"

unsigned int
dwarf_basic_crc32 (const unsigned char *buf,
    unsigned long len,
    unsigned int init)
{
    return _dwarf_crc32(init,buf,len);
}

/*  Returns DW_DLV_OK DW_DLV_NO_ENTRY or DW_DLV_ERROR
    crc32 used for debuglink crc calculation.
    Caller passes pointer to array of 4 unsighed char
    and if this returns DW_DLV_OK that is filled in.*/
int
dwarf_crc32 (Dwarf_Debug dbg,unsigned char *crcbuf,
    Dwarf_Error *error)
{
    off_t size_left = 0;
    off_t fsize = 0;
    off_t lsval = 0;
    ssize_t readlen = 1000;
    unsigned char *readbuf = 0;
    ssize_t readval = 0;
    unsigned int tcrc = 0;
    unsigned int init = 0;
    int fd = -1;

    if (!dbg) {
        _dwarf_error_string(dbg,error,DW_DLE_DBG_NULL,
            "DW_DLE_DBG_NULL: Bad call to dwarf_crc32");
        return DW_DLV_ERROR;
    }
    if (!crcbuf) {
        return DW_DLV_NO_ENTRY;
    }
    if (!dbg->de_owns_fd) {
        return DW_DLV_NO_ENTRY;
    }
    fd = dbg->de_fd;
    if (fd < 0) {
        return DW_DLV_NO_ENTRY;
    }
    fd = dbg->de_fd;
    if (dbg->de_filesize) {
        fsize = size_left = dbg->de_filesize;
    } else {
        fsize = size_left = lseek(fd,0L,SEEK_END);
        if (fsize   == (off_t)-1) {
            _dwarf_error_string(dbg,error,DW_DLE_SEEK_ERROR,
                "DW_DLE_SEEK_ERROR: dwarf_crc32 seek "
                "to end fails");
            return DW_DLV_ERROR;
        }
    }
    if (fsize <= (off_t)500) {
        /*  Not a real object file.
            A random length check. */
        return DW_DLV_NO_ENTRY;
    }
    lsval  = lseek(fd,0L,SEEK_SET);
    if (lsval < 0) {
        _dwarf_error_string(dbg,error,DW_DLE_SEEK_ERROR,
            "DW_DLE_SEEK_ERROR: dwarf_crc32 seek "
            "to start fails");
        return DW_DLV_ERROR;
    }
    readbuf = (unsigned char *)malloc(readlen);
    if (!readbuf) {
        _dwarf_error_string(dbg,error,DW_DLE_ALLOC_FAIL,
            "DW_DLE_ALLOC_FAIL: dwarf_crc32 read buffer"
            " alloc fails");
        return DW_DLV_ERROR;
    }
    while (size_left > 0) {
        if (size_left < readlen) {
            readlen = size_left;
        }
        readval = read(fd,readbuf,readlen);
        if (readval != (ssize_t)readlen) {
            _dwarf_error_string(dbg,error,DW_DLE_READ_ERROR,
                "DW_DLE_READ_ERROR: dwarf_crc32 read fails ");
            free(readbuf);
            return DW_DLV_ERROR;
        }
        tcrc = _dwarf_crc32(init,readbuf,readlen);
        init = tcrc;
        size_left -= readlen;
    }
    /*  endianness issues?  */
    free(readbuf);
    memcpy(crcbuf,(void *)&tcrc,4);
    return DW_DLV_OK;
}
