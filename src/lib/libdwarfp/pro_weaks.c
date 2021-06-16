/*

  Copyright (C) 2000,2004 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2011-2020  David Anderson. All Rights Reserved.

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
#include "libdwarf_private.h"
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_ELFACCESS_H
#include <elfaccess.h>
#endif /*  HAVE_ELFACCESS_H */
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif /* HAVE_STDDEF_H */
#include "dwarf.h"
#include "libdwarfp.h"
#include "pro_opaque.h"
#include "pro_error.h"

/*
    This function adds another weak name to the
    list of weak names for the given Dwarf_P_Debug.
    It returns 0 on error, and 1 otherwise.
*/
Dwarf_Unsigned
dwarf_add_weakname(Dwarf_P_Debug dbg,
    Dwarf_P_Die die,
    char *weak_name, Dwarf_Error * error)
{
    int res = 0;

    res = _dwarf_add_simple_name_entry(dbg, die, weak_name,
        dwarf_snk_weakname, error);
    if (res != DW_DLV_OK) {
        return 0;
    }
    return 1;
}

int
dwarf_add_weakname_a(Dwarf_P_Debug dbg,
    Dwarf_P_Die die,
    char *weak_name, Dwarf_Error * error)
{
    int res = 0;

    res = _dwarf_add_simple_name_entry(dbg, die, weak_name,
        dwarf_snk_weakname, error);
    return res;
}