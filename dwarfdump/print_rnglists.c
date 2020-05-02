/*
    Copyright (C) 2020 David Anderson. All Rights Reserved.

    This program is free software; you can redistribute it
    and/or modify it under the terms of version 2 of the GNU
    General Public License as published by the Free Software
    Foundation.

    This program is distributed in the hope that it would
    be useful, but WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.

    Further, this software is distributed without any warranty
    that it is free of the rightful claim of any third person
    regarding infringement or the like.  Any license provided
    herein, whether implied or otherwise, applies only to
    this software file.  Patent licenses, if any, provided
    herein do not apply to combinations of this program with
    other software, or any other product whatsoever.

    You should have received a copy of the GNU General Public
    License along with this program; if not, write the Free
    Software Foundation, Inc., 51 Franklin Street - Fifth
    Floor, Boston MA 02110-1301, USA.
*/

/*  DWARF5 has the new .debug_rnglists section.
    Here we print that data.
    The raw printing covers all the content of the
    section but without relating it to any
    compilation unit.

    Printing the actual address means printing
    with the actual DIEs on hand. FIXME: do this
*/

#include "config.h"
#include "globals.h"
#include "esb.h"
#include "esb_using_functions.h"
#include "sanitized.h"

static void
print_sec_name(Dwarf_Debug dbg)
{
    struct esb_s truename;
    char buf[DWARF_SECNAME_BUFFER_SIZE];

    esb_constructor_fixed(&truename,buf,sizeof(buf));
    get_true_section_name(dbg,".debug_rnglists",
        &truename,TRUE);
    printf("\n%s\n\n",sanitized(esb_get_string(&truename)));
    esb_destructor(&truename);
}


int
print_raw_all_rnglists(Dwarf_Debug dbg,
    Dwarf_Error *error)
{
    int res = 0;
    Dwarf_Unsigned count = 0;
    Dwarf_Unsigned i = 0;

    res = dwarf_load_rnglists(dbg,&count,error);
    if (res != DW_DLV_OK) {
        return res;
    }
    print_sec_name(dbg);

    printf(" Number of rnglists contexts:  %" DW_PR_DUu "\n",
        count);
    for (i = 0; i < count ; ++i) {
        Dwarf_Unsigned header_offset = 0;
        Dwarf_Small   offset_size = 0;
        Dwarf_Small   extension_size = 0;
        unsigned      version = 0; /* 5 */
        Dwarf_Small   address_size = 0;
        Dwarf_Small   segment_selector_size = 0;
        Dwarf_Unsigned offset_entry_count = 0;
        Dwarf_Unsigned offset_of_offset_array = 0;
        Dwarf_Unsigned *offset_array_ptr = 0;
        Dwarf_Unsigned offset_of_first_rangeentry = 0;
        Dwarf_Unsigned offset_past_last_rangeentry = 0;

        res = dwarf_get_rnglist_context_basics(dbg,i,
            &header_offset,&offset_size,&extension_size,
            &version,&address_size,&segment_selector_size,
            &offset_entry_count,&offset_of_offset_array,
            &offset_array_ptr,&offset_of_first_rangeentry,
            &offset_past_last_rangeentry,error);
        if (res != DW_DLV_OK) {
            struct esb_s m;

            esb_constructor(&m);
            esb_append_printf_u(&m,"ERROR: Getting debug_rnglists "
                "entry %u we unexpectedly stop early.",i);
            simple_err_return_msg_either_action(res,
                esb_get_string(&m));
            esb_destructor(&m);
            return res;
        }
        printf("  Context number         : %3" DW_PR_DUu "\n",i);
        printf("   Version               : %3u\n",version);
        printf("   address size          : %3u\n",address_size);
        printf("   offset size           : %3u\n",offset_size);
        if(glflags.verbose) {
            printf("   extension size        : %3u\n",extension_size);
        }
        printf("   segment selector size : %3u\n",
            segment_selector_size);
        printf("   offset entry count    : %3" DW_PR_DUu "\n",
            offset_entry_count);
        printf("   context size in bytes : %3" DW_PR_DUu "\n",
            offset_past_last_rangeentry - header_offset);
        if(glflags.verbose) {
            printf("   Offset in section     : 0x%"
                DW_PR_XZEROS DW_PR_DUx"\n",
                header_offset);
            printf("   Offset  of offsets    : 0x%"
                DW_PR_XZEROS DW_PR_DUx"\n",
                offset_of_offset_array);
            printf("   Offsetof first range  : 0x%"
                DW_PR_XZEROS DW_PR_DUx"\n",
                offset_of_first_rangeentry);
            printf("   Offset past ranges    : 0x%"
                DW_PR_XZEROS DW_PR_DUx"\n",
                offset_past_last_rangeentry);
        }
    }
    return res;
}
