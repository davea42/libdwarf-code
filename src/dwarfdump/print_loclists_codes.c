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

/*  SGI has moved from the Crittenden Lane address.  */


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

/*  Prints locentry descriptsions for
    DWARF5 DW_LKIND_loclists */

int
print_debug_loclists_linecodes(Dwarf_Debug dbg,
    Dwarf_Bool checking,
    const char *  tagname,
    const char *attrname,
    unsigned int  llent,
    Dwarf_Small   lle_value,
    Dwarf_Addr    base_address,
    Dwarf_Addr    rawlopc,
    Dwarf_Addr    rawhipc,
    Dwarf_Bool    debug_addr_unavailable,
    Dwarf_Addr    lopc,
    Dwarf_Addr    hipc,
    Dwarf_Unsigned locdesc_offset,
    struct esb_s * esbp,
    Dwarf_Bool   * bError)
{
    if (debug_addr_unavailable) {
        *bError = TRUE;
    }
    switch(lle_value) {
    case  DW_LLE_base_address:
        /* debug_addr_unavailable is not applicable here. */
        esb_append_printf_u(esbp,
            "<new base address     0x%"
            DW_PR_XZEROS DW_PR_DUx
            ">", lopc);
        /*  ASSERT: *lopc = rawlopc */
        break;
    case DW_LLE_base_addressx:
        if (debug_addr_unavailable) {
            esb_append_printf_u(esbp,
                "<DW_LLE_base_addressx 0x%"
                DW_PR_XZEROS DW_PR_DUx
                " no .debug_addr found>",rawlopc);
        } else {
            if (glflags.verbose) {
                esb_append_printf_u(esbp,
                    "<DW_LLE_base_addressx 0x%"
                    DW_PR_XZEROS DW_PR_DUx ,rawlopc);
                esb_append_printf_i(esbp, "\n   [%2d]",llent);
            }
            esb_append_printf_u(esbp,
                "<new base address     0x%"
                DW_PR_XZEROS DW_PR_DUx
                ">", lopc);
        }
        break;
    case DW_LLE_end_of_list:
        /* Nothing to do. */
        esb_append(esbp,"<end-of-list>");
        break;
    case  DW_LLE_start_length:
        if (glflags.verbose) {
            esb_append_printf_u(esbp,
                "<DW_LLE_start_length  0x%"
                DW_PR_XZEROS DW_PR_DUx ,rawlopc);
            esb_append_printf_u(esbp,
                " 0x%"
                DW_PR_XZEROS DW_PR_DUx ,rawhipc);
            esb_append_printf_i(esbp, "\n   [%2d]",llent);
        }
        esb_append_printf_u(esbp,
            "<start,end            0x%"
            DW_PR_XZEROS DW_PR_DUx ,lopc);
        esb_append_printf_u(esbp,
            " 0x%"
            DW_PR_XZEROS DW_PR_DUx
            ">",hipc);
        if (checking && !debug_addr_unavailable) {
            loc_error_check(tagname,attrname,
                lopc, rawlopc,
                hipc, rawhipc, locdesc_offset, base_address,
                bError);
        }
        break;
    case DW_LLE_startx_length:
        if (debug_addr_unavailable) {
            esb_append_printf_u(esbp,
                "<DW_LLE_startx_length 0x%"
                DW_PR_XZEROS DW_PR_DUx
                ,rawlopc);
            esb_append_printf_u(esbp,
                " 0x%"
                DW_PR_XZEROS DW_PR_DUx
                " no .debug_addr found>",rawhipc);
        } else {
            if (glflags.verbose) {
                esb_append_printf_u(esbp,
                    "<DW_LLE_startx_length 0x%"
                    DW_PR_XZEROS DW_PR_DUx
                    ,rawlopc);
                esb_append_printf_u(esbp,
                    " 0x%"
                    DW_PR_XZEROS DW_PR_DUx
                    ">",rawhipc);
                esb_append_printf_i(esbp, "\n   [%2d]",llent);
            }
            esb_append_printf_u(esbp,
                "<start,end            0x%"
                DW_PR_XZEROS DW_PR_DUx,lopc);
            esb_append_printf_u(esbp,
                " 0x%"
                DW_PR_XZEROS DW_PR_DUx
                ">",hipc);
        }
        if (checking && !debug_addr_unavailable) {
            loc_error_check(tagname,attrname,
                lopc, rawlopc,
                hipc, rawhipc, locdesc_offset, base_address,
                bError);
        }
        break;
    case  DW_LLE_offset_pair:
        /*  debug_addr_unavailable does apply becase
            that might cause base address to be invalid. */
        if (debug_addr_unavailable) {
            esb_append_printf_u(esbp,
                "<DW_LLE_startx_length 0x%"
                DW_PR_XZEROS DW_PR_DUx
                ,rawlopc);
            esb_append_printf_u(esbp,
                " 0x%"
                DW_PR_XZEROS DW_PR_DUx
                " no .debug_addr found>",rawhipc);
        } else {
            if (glflags.verbose) {
                esb_append_printf_u(esbp,
                    "<DW_LLE_offset_pair   0x%"
                    DW_PR_XZEROS DW_PR_DUx
                    ,rawlopc);
                esb_append_printf_u(esbp,
                    " 0x%"
                    DW_PR_XZEROS DW_PR_DUx
                    ">",rawhipc);
                esb_append_printf_i(esbp, "\n   [%2d]",llent);
            }
            esb_append_printf_u(esbp,
                "<start,end            0x%"
                DW_PR_XZEROS DW_PR_DUx,lopc);
            esb_append_printf_u(esbp,
                " 0x%"
                DW_PR_XZEROS DW_PR_DUx
                ">",hipc);
        }
        if (checking && !debug_addr_unavailable) {
            loc_error_check(tagname,attrname,
                lopc, rawlopc,
                hipc, rawhipc, locdesc_offset, base_address,
                bError);
        }
        break;
    case DW_LLE_start_end:
        /* debug_addr_unavailable does not apply here */
        esb_append_printf_u(esbp,
            "<start,end            0x%"
            DW_PR_XZEROS DW_PR_DUx,lopc);
        esb_append_printf_u(esbp,
            " 0x%"
            DW_PR_XZEROS DW_PR_DUx
            ">",hipc);
        if (checking && !debug_addr_unavailable) {
            loc_error_check(tagname,attrname,
                lopc, rawlopc,
                hipc, rawhipc, locdesc_offset, base_address,
                bError);
        }
        break;
    case DW_LLE_startx_endx:
        if (debug_addr_unavailable) {
            esb_append_printf_u(esbp,
                "<DW_LLE_startx_endx   0x%"
                DW_PR_XZEROS DW_PR_DUx
                ,rawlopc);
            esb_append_printf_u(esbp,
                " 0x%"
                DW_PR_XZEROS DW_PR_DUx
                " no .debug_addr found>",rawhipc);
        } else {
            if (glflags.verbose) {
                esb_append_printf_u(esbp,
                    "<DW_LLE_startx_endx   0x%"
                    DW_PR_XZEROS DW_PR_DUx
                    ,rawlopc);
                esb_append_printf_u(esbp,
                    " 0x%"
                    DW_PR_XZEROS DW_PR_DUx
                    ">",rawhipc);
                esb_append_printf_i(esbp, "\n   [%2d]",llent);
            }
            esb_append_printf_u(esbp,
                "<start,end            0x%"
                DW_PR_XZEROS DW_PR_DUx,lopc);
            esb_append_printf_u(esbp,
                " 0x%"
                DW_PR_XZEROS DW_PR_DUx
                ">",hipc);
        }
        if (checking && !debug_addr_unavailable) {
            loc_error_check(tagname,attrname,
                lopc, rawlopc,
                hipc, rawhipc, locdesc_offset, base_address,
                bError);
        }
        break;
    default: {
        struct esb_s unexp;

        esb_constructor(&unexp);
        esb_append_printf_u(&unexp,
            "ERROR: Unexpected LLE code 0x%x",
            lle_value);
        print_error_and_continue(dbg,
            esb_get_string(&unexp),
            DW_DLV_OK, 0);
        esb_destructor(&unexp);
        }
        break;
    }
    return DW_DLV_OK;
}
