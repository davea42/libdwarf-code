/*
  Copyright (C) 2017-2017  David Anderson. All rights reserved.

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
#include "dwconf.h"
#include "esb.h"

/*  NEW May 2017. 
    Reports on section groupings like DWO(split dwarf)
    and COMDAT groups. */
void
print_section_groups_data(Dwarf_Debug dbg)
{
    int res = 0;
    Dwarf_Error error = 0;
    Dwarf_Unsigned section_count = 0;
    Dwarf_Unsigned group_count = 0;
    Dwarf_Unsigned selected_group = 0;
    Dwarf_Unsigned group_map_entry_count = 0;
    Dwarf_Unsigned *sec_nums = 0;
    Dwarf_Unsigned *group_nums = 0;
    Dwarf_Unsigned i = 0;

    printf("FIXME: incomplete\n");
    res = dwarf_sec_group_sizes(dbg,&section_count,
        &group_count,&selected_group, &group_map_entry_count,
        &error);
    if(res != DW_DLV_OK) {
        print_error(dbg, "dwarf_sec_group_sizes", res, error);
    }
    if (group_count == 1 && selected_group ==1 ) {
        /* This is the traditional DWARF with no split-dwarf
           and no COMDAT data. 
           We don't want to print anything as we do not want
           to see differences from existing output in this case.  
           Simplifies regression testing for now. */
        return;
    }
    printf("Section Groups data\n");
    printf("  Number of Elf-like sections: %4" DW_PR_DUu "\n",
        section_count);
    printf("  Number of groups           : %4" DW_PR_DUu "\n",
        group_count);
    printf("  Group to print             : %4" DW_PR_DUu "\n",
        selected_group);
    printf("  Count of map entries       : %4" DW_PR_DUu "\n",
        group_map_entry_count);

    sec_nums = calloc(group_map_entry_count,sizeof(Dwarf_Unsigned));
    if(!sec_nums) {
        printf("ERROR: Unable to allocate %4" DW_PR_DUu 
            " map section values, cannot print group map\n",
            group_map_entry_count);
        return;
    }
    group_nums = calloc(group_map_entry_count,sizeof(Dwarf_Unsigned));
    if(!group_nums) {
        printf("ERROR: Unable to allocate %4" DW_PR_DUu 
            " map group values, cannot print group map\n",
            group_map_entry_count);
        return;
    }

    res = dwarf_sec_group_map(dbg,group_map_entry_count,
        group_nums,sec_nums,&error);
    if(res != DW_DLV_OK) {
        print_error(dbg, "dwarf_sec_group_map", res, error);
    }

    for( i = 0; i < group_map_entry_count; ++i) {
        if (i == 0) {
            printf("  [index]  group section\n");
        }
        printf("  [%5" DW_PR_DUu "] "
            "%4" DW_PR_DUu 
            "  %4" DW_PR_DUu 
            "\n",i,group_nums[i],sec_nums[i]);
    }
    free(sec_nums);
    free(group_nums);
    return;
}
