/*
  Copyright (C) 2000,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007-2016 David Anderson. All Rights Reserved.
  Portions Copyright 2012 SN Systems Ltd. All rights reserved.

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

#ifndef globals_INCLUDED
#define globals_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#if (!defined(HAVE_RAW_LIBELF_OK) && defined(HAVE_LIBELF_OFF64_OK) )
/* At a certain point libelf.h requires _GNU_SOURCE.
   here we assume the criteria in configure determine that
   usefully.
*/
#define _GNU_SOURCE 1
#endif

#include "warningcontrol.h"

/*  We want __uint32_t and __uint64_t and __int32_t __int64_t
    properly defined but not duplicated, since duplicate typedefs
    are not legal C.
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
#include <stdarg.h>   /* For va_start va_arg va_list */
#include <stdlib.h>
#include <string.h>

/* Windows specific */
#ifdef HAVE_STDAFX_H
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */

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

#include "checkutil.h"
#ifndef BOOLEAN_TYPEDEFED
#define BOOLEAN_TYPEDEFED
typedef int boolean;
#endif /* BOOLEAN_TYPEDEFED */
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FAILED
#define FAILED 1
#endif

/* Used to try to avoid leakage when we hide errors. */
#define DROP_ERROR_INSTANCE(d,r,e)       \
    if (r == DW_DLV_ERROR) {             \
        dwarf_dealloc(d,e,DW_DLA_ERROR); \
        e = 0;                           \
    }



/* size of attrib_buffer, defined in print_die.c */
#define ATTRIB_BUFSIZ 999

typedef struct {
    int checks;
    int errors;
} Dwarf_Check_Result;

/* Compilation Unit information for improved error messages.
   If the strings are too short we just truncate so fixed length
   here is fine.  */
#define COMPILE_UNIT_NAME_LEN 512
extern char PU_name[COMPILE_UNIT_NAME_LEN]; /* PU Name */
extern char CU_name[COMPILE_UNIT_NAME_LEN]; /* CU Name */
extern char CU_producer[COMPILE_UNIT_NAME_LEN];  /* CU Producer Name */

/* Set TRUE if you want to simply assume strings to be
   printed are safe to print. Leave FALSE if you want
   dangerous or unprintable characters to be switched to the
   character '?'. */
extern boolean no_sanitize_string_garbage;

extern boolean seen_PU;                     /* Detected a PU. */
extern boolean seen_CU;                     /* Detected a CU. */
extern boolean need_CU_name;                /* Need CU name. */
extern boolean need_CU_base_address;        /* Need CU Base address. */
extern boolean need_CU_high_address;        /* Need CU High address. */
extern boolean need_PU_valid_code;          /* Need PU valid code. */

extern boolean seen_PU_base_address;        /* Detected a Base address for PU */
extern boolean seen_PU_high_address;        /* Detected a High address for PU */
extern Dwarf_Addr PU_base_address;          /* PU Base address */
extern Dwarf_Addr PU_high_address;          /* PU High address */

extern Dwarf_Off  DIE_offset;               /* DIE offset in compile unit. */
extern Dwarf_Off  DIE_overall_offset;       /* DIE offset in .debug_info. */

/* Current CU information for better error reporting. */
extern Dwarf_Off  DIE_CU_offset;            /* CU DIE offset in compile unit */
extern Dwarf_Off  DIE_CU_overall_offset;    /* CU DIE offset in .debug_info */
extern Dwarf_Addr CU_base_address;          /* CU Base address. See ranges. */
extern Dwarf_Addr CU_low_address;           /* CU lowest addr. */
extern Dwarf_Addr CU_high_address;          /* CU High address. */

extern int current_section_id;              /* Section being process. */

/*  Ranges and Location tables for better error checking: see
    dwarfdump.c comments for more information. */
extern Bucket_Group *pRangesInfo;
extern Bucket_Group *pLinkonceInfo;
extern Bucket_Group *pVisitedInfo;

/* Calculate wasted space */
extern void calculate_attributes_usage(Dwarf_Half attr,Dwarf_Half theform,
    Dwarf_Unsigned value);

/* Able to generate report on search */
extern const char *search_any_text;
extern const char *search_match_text;
extern const char *search_regex_text;
extern int search_occurrences;
#ifdef HAVE_REGEX
extern regex_t search_re;
#endif
extern boolean is_strstrnocase(const char *data, const char *pattern);

/* Options to enable debug tracing. */
#define MAX_TRACE_LEVEL 10
extern int nTrace[MAX_TRACE_LEVEL + 1];

#define DUMP_OPTIONS                0   /* Dump options. */
#define DUMP_RANGES_INFO            1   /* Dump RangesInfo Table. */
#define DUMP_LOCATION_SECTION_INFO  2   /* Dump Location (.debug_loc) Info. */
#define DUMP_RANGES_SECTION_INFO    3   /* Dump Ranges (.debug_ranges) Info. */
#define DUMP_LINKONCE_INFO          4   /* Dump Linkonce Table. */
#define DUMP_VISITED_INFO           5   /* Dump Visited Info. */

#define dump_options                nTrace[DUMP_OPTIONS]
#define dump_ranges_info            nTrace[DUMP_RANGES_INFO]
#define dump_location_section_info  nTrace[DUMP_LOCATION_SECTION_INFO]
#define dump_ranges_section_info    nTrace[DUMP_RANGES_SECTION_INFO]
#define dump_linkonce_info          nTrace[DUMP_LINKONCE_INFO]
#define dump_visited_info           nTrace[DUMP_VISITED_INFO]

/* Section IDs */
#define DEBUG_ABBREV      1
#define DEBUG_ARANGES     2
#define DEBUG_FRAME       3
#define DEBUG_INFO        4
#define DEBUG_LINE        5
#define DEBUG_LOC         6
#define DEBUG_MACINFO     7
#define DEBUG_PUBNAMES    8
#define DEBUG_RANGES      9
#define DEBUG_STATIC_VARS 10
#define DEBUG_STATIC_FUNC 11
#define DEBUG_STR         12
#define DEBUG_WEAKNAMES   13
#define DEBUG_TYPES       14
#define DEBUG_GDB_INDEX   15
#define DEBUG_FRAME_EH_GNU 16
#define DEBUG_MACRO       17
#define DEBUG_NAMES       18

extern int verbose;
extern boolean dense;
extern boolean ellipsis;
extern boolean use_mips_regnames;
extern boolean show_form_used;

/* Print the information only if unique errors is set and it is first time */
#define PRINTING_UNIQUE (!glflags.gf_found_error_message)

extern int break_after_n_units;

/* Check categories corresponding to the -k option */
typedef enum /* Dwarf_Check_Categories */ {
    abbrev_code_result,
    pubname_attr_result,
    reloc_offset_result,
    attr_tag_result,
    tag_tree_result,
    type_offset_result,
    decl_file_result,
    ranges_result,
    lines_result,
    aranges_result,
    /*  Harmless errors are errors detected inside libdwarf but
        not reported via DW_DLE_ERROR returns because the errors
        won't really affect client code.  The 'harmless' errors
        are reported and otherwise ignored.  It is difficult to report
        the error when the error is noticed by libdwarf, the error
        is reported at a later time.
        The other errors dwarfdump reports are also generally harmless
        but are detected by dwarfdump so it's possble to report the
        error as soon as the error is discovered. */
    harmless_result,
    fde_duplication,
    frames_result,
    locations_result,
    names_result,
    abbreviations_result,
    dwarf_constants_result,
    di_gaps_result,
    forward_decl_result,
    self_references_result,
    attr_encoding_result,
    duplicated_attributes_result,
    total_check_result,
    LAST_CATEGORY  /* Must be last */
} Dwarf_Check_Categories;


struct section_high_offsets_s {
    Dwarf_Unsigned debug_info_size;
    Dwarf_Unsigned debug_abbrev_size;
    Dwarf_Unsigned debug_line_size;
    Dwarf_Unsigned debug_loc_size;
    Dwarf_Unsigned debug_aranges_size;
    Dwarf_Unsigned debug_macinfo_size;
    Dwarf_Unsigned debug_pubnames_size;
    Dwarf_Unsigned debug_str_size;
    Dwarf_Unsigned debug_frame_size;
    Dwarf_Unsigned debug_ranges_size;
    Dwarf_Unsigned debug_pubtypes_size;
    Dwarf_Unsigned debug_types_size;
    Dwarf_Unsigned debug_macro_size;
    Dwarf_Unsigned debug_str_offsets_size;
    Dwarf_Unsigned debug_sup_size;
    Dwarf_Unsigned debug_cu_index_size;
    Dwarf_Unsigned debug_tu_index_size;
};
extern struct section_high_offsets_s section_high_offsets_global;

enum line_flag_type_e {
  singledw5,   /* Meaning choose single table DWARF5 new interfaces. */
  s2l,   /* Meaning choose two-level DWARF5 new interfaces. */
  orig,  /* Meaning choose DWARF2,3,4 single level interface. */
  orig2l /* Meaning choose DWARF 2,3,4 two-level interface. */
};

#include "glflags.h"

/* tsearch tree used in macro checking. */
extern void *  macro_check_tree; /* DWARF5 macros. */
extern void *  macinfo_check_tree; /* DWARF2,3,4 macros */

extern Dwarf_Off fde_offset_for_cu_low;
extern Dwarf_Off fde_offset_for_cu_high;

/* Process TAGs for checking mode and reset pRangesInfo table
   if appropriate. */
extern void tag_specific_checks_setup(Dwarf_Half val,int die_indent_level);

extern const char *program_name;

extern void print_error_and_continue (Dwarf_Debug dbg, const char * msg,int res, Dwarf_Error err);
extern void print_error (Dwarf_Debug dbg, const char * msg,int res, Dwarf_Error err);

extern void print_line_numbers_this_cu (Dwarf_Debug dbg, Dwarf_Die in_die);

struct dwconf_s;
extern void print_frames (Dwarf_Debug dbg, int print_debug_frame,
    int print_eh_frame,struct dwconf_s *);
extern void print_ranges (Dwarf_Debug dbg);
extern void print_pubnames (Dwarf_Debug dbg);
extern void print_macinfo (Dwarf_Debug dbg);
extern void print_infos (Dwarf_Debug dbg,Dwarf_Bool is_info);
extern void print_locs (Dwarf_Debug dbg);
extern void print_abbrevs (Dwarf_Debug dbg);
extern void print_strings (Dwarf_Debug dbg);
extern void print_aranges (Dwarf_Debug dbg);
extern void print_relocinfo (Dwarf_Debug dbg,char *relmap);
extern void print_static_funcs(Dwarf_Debug dbg);
extern void print_static_vars(Dwarf_Debug dbg);
enum type_type_e {SGI_TYPENAME, DWARF_PUBTYPES} ;
extern void print_types(Dwarf_Debug dbg,enum type_type_e type_type);
extern void print_weaknames(Dwarf_Debug dbg);
extern void print_exception_tables(Dwarf_Debug dbg);
extern void print_debug_names(Dwarf_Debug dbg);

/*  Space used to record range information */
extern void allocate_range_array_info(void);
extern void release_range_array_info(void);
extern void record_range_array_info_entry(Dwarf_Off die_off,
    Dwarf_Off range_off);
extern void check_range_array_info(Dwarf_Debug dbg);

struct esb_s;
extern void print_ranges_list_to_extra(Dwarf_Debug dbg,
    Dwarf_Unsigned off,
    Dwarf_Ranges *rangeset,
    Dwarf_Signed rangecount,
    Dwarf_Unsigned bytecount,
    struct esb_s *stringbuf);
boolean should_skip_this_cu(Dwarf_Debug dbg, Dwarf_Die cu_die);

void get_address_size_and_max(Dwarf_Debug dbg,
   Dwarf_Half * size,
   Dwarf_Addr * max,
   Dwarf_Error *err);

/* Returns the producer of the CU */
int get_cu_name(Dwarf_Debug dbg,Dwarf_Die cu_die,
    Dwarf_Off  dieprint_cu_offset,
    char **short_name,char **long_name);
int get_producer_name(Dwarf_Debug dbg,Dwarf_Die cu_die,
    Dwarf_Off  dieprint_cu_offset,
    struct esb_s *producername);

/* Get number of abbreviations for a CU */
extern void get_abbrev_array_info(Dwarf_Debug dbg,Dwarf_Unsigned offset);
/* Validate an abbreviation */
extern void validate_abbrev_code(Dwarf_Debug dbg,Dwarf_Unsigned abbrev_code);

extern void print_die_and_children(
    Dwarf_Debug dbg,
    Dwarf_Die in_die,
    Dwarf_Off dieprint_cu_offset,
    Dwarf_Bool is_info,
    char **srcfiles,
    Dwarf_Signed cnt);
extern boolean print_one_die(
    Dwarf_Debug dbg,
    Dwarf_Die die,
    Dwarf_Off dieprint_cu_offset,
    boolean print_information,
    int die_indent_level,
    char **srcfiles,
    Dwarf_Signed cnt,
    boolean ignore_die_stack);

/* Check for specific compiler */
extern boolean checking_this_compiler(void);
extern void update_compiler_target(const char *producer_name);
extern void add_cu_name_compiler_target(char *name);

/*  General error reporting routines. These were
    macros for a short time and when changed into functions
    they kept (for now) their capitalization.
    The capitalization will likely change. */
extern void PRINT_CU_INFO(void);
extern void DWARF_CHECK_COUNT(Dwarf_Check_Categories category, int inc);
extern void DWARF_ERROR_COUNT(Dwarf_Check_Categories category, int inc);
extern void DWARF_CHECK_ERROR_PRINT_CU(void);
extern void DWARF_CHECK_ERROR(Dwarf_Check_Categories category,
    const char *str);
extern void DWARF_CHECK_ERROR2(Dwarf_Check_Categories category,
    const char *str1, const char *str2);
extern void DWARF_CHECK_ERROR3(Dwarf_Check_Categories category,
    const char *str1, const char *str2, const char *strexpl);

extern void print_macinfo_by_offset(Dwarf_Debug dbg,Dwarf_Unsigned offset);

struct esb_s;

/*  Eliminate control characters from the input,
    leaving the input unchanged. Return pointer to
    an ephemeral location (only callfor printf,
    and only once per printf! */
const char * sanitized(const char *s);
void sanitized_string_destructor(void);
void ranges_esb_string_destructor(void);
void destruct_abbrev_array(void);

extern Dwarf_Die current_cu_die_for_print_frames; /* This is
    an awful hack, making current_cu_die_for_print_frames public.
    But it enables cleaning up (doing all dealloc needed). */
/* defined in print_sections.c, die for the current compile unit,
   used in get_fde_proc_name() */

extern void printreg(Dwarf_Unsigned reg,struct dwconf_s *config_data);

int get_proc_name(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Addr low_pc,
    char *proc_name_buf, int proc_name_buf_len, void **pcMap);

void get_attr_value(Dwarf_Debug dbg, Dwarf_Half tag,
    Dwarf_Die die,
    Dwarf_Off die_cu_offset,
    Dwarf_Attribute attrib,
    char **srcfiles,
    Dwarf_Signed cnt, struct esb_s *esbp,
    int show_form,int local_verbose);

extern void dump_block(char *prefix, char *data, Dwarf_Signed len);

extern void format_sig8_string(Dwarf_Sig8 *data,struct esb_s *out);

extern void print_gdb_index(Dwarf_Debug dbg);
extern void print_debugfission_index(Dwarf_Debug dbg,const char *type);

void dwarfdump_print_one_locdesc(Dwarf_Debug dbg,
    Dwarf_Locdesc * llbuf, /* 2014 interface */
    Dwarf_Locdesc_c  locs, /* 2015 interface */
    Dwarf_Unsigned llent, /* Which locdesc is this */
    Dwarf_Unsigned entrycount, /* count of DW_OP operators */
    Dwarf_Addr baseaddr,
    struct esb_s *string_out);
void clean_up_die_esb(void);
void clean_up_syms_malloc_data(void);
void safe_strcpy(char *out, long outlen, const char *in, long inlen);

void print_macros_5style_this_cu(Dwarf_Debug dbg, Dwarf_Die cu_die,
    Dwarf_Bool in_import_list, Dwarf_Unsigned offset);

void reset_overall_CU_error_data(void);

void format_sig8_string(Dwarf_Sig8*data, struct esb_s *out);

/* Detailed attributes encoding space */
void print_attributes_encoding(Dwarf_Debug dbg);

/* Detailed tag and attributes usage */
void print_tag_attributes_usage(Dwarf_Debug dbg);

void print_section_groups_data(Dwarf_Debug dbg);
void update_section_flags_per_groups(Dwarf_Debug dbg);

void print_any_harmless_errors(Dwarf_Debug dbg);

#include "section_bitmaps.h"

#ifdef HAVE_UNUSED_ATTRIBUTE
#define  UNUSEDARG __attribute__ ((unused))
#else
#define  UNUSEDARG
#endif

#ifdef __cplusplus
}
#endif

#endif /* globals_INCLUDED */
