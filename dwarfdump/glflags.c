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

#include "globals.h"

void
init_global_flags(void)
{
    glflags.gf_debug_names_flag = FALSE;
    glflags.gf_info_flag = FALSE;
    glflags.gf_use_old_dwarf_loclist = FALSE;  /* This so both
        dwarf_loclist_n()  and dwarf_get_loclist_c()
        and the dwarf_loclist_from_expr
        variations can be
        tested. Defaults to new
        dwarf_get_loclist_c(). See -g option.
        The original IRIX dwarf_loclist() no longer tested
        as of October 2015. */

    glflags.gf_line_flag_selection = s2l;
    glflags.gf_line_flag = FALSE;

    /*  Setting this FALSE tells dwarfdump to use the old
        line-table interfaces.  using:
            -x line5=no
        The new interfaces allow for both two-level line tables
        and access to line table headers in case
        we have a DWARF5 skeleton line table (a line table
        header with no lines).
        */
    glflags.gf_line_skeleton_flag = TRUE;
    glflags.gf_line_print_pc = TRUE;    /* Print <pc> addresses. */
    glflags.gf_abbrev_flag = FALSE;
    glflags.gf_frame_flag = FALSE;      /* .debug_frame section. */
    glflags.gf_eh_frame_flag = FALSE;   /* GNU .eh_frame section. */
    glflags.gf_pubnames_flag = FALSE;
    glflags.gf_macinfo_flag = FALSE; /* DWARF2,3,4. Old macro section*/
    glflags.gf_macro_flag = FALSE; /* DWARF5(and DWARF4 extension) new macro section */
    glflags.gf_loc_flag = FALSE;
    glflags.gf_aranges_flag = FALSE; /* .debug_aranges section. */
    glflags.gf_ranges_flag = FALSE; /* .debug_ranges section. */
    glflags.gf_string_flag = FALSE;
    glflags.gf_reloc_flag = FALSE;
    glflags.gf_static_func_flag = FALSE;
    glflags.gf_static_var_flag = FALSE;
    glflags.gf_types_flag = FALSE;
    glflags.gf_weakname_flag = FALSE;
    glflags.gf_header_flag = FALSE; /* Control printing of Elf header. */
    glflags.gf_gdbindex_flag = FALSE;
    glflags.gf_producer_children_flag = FALSE;   /* List of CUs per compiler */
    glflags.gf_check_abbrev_code = FALSE;
    glflags.gf_check_pubname_attr = FALSE;
    glflags.gf_check_reloc_offset = FALSE;
    glflags.gf_check_attr_tag = FALSE;
    glflags.gf_check_tag_tree = FALSE;
    glflags.gf_check_type_offset = FALSE;
    glflags.gf_check_decl_file = FALSE;
    glflags.gf_check_macros = FALSE;
    glflags.gf_check_lines = FALSE;
    glflags.gf_check_fdes = FALSE;
    glflags.gf_check_ranges = FALSE;
    glflags.gf_check_aranges = FALSE;
    glflags.gf_check_harmless = FALSE;
    glflags.gf_check_abbreviations = FALSE;
    glflags.gf_check_dwarf_constants = FALSE;
    glflags.gf_check_di_gaps = FALSE;
    glflags.gf_check_forward_decl = FALSE;
    glflags.gf_check_self_references = FALSE;
    glflags.gf_check_attr_encoding = FALSE;   /* Attributes encoding */
    glflags.gf_generic_1200_regs = FALSE;
    glflags.gf_suppress_check_extensions_tables = FALSE;
    glflags.gf_check_duplicated_attributes = FALSE;

    /* lots of checks make no sense on a dwp debugfission object. */
    glflags.gf_suppress_checking_on_dwp = FALSE;

    /*  suppress_nested_name_search is a band-aid.
        A workaround. A real fix for N**2 behavior is needed.  */
    glflags.gf_suppress_nested_name_search = FALSE;
    glflags.gf_uri_options_translation = TRUE;
    glflags.gf_do_print_uri_in_input = TRUE;

    glflags.gf_print_unique_errors = FALSE;
    glflags.gf_found_error_message = FALSE;

    glflags.gf_check_names = FALSE;

    /* During '-k' mode, display errors */
    glflags.gf_check_verbose_mode = TRUE;

    glflags.gf_check_frames = FALSE;

    /* Extensive frames check */
    glflags.gf_check_frames_extended = FALSE;

    glflags.gf_check_locations      = FALSE;
    glflags.gf_print_usage_tag_attr = FALSE;
    glflags.gf_print_usage_tag_attr_full = FALSE;
    glflags.gf_check_all_compilers  = TRUE;
    glflags.gf_check_snc_compiler   = FALSE;
    glflags.gf_check_gcc_compiler   = FALSE;
    glflags.gf_print_summary_all    = FALSE;

    /*  The check and print flags here make it easy to
        allow check-only or print-only.  We no longer support
        check-and-print in a single run.  */
    glflags.gf_do_check_dwarf       = FALSE;
    glflags.gf_do_print_dwarf       = FALSE;
    glflags.gf_check_show_results   = FALSE;
    glflags.gf_record_dwarf_error   = FALSE;  /* A test has failed, this
        is normally set FALSE shortly after being set TRUE, it is
        a short-range hint we should print something we might not
        otherwise print (under the circumstances). */

    glflags.gf_check_debug_names    = FALSE;

    /* Display parent/children when in wide format? */
    glflags.gf_display_parent_tree  = FALSE;
    glflags.gf_display_children_tree = FALSE;
    glflags.gf_stop_indent_level    = 0;

    /* Print search results in wide format? */
    glflags.gf_search_wide_format   = FALSE;
    /* -S option: strings for 'any' and 'match' */
    glflags.gf_search_is_on         = FALSE;
    glflags.gf_search_print_results = FALSE;
    glflags.gf_cu_name_flag         = FALSE;
    glflags.gf_show_global_offsets  = FALSE;
    glflags.gf_display_offsets      = TRUE;
}
