/*
  Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2009-2018 SN Systems Ltd. All rights reserved.
  Portions Copyright 2007-2020 David Anderson. All rights reserved.

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

/*  The address of the Free Software Foundation is
    Free Software Foundation, Inc., 51 Franklin St, Fifth
    Floor, Boston, MA 02110-1301, USA.  SGI has moved from
    the Crittenden Lane address.  */


#include "globals.h"
#ifdef HAVE_STDINT_H
#include <stdint.h> /* For uintptr_t */
#endif /* HAVE_STDINT_H */
#include "naming.h"
#include "esb.h"                /* For flexible string buffer. */
#include "esb_using_functions.h"
#include "sanitized.h"
#include "helpertree.h"
#include "tag_common.h"

/* Prints locentry descriptsions for DW_LKIND_loclist */

int
print_original_loclist_linecodes(Dwarf_Debug dbg,
    UNUSEDARG Dwarf_Bool    checking,
    unsigned int llent,
    Dwarf_Small   lle_value,
    UNUSEDARG Dwarf_Addr  * base_address,
    Dwarf_Addr    rawlopc, 
    Dwarf_Addr    rawhipc,
    UNUSEDARG Dwarf_Bool    debug_addr_unavailable,
    Dwarf_Addr  * lopc,
    Dwarf_Addr  * hipc,
    UNUSEDARG Dwarf_Unsigned locdesc_offset,
    struct esb_s * esbp)
{
    if (lle_value == DW_LLE_base_address) {
        esb_append_printf_u(esbp,
            "<new base address   0x%"
            DW_PR_XZEROS DW_PR_DUx
            ">",
            *hipc);
    } else if (lle_value == DW_LLE_end_of_list) {
        /* Nothing to do. */
        esb_append(esbp,"<end-of-list>");
    } else if (lle_value == DW_LLE_offset_pair) {
        if (glflags.verbose) {
            esb_append_printf_u(esbp,
                "<DW_LLE_offset_pair 0x%"
                DW_PR_XZEROS DW_PR_DUx,rawlopc);
            esb_append_printf_u(esbp,
                "           0x%"
                DW_PR_XZEROS DW_PR_DUx
                ">",rawhipc);
            esb_append_printf_i(esbp, "\n   [%2d]",llent);
        }
        esb_append_printf_u(esbp,
            "<low addr           0x%"
            DW_PR_XZEROS DW_PR_DUx,*lopc);
        esb_append_printf_u(esbp,
            " high addr 0x%"
            DW_PR_XZEROS DW_PR_DUx
            ">",*hipc);
#if 0
        if(checking) {
            loc_error_check(dbg,lopcfinal, *lopc,
                hipcfinal,*hipc, locdesc_offset,
                *base_address,
                &bError);
        }
#endif
    } else {
        struct esb_s unexp;

        esb_constructor(&unexp);
        glflags.gf_count_major_errors++;
        esb_append_printf_u(&unexp,
            "ERROR: Unexpected LLE code 0x%x"
            " (synthesized code error)",
            lle_value);
        print_error_and_continue(dbg,
            esb_get_string(&unexp),
            DW_DLV_OK, 0);
        esb_destructor(&unexp);
    }
    return DW_DLV_OK;
}
