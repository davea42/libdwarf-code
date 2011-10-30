/* 
  Copyright (C) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007-2011 David Anderson. All Rights Reserved.
  Portions Copyright (C) 2011 SN Systems Ltd. All Rights Reserved.

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
/*  At a certain point libelf.h requires _GNU_SOURCE.
    here we assume the criteria in configure determine that
    usefully.
*/
#define _GNU_SOURCE 1
#endif


/*  We want __uint32_t and __uint64_t and __int32_t __int64_t
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
/*  We assume __[u]int32_t and __[u]int64_t defined 
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
#include <string>
#include <iostream>
#include <sstream> // For IToDec
#include <iomanip> // For setw
#include <list> 
#include <map> 
#include <set> 
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

#ifndef FAILED
#define FAILED 1
#endif

#include "dieholder.h"
#include "srcfilesholder.h"
#include "checkutil.h"

struct Dwarf_Check_Result {
    Dwarf_Check_Result ():checks_(0),errors_(0) {};
    ~Dwarf_Check_Result() {};
    int checks_;
    int errors_;
};

extern bool search_is_on;
extern std::string search_any_text;
extern std::string search_match_text;
extern std::string search_regex_text;
#ifdef HAVE_REGEX
extern regex_t search_re;
#endif
extern bool is_strstrnocase(const char * container, const char * contained);


extern bool do_check_dwarf;
extern bool do_print_dwarf;
extern bool record_dwarf_error;  /* A test has failed, this
  is normally set FALSE shortly after being set TRUE, it is
  a short-range hint we should print something we might not
  otherwise print (under the circumstances). */

// Compilation Unit information for improved error messages.
struct Error_Message_Data {
    Error_Message_Data():
        seen_PU(false), 
        seen_CU(false),
        need_CU_name(false), 
        need_CU_base_address(false), 
        need_CU_high_address(false), 
        need_PU_valid_code(false),
        seen_PU_base_address(false),
        seen_PU_high_address(false),
        PU_base_address(0),
        PU_high_address(0),
        DIE_offset(0),
        DIE_overall_offset(0),
        DIE_CU_offset(0),
        DIE_CU_overall_offset(0),
        current_section_id(0),
        CU_base_address(0),
        CU_high_address(0),
        elf_max_address(0),
        elf_address_size(0)
        {};
    ~Error_Message_Data() {};
    std::string PU_name;
    std::string CU_name;
    std::string CU_producer;
    bool seen_PU;              // Detected a PU. 
    bool seen_CU;              // Detected a CU. 
    bool need_CU_name;          
    bool need_CU_base_address; // Need CU Base address.
    bool need_CU_high_address; // Need CU High address.
    bool need_PU_valid_code;   // Need PU valid code. 
    
    bool seen_PU_base_address; // Detected a Base address for PU 
    bool seen_PU_high_address; // Detected a High address for PU 
    Dwarf_Addr PU_base_address;// PU Base address 
    Dwarf_Addr PU_high_address;// PU High address 
    
    Dwarf_Off  DIE_offset;     // DIE offset in compile unit. 
    Dwarf_Off  DIE_overall_offset;  // DIE offset in .debug_info. 
    
    Dwarf_Off  DIE_CU_offset;  // CU DIE offset in compile unit 
    Dwarf_Off  DIE_CU_overall_offset; // CU DIE offset in .debug_info 
    int current_section_id;    // Section being process. 
    
    Dwarf_Addr CU_base_address;// CU Base address. 
    Dwarf_Addr CU_high_address;// CU High address. 
    
    Dwarf_Addr elf_max_address;// Largest representable  address offset. 
    Dwarf_Half elf_address_size;// Target pointer size. 
};
extern struct Error_Message_Data error_message_data;

//  Display parent/children when in wide format.
extern bool display_parent_tree;
extern bool display_children_tree;
extern int stop_indent_level;

//  Print search results when in wide format. 
extern bool search_wide_format;
extern bool search_is_on;

extern std::string search_any_text;
extern std::string search_match_text;
extern std::string search_regex_text;
#ifdef HAVE_REGEX
extern regex_t search_re;
#endif
extern bool is_strstrnocase(const char *data, const char *pattern);

// Options to enable debug tracing. 
#define MAX_TRACE_LEVEL 10
extern int nTrace[MAX_TRACE_LEVEL + 1];
#define DUMP_RANGES_INFO            1 // Dump RangesInfo Table.
#define DUMP_LOCATION_SECTION_INFO  2 // Dump Location (.debug_loc) Info.
#define DUMP_RANGES_SECTION_INFO    3 // Dump Ranges (.debug_ranges) Info.
#define DUMP_LINKONCE_INFO          4 // Dump Linkonce Table.
#define DUMP_VISITED_INFO           5 // Dump Visited Info.

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

extern int verbose;
extern bool dense;
extern bool ellipsis;
extern bool use_mips_regnames;
extern bool show_global_offsets;
extern bool show_form_used;
extern bool display_offsets;

extern bool check_pubname_attr;
extern bool check_attr_tag;
extern bool check_tag_tree;
extern bool check_type_offset;
extern bool check_decl_file;
extern bool check_lines;
extern bool check_ranges;
extern bool check_fdes;
extern bool check_aranges;
extern bool check_harmless;
extern bool check_abbreviations;
extern bool check_dwarf_constants;
extern bool check_di_gaps;
extern bool check_forward_decl;
extern bool check_self_references;
extern bool suppress_nested_name_search;
extern bool suppress_check_extensions_tables;

extern int break_after_n_units;

extern bool check_names;          // Check for invalid names.
extern bool check_verbose_mode;   // During '-k' mode, display errors.
extern bool check_frames;         // Frames check.
extern bool check_frames_extended;// Extensive frames check. 
extern bool check_locations;      // Location list check.


// Check categories corresponding to the -k option 
enum Dwarf_Check_Categories{ // Dwarf_Check_Categories 
    abbrev_code_result, // 0
    pubname_attr_result,
    reloc_offset_result,
    attr_tag_result,
    tag_tree_result,
    type_offset_result, // 5
    decl_file_result,
    ranges_result,
    lines_result,       //8
    aranges_result,
    //  Harmless errors are errors detected inside libdwarf but
    //  not reported via DW_DLE_ERROR returns because the errors
    //  won't really affect client code.  The 'harmless' errors
    //  are reported and otherwise ignored.  It is difficult to report
    //  the error when the error is noticed by libdwarf, the error
    //  is reported at a later time.
    //  The other errors dwarfdump reports are also generally harmless 
    //  but are detected by dwarfdump so it's possble to report the
    //  error as soon as the error is discovered. 
    harmless_result,   //10
    fde_duplication,
    frames_result,
    locations_result,
    names_result,
    abbreviations_result, // 15
    dwarf_constants_result,
    di_gaps_result,
    forward_decl_result,
    self_references_result,
    total_check_result,  //20
    LAST_CATEGORY  // Must be last.
} ;


extern bool info_flag;
extern bool line_flag;
extern bool use_old_dwarf_loclist;
extern bool producer_children_flag;   // List of CUs per compiler 

extern std::string cu_name;
extern bool cu_name_flag;
extern Dwarf_Unsigned cu_offset;
extern Dwarf_Off fde_offset_for_cu_low;
extern Dwarf_Off fde_offset_for_cu_high;

/*  Process TAGs for checking mode and reset pRangesInfo table 
    if appropriate. */
extern void tag_specific_checks_setup(Dwarf_Half val,int die_indent_level);

extern std::string program_name;
extern Dwarf_Error err;

extern void print_error_and_continue (Dwarf_Debug dbg, const std::string& msg,int res, Dwarf_Error err);
extern void print_error (Dwarf_Debug dbg, const std::string& msg,int res, Dwarf_Error err);

// The dwarf_names_print_on_error is so other apps (tag_tree.cc)
// can use the generated code in dwarf_names.cc (etc) easily.
// It is not ever set false in dwarfdump.
extern bool dwarf_names_print_on_error;

extern void print_line_numbers_this_cu (DieHolder &hdie);
struct dwconf_s;
extern void print_frames (Dwarf_Debug dbg, int print_debug_frame,
    int print_eh_frame,struct dwconf_s *);
extern void print_ranges (Dwarf_Debug dbg);
extern void print_pubnames (Dwarf_Debug dbg);
extern void print_macinfo (Dwarf_Debug dbg);
extern void print_infos (Dwarf_Debug dbg,bool is_info);
extern void print_locs (Dwarf_Debug dbg);
extern void print_abbrevs (Dwarf_Debug dbg);
extern void print_strings (Dwarf_Debug dbg);
extern void print_aranges (Dwarf_Debug dbg);
extern void print_relocinfo (Dwarf_Debug dbg, unsigned reloc_map);
extern void print_static_funcs(Dwarf_Debug dbg);
extern void print_static_vars(Dwarf_Debug dbg);
enum type_type_e {SGI_TYPENAME, DWARF_PUBTYPES} ;
extern void print_types(Dwarf_Debug dbg,enum type_type_e type_type);
extern void print_weaknames(Dwarf_Debug dbg);
extern void print_exception_tables(Dwarf_Debug dbg);
struct esb_s;
extern std::string  print_ranges_list_to_extra(Dwarf_Debug dbg,
    Dwarf_Unsigned off,
    Dwarf_Ranges *rangeset,
    Dwarf_Signed rangecount,
    Dwarf_Unsigned bytecount);
extern bool should_skip_this_cu(DieHolder &cu_die, Dwarf_Error err);

int get_cu_name(DieHolder &hcu_die,
    Dwarf_Error err,std::string &short_name_out,std::string &long_name_out);
int get_producer_name(DieHolder &hcu_die,
    Dwarf_Error err,std::string &producer_name_out);

/* Get number of abbreviations for a CU */
extern void get_abbrev_array_info(Dwarf_Debug dbg,Dwarf_Unsigned offset);
/* Validate an abbreviation */
extern void validate_abbrev_code(Dwarf_Debug dbg,Dwarf_Unsigned abbrev_code);

extern void print_die_and_children(
    DieHolder &in_die,
    Dwarf_Bool is_info,
    SrcfilesHolder &srcfiles);
extern bool print_one_die(
    DieHolder &hdie_in,
    bool print_information,
    int indent_level,
    SrcfilesHolder &srcfiles,
    bool ignore_die_printed_flag);

// Check for specific compiler.
extern bool checking_this_compiler();
extern void update_compiler_target(const std::string & producer_name);
extern void add_cu_name_compiler_target(const std::string &name);


/*  General error reporting routines. These were
    macros for a short time and when changed into functions 
    they kept (for now) their capitalization. 
    The capitalization will likely change. */
extern void PRINT_CU_INFO();
extern void DWARF_CHECK_COUNT(Dwarf_Check_Categories category, int inc);
extern void DWARF_ERROR_COUNT(Dwarf_Check_Categories category, int inc);
extern void DWARF_CHECK_ERROR_PRINT_CU();
extern void DWARF_CHECK_ERROR(Dwarf_Check_Categories category,
    const std::string &str);
extern void DWARF_CHECK_ERROR2(Dwarf_Check_Categories category,
    const std::string &str1, 
    const std::string &str2);
extern void DWARF_CHECK_ERROR3(Dwarf_Check_Categories category,
    const std::string & str1, 
    const std::string & str2, 
    const std::string & strexpl);


extern void printreg(Dwarf_Signed reg,struct dwconf_s *config_data);
extern void print_frame_inst_bytes(Dwarf_Debug dbg,
    Dwarf_Ptr cie_init_inst, Dwarf_Signed len,
    Dwarf_Signed data_alignment_factor,
    int code_alignment_factor, Dwarf_Half addr_size,
    struct dwconf_s *config_data);

bool
get_proc_name(Dwarf_Debug dbg, Dwarf_Die die,
    std::string & proc_name, Dwarf_Addr & low_pc_out);


void get_attr_value(Dwarf_Debug dbg, Dwarf_Half tag,
   Dwarf_Die die,
   Dwarf_Attribute attrib,
   SrcfilesHolder &srcfiles,
   std::string &str_out,bool show_form,
   int local_verbose);



extern Dwarf_Unsigned local_dwarf_decode_u_leb128(unsigned char *leb128,
    unsigned int *leb128_length);

extern Dwarf_Signed local_dwarf_decode_s_leb128(unsigned char *leb128,
    unsigned int *leb128_length);

extern void dump_block(const std::string &prefix, char *data, Dwarf_Signed len);

extern void format_sig8_string(Dwarf_Sig8 *data,std::string &out);

int
dwarfdump_print_one_locdesc(Dwarf_Debug dbg,
    Dwarf_Locdesc * llbuf,
    int skip_locdesc_header,
    std::string &string_out);
void clean_up_syms_malloc_data();

void print_any_harmless_errors(Dwarf_Debug dbg);

/* Definitions for printing relocations.  */
#define DW_SECTION_REL_DEBUG_INFO     0
#define DW_SECTION_REL_DEBUG_LINE     1
#define DW_SECTION_REL_DEBUG_PUBNAMES 2
#define DW_SECTION_REL_DEBUG_ABBREV   3
#define DW_SECTION_REL_DEBUG_ARANGES  4
#define DW_SECTION_REL_DEBUG_FRAME    5
#define DW_SECTION_REL_DEBUG_LOC      6
#define DW_SECTION_REL_DEBUG_RANGES   7
#define DW_SECTION_REL_DEBUG_TYPES    8

template <typename T >
std::string IToDec(T v,unsigned l=0) 
{
    std::ostringstream s;
    if (l > 0) {
        s << std::setw(l) << v;
    } else {
        s << v ;
    }
    return s.str();
};
template <typename T >
std::string IToHex(T v,unsigned l=0) 
{
    if(v == 0) {
        // For a zero value, above does not insert 0x.
        // So we do zeroes here.
        std::string out = "0x0";
        if(l > 3)  {
            out.append(l-3,'0');
        }
        return out;
    }
    std::ostringstream s;
    s.setf(std::ios::hex,std::ios::basefield); 
    s.setf(std::ios::showbase); 
    if (l > 0) {
        s << std::setw(l);
    }
    s << v ;
    return s.str();
};

inline std::string IToHex02(unsigned v)
{
    std::ostringstream s;
    // NO showbase here.
    s.setf(std::ios::hex,std::ios::basefield); 
    s << std::setfill('0');
    s << std::setw(2) << (0xff & v);
    return s.str();
}
template <typename T>
std::string IToHex0N(T v,unsigned len=0)
{
    std::ostringstream s;
    s.setf(std::ios::hex,std::ios::basefield); 
    //s.setf(std::ios::showbase); 
    s << std::setfill('0');
    if(len > 2 ) {
        s << std::setw(len-2) << v;
    } else {
        s << v;
    }
    return std::string("0x") + s.str();
}
template <typename T>
std::string IToDec0N(T v,unsigned len=0)
{
    std::ostringstream s;
    if (v < 0 && len > 2 ) {
        // Special handling for negatives: 
        // 000-27 is not what we want for example.
        s << v; 
        // ASSERT: s.str().size() >= 1
        if( len > ((s.str().size()))) {
            // Ignore the leading - and take the rest.
            std::string rest = s.str().substr(1);
            std::string::size_type zeroscount = len - (rest.size()+1); 
            std::string final;
            if(zeroscount > 0) {
                final.append(zeroscount,'0');
                final.append(rest);
            } else {
                final = rest;
            }
            return std::string("-") + final;
        } 
        return s.str();
    }
    s << std::setfill('0');
    if(len > 0) {
        s << std::setw(len) << v;
    } else {
        s << v;
    }
    return s.str();
}
inline std::string LeftAlign(unsigned minlen,const std::string &s)
{
    if(minlen <= s.size()) {
        return s;
    }
    std::string out = s;
    std::string::size_type spaces = minlen - out.size(); 
    out.append(spaces,' ');
    return out;
}

inline std::string SpaceSurround(const std::string &s) 
{
    std::string out(" ");
    out.append(s);
    out.append(" ");
    return out;
};
inline std::string BracketSurround(const std::string &s) 
{
    std::string out("<");
    out.append(s);
    out.append(">");
    return out;
};


#endif /* globals_INCLUDED */

