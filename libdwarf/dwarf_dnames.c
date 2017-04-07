/*
  Portions Copyright (C) 2017-2017 David Anderson. All Rights Reserved.

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

/*  This provides access to the DWARF5 .debug_names section. */

#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include <stdlib.h>
#include "dwarf_global.h"
#include "dwarf_dnames.h"


static int
read_uword_val(Dwarf_Debug dbg, Dwarf_Small **ptr_in, 
    Dwarf_Small *endptr,
    int   errcode,
    Dwarf_ufixed *val_out,
    Dwarf_Unsigned area_length,
    Dwarf_Error *error)
{
    Dwarf_ufixed val = 0;
    Dwarf_Small *ptr = *ptr_in;

    READ_UNALIGNED_CK(dbg, val, Dwarf_ufixed,
            ptr, sizeof(Dwarf_ufixed),
            error,endptr);
    ptr += sizeof(Dwarf_ufixed);
    if (ptr >= endptr) {
        _dwarf_error(dbg, error,errcode);
        return DW_DLV_ERROR;
    }
    /*  Some of the fields are not length fields, but
        if non-zero the size will be longer than
        the value, so we do the following
        overall sanity check to avoid overflows. */
    if (val > area_length) {
        _dwarf_error(dbg, error,errcode);
        return DW_DLV_ERROR;
    }
    *val_out = val;
    *ptr_in = ptr;
    return DW_DLV_OK;
}

/*  We do not alter the dn data here. */
static int
read_a_name_index(Dwarf_Dnames_Head dn,
    Dwarf_Small **curptr_in,
    Dwarf_Small *end_section,
    Dwarf_Unsigned remaining_section_size,
    struct Dwarf_Dnames_index_header_s ** index_header_out,
    Dwarf_Error *error)
{
    Dwarf_Unsigned area_length = 0;
    int local_length_size;
    int local_extension_size = 0;
    Dwarf_Small *past_length = 0;
    Dwarf_Small *end_dnames = 0;
    Dwarf_Half version = 0;
    Dwarf_Half padding = 0;
    Dwarf_ufixed comp_unit_count = 0;
    Dwarf_ufixed local_type_unit_count = 0;
    Dwarf_ufixed foreign_type_unit_count = 0;
    Dwarf_ufixed bucket_count = 0;
    Dwarf_ufixed name_count = 0;
    Dwarf_ufixed abbrev_table_size = 0;
    Dwarf_ufixed augmentation_string_size = 0;
    int res = 0;
    const char *str_utf8 = 0;
    Dwarf_Small *curptr = *curptr_in;
    struct Dwarf_Dnames_index_header_s *di_header = 0;
    Dwarf_Debug dbg = dn->dn_dbg;

    READ_AREA_LENGTH_CK(dbg, area_length, Dwarf_Unsigned,
        curptr, local_length_size,
        local_extension_size,error,
        remaining_section_size,end_section);

    /* curptr now points past the length field */
    past_length = curptr;

    /* Two stage length test so overflow is caught. */
    if (area_length > remaining_section_size) {
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }
    if ((area_length + local_length_size + local_extension_size) >
        remaining_section_size) {
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }
    end_dnames = curptr + area_length;

    READ_UNALIGNED_CK(dbg, version, Dwarf_Half,
            curptr, sizeof(Dwarf_Half),
            error,end_dnames);
    curptr += sizeof(Dwarf_Half);
    if (curptr >= end_dnames) {
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }
    if (version != DWARF_DNAMES_VERSION5) {
            _dwarf_error(dbg, error, DW_DLE_VERSION_STAMP_ERROR);
            return (DW_DLV_ERROR);
    }
    READ_UNALIGNED_CK(dbg, padding, Dwarf_Half,
            curptr, sizeof(Dwarf_Half),
            error,end_dnames);
    curptr += sizeof(Dwarf_Half);
    if (curptr >= end_dnames) {
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }
    if (padding) {
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return (DW_DLV_ERROR);
    }
    res = read_uword_val(dbg, &curptr,
        end_dnames, DW_DLE_DEBUG_NAMES_HEADER_ERROR,
        &comp_unit_count,area_length,error);
    if (res != DW_DLV_OK) {
        return res;
    }
    res = read_uword_val(dbg, &curptr,
        end_dnames, DW_DLE_DEBUG_NAMES_HEADER_ERROR,
        &local_type_unit_count,area_length,error);
    if (res != DW_DLV_OK) {
        return res;
    }
    res = read_uword_val(dbg, &curptr,
        end_dnames, DW_DLE_DEBUG_NAMES_HEADER_ERROR,
        &foreign_type_unit_count,area_length,error);
    if (res != DW_DLV_OK) {
        return res;
    }
    res = read_uword_val(dbg, &curptr,
        end_dnames, DW_DLE_DEBUG_NAMES_HEADER_ERROR,
        &bucket_count,area_length,error);
    if (res != DW_DLV_OK) {
        return res;
    }
    res = read_uword_val(dbg, &curptr,
        end_dnames, DW_DLE_DEBUG_NAMES_HEADER_ERROR,
        &name_count,area_length,error);
    if (res != DW_DLV_OK) {
        return res;
    }
    res = read_uword_val(dbg, &curptr,
        end_dnames, DW_DLE_DEBUG_NAMES_HEADER_ERROR,
        &abbrev_table_size,area_length,error);
    if (res != DW_DLV_OK) {
        return res;
    }
    res = read_uword_val(dbg, &curptr,
        end_dnames, DW_DLE_DEBUG_NAMES_HEADER_ERROR,
        &augmentation_string_size,area_length,error);
    if (res != DW_DLV_OK) {
        return res;
    }

    str_utf8 = (const char *) curptr;
    res = _dwarf_check_string_valid(dbg,curptr,curptr,end_dnames,
        DW_DLE_DEBUG_NAMES_AUG_STRING_ERROR,
        error);
    if (res != DW_DLV_OK) {
        return res;
    }

    curptr+= augmentation_string_size;

    di_header = (struct Dwarf_Dnames_index_header_s *)
        calloc(1,sizeof(*di_header));
    if(!di_header) {
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return (DW_DLV_ERROR);
    }


    di_header->din_dbg = dbg;
    di_header->din_indextable_data = past_length;
    di_header->din_indextable_length = area_length;
    di_header->din_version = version;
    di_header->din_comp_unit_count = comp_unit_count;
    di_header->din_local_type_unit_count = local_type_unit_count ;
    di_header->din_foreign_type_unit_count = foreign_type_unit_count ;
    di_header->din_bucket_count = bucket_count ;
    di_header->din_name_count = name_count ;
    di_header->din_abbrev_table_size = abbrev_table_size;
    di_header->din_augmentation_string_size = augmentation_string_size;
    di_header->din_augmentation_string      = str_utf8;
    /*  No terminating NUL is implied. Followed by 0-3 bytes
        of padding NULs */

    {
        Dwarf_Unsigned header_so_far = curptr - *curptr_in;
        unsigned count = header_so_far %4;
        Dwarf_Small *pad = curptr;
        Dwarf_Small *endpad = 0;
        
        curptr += count; 
        endpad = curptr;

        if(curptr > end_dnames) {
            free(di_header);
            _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
            return DW_DLV_ERROR;
        }
        for( ; pad < endpad; ++pad) {
            if(*pad) {
                /*  One could argue this is a harmless error, 
                    but for now assume it is real corruption. */
                free(di_header);
                _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_PAD_NON_ZERO);
                return DW_DLV_ERROR;
            }
        }
        
    }
    
    
    

    di_header->din_cu_list = curptr;
    curptr +=  dbg->de_length_size * comp_unit_count;
    if(curptr > end_dnames) {
        free(di_header);
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }
    di_header->din_local_tu_list = curptr;

    curptr +=  dbg->de_length_size * local_type_unit_count;
    if(curptr > end_dnames) {
        free(di_header);
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }
    di_header->din_foreign_tu_list = curptr;
   
    curptr +=  sizeof(Dwarf_Sig8) * foreign_type_unit_count;
    if(curptr > end_dnames) {
        free(di_header);
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }

    di_header->din_buckets = curptr;
    curptr +=  sizeof(Dwarf_ufixed) * bucket_count;
    if(curptr > end_dnames) {
        free(di_header);
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }

    di_header->din_hash_table = curptr;
    curptr +=  sizeof(Dwarf_Sig8) * name_count;
    if(curptr > end_dnames) {
        free(di_header);
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }

    di_header->din_string_offsets = curptr;
    curptr +=  sizeof(Dwarf_ufixed) * name_count;
    if(curptr > end_dnames) {
        free(di_header);
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }

    di_header->din_entry_offsets = curptr;
    curptr +=  sizeof(Dwarf_ufixed) * name_count;
    if(curptr > end_dnames) {
        free(di_header);
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }

    di_header->din_entry_pool = curptr;
    curptr +=   abbrev_table_size;
    if(curptr > end_dnames) {
        free(di_header);
        _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_HEADER_ERROR);
        return DW_DLV_ERROR;
    }
    *curptr_in = curptr; 
    *index_header_out = di_header;
    return DW_DLV_OK;
}



/*  There may be one debug index for an entire object file,
    for multiple CUs or there can be individual indexes
    for some CUs.  
    see DWARF5 6.1.1.3 Per_CU versus Per-Module Indexes. */
int dwarf_get_debugnames_header(Dwarf_Debug dbg,
    Dwarf_Dnames_Head * dn_out,
    Dwarf_Error *error)
{
    Dwarf_Unsigned remaining = 0;
    Dwarf_Dnames_Head dn_header = 0;
    Dwarf_Unsigned section_size;
    Dwarf_Small *start_section = 0;
    Dwarf_Small *end_section = 0;
    Dwarf_Small *curptr = 0;
    int res = 0;

    if(!dbg) {
        _dwarf_error(dbg, error,DW_DLE_DBG_NULL);
        return DW_DLV_ERROR;
    }

    res = _dwarf_load_section(dbg, &dbg->de_debug_names, error);
    if (res != DW_DLV_OK) {
        return res;
    }

    section_size = dbg->de_debug_names.dss_size;
    if(!section_size){
        return DW_DLV_NO_ENTRY;
    }
    start_section = dbg->de_debug_names.dss_data;
    curptr = start_section;
    end_section = start_section + section_size;
    remaining = section_size;
    dn_header =  (Dwarf_Dnames_Head)_dwarf_get_alloc(dbg, DW_DLA_DNAMES_HEAD, 1);    
    if(!dn_header) {
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return (DW_DLV_ERROR);
    }
    dn_header->dn_section_data = start_section;
    dn_header->dn_section_size = section_size;
    dn_header->dn_section_end = start_section + section_size;
    dn_header->dn_dbg = dbg;
    for( ; curptr < end_section; ) {
        struct Dwarf_Dnames_index_header_s * index_header = 0;
        Dwarf_Small *curptr_start = curptr;
        Dwarf_Unsigned usedspace = 0;

        res = read_a_name_index(dn_header,
            &curptr,
            end_section, 
            remaining,
            &index_header,
            error); 
        if (res == DW_DLV_ERROR) {
            dwarf_dealloc(dbg,dn_header,DW_DLA_DNAMES_HEAD);
            return res;
        }
        if (res == DW_DLV_NO_ENTRY) {
            /*  Impossible. A bug. Or possibly
                a bunch of zero pad? */
            dwarf_dealloc(dbg,dn_header,DW_DLA_DNAMES_HEAD);
            break;
        }
        /* Add the new one to the list. */
        if(!dn_header->dn_inhdr_first) {
            dn_header->dn_inhdr_count = 1;
            dn_header->dn_inhdr_first = index_header;
            dn_header->dn_inhdr_last = index_header;
        } else {
            struct Dwarf_Dnames_index_header_s *tmp = dn_header->dn_inhdr_last;
            dn_header->dn_inhdr_last = index_header;
            tmp->din_next = index_header;
            dn_header->dn_inhdr_count++;
        }
        usedspace = curptr - curptr_start;
        remaining -= - usedspace;
        if (remaining < 5) {
            /*  No more in here, just padding. Check for zero
                in padding. */
            if ((curptr +remaining) < end_section) {
                dwarf_dealloc(dbg,dn_header,DW_DLA_DNAMES_HEAD);
                _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_OFF_END);
                return DW_DLV_ERROR;
            }
            for ( ; curptr < end_section; ++curptr) {
                if(*curptr) {
                    /*  One could argue this is a harmless error,
                        but for now assume it is real corruption. */
                    dwarf_dealloc(dbg,dn_header,DW_DLA_DNAMES_HEAD);
                    _dwarf_error(dbg, error,DW_DLE_DEBUG_NAMES_PAD_NON_ZERO);
                    return DW_DLV_ERROR;
                }
            }
        }
        
    }
    *dn_out = dn_header;
    return DW_DLV_OK;
}


/*  To free any Dwarf_Dnames_Head_s data that is directly
    mallocd. */
void
_dwarf_dnames_destructor(void *m)
{
    struct Dwarf_Dnames_Head_s *h = (struct Dwarf_Dnames_Head_s *)m;

    struct Dwarf_Dnames_index_header_s *cur = 0;
    struct Dwarf_Dnames_index_header_s *tmp = 0;

    cur = h->dn_inhdr_first;
    for( ; cur; cur = tmp) {
        tmp = cur->din_next;
        free(cur);
    }
    h->dn_inhdr_first = 0;
    h->dn_inhdr_last = 0;
    h->dn_inhdr_count = 0;
}
