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


struct Dwarf_Dnames_Header_s {
    Dwarf_Debug      dn_dbg;

    /* For offset and pointer sanity calculations. */
    Dwarf_Small    * dn_section_data;
    Dwarf_Unsigned   dn_section_length;

    Dwarf_Unsigned   dn_version;
    Dwarf_Unsigned   dn_comp_unit_count;
    Dwarf_Unsigned   dn_local_type_unit_count;
    Dwarf_Unsigned   dn_foreign_type_unit_count;
    Dwarf_Unsigned   dn_bucket_count;
    Dwarf_Unsigned   dn_name_count;
    Dwarf_Unsigned   dn_abbrev_table_size;
    Dwarf_Unsigned   dn_augmentation_string_size;
    const char *     dn_augmentation_string;

    Dwarf_Small *    dn_cu_list;
    Dwarf_Small *    dn_local_tu_list;
    Dwarf_Small *    dn_foreign_tu_list;
    Dwarf_Small *    dn_hash_table;
    Dwarf_Small *    dn_name_table;
    Dwarf_Small *    dn_abbrev_table;
    Dwarf_Small *    dn_entry_pool;
};
