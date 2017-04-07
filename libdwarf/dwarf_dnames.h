/*
  Copyright (C) 2017-2017 David Anderson. All Rights Reserved.

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


#define DWARF_DNAMES_VERSION5 5

struct Dwarf_Dnames_index_header_s {
    Dwarf_Debug      din_dbg;
    struct Dwarf_Dnames_index_header_s *din_next;

    /* For offset and pointer sanity calculations. */
    Dwarf_Small    * din_indextable_data;
    Dwarf_Unsigned   din_indextable_length;

    Dwarf_Unsigned   din_version;
    Dwarf_Unsigned   din_comp_unit_count;
    Dwarf_Unsigned   din_local_type_unit_count;
    Dwarf_Unsigned   din_foreign_type_unit_count;
    Dwarf_Unsigned   din_bucket_count;
    Dwarf_Unsigned   din_name_count;

    /*  Entry pool size,bytes. Referred to 
        in the standard as abbrev table size and
        abbrev table in one place and Entry Pool
        in another place. */
 
    Dwarf_Unsigned   din_abbrev_table_size; 
    Dwarf_Unsigned   din_augmentation_string_size;
    const char *     din_augmentation_string;

    Dwarf_Small *    din_cu_list;
    Dwarf_Small *    din_local_tu_list;
    Dwarf_Small *    din_foreign_tu_list;
    Dwarf_Small *    din_buckets;
    Dwarf_Small *    din_hash_table;
    Dwarf_Small *    din_string_offsets;
    Dwarf_Small *    din_entry_offsets;
    Dwarf_Small *    din_entry_pool;
};


struct Dwarf_Dnames_Head_s {
    Dwarf_Debug               dn_dbg;
    Dwarf_Small             * dn_section_data;
    Dwarf_Small             * dn_section_end;
    Dwarf_Unsigned            dn_section_size;
    unsigned                  dn_inhdr_count;
    struct Dwarf_Dnames_index_header_s * dn_inhdr_first;
    struct Dwarf_Dnames_index_header_s * dn_inhdr_last;
};

void _dwarf_dnames_destructor(void *m);
