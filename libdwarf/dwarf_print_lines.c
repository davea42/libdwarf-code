/*

  Copyright (C) 2000,2002,2004,2005,2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007-2013 David Anderson. All Rights Reserved.
  Portions Copyright 2012 SN Systems Ltd. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, write the Free Software
  Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston MA 02110-1301,
  USA.

*/

#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include <time.h>
#include "dwarf_line.h"

/*  The line table set of registers.
    Using names from the DWARF documentation
    but preceded by lr_.  */
struct Dwarf_Line_Registers_s {
    Dwarf_Addr lr_address;        /* DWARF2 */
    Dwarf_Word lr_file ;          /* DWARF2 */
    Dwarf_Word lr_line ;          /* DWARF2 */
    Dwarf_Word lr_column ;        /* DWARF2 */
    Dwarf_Bool lr_is_stmt;        /* DWARF2 */
    Dwarf_Bool lr_basic_block;    /* DWARF2 */
    Dwarf_Bool lr_end_sequence;   /* DWARF2 */
    Dwarf_Bool lr_prologue_end;   /* DWARF3 */
    Dwarf_Bool lr_epilogue_begin; /* DWARF3 */
    Dwarf_Small lr_isa;           /* DWARF3 */
    Dwarf_Unsigned lr_op_index;      /* DWARF4 */
    Dwarf_Unsigned lr_discriminator; /* DWARF4 */
    Dwarf_Unsigned lr_context;       /* EXPERIMENTAL */
    Dwarf_Unsigned lr_subprogram;     /* EXPERIMENTAL */
};

/* To set to initial conditions, copy this in. */
struct Dwarf_Line_Registers_s default_reg_values = {
    /* Dwarf_Addr lr_address */ 0,
    /* Dwarf_Word lr_file */ 1,
    /* Dwarf_Word lr_line */  1,
    /* Dwarf_Word lr_column */  0,
    /* Dwarf_Bool lr_is_stmt */  false,
    /* Dwarf_Bool lr_basic_block */  false,
    /* Dwarf_Bool lr_end_sequence */  false,
    /* Dwarf_Bool lr_prologue_end */  false,
    /* Dwarf_Bool lr_epilogue_begin */  false,
    /* Dwarf_Small lr_isa */  0,
    /* Dwarf_Unsigned lr_op_index  */  0,
    /* Dwarf_Unsigned lr_discriminator */  0,
    /* Dwarf_Unsigned lr_context */  0,
    /* Dwarf_Unsigned lr_subprogram */  0,
};


/* FIXME Need to add prologue_end epilogue_begin isa fields. */
static void
print_line_header(Dwarf_Debug dbg,
    Dwarf_Bool is_single_tab,
    Dwarf_Bool is_actuals_tab)
{
if (!is_single_tab) {
    if (is_actuals_tab) {
        dwarf_printf(dbg,"\nActuals Table\n");
dwarf_printf(dbg,
"                                                         b e\n"
"                                                         l s\n"
"                                                         c e\n"
" section    op                                           k q\n"
" offset     code                address/index    row isa ? ?\n");
        return;
/* FIXME */
    } else {
        dwarf_printf(dbg,"\nLogicals Table\n");
dwarf_printf(dbg,
"                                                                              spe\n" 
"                                                                              trp\n"
"                                                                              moi\n"
" section     row  op                                                          tll\n"
" offset      num  code                address/indx fil lne col disc cntx subp ???\n");
        return;
/* FIXME */
    }
}
/* Ugly indenting, but makes lines shorter to see them better. */
dwarf_printf(dbg,
"                                                         s b e p e i d\n"
"                                                         t l s r p s i\n"
"                                                         m c e o i a s\n"
" section    op                                       col t k q l l   c\n"
" offset     code               address     file line umn ? ? ? ? ? \n");
}

static void
print_line_detail(
    Dwarf_Debug dbg,
    const char *prefix,
    int opcode,
    unsigned curr_line,
    struct Dwarf_Line_Registers_s * regs,
    Dwarf_Bool is_single_table, Dwarf_Bool is_actuals_table)
{
    if(!is_single_table && is_actuals_table) {
        dwarf_printf(dbg,
            "%-15s %3d 0x%" DW_PR_XZEROS DW_PR_DUx 
            "/%01u"
            " %5lu"  /* lr_line, really logical row */ 
            " %5d"   /* isa */
            "%2d"
            "%2d\n",
            prefix,
            (int) opcode,
            (Dwarf_Unsigned) regs->lr_address,
            (unsigned) regs->lr_op_index,
            (unsigned long) regs->lr_line, /*logical row */
            regs->lr_isa,
            (int) regs->lr_basic_block, 
            (int) regs->lr_end_sequence);
        return;
    }
    if(!is_single_table && !is_actuals_table) {
        dwarf_printf(dbg,
            "[%3d] "  /* row number */
            "%-15s %3d 0x%" DW_PR_XZEROS DW_PR_DUx "/%01u"
            " %2lu %4lu %1lu",
            curr_line,
            prefix,
            (int) opcode,
            (Dwarf_Unsigned) regs->lr_address,
            (unsigned) regs->lr_op_index,
            (unsigned long) regs->lr_file, 
            (unsigned long) regs->lr_line, 
            (unsigned long) regs->lr_column);
        if (regs->lr_discriminator ||
            regs->lr_prologue_end ||
            regs->lr_epilogue_begin ||
            regs->lr_isa ||
            regs->lr_is_stmt ||
            regs->lr_context ||
            regs->lr_subprogram) {
            dwarf_printf(dbg,
                "   x%02" DW_PR_DUx , regs->lr_discriminator); /* DWARF4 */
            dwarf_printf(dbg,
                "  x%02" DW_PR_DUx , regs->lr_context); /* EXPERIMENTAL */
            dwarf_printf(dbg,
                "  x%02" DW_PR_DUx , regs->lr_subprogram); /* EXPERIMENTAL */
            dwarf_printf(dbg,
                "  %1d", (int) regs->lr_is_stmt);
            dwarf_printf(dbg,
                "%1d", regs->lr_prologue_end); /* DWARF3 */
            dwarf_printf(dbg,
                "%1d", regs->lr_epilogue_begin); /* DWARF3 */
        }
        dwarf_printf(dbg, "\n");
        return;
    }
    /*  In the first quoted line below:   
        3d looks better than 2d, but best to do that as separate
        change and test from two-level-line-tables.  */
    dwarf_printf(dbg,
        "%-15s %2d 0x%" DW_PR_XZEROS DW_PR_DUx " "
        "%2lu   %4lu %2lu   %1d %1d %1d",
        prefix,
        (int) opcode,
        (Dwarf_Unsigned) regs->lr_address,
        (unsigned long) regs->lr_file,
        (unsigned long) regs->lr_line,
        (unsigned long) regs->lr_column,
        (int) regs->lr_is_stmt, 
        (int) regs->lr_basic_block, 
        (int) regs->lr_end_sequence);
    if (regs->lr_discriminator || 
        regs->lr_prologue_end || 
        regs->lr_epilogue_begin || 
        regs->lr_isa) {
        dwarf_printf(dbg,
            " %1d", regs->lr_prologue_end); /* DWARF3 */
        dwarf_printf(dbg,
            " %1d", regs->lr_epilogue_begin); /* DWARF3 */
        dwarf_printf(dbg,
            " %1d", regs->lr_isa); /* DWARF3 */
        dwarf_printf(dbg,
            " 0x%" DW_PR_DUx , regs->lr_discriminator); /* DWARF4 */
    }
    dwarf_printf(dbg, "\n");
}


/*  This prints the details of a statement program.
    The two-level table operators are not yet fully handled.  */
static int 
print_statement_program(Dwarf_Debug dbg,
    Dwarf_Die die,
    Dwarf_CU_Context cu_context,
    struct Line_Table_Prefix_s *prefix,
    Dwarf_Small *orig_line_ptr,
    Dwarf_Small *line_ptr,
    Dwarf_Small *line_ptr_end,
    Dwarf_Bool  is_single_table,
    Dwarf_Bool  is_actuals_table,
    Dwarf_Error *error,
    int * err_count_out)
{
    struct Dwarf_Line_Registers_s regs;
    Dwarf_Word leb128_num=0;
    Dwarf_Word leb128_length=0;
    Dwarf_Sword advance_line=0;
    /*  This is the operand of the latest fixed_advance_pc extended
        opcode. */
    Dwarf_Half fixed_advance_pc=0;

    /*  For each new table row in the Logicals table
        we want to know the row number so we can print it.  
        Things refer to it.
        The rows are numbered starting from 1, so we 
        increment before printing.
    */
    unsigned curr_line = 0;

    
    regs = default_reg_values;
    /*  Initialize the part of the state machine dependent on the
        prefix.  */
    regs.lr_is_stmt = prefix->pf_default_is_stmt;
    /* Start of statement program.  */
    while (line_ptr < line_ptr_end) {
        int type = 0;
        Dwarf_Small opcode = 0;

        /* Here we print the table area byte offset. */
        dwarf_printf(dbg,
            " [0x%06" DW_PR_DSx "] ",
            (Dwarf_Signed) (line_ptr - orig_line_ptr));
        opcode = *(Dwarf_Small *) line_ptr;
        line_ptr++;
        /* 'type' is the output */
        WHAT_IS_OPCODE(type, opcode, prefix->pf_opcode_base,
            prefix->pf_opcode_length_table, line_ptr,
            prefix->pf_std_op_count);
        if (type == LOP_DISCARD) {
            int oc = 0;
            int opcnt = prefix->pf_opcode_length_table[opcode];

            dwarf_printf(dbg,
                "*** DWARF CHECK: DISCARD standard opcode %d "
                "with %d operands: "
                "not understood.", opcode, opcnt);
            *err_count_out += 1;
            for (oc = 0; oc < opcnt; oc++) {
                /*  Read and discard operands we don't
                    understand.
                    Arbitrary choice of unsigned read.
                    Signed read would work as well.  */
                Dwarf_Unsigned utmp2 = 0;

                DECODE_LEB128_UWORD(line_ptr, utmp2);
                dwarf_printf(dbg,
                    " %" DW_PR_DUu
                    " (0x%" DW_PR_XZEROS DW_PR_DUx ")",
                    (Dwarf_Unsigned) utmp2,
                    (Dwarf_Unsigned) utmp2);
            }
            dwarf_printf(dbg,
                "***\n");
            /* Do nothing, necessary ops done */
        } else if (type == LOP_SPECIAL) {
            /*  This op code is a special op in the object, no matter
                that it might fall into the standard op range in this
                compile Thatis, these are special opcodes between
                special_opcode_base and MAX_LINE_OP_CODE.  (including
                special_opcode_base and MAX_LINE_OP_CODE) */
            char special[50];
            Dwarf_Unsigned operation_advance = 0;
            unsigned origop = opcode;

            opcode = opcode - prefix->pf_opcode_base;
            operation_advance = (opcode / prefix->pf_line_range);
            if (prefix->pf_maximum_ops_per_instruction < 2) {
                regs.lr_address = regs.lr_address + 
                    (prefix->pf_minimum_instruction_length *
                    operation_advance);
            } else {
                regs.lr_address = regs.lr_address + 
                    (prefix->pf_minimum_instruction_length *
                    ((regs.lr_op_index + operation_advance)/
                    prefix->pf_maximum_ops_per_instruction));
                regs.lr_op_index = (regs.lr_op_index +operation_advance)%
                    prefix->pf_maximum_ops_per_instruction;
            }
            regs.lr_line = regs.lr_line + prefix->pf_line_base +
                opcode % prefix->pf_line_range;
            sprintf(special, "Specialop %3u", origop);
            curr_line++;
            print_line_detail(dbg,special,
                opcode,curr_line, &regs,is_single_table, is_actuals_table);
            regs.lr_basic_block = false;
            regs.lr_prologue_end = false;
            regs.lr_epilogue_begin = false;
            regs.lr_discriminator = 0;
        } else if (type == LOP_STANDARD) {
            switch (opcode) {

            case DW_LNS_copy:{
                curr_line++;
                print_line_detail(dbg,"DW_LNS_copy",
                    opcode,curr_line, &regs,is_single_table, is_actuals_table);
                regs.lr_basic_block = false;
                regs.lr_prologue_end = false;
                regs.lr_epilogue_begin = false;
                regs.lr_discriminator = 0;
                }
                break;

            case DW_LNS_advance_pc:{
                Dwarf_Unsigned utmp2 = 0;

                DECODE_LEB128_UWORD(line_ptr, utmp2);
                dwarf_printf(dbg,
                    "DW_LNS_advance_pc val %"
                    DW_PR_DSd " 0x%"
                    DW_PR_XZEROS DW_PR_DUx "\n",
                    (Dwarf_Signed) (Dwarf_Word) utmp2,
                    (Dwarf_Unsigned) (Dwarf_Word) utmp2);
                leb128_num = (Dwarf_Word) utmp2;
                regs.lr_address = regs.lr_address +
                    prefix->pf_minimum_instruction_length * leb128_num;
                }
                break;
            case DW_LNS_advance_line:{
                Dwarf_Signed stmp = 0;

                DECODE_LEB128_SWORD(line_ptr, stmp);
                advance_line = (Dwarf_Sword) stmp;
                dwarf_printf(dbg,
                    "DW_LNS_advance_line val %" DW_PR_DSd " 0x%"
                    DW_PR_XZEROS DW_PR_DSx "\n",
                    (Dwarf_Signed) advance_line,
                    (Dwarf_Signed) advance_line);
                regs.lr_line = regs.lr_line + advance_line;
                }
                break;

            case DW_LNS_set_file:{
                Dwarf_Unsigned utmp2 = 0;

                DECODE_LEB128_UWORD(line_ptr, utmp2);
                regs.lr_file = (Dwarf_Word) utmp2;
                dwarf_printf(dbg,
                    "DW_LNS_set_file  %ld\n", (long) regs.lr_file);
                }
                break;
            case DW_LNS_set_column:{
                Dwarf_Unsigned utmp2 = 0;

                DECODE_LEB128_UWORD(line_ptr, utmp2);
                regs.lr_column = (Dwarf_Word) utmp2;
                dwarf_printf(dbg,
                    "DW_LNS_set_column val %" DW_PR_DSd " 0x%"
                    DW_PR_XZEROS DW_PR_DSx "\n",
                    (Dwarf_Signed) regs.lr_column, 
                    (Dwarf_Signed) regs.lr_column);
                }
                break;
            case DW_LNS_negate_stmt:{
                regs.lr_is_stmt = !regs.lr_is_stmt;
                dwarf_printf(dbg,
                    "DW_LNS_negate_stmt\n");
                }
                break;
            case DW_LNS_set_basic_block:{
                dwarf_printf(dbg,
                    "DW_LNS_set_basic_block\n");
                regs.lr_basic_block = true;
                }
                break;

            case DW_LNS_const_add_pc:{
                opcode = MAX_LINE_OP_CODE - prefix->pf_opcode_base;
                if (prefix->pf_maximum_ops_per_instruction < 2) {
                    Dwarf_Unsigned operation_advance =
                        (opcode / prefix->pf_line_range);
                    regs.lr_address = regs.lr_address +
                        prefix->pf_minimum_instruction_length *
                            operation_advance;
                } else {
                    Dwarf_Unsigned operation_advance =
                        (opcode / prefix->pf_line_range);
                    regs.lr_address = regs.lr_address + 
                        prefix->pf_minimum_instruction_length *
                        ((regs.lr_op_index + operation_advance)/
                        prefix->pf_maximum_ops_per_instruction);
                    regs.lr_op_index = (regs.lr_op_index +operation_advance)%
                        prefix->pf_maximum_ops_per_instruction;
                }

                dwarf_printf(dbg,
                    "DW_LNS_const_add_pc new address 0x%"
                    DW_PR_XZEROS DW_PR_DSx "\n",
                    (Dwarf_Signed) regs.lr_address);
                }
                break;
            case DW_LNS_fixed_advance_pc:{
                READ_UNALIGNED(dbg, fixed_advance_pc, Dwarf_Half,
                    line_ptr, sizeof(Dwarf_Half));
                line_ptr += sizeof(Dwarf_Half);
                regs.lr_address = regs.lr_address + fixed_advance_pc;
                dwarf_printf(dbg,
                    "DW_LNS_fixed_advance_pc val %" DW_PR_DSd
                    " 0x%" DW_PR_XZEROS DW_PR_DSx
                    " new address 0x%" DW_PR_XZEROS DW_PR_DSx "\n",
                    (Dwarf_Signed) fixed_advance_pc,
                    (Dwarf_Signed) fixed_advance_pc,
                    (Dwarf_Signed) regs.lr_address);
                regs.lr_op_index = 0;
                }
                break;
            case DW_LNS_set_prologue_end:{
                regs.lr_prologue_end = true;
                dwarf_printf(dbg,
                    "DW_LNS_set_prologue_end set true.\n");
                }
                break;
                /* New in DWARF3 */
            case DW_LNS_set_epilogue_begin:{
                regs.lr_epilogue_begin = true;
                dwarf_printf(dbg,
                    "DW_LNS_set_epilogue_begin set true.\n");
                }
                break;

                /* New in DWARF3 */
            case DW_LNS_set_isa:{
                Dwarf_Unsigned utmp2;

                DECODE_LEB128_UWORD(line_ptr, utmp2);
                regs.lr_isa = utmp2;
                dwarf_printf(dbg,
                    "DW_LNS_set_isa new value 0x%"
                    DW_PR_XZEROS DW_PR_DUx ".\n",
                    (Dwarf_Unsigned) utmp2);
                if (regs.lr_isa != utmp2) {
                    /*  The value of the isa did not fit in our
                        local so we record it wrong. declare an
                        error. */
                    _dwarf_error(dbg, error,
                        DW_DLE_LINE_NUM_OPERANDS_BAD);
                    return (DW_DLV_ERROR);
                }
                }
                break;
            /*  The following is both DW_LNS_set_subprogram and DW_LNS_set_subprogram
                which is not the sort of thing a final Standard would do. */
            case DW_LNS_set_subprogram:{ /* EXPERIMENTAL_LINE_TABLES_VERSION */
                if (is_actuals_table) {
                    /* DW_LNS_set_address_from_logical */
                    Dwarf_Signed stmp = 0;

                    DECODE_LEB128_SWORD(line_ptr, stmp);
                    advance_line = (Dwarf_Sword) stmp;
                    dwarf_printf(dbg,"DW_LNS_set_address_from_logical "
                        "%" DW_PR_DSd " 0x%" DW_PR_XZEROS DW_PR_DSx ".\n",
                        stmp,stmp);
                    dwarf_printf(dbg,"    address/op_index not set. FIXME\n");
                    /* FIXME: incomplete register setting. */
                    regs.lr_line = regs.lr_line + advance_line;
                    
                } else {
                    /* DW_LNS_set_subprogram */
                    Dwarf_Unsigned utmp2 = 0;

                    regs.lr_context = 0;
                    DECODE_LEB128_UWORD(line_ptr, utmp2);
                    regs.lr_subprogram = (Dwarf_Word) utmp2;
                    dwarf_printf(dbg,"DW_LNS_set_subprogram "
                        "%" DW_PR_DSd " 0x%" DW_PR_XZEROS DW_PR_DSx "\n",
                        (Dwarf_Signed)utmp2,(Dwarf_Signed)utmp2);
                    /* FIXME: incomplete register setting. */
                }
                break;
                }
            case DW_LNS_inlined_call:{ /* EXPERIMENTAL_LINE_TABLES_VERSION */
                Dwarf_Signed stmp = 0;
                Dwarf_Signed utmp = 0;

                DECODE_LEB128_SWORD(line_ptr, stmp);
                /* regs.lr_context = line_count + stmp; */
                DECODE_LEB128_UWORD(line_ptr, utmp);
                regs.lr_subprogram = utmp;
                dwarf_printf(dbg,"DW_LNS_inlined_call "
                    "%" DW_PR_DSd " (0x%" DW_PR_XZEROS DW_PR_DSx "),"
                    "%" DW_PR_DSd " (0x%" DW_PR_XZEROS DW_PR_DSx ")\n",
                    stmp,stmp,
                    (Dwarf_Signed)utmp,(Dwarf_Signed)utmp);
                }
            case DW_LNS_pop_context:{ /* EXPERIMENTAL_LINE_TABLES_VERSION */
                dwarf_printf(dbg,"DW_LNS_pop_context. Register values not set.\n");
                /* FIXME: incomplete register setting. */
                }
                break;
            } /* end switch */
        } else if (type == LOP_EXTENDED) {
            Dwarf_Unsigned utmp3 = 0;
            Dwarf_Word instr_length = 0;
            Dwarf_Small ext_opcode = 0;

            DECODE_LEB128_UWORD(line_ptr, utmp3);
            instr_length = (Dwarf_Word) utmp3;
            ext_opcode = *(Dwarf_Small *) line_ptr;
            line_ptr++;
            switch (ext_opcode) {

            case DW_LNE_end_sequence:{
                regs.lr_end_sequence = true;
                curr_line++;
                print_line_detail(dbg,"DW_LNE_end_sequence extended",
                    opcode, curr_line, &regs,is_single_table, is_actuals_table);
                regs.lr_address = 0;
                regs.lr_file = 1;
                regs.lr_line = 1;
                regs.lr_column = 0;
                regs.lr_is_stmt = prefix->pf_default_is_stmt;
                regs.lr_basic_block = false;
                regs.lr_end_sequence = false;
                regs.lr_prologue_end = false;
                regs.lr_epilogue_begin = false;
                regs.lr_isa = 0;
                regs.lr_discriminator = 0;
                regs.lr_op_index = 0;
                }
                break;
            case DW_LNE_set_address:{
                READ_UNALIGNED(dbg,regs.lr_address, Dwarf_Addr,
                    line_ptr,
                    cu_context->cc_address_size);

                line_ptr += cu_context->cc_address_size;
                dwarf_printf(dbg,
                    "DW_LNE_set_address address 0x%"
                    DW_PR_XZEROS DW_PR_DUx "\n",
                    (Dwarf_Unsigned) regs.lr_address);

                regs.lr_op_index = 0;
                }
                break;
            case DW_LNE_define_file:{
                Dwarf_Unsigned di = 0;
                Dwarf_Unsigned tlm = 0;
                Dwarf_Unsigned fl = 0;

                Dwarf_Small *fn = (Dwarf_Small *) line_ptr;
                line_ptr = line_ptr + strlen((char *) line_ptr) + 1;
                di = _dwarf_decode_u_leb128(line_ptr,
                    &leb128_length);
                line_ptr = line_ptr + leb128_length;
                tlm = _dwarf_decode_u_leb128(line_ptr,
                    &leb128_length);
                line_ptr = line_ptr + leb128_length;
                fl = _dwarf_decode_u_leb128(line_ptr,
                    &leb128_length);
                line_ptr = line_ptr + leb128_length;

                dwarf_printf(dbg,
                    "DW_LNE_define_file %s \n", fn);
                dwarf_printf(dbg,
                    "    dir index %d\n", (int) di);
                {
                    time_t tt3 = (time_t) tlm;

                    /* ctime supplies newline */
                    dwarf_printf(dbg,
                        "    last time 0x%x %s",
                        (unsigned) tlm, ctime(&tt3));
                }
                dwarf_printf(dbg,
                    "    file length %ld 0x%lx\n",
                    (long) fl, (unsigned long) fl);

                }
                break;
            case DW_LNE_set_discriminator:{
                /* new in DWARF4 */
                Dwarf_Unsigned utmp2 = 0;

                DECODE_LEB128_UWORD(line_ptr, utmp2);
                regs.lr_discriminator = (Dwarf_Word) utmp2;
                dwarf_printf(dbg,
                    "DW_LNE_set_discriminator 0x%"
                    DW_PR_XZEROS DW_PR_DUx "\n",utmp2);
                }
                break;

            default:{
                /*  This is an extended op code we do not know about,
                    other than we know now many bytes it is
                    (and the op code and the bytes of operand). */

                Dwarf_Unsigned remaining_bytes = instr_length -1;
                if (instr_length < 1 || remaining_bytes > DW_LNE_LEN_MAX) {
                    _dwarf_error(dbg, error,
                        DW_DLE_LINE_EXT_OPCODE_BAD);
                    return (DW_DLV_ERROR);
                }
                dwarf_printf(dbg,
                    "DW_LNE extended op 0x%x ",ext_opcode);
                dwarf_printf(dbg,
                    "Bytecount: %" DW_PR_DUu , (Dwarf_Unsigned)instr_length);
                if (remaining_bytes > 0) {
                    dwarf_printf(dbg,
                        " linedata: 0x");
                    while (remaining_bytes > 0) {
                        dwarf_printf(dbg,
                            "%02x",(unsigned char)(*(line_ptr)));
                        line_ptr++;
                        remaining_bytes--;
                    }
                }
                dwarf_printf(dbg,
                    "\n");
            }
            break;
            } /* Dnd switch */
        }
    }
    return (DW_DLV_OK);
}



/*  return DW_DLV_OK if ok. else DW_DLV_NO_ENTRY or DW_DLV_ERROR
    If err_count_out is non-NULL, this is a special 'check'
    call.  */
static int
_dwarf_internal_printlines(Dwarf_Die die, 
    Dwarf_Error * error,
    int * err_count_out, 
    int only_line_header)
{
    /*  This pointer is used to scan the portion of the .debug_line
        section for the current cu. */
    Dwarf_Small *line_ptr = 0;
    Dwarf_Small *orig_line_ptr = 0;

    /*  This points to the last byte of the .debug_line portion for the
        current cu. */
    Dwarf_Small *line_ptr_end = 0;

    /*  Pointer to a DW_AT_stmt_list attribute in case it exists in the
        die. */
    Dwarf_Attribute stmt_list_attr = 0;

    /*  Pointer to DW_AT_comp_dir attribute in die. */
    Dwarf_Attribute comp_dir_attr = 0;

    /*  Pointer to name of compilation directory. */
    Dwarf_Small *comp_dir = NULL;

    /*  Offset into .debug_line specified by a DW_AT_stmt_list
        attribute. */
    Dwarf_Unsigned line_offset = 0;

    struct Line_Table_Prefix_s prefix;

    Dwarf_Sword i=0;
    Dwarf_Word u=0;

    /*  This is the current opcode read from the statement program. */
    Dwarf_Small opcode=0;


    /*  These variables are used to decode leb128 numbers. Leb128_num
        holds the decoded number, and leb128_length is its length in
        bytes. */
    Dwarf_Word leb128_num=0;
    Dwarf_Word leb128_length=0;
    Dwarf_Sword advance_line=0;
    Dwarf_Half attrform = 0;
    /*  This is the operand of the latest fixed_advance_pc extended
        opcode. */
    Dwarf_Half fixed_advance_pc=0;

    /*  In case there are wierd bytes 'after' the line table
        prologue this lets us print something. This is a gcc
        compiler bug and we expect the bytes count to be 12.  */
    Dwarf_Small* bogus_bytes_ptr = 0;
    Dwarf_Unsigned bogus_bytes_count = 0;


    /* The Dwarf_Debug this die belongs to. */
    Dwarf_Debug dbg=0;
    Dwarf_CU_Context context = 0;
    int resattr = DW_DLV_ERROR;
    int lres =    DW_DLV_ERROR;
    int res  =    DW_DLV_ERROR;

    /* ***** BEGIN CODE ***** */

    if (error != NULL) {
        *error = NULL;
    }

    CHECK_DIE(die, DW_DLV_ERROR);
    context = die->di_cu_context;
    dbg = context->cc_dbg;

    res = _dwarf_load_section(dbg, &dbg->de_debug_line,error);
    if (res != DW_DLV_OK) {
        return res;
    }
    if (!dbg->de_debug_line.dss_size) {
        return (DW_DLV_NO_ENTRY);
    }


    resattr = dwarf_attr(die, DW_AT_stmt_list, &stmt_list_attr, error);
    if (resattr != DW_DLV_OK) {
        return resattr;
    }


    /*  The list of relevant FORMs is small.
        DW_FORM_data4, DW_FORM_data8, DW_FORM_sec_offset
    */
    lres = dwarf_whatform(stmt_list_attr,&attrform,error);
    if (lres != DW_DLV_OK) {
        return lres;
    }
    if (attrform != DW_FORM_data4 && attrform != DW_FORM_data8 &&
        attrform != DW_FORM_sec_offset ) {
        _dwarf_error(dbg, error, DW_DLE_LINE_OFFSET_BAD);
        return (DW_DLV_ERROR);
    }
    lres = dwarf_global_formref(stmt_list_attr, &line_offset, error);
    if (lres != DW_DLV_OK) {
        return lres;
    }

    if (line_offset >= dbg->de_debug_line.dss_size) {
        _dwarf_error(dbg, error, DW_DLE_LINE_OFFSET_BAD);
        return (DW_DLV_ERROR);
    }
    orig_line_ptr = dbg->de_debug_line.dss_data;
    line_ptr = dbg->de_debug_line.dss_data + line_offset;
    dwarf_dealloc(dbg, stmt_list_attr, DW_DLA_ATTR);

    /*  If die has DW_AT_comp_dir attribute, get the string that names
        the compilation directory. */
    resattr = dwarf_attr(die, DW_AT_comp_dir, &comp_dir_attr, error);
    if (resattr == DW_DLV_ERROR) {
        return resattr;
    }
    if (resattr == DW_DLV_OK) {
        int cres = DW_DLV_ERROR;
        char *cdir = 0;

        cres = dwarf_formstring(comp_dir_attr, &cdir, error);
        if (cres == DW_DLV_ERROR) {
            return cres;
        } else if (cres == DW_DLV_OK) {
            comp_dir = (Dwarf_Small *) cdir;
        }
    }
    if (resattr == DW_DLV_OK) {
        dwarf_dealloc(dbg, comp_dir_attr, DW_DLA_ATTR);
    }

    dwarf_init_line_table_prefix(&prefix);
    {
        Dwarf_Small *line_ptr_out = 0;
        int dres = _dwarf_read_line_table_prefix(dbg, context,
            line_ptr,dbg->de_debug_line.dss_size - line_offset,
            &line_ptr_out,
            &prefix,
            &bogus_bytes_ptr,
            &bogus_bytes_count,
            error,
            err_count_out);
        if (dres == DW_DLV_ERROR) {
            dwarf_free_line_table_prefix(&prefix);
            return dres;
        }
        if (dres == DW_DLV_NO_ENTRY) {
            dwarf_free_line_table_prefix(&prefix);
            return dres;
        }
        line_ptr_end = prefix.pf_line_ptr_end;
        line_ptr = line_ptr_out;
    }
    if (only_line_header) {
        /* Just checking for header errors, nothing more here.*/
        dwarf_free_line_table_prefix(&prefix);
        return DW_DLV_OK;
    }

    dwarf_printf(dbg,
        "total line info length %ld bytes, "
        "line offset 0x%" DW_PR_XZEROS DW_PR_DUx " %" DW_PR_DSd "\n",
        (long) prefix.pf_total_length,
        (Dwarf_Unsigned) line_offset, (Dwarf_Signed) line_offset);
    if (prefix.pf_version <= DW_LINE_VERSION5) {
        dwarf_printf(dbg,
            "line table version %d\n",(int) prefix.pf_version);
    } else {
        dwarf_printf(dbg,
            "line table version 0x%x\n",(int) prefix.pf_version);
    }
    dwarf_printf(dbg,
        "line table length field length %d prologue length %d\n",
        (int)prefix.pf_length_field_length,
        (int)prefix.pf_prologue_length);
    dwarf_printf(dbg,
        "compilation_directory %s\n",
        comp_dir ? ((char *) comp_dir) : "");

    dwarf_printf(dbg,
        "  min instruction length %d\n",
        (int) prefix.pf_minimum_instruction_length);
    if (prefix.pf_version == EXPERIMENTAL_LINE_TABLES_VERSION) {
        dwarf_printf(dbg, "  actuals table offset "
            "0x%" DW_PR_XZEROS DW_PR_DUx
            " logicals table offset "
            "0x%" DW_PR_XZEROS DW_PR_DUx "\n",
            prefix.pf_actuals_table_offset,
            prefix.pf_logicals_table_offset);
    }
    if (prefix.pf_version == DW_LINE_VERSION5) {
        dwarf_printf(dbg,
            "  segment selector size %d\n",
            (int) prefix.pf_segment_selector_size);
        dwarf_printf(dbg,
            "  address    size       %d\n",
            (int) prefix.pf_address_size);
    }
    dwarf_printf(dbg,
        "  default is stmt        %d\n", (int) prefix.pf_default_is_stmt);
    dwarf_printf(dbg,
        "  line base              %d\n", (int) prefix.pf_line_base);
    dwarf_printf(dbg,
        "  line_range             %d\n", (int) prefix.pf_line_range);
    dwarf_printf(dbg,
        "  opcode base            %d\n", (int) prefix.pf_opcode_base);
    dwarf_printf(dbg,
        "  standard opcode count  %d\n", (int) prefix.pf_std_op_count);

    for (i = 1; i < prefix.pf_opcode_base; i++) {
        dwarf_printf(dbg,
            "  opcode[%2d] length  %d\n", (int) i,
            (int) prefix.pf_opcode_length_table[i - 1]);
    }
    dwarf_printf(dbg,
        "  include directories count %d\n",
        (int) prefix.pf_include_directories_count);
    for (u = 0; u < prefix.pf_include_directories_count; ++u) {
        dwarf_printf(dbg,
            "  include dir[%u] %s\n",
            (int) u, prefix.pf_include_directories[u]);
    }
    
    dwarf_printf(dbg,
        "  files count            %d\n",
        (int) prefix.pf_files_count);

    for (u = 0; u < prefix.pf_files_count; ++u) {
        struct Line_Table_File_Entry_s *lfile =
            prefix.pf_line_table_file_entries + u;
        Dwarf_Unsigned tlm2 = lfile->lte_last_modification_time;
        Dwarf_Unsigned di = lfile->lte_directory_index;
        Dwarf_Unsigned fl = lfile->lte_length_of_file;

        dwarf_printf(dbg,
            "  file[%u]  %s (file-number: %u) \n",
            (unsigned) u, (char *) lfile->lte_filename,
            (unsigned)(u+1));
        dwarf_printf(dbg,
            "    dir index %d\n", (int) di);
        {
            time_t tt = (time_t) tlm2;

            /* ctime supplies newline */
            dwarf_printf(dbg,
                "    last time 0x%x %s",
                (unsigned) tlm2, ctime(&tt));
        }
        dwarf_printf(dbg,
            "    file length %ld 0x%lx\n",
            (long) fl, (unsigned long) fl);

    }


    {
        Dwarf_Unsigned offset = 0;
        if (bogus_bytes_count > 0) {
            Dwarf_Unsigned wcount = bogus_bytes_count;
            Dwarf_Unsigned boffset = bogus_bytes_ptr - orig_line_ptr;
            dwarf_printf(dbg,
                "*** DWARF CHECK: the line table prologue  header_length "
                " is %" DW_PR_DUu " too high, we pretend it is smaller."
                "Section offset: 0x%" DW_PR_XZEROS DW_PR_DUx
                " (%" DW_PR_DUu ") ***\n",
                wcount, boffset,boffset);
            *err_count_out += 1;
        }
        offset = line_ptr - orig_line_ptr;
        dwarf_printf(dbg,
            "  statement prog offset in section: 0x%"
            DW_PR_XZEROS DW_PR_DUx " (%" DW_PR_DUu ")\n",
            offset, offset);
    }



    {
        Dwarf_Small *line_ptr_actuals  = 0;
        Dwarf_Small *line_ptr_end = prefix.pf_line_ptr_end;
        Dwarf_Bool is_actuals_table = false;
        Dwarf_Bool is_single_table = true;

        if (prefix.pf_actuals_table_offset > 0) {
            line_ptr_actuals = prefix.pf_line_ptr_start +
                prefix.pf_actuals_table_offset;
        }
        if (!line_ptr_actuals) {
            /* Normal single level line table. */
            print_line_header(dbg, is_single_table, is_actuals_table);
            res = print_statement_program(dbg,
                die, context,&prefix,
                orig_line_ptr,
                line_ptr,line_ptr_end,
                is_single_table,
                is_actuals_table,
                error,err_count_out);

            if (res != DW_DLV_OK) {
                dwarf_free_line_table_prefix(&prefix);
                return res;
            }
        } else {
            is_single_table = false;
            if (prefix.pf_version != EXPERIMENTAL_LINE_TABLES_VERSION) {
                _dwarf_error(dbg, error, DW_DLE_VERSION_STAMP_ERROR);
                return (DW_DLV_ERROR);
            }

            /* Read Logicals */
            print_line_header(dbg, is_single_table, is_actuals_table);
            res = print_statement_program(dbg,
                die,context,&prefix,
                orig_line_ptr,
                line_ptr,line_ptr_actuals,
                is_single_table,
                is_actuals_table,
                error,err_count_out);
                

            if (res != DW_DLV_OK) {
                dwarf_free_line_table_prefix(&prefix);
                return res;
            }
            if (prefix.pf_actuals_table_offset > 0) {
                is_actuals_table = true;
                /* Read Actuals */
                print_line_header(dbg, is_single_table, is_actuals_table);
                res = print_statement_program(dbg,
                    die,context,&prefix,
                    orig_line_ptr,
                    line_ptr_actuals,line_ptr_end,
                    is_single_table,
                    is_actuals_table,
                    error,err_count_out);

            }
        }
    }
    dwarf_free_line_table_prefix(&prefix);
    return res;
}


/*  This is support for dwarfdump: making it possible
    for clients wanting line detail info on stdout
    to get that detail without including internal libdwarf
    header information.
    Caller passes in compilation unit DIE.
    The _dwarf_ version is obsolete (though supported for
    compatibility).
    The dwarf_ version is preferred.
    The functions are intentionally identical: having
    _dwarf_print_lines call dwarf_print_lines might
    better emphasize they are intentionally identical, but
    that seemed slightly silly given how short the functions are.
    Interface adds error_count (output value) February 2009.  */
int
dwarf_print_lines(Dwarf_Die die, Dwarf_Error * error,int *error_count)
{
    int only_line_header = 0;
    int res = _dwarf_internal_printlines(die, error,
        error_count,
        only_line_header);
    if (res != DW_DLV_OK) {
        return res;
    }
    return res;
}
int
_dwarf_print_lines(Dwarf_Die die, Dwarf_Error * error)
{
    int only_line_header = 0;
    int err_count = 0;
    int res = _dwarf_internal_printlines(die, error,
        &err_count,
        only_line_header);
    /* No way to get error count back in this interface */
    if (res != DW_DLV_OK) {
        return res;
    }
    return res;
}

/* The check is in case we are not printing full line data,
   this gets some of the issues noted with .debug_line,
   but not all. Call dwarf_print_lines() to get all issues.
   Intended for apps like dwarfdump.
*/
void
dwarf_check_lineheader(Dwarf_Die die, int *err_count_out)
{
    Dwarf_Error err;
    int only_line_header = 1;
    _dwarf_internal_printlines(die, &err,err_count_out,
        only_line_header);
    return;
}

