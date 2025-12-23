/*
  Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007-2025 David Anderson. All Rights Reserved.
  Portions Copyright 2012 SN Systems Ltd. All rights reserved.

  This program is free software; you can redistribute it
  and/or modify it under the terms of version 2.1 of the
  GNU Lesser General Public License as published by the Free
  Software Foundation.

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

  You should have received a copy of the GNU Lesser General
  Public License along with this program; if not, write the
  Free Software Foundation, Inc., 51 Franklin Street - Fifth
  Floor, Boston MA 02110-1301, USA.

*/

#include <config.h>

#include <stdlib.h> /* calloc() free() */
#include <string.h> /* memset() */
#include <stdio.h> /* memset() */
#include <limits.h> /* MAX/MIN() */

#if defined(_WIN32) && defined(HAVE_STDAFX_H)
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */

#ifdef HAVE_STDINT_H
#include <stdint.h> /* uintptr_t */
#endif /* HAVE_STDINT_H */

#include "dwarf.h"
#include "libdwarf.h"
#include "dwarf_local_malloc.h"
#include "libdwarf_private.h"
#include "dwarf_base_types.h"
#include "dwarf_opaque.h"
#include "dwarf_frame.h"
#include "dwarf_alloc.h"
#include "dwarf_error.h"
#include "dwarf_util.h"

/* Structs, not pointers */
static const Dwarf_Regtable3_i         zero_reg_table_i;
static const struct Dwarf_Allreg_Args_s zero_allreg_data;

int
dwarf_iterate_fde_all_regs3(Dwarf_Fde fde,
    Dwarf_Regtable3 *regtab3,
    dwarf_iterate_fde_callback_function_type
        dwarf_callback_all_regs3,
    void            *user_data,
    Dwarf_Error     *error)
{
    int             res = 0;
    Dwarf_Addr      row_pc = 0;
    struct Dwarf_Frame_s *fde_table = 0; /*ptr to public struct */
    Dwarf_Debug     dbg = 0;
    Dwarf_Unsigned output_table_real_data_size = 0;
    Dwarf_Regtable3_i reg_table_i; /* struct, not pointer */
    struct Dwarf_Allreg_Args_s allreg_data; /* struct */
    Dwarf_Bool                 has_more_rows = FALSE;
    Dwarf_Addr                 subsequent_pc = 0;
    Dwarf_Addr      lowpc = 0;
    Dwarf_Unsigned  func_length = 0;
    Dwarf_Small    *fde_bytes;
    Dwarf_Unsigned  fde_byte_length = 0;
    Dwarf_Off       cie_offset = 0;
    Dwarf_Signed    cie_index = 0;
    Dwarf_Off       fde_offset = 0;

    allreg_data = zero_allreg_data;
    reg_table_i = zero_reg_table_i;
    fde_table   = &(fde->fd_fde_table);
    FDE_NULL_CHECKS_AND_SET_DBG(fde, dbg);
    if (!regtab3) {
        _dwarf_error_string(dbg,error,DW_DLE_DEBUGFRAME_ERROR,
            "DW_DLE_DEBUGFRAME_ERROR: dwarf_iterate_fde_all_regs3 "
            " Dwarf_Regtable3 argument NULL");
        return DW_DLV_ERROR;
    }
    if (!dwarf_callback_all_regs3) {
        _dwarf_error_string(dbg,error,DW_DLE_DEBUGFRAME_ERROR,
            "DW_DLE_DEBUGFRAME_ERROR: dwarf_iterate_fde_all_regs3 "
            " callback function pointer NULL");
        return DW_DLV_ERROR;
    }

    res = dwarf_get_fde_range(fde,&lowpc,&func_length,&fde_bytes,
        &fde_byte_length,&cie_offset,&cie_index,&fde_offset,error);
    if (res != DW_DLV_OK) {
        printf("Problem getting fde range \n");
        return res;
    }
    fde_table->fr_loc = lowpc;
    allreg_data.aa_user_data = user_data;
    allreg_data.aa_dbg = dbg;
    allreg_data.aa_callback  = dwarf_callback_all_regs3;
    allreg_data.aa_regti     = &reg_table_i;
    allreg_data.aa_regtab3   = regtab3;
    allreg_data.aa_pubregtable  = fde_table;
    allreg_data.aa_localregtab = 0; /* See dwarf_cfa_read.c */
    reg_table_i.rt3_reg_table_size =
        dbg->de_frame_reg_rules_entry_count;
    output_table_real_data_size = reg_table_i.rt3_reg_table_size;
    res = _dwarf_initialize_fde_table(dbg, fde_table,
        output_table_real_data_size,
        error);
    if (res != DW_DLV_OK) {
        return res;
    }
    /* Allocate array of internal structs to match,
        in count, what was passed in. */
    reg_table_i.rt3_rules =
        calloc(regtab3->rt3_reg_table_size,
        sizeof(Dwarf_Regtable_Entry3_i));
    if (!reg_table_i.rt3_rules) {
        _dwarf_empty_fde_table(fde_table);
        _dwarf_error_string(dbg,error,
            DW_DLE_ALLOC_FAIL,
            "Failure allocating Dwarf_Regtable_Entry3_i "
            "in dwarf_get_fde_info_for_all_regs3()");
        return DW_DLV_ERROR;
    }
    /*  This returns all data via a succession
        of callbacks to the user function via the
        dwarf_callback_all_regs3 function pointer.
        The values returned by this one call must
        be ignored other than the result and error. */
    _dwarf_rule_copy(dbg, fde_table, regtab3, &reg_table_i,
        output_table_real_data_size, &row_pc);
    res = _dwarf_get_fde_info_for_a_pc_row(fde, lowpc,
        fde_table,
        dbg->de_frame_cfa_col_number,
        &has_more_rows,&subsequent_pc,
        &allreg_data,
        error);
    _dwarf_empty_fde_table(fde_table);
    free(reg_table_i.rt3_rules);
    reg_table_i.rt3_rules = 0;
    reg_table_i.rt3_reg_table_size = 0;
    return res;
}
