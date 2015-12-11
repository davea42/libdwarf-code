/*
  Copyright (C) 2015-2015 David Anderson. All Rights Reserved.

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

/*
   dwarf_macro5.h
   For the DWARF5 .debug_macro section
   (also appears as an extension to DWARF4)
*/

struct Dwarf_Macro_Forms_s {
    /* Code means DW_MACRO_define etc. */
    Dwarf_Small         mf_code;

    /* How many entries in mf_formbytes array. */
    Dwarf_Small         mf_formcount;

    /* Never free these, these are in the object file memory */
    const Dwarf_Small * mf_formbytes;
};

struct Dwarf_Macro_OperationsList_s {
    unsigned mol_count;
    struct Dwarf_Macro_Forms_s * mol_data;
};


#define MACRO_OFFSET_SIZE_FLAG 1
#define MACRO_LINE_OFFSET_FLAG 2
#define MACRO_OP_TABLE_FLAG 4

struct Dwarf_Macro_Context_s {
    Dwarf_Word mc_sentinel;
    /* Section_offset in .debug_macro of macro header */
    Dwarf_Unsigned mc_section_offset;
    Dwarf_Half mc_version_number;
    /*  Total length of the macro data for this CU.
        Calculated, not part of header. */
    Dwarf_Unsigned mc_total_length;

    Dwarf_Half  mc_macro_header_length;

    Dwarf_Small mc_flags;
    Dwarf_Unsigned mc_debug_line_offset;

    /* the following three set from the bits in mc_flags  */
    Dwarf_Bool mc_offset_size_flag; /* If 1, offsets 64 bits */

    /* if 1, debug_line offset is present. */
    Dwarf_Bool mc_debug_line_offset_flag;

    /* 4 or 8 */
    Dwarf_Small    mc_offset_size;

    /*  If one the operands/opcodes (mc_opcode_forms) table is present
        in the header. If not we use a default table.

        Even when there are operands in the  header
        the standardops may or may not be
        defined in the header. */
    Dwarf_Bool mc_operands_table_flag;

    /*  Count of the Dwarf_Macro_Forms_s structs pointed to by
        mc_opcode_forms.  */
    Dwarf_Small  mc_opcode_count;
    struct Dwarf_Macro_Forms_s *mc_opcode_forms;

    Dwarf_Debug mc_dbg;
    Dwarf_CU_Context mc_cu_context;

};


int _dwarf_macro_constructor(Dwarf_Debug dbg, void *m);
void _dwarf_macro_destructor(void *m);
