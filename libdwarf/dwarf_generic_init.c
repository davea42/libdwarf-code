/*
  Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2008-2010 Arxan Technologies, Inc. All rights reserved.
  Portions Copyright 2011-2015 David Anderson. All rights reserved.
  Portions Copyright 2012 SN Systems Ltd. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, write the Free Software
  Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston MA 02110-1301,
  USA.

*/

#include "config.h"
#ifdef HAVE_LIBELF_H
#include <libelf.h>
#else
#ifdef HAVE_LIBELF_LIBELF_H
#include <libelf/libelf.h>
#endif
#endif
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include "dwarf_incl.h"
#include "dwarf_error.h"
#include "dwarf_object_detector.h"
#include "dwarf_elf_access.h" /* Needed while libelf in use */

/*  This is the initialization set intended to
    handle multiple object formats.
    Created September 2018  */


#define DWARF_DBG_ERROR(dbg,errval,retval) \
    _dwarf_error(dbg, error, errval); return(retval);


#define FALSE  0
#define TRUE   1
/*  The basic dwarf initializer functions for consumers.
    Return a libdwarf error code on error, return DW_DLV_OK
    if this succeeds.  */
int
dwarf_init(int fd,
    Dwarf_Unsigned access,
    Dwarf_Handler errhand,
    Dwarf_Ptr errarg, Dwarf_Debug * ret_dbg, Dwarf_Error * error)
{
    return dwarf_init_b(fd,access, DW_GROUPNUMBER_ANY,
        errhand,errarg,ret_dbg,error);
}


int dwarf_init_path(const char *path,
    char *true_path_out_buffer,
    unsigned true_path_bufferlen,
    Dwarf_Unsigned    access,
    unsigned          groupnumber,
    Dwarf_Handler     errhand,
    Dwarf_Ptr         errarg,
    Dwarf_Debug*      ret_dbg,
    UNUSEDARG const char *       reserved1,
    UNUSEDARG Dwarf_Unsigned     reserved2,
    UNUSEDARG Dwarf_Unsigned  *  reserved3,
    Dwarf_Error*      error)
{
    unsigned       ftype = 0;
    unsigned       endian = 0;
    unsigned       offsetsize = 0;
    Dwarf_Unsigned filesize = 0;
    int res = 0;
    int errcode = 0;
    int fd = -1;
    int lib_owns_fd = TRUE;

    res = dwarf_object_detector_path(path,
        true_path_out_buffer,
        true_path_bufferlen,
        &ftype,&endian,&offsetsize,&filesize,&errcode);
    if (res == DW_DLV_NO_ENTRY) {
        return res;
    }
    if (res == DW_DLV_ERROR) {
        DWARF_DBG_ERROR(NULL, DW_DLE_FILE_UNAVAILABLE, DW_DLV_ERROR);
    }
    switch(ftype) {
    case DW_FTYPE_ELF: {
        if (true_path_out_buffer) {
            fd = open(true_path_out_buffer,O_RDONLY);
        } else {
            fd = open(path,O_RDONLY);
        }
        if(fd < 0) {
            DWARF_DBG_ERROR(NULL, DW_DLE_FILE_UNAVAILABLE,
                DW_DLV_ERROR);
        }
        res = _dwarf_elf_setup(fd,
            true_path_out_buffer?true_path_out_buffer:"",
            ftype,endian,offsetsize,filesize,
            access,groupnumber,errhand,errarg,ret_dbg,error);
        return res;
    }
    case DW_FTYPE_MACH_O: {
        res = _dwarf_macho_setup(fd,
            true_path_out_buffer?true_path_out_buffer:"",
            lib_owns_fd,
            ftype,endian,offsetsize,filesize,
            access,groupnumber,errhand,errarg,ret_dbg,error);
        return res;
    }
    case DW_FTYPE_PE:
    default:
        DWARF_DBG_ERROR(NULL, DW_DLE_FILE_WRONG_TYPE, DW_DLV_ERROR);
    }
    return DW_DLV_NO_ENTRY; /* placeholder for now */
}


/* To be revised to not use libelf. */
/* New March 2017 */
int
dwarf_init_b(int fd,
    Dwarf_Unsigned access,
    unsigned  group_number,
    Dwarf_Handler errhand,
    Dwarf_Ptr errarg,
    Dwarf_Debug * ret_dbg,
    Dwarf_Error * error)
{
    unsigned ftype = 0;
    unsigned endian = 0;
    unsigned offsetsize = 0;
    Dwarf_Unsigned   filesize = 0;
    int res = 0;
    int errcode = 0;
    int lib_owns_fd = FALSE;

    res = dwarf_object_detector_fd(fd, &ftype,
        &endian,&offsetsize,&filesize,&errcode);
    if (res == DW_DLV_NO_ENTRY) {
        return res;
    } else if (res == DW_DLV_ERROR) {
        DWARF_DBG_ERROR(NULL, DW_DLE_FILE_WRONG_TYPE, DW_DLV_ERROR);
    }
    switch(ftype) {
    case DW_FTYPE_ELF: {
        res = _dwarf_elf_setup(fd,
            "",
            ftype,endian,offsetsize,filesize,
            access,group_number,errhand,errarg,ret_dbg,error);
        return res;
        }
    case DW_FTYPE_MACH_O: {
        res = _dwarf_macho_setup(fd,"",
            lib_owns_fd,
            ftype,endian,offsetsize,filesize,
            access,group_number,errhand,errarg,ret_dbg,error);
        return res;
        }

    case DW_FTYPE_PE: {
        /* FIXME: temporary */
        break;
        }
    }
    DWARF_DBG_ERROR(NULL, DW_DLE_FILE_WRONG_TYPE, DW_DLV_ERROR);
    return res;
}

/*
    Frees all memory that was not previously freed
    by dwarf_dealloc.
    Aside from certain categories.

    Applicable when dwarf_init() or dwarf_elf_init()
    or the -b() form was used to init 'dbg'.
*/
int
dwarf_finish(Dwarf_Debug dbg, Dwarf_Error * error)
{
    if(!dbg) {
        DWARF_DBG_ERROR(NULL, DW_DLE_DBG_NULL, DW_DLV_ERROR);
    }
    if (dbg->de_obj_file) {
        char otype  = *(char *)(dbg->de_obj_file->object);

        if (otype == 'E') {
            dwarf_elf_object_access_finish(dbg->de_obj_file);
        } else if (otype == 'M') {
            _dwarf_destruct_macho_access(dbg->de_obj_file);
#if 0
        } else if (otype == 'P') {
            /* PE files not yet dealt with */
#endif
        } else {
            /*  Do nothing. A serious internal error */
        }
    }
    return dwarf_object_finish(dbg, error);
}
