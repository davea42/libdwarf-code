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

/*  All the dwarfdump flags are gathered into a single
    global struct as it has been hard to know how many there
    were or what they were all for. */


struct glflags_s {

    /* This so both
        dwarf_loclist_n()  and dwarf_get_loclist_c()
        and the dwarf_loclist_from_expr
        variations can be
        tested. Defaults to new
        dwarf_get_loclist_c(). See -g option.
        original IRIX dwarf_loclist() no longer tested
        as of October 2015. */
    boolean gf_use_old_dwarf_loclist;
    enum line_flag_type_e gf_line_flag_selection;

    boolean gf_abbrev_flag;
    boolean gf_aranges_flag; /* .debug_aranges section. */
    boolean gf_debug_names_flag;
    boolean gf_eh_frame_flag;   /* GNU .eh_frame section. */
    boolean gf_frame_flag;      /* .debug_frame section. */
    boolean gf_gdbindex_flag;   /* .gdbindex section. */
    boolean gf_info_flag;  /* .debug_info */
    boolean gf_line_flag;
    boolean gf_line_print_pc;
    boolean gf_line_skeleton_flag;
    boolean gf_loc_flag;
    boolean gf_macinfo_flag; /* DWARF2,3,4. Old macro section*/
    boolean gf_macro_flag; /* DWARF5 */
    boolean gf_pubnames_flag;
    boolean gf_ranges_flag; /* .debug_ranges section. */
    boolean gf_reloc_flag;  /* Elf relocations, not DWARF. */
    boolean gf_static_func_flag;/* SGI only */
    boolean gf_static_var_flag; /* SGI only */
    boolean gf_string_flag;
    boolean gf_pubtypes_flag;   /* SGI only */
    boolean gf_types_flag; /* .debug_types, not all CU types */
    boolean gf_weakname_flag;   /* SGI only */

    boolean gf_header_flag; /* Control printing of Elf header. */
    boolean gf_section_groups_flag;

    boolean gf_producer_children_flag;   /* List of CUs per compiler */
    boolean gf_check_abbrev_code;
    boolean gf_check_pubname_attr;
    boolean gf_check_reloc_offset;
    boolean gf_check_attr_tag;
    boolean gf_check_tag_tree;
    boolean gf_check_type_offset;
    boolean gf_check_decl_file;
    boolean gf_check_macros;
    boolean gf_check_lines;
    boolean gf_check_fdes;
    boolean gf_check_ranges;
    boolean gf_check_aranges;
    boolean gf_check_harmless;
    boolean gf_check_abbreviations;
    boolean gf_check_dwarf_constants;
    boolean gf_check_di_gaps;
    boolean gf_check_forward_decl;
    boolean gf_check_self_references;
    boolean gf_check_attr_encoding;   /* Attributes encoding */
    boolean gf_generic_1200_regs;
    boolean gf_suppress_check_extensions_tables;
    boolean gf_check_duplicated_attributes;
    /* lots of checks make no sense on a dwp debugfission object. */
    boolean gf_suppress_checking_on_dwp;

    /*  suppress_nested_name_search is a band-aid.
        A workaround. A real fix for N**2 behavior is needed.  */
    boolean gf_suppress_nested_name_search;
    boolean gf_uri_options_translation;
    boolean gf_do_print_uri_in_input;

    /* Print global (unique) error messages */
    boolean gf_print_unique_errors;
    boolean gf_found_error_message;

    boolean gf_check_names;
    boolean gf_check_verbose_mode; /* During '-k' mode, display errors */
    boolean gf_check_frames;
    boolean gf_check_frames_extended;    /* Extensive frames check */
    boolean gf_check_locations;          /* Location list check */

    boolean gf_print_usage_tag_attr;      /* Print basic usage */
    boolean gf_print_usage_tag_attr_full; /* Print full usage */

    boolean gf_check_all_compilers;
    boolean gf_check_snc_compiler; /* Check SNC compiler */
    boolean gf_check_gcc_compiler;
    boolean gf_print_summary_all;

    /* The check and print flags here make it easy to
        allow check-only or print-only.  We no longer support
        check-and-print in a single run.  */
    boolean gf_do_check_dwarf;
    boolean gf_do_print_dwarf;
    boolean gf_check_show_results;  /* Display checks results. */
    boolean gf_record_dwarf_error;  /* A test has failed, this
        is normally set FALSE shortly after being set TRUE, it is
        a short-range hint we should print something we might not
        otherwise print (under the circumstances). */

    boolean gf_check_debug_names;

    /* Display parent/children when in wide format? */
    boolean gf_display_parent_tree;
    boolean gf_display_children_tree;
    int     gf_stop_indent_level;

    /* Print search results in wide format? */
    boolean gf_search_wide_format;

    /* -S option: strings for 'any' and 'match' */
    boolean gf_search_is_on;

    boolean gf_search_print_results;
    boolean gf_cu_name_flag;
    boolean gf_show_global_offsets;
    boolean gf_display_offsets;
};

extern struct glflags_s glflags;

extern void init_global_flags(void);


