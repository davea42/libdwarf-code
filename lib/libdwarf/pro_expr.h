/*
  Copyright (C) 2000,2004 Silicon Graphics, Inc.  All Rights Reserved.

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


/*  This is really a sanity limit. It could be increased
    as necesary for long dwarf expressions.
    Originally it was 20, and expressions longer than that
    are now pretty routine. 24 January 2021 */
#define MAXIMUM_LOC_EXPR_LENGTH 300

struct Dwarf_P_Expr_s {
    Dwarf_Small ex_byte_stream[MAXIMUM_LOC_EXPR_LENGTH];
    Dwarf_P_Debug ex_dbg;
    Dwarf_Unsigned ex_next_byte_offset;
    Dwarf_Unsigned ex_reloc_sym_index;
    Dwarf_Unsigned ex_reloc_offset;
};
