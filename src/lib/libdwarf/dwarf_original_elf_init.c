/*
  Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2008-2010 Arxan Technologies, Inc. All rights reserved.
  Portions Copyright 2011-2020 David Anderson. All rights reserved.
  Portions Copyright 2012 SN Systems Ltd. All rights reserved.

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

#include "config.h"
#include <stdio.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h> /* open(), off_t, size_t, ssize_t */
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include "dwarf_incl.h"
#include "dwarf_error.h"


#define DWARF_DBG_ERROR(dbg,errval,retval) \
    _dwarf_error(dbg, error, errval); return(retval);

/*  No longer appropriate. No libelf. */
int
dwarf_elf_init_b(
    UNUSEDARG void * elf_file_pointer,
    UNUSEDARG Dwarf_Unsigned access,
    UNUSEDARG unsigned group_number,
    UNUSEDARG Dwarf_Handler errhand,
    UNUSEDARG Dwarf_Ptr errarg,
    UNUSEDARG Dwarf_Debug * ret_dbg,
    Dwarf_Error * error)
{
    DWARF_DBG_ERROR(NULL, DW_DLE_NO_ELF_SUPPORT, DW_DLV_ERROR);
}

int
dwarf_elf_init(
    UNUSEDARG void * elf_file_pointer,
    UNUSEDARG Dwarf_Unsigned access,
    UNUSEDARG Dwarf_Handler errhand,
    UNUSEDARG Dwarf_Ptr errarg,
    UNUSEDARG Dwarf_Debug * ret_dbg,
    Dwarf_Error * error)
{
    DWARF_DBG_ERROR(NULL, DW_DLE_NO_ELF_SUPPORT, DW_DLV_ERROR);
}

