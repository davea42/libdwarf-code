/*
Copyright 2017-2018 David Anderson. All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of version 2 of the GNU General
  Public License as published by the Free Software Foundation.

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

  You should have received a copy of the GNU General Public
  License along with this program; if not, write the Free
  Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
  Boston MA 02110-1301, USA.

*/
#include "config.h"
#include "dd_globals.h"
#include "dwarf.h"
#include "libdwarf.h"
#include "dd_naming.h"
#include "dd_sanitized.h"
#include "dd_esb.h"
#include "dd_esb_using_functions.h"

static int
print_dname_record(Dwarf_Dnames_Head dn,
    Dwarf_Unsigned offset,
    Dwarf_Unsigned new_offset,
    Dwarf_Error *error)
{
    int res = 0;
    Dwarf_Unsigned comp_unit_count = 0;
    Dwarf_Unsigned local_type_unit_count = 0;
    Dwarf_Unsigned foreign_type_unit_count = 0;
    Dwarf_Unsigned bucket_count = 0;
    Dwarf_Unsigned name_count = 0;
    Dwarf_Unsigned abbrev_table_size = 0;
    Dwarf_Unsigned entry_pool_size = 0;
    Dwarf_Unsigned augmentation_string_size = 0;
    Dwarf_Unsigned section_size = 0;
    Dwarf_Half table_version = 0;
    Dwarf_Half offset_size = 0;
    char * augstring = 0;

    res = dwarf_dnames_sizes(dn,&comp_unit_count,
        &local_type_unit_count,&foreign_type_unit_count,
        &bucket_count,
        &name_count,&abbrev_table_size,
        &entry_pool_size,&augmentation_string_size,
        &augstring,
        &section_size,&table_version,
        &offset_size,
        error);
    if (res != DW_DLV_OK) {
        return res;
    }
    printf("\n");
    printf("Name table offset       : 0x%"
        DW_PR_XZEROS DW_PR_DUx "\n",
        offset);
    printf("Next name table offset  : 0x%"
        DW_PR_XZEROS DW_PR_DUx "\n",
        new_offset);
    printf("Section size            : 0x%"
        DW_PR_XZEROS DW_PR_DUx "\n",
        section_size);
    printf("Table version           : %u\n",
        table_version);
    printf("Comp unit count         : %" DW_PR_DUu "\n",
        comp_unit_count);
    printf("Type unit count         : %" DW_PR_DUu "\n",
        local_type_unit_count);
    printf("Foreign Type unit count : %" DW_PR_DUu "\n",
        foreign_type_unit_count);
    printf("Bucket count            : %" DW_PR_DUu "\n",
        bucket_count);
    printf("Name count              : %" DW_PR_DUu "\n",
        name_count);
    printf("Abbrev table length     : %" DW_PR_DUu "\n",
        abbrev_table_size);
    printf("Entry pool size         : %" DW_PR_DUu "\n",
        entry_pool_size);
    printf("Augmentation string size: %" DW_PR_DUu "\n",
        augmentation_string_size);
    if (augmentation_string_size > 0) {
        printf("Augmentation string     : %s\n",
            sanitized(augstring));
    }
    return DW_DLV_OK;
}

int
print_debug_names(Dwarf_Debug dbg,Dwarf_Error *error)
{
    Dwarf_Dnames_Head dnhead = 0;
    Dwarf_Unsigned offset = 0;
    Dwarf_Unsigned new_offset = 0;
    int res = 0;
    const char * section_name = ".debug_names";
    struct esb_s truename;
    char buf[DWARF_SECNAME_BUFFER_SIZE];

    if (!dbg) {
        printf("ERROR: Cannot print .debug_names, "
            "no Dwarf_Debug passed in");
        return DW_DLV_NO_ENTRY;
    }
    glflags.current_section_id = DEBUG_NAMES;
    /* Do nothing if not printing. */
    if (!glflags.gf_do_print_dwarf) {
        return DW_DLV_OK;
    }

    /*  Only print anything if we know it has debug names
        present. And for now there is none. FIXME. */
    res = dwarf_dnames_header(dbg,offset,&dnhead,&new_offset,error);
    if (res == DW_DLV_NO_ENTRY) {
        return res;
    }
    esb_constructor_fixed(&truename,buf,sizeof(buf));
    get_true_section_name(dbg,section_name,
        &truename,TRUE);
    printf("\n%s\n",sanitized(esb_get_string(&truename)));
    esb_destructor(&truename);
    while (res == DW_DLV_OK) {
        res = print_dname_record(dnhead,offset,new_offset,error);
        if (res != DW_DLV_OK) {
            dwarf_dealloc_dnames(dnhead);
            dnhead = 0;
            return res;
        }
        offset = new_offset;
        dwarf_dealloc_dnames(dnhead);
        dnhead = 0;
        res = dwarf_dnames_header(dbg,offset,&dnhead,
            &new_offset,error);
    }
    return res;
}
