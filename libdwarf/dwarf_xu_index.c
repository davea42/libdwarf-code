/*

  Copyright (C) 2014-2014 David Anderson. All Rights Reserved.

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
/* The address of the Free Software Foundation is
   Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
   Boston, MA 02110-1301, USA.
   SGI has moved from the Crittenden Lane address.
*/


/*  The file and functions have  'xu' because 
    the .debug_cu_index and .debug_tu_index
    sections have the same layout and this deals with both.

    This is DebugFission, part of DWARF5.
   
    It allows properly reading a .dwp object file
    with debug-information (no code).
*/



#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include <stdlib.h>
#include "dwarf_xu_index.h"

typedef __uint32_t xu_uint32_t;

#define TRUE 1
#define FALSE 0

int
dwarf_get_xu_index_header(Dwarf_Debug dbg,
    /* Pass in section_type "tu" or "cu" */
    const char *     section_type, 
    Dwarf_Xu_Index_Header * xuptr,
    Dwarf_Unsigned * version,
    Dwarf_Unsigned * number_of_columns, /* L */
    Dwarf_Unsigned * number_of_CUs,     /* N */
    Dwarf_Unsigned * number_of_slots,   /* M */
    const char    ** section_name,
    Dwarf_Error    * error)
{
    Dwarf_Xu_Index_Header indexptr = 0;
    int res = DW_DLV_ERROR;
    struct Dwarf_Section_s *sect = 0;
    Dwarf_Unsigned local_version = 0;
    Dwarf_Unsigned num_col  = 0;
    Dwarf_Unsigned num_CUs  = 0;
    Dwarf_Unsigned num_slots  = 0;
    Dwarf_Small *data = 0;
    Dwarf_Small *pastend = 0;
    unsigned datalen = sizeof(xu_uint32_t);

    if (!strcmp(section_type,"cu") ) {
        sect = &dbg->de_debug_cu_index;
    } else if (!strcmp(section_type,"tu") ) {
        sect = &dbg->de_debug_tu_index;
    } else {
        _dwarf_error(dbg, error, DW_DLE_XU_TYPE_ARG_ERROR);
        return DW_DLV_ERROR;
    }
    if (!sect->dss_size) {
        return DW_DLV_NO_ENTRY;
    }

    if (!sect->dss_data) {
        res = _dwarf_load_section(dbg, sect,error);
        if (res != DW_DLV_OK) {
            return res;
        }
    }

    data = sect->dss_data;
    pastend = data + sect->dss_size;

    if (sect->dss_size < (4*datalen) ) {
        _dwarf_error(dbg, error, DW_DLE_ERRONEOUS_GDB_INDEX_SECTION);
        return (DW_DLV_ERROR);
    }
    READ_UNALIGNED(dbg,local_version, Dwarf_Unsigned,
        data,datalen);
    data += datalen;
    READ_UNALIGNED(dbg,num_col, Dwarf_Unsigned,
        data,datalen);
    data += datalen;
    READ_UNALIGNED(dbg,num_CUs, Dwarf_Unsigned,
        data,datalen);
    data += datalen;
    READ_UNALIGNED(dbg,num_slots, Dwarf_Unsigned,
        data,datalen);
    data += datalen;


    indexptr = _dwarf_get_alloc(dbg,DW_DLA_XU_INDEX,1);
    if (indexptr == NULL) {
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return (DW_DLV_ERROR);
    }
    strcpy(indexptr->gx_type,section_type);
    indexptr->gx_section_length = sect->dss_size;
    indexptr->gx_section_data   = sect->dss_data;
    indexptr->gx_section_name   = sect->dss_name;
    indexptr->gx_version        = local_version;
    indexptr->gx_column_count_sections = num_col;
    indexptr->gx_units_in_index = num_CUs;
    indexptr->gx_slots_in_hash  = num_slots;
    *xuptr             =     indexptr;
    *version           = indexptr->gx_version;
    *number_of_columns = indexptr->gx_column_count_sections;
    *number_of_CUs     = indexptr->gx_units_in_index;
    *number_of_slots   = indexptr->gx_slots_in_hash;
    *section_name      = indexptr->gx_section_name;
    return DW_DLV_OK;
}

int dwarf_get_xu_index_section_type(Dwarf_Xu_Index_Header xuhdr,
    /*  the function returns a pointer to
        the immutable string "tu" or "cu" via this arg. Do not free.  */
    const char ** typename,
    /*  the function returns a pointer to
        the immutable section name. Do not free.
        .debug_cu_index or .debug_tu_index */
    const char ** sectionname,
    Dwarf_Error * err)
{
    *typename    = &xuhdr->gx_type[0];
    *sectionname = xuhdr->gx_section_name; 
    return DW_DLV_OK;
}



void
dwarf_xu_header_free(Dwarf_Xu_Index_Header indexptr)
{
    if(indexptr) {
        Dwarf_Debug dbg = indexptr->gx_dbg;
        dwarf_dealloc(dbg,indexptr,DW_DLA_XU_INDEX);
    }
}
