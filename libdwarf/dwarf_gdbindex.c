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





#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include <stdlib.h>
#include "dwarf_gdbindex.h"

#define TRUE 1
#define FALSE 0

/*  The dwarf_util macro READ_UNALIGNED
    cannot be directly used because
    gdb defines the section contents of
    .gdb_index as little-endian always.
*/

#if WORDS_BIGENDIAN   /* meaning on this host */
#define READ_GDBINDEX(dest,desttype, source, length)                     \
    do {                                                                 \
        BIGGEST_UINT _ltmp = 0;                                          \
        _dwarf_memcpy_swap_bytes((((char *)(&_ltmp)) + sizeof(_ltmp) - length), \
            source, length) ;                                            \
        dest = (desttype)_ltmp;                                          \
    } while (0)
#else /* little-endian on this host */
#define READ_GDBINDEX(dest,desttype, source, length)                     \
    do {                                                                 \
        BIGGEST_UINT _ltmp = 0;                                          \
        memcpy(((char *)(&_ltmp)) ,            \
            source, length) ;                                            \
        dest = (desttype)_ltmp;                                          \
    } while (0)

#endif


struct gi_fileheader_s {
    gdbindex_offset_type headerval[6];
};

struct dwarf_64bitpair {
    gdbindex_64 offset;
    gdbindex_64 length;
};

static int
set_base(Dwarf_Debug dbg,
    struct Dwarf_Gdbindex_array_instance_s * hdr,
    Dwarf_Small *start,
    Dwarf_Small *end,
    /* entrylen is the length of a single struct as seen in the object. */
    Dwarf_Unsigned entrylen,
    /* The size of each field in the struct in the object. */
    Dwarf_Unsigned fieldlen,
    enum gdbindex_type_e type,
    Dwarf_Error * err)
{

    if (type == git_std) {
        Dwarf_Unsigned count = 0;
        if( end < start) {
            _dwarf_error(dbg, err,DW_DLE_GDB_INDEX_COUNT_ERROR); 
            return DW_DLV_ERROR;
        }
        count = end - start;
        count = count / entrylen;
        hdr->dg_type = type;
        hdr->dg_base = start;
        hdr->dg_count = count;
        hdr->dg_entry_length = entrylen;
        hdr->dg_fieldlen = fieldlen;
    } else if ( type == git_cuvec) {
        /* array of offset_type values. First
           value is number of cu_instances. */
        Dwarf_Small * calc_end = 0;
        hdr->dg_base = start;
        READ_GDBINDEX(hdr->dg_count,Dwarf_Unsigned,
            start,
            sizeof(gdbindex_offset_type));
        calc_end = start + 
            /* add 1 for the initial 'count' field itself. */
            ((1 +hdr->dg_count) * sizeof(gdbindex_offset_type));
        if (end < calc_end) {
            _dwarf_error(dbg, err,DW_DLE_GDB_INDEX_CUVEC_ERROR); 
            return DW_DLV_ERROR;
        }
        /* Bypass the count, the vec starts next. */
        hdr->dg_base += sizeof(gdbindex_offset_type);
        hdr->dg_fieldlen = sizeof(gdbindex_offset_type);
        hdr->dg_entry_length = sizeof(gdbindex_offset_type);
        hdr->dg_type = type;
    } else {
        /* address area. */
        /* 64bit, 64bit, offset. Then 32bit pad. */
        Dwarf_Unsigned count = 0;
        hdr->dg_base = start;
        if( end < start) {
            _dwarf_error(dbg, err,DW_DLE_GDB_INDEX_COUNT_ADDR_ERROR); 
            return DW_DLV_ERROR;
        }
        /* entry length includes pad. */
        hdr->dg_entry_length = 2*sizeof(gdbindex_64) +
             sizeof(gdbindex_offset_type);
        count = end - start;
        count = count / hdr->dg_entry_length;
        hdr->dg_count = count;
        /* The dg_fieldlen is a fake, the fields are not
           all the same length. */
        hdr->dg_fieldlen = sizeof(gdbindex_offset_type);
        hdr->dg_type = type;
    }
    return DW_DLV_OK;
}

int 
dwarf_gdbindex_header(Dwarf_Debug dbg,
    Dwarf_Gdbindex * gdbindexptr,
    Dwarf_Unsigned * version,
    Dwarf_Unsigned * cu_list_offset,
    Dwarf_Unsigned * types_cu_list_offset,
    Dwarf_Unsigned * address_area_offset,
    Dwarf_Unsigned * symbol_table_offset,
    Dwarf_Unsigned * constant_pool_offset,
    Dwarf_Unsigned * section_size,
    Dwarf_Unsigned * unused_reserved,
    Dwarf_Error    * error)
{

    struct gi_fileheader_s header;
    Dwarf_Gdbindex indexptr = 0;
    Dwarf_Unsigned entry_count = 0;
    Dwarf_Small *base = 0;

    int res = DW_DLV_ERROR;

    if (!dbg->de_debug_gdbindex.dss_size) {
        return DW_DLV_NO_ENTRY;
    }
    if (!dbg->de_debug_gdbindex.dss_data) {
        res = _dwarf_load_section(dbg, &dbg->de_debug_gdbindex,error);
        if (res != DW_DLV_OK) {
            return res;
        }
    }

    if (dbg->de_debug_gdbindex.dss_size < sizeof(struct gi_fileheader_s) ) {
        _dwarf_error(dbg, error, DW_DLE_ERRONEOUS_GDB_INDEX_SECTION);
        return (DW_DLV_ERROR);
    }
    
    memcpy(&header,dbg->de_debug_gdbindex.dss_data,
        sizeof(struct gi_fileheader_s));

    indexptr = _dwarf_get_alloc(dbg,DW_DLA_GDBINDEX,1);
    if (indexptr == NULL) {
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return (DW_DLV_ERROR);
    }

    indexptr->gi_dbg = dbg;
    indexptr->gi_section_data = dbg->de_debug_gdbindex.dss_data;
    indexptr->gi_section_length = dbg->de_debug_gdbindex.dss_size;
    READ_GDBINDEX(indexptr->gi_version ,Dwarf_Unsigned,
        dbg->de_debug_gdbindex.dss_data,
        sizeof(gdbindex_offset_type));
    READ_GDBINDEX(indexptr->gi_cu_list_offset ,Dwarf_Unsigned,
        dbg->de_debug_gdbindex.dss_data + sizeof(gdbindex_offset_type),
        sizeof(gdbindex_offset_type));
    READ_GDBINDEX(indexptr->gi_types_cu_list_offset ,Dwarf_Unsigned,
        dbg->de_debug_gdbindex.dss_data + 2*sizeof(gdbindex_offset_type),
        sizeof(gdbindex_offset_type));
    READ_GDBINDEX(indexptr->gi_address_area_offset ,Dwarf_Unsigned,
        dbg->de_debug_gdbindex.dss_data + 3*sizeof(gdbindex_offset_type),
        sizeof(gdbindex_offset_type));
    READ_GDBINDEX(indexptr->gi_symbol_table_offset ,Dwarf_Unsigned,
        dbg->de_debug_gdbindex.dss_data + 4*sizeof(gdbindex_offset_type),
        sizeof(gdbindex_offset_type));
    READ_GDBINDEX(indexptr->gi_constant_pool_offset ,Dwarf_Unsigned,
        dbg->de_debug_gdbindex.dss_data + 5*sizeof(gdbindex_offset_type),
        sizeof(gdbindex_offset_type));

    res = set_base(dbg,&indexptr->gi_culisthdr,
        dbg->de_debug_gdbindex.dss_data + indexptr->gi_cu_list_offset,
        dbg->de_debug_gdbindex.dss_data + indexptr->gi_types_cu_list_offset,
        2*sizeof(gdbindex_64),
        sizeof(gdbindex_64), 
        git_std,error);
    res = set_base(dbg,&indexptr->gi_typesculisthdr,
        dbg->de_debug_gdbindex.dss_data + indexptr->gi_types_cu_list_offset,
        dbg->de_debug_gdbindex.dss_data + indexptr->gi_address_area_offset,
        3*sizeof(gdbindex_64),
        sizeof(gdbindex_64),
        git_std,error);
    res = set_base(dbg,&indexptr->gi_addressareahdr,
        dbg->de_debug_gdbindex.dss_data + indexptr->gi_address_area_offset,
        dbg->de_debug_gdbindex.dss_data + indexptr->gi_symbol_table_offset,
        3*sizeof(gdbindex_64),
        sizeof(gdbindex_64),
        git_address,error);
    res = set_base(dbg,&indexptr->gi_symboltablehdr,
        dbg->de_debug_gdbindex.dss_data + indexptr->gi_symbol_table_offset,
        dbg->de_debug_gdbindex.dss_data + indexptr->gi_constant_pool_offset,
        2*sizeof(gdbindex_offset_type),
        sizeof(gdbindex_offset_type),
        git_std,error);
    res = set_base(dbg,&indexptr->gi_cuvectorhdr,
        dbg->de_debug_gdbindex.dss_data + indexptr->gi_constant_pool_offset,
        /* The actual cu vector size will be calculated. */
        dbg->de_debug_gdbindex.dss_data + dbg->de_debug_gdbindex.dss_size,
        sizeof(gdbindex_offset_type),
        sizeof(gdbindex_offset_type),
        git_cuvec,error);

    /* Really just pointing to constant pool area. */
    indexptr->gi_string_pool = dbg->de_debug_gdbindex.dss_data +
        indexptr->gi_constant_pool_offset;

    *gdbindexptr          = indexptr;
    *version              = indexptr->gi_version;
    *cu_list_offset       = indexptr->gi_cu_list_offset;
    *types_cu_list_offset = indexptr->gi_types_cu_list_offset;
    *address_area_offset  = indexptr->gi_address_area_offset;
    *symbol_table_offset  = indexptr->gi_symbol_table_offset;
    *constant_pool_offset = indexptr->gi_constant_pool_offset; 
    *section_size         = indexptr->gi_section_length;
    *unused_reserved = 0;

    return DW_DLV_OK;


}


int 
dwarf_gdbindex_culist_array(Dwarf_Gdbindex gdbindexptr,
    Dwarf_Unsigned       * list_length,
    Dwarf_Error          * error)
{
    *list_length = gdbindexptr->gi_culisthdr.dg_count;
    return DW_DLV_OK;
}

/*  entryindex: 0 to list_length-1 */
int 
dwarf_gdbindex_culist_entry(Dwarf_Gdbindex gdbindexptr,
    Dwarf_Unsigned   entryindex,
    Dwarf_Unsigned * cu_offset,
    Dwarf_Unsigned * cu_length,
    Dwarf_Error    * error)
{
    Dwarf_Unsigned max =  gdbindexptr->gi_culisthdr.dg_count;
    Dwarf_Small * base = 0;
    Dwarf_Unsigned offset = 0;
    Dwarf_Unsigned length = 0;
    unsigned fieldlen = gdbindexptr->gi_culisthdr.dg_fieldlen;
    
    if (entryindex >= max) {
        _dwarf_error(gdbindexptr->gi_dbg, error,DW_DLE_GDB_INDEX_INDEX_ERROR);
        return DW_DLV_ERROR;
    }
    base = gdbindexptr->gi_culisthdr.dg_base;
    base += entryindex*gdbindexptr->gi_culisthdr.dg_entry_length;

    READ_GDBINDEX(offset ,Dwarf_Unsigned,
        base,
        fieldlen);
    READ_GDBINDEX(length ,Dwarf_Unsigned,
        base+ fieldlen,
        fieldlen);
    *cu_offset = offset;
    *cu_length = length;
    return DW_DLV_OK;
}

int
dwarf_gdbindex_types_culist_array(Dwarf_Gdbindex gdbindexptr,
    Dwarf_Unsigned       * list_length,
    Dwarf_Error          * error)
{
    *list_length = gdbindexptr->gi_typesculisthdr.dg_count;
    return DW_DLV_OK;
}

/*  entryindex: 0 to list_length-1 */
int
dwarf_gdbindex_types_culist_entry(Dwarf_Gdbindex gdbindexptr,
    Dwarf_Unsigned   entryindex,
    Dwarf_Unsigned * t_offset,
    Dwarf_Unsigned * t_length,
    Dwarf_Unsigned * t_signature,
    Dwarf_Error    * error)
{
    Dwarf_Unsigned max =  gdbindexptr->gi_typesculisthdr.dg_count;
    Dwarf_Small * base = 0;
    Dwarf_Unsigned offset = 0;
    Dwarf_Unsigned length = 0;
    Dwarf_Unsigned signature = 0;
    unsigned fieldlen = gdbindexptr->gi_typesculisthdr.dg_fieldlen;

    if (entryindex >= max) {
        _dwarf_error(gdbindexptr->gi_dbg, error,DW_DLE_GDB_INDEX_INDEX_ERROR);
        return DW_DLV_ERROR;
    }
    base = gdbindexptr->gi_typesculisthdr.dg_base;
    base += entryindex*gdbindexptr->gi_typesculisthdr.dg_entry_length;

    READ_GDBINDEX(offset ,Dwarf_Unsigned,
        base,
        fieldlen);
    READ_GDBINDEX(length ,Dwarf_Unsigned,
        base+ (1*fieldlen),
        fieldlen);
    READ_GDBINDEX(signature ,Dwarf_Unsigned,
        base+ (2*fieldlen),
        fieldlen);
    *t_offset = offset;
    *t_length = length;
    *t_signature = signature;
    return DW_DLV_OK;
}

int 
dwarf_gdbindex_addressarea(Dwarf_Gdbindex gdbindexptr,
    Dwarf_Unsigned            * list_length,
    Dwarf_Error               * error)
{
    *list_length = gdbindexptr->gi_addressareahdr.dg_count;
    return DW_DLV_OK;
}

/*    entryindex: 0 to addressarea_list_length-1 */
int 
dwarf_gdbindex_addressarea_entry(
    Dwarf_Gdbindex   gdbindexptr,
    Dwarf_Unsigned   entryindex,
    Dwarf_Unsigned * low_address,
    Dwarf_Unsigned * high_address,
    Dwarf_Unsigned * cu_index,
    Dwarf_Error    * error)
{
    Dwarf_Unsigned max =  gdbindexptr->gi_addressareahdr.dg_count;
    Dwarf_Small * base = 0;
    Dwarf_Unsigned lowaddr = 0;
    Dwarf_Unsigned highaddr = 0;
    Dwarf_Unsigned cuindex = 0;

    if (entryindex >= max) {
        _dwarf_error(gdbindexptr->gi_dbg, error,DW_DLE_GDB_INDEX_INDEX_ERROR);
        return DW_DLV_ERROR;
    }
    base = gdbindexptr->gi_addressareahdr.dg_base;
    base += entryindex*gdbindexptr->gi_addressareahdr.dg_entry_length;

    READ_GDBINDEX(lowaddr ,Dwarf_Unsigned,
        base,
        sizeof(gdbindex_64));
    READ_GDBINDEX(highaddr ,Dwarf_Unsigned,
        base+ (1*sizeof(gdbindex_64)),
        sizeof(gdbindex_64));
    READ_GDBINDEX(cuindex ,Dwarf_Unsigned,
        base+ (2*sizeof(gdbindex_64)),
        sizeof(gdbindex_offset_type));
    *low_address = lowaddr;
    *high_address = highaddr;
    *cu_index = cuindex;
    return DW_DLV_OK;
}




void
dwarf_gdbindex_free(Dwarf_Gdbindex indexptr)
{
    if(indexptr) {
        Dwarf_Debug dbg = indexptr->gi_dbg;
        dwarf_dealloc(dbg,indexptr,DW_DLA_GDBINDEX);
    }
}
