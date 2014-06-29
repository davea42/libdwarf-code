/*
  Copyright 2014-2014 David Anderson. All rights reserved.

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

#include "globals.h"
#include "naming.h"
#include "esb.h"

#include "print_sections.h"


extern void
print_gdb_index(Dwarf_Debug dbg)
{
    Dwarf_Gdbindex  gdbindex = 0;
    Dwarf_Unsigned version = 0;
    Dwarf_Unsigned cu_list_offset = 0;
    Dwarf_Unsigned types_cu_list_offset = 0;
    Dwarf_Unsigned address_area_offset = 0;
    Dwarf_Unsigned symbol_table_offset = 0;
    Dwarf_Unsigned constant_pool_offset = 0;
    Dwarf_Unsigned section_size = 0;
    Dwarf_Unsigned unused = 0;
    Dwarf_Error error = 0;

    int res = 0;
    current_section_id = DEBUG_GDB_INDEX;
    res = dwarf_gdbindex_header(dbg, &gdbindex,
        &version,
        &cu_list_offset,
        &types_cu_list_offset,
        &address_area_offset,
        &symbol_table_offset,
        &constant_pool_offset,
        &section_size,
        &unused,
        &error);

    if (!do_print_dwarf) {
        return;
    }
    if(res == DW_DLV_NO_ENTRY) {
        /*  Silently! The section is rare so lets
            say nothing. */
        return;
    }
    printf("\n.gdb_index\n");
    if( res == DW_DLV_ERROR) {
        print_error(dbg,"dwarf_gdbindex_header",res,error);
        return;
    }

    printf("  Version             : "
        "%" DW_PR_XZEROS DW_PR_DUx  "\n",
        version);
    printf("  CU list offset      : "
        "%" DW_PR_XZEROS DW_PR_DUx "\n",
        cu_list_offset);
    printf("  Address area offset : "
        "%" DW_PR_XZEROS DW_PR_DUx "\n",
        types_cu_list_offset);
    printf("  Symboltable offset  : "
        "%" DW_PR_XZEROS DW_PR_DUx "\n",
        address_area_offset);
    printf("  Constant pool offset: "
        "%" DW_PR_XZEROS DW_PR_DUx "\n",
        constant_pool_offset);
    printf("  section size        : "
        "%" DW_PR_XZEROS DW_PR_DUx "\n",
        section_size);
}

