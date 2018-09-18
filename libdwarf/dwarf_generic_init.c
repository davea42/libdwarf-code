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
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "dwarf_incl.h"
#include "dwarf_error.h"
#include "dwarf_elf_access.h"

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

#if 0
int
dwarf_init_path(const char *path,
    Dwarf_Unsigned access,
    Dwarf_Handler errhand,
    Dwarf_Ptr errarg, Dwarf_Debug * ret_dbg, Dwarf_Error * error)
{
FIXME
    return dwarf_init_b(fd,access, DW_GROUPNUMBER_ANY,
        errhand,errarg,ret_dbg,error);
}
#endif

/* New March 2017 */
int
dwarf_init_b(int fd,
    Dwarf_Unsigned access,
    unsigned  group_number,
    Dwarf_Handler errhand,
    Dwarf_Ptr errarg, Dwarf_Debug * ret_dbg, Dwarf_Error * error)
{
    struct stat fstat_buf;
    dwarf_elf_handle elf_file_pointer = 0;
    /* ELF_C_READ is a portable value */
    Elf_Cmd what_kind_of_elf_read = ELF_C_READ;

#if !defined(S_ISREG)
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif
    if (fstat(fd, &fstat_buf) != 0) {
        DWARF_DBG_ERROR(NULL, DW_DLE_FSTAT_ERROR, DW_DLV_ERROR);
    }
    if (!S_ISREG(fstat_buf.st_mode)) {
        DWARF_DBG_ERROR(NULL, DW_DLE_FSTAT_MODE_ERROR, DW_DLV_ERROR);
    }

    if (access != DW_DLC_READ) {
        DWARF_DBG_ERROR(NULL, DW_DLE_INIT_ACCESS_WRONG, DW_DLV_ERROR);
    }

    elf_version(EV_CURRENT);
    /*  Changed to mmap request per bug 281217. 6/95 */
#ifdef HAVE_ELF_C_READ_MMAP
    /*  ELF_C_READ_MMAP is an SGI IRIX specific enum value from IRIX
        libelf.h meaning read but use mmap.
        It is never necessary -- it is just a convenience.
        HAVE_ELF_C_READ_MMAP has not been in config.h via
        configure since 2004 at least. */
    what_kind_of_elf_read = ELF_C_READ_MMAP;
#endif /* !HAVE_ELF_C_READ_MMAP */

    elf_file_pointer = elf_begin(fd, what_kind_of_elf_read, 0);
    if (elf_file_pointer == NULL) {
        DWARF_DBG_ERROR(NULL, DW_DLE_ELF_BEGIN_ERROR, DW_DLV_ERROR);
    }
    return _dwarf_elf_init_file_ownership(elf_file_pointer,
        TRUE, group_number, access, errhand, errarg, ret_dbg, error);
}

