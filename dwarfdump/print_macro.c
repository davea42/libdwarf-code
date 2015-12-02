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
