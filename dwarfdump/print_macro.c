/*
  Copyright 2015-2015 David Anderson. All rights reserved.

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
#include "uri.h"
#include <ctype.h>
#include <time.h>

#include "print_sections.h"

static void
print_source_intro(Dwarf_Die cu_die)
{   
    Dwarf_Off off = 0; 
    int ores = dwarf_dieoffset(cu_die, &off, &err);

    if (ores == DW_DLV_OK) {
        int lres = 0;
        const char *sec_name = 0;
        lres = dwarf_get_die_section_name_b(cu_die,
            &sec_name,&err);
        if (lres != DW_DLV_OK ||  !sec_name || !strlen(sec_name)) {
            sec_name = ".debug_info";
        }

        printf("Macro data from CU-DIE at %s offset 0x%"
            DW_PR_XZEROS DW_PR_DUx "):\n",
            sec_name,
            (Dwarf_Unsigned) off);
    } else {
        printf("Macro data (for the CU-DIE at unknown location):\n");
    }
}


extern void
print_macros_5style_this_cu(Dwarf_Debug dbg, Dwarf_Die cu_die)
{
    int lres = 0;
    const char *sec_name = 0;
    Dwarf_Unsigned version = 0;
    Dwarf_Macro_Context macro_context = 0;

    current_section_id = DEBUG_MACRO;
    lres = dwarf_get_macro_section_name(dbg,&sec_name,&err);
    if (lres != DW_DLV_OK || !sec_name || !strlen(sec_name)) {
        sec_name = ".debug_macro";
    }
    if (do_print_dwarf) {
        printf("\n%s: Macro info for a single cu\n", sec_name);
    } else {
        /* We are checking, not printing. */
        Dwarf_Half tag = 0;
        int tres = dwarf_tag(cu_die, &tag, &err);
        if (tres != DW_DLV_OK) {
            /*  Something broken here. */
            print_error(dbg,"Unable to see CU DIE tag "
                "though we could see it earlier. Something broken.",
                tres,err);
            return;
        } else if (tag == DW_TAG_type_unit) {
            /*  Not checking since type units missing
                address or range in CU header. */
            return;
        }
    }
    lres = dwarf_get_macro_context(cu_die,&version,&macro_context,
        &err);
    if(lres == DW_DLV_NO_ENTRY) {
        return;
    }
    if(lres == DW_DLV_ERROR) {
        print_error(dbg,"Unable to dwarf_get_macro_context()",
            lres,err);
        return;
    }
    if (do_print_dwarf && verbose > 1) {
        int errcount = 0;
        print_source_intro(cu_die);
        print_one_die(dbg, cu_die,
            /* print_information= */ 1,
            /* indent level */0,
            /* srcfiles= */ 0, /* cnt= */ 0,
            /* ignore_die_stack= */TRUE);
#if 0
        DWARF_CHECK_COUNT(lines_result,1);
        lres = dwarf_print_lines(cu_die, &err,&errcount);
        if (errcount > 0) {
            DWARF_ERROR_COUNT(lines_result,errcount);
            DWARF_CHECK_COUNT(lines_result,(errcount-1));
        }
        if (lres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_srclines details", lres, err);
        }
#endif
    }
    if (do_print_dwarf) {
        Dwarf_Half lversion =0;
        Dwarf_Unsigned mac_offset =0;
        Dwarf_Unsigned mac_len =0;
        Dwarf_Unsigned mac_header_len =0;
        unsigned mflags = 0;
        Dwarf_Bool has_line_offset = FALSE;
        Dwarf_Bool has_offset_size_64 = FALSE;
        Dwarf_Bool has_operands_table = FALSE;
        Dwarf_Half opcode_count = 0;

        lres = dwarf_macro_context_head(macro_context,
            &lversion, &mac_offset,&mac_len,
            &mac_header_len,&mflags,&has_line_offset,
            &has_offset_size_64,&has_operands_table,
            &opcode_count,&err);
        if(lres == DW_DLV_NO_ENTRY) {
            /* Impossible */
            return;
        }
        if(lres == DW_DLV_ERROR) {
            print_error(dbg,"Call to dwarf_macro_context_head() failed",
                lres,err);
            return;
        }
        printf("  Macro version: %d\n",lversion);
        if( verbose) {
            printf("  macro section offset 0x%" DW_PR_XZEROS DW_PR_DUx "\n",mac_offset);
            printf("  flags: 0x%x, line offset? %u offsetsize 64? %u, operands_table? %u\n",
               mflags,has_line_offset,has_offset_size_64, has_operands_table);
            printf("  header length: 0x%" DW_PR_XZEROS DW_PR_DUx 
                 "  total length: 0x%" DW_PR_XZEROS DW_PR_DUx "\n",
                 mac_header_len,mac_len);
           if (has_operands_table) {
               Dwarf_Half i = 0;
   
               for( i = 0; i < opcode_count; ++i) {
                   Dwarf_Half opcode_num = 0;
                   Dwarf_Half operand_count = 0;
                   const Dwarf_Small *operand_array = 0;
                   Dwarf_Half j = 0;
                   
                   lres = dwarf_macro_operands_table(macro_context,
                       i, &opcode_num, &operand_count,&operand_array,&err);
                   if (lres == DW_DLV_NO_ENTRY) {
                       dwarf_dealloc_macro_context(macro_context); 
                       print_error(dbg,"NO ENTRY? dwarf_macro_operands_table()",
                          lres,err);
                       return;
                   }
                   if (lres == DW_DLV_ERROR) {
                       dwarf_dealloc_macro_context(macro_context); 
                       print_error(dbg,"ERROR from  dwarf_macro_operands_table()",
                          lres,err);
                       return;
                   }
                   printf("  [%3u]  op: 0x%04x  %20s  operandcount: %u\n",
                       i,opcode_num, get_MACRO_name(opcode_num, dwarf_names_print_on_error),
                       operand_count);
                   for (j = 0; j < operand_count; ++j) {
                       Dwarf_Small opnd = operand_array[j];
                       printf("    [%3u] 0x%04x %20s\n", j,opnd,
                           get_FORM_name(opnd, dwarf_names_print_on_error));
                   }
               }
           }
        }
    }
#if 0
    if (check_lines && checking_this_compiler()) {
        DWARF_CHECK_COUNT(lines_result,1);
        dwarf_check_lineheader(cu_die,&line_errs);
        if (line_errs > 0) {
            DWARF_CHECK_ERROR_PRINT_CU();
            DWARF_ERROR_COUNT(lines_result,line_errs);
            DWARF_CHECK_COUNT(lines_result,(line_errs-1));
        }
    }
#endif
    dwarf_dealloc_macro_context(macro_context);
    macro_context = 0;
}
