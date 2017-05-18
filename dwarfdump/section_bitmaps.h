/*
  Copyright (C) 2017-2017 David Anderson. All Rights Reserved.

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

#ifndef SECTION_BITMAPS_H_INCLUDED
#define SECTION_BITMAPS_H_INCLUDED

struct section_bitmap_s {
    const char *name;
    unsigned    bitvalue;
};

extern struct section_bitmap_s  map_sectnames[] ;

#define DW_HDR_DEBUG_INFO            0x00000001
#define DW_HDR_DEBUG_INFO_DWO        0x00000002
#define DW_HDR_DEBUG_LINE            0x00000004
#define DW_HDR_DEBUG_LINE_DWO        0x00000008
#define DW_HDR_DEBUG_PUBNAMES        0x00000010
#define DW_HDR_DEBUG_ABBREV          0x00000020
#define DW_HDR_DEBUG_ABBREV_DWO      0x00000040
#define DW_HDR_DEBUG_ARANGES         0x00000080
#define DW_HDR_DEBUG_FRAME           0x00000100
#define DW_HDR_DEBUG_LOC             0x00000200
#define DW_HDR_DEBUG_LOCLISTS        0x00000400
#define DW_HDR_DEBUG_LOCLISTS_DWO    0x00000800
#define DW_HDR_DEBUG_RANGES          0x00001000
#define DW_HDR_DEBUG_RNGLISTS        0x00002000
#define DW_HDR_DEBUG_RNGLISTS_DWO    0x00004000
#define DW_HDR_DEBUG_STR             0x00008000
#define DW_HDR_DEBUG_STR_DWO         0x00010000
#define DW_HDR_DEBUG_STR_OFFSETS     0x00020000
#define DW_HDR_DEBUG_STR_OFFSETS_DWO 0x00040000
#define DW_HDR_DEBUG_PUBTYPES        0x00080000
#define DW_HDR_DEBUG_TYPES           0x00100000
#define DW_HDR_TEXT                  0x00200000
#define DW_HDR_GDB_INDEX             0x00400000
#define DW_HDR_EH_FRAME              0x00800000
#define DW_HDR_DEBUG_MACINFO         0x01000000
#define DW_HDR_DEBUG_MACRO           0x02000000
#define DW_HDR_DEBUG_MACRO_DWO       0x04000000
#define DW_HDR_DEBUG_NAMES           0x08000000
#define DW_HDR_DEBUG_CU_INDEX        0x10000000
#define DW_HDR_DEBUG_TU_INDEX        0x20000000
#define DW_HDR_HEADER                0x40000000
/* No bits left, more sections means a rethink */

#define DW_HDR_ALL     0x80000000
#define DW_HDR_DEFAULT 0x7fffffff
#define DW_INDEX MASK  0x7fffffff



/* Debug section names to be included in printing */
#define DW_SECTNAME_DEBUG_INFO       ".debug_info"
#define DW_SECTNAME_DEBUG_INFO_DWO   ".debug_info.dwo"
#define DW_SECTNAME_DEBUG_LINE       ".debug_line"
#define DW_SECTNAME_DEBUG_LINE_DWO   ".debug_line.dwo"
#define DW_SECTNAME_DEBUG_PUBNAMES   ".debug_pubnames"
#define DW_SECTNAME_DEBUG_ABBREV     ".debug_abbrev"
#define DW_SECTNAME_DEBUG_ABBREV_DWO ".debug_abbrev.dwo"
#define DW_SECTNAME_DEBUG_ARANGES    ".debug_aranges"
#define DW_SECTNAME_DEBUG_FRAME      ".debug_frame"
#define DW_SECTNAME_DEBUG_LOC        ".debug_loc"
#define DW_SECTNAME_DEBUG_LOCLISTS   ".debug_loclists"
#define DW_SECTNAME_DEBUG_LOCLISTS_DWO ".debug_loclists.dwo"
#define DW_SECTNAME_DEBUG_RANGES     ".debug_ranges"
#define DW_SECTNAME_DEBUG_RNGLISTS   ".debug_rnglists"
#define DW_SECTNAME_DEBUG_RNGLISTS_DWO ".debug_rnglists.dwo"
#define DW_SECTNAME_DEBUG_STR        ".debug_str"
#define DW_SECTNAME_DEBUG_STR_DWO    ".debug_str.dwo"
#define DW_SECTNAME_DEBUG_STR_OFFSETS ".debug_str_offsets"
#define DW_SECTNAME_DEBUG_STR_OFFSETS_DWO ".debug_str_offsets.dwo"
/*  obsolete SGI-only section was .debug_typenames */
#define DW_SECTNAME_DEBUG_PUBTYPES  ".debug_pubtypes"
#define DW_SECTNAME_DEBUG_TYPES     ".debug_types"
#define DW_SECTNAME_TEXT            ".text"
#define DW_SECTNAME_GDB_INDEX       ".gdb_index"
#define DW_SECTNAME_EH_FRAME        ".eh_frame"
#define DW_SECTNAME_DEBUG_SUP       ".debug_sup"
#define DW_SECTNAME_DEBUG_MACINFO   ".debug_macinfo"
#define DW_SECTNAME_DEBUG_MACRO     ".debug_macro"
#define DW_SECTNAME_DEBUG_MACRO_DWO ".debug_macro.dwo"
#define DW_SECTNAME_DEBUG_NAMES     ".debug_names"
#define DW_SECTNAME_DEBUG_CU_INDEX  ".debug_cu_index"
#define DW_SECTNAME_DEBUG_TU_INDEX  ".debug_tu_index"

/* This part is a bit obsolete: FIXME */
/* Definitions for printing relocations. */
#define DW_SECTION_REL_DEBUG_INFO     0
#define DW_SECTION_REL_DEBUG_LINE     1
#define DW_SECTION_REL_DEBUG_PUBNAMES 2
#define DW_SECTION_REL_DEBUG_ABBREV   3
#define DW_SECTION_REL_DEBUG_ARANGES  4
#define DW_SECTION_REL_DEBUG_FRAME    5
#define DW_SECTION_REL_DEBUG_LOC      6
#define DW_SECTION_REL_DEBUG_RANGES   7
#define DW_SECTION_REL_DEBUG_TYPES    8
#define DW_REL_MASK_PRINT_ALL             0x00ff

#endif /* SECTION_BITMAPS_H_INCLUDED*/
