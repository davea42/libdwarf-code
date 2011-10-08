/* 
  Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2009-2010 SN Systems Ltd. All rights reserved.
  Portions Copyright 2008-2011 David Anderson. All rights reserved.

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



$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/print_sections.c,v 1.69 2006/04/17 00:09:56 davea Exp $ */
/*  The address of the Free Software Foundation is
    Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
    Boston, MA 02110-1301, USA.  
    SGI has moved from the Crittenden Lane address.
*/

#include "globals.h"
#include <vector>
#include "naming.h"
#include "dwconf.h"

#include "print_sections.h"
#include "print_frames.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

/* get all the data in .debug_types */
extern void
print_types(Dwarf_Debug dbg, enum type_type_e type_type)
{
    string section_name;
    string offset_err_name;
    string section_open_name;
    string print_name_prefix;
    int (*get_types) (Dwarf_Debug, Dwarf_Type **, Dwarf_Signed *,
        Dwarf_Error *) = 0;
    int (*get_offset) (Dwarf_Type, char **, Dwarf_Off *, Dwarf_Off *,
        Dwarf_Error *) = 0;
    int (*get_cu_offset) (Dwarf_Type, Dwarf_Off *, Dwarf_Error *) =
        0;
    void (*dealloctype) (Dwarf_Debug, Dwarf_Type *, Dwarf_Signed) =
        0;

    if(!do_print_dwarf) {
        return;
    }

    if (type_type == DWARF_PUBTYPES) {
        section_name = ".debug_pubtypes";
        offset_err_name = "dwarf_pubtype_name_offsets";
        section_open_name = "dwarf_get_pubtypes";
        print_name_prefix = "pubtype";
        get_types = dwarf_get_pubtypes;
        get_offset = dwarf_pubtype_name_offsets;
        get_cu_offset = dwarf_pubtype_cu_offset;
        dealloctype = dwarf_pubtypes_dealloc;
    } else {
        /* SGI_TYPENAME */
        section_name = ".debug_typenames";
        offset_err_name = "dwarf_type_name_offsets";
        section_open_name = "dwarf_get_types";
        print_name_prefix = "type";
        get_types = dwarf_get_types;
        get_offset = dwarf_type_name_offsets;
        get_cu_offset = dwarf_type_cu_offset;
        dealloctype = dwarf_types_dealloc;
    }



    Dwarf_Signed count = 0;
    Dwarf_Type *typebuf = NULL;
    int gtres = get_types(dbg, &typebuf, &count, &err);
    if (gtres == DW_DLV_ERROR) {
        print_error(dbg, section_open_name, gtres, err);
    } else if (gtres == DW_DLV_NO_ENTRY) {
        /* no types */
    } else {
        Dwarf_Unsigned maxoff = get_info_max_offset(dbg);

        /*  Before July 2005, the section name was printed
            unconditionally, now only prints if non-empty section really 
            exists. */
        cout << endl;
        cout << section_name << endl;

        for (Dwarf_Signed i = 0; i < count; i++) {
            char *name = NULL;
            Dwarf_Off die_off = 0;
            Dwarf_Off cu_off = 0;
            Dwarf_Off global_cu_off = 0;

            int tnres =
                get_offset(typebuf[i], &name, &die_off, &cu_off, &err);
            deal_with_name_offset_err(dbg, offset_err_name, name,
                die_off, tnres, err);

            int cures3 = get_cu_offset(typebuf[i], &global_cu_off, &err);

            if (cures3 != DW_DLV_OK) {
                print_error(dbg, "dwarf_var_cu_offset", cures3, err);
            }
            print_pubname_style_entry(dbg,
                print_name_prefix,
                name, die_off, cu_off,
                global_cu_off, maxoff);

            /* print associated die too? */
        }
        dealloctype(dbg, typebuf, count);
    }
}   /* print_types() */
