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
    Dwarf_Unsigned *culist_len,
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
    *culist_len = list_len;
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
static int
print_addressarea(Dwarf_Debug dbg,
    Dwarf_Gdbindex  gdbindex,
    Dwarf_Error * err)
{
    Dwarf_Unsigned list_len = 0;
    Dwarf_Unsigned i;
    int res = dwarf_gdbindex_addressarea(gdbindex,
        &list_len,err);
    if (res != DW_DLV_OK) {
        print_error_and_continue(dbg,
            "dwarf_gdbindex_addressarea failed",res,*err);
        return res;
    }
    cout <<"  Address table array length: " <<list_len <<
        " format: [entry#] lowpc highpc cu-index" <<endl;

    for( i  = 0; i < list_len; i++) {
        Dwarf_Unsigned lowpc = 0;
        Dwarf_Unsigned highpc = 0;
        Dwarf_Unsigned cu_index,
        res = dwarf_gdbindex_addressarea_entry(gdbindex,i,
            &lowpc,&highpc,
            &cu_index,
            err);
        if (res != DW_DLV_OK) {
            print_error_and_continue(dbg,
                "dwarf_gdbindex_addressarea_entry failed",res,*err);
            return res;
        }
        cout <<"    ["<< IToDec(i,4) << "] " <<
            IToHex0N(lowpc,10) << " " <<
            IToHex0N(highpc,10) << " " <<
            IToDec(cu_index,4) << endl;
    }
    printf("\n");
    return DW_DLV_OK;
}

const char *kind_list[] = {
  "unknown(0)  ",
  "type(1)     ",
  "var-enum(2) ",
  "function(3) ",
  "other-sym(4)",
  "reserved(5) ",
  "function(6) ",
  "reserved(7) ",
};
const char *
get_kind(unsigned k)
{
    if (k <= 7) {
        return kind_list[k];
    }
    return "kind-erroneous";
}

static string
cu_index_string(Dwarf_Unsigned index,
    Dwarf_Unsigned culist_len)
{
    char  temp_space[40];
    if (index > 162) cout << "dadebug index " <<index<< endl;
    if (index < culist_len) {
        return IToDec(index,4);
    }
    Dwarf_Unsigned type_index = index-culist_len;
    string out = IToDec(index,4);
    string tnum = "(T" + IToDec(type_index) + ")";
    out.append(tnum);
    return out;
}


static int
print_symtab_entry(Dwarf_Debug dbg,
    Dwarf_Gdbindex gdbindex,
    Dwarf_Unsigned index,
    Dwarf_Unsigned symnameoffset,
    Dwarf_Unsigned cuvecoffset,
    Dwarf_Unsigned culist_len,
    Dwarf_Error *err)
{
    int res = 0;
    const char *name = 0;
    Dwarf_Unsigned cuvec_len = 0;
    Dwarf_Unsigned ii = 0;

    if (symnameoffset == 0 && cuvecoffset == 0) {
        if (verbose > 1) {
            cout <<"        [" << IToDec(index,4) <<
                "] \"empty-hash-entry\"" << endl;
        }
        return DW_DLV_OK;
    }
    res = dwarf_gdbindex_string_by_offset(gdbindex,
        symnameoffset,&name,err);
    if(res != DW_DLV_OK) {
        print_error_and_continue(dbg,
            "dwarf_gdbindex_string_by_offset failed",res,*err);
        return res;
    }
    res = dwarf_gdbindex_cuvector_length(gdbindex,
        cuvecoffset,&cuvec_len,err);
    if( res != DW_DLV_OK) {
        print_error_and_continue(dbg,
            "dwarf_gdbindex_cuvector_length failed",res,*err);
        return res;
    }
    if (verbose > 1) {
        cout <<"     [" << IToDec(index,4) << "]" <<
            "stroff "<<IToHex0N(symnameoffset,10) <<
            " cuvecoff "<<IToHex0N(cuvecoffset,10) <<
            " cuveclen "<<IToHex0N(cuvec_len,10) << endl;
    }
    for(ii = 0; ii < cuvec_len; ++ii ) {
        Dwarf_Unsigned attributes = 0;
        Dwarf_Unsigned cu_index = 0;
        Dwarf_Unsigned reserved1 = 0;
        Dwarf_Unsigned symbol_kind = 0;
        Dwarf_Unsigned is_static = 0;


        res = dwarf_gdbindex_cuvector_inner_attributes(
            gdbindex,cuvecoffset,ii,
            &attributes,err);
        if( res != DW_DLV_OK) {
            print_error_and_continue(dbg,
                "dwarf_gdbindex_cuvector_inner_attributes failed",res,*err);
            return res;
        }
        // if cu_index is > the culist_len, then it  refers
        // to a tu_index of  'cu_index - culist_len'
        res = dwarf_gdbindex_cuvector_instance_expand_value(gdbindex,
            attributes, &cu_index,&reserved1,&symbol_kind, &is_static,
            err);
        if( res != DW_DLV_OK) {
            print_error_and_continue(dbg,
                "dwarf_gdbindex_cuvector_instance_expand_value failed",res,*err);
            return res;
        }
        if (cuvec_len == 1) {
            cout <<"  [" << IToDec(index,4) << "]" <<
                cu_index_string(cu_index,culist_len) <<
                " ["  <<
                (is_static?
                    "static ":
                    "global ") <<
                " " <<
                get_kind(symbol_kind) << "] " <<
                "\"" << name << "\"" << endl;
        } else if (ii == 0) {
            cout <<"  [" << IToDec(index,4) << "]" <<
                " \"" << name << "\"" << endl;
            cout <<"         " << cu_index_string(cu_index,culist_len) <<
                " [" <<
                (is_static?
                    "static ":
                    "global ") <<
                " " <<
                get_kind(symbol_kind) << "]" << endl;
        }else{
            cout <<"         " << cu_index_string(cu_index,culist_len) <<
                " [" <<
                (is_static?
                    "static ":
                    "global ") <<
                " " <<
                get_kind(symbol_kind) << "]" << endl;
        }
        if (verbose > 1) {
            cout <<"        ["<< IToDec(ii,4) << "]" <<
                "attr " << IToHex0N(attributes,10) <<
                " cuindx " << IToHex0N(cu_index,10) <<
                " kind " << IToHex0N(symbol_kind,10) <<
                " static " << IToHex0N(is_static,10) << endl;
        }

    }
    return DW_DLV_OK;
}


static int
print_symboltable(Dwarf_Debug dbg,
    Dwarf_Gdbindex  gdbindex,
    Dwarf_Unsigned culist_len,
    Dwarf_Error * err)
{
    Dwarf_Unsigned list_len = 0;
    Dwarf_Unsigned i;
    int res = dwarf_gdbindex_symboltable_array(gdbindex,
        &list_len,err);
    if (res != DW_DLV_OK) {
        print_error_and_continue(dbg,
            "dwarf_gdbindex_symboltable failed",res,*err);
        return res;
    }
    cout << endl;
    cout <<"  Symbol table: length " << list_len <<
        " format: [entry#] symindex cuindex [type] \"name\" or " << endl;
    cout <<"                          "
        " format: [entry#]  \"name\" , list of  cuindex [type]" << endl;

    for( i  = 0; i < list_len; i++) {
        Dwarf_Unsigned symnameoffset = 0;
        Dwarf_Unsigned cuvecoffset = 0;
        res = dwarf_gdbindex_symboltable_entry(gdbindex,i,
            &symnameoffset,&cuvecoffset,
            err);
        if (res != DW_DLV_OK) {
            print_error_and_continue(dbg,
                "dwarf_gdbindex_symboltable_entry failed",res,*err);
            return res;
        }
        res = print_symtab_entry(dbg,gdbindex,i,symnameoffset,cuvecoffset,culist_len,err);
        if (res != DW_DLV_OK) {
            return res;
        }
    }
    printf("\n");
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
    const char *section_name = 0;

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
        &section_name,
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
    if (!section_name || !*section_name) {
        section_name = ".gdb_index";
    }
    cout << section_name << endl;
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
    cout << "  Symbol table offset : " <<
        IToHex0N(address_area_offset,10) <<
        endl;
    cout << "  Constant pool offset: " <<
        IToHex0N(constant_pool_offset,10) <<
        endl;
    cout << "  section size        : " <<
        IToHex0N(section_size,10) <<
        endl;

    Dwarf_Unsigned culist_len = 0;
    res = print_culist_array(dbg,gdbindex,&culist_len,&error);
    if (res != DW_DLV_OK) {
        return;
    }
    res = print_types_culist_array(dbg,gdbindex,&error);
    if (res != DW_DLV_OK) {
        return;
    }
    res = print_addressarea(dbg,gdbindex,&error);
    if (res != DW_DLV_OK) {
        return;
    }
    res = print_symboltable(dbg,gdbindex,culist_len,&error);
    if (res != DW_DLV_OK) {
        return;
    }
}

