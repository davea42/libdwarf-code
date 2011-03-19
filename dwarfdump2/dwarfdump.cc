/* 
  Copyright (C) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007-2010 David Anderson. All Rights Reserved.
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

/* for 'open' */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>             /* For getopt. */
#include "dwconf.h"
#include "naming.h"
#define DWARFDUMP_VERSION " Thu Jan 13 16:21:01 PST 2011  "

using std::string;
using std::cout;
using std::cerr;
using std::endl;

#define OKAY 0
#define BYTES_PER_INSTRUCTION 4

extern char *optarg;
static string process_args(int argc, char *argv[]);
static void print_usage_message(void);

std::string program_name;
int check_error = 0;

bool info_flag = false;
bool use_old_dwarf_loclist = false;  /* This so both dwarf_loclist() 
                                           and dwarf_loclist_n() can be
                                           tested. Defaults to new
                                           dwarf_loclist_n() */

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

int verbose = 0;
bool dense = false;
bool ellipsis = false;
bool show_global_offsets = false;
bool show_form_used = false;

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
bool generic_1200_regs = false;
bool suppress_check_extensions_tables = false;
/* suppress_nested_name_search is a band-aid. 
   A workaround. A real fix for N**2 behavior is needed. 
*/
bool suppress_nested_name_search = false;

/* break_after_n_units is mainly for testing.
   It enables easy limiting of output size/running time
   when one wants the output limited. 
   For example,
     -H 2
   limits the -i output to 2 compilation units and 
   the -f or -F output to 2 FDEs and 2 CIEs.
*/
int break_after_n_units = INT_MAX;

static bool dwarf_check = false;

bool search_is_on;
std::string search_any_text;
std::string search_match_text;
std::string search_regex_text;
#ifdef HAVE_REGEX
regex_t search_re;
#endif


/* These configure items are for the 
   frame data.
*/
static string config_file_path;
static string config_file_abi;
static const char *  config_file_defaults[] = {
    "./dwarfdump.conf",
    /* Note: HOME location uses .dwarfdump.  */
    "HOME/.dwarfdump.conf",
#ifdef CONFPREFIX
/* See Makefile.in  "libdir"  and CFLAGS  */
/* We need 2 levels of macro to get the name turned into
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

Dwarf_Check_Result abbrev_code_result;
Dwarf_Check_Result pubname_attr_result;
Dwarf_Check_Result reloc_offset_result;
Dwarf_Check_Result attr_tag_result;
Dwarf_Check_Result tag_tree_result;
Dwarf_Check_Result type_offset_result;
Dwarf_Check_Result decl_file_result;
Dwarf_Check_Result ranges_result;
Dwarf_Check_Result aranges_result;
/* Harmless errors are errors detected inside libdwarf but
   not reported via DW_DLE_ERROR returns because the errors
   won't really affect client code.  The 'harmless' errors
   are reported and otherwise ignored.  It is difficult to report
   the error when the error is noticed by libdwarf, the error
   is reported at a later time.
   The other errors dwarfdump reports are also generally harmless 
   but are detected by dwarfdump so it's possble to report the
   error as soon as the error is discovered. */
Dwarf_Check_Result harmless_result;
Dwarf_Check_Result fde_duplication;
/* The lines_result errors could be transformed into 'harmless errors'
   quite easily (with a change to libdwarf and dwarfdump). */
Dwarf_Check_Result lines_result;

Dwarf_Error err;

#define PRINT_CHECK_RESULT(str,result)  { \
    cerr << LeftAlign(24,str) << " " <<     \
        IToDec(result.checks,8) << " " << \
        IToDec(result.errors,8) << endl;  \
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

/*
 * Iterate through dwarf and print all info.
 */
int
main(int argc, char *argv[])
{
    int archive = 0;

    (void) elf_version(EV_NONE);
    if (elf_version(EV_CURRENT) == EV_NONE) {
        cerr << "dwarfdump: libelf.a out of date." << endl;
        exit(1);
    }

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

    if (check_error)
        return FAILED;
    else
        return OKAY;
}

void
print_any_harmless_errors(Dwarf_Debug dbg)
{
#define LOCAL_PTR_ARY_COUNT 50
    /* We do not need to initialize the local array,
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
    for(i = 0 ; buf[i]; ++i) {
        ++printcount;
        harmless_result.checks++;
        DWARF_CHECK_ERROR(harmless_result,buf[i]);
    }
    if(totalcount > printcount) {
        harmless_result.errors += (totalcount - printcount);
	harmless_result.checks += (totalcount - printcount);
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
    int dres;

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
    dwarf_set_harmless_error_list_size(dbg,50);
    print_any_harmless_errors(dbg);

    if (info_flag || line_flag || cu_name_flag || search_is_on)
        print_infos(dbg);
    if (pubnames_flag)
        print_pubnames(dbg);
    if (macinfo_flag)
        print_macinfo(dbg);
    if (loc_flag)
        print_locs(dbg);
    if (abbrev_flag)
        print_abbrevs(dbg);
    if (string_flag)
        print_strings(dbg);
    if (aranges_flag)
        print_aranges(dbg);
    if (ranges_flag)
        print_ranges(dbg);
    if (frame_flag || eh_frame_flag) {
        print_frames(dbg, frame_flag, eh_frame_flag, config_file_data);
    }
    if (static_func_flag)
        print_static_funcs(dbg);
    if (static_var_flag)
        print_static_vars(dbg);
    /* DWARF_PUBTYPES is the standard typenames dwarf section.
       SGI_TYPENAME is the same concept but is SGI specific ( it was
       defined 10 years before dwarf pubtypes). */

    if (type_flag) {
        print_types(dbg, DWARF_PUBTYPES);
        print_types(dbg, SGI_TYPENAME);
    }
    if (weakname_flag)
        print_weaknames(dbg);
    if (reloc_flag)
        print_relocinfo(dbg);
    if (dwarf_check) {
        cerr << "DWARF CHECK RESULT" << endl;
        cerr << "<check-name>             <checks> <errors>" <<endl;
    }
    if (check_pubname_attr) {
        PRINT_CHECK_RESULT("pubname_attr", pubname_attr_result)
    }
    if (check_attr_tag) {
        PRINT_CHECK_RESULT("attr_tag", attr_tag_result)
    }
    if (check_tag_tree) {
        PRINT_CHECK_RESULT("tag_tree", tag_tree_result)
    }
    if (check_type_offset) {
        PRINT_CHECK_RESULT("type_offset", type_offset_result)
    }
    if (check_decl_file) {
        PRINT_CHECK_RESULT("decl_file", decl_file_result)
    }
    if (check_ranges) {
        PRINT_CHECK_RESULT("ranges", ranges_result)
    }
    if (check_lines) {
        PRINT_CHECK_RESULT("line table", lines_result)
    }
    if (check_fdes) {
        PRINT_CHECK_RESULT("fde table", fde_duplication)
    }
    if (check_aranges) {
        PRINT_CHECK_RESULT("aranges", aranges_result)
    }
    if (check_harmless) {
        print_any_harmless_errors(dbg);
        PRINT_CHECK_RESULT("harmless_errors", harmless_result)
    }
    dres = dwarf_finish(dbg, &err);
    if (dres != DW_DLV_OK) {
        print_error(dbg, "dwarf_finish", dres, err);
    }
    return 0;

}

static void do_all()
{
        info_flag = line_flag = frame_flag = abbrev_flag = true;
        pubnames_flag = aranges_flag = macinfo_flag = true;
        // Do not do loc_flag = TRUE because nothing in
        // the DWARF spec guarantees .debug_loc is free of random bytes
        // in areas not referenced by .debug_info 
        string_flag = true;
        reloc_flag = true;
        static_func_flag = static_var_flag = true;
        type_flag = weakname_flag = true;
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

    /* j q unused */
    if (argv[1] != NULL && argv[1][0] != '-') {
                do_all();
    }

    while ((c =
            getopt(argc, argv,
                   "abcCdefFgGhH:ik:lmMnNoprRsS:t:u:vVwx:yz")) != EOF) {
        switch (c) {
        case 'M':
            show_form_used =  true;
            break;
        case 'x':               /* Select abi/path to use */
            {
                string path;
                string abi;

                /* -x name=<path> meaning name dwarfdump.conf file -x
                   abi=<abi> meaning select abi from dwarfdump.conf
                   file. Must always select abi to use dwarfdump.conf */
                if (strncmp(optarg, "name=", 5) == 0) {
                    path = &optarg[5];
                    if (path.empty())
                        goto badopt;
                    config_file_path = path;
                } else if (strncmp(optarg, "abi=", 4) == 0) {
                    abi = &optarg[4];
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
            /* FALL THROUGH. */
        case 'i':
            info_flag = true;
            break;
        case 'n':
            suppress_nested_name_search = true;
            break;
        case 'l':
            line_flag = true;
            break;
        case 'f':
            frame_flag = true;
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
            break;
        case 'b':
            abbrev_flag = true;
            break;
        case 'p':
            pubnames_flag = true;
            break;
        case 'r':
            aranges_flag = true;
            break;
        case 'N':
            ranges_flag = true;
            break;
        case 'R':
            generic_1200_regs = true;
            break;
        case 'm':
            macinfo_flag = true;
            break;
        case 'c':
            loc_flag = true;
            break;
        case 's':
            string_flag = true;
            break;
        case 'S':
            /* -S option: strings for 'any' and 'match' */
            {
                bool err = true;
                search_is_on = true;
                /* -S text */
                if (strncmp(optarg,"match=",6) == 0) {
                    search_match_text = (&optarg[6]);
                    if (search_match_text.size() > 0) {
                        err = false;
                    }
                }
                else {
                    if (strncmp(optarg,"any=",4) == 0) {
                        search_any_text = (&optarg[4]);
                        if (search_any_text.size() > 0) {
                            err = false;
                        }
                    }
#ifdef HAVE_REGEX
                    else {
                        if (strncmp(optarg,"regex=",6) == 0) {
                            search_regex_text = (&optarg[6]);
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
            dense = true;
            break;
        case 'e':
            ellipsis = true;
            break;
        case 'o':
            reloc_flag = true;
            break;
        case 'k':
            dwarf_check = true;
            oarg = optarg[0];
            switch (oarg) {
            case 'a':
                check_pubname_attr = true;
                check_attr_tag = true;
                check_tag_tree = check_type_offset = true;
                pubnames_flag = info_flag = true;
                check_decl_file = true;
                check_ranges = true;
                check_aranges = true;
                check_lines = true;
                check_fdes = true;
                check_harmless = true;
                break;
            case 'e':
                check_pubname_attr = true;
                pubnames_flag = true;
                check_harmless = true;
                break;
            case 'r':
                check_attr_tag = true;
                check_harmless = true;
                info_flag = true;
                break;
            case 't':
                check_tag_tree = true;
                check_harmless = true;
                info_flag = true;
                break;
            case 'f':
                check_fdes = true;
                check_harmless = true;
                break;
            case 'y':
                check_type_offset = true;
                check_decl_file = true;
                check_harmless = true;
                info_flag = true;
                check_ranges = true;
                check_aranges = true;
                break;
            default:
                usage_error = true;
                break;
            }
            break;
        case 'u':               /* compile unit */
            cu_name_flag = true;
            cu_name = optarg;
            break;
        case 't':
            oarg = optarg[0];
            switch (oarg) {
            case 'a':
                /* all */
                static_func_flag = static_var_flag = true;
                break;
            case 'f':
                /* .debug_static_func */
                static_func_flag = true;
                break;
            case 'v':
                /* .debug_static_var */
                static_var_flag = true;
                break;
            default:
                usage_error = true;
                break;
            }
            break;
        case 'y':               /* .debug_types */
            type_flag = true;
            break;
        case 'w':               /* .debug_weaknames */
            weakname_flag = true;
            break;
        case 'z':
            cerr << "-z is no longer supported:ignored" << endl;
            break;
        case 'G':
            show_global_offsets = true;
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
        print_usage_message();
        exit(FAILED);
    }
    return argv[optind];
}

static const char *usage_text[] = {
"options:\t-a\tprint all .debug_* sections",
"\t\t-b\tprint abbrev section",
"\t\t-c\tprint loc section",
"\t\t-C\tactivate printing (with -i) of warnings about",
"\t\t\tcertain common extensions of DWARF.",
"\t\t-d\tdense: one line per entry (info section only)",
"\t\t-e\tellipsis: short names for tags, attrs etc.",
"\t\t-f\tprint dwarf frame section",
"\t\t-F\tprint gnu .eh_frame section",
"\t\t-g\t(use incomplete loclist support)",
"\t\t-G\tshow global die offsets",
"\t\t-h\tprint IRIX exception tables (unsupported)",
"\t\t-H <num>\tlimit output to the first <num> major units",
"\t\t\t  example: to stop after <num> compilation units",
"\t\t-i\tprint info section",
"\t\t-k[aefrty] check dwarf information",
"\t\t   a\tdo all checks",
"\t\t   e\texamine attributes of pubnames",
"\t\t   f\texamine frame information (use with -f or -F)",
"\t\t   r\texamine tag-attr relation",
"\t\t   t\texamine tag-tag relations",
"\t\t   y\texamine type info",
"\t\t\tUnless -C option given certain common tag-attr and tag-tag",
"\t\t\textensions are assumed to be ok (not reported).",
"\t\t-l\tprint line section",
"\t\t-m\tprint macinfo section",
"\t\t-M\tprint the form name for each attribute",
"\t\t-o\tprint relocation info",
"\t\t-p\tprint pubnames section",
"\t\t-N\tprint ranges section",
"\t\t-n\tsuppress frame information function name lookup",
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
"\t\t-y\tprint type section",
0};

static void
print_usage_message(void)
{
    cerr  << "Usage:  " << program_name << 
         " <options> <object file>" << endl;
    for (unsigned i = 0; usage_text[i]; ++i) {
        cerr << usage_text[i] << endl;
    }

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
}

/* Predicate function. Returns 'true' if the CU should
 * be skipped as the DW_AT_name of the CU
 * does not match the command-line-supplied
 * cu name.  Else returns false.*/
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
