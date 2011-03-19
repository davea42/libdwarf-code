/* 
  Copyright (C) 2000,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007-2010 David Anderson. All Rights Reserved.

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

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan



$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/globals.h,v 1.25 2006/04/17 00:09:56 davea Exp $ */

#ifndef globals_INCLUDED
#define globals_INCLUDED

#include "config.h"
#if (!defined(HAVE_RAW_LIBELF_OK) && defined(HAVE_LIBELF_OFF64_OK) )
/* At a certain point libelf.h requires _GNU_SOURCE.
   here we assume the criteria in configure determine that
   usefully.
*/
#define _GNU_SOURCE 1
#endif


/* We want __uint32_t and __uint64_t and __int32_t __int64_t
   properly defined but not duplicated, since duplicate typedefs
   are not legal C.
*/
/*
 HAVE___UINT32_T
 HAVE___UINT64_T will be set by configure if
 our 4 types are predefined in compiler
*/


#if (!defined(HAVE___UINT32_T)) && defined(HAVE_SGIDEFS_H)
#include <sgidefs.h> /* sgidefs.h defines them */
#define HAVE___UINT32_T 1
#define HAVE___UINT64_T 1
#endif



#if (!defined(HAVE___UINT32_T)) && defined(HAVE_SYS_TYPES_H) && defined(HAVE___UINT32_T_IN_SYS_TYPES_H)
#  include <sys/types.h>
/* we assume __[u]int32_t and __[u]int64_t defined 
   since __uint32_t defined in the sys/types.h in use */
#define HAVE___UINT32_T 1
#define HAVE___UINT64_T 1
#endif

#ifndef HAVE___UINT32_T
typedef int __int32_t;
typedef unsigned  __uint32_t;
#define HAVE___UINT32_T 1
#endif
#ifndef HAVE___UINT64_T
typedef long long __int64_t;
typedef unsigned long long  __uint64_t;
#define HAVE___UINT64_T 1
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_ELF_H
#include <elf.h>
#endif
#ifdef HAVE_LIBELF_H
#include <libelf.h>
#else
#ifdef HAVE_LIBELF_LIBELF_H
#include <libelf/libelf.h>
#endif
#endif
#include <dwarf.h>
#include <libdwarf.h>
#ifdef HAVE_REGEX
#include <regex.h>
#endif

typedef char * string;
typedef int boolean;
#ifndef FALSE
#define FALSE 0
#endif 
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FAILED
#define FAILED 1
#endif

/* size of attrib_buffer, defined in print_die.c */
#define ATTRIB_BUFSIZ 999

typedef struct {
    int checks;
    int errors;
} Dwarf_Check_Result;

extern boolean search_is_on;
extern char *search_any_text;
extern char *search_match_text;
extern char *search_regex_text;
#ifdef HAVE_REGEX
extern regex_t search_re;
#endif
extern boolean is_strstrnocase(const char *data, const char *pattern);

extern int verbose;
extern boolean dense;
extern boolean ellipsis;
extern boolean use_mips_regnames;
extern boolean show_global_offsets;
extern boolean show_form_used;

extern boolean check_pubname_attr;
extern boolean check_attr_tag;
extern boolean check_tag_tree;
extern boolean check_type_offset;
extern boolean check_decl_file;
extern boolean check_lines;
extern boolean check_fdes;
extern boolean check_aranges;
extern boolean check_harmless;
extern boolean suppress_nested_name_search;
extern boolean suppress_check_extensions_tables;

extern int break_after_n_units;

extern Dwarf_Check_Result abbrev_code_result;
extern Dwarf_Check_Result pubname_attr_result;
extern Dwarf_Check_Result reloc_offset_result;
extern Dwarf_Check_Result attr_tag_result;
extern Dwarf_Check_Result tag_tree_result;
extern Dwarf_Check_Result type_offset_result;
extern Dwarf_Check_Result decl_file_result;
extern Dwarf_Check_Result ranges_result;
extern Dwarf_Check_Result lines_result;
extern Dwarf_Check_Result aranges_result;
extern Dwarf_Check_Result harmless_result;
extern Dwarf_Check_Result fde_duplication;

extern boolean info_flag;
extern boolean line_flag;
extern boolean use_old_dwarf_loclist;

extern char cu_name[ ];
extern boolean cu_name_flag;
extern Dwarf_Unsigned cu_offset;
extern Dwarf_Off fde_offset_for_cu_low;
extern Dwarf_Off fde_offset_for_cu_high;


extern char *program_name;
extern int check_error;
extern Dwarf_Error err;
extern void print_error_and_continue (Dwarf_Debug dbg, string msg,int res, Dwarf_Error err);
extern void print_error (Dwarf_Debug dbg, string msg,int res, Dwarf_Error err);

extern void print_line_numbers_this_cu (Dwarf_Debug dbg, Dwarf_Die in_die);
struct dwconf_s;
extern void print_frames (Dwarf_Debug dbg, int print_debug_frame,
                int print_eh_frame,struct dwconf_s *);
extern void print_ranges (Dwarf_Debug dbg);
extern void print_pubnames (Dwarf_Debug dbg);
extern void print_macinfo (Dwarf_Debug dbg);
extern void print_infos (Dwarf_Debug dbg);
extern void print_locs (Dwarf_Debug dbg);
extern void print_abbrevs (Dwarf_Debug dbg);
extern void print_strings (Dwarf_Debug dbg);
extern void print_aranges (Dwarf_Debug dbg);
extern void print_relocinfo (Dwarf_Debug dbg);
extern void print_static_funcs(Dwarf_Debug dbg);
extern void print_static_vars(Dwarf_Debug dbg);
enum type_type_e {SGI_TYPENAME, DWARF_PUBTYPES} ;
extern void print_types(Dwarf_Debug dbg,enum type_type_e type_type);
extern void print_weaknames(Dwarf_Debug dbg);
extern void print_exception_tables(Dwarf_Debug dbg);
struct esb_s;
extern void print_ranges_list_to_extra(Dwarf_Debug dbg,
    Dwarf_Unsigned off,
    Dwarf_Ranges *rangeset,
    Dwarf_Signed rangecount,
    Dwarf_Unsigned bytecount,
    struct esb_s *stringbuf);
boolean should_skip_this_cu(Dwarf_Debug dbg, Dwarf_Die cu_die, Dwarf_Error err);


extern void print_die_and_children(
        Dwarf_Debug dbg, 
        Dwarf_Die in_die,
        char **srcfiles,
        Dwarf_Signed cnt);
extern boolean print_one_die(
        Dwarf_Debug dbg, 
        Dwarf_Die die, 
        boolean print_information,
        int die_indent_level,
        char **srcfiles,
        Dwarf_Signed cnt,
        boolean ignore_die_stack);

#define DWARF_CHECK_ERROR(var,str) {\
        var.errors++; \
        printf("*** DWARF CHECK: %s ***\n", str);\
        check_error ++; \
}

#define DWARF_CHECK_ERROR2(var,str1, str2) {\
        var.errors++; \
        printf("*** DWARF CHECK: %s: %s ***\n", str1, str2);\
        check_error ++; \
}

#define DWARF_CHECK_ERROR3(var,str1, str2,strexpl) {\
        var.errors++; \
        printf("*** DWARF CHECK: %s -> %s: %s ***\n", str1, str2,strexpl);\
        check_error ++; \
}

struct esb_s;
extern Dwarf_Die current_cu_die_for_print_frames; /* This is
        an awful hack, making this public. But it enables
        cleaning up (doing all dealloc needed). */
extern void printreg(Dwarf_Signed reg,struct dwconf_s *config_data);
extern void print_frame_inst_bytes(Dwarf_Debug dbg,
                       Dwarf_Ptr cie_init_inst, Dwarf_Signed len,
                       Dwarf_Signed data_alignment_factor,
                       int code_alignment_factor, Dwarf_Half addr_size,
                        struct dwconf_s *config_data);


extern Dwarf_Unsigned local_dwarf_decode_u_leb128(unsigned char *leb128,
                            unsigned int *leb128_length);

extern Dwarf_Signed local_dwarf_decode_s_leb128(unsigned char *leb128,
                            unsigned int *leb128_length);

extern void dump_block(char *prefix, char *data, Dwarf_Signed len);

int
dwarfdump_print_one_locdesc(Dwarf_Debug dbg,
                         Dwarf_Locdesc * llbuf,
                         int skip_locdesc_header,
                         struct esb_s *string_out);
void clean_up_die_esb();
void clean_up_syms_malloc_data();

void print_any_harmless_errors(Dwarf_Debug dbg);




#endif /* globals_INCLUDED */
