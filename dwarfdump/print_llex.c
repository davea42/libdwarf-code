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

/* Prints locentry descriptsions for DW_LKIND_GNU_exp_list */

int
print_llex_linecodes(Dwarf_Debug dbg,
    Dwarf_Bool    checking,
    Dwarf_Small   lle_value,
    Dwarf_Addr  * base_address,
    Dwarf_Addr  * lopc,
    Dwarf_addr  * hipc,
    Dwarf_Unsigned locdesc_offset,
    struct esb_s * ebsp,
    Dwarf_Error  * llerr)
{
    Dwarf_Unsigned lopcfinal = 0;
    Dwarf_Unsigned hipcfinal = 0;


    if (lle_value == DW_LLEX_base_address_selection_entry) {
        /*  (0xffffffff,addr), use specific address
            (current PU address) */
        Dwarf_Addr realaddr = 0;
        {
            /*  hipc is index of a slot in .debug_addr section.
                which contains base_address. */
            int res = dwarf_debug_addr_index_to_addr(die,
                hipc,&realaddr,llerr);
            if(res == DW_DLV_OK) {
                *base_address = realaddr;
            } else if(res == DW_DLV_ERROR) {
                glflags.gf_count_major_errors++;
                esb_append_printf_u(esbp,
                            "<debug_addr index 0x%"
                    DW_PR_XZEROS DW_PR_DUx,hipc);
                esb_append_printf_s(esbp,
                    " %s>",
                    adexplain(dwarf_errno(*llerr),
                    "base-address-unavailable"));
                *base_address = 0;
                print_error_and_continue(dbg,
                    "ERROR printing addr index val fails",
                    res,*llerr);
                return res;
            } else { /* DW_DLV_NO_ENTRY */
                esb_append_printf_u(esbp,
                    "<debug_addr index 0x%"
                    DW_PR_XZEROS DW_PR_DUx
                    " no-entry finding index >",hipc);
                /* Cannot find .debug_addr */
                *base_address = 0;
            }
            esb_append_printf_u(esbp,
                "<index to debug_addr : 0x%"
                DW_PR_XZEROS DW_PR_DUx,hipc);
            esb_append_printf_u(esbp,
                " new base address 0x%"
                DW_PR_XZEROS DW_PR_DUx
                ">", *base_address);
        }
    } else if (lle_value == DW_LLEX_end_of_list_entry) {
        /* Nothing to do. */
        esb_append(esbp,"<end-of-list>");
    } else if (lle_value == DW_LLEX_start_length_entry) {
        int foundaddr = FALSE;
        {
            Dwarf_Addr realaddr = 0;
            Dwarf_Addr slotindex = *lopc;
            /*  start (lopc) is index of a slot
                in .debug_addr section. */
            int res = dwarf_debug_addr_index_to_addr(die,
                *lopc,&realaddr,llerr);
            if(res == DW_DLV_OK) {
                *lopc = realaddr;
                foundaddr = TRUE;
            } else if(res == DW_DLV_ERROR) {
               esb_append_printf_u(esbp,
                    "<debug_addr index 0x%"
                    DW_PR_XZEROS DW_PR_DUx,*
                esb_append_printf_s(esbp,
                    " %s>",
                    adexplain(dwarf_errno(*llerr),
                    "start-address-unavailable"));
                glflags.gf_count_major_errors++;
                DROP_ERROR_INSTANCE(dbg,res,*llerr);
            } else {
                esb_append_printf_u(esbp,
                    "<debug_addr index 0x%"
                    DW_PR_XZEROS DW_PR_DUx
                    " no-entry finding start index >",*
                /* Cannot find .debug_addr */
                *lopc = 0;
            }
            esb_append_printf_u(esbp,
                "<start-length index to debug_addr : 0x%"
                DW_PR_XZEROS DW_PR_DUx,slotindex);
            esb_append_printf_u(esbp,
                " addr  0x%"
                DW_PR_XZEROS DW_PR_DUx,realaddr);
            esb_append_printf_u(esbp,
                " length 0x%"
                DW_PR_XZEROS DW_PR_DUx
                "> ",hipc);
        }
        lopcfinal = *lopc;
        hipcfinal = lopcfinal + *hipc;
        if (checking && foundaddr) {
            /*  bError is a boolean, not a Dwarf_Error,
                here and below. */
            loc_error_check(dbg,lopcfinal, *lopc,
                hipcfinal, *hipc, locdesc_offset,
                *base_address,
                &bError);
        }
    } else if (lle_value == DW_LLEX_offset_pair_entry) {
        /* Same for both loclist_source. */
        lopcfinal = *lopc + *base_address;
        hipcfinal = *hipc + *base_address;
        esb_append_printf_u(esbp,
            "< offset pair low-off : 0x%"
            DW_PR_XZEROS DW_PR_DUx,*lopc);
        esb_append_printf_u(esbp,
            " addr  0x%"
            DW_PR_XZEROS DW_PR_DUx,lopcfinal);
        esb_append_printf_u(esbp,
            " high-off  0x%"
            DW_PR_XZEROS DW_PR_DUx,*hipc);
        esb_append_printf_u(esbp,
            " addr 0x%"
            DW_PR_XZEROS DW_PR_DUx
            ">",hipcfinal);
        if(checking) {
            loc_error_check(dbg,lopcfinal, *lopc,
                hipcfinal,*hipc, locdesc_offset,
                *base_address,
                &bError);
        }
    } else if (lle_value == DW_LLEX_start_end_entry) {
        int foundaddr = FALSE;
        /* These are NOT relative to *base_address */
        {
            /*  indices in .debug_addr of start and end
                addresses. */
            Dwarf_Addr reallo = 0;
            Dwarf_Addr realhi = 0;
            /* start is index of a slot in .debug_addr section. */
            int res = dwarf_debug_addr_index_to_addr(die,
                *lopc,&reallo,llerr);
            if(res == DW_DLV_OK) {
                lopcfinal = reallo;
                foundaddr = TRUE;
            } else if(res == DW_DLV_ERROR) {
                glflags.gf_count_major_errors++;
                esb_append_printf_u(esbp,
                    "<debug_addr index 0x%"
                    DW_PR_XZEROS DW_PR_DUx,lopc);
                esb_append_printf_s(esbp,
                    " %s>",
                    adexplain(dwarf_errno(*llerr),
                    "start-address-unavailable"));
                glflags.gf_count_major_errors++;
                DROP_ERROR_INSTANCE(dbg,res,*llerr);
            } else {
                esb_append_printf_u(esbp,
                    "<debug_addr index 0x%"
                    DW_PR_XZEROS DW_PR_DUx
                    " error finding start index >",lopc);
                /* Cannot find .debug_addr */
                lopcfinal = 0;
            }
            res = dwarf_debug_addr_index_to_addr(die,
                hipc,&realhi,llerr);
            if(res == DW_DLV_OK) {
                hipcfinal = realhi;
            } else if(res == DW_DLV_ERROR) {
                glflags.gf_count_major_errors++;
                esb_append_printf_u(esbp,
                    "<debug_addr index 0x%"
                    DW_PR_XZEROS DW_PR_DUx,hipc);
                esb_append_printf_s(esbp,
                    " %s>",
                adexplain(dwarf_errno(*llerr),
                    "end-address-unavailable"));
                foundaddr = FALSE;
                glflags.gf_count_major_errors++;
                DROP_ERROR_INSTANCE(dbg,res,*llerr);
            } else {
                esb_append_printf_u(esbp,
                    "<debug_addr index 0x%"
                    DW_PR_XZEROS DW_PR_DUx
                     " problem-finding-end-address >",hipc);
                /* Cannot find .debug_addr */
                hipcfinal = 0;
                foundaddr = FALSE;
             }
             esb_append_printf_u(esbp,
                 "< start-end low-index : 0x%"
                 DW_PR_XZEROS DW_PR_DUx,*lopc);
             esb_append_printf_u(esbp,
                 " addr  0x%"
                 DW_PR_XZEROS DW_PR_DUx,lopcfinal);
             esb_append_printf_u(esbp,
                 " high-index  0x%"
                 DW_PR_XZEROS DW_PR_DUx,*hipc);
             esb_append_printf_u(esbp,
                 " addr 0x%"
                 DW_PR_XZEROS DW_PR_DUx
                 ">",hipcfinal);
        }
        if (checking && foundaddr) {
            loc_error_check(dbg,lopcfinal, *lopc,
                hipcfinal, *hipc, locdesc_offset, 0,
                &bError);
        }
    } else {
        struct esb_s unexp;

        esb_constructor(&unexp);
        glflags.gf_count_major_errors++;
        esb_append_printf_u(&unexp,
            "ERROR: Unexpected LLEX code 0x%x",
            lle_value);
        print_error_and_continue(dbg,
            esb_get_string(&unexp),
            DW_DLV_OK, 0);
        esb_destructor(&unexp);
    }
    return DW_DLV_OK;
}
