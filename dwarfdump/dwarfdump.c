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
#include "common.h"
#include "esb.h"                /* For flexible string buffer. */

#ifdef WIN32
extern int elf_open(char *name,int mode);
#endif

#define DWARFDUMP_VERSION " Tue Apr 10 11:43:32 PDT 2012  "

extern char *optarg;


#define OKAY 0
#define BYTES_PER_INSTRUCTION 4

static const char* process_args(int argc, char *argv[]);

char * program_name;
static int check_error = 0;

/*  The type of Bucket. */
#define KIND_RANGES_INFO       1
#define KIND_SECTIONS_INFO     2
#define KIND_VISITED_INFO      3

/* pRangesInfo records the DW_AT_high_pc and DW_AT_low_pc
   and is used to check that line range info falls inside
   the known valid ranges.   The data is per CU, and is
   reset per CU in tag_specific_checks_setup(). */
Bucket_Group *pRangesInfo = NULL;

/* pLinkonceInfo records data about the link once sections.
   If a line range is not valid in the current CU it might
   be valid in a linkonce section, this data records the
   linkonce sections.  So it is filled in when an
   object file is read and remains unchanged for an entire
   object file.  */
Bucket_Group *pLinkonceInfo = NULL;
/*  pVisitedInfo records a recursive traversal of DIE attributes
    DW_AT_specification DW_AT_abstract_origin DW_AT_type
    that let DWARF refer (as in a general graph) to
    arbitrary other DIEs.
    These traversals use pVisitedInfo to 
    detect any compiler errors that introduce circular references. 
    Printing of the traversals is also done on request.
    Entries are added and deleted as they are visited in
    a depth-first traversal.  */
Bucket_Group *pVisitedInfo = NULL;

/* Options to enable debug tracing */
int nTrace[MAX_TRACE_LEVEL + 1];

/* Build section information */
void build_linkonce_info(Dwarf_Debug dbg);
static const char * do_uri_translation(const char *s,
    const char *context);
static void reset_overall_CU_error_data();

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
static boolean aranges_flag = FALSE; /* .debug_aranges section. */
static boolean ranges_flag = FALSE; /* .debug_ranges section. */
static boolean string_flag = FALSE;
static boolean reloc_flag = FALSE;
static boolean static_func_flag = FALSE;
static boolean static_var_flag = FALSE;
static boolean type_flag = FALSE;
static boolean weakname_flag = FALSE;
static boolean header_flag = FALSE; /* Control printing of Elf header. */
boolean producer_children_flag = FALSE;   /* List of CUs per compiler */

/* Bitmap for relocations. See globals.h for DW_SECTION_REL_DEBUG_RANGES etc.*/
static unsigned reloc_map = 0;

/*  Start verbose at zero. verbose can
    be incremented with -v but not decremented. */
int verbose = 0; 

boolean dense = FALSE;
boolean ellipsis = FALSE;
boolean show_global_offsets = FALSE; /* Show global and relative offsets */
boolean show_form_used = FALSE;
boolean display_offsets = TRUE;  /* Emit offsets */

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
boolean check_abbreviations = FALSE; 
boolean check_dwarf_constants = FALSE; 
boolean check_di_gaps = FALSE; 
boolean check_forward_decl = FALSE; 
boolean check_self_references = FALSE; 
boolean generic_1200_regs = FALSE;
boolean suppress_check_extensions_tables = FALSE;

/* suppress_nested_name_search is a band-aid. 
   A workaround. A real fix for N**2 behavior is needed. 
*/
boolean suppress_nested_name_search = FALSE;
static boolean uri_options_translation = TRUE;
static boolean do_print_uri_in_input = TRUE;


/* break_after_n_units is mainly for testing.
   It enables easy limiting of output size/running time
   when one wants the output limited. 
   For example,
    -H 2
   limits the -i output to 2 compilation units and 
   the -f or -F output to 2 FDEs and 2 CIEs.
*/
int break_after_n_units = INT_MAX;

boolean check_names = FALSE;     
boolean check_verbose_mode = TRUE; /* During '-k' mode, display errors */
boolean check_frames = FALSE;    
boolean check_frames_extended = FALSE;    /* Extensive frames check */
boolean check_locations = FALSE;          /* Location list check */

static boolean check_all_compilers = TRUE; 
static boolean check_snc_compiler = FALSE; /* Check SNC compiler */
static boolean check_gcc_compiler = FALSE; 
static boolean print_summary_all = FALSE; 

#define COMPILER_TABLE_MAX 100
typedef struct anc {
    struct anc *next;
    char *item;
} a_name_chain;

/*  Records information about compilers (producers) found in the
    debug information, including the check results for several
    categories (see -k option). */
typedef struct {
    const char *name;
    boolean verified;
    a_name_chain *cu_list;
    a_name_chain *cu_last;
    Dwarf_Check_Result results[LAST_CATEGORY];
} Compiler;

/* Record compilers  whose CU names have been seen. 
   Full CU names recorded here, though only a portion
   of the name may have been checked to cause the
   compiler data  to be entered here.
   The +1 guarantees we do not overstep the array.
*/
static Compiler compilers_detected[COMPILER_TABLE_MAX];
static int compilers_detected_count = 0;

/* compilers_targeted is a list of indications of compilers
   on which we wish error checking (and the counts
   of checks made and errors found).   We do substring
   comparisons, so the compilers_targeted name might be simply a
   compiler version number or a short substring of a
   CU producer name.
   The +1 guarantees we do not overstep the array.
*/ 
static Compiler compilers_targeted[COMPILER_TABLE_MAX];
static int compilers_targeted_count = 0;
static int current_compiler = -1;

static void reset_compiler_entry(Compiler *compiler);
static void PRINT_CHECK_RESULT(char *str,
    Compiler *pCompiler, Dwarf_Check_Categories category);


/* The check and print flags here make it easy to 
   allow check-only or print-only.  We no longer support
   check-and-print in a single run.  */
boolean do_check_dwarf = FALSE;
boolean do_print_dwarf = FALSE;
boolean check_show_results = FALSE;  /* Display checks results. */
boolean record_dwarf_error = FALSE;  /* A test has failed, this
    is normally set FALSE shortly after being set TRUE, it is
    a short-range hint we should print something we might not
    otherwise print (under the circumstances). */


/* These names make diagnostic messages more complete, the
   fixed length is safe, though ultra long names will get
   truncated. */
char PU_name[COMPILE_UNIT_NAME_LEN];  
char CU_name[COMPILE_UNIT_NAME_LEN];  
char CU_producer[COMPILE_UNIT_NAME_LEN];

boolean seen_PU = FALSE;              /* Detected a PU */
boolean seen_CU = FALSE;              /* Detected a CU */
boolean need_CU_name = TRUE;          /* Need CU name */
boolean need_CU_base_address = TRUE;  /* Need CU Base address */
boolean need_CU_high_address = TRUE;  /* Need CU High address */
boolean need_PU_valid_code = TRUE;    /* Need PU valid code */

boolean seen_PU_base_address = FALSE; /* Detected a Base address for PU */
boolean seen_PU_high_address = FALSE; /* Detected a High address for PU */
Dwarf_Addr PU_base_address = 0;       /* PU Base address */
Dwarf_Addr PU_high_address = 0;       /* PU High address */

Dwarf_Off  DIE_offset = 0;            /* DIE offset in compile unit */
Dwarf_Off  DIE_overall_offset = 0;    /* DIE offset in .debug_info */

/*  These globals  enable better error reporting. */
Dwarf_Off  DIE_CU_offset = 0;         /* CU DIE offset in compile unit */
Dwarf_Off  DIE_CU_overall_offset = 0; /* CU DIE offset in .debug_info */
int current_section_id = 0;           /* Section being process */

Dwarf_Addr CU_base_address = 0;       /* CU Base address */
Dwarf_Addr CU_high_address = 0;       /* CU High address */

Dwarf_Addr elf_max_address = 0;       /* Largest representable address offset */
Dwarf_Half elf_address_size = 0;      /* Target pointer size */

/* Display parent/children when in wide format? */
boolean display_parent_tree = FALSE;
boolean display_children_tree = FALSE;
int stop_indent_level = 0;

/* Print search results in wide format? */
boolean search_wide_format = FALSE;
/* -S option: strings for 'any' and 'match' */
boolean search_is_on = FALSE;
const char *search_any_text = 0;
const char *search_match_text = 0;
const char *search_regex_text = 0;
#ifdef HAVE_REGEX
/* -S option: the compiled_regex */
regex_t search_re;
#endif


/* These configure items are for the 
   frame data.  We're pretty flexible in
   the path to dwarfdump.conf .
*/
static const char *config_file_path = 0;
static const char *config_file_abi = 0;
static char *config_file_defaults[] = {
    "dwarfdump.conf",
    "./dwarfdump.conf",
    "HOME/.dwarfdump.conf",
    "HOME/dwarfdump.conf",
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

Dwarf_Error err;

static void suppress_check_dwarf()
{
    do_print_dwarf = TRUE;
    if(do_check_dwarf) {
        fprintf(stderr,"Warning: check flag turned off, "
            "checking and printing are separate.\n");
    }
    do_check_dwarf = FALSE;
}
static void suppress_print_dwarf()
{
    do_print_dwarf = FALSE;
    do_check_dwarf = TRUE;
}

static int process_one_file(Elf * elf, const char * file_name, int archive,
    struct dwconf_s *conf);
static int
open_a_file(const char * name)
{
    /* Set to a file number that cannot be legal. */
    int f = -1;

#if defined(__CYGWIN__) || defined(WIN32)
    /*  It is not possible to share file handles
        between applications or DLLs. Each application has its own
        file-handle table. For two applications to use the same file
        using a DLL, they must both open the file individually.
        Let the 'libelf' dll to open and close the file.  */

    /* For WIN32 open the file as binary */
    f = elf_open(name, O_RDONLY | O_BINARY);
#else
    f = open(name, O_RDONLY);
#endif
    return f;

}
static void
close_a_file(int f)
{
    close(f);
}

/*
   Iterate through dwarf and print all info.
*/
int
main(int argc, char *argv[])
{
    const char * file_name = 0;
    int f = 0;
    Elf_Cmd cmd = 0;
    Elf *arf = 0; 
    Elf *elf = 0;
    int archive = 0;

#ifdef WIN32
    /* Windows specific. */
    /* Redirect stderr to stdout. */
    /* Tried to use SetStdHandle, but it does not work properly. */
    //BOOL bbb = SetStdHandle(STD_ERROR_HANDLE,GetStdHandle(STD_OUTPUT_HANDLE));
    //_iob[2]._file = _iob[1]._file;
    stderr->_file = stdout->_file;
#endif /* WIN32 */
  
    print_version_details(argv[0],FALSE);

    (void) elf_version(EV_NONE);
    if (elf_version(EV_CURRENT) == EV_NONE) {
        (void) fprintf(stderr, "dwarfdump: libelf.a out of date.\n");
        exit(1);
    }

    file_name = process_args(argc, argv);

    /*  Because LibDwarf now generates some new warnings,
        allow the user to hide them by using command line options */
    {
        Dwarf_Cmdline_Options cmd;
        cmd.check_verbose_mode = check_verbose_mode;
        dwarf_record_cmdline_options(cmd);
    }
    print_args(argc,argv);
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

    /*  If we are checking .debug_line, .debug_ranges, .debug_aranges,
        or .debug_loc build the tables containing 
        the pairs LowPC and HighPC. It is safer  (and not
        expensive) to build all
        of these at once so mistakes in options do not lead
        to coredumps (like -ka -p did once). */
    if (check_decl_file || check_ranges || check_locations ||
        do_check_dwarf || check_self_references) {
        pRangesInfo = AllocateBucketGroup(KIND_RANGES_INFO);
        pLinkonceInfo = AllocateBucketGroup(KIND_SECTIONS_INFO);
        pVisitedInfo = AllocateBucketGroup(KIND_VISITED_INFO);
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
                /* dwarfdump is almost-quiet when not an object */
                fprintf(stderr, "Can't process %s: unknown format\n",file_name);
                check_error = 1;
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

    if(pRangesInfo) {
        ReleaseBucketGroup(pRangesInfo);
        pRangesInfo = 0;
    }

    if(pLinkonceInfo) {
        ReleaseBucketGroup(pLinkonceInfo);
        pLinkonceInfo = 0;
    }

    if(pVisitedInfo) {
        ReleaseBucketGroup(pVisitedInfo);
        pVisitedInfo = 0;
    }

#ifdef HAVE_REGEX
    if(search_regex_text) {
        regfree(&search_re);
    }
#endif
    close_a_file(f);
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
        printf("\n*** HARMLESS ERROR COUNT: %u ***\n",totalcount);
    }
    for(i = 0 ; buf[i]; ++i) {
        ++printcount;
        DWARF_CHECK_COUNT(harmless_result,1);
        DWARF_CHECK_ERROR(harmless_result,buf[i]);
    }
    if(totalcount > printcount) {
        //harmless_result.checks += (totalcount - printcount);
        DWARF_CHECK_COUNT(harmless_result,(totalcount - printcount));
        //harmless_result.errors += (totalcount - printcount);
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
        printf("\nObject Header:\ne_ident:\n");
        printf("  File ID       = %s\n",eh_literals.e_ident_file_id);
        printf("  File class    = %02x (%s)\n",
            eh32->e_ident[EI_CLASS], eh_literals.e_ident_file_class);
        printf("  Data encoding = %02x (%s)\n",
            eh32->e_ident[EI_DATA], eh_literals.e_ident_data_encoding);
        printf("  File version  = %02x (%s)\n",
            eh32->e_ident[EI_VERSION], eh_literals.e_ident_file_version);
        printf("  OS ABI        = %02x (%s) (%s)\n", eh32->e_ident[EI_OSABI],
            eh_literals.e_ident_os_abi_s, eh_literals.e_ident_os_abi_l);
        //printf("  ABI version   = %02x (%s)\n",
        //  eh32->e_ident[EI_ABIVERSION], eh_literals.e_ident_abi_version);
        printf("e_type   : 0x%x (%s)\n",
            eh32->e_type, eh_literals.e_type);
        printf("e_machine: 0x%x (%s) (%s)\n", eh32->e_machine,
            eh_literals.e_machine_s, eh_literals.e_machine_l);
        printf("e_version: 0x%x\n", eh32->e_version);
        //printf("e_entry     = 0x%I64x\n", eh32->e_entry);
        printf("e_flags  : 0x%x\n", eh32->e_flags);
        printf("e_phnum  : 0x%x\n", eh32->e_phnum);
        printf("e_shnum  : 0x%x\n", eh32->e_shnum);
    }
    else {
#ifdef HAVE_ELF64_GETEHDR
        /* not a 32-bit obj */
        eh64 = elf64_getehdr(elf);
        if (eh64) {
            /* Get literal strings for header fields */
            elf64_gethdr_literals(eh64,&eh_literals);
            /* Print 64-bit obj header */
            printf("\nObject Header:\ne_ident:\n");
            printf("  File ID       = %s\n",eh_literals.e_ident_file_id);
            printf("  File class    = %02x (%s)\n",
                eh64->e_ident[EI_CLASS], eh_literals.e_ident_file_class);
            printf("  Data encoding = %02x (%s)\n",
                eh64->e_ident[EI_DATA], eh_literals.e_ident_data_encoding);
            printf("  File version  = %02x (%s)\n",
                eh64->e_ident[EI_VERSION], eh_literals.e_ident_file_version);
            printf("  OS ABI        = %02x (%s) (%s)\n", eh64->e_ident[EI_OSABI],
                eh_literals.e_ident_os_abi_s, eh_literals.e_ident_os_abi_l);
            //printf("  ABI version   = %02x (%s)\n",
            //  eh64->e_ident[EI_ABIVERSION], eh_literals.e_ident_abi_version);
            printf("e_type   : 0x%x (%s)\n",
                eh64->e_type, eh_literals.e_type);
            printf("e_machine: 0x%x (%s) (%s)\n", eh64->e_machine,
                eh_literals.e_machine_s, eh_literals.e_machine_l);
            printf("e_version: 0x%x\n", eh64->e_version);
            //printf("e_entry     = 0x%I64x\n", eh64->e_entry);
            printf("e_flags  : 0x%x\n", eh64->e_flags);
            printf("e_phnum  : 0x%x\n", eh64->e_phnum);
            printf("e_shnum  : 0x%x\n", eh64->e_shnum);
        }
#endif /* HAVE_ELF64_GETEHDR */
    }
#endif /* WIN32 */
}

/* Print checks and errors for a specific compiler */
static void
print_specific_checks_results(Compiler *pCompiler)
{
    fprintf(stderr, "\nDWARF CHECK RESULT\n");
        fprintf(stderr, "<item>                    <checks>    <errors>\n");
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

static int
qsort_compare_compiler(const void *elem1,const void *elem2)
{
  Compiler cmp1 = *(Compiler *)elem1;
  Compiler cmp2 = *(Compiler *)elem2;
  int cnt1 = cmp1.results[total_check_result].errors;
  int cnt2 = cmp2.results[total_check_result].errors;
  int sc = 0;

  if (cnt1 < cnt2) {
    return 1;
  } else if (cnt1 > cnt2) {
    return -1;
  }
  /* When error counts match, sort on name. */
  sc = strcmp(cmp2.name,cmp1.name);
  return sc;
}

/* Print a summary of checks and errors */
static void
print_checks_results()
{
    int index = 0;
    Compiler *pCompilers;
    Compiler *pCompiler;

    fflush(stdout);

    /* Sort based on errors detected; the first entry is reserved */
    pCompilers = &compilers_detected[1];
    qsort((void *)pCompilers, compilers_detected_count,
        sizeof(Compiler),qsort_compare_compiler);

    /* Print list of CUs for each compiler detected */
    if (producer_children_flag) {

        a_name_chain *nc = 0;
        a_name_chain *nc_next = 0;
        int count = 0;
        int total = 0;

        fprintf(stderr,"\n*** CU NAMES PER COMPILER ***\n");
        for (index = 1; index <= compilers_detected_count; ++index) {
            pCompiler = &compilers_detected[index];
            fprintf(stderr,"\n%02d: %s\n",index,pCompiler->name);
            count = 0;
            for (nc = pCompiler->cu_list; nc; nc = nc_next) {
                fprintf(stderr,"\n    %02d: '%s'",++count,nc->item);
                nc_next = nc->next;
                free(nc);
            }
            total += count;
            fprintf(stderr,"\n");
        }
        fprintf(stderr,"\nDetected %d CU names\n",total);
    }

    /* Print error report only if errors have been detected */
    /* Print error report if the -kd option */
    if ((do_check_dwarf && check_error) || check_show_results) {
        int count = 0;
        int compilers_not_detected = 0;
        int compilers_verified = 0;

        /* Find out how many compilers have been verified. */
        for (index = 1; index <= compilers_detected_count; ++index) {
            if (compilers_detected[index].verified) {
                ++compilers_verified;
            }
        }
        /* Find out how many compilers have been not detected. */
        for (index = 1; index <= compilers_targeted_count; ++index) {
            if (!compilers_targeted[index].verified) {
                ++compilers_not_detected;
            }
        }

        /* Print compilers detected list */
        fprintf(stderr,
            "\n%d Compilers detected:\n",compilers_detected_count);
        for (index = 1; index <= compilers_detected_count; ++index) {
            pCompiler = &compilers_detected[index];
            fprintf(stderr,"%02d: %s\n",index,pCompiler->name);
        }

        /*  Print compiler list specified by the user with the
            '-c<str>', that were not detected. */
        if (compilers_not_detected) {
            count = 0;
            fprintf(stderr,
                "\n%d Compilers not detected:\n",compilers_not_detected);
            for (index = 1; index <= compilers_targeted_count; ++index) {
                if (!compilers_targeted[index].verified) {
                    fprintf(stderr,
                        "%02d: '%s'\n",
                        ++count,compilers_targeted[index].name);
                }
            }
        }

        count = 0;
        fprintf(stderr,"\n%d Compilers verified:\n",compilers_verified);
        for (index = 1; index <= compilers_detected_count; ++index) {
            pCompiler = &compilers_detected[index];
            if (pCompiler->verified) {
                fprintf(stderr,"%02d: errors = %5d, %s\n",
                    ++count,
                    pCompiler->results[total_check_result].errors,
                    pCompiler->name);
            }
        }

        /*  Print summary if we have verified compilers or
            if the -kd option used. */
        if (compilers_verified || check_show_results) {
            /* Print compilers detected summary*/
            if (print_summary_all) {
                count = 0;
                fprintf(stderr,"\n*** ERRORS PER COMPILER ***\n");
                for (index = 1; index <= compilers_detected_count; ++index) {
                    pCompiler = &compilers_detected[index];
                    if (pCompiler->verified) {
                        fprintf(stderr,"%02d: %s\n",
                            ++count,pCompiler->name);
                        print_specific_checks_results(pCompiler);
                    }
                }
            }

            /* Print general summary (all compilers checked) */
            fprintf(stderr,"\n*** TOTAL ERRORS FOR ALL COMPILERS ***\n");
            print_specific_checks_results(&compilers_detected[0]);
        }
    }
}

/*
  Given a file which we know is an elf file, process
  the dwarf data.

*/
static int
process_one_file(Elf * elf, const char * file_name, int archive,
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
    if(config_file_data->cf_address_size) {
        dwarf_set_default_address_size(dbg, config_file_data->cf_address_size);
    }
    dwarf_set_harmless_error_list_size(dbg,50);


    /* Get address size and largest representable address */
    dres = dwarf_get_address_size(dbg,&elf_address_size,&err);
    if (dres != DW_DLV_OK) {
        print_error(dbg, "get_location_list", dres, err);
    }

    elf_max_address = (elf_address_size == 8 ) ? 
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
        if (pRangesInfo) {
            SetLimitsBucketGroup(pRangesInfo,lower,upper);
        }

        /* Build section information */
        build_linkonce_info(dbg);
    }

    if (header_flag) {
        print_object_header(elf,dbg);
    }
    reset_overall_CU_error_data();
    if (info_flag || line_flag || cu_name_flag || search_is_on || 
        producer_children_flag) {
        print_infos(dbg,TRUE);
        reset_overall_CU_error_data();
        print_infos(dbg,FALSE);
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
        current_cu_die_for_print_frames = 0;
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
        print_relocinfo(dbg, reloc_map);
    }
    /* The right time to do this is unclear. But we need to do it. */
    print_any_harmless_errors(dbg);

    /* Print error report only if errors have been detected */
    /* Print error report if the -kd option */
    print_checks_results();

    dres = dwarf_finish(dbg, &err);
    if (dres != DW_DLV_OK) {
        print_error(dbg, "dwarf_finish", dres, err);
    }

    printf("\n");
    fflush(stderr);
    return 0;

}

/* Do printing of most sections. 
   Do not do detailed checking.
*/
static void 
do_all()
{
    info_flag = line_flag = frame_flag = TRUE;
    pubnames_flag = macinfo_flag = TRUE;
    aranges_flag = TRUE;
    /*  Do not do 
        loc_flag = TRUE 
        abbrev_flag = TRUE;
        ranges_flag = TRUE;
        because nothing in
        the DWARF spec guarantees the sections are free of random bytes
        in areas not referenced by .debug_info */
    string_flag = TRUE;
    /*  Do not do 
        reloc_flag = TRUE;
        as print_relocs makes no sense for non-elf dwarfdump users.  */  
    static_func_flag = static_var_flag = TRUE;
    type_flag = weakname_flag = TRUE;
    header_flag = TRUE; /* Dump header info */
}


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

0
};


/* Remove matching leading/trailing quotes.
   Does not alter the passed in string.
   If quotes removed does a makename on a modified string. */ 
static const char *
remove_quotes_pair(const char *text)
{
    static char single_quote = '\'';
    static char double_quote = '\"';
    char quote = 0;
    const char *p = text;
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
            char *altered = calloc(1,len+1);
            const char *str2 = 0;
            strcpy(altered,p+1);
            altered[len - 2] = '\0';
            str2 =  makename(altered);
            free(altered);
            return str2;
        }
    }
    return p;
}

/* process arguments and return object filename */
static const char *
process_args(int argc, char *argv[])
{
    extern int optind;
    int c = 0;
    boolean usage_error = FALSE;
    int oarg = 0;

    program_name = argv[0];

    suppress_check_dwarf();
    if (argv[1] != NULL && argv[1][0] != '-') {
        do_all();
    }

    /* j unused */
    while ((c = getopt(argc, argv,
        "#:abc::CdDeEfFgGhH:ik:lmMnNo::pPqQrRsS:t:u:UvVwW::x:yz")) != EOF) {

        switch (c) {
        /* Internal debug level setting. */
        case '#':
        {
            int nTraceLevel =  atoi(optarg);
            if (nTraceLevel > 0 && nTraceLevel <= MAX_TRACE_LEVEL) {
                nTrace[nTraceLevel] = 1;
            }
            break;
        }
        case 'M':
            show_form_used =  TRUE;
            break;
        case 'x':               /* Select abi/path to use */
            {
                const char *path = 0;
                const char *abi = 0;
                /*  -x name=<path> meaning name dwarfdump.conf file -x
                    abi=<abi> meaning select abi from dwarfdump.conf
                    file. Must always select abi to use dwarfdump.conf */
                if (strncmp(optarg, "name=", 5) == 0) {
                    path = do_uri_translation(&optarg[5],"-x name=");
                    if (strlen(path) < 1) {
                        goto badopt;
                    }
                    config_file_path = path;
                } else if (strncmp(optarg, "abi=", 4) == 0) {
                    abi = do_uri_translation(&optarg[4],"-x abi=");
                    if (strlen(abi) < 1) {
                        goto badopt;
                    }
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
            suppress_check_dwarf();
            break;
        case 'i':
            info_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'n':
            suppress_nested_name_search = TRUE;
            break;
        case 'l':
            line_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'f':
            frame_flag = TRUE;
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
            eh_frame_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'b':
            abbrev_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'p':
            pubnames_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'P':
            /* List of CUs per compiler */
            producer_children_flag = TRUE;
            break;
        case 'r':
            aranges_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'N':
            ranges_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'R':
            generic_1200_regs = TRUE;
            break;
        case 'm':
            macinfo_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'c':
            /* Specify compiler name. */
            if (optarg) {
                if ('s' == optarg[0]) {
                    /* -cs : Check SNC compiler */
                    check_snc_compiler = TRUE;
                    check_all_compilers = FALSE;
                }
                else {
                    if ('g' == optarg[0]) {
                        /* -cg : Check GCC compiler */
                        check_gcc_compiler = TRUE;
                        check_all_compilers = FALSE;
                    }
                    else {
                        /*  Assume a compiler version to check,
                            most likely a substring of a compiler name.  */
                        if ((compilers_targeted_count+1) < COMPILER_TABLE_MAX) {
                            Compiler *pCompiler = 0;
                            const char *cmp = 0;
                            cmp = do_uri_translation(optarg,"-c<compiler name>");
                            /* First compiler at position [1] */
                            compilers_targeted_count++;
                            pCompiler = &compilers_targeted[compilers_targeted_count];
                            reset_compiler_entry(pCompiler);
                            pCompiler->name = cmp;
                            check_all_compilers = FALSE;
                        } else {
                            fprintf(stderr, "Compiler table max %d exceeded, "
                                "limiting the tracked compilers to %d\n",
                                COMPILER_TABLE_MAX,COMPILER_TABLE_MAX);
                        }
                    }
                }
            } else {
                loc_flag = TRUE;
                suppress_check_dwarf();
            }
            break;
        case 'Q':
            /* Q suppresses section data printing. */
            do_print_dwarf = FALSE;
            break;
        case 'q':
            /* Suppress uri-did-transate notification */
            do_print_uri_in_input = FALSE;
            break;
        case 's':
            string_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'S':
            /* -S option: strings for 'any' and 'match' */
            {
                boolean err = TRUE;
                search_is_on = TRUE;
                const char *tempstr = 0;
                /* -S text */
                if (strncmp(optarg,"match=",6) == 0) {
               
                    search_match_text = makename(&optarg[6]);
                    tempstr = remove_quotes_pair(search_match_text);
                    search_match_text = do_uri_translation(tempstr,"-S match=");
                    if (strlen(search_match_text) > 0) {
                        err = FALSE;
                    }
                }
                else {
                    if (strncmp(optarg,"any=",4) == 0) {
                        search_any_text = makename(&optarg[4]);
                        tempstr = remove_quotes_pair(search_any_text);
                        search_any_text = do_uri_translation(tempstr,"-S any=");
                        if (strlen(search_any_text) > 0) {
                            err = FALSE;
                        }
                    }
#ifdef HAVE_REGEX
                    else {
                        if (strncmp(optarg,"regex=",6) == 0) {
                            search_regex_text = makename(&optarg[6]);
                            tempstr = remove_quotes_pair(
                                search_regex_text);
                            search_regex_text = do_uri_translation(tempstr,
                                "-S regex=");
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
            suppress_check_dwarf();
            do_all();
            break;
        case 'v':
            verbose++;
            break;
        case 'V':
            /* Display dwarfdump compilation date and time */
            print_version_details(argv[0],TRUE);
            exit(OKAY);
            break;
        case 'd':
            /*  This is sort of useless unless printing,
                but harmless, so we do not insist we
                are printing with suppress_check_dwarf(). */
            dense = TRUE;
            break;
        case 'D':
            /* Do not emit offset in output */
            display_offsets = FALSE;
            break;
        case 'e':
            suppress_check_dwarf();
            ellipsis = TRUE;
            break;
        case 'E':
            /* Object Header information (but maybe really print) */
            header_flag = TRUE;
            break;
        case 'o':
            reloc_flag = TRUE;
            if (optarg) {
                switch (optarg[0]) {
                case 'i': 
                    reloc_map |= (1 << DW_SECTION_REL_DEBUG_INFO); break;
                    reloc_map |= (1 << DW_SECTION_REL_DEBUG_TYPES); break;
                case 'l': reloc_map |= (1 << DW_SECTION_REL_DEBUG_LINE); break;
                case 'p': reloc_map |= (1 << DW_SECTION_REL_DEBUG_PUBNAMES); break;
                /*  Case a has no effect, no relocations can point out
                    of the abbrev section. */
                case 'a': reloc_map |= (1 << DW_SECTION_REL_DEBUG_ABBREV); break;
                case 'r': reloc_map |= (1 << DW_SECTION_REL_DEBUG_ARANGES); break;
                case 'f': reloc_map |= (1 << DW_SECTION_REL_DEBUG_FRAME); break;
                case 'o': reloc_map |= (1 << DW_SECTION_REL_DEBUG_LOC); break;
                case 'R': reloc_map |= (1 << DW_SECTION_REL_DEBUG_RANGES); break;
                default: usage_error = TRUE; break;
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
                check_pubname_attr = TRUE;
                check_attr_tag = TRUE;
                check_tag_tree = check_type_offset = TRUE;
                check_names = TRUE;  
                pubnames_flag = info_flag = TRUE;
                check_decl_file = TRUE;
                check_frames = TRUE;  
                /*check_frames_extended = FALSE; */
                check_locations = TRUE; 
                frame_flag = eh_frame_flag = TRUE;
                check_ranges = TRUE;
                check_lines = TRUE;
                check_fdes = TRUE;
                check_harmless = TRUE;
                check_aranges = TRUE;
                aranges_flag = TRUE;  /* Aranges section */
                check_abbreviations = TRUE; 
                check_dwarf_constants = TRUE; 
                check_di_gaps = TRUE; /* Check debug info gaps */
                check_forward_decl = TRUE;  /* Check forward declarations */
                check_self_references = TRUE;  /* Check self references */
                break;
            /* Abbreviations */
            case 'b':
                check_abbreviations = TRUE;
                info_flag = TRUE;
                break;
            /* DWARF constants */
            case 'c':
                check_dwarf_constants = TRUE;
                info_flag = TRUE;
                break;
            /* Display check results */
            case 'd':
                check_show_results = TRUE;
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
            /* files-lines */
            case 'F':
                check_decl_file = TRUE;
                check_lines = TRUE;
                info_flag = TRUE;
                break;
            /* Check debug info gaps */
            case 'g':
                check_di_gaps = TRUE;
                info_flag = TRUE;
                break;
            /* Locations list */
            case 'l':
                check_locations = TRUE;
                info_flag = TRUE;
                loc_flag = TRUE;
                break;
            /* Ranges */
            case 'm':
                check_ranges = TRUE;
                info_flag = TRUE;
                break;
            /* Aranges */
            case 'M':
                check_aranges = TRUE;
                aranges_flag = TRUE;
                break;
            /* invalid names */
            case 'n':
                check_names = TRUE;
                info_flag = TRUE;
                break;
            case 'r':
                check_attr_tag = TRUE;
                info_flag = TRUE;
                check_harmless = TRUE;
                break;
            /* forward declarations in DW_AT_specification */
            case 'R':
                check_forward_decl = TRUE;
                info_flag = TRUE;
                break;
            /* Check verbose mode */
            case 's':
                check_verbose_mode = FALSE;
                break;
            /*  self references in:
                DW_AT_specification, DW_AT_type, DW_AT_abstract_origin */
            case 'S':
                check_self_references = TRUE;
                info_flag = TRUE;
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
            /* Summary for each compiler */
            case 'i':
                print_summary_all = TRUE;
                break;
            /* Frames check */
            case 'x':
                check_frames = TRUE;
                frame_flag = TRUE;
                eh_frame_flag = TRUE;
                if (optarg[1]) {
                    if ('e' == optarg[1]) {
                        /* -xe : Extended frames check */
                        check_frames = FALSE;
                        check_frames_extended = TRUE;
                    } else {
                        usage_error = TRUE;
                    }
                }
                break;
            default:
                usage_error = TRUE;
                break;
            }
            break;
        case 'u': {             /* compile unit */
            const char *tstr = 0;
            cu_name_flag = TRUE;
            tstr = do_uri_translation(optarg,"-u<cu name>");
            safe_strcpy(cu_name,sizeof(cu_name), tstr,strlen(tstr));
            }
            break;
        case 'U': 
            uri_options_translation = FALSE;
            break;
        case 't':
            oarg = optarg[0];
            switch (oarg) {
            case 'a':
                /* all */
                static_func_flag = static_var_flag = TRUE;
                suppress_check_dwarf();
                break;
            case 'f':
                /* .debug_static_func */
                static_func_flag = TRUE;
                suppress_check_dwarf();
                break;
            case 'v':
                /* .debug_static_var */
                static_var_flag = TRUE;
                suppress_check_dwarf();
                break;
            default:
                usage_error = TRUE;
                break;
            }
            break;
        case 'y':               /* .debug_types */
            suppress_check_dwarf();
            type_flag = TRUE;
            break;
        case 'w':               /* .debug_weaknames */
            weakname_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'z':
            fprintf(stderr, "-z is no longer supported:ignored\n");
            break;
        case 'G':
            show_global_offsets = TRUE;
            break;
        case 'W':
            /* Search results in wide format */
            search_wide_format = TRUE;
            if (optarg) {
                if ('c' == optarg[0]) {
                    /* -Wc : Display children tree */
                    display_children_tree = TRUE;
                } else {
                    if ('p' == optarg[0]) {
                        /* -Wp : Display parent tree */
                        display_parent_tree = TRUE;
                    } else {
                        usage_error = TRUE;
                    }
                }
            }
            else {
                /* -W : Display parent and children tree */
                display_children_tree = TRUE;
                display_parent_tree = TRUE;
            }
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
        print_usage_message(program_name,usage_text);
        exit(FAILED);
    }

    if (do_check_dwarf) {
        /* Reduce verbosity when checking (checking means checking-only). */
        verbose = 1;
    }
    return do_uri_translation(argv[optind],"file-to-process"); 
}


void
print_error(Dwarf_Debug dbg, string msg, int dwarf_code,
    Dwarf_Error err)
{
    print_error_and_continue(dbg,msg,dwarf_code,err);
    dwarf_finish(dbg, &err);
    exit(FAILED);
}
/* ARGSUSED */
void
print_error_and_continue(Dwarf_Debug dbg, string msg, int dwarf_code,
    Dwarf_Error err)
{
    fflush(stdout);
    fflush(stderr);

    fprintf(stderr,"\n");

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

    /* Display compile unit name */
    PRINT_CU_INFO();
}

/*  Predicate function. Returns 'true' if the CU should
    be skipped as the DW_AT_name of the CU
    does not match the command-line-supplied
    cu name.  Else returns false.*/
boolean
should_skip_this_cu(Dwarf_Debug dbg, Dwarf_Die cu_die, Dwarf_Error err)
{
    Dwarf_Half tag = 0;
    Dwarf_Attribute attrib = 0;
    Dwarf_Half theform = 0;
    int dares = 0;
    int tres = 0;
    int fres = 0;

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
                /* Ignore case if Windows */
#if WIN32
                if (stricmp(cu_name, p)) {
                    // skip this cu.
                    return TRUE;
                }
#else
                if (strcmp(cu_name, p)) {
                    // skip this cu.
                    return TRUE;
                }
#endif /* WIN32 */

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

/* Returns the DW_AT_name of the CU */
string
old_get_cu_name(Dwarf_Debug dbg, Dwarf_Die cu_die, Dwarf_Error err)
{
    static struct esb_s esb_attr_name;
    Dwarf_Half tag;
    Dwarf_Attribute attrib;
    Dwarf_Half theform;
    int dares;
    int tres;
    int fres;

    /* Initialize flexible string buffer */
    esb_empty_string(&esb_attr_name);

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
                esb_append(&esb_attr_name,p);
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

    /* Return the esb internal string */
    return esb_get_string(&esb_attr_name);
}

/* Returns the cu of the CU */
int get_cu_name(Dwarf_Debug dbg, Dwarf_Die cu_die,
    Dwarf_Error err, string *short_name, string *long_name)
{
    Dwarf_Attribute name_attr = 0;
    int ares;

    ares = dwarf_attr(cu_die, DW_AT_name, &name_attr, &err);
    if (ares == DW_DLV_ERROR) {
        print_error(dbg, "hassattr on DW_AT_name", ares, err);
    } else {
        if (ares == DW_DLV_NO_ENTRY) {
            *short_name = "<unknown name>";
            *long_name = "<unknown name>";
        } else {
            /* DW_DLV_OK */
            /*  The string return is valid until the next call to this
                function; so if the caller needs to keep the returned
                string, the string must be copied (makename()). */
            static struct esb_s esb_short_name;
            static struct esb_s esb_long_name;
            char *filename;
            esb_empty_string(&esb_long_name);
            get_attr_value(dbg, DW_TAG_compile_unit, 
                cu_die, name_attr, NULL, 0, &esb_long_name,
                0 /*show_form_used*/,0 /* verbose */);
            *long_name = esb_get_string(&esb_long_name);
            /* Generate the short name (filename) */
            filename = strrchr(*long_name,'/');
            if (!filename) {
                filename = strrchr(*long_name,'\\');
            }
            if (filename) {
                ++filename;
            } else {
                filename = *long_name;
            }
            esb_empty_string(&esb_short_name);
            esb_append(&esb_short_name,filename);
            *short_name = esb_get_string(&esb_short_name);
        }
    }

    dwarf_dealloc(dbg, name_attr, DW_DLA_ATTR);
    return ares;
}

/* Returns the producer of the CU */
int get_producer_name(Dwarf_Debug dbg, Dwarf_Die cu_die,
    Dwarf_Error err, string *producer_name)
{
    Dwarf_Attribute producer_attr = 0;
    int ares;

    ares = dwarf_attr(cu_die, DW_AT_producer, &producer_attr, &err);
    if (ares == DW_DLV_ERROR) {
        print_error(dbg, "hassattr on DW_AT_producer", ares, err);
    } else {
        if (ares == DW_DLV_NO_ENTRY) {
            /*  We add extra quotes so it looks more like
                the names for real producers that get_attr_value
                produces. */
            *producer_name = "\"<CU-missing-DW_AT_producer>\"";
        } else {
            /*  DW_DLV_OK */
            /*  The string return is valid until the next call to this
                function; so if the caller needs to keep the returned
                string, the string must be copied (makename()). */
            static struct esb_s esb_producer;
            esb_empty_string(&esb_producer);
            get_attr_value(dbg, DW_TAG_compile_unit, 
                cu_die, producer_attr, NULL, 0, &esb_producer,
                0 /*show_form_used*/,0 /* verbose */);
            *producer_name = esb_get_string(&esb_producer);
        }
    }

    dwarf_dealloc(dbg, producer_attr, DW_DLA_ATTR);
    return ares;
}

/* GCC linkonce names */
char *lo_text           = ".text.";               /*".gnu.linkonce.t.";*/
char *lo_debug_abbr     = ".gnu.linkonce.wa.";
char *lo_debug_aranges  = ".gnu.linkonce.wr.";
char *lo_debug_frame_1  = ".gnu.linkonce.wf.";
char *lo_debug_frame_2  = ".gnu.linkonce.wF.";
char *lo_debug_info     = ".gnu.linkonce.wi.";
char *lo_debug_line     = ".gnu.linkonce.wl.";
char *lo_debug_macinfo  = ".gnu.linkonce.wm.";
char *lo_debug_loc      = ".gnu.linkonce.wo.";
char *lo_debug_pubnames = ".gnu.linkonce.wp.";
char *lo_debug_ranges   = ".gnu.linkonce.wR.";
char *lo_debug_str      = ".gnu.linkonce.ws.";

/* SNC compiler/linker linkonce names */
char *nlo_text           = ".text.";
char *nlo_debug_abbr     = ".debug.wa.";
char *nlo_debug_aranges  = ".debug.wr.";
char *nlo_debug_frame_1  = ".debug.wf.";
char *nlo_debug_frame_2  = ".debug.wF.";
char *nlo_debug_info     = ".debug.wi.";
char *nlo_debug_line     = ".debug.wl.";
char *nlo_debug_macinfo  = ".debug.wm.";
char *nlo_debug_loc      = ".debug.wo.";
char *nlo_debug_pubnames = ".debug.wp.";
char *nlo_debug_ranges   = ".debug.wR.";
char *nlo_debug_str      = ".debug.ws.";

/* Build linkonce section information */
void 
build_linkonce_info(Dwarf_Debug dbg)
{
    int nCount = 0;
    int section_index = 0;
    int res = 0;

    static char **linkonce_names[] = {
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
                    AddEntryIntoBucketGroup(pLinkonceInfo,
                        section_index,
                        section_addr,section_addr,
                        section_addr + section_size,
                        section_name,
                        TRUE);
                    break;
                }
            }
        }
    }

    if (dump_linkonce_info) {
        PrintBucketGroup(pLinkonceInfo,TRUE);
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
        seen_CU = TRUE;
        /*  If we are checking line information, build
            the table containing the pairs LowPC and HighPC */
        if (check_decl_file || check_ranges || check_locations) {
            ResetBucketGroup(pRangesInfo);
        }
        /*  The following flag indicate that only low_pc and high_pc
            values found in DW_TAG_subprograms are going to be considered when
            building the address table used to check ranges, lines, etc */
        need_PU_valid_code = TRUE;
        break;

    case DW_TAG_subprogram:
        /* Keep track of a PU */
        if (die_indent_level == 1) {
            /*  A DW_TAG_subprogram can be nested, when is used to
                declare a member function for a local class; process the DIE
                only if we are at level zero in the DIEs tree */
            seen_PU = TRUE;
            seen_PU_base_address = FALSE;
            seen_PU_high_address = FALSE;
            PU_name[0] = 0;
            need_PU_valid_code = TRUE;
        }
        break;
    }
}

/* Indicates if the current CU is a target */
static boolean current_cu_is_checked_compiler = TRUE;

/*  Are we checking for errors from the
    compiler of the current compilation unit?
*/
boolean 
checking_this_compiler()
{
    /*  This flag has been update by 'update_compiler_target()'
        and indicates if the current CU is in a targeted compiler
        specified by the user. Default value is TRUE, which
        means test all compilers until a CU is detected. */
    return current_cu_is_checked_compiler || check_all_compilers;
}

static int
hasprefix(const char *sample, const char *prefix)
{
    unsigned prelen = strlen(prefix);
    if ( strncmp(sample,prefix,prelen) == 0) {
        return TRUE;
    }
    return FALSE;
}

/*  Record which compiler was used (or notice we saw
    it before) and set a couple variables as
    a side effect (which are used all over):
        current_cu_is_checked_compiler (used in checking_this_compiler() )
        current_compiler
    The compiler name is from DW_AT_producer.
*/
void 
update_compiler_target(const char *producer_name)
{
    Dwarf_Bool cFound = FALSE;
    int index = 0;

    safe_strcpy(CU_producer,sizeof(CU_producer),producer_name,
        strlen(producer_name));
    current_cu_is_checked_compiler = FALSE;

    /* This list of compilers is just a start:
        GCC id : "GNU"
        SNC id : "SN Systems" */

    /* Find a compiler version to check */
    if (compilers_targeted_count) {
        for (index = 1; index <= compilers_targeted_count; ++index) {
            if (is_strstrnocase(CU_producer,compilers_targeted[index].name)) {
                compilers_targeted[index].verified = TRUE;
                current_cu_is_checked_compiler = TRUE;
                break;
            }
        }
    } else {
        /* Take into account that internally all strings are double quoted */
        boolean snc_compiler = hasprefix(CU_producer,"\"SN")? TRUE : FALSE;
        boolean gcc_compiler = hasprefix(CU_producer,"\"GNU")?TRUE : FALSE; 
        current_cu_is_checked_compiler = check_all_compilers ||
            (snc_compiler && check_snc_compiler) ||
            (gcc_compiler && check_gcc_compiler) ;
    }

    /* Check for already detected compiler */
    for (index = 1; index <= compilers_detected_count; ++index) {
        if (
#if WIN32
            !stricmp(compilers_detected[index].name,CU_producer)
#else
            !strcmp(compilers_detected[index].name,CU_producer)
#endif
            ) {
            /* Set current compiler index */
            current_compiler = index;
            cFound = TRUE;
            break;
        }
    }
    if (!cFound) {
        /* Record a new detected compiler name. */
        if (compilers_detected_count + 1 < COMPILER_TABLE_MAX) {
            Compiler *pCompiler = 0;
            char *cmp = makename(CU_producer);
            /* Set current compiler index, first compiler at position [1] */
            current_compiler = ++compilers_detected_count;
            pCompiler = &compilers_detected[current_compiler];
            reset_compiler_entry(pCompiler);
            pCompiler->name = cmp;
        }
    }
}

/*  Add a CU name to the current compiler entry, specified by the
    'current_compiler'; the name is added to the 'compilers_detected'
    table and is printed if the '-P' option is specified in the
    command line. */
void
add_cu_name_compiler_target(char *name)
{
    a_name_chain *cu_last = 0;
    a_name_chain *nc = 0;
    Compiler *pCompiler = 0;

    if (current_compiler < 1) {
        fprintf(stderr,"Current  compiler set to %d, cannot add "
            "Compilation unit name.  Giving up.",current_compiler);
        exit(1);
    }
    pCompiler = &compilers_detected[current_compiler];
    cu_last = pCompiler->cu_last;
    /* Record current cu name */
    nc = (a_name_chain *)malloc(sizeof(a_name_chain));
    nc->item = makename(name);
    nc->next = NULL;
    if (cu_last) {
        cu_last->next = nc;
    } else {
        pCompiler->cu_list = nc;
    }
    pCompiler->cu_last = nc;
}

/* Reset a compiler entry, so all fields are properly set */
static void
reset_compiler_entry(Compiler *compiler)
{
    memset(compiler,0,sizeof(Compiler));
}

/*  Making this a named string makes it simpler to change
    what the reset,or 'I do not know'  value is for
    CU name or producer name for PRINT_CU_INFO. */
static const char * default_cu_producer = "<unknown>";
static void
reset_overall_CU_error_data()
{
   strcpy(CU_name,default_cu_producer);
   strcpy(CU_producer,default_cu_producer);
   DIE_offset = 0;
   DIE_overall_offset = 0;
   DIE_CU_offset = 0;
   DIE_CU_overall_offset = 0;
   CU_base_address = 0;
   CU_high_address = 0;
}


static boolean 
cu_data_is_set()
{
    if(strcmp(CU_name,default_cu_producer) || 
        strcmp(CU_producer,default_cu_producer)) {
        return 1;
    }
    if(DIE_offset  || DIE_overall_offset) {
        return 1;
    }
    if(CU_base_address || CU_high_address) {
        return 1;
    }
    return 0;
}
/* Print CU basic information */
void PRINT_CU_INFO()
{
    if (current_section_id == DEBUG_LINE || 
        current_section_id == DEBUG_ARANGES) {
        /*  Only in the DEBUG_LINE/ARANGES case is DIE_CU_offset or 
            DIE_CU_overall_offset what we want to print here.
            In other cases DIE_CU_offset is not really a CU
            offset at all. */
        DIE_offset = DIE_CU_offset;
        DIE_overall_offset = DIE_CU_overall_offset;
    }
    if(!cu_data_is_set()) {
        return;
    }
    printf("\n");
    printf("CU Name = %s\n",CU_name);
    printf("CU Producer = %s\n",CU_producer);
    printf("DIE OFF = 0x%08" DW_PR_DUx
        " GOFF = 0x%08" DW_PR_DUx ,DIE_offset,DIE_overall_offset);
    printf(", Low PC = 0x%08" DW_PR_DUx ", High PC = 0x%08" DW_PR_DUx ,
        CU_base_address,CU_high_address);
    printf("\n");
    fflush(stdout);
}

void DWARF_CHECK_COUNT(Dwarf_Check_Categories category, int inc)
{
    compilers_detected[0].results[category].checks += inc;
    compilers_detected[0].results[total_check_result].checks += inc;
    if(current_compiler > 0 && current_compiler <  COMPILER_TABLE_MAX) {
        compilers_detected[current_compiler].results[category].checks += inc;
        compilers_detected[current_compiler].results[total_check_result].checks
            += inc;
        compilers_detected[current_compiler].verified = TRUE;
    }
}

void DWARF_ERROR_COUNT(Dwarf_Check_Categories category, int inc)
{
    compilers_detected[0].results[category].errors += inc;
    compilers_detected[0].results[total_check_result].errors += inc;
    if(current_compiler > 0 && current_compiler <  COMPILER_TABLE_MAX) {
        compilers_detected[current_compiler].results[category].errors += inc;
        compilers_detected[current_compiler].results[total_check_result].errors
            += inc;
    }
}

void PRINT_CHECK_RESULT(char *str,
    Compiler *pCompiler, Dwarf_Check_Categories category)
{
    Dwarf_Check_Result result = pCompiler->results[category];
    fprintf(stderr, "%-24s%10d  %10d\n", str, result.checks, result.errors);
}

void DWARF_CHECK_ERROR_PRINT_CU()
{
    if (check_verbose_mode) {
        PRINT_CU_INFO();
    }
    check_error++;
    record_dwarf_error = TRUE;
}

void DWARF_CHECK_ERROR(Dwarf_Check_Categories category,
    const char *str)
{
    if (checking_this_compiler()) {
        DWARF_ERROR_COUNT(category,1);
        if (check_verbose_mode) {
            printf("\n*** DWARF CHECK: %s ***\n", str);
        }
        DWARF_CHECK_ERROR_PRINT_CU();
    }
}

void DWARF_CHECK_ERROR2(Dwarf_Check_Categories category,
    const char *str1, const char *str2)
{
    if (checking_this_compiler()) {
        DWARF_ERROR_COUNT(category,1);
        if (check_verbose_mode) {
            printf("\n*** DWARF CHECK: %s: %s ***\n", str1, str2);
        }
        DWARF_CHECK_ERROR_PRINT_CU();
    }
}

void DWARF_CHECK_ERROR3(Dwarf_Check_Categories category,
    const char *str1, const char *str2, const char *strexpl)
{
    if (checking_this_compiler()) {
        DWARF_ERROR_COUNT(category,1);
        if (check_verbose_mode) {
            printf("\n*** DWARF CHECK: %s -> %s: %s ***\n",
                str1, str2,strexpl);
        }
        DWARF_CHECK_ERROR_PRINT_CU();
    }
}

/* Precondition: 'out' is already constructed and empty. */
static const char *
do_uri_translation(const char *s,const char *context)
{
    struct esb_s str;
    char *finalstr = 0;
    if (!uri_options_translation) {
        return makename(s);
    }
    esb_constructor(&str);
    translate_from_uri(s,&str);
    if (do_print_uri_in_input) {
        if(strcmp(s,esb_get_string(&str))) {
            printf("Uri Translation on option %s\n",context);
            printf("    \'%s\'\n",s);
            printf("    \'%s\'\n",esb_get_string(&str));
        }
    }
    finalstr = makename(esb_get_string(&str));
    esb_destructor(&str);
    return finalstr;
}
