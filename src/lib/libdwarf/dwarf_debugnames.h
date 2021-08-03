/*
  Copyright (C) 2017-2021 David Anderson. All Rights Reserved.

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

/*  Only 5 abbrev DW_IDX defined, we
    allow three user defined (arbitrarily) */
#define ABB_PAIRS_MAX 8

struct abbrev_pair_s {
    unsigned ap_index;
    unsigned ap_form;
};

#if 0
struct Dwarf_D_Abbrev_s {
    struct Dwarf_D_Abbrev_s * da_next;
    unsigned da_abbrev_code;
    unsigned da_tag;
    unsigned da_pairs_count;
    struct abbrev_pair_s da_pairs[ABB_PAIRS_MAX];
};
#endif

#define DWARF_DNAMES_VERSION5 5
#define DWARF_DNAMES_MAGIC  0xabcd

/* All offsets section global */
struct Dwarf_Dnames_Head_s {
    Dwarf_Unsigned   dn_magic;
    Dwarf_Debug      dn_dbg;
    /* For entire section */
    Dwarf_Small      * dn_section_data;
    Dwarf_Small      * dn_section_end;
    Dwarf_Unsigned   dn_section_size;

    /* For this names table set of data */
    Dwarf_Unsigned dn_section_offset; /* unit length offset*/
    Dwarf_Small  * dn_indextable_data; /* unit length ptr */
    Dwarf_Unsigned dn_unit_length;
    Dwarf_Small  * dn_indextable_data_end;
    Dwarf_Unsigned dn_next_set_offset;
    Dwarf_Half     dn_offset_size;
    Dwarf_Half     dn_version;

    Dwarf_Unsigned dn_comp_unit_count;
    Dwarf_Unsigned dn_local_type_unit_count;
    Dwarf_Unsigned dn_foreign_type_unit_count;
    Dwarf_Unsigned dn_bucket_count;
    /*  dn_name_count gives the size of
        the dn_string_offsets and dn_entry_offsets arrays,
        and if hashes present, the size of the
        dn_hash_table array. */
    Dwarf_Unsigned dn_name_count;
    Dwarf_Unsigned dn_abbrev_table_size;   /* bytes */
    Dwarf_Unsigned dn_entry_pool_size;   /* bytes */
    Dwarf_Unsigned dn_augmentation_string_size;
    char *   dn_augmentation_string; /* local copy */
    /*Offsets are non-decreasing (even empty tables */
    Dwarf_Unsigned dn_cu_list_offset;
    Dwarf_Unsigned dn_local_tu_list_offset;
    Dwarf_Unsigned dn_foreign_tu_list_offset;
    Dwarf_Unsigned dn_buckets_offset;
    Dwarf_Unsigned dn_hash_table_offset;
    Dwarf_Unsigned dn_string_offsets_offset;
    Dwarf_Unsigned dn_entry_offsets_offset;
    Dwarf_Unsigned dn_abbrevs_offset;
    Dwarf_Unsigned dn_entry_pool_offset;
    /* pointers non-decreasing (even empty tables) */
    Dwarf_Small *  dn_cu_list;
    Dwarf_Small *  dn_local_tu_list;
    Dwarf_Small *  dn_foreign_tu_list;
    Dwarf_Small *  dn_buckets;
    Dwarf_Small *  dn_hash_table;
    Dwarf_Small *  dn_string_offsets;
    Dwarf_Small *  dn_entry_offsets;
    Dwarf_Small *  dn_abbrevs;
    Dwarf_Small *  dn_entry_pool;
    /* Array of Dwarf_Dnames_Bucket_s sorted by bucket value */
    struct Dwarf_Dnames_Bucket_s * dn_bucket_sort;
    Dwarf_Unsigned b_value;
    Dwarf_Unsigned b_orig_bucket_index;
    Dwarf_Unsigned b_sorted_bucket_index;
};

void _dwarf_dnames_destructor(void *m);
void dwarf_dealloc_dnames(Dwarf_Dnames_Head dn);
