/* 
  Copyright (C) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
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
#include <string>
#include <iostream>
#include <sstream> // For IToDec
#include <iomanip> // For setw
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

typedef struct {
    int checks;
    int errors;
} Dwarf_Check_Result;

extern bool search_is_on;
extern std::string search_any_text;
extern std::string search_match_text;
extern std::string search_regex_text;
#ifdef HAVE_REGEX
extern regex_t search_re;
#endif
extern bool is_strstrnocase(const char * container, const char * contained);



extern int verbose;
extern bool dense;
extern bool ellipsis;
extern bool use_mips_regnames;
extern bool show_global_offsets;
extern bool show_form_used;

extern bool check_pubname_attr;
extern bool check_attr_tag;
extern bool check_tag_tree;
extern bool check_type_offset;
extern bool check_decl_file;
extern bool check_lines;
extern bool check_aranges;
extern bool check_harmless;
extern bool suppress_nested_name_search;
extern bool suppress_check_extensions_tables;
extern bool check_fdes;


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

extern bool info_flag;
extern bool line_flag;
extern bool use_old_dwarf_loclist;

extern std::string cu_name;
extern bool cu_name_flag;
extern Dwarf_Unsigned cu_offset;
extern Dwarf_Off fde_offset_for_cu_low;
extern Dwarf_Off fde_offset_for_cu_high;
extern std::string program_name;

extern int check_error;
extern Dwarf_Error err;
extern void print_error (Dwarf_Debug dbg, const std::string& msg,int res, Dwarf_Error err);
extern void print_error_and_continue (Dwarf_Debug dbg, const std::string& msg,int res, Dwarf_Error err);

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
extern std::string  print_ranges_list_to_extra(Dwarf_Debug dbg,
    Dwarf_Unsigned off,
    Dwarf_Ranges *rangeset,
    Dwarf_Signed rangecount,
    Dwarf_Unsigned bytecount);
extern bool should_skip_this_cu(DieHolder &cu_die, Dwarf_Error err);


extern void print_die_and_children(
        DieHolder &in_die,
        SrcfilesHolder &srcfiles);
extern bool print_one_die(
        DieHolder &hdie_in,
        bool print_information,
        int indent_level,
        SrcfilesHolder &srcfiles,
        bool ignore_die_printed_flag);

#define DWARF_CHECK_ERROR(var,str) {\
        var.errors++; \
        cout << "*** DWARF CHECK: " << str << " ***" << endl;\
        check_error ++; \
}

#define DWARF_CHECK_ERROR2(var,str1, str2) {\
        var.errors++; \
        cout << "*** DWARF CHECK: " << str1 << ": " << \
            str2 << " ***" << endl;\
        check_error ++; \
}

#define DWARF_CHECK_ERROR3(var,str1, str2,strexpl) {\
        var.errors++; \
        cout << "*** DWARF CHECK: " << str1 << " -> " << \
           str2 << ": " << strexpl << " ***" << endl;\
        check_error ++; \
}

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

extern void dump_block(const std::string &prefix, char *data, Dwarf_Signed len);

int
dwarfdump_print_one_locdesc(Dwarf_Debug dbg,
                         Dwarf_Locdesc * llbuf,
                         int skip_locdesc_header,
                         std::string &string_out);
void clean_up_syms_malloc_data();

void print_any_harmless_errors(Dwarf_Debug dbg);


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

