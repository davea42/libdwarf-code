/* 
  Copyright (C) 2000,2002,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.
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
/* for 'open' */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>  /* For getopt */
#include "makename.h"
#include "dwconf.h"

#define DWARFDUMP_VERSION " Thu Jan 13 16:21:01 PST 2011  "

extern char *optarg;

#define OKAY 0
#define BYTES_PER_INSTRUCTION 4

static string process_args(int argc, char *argv[]);
static void print_usage_message(void);

char * program_name;
int check_error = 0;

/* defined in print_sections.c, die for the current compile unit, 
   used in get_fde_proc_name() */
extern Dwarf_Die current_cu_die_for_print_frames;


boolean info_flag = FALSE;
boolean use_old_dwarf_loclist = FALSE;  /* This so both dwarf_loclist() 
                                           and dwarf_loclist_n() can be
                                           tested. Defaults to new
                                           dwarf_loclist_n() */

boolean line_flag = FALSE;
static boolean abbrev_flag = FALSE;
static boolean frame_flag = FALSE;      /* .debug_frame section. */
static boolean eh_frame_flag = FALSE;   /* GNU .eh_frame section. */
static boolean pubnames_flag = FALSE;
static boolean macinfo_flag = FALSE;
static boolean loc_flag = FALSE;
static boolean aranges_flag = FALSE;
static boolean ranges_flag = FALSE; /* .debug_ranges section. */
static boolean string_flag = FALSE;
static boolean reloc_flag = FALSE;
static boolean static_func_flag = FALSE;
static boolean static_var_flag = FALSE;
static boolean type_flag = FALSE;
static boolean weakname_flag = FALSE;

int verbose = 0;
boolean dense = FALSE;
boolean ellipsis = FALSE;
boolean show_global_offsets = FALSE;
boolean show_form_used = FALSE;

boolean check_abbrev_code = FALSE;
boolean check_pubname_attr = FALSE;
boolean check_reloc_offset = FALSE;
boolean check_attr_tag = FALSE;
boolean check_tag_tree = FALSE;
boolean check_type_offset = FALSE;
boolean check_decl_file = FALSE;
boolean check_lines = FALSE;
boolean check_fdes = FALSE;
boolean check_ranges = FALSE;
boolean check_aranges = FALSE;
boolean check_harmless = FALSE;
boolean generic_1200_regs = FALSE;
boolean suppress_check_extensions_tables = FALSE;
/* suppress_nested_name_search is a band-aid. 
   A workaround. A real fix for N**2 behavior is needed. 
*/
boolean suppress_nested_name_search = FALSE;

/* break_after_n_units is mainly for testing.
   It enables easy limiting of output size/running time
   when one wants the output limited. 
   For example,
     -H 2
   limits the -i output to 2 compilation units and 
   the -f or -F output to 2 FDEs and 2 CIEs.
*/
int break_after_n_units = INT_MAX;

static boolean dwarf_check = FALSE;

/* -S option: strings for 'any' and 'match' */
boolean search_is_on = FALSE;
char *search_any_text = 0;
char *search_match_text = 0;
char *search_regex_text = 0;
#ifdef HAVE_REGEX
/* -S option: the compiled_regex */
regex_t search_re;
#endif


/* These configure items are for the 
   frame data.
*/
static char *config_file_path = 0;
static char *config_file_abi = 0;
static char *config_file_defaults[] = {
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

char cu_name[BUFSIZ];
boolean cu_name_flag = FALSE;
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

#define PRINT_CHECK_RESULT(str,result)  {\
    fprintf(stderr, "%-24s %8d %8d\n", str, result.checks, result.errors); \
}

static int process_one_file(Elf * elf, string file_name, int archive,
                            struct dwconf_s *conf);
static int
open_a_file(string name)
{
    int f = 0;

#ifdef __CYGWIN__
    f = open(name, O_RDONLY | O_BINARY);
#else
    f = open(name, O_RDONLY);
#endif
    return f;

}

/*
 * Iterate through dwarf and print all info.
 */
int
main(int argc, char *argv[])
{
    string file_name;
    int f;
    Elf_Cmd cmd;
    Elf *arf, *elf;
    int archive = 0;

    (void) elf_version(EV_NONE);
    if (elf_version(EV_CURRENT) == EV_NONE) {
        (void) fprintf(stderr, "dwarfdump: libelf.a out of date.\n");
        exit(1);
    }

    file_name = process_args(argc, argv);
    f = open_a_file(file_name);
    if (f == -1) {
        fprintf(stderr, "%s ERROR:  can't open %s\n", program_name,
                file_name);
        return (FAILED);
    }

    cmd = ELF_C_READ;
    arf = elf_begin(f, cmd, (Elf *) 0);
    if (elf_kind(arf) == ELF_K_AR) {
        archive = 1;
    }
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
    clean_up_die_esb();
    clean_up_syms_malloc_data();
#ifdef HAVE_REGEX
    if(search_regex_text) {
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
        harmless_result.checks += (totalcount - printcount);
        harmless_result.errors += (totalcount - printcount);
    }
}

/*
  Given a file which we know is an elf file, process
  the dwarf data.

*/
static int
process_one_file(Elf * elf, string file_name, int archive,
                 struct dwconf_s *config_file_data)
{
    Dwarf_Debug dbg;
    int dres;

    dres = dwarf_elf_init(elf, DW_DLC_READ, NULL, NULL, &dbg, &err);
    if (dres == DW_DLV_NO_ENTRY) {
        printf("No DWARF information present in %s\n", file_name);
        return 0;
    }
    if (dres != DW_DLV_OK) {
        print_error(dbg, "dwarf_elf_init", dres, err);
    }

    if (archive) {
        Elf_Arhdr *mem_header = elf_getarhdr(elf);

        printf("\narchive member \t%s\n",
               mem_header ? mem_header->ar_name : "");
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
        current_cu_die_for_print_frames = 0;
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
        fprintf(stderr, "DWARF CHECK RESULT\n");
        fprintf(stderr, "<check-name>             <checks> <errors>\n");
    }
    if (check_pubname_attr)
        PRINT_CHECK_RESULT("pubname_attr", pubname_attr_result)
    if (check_attr_tag)
        PRINT_CHECK_RESULT("attr_tag", attr_tag_result)
    if (check_tag_tree)
        PRINT_CHECK_RESULT("tag_tree", tag_tree_result)
    if (check_type_offset)
        PRINT_CHECK_RESULT("type_offset", type_offset_result)
    if (check_decl_file)
        PRINT_CHECK_RESULT("decl_file", decl_file_result)
    if (check_ranges)
        PRINT_CHECK_RESULT("ranges", ranges_result)
    if (check_lines )
        PRINT_CHECK_RESULT("line table", lines_result)
    if (check_fdes)
        PRINT_CHECK_RESULT("fde table", fde_duplication);
    if (check_aranges)
        PRINT_CHECK_RESULT("aranges", aranges_result)
    if( check_harmless) {
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
        info_flag = line_flag = frame_flag = abbrev_flag = TRUE;
        pubnames_flag = aranges_flag = macinfo_flag = TRUE;
        /* Do not do loc_flag = TRUE because nothing in
        the DWARF spec guarantees .debug_loc is free of random bytes
        in areas not referenced by .debug_info */
        string_flag = TRUE;
        reloc_flag = TRUE;
        static_func_flag = static_var_flag = TRUE;
        type_flag = weakname_flag = TRUE;
}

/* process arguments and return object filename */
static string
process_args(int argc, char *argv[])
{
    extern int optind;
    int c = 0;
    boolean usage_error = FALSE;
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
            show_form_used =  TRUE;
            break;
        case 'x':               /* Select abi/path to use */
            {
                char *path = 0;
                char *abi = 0;

                /* -x name=<path> meaning name dwarfdump.conf file -x
                   abi=<abi> meaning select abi from dwarfdump.conf
                   file. Must always select abi to use dwarfdump.conf */
                if (strncmp(optarg, "name=", 5) == 0) {
                    path = makename(&optarg[5]);
                    if (strlen(path) < 1)
                        goto badopt;
                    config_file_path = path;
                } else if (strncmp(optarg, "abi=", 4) == 0) {
                    abi = makename(&optarg[4]);
                    if (strlen(abi) < 1)
                        goto badopt;
                    config_file_abi = abi;
                    break;
                } else {
                  badopt:
                    fprintf(stderr, "-x name=<path-to-conf> \n");
                    fprintf(stderr, " and  \n");
                    fprintf(stderr, "-x abi=<abi-in-conf> \n");
                    fprintf(stderr, "are legal, not -x %s\n", optarg);
                    usage_error = TRUE;
                    break;
                }
            }
            break;
        case 'C':
            suppress_check_extensions_tables = TRUE;
            break;
        case 'g':
            use_old_dwarf_loclist = TRUE;
            info_flag = TRUE;
            break;
        case 'i':
            info_flag = TRUE;
            break;
        case 'n':
            suppress_nested_name_search = TRUE;
            break;
        case 'l':
            line_flag = TRUE;
            break;
        case 'f':
            frame_flag = TRUE;
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
            eh_frame_flag = TRUE;
            break;
        case 'b':
            abbrev_flag = TRUE;
            break;
        case 'p':
            pubnames_flag = TRUE;
            break;
        case 'r':
            aranges_flag = TRUE;
            break;
        case 'N':
            ranges_flag = TRUE;
            break;
        case 'R':
            generic_1200_regs = TRUE;
            break;
        case 'm':
            macinfo_flag = TRUE;
            break;
        case 'c':
            loc_flag = TRUE;
            break;
        case 's':
            string_flag = TRUE;
            break;
        case 'S':
            /* -S option: strings for 'any' and 'match' */
            {
                boolean err = TRUE;
                search_is_on = TRUE;
                /* -S text */
                if (strncmp(optarg,"match=",6) == 0) {
                    search_match_text = makename(&optarg[6]);
                    if (strlen(search_match_text) > 0) {
                        err = FALSE;
                    }
                }
                else {
                    if (strncmp(optarg,"any=",4) == 0) {
                        search_any_text = makename(&optarg[4]);
                        if (strlen(search_any_text) > 0) {
                            err = FALSE;
                        }
                    }
#ifdef HAVE_REGEX
                    else {
                        if (strncmp(optarg,"regex=",6) == 0) {
                            search_regex_text = makename(&optarg[6]);
                            if (strlen(search_regex_text) > 0) {
                                if (regcomp(&search_re,search_regex_text,
                                    REG_EXTENDED)) {
                                    fprintf(stderr,
                                        "regcomp: unable to compile %s\n",
                                        search_regex_text);
                                }
                                else {
                                    err = FALSE;
                                }
                            }
                        }
                    }
#endif /* HAVE_REGEX */
                }
                if (err) {
                    fprintf(stderr,"-S any=<text> or -S match=<text> or -S regex=<text>\n");
                    fprintf(stderr, "is allowed, not -S %s\n",optarg);
                    usage_error = TRUE;
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
                printf("%s\n",DWARFDUMP_VERSION);
                exit(0);
            }
            break;
        case 'd':
            dense = TRUE;
            break;
        case 'e':
            ellipsis = TRUE;
            break;
        case 'o':
            reloc_flag = TRUE;
            break;
        case 'k':
            dwarf_check = TRUE;
            oarg = optarg[0];
            switch (oarg) {
            case 'a':
                check_pubname_attr = TRUE;
                check_attr_tag = TRUE;
                check_tag_tree = check_type_offset = TRUE;
                pubnames_flag = info_flag = TRUE;
                check_decl_file = TRUE;
                check_ranges = TRUE;
                check_aranges = TRUE;
                check_lines = TRUE;
                check_fdes = TRUE;
                check_harmless = TRUE;
                break;
            case 'e':
                check_pubname_attr = TRUE;
                pubnames_flag = TRUE;
                check_harmless = TRUE;
                check_fdes = TRUE;
                break;
            case 'f':
                check_harmless = TRUE;
                check_fdes = TRUE;
                break;
            case 'r':
                check_attr_tag = TRUE;
                info_flag = TRUE;
                check_harmless = TRUE;
                break;
            case 't':
                check_tag_tree = TRUE;
                check_harmless = TRUE;
                info_flag = TRUE;
                break;
            case 'y':
                check_type_offset = TRUE;
                check_harmless = TRUE;
                check_decl_file = TRUE;
                info_flag = TRUE;
                check_ranges = TRUE;
                check_aranges = TRUE;
                break;
            default:
                usage_error = TRUE;
                break;
            }
            break;
        case 'u':               /* compile unit */
            cu_name_flag = TRUE;
            strcpy(cu_name, optarg);
            break;
        case 't':
            oarg = optarg[0];
            switch (oarg) {
            case 'a':
                /* all */
                static_func_flag = static_var_flag = TRUE;
                break;
            case 'f':
                /* .debug_static_func */
                static_func_flag = TRUE;
                break;
            case 'v':
                /* .debug_static_var */
                static_var_flag = TRUE;
                break;
            default:
                usage_error = TRUE;
                break;
            }
            break;
        case 'y':               /* .debug_types */
            type_flag = TRUE;
            break;
        case 'w':               /* .debug_weaknames */
            weakname_flag = TRUE;
            break;
        case 'z':
            fprintf(stderr, "-z is no longer supported:ignored\n");
            break;
        case 'G':
            show_global_offsets = TRUE;
            break;
        default:
            usage_error = TRUE;
            break;
        }
    }

    init_conf_file_data(&config_file_data);
    if (config_file_abi && generic_1200_regs) {
        printf("Specifying both -R and -x abi= is not allowed. Use one "
              "or the other.  -x abi= ignored.\n");
        config_file_abi = FALSE;
    }
    if(generic_1200_regs) {
        init_generic_config_1200_regs(&config_file_data);
    }
    if (config_file_abi && (frame_flag || eh_frame_flag)) {
        int res = find_conf_file_and_read_config(config_file_path,
                                                 config_file_abi,
                                                 config_file_defaults,
                                                 &config_file_data);

        if (res > 0) {
            printf
                ("Frame not configured due to error(s). Giving up.\n");
            eh_frame_flag = FALSE;
            frame_flag = FALSE;
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
    unsigned i;
    fprintf(stderr,"Usage:  %s  <options> <object file>\n", program_name);
    for (i = 0; usage_text[i]; ++i) {
        fprintf(stderr,"%s\n", usage_text[i]);
    }
}

void
print_error(Dwarf_Debug dbg, string msg, int dwarf_code,
            Dwarf_Error err)
{
    print_error_and_continue(dbg,msg,dwarf_code,err);
    exit(FAILED);
}
/* ARGSUSED */
void
print_error_and_continue(Dwarf_Debug dbg, string msg, int dwarf_code,
            Dwarf_Error err)
{
    fflush(stdout);
    fflush(stderr);
    if (dwarf_code == DW_DLV_ERROR) {
        string errmsg = dwarf_errmsg(err);
        Dwarf_Unsigned myerr = dwarf_errno(err);

        fprintf(stderr, "%s ERROR:  %s:  %s (%lu)\n",
                program_name, msg, errmsg, (unsigned long) myerr);
    } else if (dwarf_code == DW_DLV_NO_ENTRY) {
        fprintf(stderr, "%s NO ENTRY:  %s: \n", program_name, msg);
    } else if (dwarf_code == DW_DLV_OK) {
        fprintf(stderr, "%s:  %s \n", program_name, msg);
    } else {
        fprintf(stderr, "%s InternalError:  %s:  code %d\n",
                program_name, msg, dwarf_code);
    }
    fflush(stderr);
}

/* Predicate function. Returns 'true' if the CU should
 * be skipped as the DW_AT_name of the CU
 * does not match the command-line-supplied
 * cu name.  Else returns false.*/
boolean
should_skip_this_cu(Dwarf_Debug dbg, Dwarf_Die cu_die, Dwarf_Error err)
{
    Dwarf_Half tag;
    Dwarf_Attribute attrib;
    Dwarf_Half theform;
    int dares;
    int tres;
    int fres;

    tres = dwarf_tag(cu_die, &tag, &err);
    if (tres != DW_DLV_OK) {
        print_error(dbg, "dwarf_tag in aranges",
            tres, err);
    }
    dares = dwarf_attr(cu_die, DW_AT_name, &attrib,
                                       &err);
    if (dares != DW_DLV_OK) {
        print_error(dbg, "dwarf_attr arange"
            " derived die has no name",
            dares, err);
        }
    fres = dwarf_whatform(attrib, &theform, &err);
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
                if (strcmp(cu_name, p)) {
                   // skip this cu.
                   return TRUE;
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
     return FALSE;
}
