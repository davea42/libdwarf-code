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
#include <vector>
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;


static int
print_culist_array(Dwarf_Debug dbg,
    Dwarf_Gdbindex  gdbindex,
    Dwarf_Error * err)
{
    Dwarf_Unsigned list_len = 0;
    Dwarf_Unsigned i;
    int res = dwarf_gdbindex_culist_array(gdbindex,
        &list_len,err);
    if (res != DW_DLV_OK) {
        print_error_and_continue(dbg,
            "dwarf_gdbindex_culist_array failed",res,*err);
        return res;
    }
    cout <<"  CU list. array length: " << list_len <<
        " format: [entry#] cuoffset culength"  <<endl;

    for( i  = 0; i < list_len; i++) {
        Dwarf_Unsigned cuoffset = 0;
        Dwarf_Unsigned culength = 0;
        res = dwarf_gdbindex_culist_entry(gdbindex,i,
            &cuoffset,&culength,err);
        if (res != DW_DLV_OK) {
            print_error_and_continue(dbg,
               "dwarf_gdbindex_culist_entry failed",res,*err);
            return res;
        }
        cout <<"    ["<< IToDec(i,4) << "] " <<
             IToHex0N(cuoffset,10) << " " <<
             IToHex0N(culength,10) << endl;
    }
    cout << endl;
    return DW_DLV_OK;
}

static int
print_types_culist_array(Dwarf_Debug dbg,
    Dwarf_Gdbindex  gdbindex,
    Dwarf_Error * err)
{
    Dwarf_Unsigned list_len = 0;
    Dwarf_Unsigned i;
    int res = dwarf_gdbindex_types_culist_array(gdbindex,
        &list_len,err);
    if (res != DW_DLV_OK) {
        print_error_and_continue(dbg,
            "dwarf_gdbindex_types_culist_array failed",res,*err);
        return res;
    }
    cout <<"  TU list. array length: " << list_len <<
        " format: [entry#] cuoffset culength signature"  <<endl;

    for( i  = 0; i < list_len; i++) {
        Dwarf_Unsigned cuoffset = 0;
        Dwarf_Unsigned culength = 0;
        Dwarf_Unsigned signature = 0;
        res = dwarf_gdbindex_types_culist_entry(gdbindex,i,
            &cuoffset,&culength,
            &signature,err);
        if (res != DW_DLV_OK) {
            print_error_and_continue(dbg,
               "dwarf_gdbindex_types_culist_entry failed",res,*err);
            return res;
        }
        cout <<"    ["<< IToDec(i,4) << "] " <<
             IToHex0N(cuoffset,10) << " " <<
             IToHex0N(culength,10) << " " <<
             IToHex0N(signature,10) << endl;
    }
    cout << endl;
    return DW_DLV_OK;
}

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
    error_message_data.current_section_id = DEBUG_GDB_INDEX;
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
    cout << endl ;
    cout << ".gdb_index" << endl;
    if( res == DW_DLV_ERROR) {
        print_error(dbg,"dwarf_gdbindex_header",res,error);
        return;
    }

    cout <<"  Version             : " <<
        IToHex0N(version,10) << 
        endl;
    cout << "  CU list offset      : " <<
        IToHex0N(cu_list_offset,10) << 
        endl;
    cout << "  Address area offset : " <<
        IToHex0N(types_cu_list_offset,10) << 
        endl;
    cout << "  Symboltable offset  : " <<
        IToHex0N(address_area_offset,10) << 
        endl;
    cout << "  Constant pool offset: " <<
        IToHex0N(constant_pool_offset,10) << 
        endl;
    cout << "  section size        : " <<
        IToHex0N(section_size,10) << 
        endl;

    res = print_culist_array(dbg,gdbindex,&error);
    if (res != DW_DLV_OK) {
        return;
    }
    res = print_types_culist_array(dbg,gdbindex,&error);
    if (res != DW_DLV_OK) {
        return;
    }

}

