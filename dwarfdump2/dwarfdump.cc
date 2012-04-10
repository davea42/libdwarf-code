/* 
  Copyright (C) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007-2011 David Anderson. All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  

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


$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/dwarfdump.c,v 1.48 2006/04/18 18:05:57 davea Exp $ */

/* The address of the Free Software Foundation is
   Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
   Boston, MA 02110-1301, USA.
   SGI has moved from the Crittenden Lane address.
*/



#include "globals.h"
#include <vector>
#include <algorithm> // for sort
#include <iomanip>

/* for 'open' */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>             /* For getopt. */
#include "dwconf.h"
#include "common.h"
#include "naming.h"
#include "uri.h"
#define DWARFDUMP_VERSION " Tue Apr 10 11:43:32 PDT 2012  "

using std::string;
using std::cout;
using std::cerr;
using std::endl;

#define OKAY 0
#define BYTES_PER_INSTRUCTION 4

extern char *optarg;
static string process_args(int argc, char *argv[]);
static void increment_compilers_detected(bool beyond);
static void increment_compilers_targeted(bool beyond);

static const char *usage_text[] = {
"options:\t-a\tprint all .debug_* sections",
"\t\t-b\tprint abbrev section",
"\t\t-c\tprint loc section",
"\t\t-c<str>\tcheck only specific compiler objects", 
"\t\t  \t  <str> is described by 'DW_AT_producer'. Examples:",
"\t\t  \t    -cg       check only GCC compiler objects",
"\t\t  \t    -cs       check only SNC compiler objects",
"\t\t  \t    -c'350.1' check only compiler objects with 350.1 in the CU name",
"\t\t-C\tactivate printing (with -i) of warnings about",
"\t\t\tcertain common extensions of DWARF.",
"\t\t-d\tdense: one line per entry (info section only)",
"\t\t-D\tdo not show offsets",  /* Do not show any offsets */
"\t\t-e\tellipsis: short names for tags, attrs etc.",
"\t\t-E\tprint object Header information",
"\t\t-f\tprint dwarf frame section",
"\t\t-F\tprint gnu .eh_frame section",
"\t\t-g\t(use incomplete loclist support)",
"\t\t-G\tshow global die offsets",
"\t\t-h\tprint IRIX exception tables (unsupported)",
"\t\t-H <num>\tlimit output to the first <num> major units",
"\t\t\t  example: to stop after <num> compilation units",
"\t\t-i\tprint info section",
"\t\t-k[abcdeEfFgilmMnrRsStx[e]y] check dwarf information",
"\t\t   a\tdo all checks",
"\t\t   b\tcheck abbreviations",     /* Check abbreviations */
"\t\t   c\texamine DWARF constants", /* Check for valid DWARF constants */
"\t\t   d\tshow check results",      /* Show check results */
"\t\t   e\texamine attributes of pubnames",
"\t\t   E\tignore DWARF extensions",  /*  Ignore DWARF extensions */
"\t\t   f\texamine frame information (use with -f or -F)",
"\t\t   F\texamine integrity of files-lines attributes", /* Files-Lines integrity */
"\t\t   g\tcheck debug info gaps", /* Check for debug info gaps */
"\t\t   i\tdisplay summary for all compilers", /* Summary all compilers */
"\t\t   l\tcheck location list (.debug_loc)",  /* Location list integrity */
"\t\t   m\tcheck ranges list (.debug_ranges)", /* Ranges list integrity */
"\t\t   M\tcheck ranges list (.debug_aranges)",/* Aranges list integrity */
"\t\t   n\texamine names in attributes",       /* Check for valid names */
"\t\t   r\texamine tag-attr relation",
"\t\t   R\tcheck forward references to DIEs (declarations)", /* Check DW_AT_specification references */
"\t\t   s\tperform checks in silent mode",
"\t\t   S\tcheck self references to DIEs",
"\t\t   t\texamine tag-tag relations",
"\t\t   x\tbasic frames check (.eh_frame, .debug_frame)",
"\t\t   xe\textensive frames check (.eh_frame, .debug_frame)", 
"\t\t   y\texamine type info",
"\t\t\tUnless -C option given certain common tag-attr and tag-tag",
"\t\t\textensions are assumed to be ok (not reported).",
"\t\t-l\tprint line section",
"\t\t-m\tprint macinfo section",
"\t\t-M\tprint the form name for each attribute",
"\t\t-n\tsuppress frame information function name lookup",
"\t\t  \t(when printing frame information from multi-gigabyte",
"\t\t  \tobject files this option may save significant time).",
"\t\t-N\tprint ranges section",
"\t\t-o[liaprfoR]\tprint relocation info",
"\t\t  \tl=line,i=info,a=abbrev,p=pubnames,r=aranges,f=frames,o=loc,R=Ranges",
"\t\t-p\tprint pubnames section",
"\t\t-P\tprint list of compile units per producer", /* List of CUs per compiler */
"\t\t-Q\tsuppress printing section data",  
"\t\t-r\tprint aranges section",
"\t\t-R\tPrint frame register names as r33 etc",
"\t\t  \t    and allow up to 1200 registers.",
"\t\t  \t    Print using a 'generic' register set.",
"\t\t-s\tprint string section",
"\t\t-S <option>=<text>\tsearch for <text> in attributes",
"\t\t  \twith <option>:",
"\t\t  \t-S any=<text>\tany <text>",
"\t\t  \t-S match=<text>\tmatching <text>",
#ifdef HAVE_REGEX
"\t\t  \t-S regex=<text>\tuse regular expression matching", 
#endif
"\t\t  \t (only one -S option allowed, any= and regex= ",
"\t\t  \t  only usable if the functions required are ",
"\t\t  \t  found at configure time)",
"\t\t-t[afv] static: ",
"\t\t   a\tprint both sections",
"\t\t   f\tprint static func section",
"\t\t   v\tprint static var section",
"\t\t-u<file> print sections only for specified file",
"\t\t-v\tverbose: show more information",
"\t\t-vv verbose: show even more information",
"\t\t-V print version information",
"\t\t-x name=<path>\tname dwarfdump.conf",
"\t\t-x abi=<abi>\tname abi in dwarfdump.conf",
"\t\t-w\tprint weakname section",
"\t\t-W\tprint parent and children tree (wide format) with the -S option",
"\t\t-Wp\tprint parent tree (wide format) with the -S option",
"\t\t-Wc\tprint children tree (wide format) with the -S option",
"\t\t-y\tprint type section",
"",

0};



std::string program_name;
int check_error = 0;
LinkOnceData *pLinkOnceData;
AddressRangesData *pAddressRangesData;
VisitedOffsetData *pVisitedOffsetData;

/* Options to enable debug tracing */
int nTrace[MAX_TRACE_LEVEL + 1];

/* Build section information */
void build_linkonce_info(Dwarf_Debug dbg);
static string do_uri_translation(const string &s, const std::string &context);
static void reset_overall_CU_error_data();

bool info_flag = false;

/*  This so both dwarf_loclist() 
    and dwarf_loclist_n() can be
    tested. Defaults to new
    dwarf_loclist_n() */
bool use_old_dwarf_loclist = false;  

bool line_flag = false;
static bool abbrev_flag = false;
static bool frame_flag = false;      /* .debug_frame section. */
static bool eh_frame_flag = false;   /* GNU .eh_frame section. */
static bool pubnames_flag = false;
static bool macinfo_flag = false;
static bool loc_flag = false;
static bool aranges_flag = false;
static bool ranges_flag = false; /* .debug_ranges section. */
static bool string_flag = false;
static bool reloc_flag = false;
static bool static_func_flag = false;
static bool static_var_flag = false;
static bool type_flag = false;
static bool weakname_flag = false;
static bool header_flag = false; /* Control printing of Elf header. */
bool producer_children_flag = false;   /* List of CUs per compiler */

// Bitmap for relocations. See globals.h for DW_SECTION_REL_DEBUG_RANGES etc.
static unsigned reloc_map = 0;

// Start verbose at zero. verbose can 
// be incremented with -v but not decremented.
int verbose = 0;
bool dense = false;
bool ellipsis = false;
bool show_global_offsets = false;
bool show_form_used = false;
bool display_offsets = true;  /* Emit offsets */

bool check_abbrev_code = false;
bool check_pubname_attr = false;
bool check_reloc_offset = false;
bool check_attr_tag = false;
bool check_tag_tree = false;
bool check_type_offset = false;
bool check_decl_file = false;
bool check_lines = false;
bool check_fdes = false;
bool check_ranges = false;
bool check_aranges = false;
bool check_harmless = false;
bool check_abbreviations = false;
bool check_dwarf_constants = false;
bool check_di_gaps = false;
bool check_forward_decl = false;
bool check_self_references = false;
bool generic_1200_regs = false;
bool suppress_check_extensions_tables = false;
// suppress_nested_name_search is a band-aid. 
// A workaround. A real fix for N**2 behavior is needed. 
bool suppress_nested_name_search = false;
static bool uri_options_translation = true;
static bool do_print_uri_in_input = true;

/*  break_after_n_units is mainly for testing.
    It enables easy limiting of output size/running time
    when one wants the output limited. 
    For example,
        -H 2
    limits the -i output to 2 compilation units and 
    the -f or -F output to 2 FDEs and 2 CIEs.  */
int break_after_n_units = INT_MAX;

bool check_names = false;     
bool check_verbose_mode = true; /* During '-k' mode, display errors */
bool check_frames = false;    
bool check_frames_extended = false;    /* Extensive frames check */
bool check_locations = false;          /* Location list check */

static bool check_all_compilers = true; 
static bool check_snc_compiler = false; /* Check SNC compiler */
static bool check_gcc_compiler = false; 
static bool print_summary_all = false; 


/*  Records information about compilers (producers) found in the
    debug information, including the check results for several
    categories (see -k option). */
struct Compiler {
    Compiler():verified_(false) { results_.resize((int)LAST_CATEGORY); };
    ~Compiler() {};
    std::string name_;
    bool verified_;
    std::vector<std::string> cu_list_;
    std::vector<Dwarf_Check_Result> results_;
};

/* Record compilers  whose CU names have been seen. 
   Full CU names recorded here, though only a portion
   of the name may have been checked to cause the
   compiler data  to be entered here.
*/

static std::vector<Compiler> compilers_detected;

/* compilers_targeted is a list of indications of compilers
   on which we wish error checking (and the counts
   of checks made and errors found).   We do substring
   comparisons, so the compilers_targeted name might be simply a
   compiler version number or a short substring of a
   CU producer name.
*/
static std::vector<Compiler> compilers_targeted;
static int current_compiler = 0;

static void PRINT_CHECK_RESULT(const std::string &str,
    Compiler *pCompiler, Dwarf_Check_Categories category);


/* The check and print flags here make it easy to 
   allow check-only or print-only.  We no longer support
   check-and-print in a single run.  */
bool do_check_dwarf = false;
bool do_print_dwarf = false;
bool check_show_results = false;  /* Display checks results. */
bool record_dwarf_error = false;  /* A test has failed, this
    is normally set false shortly after being set TRUE, it is
    a short-range hint we should print something we might not
    otherwise print (under the circumstances). */
struct Error_Message_Data error_message_data;

bool display_parent_tree = false;
bool display_children_tree = false;
int stop_indent_level = 0;

/* Print search results in wide format? */
bool search_wide_format = false;










bool search_is_on;
std::string search_any_text;
std::string search_match_text;
std::string search_regex_text;
#ifdef HAVE_REGEX
regex_t search_re;
#endif


/*  These configure items are for the 
    frame data.  */
static string config_file_path;
static string config_file_abi;
static const char *  config_file_defaults[] = {
    "dwarfdump.conf",
    "./dwarfdump.conf",
    /* Note: HOME location uses .dwarfdump.conf or dwarfdump.conf .  */
    "HOME/.dwarfdump.conf",
    "HOME/dwarfdump.conf",
#ifdef CONFPREFIX
/*  See Makefile.in  "libdir"  and CFLAGS  */
/*  We need 2 levels of macro to get the name turned into
    the string we want. */
#define STR2(s) # s
#define STR(s)  STR2(s)
    STR(CONFPREFIX)
        "/dwarfdump.conf",
#else
    "/usr/lib/dwarfdump.conf",
#endif
    0
};
static struct dwconf_s config_file_data;

string cu_name;
bool cu_name_flag = false;
Dwarf_Unsigned cu_offset = 0;

Dwarf_Error err;

static void suppress_check_dwarf()
{
    do_print_dwarf = true;
    if(do_check_dwarf) {
        cerr <<"Warning: check flag turned off, "
            "checking and printing are separate." <<
            endl;
    }
    do_check_dwarf = false;
}
static void suppress_print_dwarf()
{
    do_print_dwarf = false;
    do_check_dwarf = true;
}


static int process_one_file(Elf * elf, const string &file_name, int archive,
    struct dwconf_s *conf);
static int
open_a_file(const string &name)
{
    int f = 0;

#ifdef __CYGWIN__
    f = open(name.c_str(), O_RDONLY | O_BINARY);
#else
    f = open(name.c_str(), O_RDONLY);
#endif
    return f;

}

/* Iterate through dwarf and print all info.  */
int
main(int argc, char *argv[])
{
    int archive = 0;

    print_version_details(argv[0],false);

    // Ensure we have the zero entry of the vectors in
    // these three data structures.
    pAddressRangesData = new AddressRangesData;
    pLinkOnceData = new LinkOnceData;
    pVisitedOffsetData = new VisitedOffsetData;

    increment_compilers_detected(false);
    increment_compilers_targeted(false);
    (void) elf_version(EV_NONE);
    if (elf_version(EV_CURRENT) == EV_NONE) {
        cerr << "dwarfdump: libelf.a out of date." << endl;
        exit(1);
    }
    /*  Because LibDwarf now generates some new warnings,
        allow the user to hide them by using command line options */
    {
        Dwarf_Cmdline_Options cmd;
        cmd.check_verbose_mode = check_verbose_mode;
        dwarf_record_cmdline_options(cmd);
    }

    print_args(argc,argv);
    string file_name = process_args(argc, argv);
    int f = open_a_file(file_name);
    if (f == -1) {
        cerr << program_name << " ERROR:  can't open " <<
            file_name << endl;
        return (FAILED);
    }

    Elf_Cmd cmd = ELF_C_READ;
    Elf *arf = elf_begin(f, cmd, (Elf *) 0);
    if (elf_kind(arf) == ELF_K_AR) {
        archive = 1;
    }
    Elf *elf = 0;
    while ((elf = elf_begin(f, cmd, arf)) != 0) {
        Elf32_Ehdr *eh32;

#ifdef HAVE_ELF64_GETEHDR
        Elf64_Ehdr *eh64;
#endif /* HAVE_ELF64_GETEHDR */
        eh32 = elf32_getehdr(elf);
        if (!eh32) {
#ifdef HAVE_ELF64_GETEHDR
            /* not a 32-bit obj */
            eh64 = elf64_getehdr(elf);
            if (!eh64) {
                /* not a 64-bit obj either! */
                /* dwarfdump is quiet when not an object */
            } else {
                process_one_file(elf, file_name, archive,
                    &config_file_data);
            }
#endif /* HAVE_ELF64_GETEHDR */
        } else {
            process_one_file(elf, file_name, archive,
                &config_file_data);
        }
        cmd = elf_next(elf);
        elf_end(elf);
    }
    elf_end(arf);
    /* Trivial malloc space cleanup. */
    clean_up_syms_malloc_data();
    delete pAddressRangesData;
    delete pLinkOnceData;
    delete pVisitedOffsetData;
#ifdef HAVE_REGEX
    if(!search_regex_text.empty()) {
        regfree(&search_re);
    }
#endif


    if (check_error)
        return FAILED;
    else
        return OKAY;
}

void
print_any_harmless_errors(Dwarf_Debug dbg)
{
#define LOCAL_PTR_ARY_COUNT 50
    /*  We do not need to initialize the local array,
        libdwarf does it. */
    const char *buf[LOCAL_PTR_ARY_COUNT];
    unsigned totalcount = 0;
    unsigned i = 0;
    unsigned printcount = 0;
    int res = dwarf_get_harmless_error_list(dbg,LOCAL_PTR_ARY_COUNT,buf,
        &totalcount);
    if(res == DW_DLV_NO_ENTRY) {
        return;
    }
    if(totalcount > 0) {
        cout << endl;
        cout << "*** HARMLESS ERROR COUNT: " << IToDec(totalcount) <<
            " ***" << endl;
    }
    for(i = 0 ; buf[i]; ++i) {
        ++printcount;
        DWARF_CHECK_COUNT(harmless_result,1);
        DWARF_CHECK_ERROR(harmless_result,buf[i]);
    }
    if(totalcount > printcount) {
        DWARF_CHECK_COUNT(harmless_result,(totalcount - printcount));
        DWARF_ERROR_COUNT(harmless_result,(totalcount - printcount));
    }
}

static void
print_object_header(Elf *elf,Dwarf_Debug dbg)
{
#ifdef WIN32
    /*  Standard libelf has no function generating the names of the 
        encodings, but this libelf apparently does. */
    Elf_Ehdr_Literal eh_literals;
    Elf32_Ehdr *eh32;
#ifdef HAVE_ELF64_GETEHDR
    Elf64_Ehdr *eh64;
#endif /* HAVE_ELF64_GETEHDR */

    eh32 = elf32_getehdr(elf);
    if (eh32) {
        /* Get literal strings for header fields */
        elf32_gethdr_literals(eh32,&eh_literals);
        /* Print 32-bit obj header */
        cout << endl;   
        cout << "Object Header:" << endl; 
        cout << "e_ident:" << endl;
        cout <<"  File ID       = " << eh_literals.e_ident_file_id <<endl;
        cout <<"  File class    = " <<
            IToHex0N(eh32->e_ident[EI_CLASS],4)<<
            eh_literals.e_ident_file_class << endl;
        cout <<"  Data encoding = " <<
            IToHex0N(eh32->e_ident[EI_DATA],4)<<
            eh_literals.e_ident_data_encoding << endl;
        cout <<"  File version  = "<<
            IToHex0N(eh32->e_ident[EI_VERSION],4)<<
            eh_literals.e_ident_file_version << endl;
        cout <<"  OS ABI        = " <<
            IToHex0N(eh32->e_ident[EI_VERSION],4)<<
            " (" <<eh_literals.e_ident_os_abi_s << 
            ") (" <<eh_literals.e_ident_os_abi_l << 
            ")" <<endl;
        //printf("  ABI version   = %02x (%s)\n",
        //  eh32->e_ident[EI_ABIVERSION], eh_literals.e_ident_abi_version);
        cout <<"e_type   : " <<
            << IToHex(eh32->e_type)<< 
            " ("<< eh_literals.e_type << ")" << endl;
        cout <<"e_machine: " << 
            << IToHex(eh32->e_machine)<< 
            " (" << eh_literals.e_machine_s <<  
            ") (" << eh_literals.e_machine_l << ")" << endl;
        cout <<"e_version: " <<  IToHex(eh32->e_version) << endl;
        //printf("e_entry     = 0x%I64x\n", eh32->e_entry);
        cout <<"e_flags  : " <<  IToHex(eh32->e_version);
        cout <<"e_flags  : "<< IToHex(eh32->e_flags) << endl;
        cout <<"e_phnum  : "<<IToHex( eh32->e_phnum) << endl;
        cout <<"e_shnum  : " <<IToHex(eh32->e_shnum) << endl;
    }
    else {
#ifdef HAVE_ELF64_GETEHDR
        /* not a 32-bit obj */
        eh64 = elf64_getehdr(elf);
        if (eh64) {
            /* Get literal strings for header fields */
            elf64_gethdr_literals(eh64,&eh_literals);
            /* Print 64-bit obj header */
            cout << endl;   
            cout << "Object Header:" << endl; 
            cout << "e_ident:" << endl;
            cout <<"  File ID       = " << eh_literals.e_ident_file_id <<endl;
            cout <<"  File class    = " <<
                IToHex0N(eh64->e_ident[EI_CLASS],4)<<
                eh_literals.e_ident_file_class << endl;
            cout <<"  Data encoding = " <<
                IToHex0N(eh64->e_ident[EI_DATA],4)<<
                eh_literals.e_ident_data_encoding << endl;
            cout <<"  File version  = "<<
                IToHex0N(eh64->e_ident[EI_VERSION],4)<<
                eh_literals.e_ident_file_version << endl;
            cout <<"  OS ABI        = " <<
                IToHex0N(eh64->e_ident[EI_VERSION],4)<<
                " (" <<eh_literals.e_ident_os_abi_s << 
                ") (" <<eh_literals.e_ident_os_abi_l << 
                ")" <<endl;
            //printf("  ABI version   = %02x (%s)\n",
            //  eh64->e_ident[EI_ABIVERSION], eh_literals.e_ident_abi_version);
            cout <<"e_type   : " <<
                << IToHex(eh64->e_type)<< 
                " ("<< eh_literals.e_type << ")" << endl;
            cout <<"e_machine: " << 
                << IToHex(eh64->e_machine)<< 
                " (" << eh_literals.e_machine_s <<  
                ") (" << eh_literals.e_machine_l << ")" << endl;
            cout <<"e_version: " <<  IToHex(eh64->e_version) << endl;
            //printf("e_entry     = 0x%I64x\n", eh64->e_entry);
            cout <<"e_flags  : " <<  IToHex(eh64->e_version);
            cout <<"e_flags  : "<< IToHex(eh64->e_flags) << endl;
            cout <<"e_phnum  : "<<IToHex( eh64->e_phnum) << endl;
            cout <<"e_shnum  : " <<IToHex(eh64->e_shnum) << endl;
        }
#endif /* HAVE_ELF64_GETEHDR */
    }
#endif /* WIN32 */
}


/* Print checks and errors for a specific compiler */
static void
print_specific_checks_results(Compiler *pCompiler)
{
    cerr << endl;
    cerr << "DWARF CHECK RESULT" << endl;
    cerr << "<item>                    <checks>    <errors>" << endl;
    if (check_pubname_attr) {
        PRINT_CHECK_RESULT("pubname_attr", pCompiler, pubname_attr_result);
    }
    if (check_attr_tag) {
        PRINT_CHECK_RESULT("attr_tag", pCompiler, attr_tag_result);
    }
    if (check_tag_tree) {
        PRINT_CHECK_RESULT("tag_tree", pCompiler, tag_tree_result);
    }
    if (check_type_offset) {
        PRINT_CHECK_RESULT("type_offset", pCompiler, type_offset_result);
    }
    if (check_decl_file) {
        PRINT_CHECK_RESULT("decl_file", pCompiler, decl_file_result);
    }
    if (check_ranges) {
        PRINT_CHECK_RESULT("ranges", pCompiler, ranges_result);
    }
    if (check_lines) {
        PRINT_CHECK_RESULT("line_table", pCompiler, lines_result);
    }
    if (check_fdes) {
        PRINT_CHECK_RESULT("fde table", pCompiler, fde_duplication);
    }
    if (check_aranges) {
        PRINT_CHECK_RESULT("aranges", pCompiler, aranges_result);
    }

    if (check_names) {
        PRINT_CHECK_RESULT("names",pCompiler, names_result);
    }
    if (check_frames) {
        PRINT_CHECK_RESULT("frames",pCompiler, frames_result);
    }
    if (check_locations) {
        PRINT_CHECK_RESULT("locations",pCompiler, locations_result);
    }

    if(check_harmless) {
        PRINT_CHECK_RESULT("harmless_errors", pCompiler, harmless_result);
    }

    if (check_abbreviations) {
        PRINT_CHECK_RESULT("abbreviations", pCompiler, abbreviations_result);
    }
    
    if (check_dwarf_constants) {
        PRINT_CHECK_RESULT("dwarf_constants",
            pCompiler, dwarf_constants_result);
    }
   
    if (check_di_gaps) {
        PRINT_CHECK_RESULT("debug_info_gaps", pCompiler, di_gaps_result);
    }
  
    if (check_forward_decl) {
        PRINT_CHECK_RESULT("forward_declarations", 
            pCompiler, forward_decl_result);
    }
 
    if (check_self_references) {
        PRINT_CHECK_RESULT("self_references", 
            pCompiler, self_references_result);
    }

    PRINT_CHECK_RESULT("** Summarize **",pCompiler, total_check_result);
}

// StrictWeakOrdering, like LessThanComparable.
// But reversed... !!
static bool
sort_compare_compiler(const Compiler &cmp1,const  Compiler &cmp2)
{
    int cnt1 = cmp1.results_[total_check_result].errors_;
    int cnt2 = cmp2.results_[total_check_result].errors_;

    if (cnt1 > cnt2) {
        return true;
    }
    /* When error counts match, sort on name. */
    if(cnt1 == cnt2) {
        if (cmp1.name_ > cmp2.name_) {
            return true;  
        }
    }
    return false;
}
/* Print a summary of checks and errors */
static void
print_checks_results()
{

    cerr.flush();
    cout.flush();

    if(compilers_detected.size() > 1) {
        std::stable_sort(compilers_detected.begin()+ 1,
            compilers_detected.end(),sort_compare_compiler);
    }

    /* Print list of CUs for each compiler detected */
    if (producer_children_flag) {

        unsigned count = 0;
        unsigned total = 0;

        cerr <<  endl;
        cerr << "*** CU NAMES PER COMPILER ***"<< endl;
        for (unsigned index = 1; index < compilers_detected.size(); ++index) {
            const Compiler& c = compilers_detected[index];
            cerr << endl;
            cerr << IToDec0N(index,2) << ": " << c.name_;
            cerr << endl;
            count = 0;
            for (unsigned nc = 0; 
                nc < c.cu_list_.size(); 
                ++nc ) {

                ++count;
                cerr << endl;
                cerr << "    " << IToDec0N(count,2) <<": '" <<
                    c.cu_list_[nc]<< "'" ;
            }
            total += count;
            cerr << endl;
        }
        cerr << endl;
        cerr<< "Detected " << total << " CU names" << endl;
    }

    /* Print error report only if errors have been detected */
    /* Print error report if the -kd option */
    if ((do_check_dwarf && check_error) || check_show_results) {
        int compilers_not_detected = 0;
        int compilers_verified = 0;

        /* Find out how many compilers have been verified. */
        for (unsigned index = 1; index < compilers_detected.size(); ++index) {
            if (compilers_detected[index].verified_) {
                ++compilers_verified;
            }
        }
        /* Find out how many compilers have been not detected. */
        for (unsigned index = 1; index < compilers_targeted.size(); ++index) {
            if (!compilers_targeted[index].verified_) {
                ++compilers_not_detected;
            }
        }

        /* Print compilers detected list */
        cerr << endl;
        cerr << compilers_detected.size() -1 <<  " Compilers detected:"
            << endl;
        for (unsigned index = 1; index < compilers_detected.size(); ++index) {
            cerr << IToDec0N(index,2) << ": " <<
                compilers_detected[index].name_<< endl;
        }

        /*  Print compiler list specified by the user with the
            '-c<str>', that were not detected. */
        if (compilers_not_detected) {
            unsigned count = 0;
            cerr << endl;
            cerr << compilers_not_detected <<  " Compilers not detected:"
                << endl;
            for (unsigned index = 1; index < compilers_targeted.size(); ++index) {
                Compiler *pCompiler = &compilers_targeted[index];
                if (!pCompiler->verified_) {
                    ++count;
                    cerr <<  IToDec0N(count,2) << ": '" <<
                        pCompiler->name_ << "'" << endl;
                }
            }
        }

        unsigned count2 = 0;
        cerr << endl;
        cerr << compilers_verified <<  " Compilers verified:"
            << endl;
        for (unsigned index = 1; index < compilers_detected.size(); ++index) {
            if (compilers_detected[index].verified_) {
                ++count2;
                Compiler *pCompiler = &compilers_detected[index];
                cerr << IToDec0N(count2,2) << ": errors = "<<
                    IToDec(pCompiler->results_[total_check_result].errors_,5) 
                    << ", " <<
                    pCompiler->name_ <<
                    endl;
            }
        }

        /*  Print summary if we have verified compilers or
            if the -kd option used. */
        if (compilers_verified || check_show_results) {
            /* Print compilers detected summary*/
            if (print_summary_all) {
                int count = 0;
                cerr << endl;
                cerr << "*** ERRORS PER COMPILER ***" << endl;
                for (unsigned index = 1; index < compilers_detected.size(); ++index) {
                    Compiler *pCompiler = &compilers_detected[index];
                    if (pCompiler->verified_) {
                        ++count;
                        cerr << IToDec0N(count,2) << ": " <<
                            pCompiler->name_ << endl;
                        print_specific_checks_results(pCompiler);
                    }
                }
            }

            /* Print general summary (all compilers checked) */
            cerr << endl;
            cerr <<"*** TOTAL ERRORS FOR ALL COMPILERS ***" << endl;
            print_specific_checks_results(&compilers_detected[0]);
        }
    }
}


/*
  Given a file which we know is an elf file, process
  the dwarf data.

*/
static int
process_one_file(Elf * elf,const  string & file_name, int archive,
    struct dwconf_s *config_file_data)
{
    Dwarf_Debug dbg;
    int dres = 0;

    dres = dwarf_elf_init(elf, DW_DLC_READ, NULL, NULL, &dbg, &err);
    if (dres == DW_DLV_NO_ENTRY) {
        cout <<"No DWARF information present in " << file_name <<endl;
        return 0;
    }
    if (dres != DW_DLV_OK) {
        print_error(dbg, "dwarf_elf_init", dres, err);
    }

    if (archive) {
        Elf_Arhdr *mem_header = elf_getarhdr(elf);

        cout << endl;
        cout << "archive member \t" << 
            (mem_header ? mem_header->ar_name : "") << endl;
    }
    dwarf_set_frame_rule_initial_value(dbg,
        config_file_data->cf_initial_rule_value);
    dwarf_set_frame_rule_table_size(dbg,
        config_file_data->cf_table_entry_count);
    dwarf_set_frame_cfa_value(dbg,
        config_file_data->cf_cfa_reg);
    dwarf_set_frame_same_value(dbg,
        config_file_data->cf_same_val);
    dwarf_set_frame_undefined_value(dbg,
        config_file_data->cf_undefined_val);
    if(config_file_data->cf_address_size) {
        dwarf_set_default_address_size(dbg, config_file_data->cf_address_size);
    }
    dwarf_set_harmless_error_list_size(dbg,50);

    dres = dwarf_get_address_size(dbg,
        &error_message_data.elf_address_size,&err);
    if (dres != DW_DLV_OK) {
        print_error(dbg, "get_location_list", dres, err);
    }
    error_message_data.elf_max_address = 
        (error_message_data.elf_address_size == 8 ) ?
        0xffffffffffffffffULL : 0xffffffff;

   /* Get .text and .debug_ranges info if in check mode */
    if (do_check_dwarf) {
        Dwarf_Addr lower = 0;
        Dwarf_Addr upper = 0;
        Dwarf_Unsigned size = 0;
        int res = 0;
        res = dwarf_get_section_info_by_name(dbg,".text",&lower,&size,&err);
        if (DW_DLV_OK == res) {
            upper = lower + size;
        }

        /* Set limits for Ranges Information */
        pAddressRangesData->SetLimitsAddressRange(lower,upper);

        /* Build section information */
        build_linkonce_info(dbg);
    }
 
    if (header_flag) {
        print_object_header(elf,dbg);
    }
    reset_overall_CU_error_data();
    if (info_flag || line_flag || cu_name_flag || search_is_on ||
        producer_children_flag) {
        print_infos(dbg,true);
        reset_overall_CU_error_data();
        print_infos(dbg,false);
    }
    if (pubnames_flag) {
        reset_overall_CU_error_data();
        print_pubnames(dbg);
    }
    if (macinfo_flag) {
        reset_overall_CU_error_data();
        print_macinfo(dbg);
    }
    if (loc_flag) {
        reset_overall_CU_error_data();
        print_locs(dbg);
    }
    if (abbrev_flag) {
        reset_overall_CU_error_data();
        print_abbrevs(dbg);
    }
    if (string_flag) {
        reset_overall_CU_error_data();
        print_strings(dbg);
    }
    if (aranges_flag) {
        reset_overall_CU_error_data();
        print_aranges(dbg);
    }
    if (ranges_flag) {
        reset_overall_CU_error_data();
        print_ranges(dbg);
    }
    if (frame_flag || eh_frame_flag) {
        reset_overall_CU_error_data();
        print_frames(dbg, frame_flag, eh_frame_flag, config_file_data);
    }
    if (static_func_flag) {
        reset_overall_CU_error_data();
        print_static_funcs(dbg);
    }
    if (static_var_flag) {
        reset_overall_CU_error_data();
        print_static_vars(dbg);
    }
    /*  DWARF_PUBTYPES is the standard typenames dwarf section.
        SGI_TYPENAME is the same concept but is SGI specific ( it was
        defined 10 years before dwarf pubtypes). */

    if (type_flag) {
        reset_overall_CU_error_data();
        print_types(dbg, DWARF_PUBTYPES);
        reset_overall_CU_error_data();
        print_types(dbg, SGI_TYPENAME);
    }
    if (weakname_flag) {
        reset_overall_CU_error_data();
        print_weaknames(dbg);
    }
    if (reloc_flag) {
        reset_overall_CU_error_data();
        print_relocinfo(dbg,reloc_map);
    }
    // The right time to do this is unclear, but we
    // need to do it.
    print_any_harmless_errors(dbg);

    print_checks_results();

    dres = dwarf_finish(dbg, &err);
    if (dres != DW_DLV_OK) {
        print_error(dbg, "dwarf_finish", dres, err);
    }
    cout << endl;
    cerr.flush();
    return 0;

}

static void do_all()
{
    info_flag = line_flag = frame_flag =  true;
    pubnames_flag = macinfo_flag = true;
    aranges_flag = true;
    /*  Do not do 
        loc_flag = TRUE 
        abbrev_flag = TRUE;
        ranges_flag = true;
        because nothing in
        the DWARF spec guarantees the sections are free of random bytes
        in areas not referenced by .debug_info */

    string_flag = true;
    /*  Do not do 
        reloc_flag = TRUE;
        as print_relocs makes no sense for non-elf dwarfdump users.  */
    static_func_flag = static_var_flag = true;
    type_flag = weakname_flag = true;
    header_flag = true;
}

/* Remove matching leading/trailing quotes.
   Does not alter the passed in string.
   If quotes removed does a makename on a modified string. */
static string
remove_quotes_pair(char *text)
{
    static char single_quote = '\'';
    static char double_quote = '\"';
    string out;
    char quote = 0;
    char *p = text;
    int len = strlen(text);

    if (len < 2) {
        return p;
    }

    /* Compare first character with ' or " */
    if (p[0] == single_quote) {
        quote = single_quote;
    } else {
        if (p[0] == double_quote) {
            quote = double_quote;
        }
        else {
            return p;
        }
    }
    {
        if (p[len - 1] == quote) {
            out = string(p+1,p+len-1);
            return out;
        }
    }
    return p;
}


/* process arguments and return object filename */
static string
process_args(int argc, char *argv[])
{
    extern int optind;
    int c = 0;
    bool usage_error = false;
    int oarg = 0;

    program_name = argv[0];

    suppress_check_dwarf();
    /* j q unused */
    if (argv[1] != NULL && argv[1][0] != '-') {
        do_all();
    }

    while ((c =
        getopt(argc, argv,
        "#:abc::CdDeEfFgGhH:ik:lmMnNo::pPqQrRsS:t:u:UvVwW::x:yz")) != EOF) {

        switch (c) {
        case '#':
        {
            int nTraceLevel =  atoi(optarg);
            if (nTraceLevel > 0 && nTraceLevel <= MAX_TRACE_LEVEL) {
                nTrace[nTraceLevel] = 1;
            }
            break;
        }
        case 'M':
            show_form_used =  true;
            break;
        case 'x':               /* Select abi/path to use */
            {
                string path;
                string abi;

                /*  -x name=<path> meaning name dwarfdump.conf file -x
                    abi=<abi> meaning select abi from dwarfdump.conf
                    file. Must always select abi to use dwarfdump.conf */
                if (strncmp(optarg, "name=", 5) == 0) {
                    path = do_uri_translation(&optarg[5],"-x name=");
                    if (path.empty())
                        goto badopt;
                    config_file_path = path;
                } else if (strncmp(optarg, "abi=", 4) == 0) {
                    abi = do_uri_translation(&optarg[4],"-x abi=");
                    if (abi.empty())
                        goto badopt;
                    config_file_abi = abi;
                    break;
                } else {

                    badopt:
                    cerr << "-x name=<path-to-conf>" <<endl;
                    cerr << " and  " << endl;
                    cerr << "-x abi=<abi-in-conf> " << endl;
                    cerr << "are legal, not -x " << optarg<< endl;
                    usage_error = true;
                    break;
                }
            }
            break;
        case 'C':
            suppress_check_extensions_tables = true;
            break;
        case 'g':
            use_old_dwarf_loclist = true;
            info_flag = true;
            suppress_check_dwarf();
            break;
        case 'i':
            info_flag = true;
            suppress_check_dwarf();
            break;
        case 'n':
            suppress_nested_name_search = true;
            break;
        case 'l':
            line_flag = true;
            suppress_check_dwarf();
            break;
        case 'f':
            frame_flag = true;
            suppress_check_dwarf();
            break;
        case 'H': 
            {
                int break_val =  atoi(optarg);
                if(break_val > 0) {
                    break_after_n_units = break_val;
                }
            }
            break;
        case 'F':
            eh_frame_flag = true;
            suppress_check_dwarf();
            break;
        case 'b':
            abbrev_flag = true;
            suppress_check_dwarf();
            break;
        case 'p':
            pubnames_flag = true;
            suppress_check_dwarf();
            break;
        case 'P':
            /* List of CUs per compiler */
            producer_children_flag = true;
            break;
        case 'r':
            aranges_flag = true;
            suppress_check_dwarf();
            break;
        case 'N':
            ranges_flag = true;
            suppress_check_dwarf();
            break;
        case 'R':
            generic_1200_regs = true;
            break;
        case 'm':
            macinfo_flag = true;
            suppress_check_dwarf();
            break;
        case 'c':
            // Specify compiler name.
            if (optarg) {
                if ('s' == optarg[0]) {
                    /* -cs : Check SNC compiler */
                    check_snc_compiler = true;
                    check_all_compilers = false;
                }
                else {
                    if ('g' == optarg[0]) {
                        /* -cg : Check GCC compiler */
                        check_gcc_compiler = true;
                        check_all_compilers = false;
                    }
                    else {
                        check_all_compilers = false;
                        increment_compilers_targeted(true);
                        unsigned cc = compilers_targeted.size() -1;
                        compilers_targeted[cc].name_ = 
                            do_uri_translation(optarg,"-c<compiler name>");
                        //  Assume a compiler version to check,
                        //  most likely a substring of a compiler name.
                    }
                }
            } else {
                loc_flag = true;
                suppress_check_dwarf();
            }
            break;
        case 'Q':
            // Q suppresses section data printing.
            do_print_dwarf = false;
            break;
        case 'q':
            // suppress uri-did-translate notification.
            do_print_uri_in_input = false;
            break;
        case 's':
            string_flag = true;
            suppress_check_dwarf();
            break;
        case 'S':
            /* -S option: strings for 'any' and 'match' */
            {
                bool err = true;
                search_is_on = true;
                /* -S text */
                if (strncmp(optarg,"match=",6) == 0) {
                    string noquotes = remove_quotes_pair(&optarg[6]);
                    search_match_text = do_uri_translation(noquotes,"-S match=");
                    if (search_match_text.size() > 0) {
                        err = false;
                    }
                }
                else {
                    if (strncmp(optarg,"any=",4) == 0) {
                        string noquotes = remove_quotes_pair(&optarg[4]);
                        search_any_text=do_uri_translation(noquotes,"-S any=");
                        if (search_any_text.size() > 0) {
                            err = false;
                        }
                    }
#ifdef HAVE_REGEX
                    else {
                        if (strncmp(optarg,"regex=",6) == 0) {
                            string noquotes = remove_quotes_pair(&optarg[6]);
                            search_regex_text = do_uri_translation(noquotes,"-S regex=");
                            if (search_regex_text.size() > 0) {
                                if (regcomp(&search_re,
                                    search_regex_text.c_str(),
                                    REG_EXTENDED)) {
                                    cerr <<
                                        "regcomp: unable to compile " <<
                                        search_regex_text << endl;
                                }
                                else {
                                    err = false;
                                }
                            }
                        }
                    }
#endif /* HAVE_REGEX */
                }
                if (err) {
                    cerr << 
                        "-S any=<text> or -S match=<text> or -S regex=<text>"
                        << endl;
                    cerr <<  "is allowed, not -S " <<optarg << endl;
                    usage_error = true;
                }
            }
            break;
        case 'a':
            suppress_check_dwarf();
            do_all();
            break;
        case 'v':
            verbose++;
            break;
        case 'V':
            {
            cout << DWARFDUMP_VERSION << endl;
            exit(0);
            }
            break;
        case 'd':
            /*  This is sort of useless unless printing,
                but harmless, so we do not insist we
                are printing with suppress_check_dwarf(). */
            dense = true;
            break;
        case 'D':
            /* Do not emit offset in output */
            display_offsets = false;
            break;
        case 'e':
            suppress_check_dwarf();
            ellipsis = true;
            break;
        case 'E':
            // Object Header information (but maybe really print).
            header_flag = true;
            break;
        case 'o':
            reloc_flag = true;
            if (optarg) {
                switch (optarg[0]) {
                case 'i': 
                    reloc_map |= (1 <<DW_SECTION_REL_DEBUG_INFO); 
                    reloc_map |= (1 <<DW_SECTION_REL_DEBUG_TYPES); 
                    break;
                case 'l': reloc_map |= (1 <<DW_SECTION_REL_DEBUG_LINE); break;
                case 'p': reloc_map |= (1 <<DW_SECTION_REL_DEBUG_PUBNAMES); break;
                /*  Case a has no effect, no relocations can point out
                    of the abbrev section. */
                case 'a': reloc_map |= (1 <<DW_SECTION_REL_DEBUG_ABBREV); break;
                case 'r': reloc_map |= (1 <<DW_SECTION_REL_DEBUG_ARANGES); break;
                case 'f': reloc_map |= (1 <<DW_SECTION_REL_DEBUG_FRAME); break;
                case 'o': reloc_map |= (1 <<DW_SECTION_REL_DEBUG_LOC); break;
                case 'R': reloc_map |= (1 <<DW_SECTION_REL_DEBUG_RANGES); break;
                default: usage_error = true; break;
                }
            } else {
                /* Display all relocs */
                reloc_map = 0x00ff;
            }
            break;
        case 'k':
            suppress_print_dwarf();
            oarg = optarg[0];
            switch (oarg) {
            case 'a':
                check_pubname_attr = true;
                check_attr_tag = true;
                check_tag_tree = check_type_offset = true;
                check_names = true;
                pubnames_flag = info_flag = true;
                check_decl_file = true;
                check_frames = true;
                // check_frames_extended = false;
                check_locations = true;
                frame_flag = eh_frame_flag = true;
                check_ranges = true;
                check_lines = true;
                check_fdes = true;
                check_harmless = true;
                check_aranges = true;
                aranges_flag = true;  /* Aranges section */
                check_abbreviations = true;
                check_dwarf_constants = true;
                check_di_gaps = true; /* Check debug info gaps */
                check_forward_decl = true;  /* Check forward declarations */
                check_self_references = true;  /* Check self references */
                break;
            /* Abbreviations */
            case 'b':
                check_abbreviations = true;
                info_flag = true;
                break;
            /* DWARF constants */
            case 'c':
                check_dwarf_constants = true;
                info_flag = true;
                break;
            /* Display check results */
            case 'd':
                check_show_results = true;
                break;
            case 'e':
                check_pubname_attr = true;
                pubnames_flag = true;
                check_harmless = true;
                check_fdes = true;
                break;
            case 'f':
                check_harmless = true;
                check_fdes = true;
                break;
            /* files-lines */
            case 'F':
                check_decl_file = true;
                check_lines = true;
                info_flag = true;
                break;
            /* Check debug info gaps */
            case 'g':
                check_di_gaps = true;
                info_flag = true;
                break;
            /* Locations list */
            case 'l':
                check_locations = true;
                info_flag = true;
                loc_flag = true;
                break;
            /* Ranges */
            case 'm':
                check_ranges = true;
                info_flag = true;
                break;
            /* Aranges */
            case 'M':
                check_aranges = true;
                aranges_flag = true;
                break;
            /* invalid names */
            case 'n':
                check_names = true;
                info_flag = true;
                break;

            case 'r':
                check_attr_tag = true;
                info_flag = true;
                check_harmless = true;
                break;
            /* forward declarations in DW_AT_specification */
            case 'R':
                check_forward_decl = true;
                info_flag = true;
                break;
            /* Check verbose mode */
            case 's':
                check_verbose_mode = false;
                break;
            /*  self references in:
                DW_AT_specification, DW_AT_type, DW_AT_abstract_origin */
            case 'S':
                check_self_references = true;
                info_flag = true;
                break;

            case 't':
                check_tag_tree = true;
                check_harmless = true;
                info_flag = true;
                break;
            case 'y':
                check_type_offset = true;
                check_harmless = true;
                check_decl_file = true;
                info_flag = true;
                check_ranges = true;
                check_aranges = true;
                break;
            /* Summary for each compiler */
            case 'i':
                print_summary_all = true;
                break;
            /* Frames check */
            case 'x':
                check_frames = true;
                frame_flag = true;
                eh_frame_flag = true;
                if (optarg[1]) {
                    if ('e' == optarg[1]) {
                        /* -xe : Extended frames check */
                        check_frames = false;
                        check_frames_extended = true;
                    } else {
                        usage_error = true;
                    }
                }
                break;

            default:
                usage_error = true;
                break;
            }
            break;
        case 'u':               /* compile unit */
            cu_name_flag = true;
            cu_name = do_uri_translation(optarg,"-u<cu name>");
            break;
        case 'U':               /* Suppress URI translation. */
            uri_options_translation = false;
            break;
        case 't':
            oarg = optarg[0];
            switch (oarg) {
            case 'a':
                /* all */
                static_func_flag = static_var_flag = true;
                suppress_check_dwarf();
                break;
            case 'f':
                /* .debug_static_func */
                static_func_flag = true;
                suppress_check_dwarf();
                break;
            case 'v':
                /* .debug_static_var */
                static_var_flag = true;
                suppress_check_dwarf();
                break;
            default:
                usage_error = true;
                break;
            }
            break;
        case 'y':               /* .debug_types */
            suppress_check_dwarf();
            type_flag = true;
            break;
        case 'w':               /* .debug_weaknames */
            weakname_flag = true;
            suppress_check_dwarf();
            break;
        case 'z':
            cerr << "-z is no longer supported:ignored" << endl;
            break;
        case 'G':
            show_global_offsets = true;
            break;
        case 'W':
            /* Search results in wide format */
            search_wide_format = true;
            if (optarg) {
                if ('c' == optarg[0]) {
                    /* -Wc : Display children tree */
                    display_children_tree = true;
                } else {
                    if ('p' == optarg[0]) {
                        /* -Wp : Display parent tree */
                        display_parent_tree = true;
                    } else {
                        usage_error = true;
                    }
                }
            }
            else {
                /* -W : Display parent and children tree */
                display_children_tree = true;
                display_parent_tree = true;
            }
            break;
        default:
            usage_error = true;
            break;
        }
    }

    init_conf_file_data(&config_file_data);
    if ((!config_file_abi.empty()) && generic_1200_regs) {
        cout << "Specifying both -R and -x abi= is not allowed. Use one "
            "or the other.  -x abi= ignored." <<endl;
        config_file_abi = "";
    }
    if(generic_1200_regs) {
        init_generic_config_1200_regs(&config_file_data);
    }
    if ((!config_file_abi.empty()) && (frame_flag || eh_frame_flag)) {
        int res = find_conf_file_and_read_config(config_file_path,
            config_file_abi,
            config_file_defaults,
            &config_file_data);
        if (res > 0) {
            cout <<
                "Frame not configured due to error(s). Giving up."<<endl;
            eh_frame_flag = false;
            frame_flag = false; 
        }
    }
    if (usage_error || (optind != (argc - 1))) {
        print_usage_message(program_name,usage_text);
        exit(FAILED);
    }
    if(do_check_dwarf) {
        /* Reduce verbosity when checking (checking means checking-only). */
        verbose = 1;
    }
    return do_uri_translation(argv[optind],"file-to-process");
}

/* ARGSUSED */
void
print_error(Dwarf_Debug dbg, const string & msg, int dwarf_code,
    Dwarf_Error err)
{
    print_error_and_continue(dbg,msg,dwarf_code,err);
    exit(FAILED);
}
/* ARGSUSED */
void
print_error_and_continue(Dwarf_Debug dbg, const string & msg, int dwarf_code,
    Dwarf_Error err)
{
    cout.flush();
    cerr.flush();
    cerr << endl;
    if (dwarf_code == DW_DLV_ERROR) {
        string errmsg = dwarf_errmsg(err);
        Dwarf_Unsigned myerr = dwarf_errno(err);
        cerr << program_name <<
            " ERROR:  " << msg << ":  " << errmsg << " (" << myerr<< 
            ")" << endl;
    } else if (dwarf_code == DW_DLV_NO_ENTRY) {
        cerr << program_name <<
            " NO ENTRY:  " <<  msg << ": " << endl;
    } else if (dwarf_code == DW_DLV_OK) {
        cerr << program_name<< ":  " << msg << endl;
    } else {
        cerr << program_name<< " InternalError:  "<<  msg << 
            ":  code " << dwarf_code << endl;
    }
    cerr.flush();

    // Display compile unit name.
    PRINT_CU_INFO();
}

/*  Predicate function. Returns 'true' if the CU should
    be skipped as the DW_AT_name of the CU
    does not match the command-line-supplied
    cu name.  Else returns false.*/
bool
should_skip_this_cu(DieHolder& hcu_die, Dwarf_Error err)
{
    Dwarf_Half tag = 0;
    Dwarf_Attribute attrib;
    Dwarf_Half theform = 0;
    Dwarf_Die cu_die = hcu_die.die();
    Dwarf_Debug dbg = hcu_die.dbg();

    int tres = dwarf_tag(cu_die, &tag, &err);
    if (tres != DW_DLV_OK) {
        print_error(dbg, "dwarf_tag when checking if cu skippable ",
            tres, err);
    }
    int dares = dwarf_attr(cu_die, DW_AT_name, &attrib, &err);
    if (dares != DW_DLV_OK) {
        print_error(dbg, 
            "dwarf cu_die has no name, when checking if cu skippable",
            dares, err);
    }
    int fres = dwarf_whatform(attrib, &theform, &err);
    if (fres == DW_DLV_OK) {
        if (theform == DW_FORM_string
            || theform == DW_FORM_strp) {
            char * temps = 0;
            int sres = dwarf_formstring(attrib, &temps,
                &err);
            if (sres == DW_DLV_OK) {
                char *p = temps;
                if (cu_name[0] != '/') {
                    p = strrchr(temps, '/');
                    if (p == NULL) {
                        p = temps;
                    } else {
                        p++;
                    }
                }
                if (strcmp(cu_name.c_str(), p)) {
                    // skip this cu.
                    return true;
                }
            } else {
                print_error(dbg,
                "arange: string missing",
                sres, err);
            }
        }
    } else {
        print_error(dbg,
            "dwarf_whatform unexpected value",
            fres, err);
    }
    dwarf_dealloc(dbg, attrib, DW_DLA_ATTR);
    return false;
}


/* Returns the DW_AT_name of the CU */
string
old_get_cu_name(Dwarf_Debug dbg, Dwarf_Die cu_die, Dwarf_Error err)
{
    Dwarf_Half tag = 0;
    Dwarf_Attribute attrib = 0;
    Dwarf_Half theform = 0;
    string attr_name;

    int tres = dwarf_tag(cu_die, &tag, &err);
    if (tres != DW_DLV_OK) {
        print_error(dbg, "dwarf_tag in aranges",
            tres, err);
    }
    int dares = dwarf_attr(cu_die, DW_AT_name, &attrib,
        &err);
    if (dares != DW_DLV_OK) {
        print_error(dbg, "dwarf_attr arange"
            " derived die has no name",
            dares, err);
        }
    int fres = dwarf_whatform(attrib, &theform, &err);
    if (fres == DW_DLV_OK) {
        if (theform == DW_FORM_string
            || theform == DW_FORM_strp) {
            char * temps = 0;
            int sres = dwarf_formstring(attrib, &temps,
                &err);
            if (sres == DW_DLV_OK) {
                char *p = temps;
                if (cu_name[0] != '/') {
                    p = strrchr(temps, '/');
                    if (p == NULL) {
                        p = temps;
                    } else {
                        p++;
                    }
                }
                attr_name.append(p);
            } else {
                print_error(dbg,
                    "arange: string missing",
                    sres, err);
            }
        }
    } else {
        print_error(dbg,
            "dwarf_whatform unexpected value",
            fres, err);
    }
    dwarf_dealloc(dbg, attrib, DW_DLA_ATTR);

    return attr_name;
}

/* Returns the cu name of the CU */
int get_cu_name(DieHolder &hcu_die,
    Dwarf_Error err, string &short_name, string &long_name)
{
    Dwarf_Attribute name_attr = 0;

    int ares = dwarf_attr(hcu_die.die(),DW_AT_name, &name_attr, &err);
    if (ares == DW_DLV_ERROR) {
        print_error(hcu_die.dbg(), "hassattr on DW_AT_name", ares, err);
    } else {
        if (ares == DW_DLV_NO_ENTRY) {
            short_name = "<unknown name>";
            long_name = "<unknown name>";
        } else {
            /* DW_DLV_OK */

            SrcfilesHolder srcfiles;
            get_attr_value(hcu_die.dbg(), DW_TAG_compile_unit, 
                hcu_die.die(), name_attr, 
                srcfiles,
                long_name,
                false /*show_form_used*/,0 /* verbose */);
            /* Generate the short name (filename) */
            const char * filename = strrchr(long_name.c_str(),'/');
            if (!filename) {
                filename = strrchr(long_name.c_str(),'\\');
            }
            if (filename) {
                ++filename;
            } else {
                filename = long_name.c_str();
            }
            short_name = filename;
        }
    }
    dwarf_dealloc(hcu_die.dbg(), name_attr, DW_DLA_ATTR);
    return ares;
}

/* Returns the producer of the CU */
int get_producer_name(DieHolder &hcu_die,
    Dwarf_Error err, string &producer_name)
{
    Dwarf_Attribute producer_attr = 0;

    int ares = dwarf_attr(hcu_die.die(), DW_AT_producer, &producer_attr, &err);
    if (ares == DW_DLV_ERROR) {
        print_error(hcu_die.dbg(), "hassattr on DW_AT_producer", ares, err);
    } else {
        if (ares == DW_DLV_NO_ENTRY) {
            /*  We add extra quotes so it looks more like
                the names for real producers that get_attr_value
                produces. */
            producer_name = "\"<CU-missing-DW_AT_producer>\"";
        } else {
            /*  DW_DLV_OK */
            /*  The string return is valid until the next call to this
                function; so if the caller needs to keep the returned
                string, the string must be copied (makename()). */
            string esb_producer;
            SrcfilesHolder srcfiles;
            get_attr_value(hcu_die.dbg(), DW_TAG_compile_unit, 
                hcu_die.die(), producer_attr, 
                srcfiles,producer_name,
                false /*show_form_used*/,
                0 /* verbose */);
        }
    }

    dwarf_dealloc(hcu_die.dbg(), producer_attr, DW_DLA_ATTR);
    return ares;
}

/* GCC linkonce names */
const char *lo_text           = ".text."; /*".gnu.linkonce.t.";*/
const char *lo_debug_abbr     = ".gnu.linkonce.wa.";
const char *lo_debug_aranges  = ".gnu.linkonce.wr.";
const char *lo_debug_frame_1  = ".gnu.linkonce.wf.";
const char *lo_debug_frame_2  = ".gnu.linkonce.wF.";
const char *lo_debug_info     = ".gnu.linkonce.wi.";
const char *lo_debug_line     = ".gnu.linkonce.wl.";
const char *lo_debug_macinfo  = ".gnu.linkonce.wm.";
const char *lo_debug_loc      = ".gnu.linkonce.wo.";
const char *lo_debug_pubnames = ".gnu.linkonce.wp.";
const char *lo_debug_ranges   = ".gnu.linkonce.wR.";
const char *lo_debug_str      = ".gnu.linkonce.ws.";

/* SNC compiler/linker linkonce names */
const char *nlo_text           = ".text.";
const char *nlo_debug_abbr     = ".debug.wa.";
const char *nlo_debug_aranges  = ".debug.wr.";
const char *nlo_debug_frame_1  = ".debug.wf.";
const char *nlo_debug_frame_2  = ".debug.wF.";
const char *nlo_debug_info     = ".debug.wi.";
const char *nlo_debug_line     = ".debug.wl.";
const char *nlo_debug_macinfo  = ".debug.wm.";
const char *nlo_debug_loc      = ".debug.wo.";
const char *nlo_debug_pubnames = ".debug.wp.";
const char *nlo_debug_ranges   = ".debug.wR.";
const char *nlo_debug_str      = ".debug.ws.";

/* Build linkonce section information */
void 
build_linkonce_info(Dwarf_Debug dbg)
{
    int nCount = 0;
    int section_index = 0;
    int res = 0;

    static const char **linkonce_names[] = {
        &lo_text,            /* .text */
        &nlo_text,           /* .text */
        &lo_debug_abbr,      /* .debug_abbr */
        &nlo_debug_abbr,     /* .debug_abbr */
        &lo_debug_aranges,   /* .debug_aranges */
        &nlo_debug_aranges,  /* .debug_aranges */
        &lo_debug_frame_1,   /* .debug_frame */
        &nlo_debug_frame_1,  /* .debug_frame */
        &lo_debug_frame_2,   /* .debug_frame */
        &nlo_debug_frame_2,  /* .debug_frame */
        &lo_debug_info,      /* .debug_info */
        &nlo_debug_info,     /* .debug_info */
        &lo_debug_line,      /* .debug_line */
        &nlo_debug_line,     /* .debug_line */
        &lo_debug_macinfo,   /* .debug_macinfo */
        &nlo_debug_macinfo,  /* .debug_macinfo */
        &lo_debug_loc,       /* .debug_loc */
        &nlo_debug_loc,      /* .debug_loc */
        &lo_debug_pubnames,  /* .debug_pubnames */
        &nlo_debug_pubnames, /* .debug_pubnames */
        &lo_debug_ranges,    /* .debug_ranges */
        &nlo_debug_ranges,   /* .debug_ranges */
        &lo_debug_str,       /* .debug_str */
        &nlo_debug_str,      /* .debug_str */
        NULL
    };

    const char *section_name = NULL;
    Dwarf_Addr section_addr = 0;
    Dwarf_Unsigned section_size = 0;
    Dwarf_Error error = 0;
    int nIndex = 0;

    nCount = dwarf_get_section_count(dbg);

    /* Ignore section with index=0 */
    for (section_index = 1; section_index < nCount; ++section_index) {
        res = dwarf_get_section_info_by_index(dbg,section_index,
            &section_name,
            &section_addr,
            &section_size,
            &error);

        if (res == DW_DLV_OK) {
            for (nIndex = 0; linkonce_names[nIndex]; ++nIndex) {
                if (section_name == strstr(section_name,
                    *linkonce_names[nIndex])) {

                    /* Insert only linkonce sections */
                    pLinkOnceData->AddLinkOnceEntry(
                        LinkOnceEntry(
                            section_index,
                            section_addr,
                            section_addr + section_size,
                            section_name));
                    break;
                }
            }
        }
    }

    if (dump_linkonce_info) {
        pLinkOnceData->PrintLinkOnceData();
    }
}

/* Check for specific TAGs and initialize some 
    information used by '-k' options */
void 
tag_specific_checks_setup(Dwarf_Half val,int die_indent_level)
{
    switch (val) {
    case DW_TAG_compile_unit:
        /* To help getting the compile unit name */
        error_message_data.seen_CU = true;
        /*  If we are checking line information, build
            the table containing the pairs LowPC and HighPC */
        if (check_decl_file || check_ranges || check_locations) {
            pAddressRangesData->ResetRangesList();
        }
        /*  The following flag indicate that only low_pc and high_pc
            values found in DW_TAG_subprograms are going to be considered when
            building the address table used to check ranges, lines, etc */
        error_message_data.need_PU_valid_code = true;
        break;

    case DW_TAG_subprogram:
        /* Keep track of a PU */
        if (die_indent_level == 1) {
            /*  A DW_TAG_subprogram can be nested, when is used to
                declare a member function for a local class; process the DIE
                only if we are at level zero in the DIEs tree */
            error_message_data.seen_PU = true;
            error_message_data.seen_PU_base_address = false;
            error_message_data.seen_PU_high_address = false;
            error_message_data.PU_name = "";
            error_message_data.need_PU_valid_code = true;
        }
        break;
    }
}

/* Indicates if the current CU is a target */
static bool current_cu_is_checked_compiler = true;

/*  Are we checking for errors from the
    compiler of the current compilation unit?
*/
bool 
checking_this_compiler()
{
    /*  This flag has been update by 'update_compiler_target()'
        and indicates if the current CU is in a targeted compiler
        specified by the user. Default value is tRUE, which
        means test all compilers until a CU is detected. */
    return current_cu_is_checked_compiler || check_all_compilers;
}

static int
hasprefix(const char *sample, const char *prefix)
{
    unsigned prelen = strlen(prefix);
    if ( strncmp(sample,prefix,prelen) == 0) {
        return true;
    }
    return false;
}

static void 
increment_compilers_detected(bool beyond)
{
    if( compilers_detected.empty()) {
        // For the standard 'all' entry [0].
        Compiler c;
        compilers_detected.push_back(c);
    }
    if (beyond) {
        Compiler c;
        compilers_detected.push_back(c);
    }
}
static void 
increment_compilers_targeted(bool beyond)
{
    if( compilers_targeted.empty()) {
        // For the standard 'all' entry [0].
        Compiler c;
        compilers_targeted.push_back(c);
    }
    if (beyond) {
        Compiler c;
        compilers_targeted.push_back(c);
    }
}


/*  Record which compiler was used (or notice we saw
    it before) and set a couple variables as
    a side effect (which are used all over):
        current_cu_is_checked_compiler (used in checking_this_compiler() )
        current_compiler
    The compiler name is from DW_AT_producer.
*/
void 
update_compiler_target(const string &producer_name)
{
    unsigned index = 0;

    error_message_data.CU_producer = producer_name;
    current_cu_is_checked_compiler = false;

    /* This list of compilers is just a start:
        GCC id : "GNU"
        SNC id : "SN Systems" */

    /* Find a compiler version to check */
    if (!compilers_targeted.empty()) {
        for (index = 1; index < compilers_targeted.size(); ++index) {
            if (is_strstrnocase(error_message_data.CU_producer.c_str(),
                compilers_targeted[index].name_.c_str())) {
                compilers_targeted[index].verified_ = true;
                current_cu_is_checked_compiler = true;
                break;
            }
        }
    } else {
        /* Take into account that internally all strings are double quoted */
        bool snc_compiler = hasprefix(
            error_message_data.CU_producer.c_str(),
            "\"SN")? true : false;
        bool gcc_compiler = hasprefix(
            error_message_data.CU_producer.c_str(),
            "\"GNU")?true : false; 
        current_cu_is_checked_compiler = check_all_compilers ||
            (snc_compiler && check_snc_compiler) ||
            (gcc_compiler && check_gcc_compiler) ;
    }

    /* Check for already detected compiler */
    bool cFound = false;
    for (index = 1; index < compilers_detected.size(); ++index) {
        if (
#if WIN32
            !stricmp(compilers_detected[index].name_.c_str(),
                error_message_data.CU_producer.c_str())
#else
            compilers_detected[index].name_ == error_message_data.CU_producer
#endif
            ) {
            /* Set current compiler index */
            current_compiler = index;
            cFound = true;
            break;
        }
    }
    if (!cFound) {
        /* Record a new detected compiler name. */
        increment_compilers_detected(true);
        current_compiler = compilers_detected.size()-1;
        compilers_detected[current_compiler].name_ =
            error_message_data.CU_producer;
    }
}

/*  Add a CU name to the current compiler entry, specified by the
    'current_compiler'; the name is added to the 'compilers_detected'
    table and is printed if the '-P' option is specified in the
    command line. */
void
add_cu_name_compiler_target(const string & name)
{
    if (current_compiler < 1) {
        cerr << "Current  compiler set to " << current_compiler <<
            "cannot add Compilation unit name.  Giving up." << endl;
        exit(1);
    }
    compilers_detected[current_compiler].cu_list_.push_back(name);
}

/*  Making this a named string makes it simpler to change
    what the reset,or 'I do not know'  value is for
    CU name or producer name for PRINT_CU_INFO. */
static string default_cu_producer("<unknown>");
static void
reset_overall_CU_error_data()
{
   error_message_data.CU_name = default_cu_producer;
   error_message_data.CU_producer = default_cu_producer;
   error_message_data.DIE_offset = 0;
   error_message_data.DIE_overall_offset = 0;
   error_message_data.DIE_CU_offset = 0;
   error_message_data.DIE_CU_overall_offset = 0;
   error_message_data.CU_base_address = 0;
   error_message_data.CU_high_address = 0;
}

static bool
cu_data_is_set()
{
    if(error_message_data.CU_name != default_cu_producer || 
        error_message_data.CU_producer != default_cu_producer) {
        return true;
    }
    if(error_message_data.DIE_offset  || 
        error_message_data.DIE_overall_offset) {
        return true;
    }
    if(error_message_data.CU_base_address || 
        error_message_data.CU_high_address) {
        return true;
    }
    return false;
}

/* Print CU basic information */
void PRINT_CU_INFO()
{
    cerr.flush();
    cout.flush();
    if (error_message_data.current_section_id == DEBUG_LINE ||
        error_message_data.current_section_id == DEBUG_ARANGES) {
        /*  Only in the DEBUG_LINE/ARANGES case is DIE_CU_offset or 
            DIE_CU_overall_offset what we want to print here.
            In other cases DIE_CU_offset is not really a CU
            offset at all. */
        error_message_data.DIE_offset = error_message_data.DIE_CU_offset;
        error_message_data.DIE_overall_offset = 
            error_message_data.DIE_CU_overall_offset;
    }
    if(!cu_data_is_set()) {
        return;
    }
    cout <<  endl;
    cout <<"CU Name = " <<error_message_data.CU_name << endl;
    cout << "CU Producer = " <<error_message_data.CU_producer << endl;
    cout <<"DIE OFF = "<< IToHex0N(error_message_data.DIE_offset,10) <<
        " GOFF = "<< IToHex0N(error_message_data.DIE_overall_offset,10);
    cout <<", Low PC = " << 
        IToHex0N(error_message_data.CU_base_address,10) << 
        ", High PC = " << 
        IToHex0N(error_message_data.CU_high_address,10);
    cout << endl;
    cout.flush();
}

void DWARF_CHECK_COUNT(Dwarf_Check_Categories category, int inc)
{
    compilers_detected[0].results_[category].checks_ += inc;
    compilers_detected[0].results_[total_check_result].checks_ += inc;
    if(current_compiler > 0) {
        compilers_detected[current_compiler].results_[category].checks_ += inc;
        compilers_detected[current_compiler].results_[total_check_result].checks_
            += inc;
        compilers_detected[current_compiler].verified_ = true;
    }
}

void DWARF_ERROR_COUNT(Dwarf_Check_Categories category, int inc)
{
    compilers_detected[0].results_[category].errors_ += inc;
    compilers_detected[0].results_[total_check_result].errors_ += inc;
    if(current_compiler > 0) {
        compilers_detected[current_compiler].results_[category].errors_ += inc;
        compilers_detected[current_compiler].results_[total_check_result].errors_
            += inc;
    }
}

static void 
PRINT_CHECK_RESULT(const string &str,
    Compiler *pCompiler, Dwarf_Check_Categories category)
{
    Dwarf_Check_Result result = pCompiler->results_[category];
    cerr << std::setw(24) << std::left << str <<
        IToDec(result.checks_,10) <<  
        "  " <<
        IToDec(result.errors_,10) << endl; 
}

void DWARF_CHECK_ERROR_PRINT_CU()
{
    if (check_verbose_mode) {
        PRINT_CU_INFO();
    }
    check_error++;
    record_dwarf_error = true;
}

void DWARF_CHECK_ERROR(Dwarf_Check_Categories category,
    const std::string& str)
{
    if (checking_this_compiler()) {
        DWARF_ERROR_COUNT(category,1);
        if (check_verbose_mode) {
            cout << endl;
            cout << "*** DWARF CHECK: " << str << " ***" <<
                endl;
        }
        DWARF_CHECK_ERROR_PRINT_CU();
    }
}

void DWARF_CHECK_ERROR2(Dwarf_Check_Categories category,
    const std::string & str1, const std::string & str2)
{
    if (checking_this_compiler()) {
        DWARF_ERROR_COUNT(category,1);
        if (check_verbose_mode) {
            cout << endl;
            cout << "*** DWARF CHECK: " << str1 << ": " <<
                str2 << " ***" <<
                endl;
        }
        DWARF_CHECK_ERROR_PRINT_CU();
    }
}

void DWARF_CHECK_ERROR3(Dwarf_Check_Categories category,
    const std::string &str1, const std::string &str2,
    const std::string &strexpl)
{
    if (checking_this_compiler()) {
        DWARF_ERROR_COUNT(category,1);
        if (check_verbose_mode) {
            cout << endl;
            cout << "*** DWARF CHECK: " << str1 << " -> " <<
                str2 << ": " <<
                strexpl << " ***" <<
                endl;
        }
        DWARF_CHECK_ERROR_PRINT_CU();
    }
}

static string
do_uri_translation(const string &s,const string&context)
{
    string out;
    if (!uri_options_translation) {
        return s;
    }
    translate_from_uri(s.c_str(),out);
    if (do_print_uri_in_input) {
        if(s != out) {
            cout << "Uri Translation on option " << context << endl;
            cout << "    \'" << s << "\'"<< endl;
            cout << "    \'" << out << "\'"<< endl;
        }
    }
    return out;
}

