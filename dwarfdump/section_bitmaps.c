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

#include "globals.h"

struct section_bitmap_s
map_sectnames[] = {
    {DW_SECTNAME_DEBUG_INFO,            DW_HDR_DEBUG_INFO},
    {DW_SECTNAME_DEBUG_INFO_DWO,        DW_HDR_DEBUG_INFO_DWO},
    {DW_SECTNAME_DEBUG_LINE,            DW_HDR_DEBUG_LINE},
    {DW_SECTNAME_DEBUG_LINE_DWO,        DW_HDR_DEBUG_LINE_DWO},
    {DW_SECTNAME_DEBUG_PUBNAMES,        DW_HDR_DEBUG_PUBNAMES},
    {DW_SECTNAME_DEBUG_ABBREV,          DW_HDR_DEBUG_ABBREV},
    {DW_SECTNAME_DEBUG_ABBREV_DWO,      DW_HDR_DEBUG_ABBREV_DWO},
    {DW_SECTNAME_DEBUG_ARANGES,         DW_HDR_DEBUG_ARANGES},
    {DW_SECTNAME_DEBUG_FRAME,           DW_HDR_DEBUG_FRAME},
    {DW_SECTNAME_DEBUG_LOC,             DW_HDR_DEBUG_LOC},
    {DW_SECTNAME_DEBUG_LOCLISTS,        DW_HDR_DEBUG_LOCLISTS},
    {DW_SECTNAME_DEBUG_LOCLISTS_DWO,    DW_HDR_DEBUG_LOCLISTS_DWO},
    {DW_SECTNAME_DEBUG_RANGES,          DW_HDR_DEBUG_RANGES},
    {DW_SECTNAME_DEBUG_RNGLISTS,        DW_HDR_DEBUG_RNGLISTS},
    {DW_SECTNAME_DEBUG_RNGLISTS_DWO,    DW_HDR_DEBUG_RNGLISTS_DWO},
    {DW_SECTNAME_DEBUG_STR,             DW_HDR_DEBUG_STR},
    {DW_SECTNAME_DEBUG_STR_DWO,         DW_HDR_DEBUG_STR_DWO},
    {DW_SECTNAME_DEBUG_STR_OFFSETS,     DW_HDR_DEBUG_STR_OFFSETS},
    {DW_SECTNAME_DEBUG_STR_OFFSETS_DWO, DW_HDR_DEBUG_STR_OFFSETS_DWO},
    {DW_SECTNAME_DEBUG_PUBTYPES,        DW_HDR_DEBUG_PUBTYPES},
    {DW_SECTNAME_DEBUG_TYPES,           DW_HDR_DEBUG_TYPES},
    {DW_SECTNAME_TEXT,                  DW_HDR_TEXT},
    {DW_SECTNAME_GDB_INDEX,             DW_HDR_GDB_INDEX},
    {DW_SECTNAME_EH_FRAME,              DW_HDR_EH_FRAME},
    {DW_SECTNAME_DEBUG_MACINFO,         DW_HDR_DEBUG_MACINFO},
    {DW_SECTNAME_DEBUG_MACRO,           DW_HDR_DEBUG_MACRO},
    {DW_SECTNAME_DEBUG_MACRO_DWO,       DW_HDR_DEBUG_MACRO_DWO},
    {DW_SECTNAME_DEBUG_NAMES,           DW_HDR_DEBUG_NAMES},
    {DW_SECTNAME_DEBUG_CU_INDEX,        DW_HDR_DEBUG_CU_INDEX},
    {DW_SECTNAME_DEBUG_TU_INDEX,        DW_HDR_DEBUG_TU_INDEX},
    {"",0}
};

#ifdef SELFTEST

int main()
{
    unsigned i = 0;

    for ( ; *map_sectnames[i].name ; ++i) {
        unsigned bitv = 0;
        if  ( i > 30) {
            printf("FAIL: map_sections.c ,the table is too large for the current approach\n");
            exit(1);
        }
        bitv = 1<<i;
        if (map_sectnames[i].bitvalue != bitv) {
            printf("FAIL map_sections.c at entry %s we have 0x%x vs 0x%x"
                " mismatch\n",
                map_sectnames[i].name,map_sectnames[i].bitvalue,bitv);
            exit(1);
        }
    }
    printf("PASS section bitmaps\n");
    return 0;
}




#endif /* SELFTEST */
