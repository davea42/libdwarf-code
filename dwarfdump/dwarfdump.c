/*
  Copyright (C) 2000,2002,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007-2017 David Anderson. All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2012 SN Systems Ltd. All rights reserved.:w

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
#ifdef HAVE_UNISTD_H
#include <unistd.h> /* for dup2() */
#endif
#include "dwgetopt.h"
#include "makename.h"
#include "dwconf.h"
#include "common.h"
#include "macrocheck.h"
#include "helpertree.h"
#include "uri.h"
#include "esb.h"                /* For flexible string buffer. */
#include "tag_common.h"

#ifdef _WIN32
extern int elf_open(const char *name,int mode);
#endif /* _WIN32 */

#define DW_VERSION_DATE_STR " 2017-07-09 16:38:09-07:00  "

extern char *dwoptarg;


#define OKAY 0
#define BYTES_PER_INSTRUCTION 4

static const char* process_args(int argc, char *argv[]);

const char * program_name;
static int check_error = 0;

/* used in special_program_name() only */
static struct esb_s newprogname;

/*  The type of Bucket. */
#define KIND_RANGES_INFO       1
#define KIND_SECTIONS_INFO     2
#define KIND_VISITED_INFO      3

/* pRangesInfo records the DW_AT_high_pc and DW_AT_low_pc
   and is used to check that line range info falls inside
   the known valid ranges.   The data is per CU, and is
   reset per CU in tag_specific_checks_setup(). */
Bucket_Group *pRangesInfo = NULL;

struct section_high_offsets_s section_high_offsets_global;

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

struct glflags_s glflags;

/* See section_bitmaps.c, .h Control section header
   printing. (not DWARF printing)  */
static char reloc_map[DW_SECTION_REL_ARRAY_SIZE];
static char section_map[DW_HDR_ARRAY_SIZE];

/*  Start verbose at zero. verbose can
    be incremented with -v but not decremented. */
int verbose = 0;

boolean dense = FALSE;
boolean ellipsis = FALSE;
boolean show_form_used = FALSE;

/* break_after_n_units is mainly for testing.
   It enables easy limiting of output size/running time
   when one wants the output limited.
   For example,
    -H 2
   limits the -i output to 2 compilation units and
   the -f or -F output to 2 FDEs and 2 CIEs.
*/
int break_after_n_units = INT_MAX;

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

/* These names make diagnostic messages more complete, the
   fixed length is safe, though ultra long names will get
   truncated. */
char PU_name[COMPILE_UNIT_NAME_LEN];
char CU_name[COMPILE_UNIT_NAME_LEN];
char CU_producer[COMPILE_UNIT_NAME_LEN];

/*  Base address has a special meaning in DWARF4 relative to address ranges. */
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

/*  Base Address is needed for range lists and must come from a CU.
    Low address is for information and can come from a function
    or something in the CU. */
Dwarf_Addr CU_base_address = 0;       /* CU Base address */
Dwarf_Addr CU_low_address = 0;        /* CU low address */
Dwarf_Addr CU_high_address = 0;       /* CU High address */

const char *search_any_text = 0;
const char *search_match_text = 0;
const char *search_regex_text = 0;
int search_occurrences = 0;

#ifdef HAVE_REGEX
/* -S option: the compiled_regex */
regex_t search_re;
#endif

/* Functions used to manage the unique errors table */
static void allocate_unique_errors_table(void);
static void release_unique_errors_table(void);
#ifdef TESTING
static void dump_unique_errors_table(void);
#endif
static boolean add_to_unique_errors_table(char * error_text);

/*  These configure items are for the
    frame data.  We're flexible in
    the path to dwarfdump.conf .
    The HOME strings here are transformed in
    dwconf.c to reference the environment
    variable $HOME .
*/
static struct esb_s config_file_path;
static struct esb_s config_file_tiedpath;
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
static struct dwconf_s g_config_file_data;

static struct esb_s esb_short_cu_name;
static struct esb_s esb_long_cu_name;
static struct esb_s dwarf_error_line;
static struct esb_s cu_name;

/* Output filename */
static const char *output_file = 0;
static int group_number = 0;


static int
get_number_value(char *v_in,long int *v_out)
{
    long int v= 0;
    size_t len = strlen(v_in);
    char *endptr = 0;

    if (len < 1) {
        return DW_DLV_ERROR;
    }
    v = strtol(v_in,&endptr,10);
    if (endptr == v_in) {
        return DW_DLV_NO_ENTRY;
    }
    if (*endptr != '\0') {
        return DW_DLV_ERROR;
    }
    *v_out = v;
    return DW_DLV_OK;
}

/*  When we add a 'print' option after an option
    requests one or more checks
    we turn off all checking, putting it back to default
    checking state. */
static void
set_checks_off()
{
    glflags.gf_check_abbrev_code = FALSE;
    glflags.gf_check_pubname_attr = FALSE;
    glflags.gf_check_reloc_offset = FALSE;
    glflags.gf_check_attr_tag = FALSE;
    glflags.gf_check_tag_tree = FALSE;
    glflags.gf_check_type_offset = FALSE;
    glflags.gf_check_decl_file = FALSE;
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
    glflags.gf_check_attr_encoding = FALSE;
    glflags.gf_check_duplicated_attributes = FALSE;
    glflags.gf_check_debug_names = FALSE;
}

static void suppress_check_dwarf()
{
    glflags.gf_do_print_dwarf = TRUE;
    if (glflags.gf_do_check_dwarf) {
        printf("Warning: check flag turned off, "
            "checking and printing are separate.\n");
    }
    glflags.gf_do_check_dwarf = FALSE;
    set_checks_off();
}
static void suppress_print_dwarf()
{
    glflags.gf_do_print_dwarf = FALSE;
    glflags.gf_do_check_dwarf = TRUE;
}

static int process_one_file(Elf * elf, Elf *elftied,
    const char * file_name,
    const char * tied_file_name,
    int archive,
    struct dwconf_s *conf);

static int
open_a_file(const char * name)
{
    /* Set to a file number that cannot be legal. */
    int f = -1;

#if defined(__CYGWIN__) || defined(_WIN32)
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

static int
is_it_known_elf_header(Elf *elf)
{
    Elf32_Ehdr *eh32;

    eh32 = elf32_getehdr(elf);
    if (eh32) {
        return 1;
    }
#ifdef HAVE_ELF64_GETEHDR
    {
        Elf64_Ehdr *eh64;
        /* not a 32-bit obj */
        eh64 = elf64_getehdr(elf);
        if (eh64) {
            return 1;
        }
    }
#endif /* HAVE_ELF64_GETEHDR */
    /* Not something we can handle. */
    return 0;
}

/*
   Iterate through dwarf and print all info.
*/
int
main(int argc, char *argv[])
{
    const char * file_name = 0;
    const char * tied_file_name = 0;
    int f = 0;
    int ftied = 0;
    Elf_Cmd cmd = 0;
    Elf *arf = 0;
    Elf *elf = 0;
    Elf *elftied = 0;
    int archive = 0;

#ifdef _WIN32
    /* Open the null device used during formatting printing */
    if (!esb_open_null_device()) {
        fprintf(stderr,"dwarfdump: Unable to open null device.\n");
        exit(FAILED);
    }
#endif /* _WIN32 */

    init_global_flags();
    set_checks_off();
    esb_constructor(&config_file_path);
    esb_constructor(&config_file_tiedpath);
    esb_constructor(&esb_short_cu_name);
    esb_constructor(&esb_long_cu_name);
    esb_constructor(&dwarf_error_line);
    esb_constructor(&cu_name);
#ifdef _WIN32
    /*  Often we redirect the output to a file, but we have found
        issues due to the buffering associated with stdout. Some issues
        were fixed just by the use of 'fflush', but the main issued
        remained.
        The stdout stream is buffered, so will only display what's in the
        buffer after it reaches a newline (or when it's told to). We have a
        few options to print immediately:
        - Print to stderr instead using fprintf.
        - Print to stdout and flush stdout whenever we need it to using fflush.
        - We can also disable buffering on stdout by using setbuf:
            setbuf(stdout,NULL);
            Make stdout unbuffered; this seems to work for all cases.
    */

    setbuf(stdout,NULL);
    /* Windows specific. */
    /* Redirect stderr to stdout. */
    /* Tried to use SetStdHandle, but it does not work properly. */
#if 0
    BOOL bbb = SetStdHandle(STD_ERROR_HANDLE,GetStdHandle(STD_OUTPUT_HANDLE));
    _iob[2]._file = _iob[1]._file;
    stderr->_file = stdout->_file;
#endif
    dup2(fileno(stdout),fileno(stderr));
#endif /* _WIN32 */

    print_version_details(argv[0],FALSE);

    (void) elf_version(EV_NONE);
    if (elf_version(EV_CURRENT) == EV_NONE) {
        (void) fprintf(stderr, "dwarfdump: libelf.a out of date.\n");
        exit(FAILED);
    }

    esb_constructor(&newprogname);
    file_name = process_args(argc, argv);
    print_args(argc,argv);

    /* Redirect stdout and stderr to an specific file */
    if (output_file) {
        if (NULL == freopen(output_file,"w",stdout)) {
            fprintf(stderr,
                "dwarfdump: Unable to redirect output to '%s'\n",output_file);
            exit(FAILED);
        }
        dup2(fileno(stdout),fileno(stderr));
        /* Record version and arguments in the output file */
        print_version_details(argv[0],TRUE);
        print_args(argc,argv);
    }

    /*  Because LibDwarf now generates some new warnings,
        allow the user to hide them by using command line options */
    {
        Dwarf_Cmdline_Options wcmd;
        /* The struct has just one field!. */
        wcmd.check_verbose_mode = glflags.gf_check_verbose_mode;
        dwarf_record_cmdline_options(wcmd);
    }


    f = open_a_file(file_name);
    if (f == -1) {
        fprintf(stderr, "%s ERROR:  can't open %s\n", program_name,
            file_name);
        return (FAILED);
    }

    cmd = ELF_C_READ;
    arf = elf_begin(f, cmd, (Elf *) 0);
    if (elf_kind(arf) == ELF_K_AR) {
        /* This option is never tested and may not work sensibly. */
        archive = 1;
    }

    if (esb_string_len(&config_file_tiedpath) > 0) {
        int isknown = 0;
        tied_file_name = esb_get_string(&config_file_tiedpath);
        ftied = open_a_file(tied_file_name);
        if (ftied == -1) {
            fprintf(stderr, "%s ERROR:  can't open tied file %s\n",
                program_name,
                tied_file_name);
            return (FAILED);
        }
        elftied = elf_begin(ftied, cmd, (Elf *) 0);
        if (elf_kind(elftied) == ELF_K_AR) {
            fprintf(stderr, "%s ERROR:  tied file  %s is "
                "an archive. Not allowed. Giving up.\n",
                program_name,
                tied_file_name);
            return (FAILED);
        }
        isknown = is_it_known_elf_header(elftied);
        if (!isknown) {
            fprintf(stderr,
                "Cannot process tied file %s: unknown format\n",
                tied_file_name);
            return FAILED;
        }
    }

    /*  If we are checking .debug_line, .debug_ranges, .debug_aranges,
        or .debug_loc build the tables containing
        the pairs LowPC and HighPC. It is safer  (and not
        expensive) to build all
        of these at once so mistakes in options do not lead
        to coredumps (like -ka -p did once). */
    if (glflags.gf_check_decl_file || glflags.gf_check_ranges ||
        glflags.gf_check_locations ||
        glflags.gf_do_check_dwarf ||
        glflags.gf_check_self_references) {
        pRangesInfo = AllocateBucketGroup(KIND_RANGES_INFO);
        pLinkonceInfo = AllocateBucketGroup(KIND_SECTIONS_INFO);
        pVisitedInfo = AllocateBucketGroup(KIND_VISITED_INFO);
    }

    /* Create the unique error table */
    if (glflags.gf_print_unique_errors) {
        allocate_unique_errors_table();
    }

    /* Allocate range array to be used by all CUs */
    if (glflags.gf_check_ranges) {
        allocate_range_array_info();
    }

    while ((elf = elf_begin(f, cmd, arf)) != 0) {
        int isknown = is_it_known_elf_header(elf);
        if (!isknown) {
            /* not a 64-bit obj either! */
            /* dwarfdump is almost-quiet when not an object */
            fprintf(stderr, "Can't process %s: unknown format\n",file_name);
            check_error = 1;
            cmd = elf_next(elf);
            elf_end(elf);
            continue;
        }
        memset(&section_high_offsets_global,0,
            sizeof(section_high_offsets_global));
        process_one_file(elf,elftied,
            file_name, tied_file_name,
            archive, &g_config_file_data);
        cmd = elf_next(elf);
        elf_end(elf);
    }
    elf_end(arf);
    if (elftied) {
        elf_end(elftied);
        elftied = 0;
    }
    /* Trivial malloc space cleanup. */
    clean_up_syms_malloc_data();

    if (pRangesInfo) {
        ReleaseBucketGroup(pRangesInfo);
        pRangesInfo = 0;
    }

    if (pLinkonceInfo) {
        ReleaseBucketGroup(pLinkonceInfo);
        pLinkonceInfo = 0;
    }

    if (pVisitedInfo) {
        ReleaseBucketGroup(pVisitedInfo);
        pVisitedInfo = 0;
    }

    /* Release range array to be used by all CUs */
    if (glflags.gf_check_ranges) {
        release_range_array_info();
    }

    /* Delete the unique error set */
    if (glflags.gf_print_unique_errors) {
        release_unique_errors_table();
    }

#ifdef HAVE_REGEX
    if (search_regex_text) {
        regfree(&search_re);
    }
#endif
    makename_destructor();
    esb_destructor(&config_file_path);
    esb_destructor(&config_file_tiedpath);
    esb_destructor(&esb_long_cu_name);
    esb_destructor(&esb_short_cu_name);
    esb_destructor(&dwarf_error_line);
    esb_destructor(&cu_name);
    sanitized_string_destructor();
    ranges_esb_string_destructor();
    destruct_abbrev_array();
    esb_destructor(&newprogname);

    close_a_file(f);

#ifdef _WIN32
    /* Close the null device used during formatting printing */
    esb_close_null_device();
#endif /* _WIN32 */

    /*  As the tool have reached this point, it means there are
        no internal errors and we should return an OKAY condition,
        regardless if the file being processed has errors. */
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
    if (res == DW_DLV_NO_ENTRY) {
        return;
    }
    if (totalcount > 0) {
        printf("\n*** HARMLESS ERROR COUNT: %u ***\n",totalcount);
    }
    for (i = 0 ; buf[i]; ++i) {
        ++printcount;
        DWARF_CHECK_COUNT(harmless_result,1);
        DWARF_CHECK_ERROR(harmless_result,buf[i]);
    }
    if (totalcount > printcount) {
        /*harmless_result.checks += (totalcount - printcount); */
        DWARF_CHECK_COUNT(harmless_result,(totalcount - printcount));
        /*harmless_result.errors += (totalcount - printcount); */
        DWARF_ERROR_COUNT(harmless_result,(totalcount - printcount));
    }
}

static void
print_object_header(UNUSEDARG Elf *elf,
    Dwarf_Debug dbg)
{
    /* Check if header information is required */
    if (section_map[DW_HDR_HEADER]) {
#ifdef _WIN32
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
            eh32->e_ident[EI_CLASS],eh_literals.e_ident_file_class);
        printf("  Data encoding = %02x (%s)\n",
            eh32->e_ident[EI_DATA],eh_literals.e_ident_data_encoding);
        printf("  File version  = %02x (%s)\n",
            eh32->e_ident[EI_VERSION],eh_literals.e_ident_file_version);
        printf("  OS ABI        = %02x (%s) (%s)\n",eh32->e_ident[EI_OSABI],
            eh_literals.e_ident_os_abi_s,eh_literals.e_ident_os_abi_l);
        printf("  ABI version   = %02x (%s)\n",
            eh32->e_ident[EI_ABIVERSION], eh_literals.e_ident_abi_version);
        printf("e_type     : 0x%x (%s)\n",
            eh32->e_type,eh_literals.e_type);
        printf("e_machine  : 0x%x (%s) (%s)\n",eh32->e_machine,
            eh_literals.e_machine_s,eh_literals.e_machine_l);
        printf("e_version  : 0x%x\n", eh32->e_version);
        printf("e_entry    : 0x%" DW_PR_XZEROS DW_PR_DUx "\n",eh32->e_entry);
        printf("e_phoff    : 0x%" DW_PR_XZEROS DW_PR_DUx "\n",eh32->e_phoff);
        printf("e_shoff    : 0x%" DW_PR_XZEROS DW_PR_DUx "\n",eh32->e_shoff);
        printf("e_flags    : 0x%x\n",eh32->e_flags);
        printf("e_ehsize   : 0x%x\n",eh32->e_ehsize);
        printf("e_phentsize: 0x%x\n",eh32->e_phentsize);
        printf("e_phnum    : 0x%x\n",eh32->e_phnum);
        printf("e_shentsize: 0x%x\n",eh32->e_shentsize);
        printf("e_shnum    : 0x%x\n",eh32->e_shnum);
        printf("e_shstrndx : 0x%x\n",eh32->e_shstrndx);
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
                eh64->e_ident[EI_CLASS],eh_literals.e_ident_file_class);
            printf("  Data encoding = %02x (%s)\n",
                eh64->e_ident[EI_DATA],eh_literals.e_ident_data_encoding);
            printf("  File version  = %02x (%s)\n",
                eh64->e_ident[EI_VERSION],eh_literals.e_ident_file_version);
            printf("  OS ABI        = %02x (%s) (%s)\n",eh64->e_ident[EI_OSABI],
                eh_literals.e_ident_os_abi_s,eh_literals.e_ident_os_abi_l);
            printf("  ABI version   = %02x (%s)\n",
                eh64->e_ident[EI_ABIVERSION], eh_literals.e_ident_abi_version);
            printf("e_type     : 0x%x (%s)\n",
                eh64->e_type,eh_literals.e_type);
            printf("e_machine  : 0x%x (%s) (%s)\n",eh64->e_machine,
                eh_literals.e_machine_s,eh_literals.e_machine_l);
            printf("e_version  : 0x%x\n", eh64->e_version);
            printf("e_entry    : 0x%" DW_PR_XZEROS DW_PR_DUx "\n",eh64->e_entry);
            printf("e_phoff    : 0x%" DW_PR_XZEROS DW_PR_DUx "\n",eh64->e_phoff);
            printf("e_shoff    : 0x%" DW_PR_XZEROS DW_PR_DUx "\n",eh64->e_shoff);
            printf("e_flags    : 0x%x\n",eh64->e_flags);
            printf("e_ehsize   : 0x%x\n",eh64->e_ehsize);
            printf("e_phentsize: 0x%x\n",eh64->e_phentsize);
            printf("e_phnum    : 0x%x\n",eh64->e_phnum);
            printf("e_shentsize: 0x%x\n",eh64->e_shentsize);
            printf("e_shnum    : 0x%x\n",eh64->e_shnum);
            printf("e_shstrndx : 0x%x\n",eh64->e_shstrndx);
        }
#endif /* HAVE_ELF64_GETEHDR */
    }
#endif /* _WIN32 */
    }
    /* Print basic section information is required */
    /* Mask only known sections (debug and text) bits */
    if (any_section_header_to_print(section_map)) {
        int nCount = 0;
        int section_index = 0;
        int res = 0;
        const char *section_name = NULL;
        Dwarf_Addr section_addr = 0;
        Dwarf_Unsigned section_size = 0;
        Dwarf_Error error = 0;
        boolean print_it = FALSE;
        Dwarf_Unsigned total_bytes = 0;
        int printed_sections = 0;

        /* Print section information (name, size, address). */
        nCount = dwarf_get_section_count(dbg);
        printf("\nInfo for %d sections:\n"
            "  Nro Index Address    Size(h)    Size(d)  Name\n",nCount);
        /* Ignore section with index=0 */
        for (section_index = 1; section_index < nCount; ++section_index) {
            res = dwarf_get_section_info_by_index(dbg,section_index,
                &section_name,
                &section_addr,
                &section_size,
                &error);
            if (res == DW_DLV_OK) {
                print_it = FALSE;
                /* Use original mapping */
                /* Check if the section name is a debug section */
                print_it = section_name_is_debug_and_wanted(section_name,
                    section_map);
                if (print_it) {
                    ++printed_sections;
                    printf("  %3d "                         /* nro */
                        "0x%03x "                        /* index */
                        "0x%" DW_PR_XZEROS DW_PR_DUx " " /* address */
                        "0x%" DW_PR_XZEROS DW_PR_DUx " " /* size (hex) */
                        "%" DW_PR_XZEROS DW_PR_DUu " "   /* size (dec) */
                        "%s\n",                          /* name */
                        printed_sections,
                        section_index,
                        section_addr,
                        section_size, section_size,
                        section_name);
                    total_bytes += section_size;
                }
            }
        }
        printf("*** Summary: %" DW_PR_DUu " bytes for %d section(s) ***\n",
            total_bytes, printed_sections);
    }
}

/* Print checks and errors for a specific compiler */
static void
print_specific_checks_results(Compiler *pCompiler)
{
    printf("\nDWARF CHECK RESULT\n");
    printf("<item>                    <checks>    <errors>\n");
    if (glflags.gf_check_pubname_attr) {
        PRINT_CHECK_RESULT("pubname_attr", pCompiler, pubname_attr_result);
    }
    if (glflags.gf_check_attr_tag) {
        PRINT_CHECK_RESULT("attr_tag", pCompiler, attr_tag_result);
    }
    if (glflags.gf_check_tag_tree) {
        PRINT_CHECK_RESULT("tag_tree", pCompiler, tag_tree_result);
    }
    if (glflags.gf_check_type_offset) {
        PRINT_CHECK_RESULT("type_offset", pCompiler, type_offset_result);
    }
    if (glflags.gf_check_decl_file) {
        PRINT_CHECK_RESULT("decl_file", pCompiler, decl_file_result);
    }
    if (glflags.gf_check_ranges) {
        PRINT_CHECK_RESULT("ranges", pCompiler, ranges_result);
    }
    if (glflags.gf_check_lines) {
        PRINT_CHECK_RESULT("line_table", pCompiler, lines_result);
    }
    if (glflags.gf_check_fdes) {
        PRINT_CHECK_RESULT("fde_table", pCompiler, fde_duplication);
    }
    if (glflags.gf_check_aranges) {
        PRINT_CHECK_RESULT("aranges", pCompiler, aranges_result);
    }

    if (glflags.gf_check_names) {
        PRINT_CHECK_RESULT("names",pCompiler, names_result);
    }
    if (glflags.gf_check_frames) {
        PRINT_CHECK_RESULT("frames",pCompiler, frames_result);
    }
    if (glflags.gf_check_locations) {
        PRINT_CHECK_RESULT("locations",pCompiler, locations_result);
    }

    if (glflags.gf_check_harmless) {
        PRINT_CHECK_RESULT("harmless_errors", pCompiler, harmless_result);
    }

    if (glflags.gf_check_abbreviations) {
        PRINT_CHECK_RESULT("abbreviations", pCompiler, abbreviations_result);
    }

    if (glflags.gf_check_dwarf_constants) {
        PRINT_CHECK_RESULT("dwarf_constants",
            pCompiler, dwarf_constants_result);
    }

    if (glflags.gf_check_di_gaps) {
        PRINT_CHECK_RESULT("debug_info_gaps", pCompiler, di_gaps_result);
    }

    if (glflags.gf_check_forward_decl) {
        PRINT_CHECK_RESULT("forward_declarations",
            pCompiler, forward_decl_result);
    }

    if (glflags.gf_check_self_references) {
        PRINT_CHECK_RESULT("self_references",
            pCompiler, self_references_result);
    }

    /* Display attributes encoding results */
    if (glflags.gf_check_attr_encoding) {
        PRINT_CHECK_RESULT("attr_encoding", pCompiler, attr_encoding_result);
    }

    /* Duplicated attributes */
    if (glflags.gf_check_duplicated_attributes) {
        PRINT_CHECK_RESULT("duplicated_attributes",
            pCompiler, duplicated_attributes_result);
    }

    PRINT_CHECK_RESULT("** Summarize **",pCompiler, total_check_result);
    fflush(stdout);
}

static int
qsort_compare_compiler(const void *elem1,const void *elem2)
{
    Compiler cmp1 = *(Compiler *)elem1;
    Compiler cmp2 = *(Compiler *)elem2;
    int cnt1 = cmp1.results[total_check_result].errors;
    int cnt2 = cmp2.results[total_check_result].errors;

    if (cnt1 < cnt2) {
        return 1;
    } else if (cnt1 > cnt2) {
        return -1;
    }
    /* When error counts match, sort on name. */
    {
        int v = strcmp(cmp2.name, cmp1.name);
        return v;
    }
}

/* Print a summary of search results */
static void
print_search_results(void)
{
    const char *search_type = 0;
    const char *search_text = 0;
    if (search_any_text) {
        search_type = "any";
        search_text = search_any_text;
    } else {
        if (search_match_text) {
            search_type = "match";
            search_text = search_match_text;
        } else {
            search_type = "regex";
            search_text = search_regex_text;
        }
    }
    fflush(stdout);
    fflush(stderr);
    printf("\nSearch type      : '%s'\n",search_type);
    printf("Pattern searched : '%s'\n",search_text);
    printf("Occurrences Found: %d\n",search_occurrences);
    fflush(stdout);
}

/* Print a summary of checks and errors */
static void
print_checks_results(void)
{
    int index = 0;
    Compiler *pCompilers;
    Compiler *pCompiler;

    /* Sort based on errors detected; the first entry is reserved */
    pCompilers = &compilers_detected[1];
    qsort((void *)pCompilers,compilers_detected_count,
        sizeof(Compiler),qsort_compare_compiler);

    /* Print list of CUs for each compiler detected */
    if (glflags.gf_producer_children_flag) {

        a_name_chain *nc = 0;
        a_name_chain *nc_next = 0;
        int count = 0;
        int total = 0;

        printf("\n*** CU NAMES PER COMPILER ***\n");
        for (index = 1; index <= compilers_detected_count; ++index) {
            pCompiler = &compilers_detected[index];
            printf("\n%02d: %s",index,pCompiler->name);
            count = 0;
            for (nc = pCompiler->cu_list; nc; nc = nc_next) {
                printf("\n    %02d: '%s'",++count,nc->item);
                nc_next = nc->next;
                free(nc);
            }
            total += count;
            printf("\n");
        }
        printf("\nDetected %d CU names\n",total);
    }

    /* Print error report only if errors have been detected */
    /* Print error report if the -kd option */
    if ((glflags.gf_do_check_dwarf && check_error) ||
        glflags.gf_check_show_results) {
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
        printf(
            "\n%d Compilers detected:\n",compilers_detected_count);
        for (index = 1; index <= compilers_detected_count; ++index) {
            pCompiler = &compilers_detected[index];
            printf("%02d: %s\n",index,pCompiler->name);
        }

        /*  Print compiler list specified by the user with the
            '-c<str>', that were not detected. */
        if (compilers_not_detected) {
            count = 0;
            printf(
                "\n%d Compilers not detected:\n",compilers_not_detected);
            for (index = 1; index <= compilers_targeted_count; ++index) {
                if (!compilers_targeted[index].verified) {
                    printf(
                        "%02d: '%s'\n",
                        ++count,compilers_targeted[index].name);
                }
            }
        }

        count = 0;
        printf("\n%d Compilers verified:\n",compilers_verified);
        for (index = 1; index <= compilers_detected_count; ++index) {
            pCompiler = &compilers_detected[index];
            if (pCompiler->verified) {
                printf("%02d: errors = %5d, %s\n",
                    ++count,
                    pCompiler->results[total_check_result].errors,
                    pCompiler->name);
            }
        }

        /*  Print summary if we have verified compilers or
            if the -kd option used. */
        if (compilers_verified || glflags.gf_check_show_results) {
            /* Print compilers detected summary*/
            if (glflags.gf_print_summary_all) {
                count = 0;
                printf("\n*** ERRORS PER COMPILER ***\n");
                for (index = 1; index <= compilers_detected_count; ++index) {
                    pCompiler = &compilers_detected[index];
                    if (pCompiler->verified) {
                        printf("\n%02d: %s",
                            ++count,pCompiler->name);
                        print_specific_checks_results(pCompiler);
                    }
                }
            }

            /* Print general summary (all compilers checked) */
            printf("\n*** TOTAL ERRORS FOR ALL COMPILERS ***\n");
            print_specific_checks_results(&compilers_detected[0]);
        }
    }
    fflush(stdout);
}

/* This is for dwarf_print_lines() */
static void
printf_callback_for_libdwarf(UNUSEDARG void *userdata,
    const char *data)
{
    printf("%s",data);
}


/*  Does not return on error. */
void
get_address_size_and_max(Dwarf_Debug dbg,
   Dwarf_Half * size,
   Dwarf_Addr * max,
   Dwarf_Error *aerr)
{
    int dres = 0;
    Dwarf_Half lsize = 4;
    /* Get address size and largest representable address */
    dres = dwarf_get_address_size(dbg,&lsize,aerr);
    if (dres != DW_DLV_OK) {
        print_error(dbg, "get_address_size()", dres, *aerr);
    }
    if(max) {
        *max = (lsize == 8 ) ? 0xffffffffffffffffULL : 0xffffffff;
    }
    if(size) {
        *size = lsize;
    }
}


/* dbg is often null when dbgtied was passed in. */
static void
dbgsetup(Dwarf_Debug dbg,struct dwconf_s *setup_config_file_data)
{
    if (!dbg) {
        return;
    }
    dwarf_set_frame_rule_initial_value(dbg,
        setup_config_file_data->cf_initial_rule_value);
    dwarf_set_frame_rule_table_size(dbg,
        setup_config_file_data->cf_table_entry_count);
    dwarf_set_frame_cfa_value(dbg,
        setup_config_file_data->cf_cfa_reg);
    dwarf_set_frame_same_value(dbg,
        setup_config_file_data->cf_same_val);
    dwarf_set_frame_undefined_value(dbg,
        setup_config_file_data->cf_undefined_val);
    if (setup_config_file_data->cf_address_size) {
        dwarf_set_default_address_size(dbg,
            setup_config_file_data->cf_address_size);
    }
    dwarf_set_harmless_error_list_size(dbg,50);
}

/*  Callable at any time, Sets section sizes with the sizes
    known as of the call.
    Repeat whenever about to  reference a size that might not
    have been set as of the last call. */
static void
set_global_section_sizes(Dwarf_Debug dbg)
{
    dwarf_get_section_max_offsets_c(dbg,
        &section_high_offsets_global.debug_info_size,
        &section_high_offsets_global.debug_abbrev_size,
        &section_high_offsets_global.debug_line_size,
        &section_high_offsets_global.debug_loc_size,
        &section_high_offsets_global.debug_aranges_size,
        &section_high_offsets_global.debug_macinfo_size,
        &section_high_offsets_global.debug_pubnames_size,
        &section_high_offsets_global.debug_str_size,
        &section_high_offsets_global.debug_frame_size,
        &section_high_offsets_global.debug_ranges_size,
        &section_high_offsets_global.debug_pubtypes_size,
        &section_high_offsets_global.debug_types_size,
        &section_high_offsets_global.debug_macro_size,
        &section_high_offsets_global.debug_str_offsets_size,
        &section_high_offsets_global.debug_sup_size,
        &section_high_offsets_global.debug_cu_index_size,
        &section_high_offsets_global.debug_tu_index_size);
}

/*
  Given a file which we know is an elf file, process
  the dwarf data.

*/
static int
process_one_file(Elf * elf,Elf *elftied,
    const char * file_name,
    const char * tied_file_name,
    int archive,
    struct dwconf_s *l_config_file_data)
{
    Dwarf_Debug dbg = 0;
    Dwarf_Debug dbgtied = 0;
    int dres = 0;
    struct Dwarf_Printf_Callback_Info_s printfcallbackdata;
    Dwarf_Half elf_address_size = 0;      /* Target pointer size */
    Dwarf_Error onef_err = 0;

    /*  If using a tied file group number should be 2 DW_GROUPNUMBER_DWO
        but in a dwp or separate-split-dwarf object then
        0 will find the .dwo data automatically. */
    dres = dwarf_elf_init_b(elf, DW_DLC_READ,group_number,
        NULL, NULL, &dbg, &onef_err);
    if (dres == DW_DLV_NO_ENTRY) {
        if (group_number > 0) {
            printf("No DWARF information present in %s "
                "for section group %d \n", file_name,group_number);
        } else {
            printf("No DWARF information present in %s\n",file_name);
        }
        return 0;
    }
    if (dres != DW_DLV_OK) {
        print_error(dbg, "dwarf_elf_init", dres, onef_err);
    }

    if (elftied) {
        /*  The tied file we define as group 1, BASE. */
        dres = dwarf_elf_init_b(elftied, DW_DLC_READ,
            DW_GROUPNUMBER_BASE, NULL, NULL, &dbgtied,
            &onef_err);
        if (dres == DW_DLV_NO_ENTRY) {
            printf("No DWARF information present in tied file: %s\n",
                tied_file_name);
            return 0;
        }
        if (dres != DW_DLV_OK) {
            print_error(dbg, "dwarf_elf_init on tied_file", dres, onef_err);
        }
    }

    memset(&printfcallbackdata,0,sizeof(printfcallbackdata));
    printfcallbackdata.dp_fptr = printf_callback_for_libdwarf;
    dwarf_register_printf_callback(dbg,&printfcallbackdata);
    if (dbgtied) {
        dwarf_register_printf_callback(dbgtied,&printfcallbackdata);
    }
    memset(&printfcallbackdata,0,sizeof(printfcallbackdata));



    if (archive) {
        Elf_Arhdr *mem_header = elf_getarhdr(elf);

        printf("\narchive member \t%s\n",
            mem_header ? mem_header->ar_name : "");
    }
    dbgsetup(dbg,l_config_file_data);
    dbgsetup(dbgtied,l_config_file_data);
    get_address_size_and_max(dbg,&elf_address_size,0,&onef_err);

    /*  Ok for dbgtied to be NULL. */
    dres = dwarf_set_tied_dbg(dbg,dbgtied,&onef_err);
    if (dres != DW_DLV_OK) {
        print_error(dbg, "dwarf_set_tied_dbg() failed", dres, onef_err);
    }

    /* Get .text and .debug_ranges info if in check mode */
    if (glflags.gf_do_check_dwarf) {
        Dwarf_Addr lower = 0;
        Dwarf_Addr upper = 0;
        Dwarf_Unsigned size = 0;
        int res = 0;
        res = dwarf_get_section_info_by_name(dbg,".text",&lower,&size,&onef_err);
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

    if (glflags.gf_header_flag) {
        print_object_header(elf,dbg);
    }

    if (glflags.gf_section_groups_flag) {
        print_section_groups_data(dbg);
        /*  If groupnum > 2 this turns off some
            of the gf_flags here so we don't print
            section names of things we do not
            want to print. */
        update_section_flags_per_groups(dbg);
    }

    reset_overall_CU_error_data();
    if (glflags.gf_info_flag || glflags.gf_line_flag ||
        glflags.gf_types_flag ||
        glflags.gf_check_macros || glflags.gf_macinfo_flag ||
        glflags.gf_macro_flag ||
        glflags.gf_cu_name_flag || glflags.gf_search_is_on ||
        glflags.gf_producer_children_flag) {

        print_infos(dbg,TRUE);
        reset_overall_CU_error_data();
        print_infos(dbg,FALSE);
        if (glflags.gf_check_macros) {
            set_global_section_sizes(dbg);
            if(macro_check_tree) {
                /* Fake item representing end of section. */
                /* Length of the fake item is zero. */
                print_macro_statistics("DWARF5 .debug_macro",
                    &macro_check_tree,
                    section_high_offsets_global.debug_macro_size);
            }
            if(macinfo_check_tree) {
                /* Fake item representing end of section. */
                /* Length of the fake item is zero. */
                print_macro_statistics("DWARF2 .debug_macinfo",
                    &macinfo_check_tree,
                    section_high_offsets_global.debug_macinfo_size);
            }
        }
        clear_macro_statistics(&macro_check_tree);
        clear_macro_statistics(&macinfo_check_tree);
    }
    if (glflags.gf_gdbindex_flag) {
        reset_overall_CU_error_data();
        /*  By definition if gdb_index is present
            then "cu" and "tu" will not be. And vice versa.  */
        print_gdb_index(dbg);
        print_debugfission_index(dbg,"cu");
        print_debugfission_index(dbg,"tu");
    }
    if (glflags.gf_pubnames_flag) {
        reset_overall_CU_error_data();
        print_pubnames(dbg);
    }
    if (glflags.gf_loc_flag) {
        reset_overall_CU_error_data();
        print_locs(dbg);
    }
    if (glflags.gf_abbrev_flag) {
        reset_overall_CU_error_data();
        print_abbrevs(dbg);
    }
    if (glflags.gf_string_flag) {
        reset_overall_CU_error_data();
        print_strings(dbg);
    }
    if (glflags.gf_aranges_flag) {
        reset_overall_CU_error_data();
        print_aranges(dbg);
    }
    if (glflags.gf_ranges_flag) {
        reset_overall_CU_error_data();
        print_ranges(dbg);
    }
    if (glflags.gf_frame_flag || glflags.gf_eh_frame_flag) {
        reset_overall_CU_error_data();
        current_cu_die_for_print_frames = 0;
        print_frames(dbg, glflags.gf_frame_flag,
            glflags.gf_eh_frame_flag, l_config_file_data);
    }
    if (glflags.gf_static_func_flag) {
        reset_overall_CU_error_data();
        print_static_funcs(dbg);
    }
    if (glflags.gf_static_var_flag) {
        reset_overall_CU_error_data();
        print_static_vars(dbg);
    }
    /*  DWARF_PUBTYPES is the standard typenames dwarf section.
        SGI_TYPENAME is the same concept but is SGI specific ( it was
        defined 10 years before dwarf pubtypes). */

    if (glflags.gf_pubtypes_flag) {
        reset_overall_CU_error_data();
        print_types(dbg, DWARF_PUBTYPES);
        reset_overall_CU_error_data();
        print_types(dbg, SGI_TYPENAME);
    }
    if (glflags.gf_weakname_flag) {
        reset_overall_CU_error_data();
        print_weaknames(dbg);
    }
    if (glflags.gf_reloc_flag) {
        reset_overall_CU_error_data();
        print_relocinfo(dbg, reloc_map);
    }
    if (glflags.gf_debug_names_flag) {
        reset_overall_CU_error_data();
        print_debug_names(dbg);
    }

    /* Print search results */
    if (glflags.gf_search_print_results && glflags.gf_search_is_on) {
        print_search_results();
    }

    /* The right time to do this is unclear. But we need to do it. */
    if (glflags.gf_check_harmless) {
        print_any_harmless_errors(dbg);
    }

    /* Print error report only if errors have been detected */
    /* Print error report if the -kd option */
    print_checks_results();

    /* Print the detailed attribute usage space */
    if (glflags.gf_check_attr_encoding) {
        print_attributes_encoding(dbg);
    }

    /* Print the tags and attribute usage */
    if (glflags.gf_print_usage_tag_attr) {
        print_tag_attributes_usage(dbg);
    }

    /*  Could finish dbg first. Either order ok. */
    if (dbgtied) {
        dres = dwarf_finish(dbgtied,&onef_err);
        if (dres != DW_DLV_OK) {
            print_error(dbg, "dwarf_finish on dbgtied", dres, onef_err);
        }
        dbgtied = 0;
    }
    dres = dwarf_finish(dbg, &onef_err);
    if (dres != DW_DLV_OK) {
        print_error(dbg, "dwarf_finish", dres, onef_err);
        dbg = 0;
    }
    printf("\n");
    helpertree_clear_statistics(&helpertree_offsets_base_info);
    helpertree_clear_statistics(&helpertree_offsets_base_types);
    return 0;
}

/* Do printing of most sections.
   Do not do detailed checking.
*/
static void
do_all(void)
{
    glflags.gf_frame_flag = TRUE;
    glflags.gf_info_flag = TRUE;
    glflags.gf_types_flag =  TRUE; /* .debug_types */
    glflags.gf_line_flag = TRUE;
    glflags.gf_pubnames_flag = TRUE;
    glflags.gf_macinfo_flag = TRUE;
    glflags.gf_macro_flag = TRUE;
    glflags.gf_aranges_flag = TRUE;
    /*  Do not do
        glflags.gf_loc_flag = TRUE
        glflags.gf_abbrev_flag = TRUE;
        glflags.gf_ranges_flag = TRUE;
        because nothing in
        the DWARF spec guarantees the sections are free of random bytes
        in areas not referenced by .debug_info */
    glflags.gf_string_flag = TRUE;
    /*  Do not do
        glflags.gf_reloc_flag = TRUE;
        as print_relocs makes no sense for non-elf dwarfdump users.  */
    glflags.gf_static_func_flag = TRUE; /* SGI only*/
    glflags.gf_static_var_flag = TRUE; /* SGI only*/
    glflags.gf_pubtypes_flag = TRUE;  /* both SGI typenames and dwarf_pubtypes. */
    glflags.gf_weakname_flag = TRUE; /* SGI only*/
    glflags.gf_header_flag = TRUE; /* Dump header info */
    glflags.gf_debug_names_flag = TRUE;
}

static const char *usage_text[] = {
"Usage: DwarfDump <options> <object file>",
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
"\t\t-E[hliaprfoRstxd]\tprint object Header and/or section information",
"\t\t  \th=header,l=line,i=info,a=abbrev,p=pubnames,r=aranges,",
"\t\t  \tf=frames,o=loc,R=Ranges,s=strings,t=pubtypes,x=text,",
"\t\t  \td=default sections, same as -E and {liaprfoRstx}",
"\t\t-f\tprint dwarf frame section",
"\t\t-F\tprint gnu .eh_frame section",
"\t\t-g\t(use incomplete loclist support)",
"\t\t-G\tshow global die offsets",
"\t\t-h\tprint IRIX exception tables (unsupported)",
"\t\t-H <num>\tlimit output to the first <num> major units",
"\t\t\t  example: to stop after <num> compilation units",
"\t\t-i\tprint info section",
"\t\t-I\tprint sections .gdb_index, .debug_cu_index, .debug_tu_index",
/* FIXME -kw is check macros */
"\t\t-k[abcdDeEfFgGilmMnrRsStu[f]x[e]y] check dwarf information",
"\t\t   a\tdo all checks",
"\t\t   b\tcheck abbreviations",     /* Check abbreviations */
"\t\t   c\texamine DWARF constants", /* Check for valid DWARF constants */
"\t\t   d\tshow check results",      /* Show check results */
"\t\t   D\tcheck duplicated attributes",  /* Duplicated attributes */
"\t\t   e\texamine attributes of pubnames",
"\t\t   E\texamine attributes encodings",  /* Attributes encoding */
"\t\t   f\texamine frame information (use with -f or -F)",
"\t\t   F\texamine integrity of files-lines attributes", /* Files-Lines integrity */
"\t\t   g\tcheck debug info gaps", /* Check for debug info gaps */
"\t\t   G\tprint only unique errors", /* Only print the unique errors */
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
#ifdef HAVE_USAGE_TAG_ATTR
"\t\t   u\tprint tag-tree and tag-attribute usage (basic format)",
"\t\t   uf\tprint tag-tree and tag-attribute usage (full format)",
#endif /* HAVE_USAGE_TAG_ATTR */
"\t\t   x\tbasic frames check (.eh_frame, .debug_frame)",
"\t\t   xe\textensive frames check (.eh_frame, .debug_frame)",
"\t\t   y\texamine type info",
"\t\t\tUnless -C option given certain common tag-attr and tag-tag",
"\t\t\textensions are assumed to be ok (not reported).",
"\t\t-l[s]\tprint line section",
"\t\t   s\tdo not print <pc> address",
"\t\t-m\tprint macinfo section",
"\t\t-M\tprint the form name for each attribute",
"\t\t-n\tsuppress frame information function name lookup",
"\t\t  \t(when printing frame information from multi-gigabyte",
"\t\t  \tobject files this option may save significant time).",
"\t\t-N\tprint ranges section",
"\t\t-O file=<path>\tname the output file",
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
"\t\t-S[v] <option>=<text>\tsearch for <text> in attributes",
"\t\t  \tv\tprint number of occurrences",
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
"\t\t-x abi=<abi>\tname abi in dwarfdump.conf",
"\t\t-x groupnumber=<n>\tgroupnumber to print",
"\t\t-x name=<path>\tname dwarfdump.conf",
"\t\t-x tied=<tiedpath>\tname an associated object file (Split DWARF)",
#if 0
"\t\t-x nosanitizestrings\tLet bogus string characters come thru printf",
#endif
"\t\t-w\tprint weakname section",
"\t\t-W\tprint parent and children tree (wide format) with the -S option",
"\t\t-Wp\tprint parent tree (wide format) with the -S option",
"\t\t-Wc\tprint children tree (wide format) with the -S option",
"\t\t-y\tprint type section",
"",
0
};

/* Generic constants for debugging */
#define DUMP_RANGES_INFO            1   /* Dump RangesInfo Table. */
#define DUMP_LOCATION_SECTION_INFO  2   /* Dump Location (.debug_loc) Info. */
#define DUMP_RANGES_SECTION_INFO    3   /* Dump Ranges (.debug_ranges) Info. */
#define DUMP_LINKONCE_INFO          4   /* Dump Linkonce Table. */
#define DUMP_VISITED_INFO           5   /* Dump Visited Info. */

static const char *usage_debug_text[] = {
"Usage: DwarfDump <debug_options>",
"options:\t-0\tprint this information",
"\t\t-1\tDump RangesInfo Table",
"\t\t-2\tDump Location (.debug_loc) Info",
"\t\t-3\tDump Ranges (.debug_ranges) Info",
"\t\t-4\tDump Linkonce Table",
"\t\t-5\tDump Visited Info",
""
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

/*  By trimming a /dwarfdump.O
    down to /dwarfdump  (keeping any prefix
    or suffix)
    we can avoid a sed command in
    regressiontests/DWARFTEST.sh
    and save 12 minutes run time of a regression
    test.

    The effect is, when nothing has changed in the
    normal output, that the program_name matches too.
    Because we don't want a different name of dwarfdump
    to cause a mismatch.  */
static char *
special_program_name(char *n)
{
    char * mp = "/dwarfdump.O";
    char * revstr = "/dwarfdump";
    char *cp = n;
    size_t mslen = strlen(mp);

    for(  ; *cp; ++cp ) {
        if (*cp == *mp) {
            if(!strncmp(cp,mp,mslen)){
                esb_append(&newprogname,revstr);
                cp += mslen-1;
            } else {
                esb_appendn(&newprogname,cp,1);
            }
        } else {
            esb_appendn(&newprogname,cp,1);
        }
    }
    return esb_get_string(&newprogname);
}

/* process arguments and return object filename */
static const char *
process_args(int argc, char *argv[])
{
    extern int dwoptind;
    int c = 0;
    boolean usage_error = FALSE;
    int oarg = 0;

    program_name = special_program_name(argv[0]);
    suppress_check_dwarf();
    if (argv[1] != NULL && argv[1][0] != '-') {
        do_all();
    }
    glflags.gf_section_groups_flag = TRUE;

    /* j unused */
    while ((c = dwgetopt(argc, argv,
        "#:abc::CdDeE::fFgGhH:iIk:l::mMnNo::O:pPqQrRsS:t:u:UvVwW::x:yz")) != EOF) {

        switch (c) {
        /* Internal debug level setting. */
        case '#':
        {
            int nTraceLevel =  atoi(dwoptarg);
            if (nTraceLevel >= 0 && nTraceLevel <= MAX_TRACE_LEVEL) {
                nTrace[nTraceLevel] = 1;
            }
            /* Display dwarfdump debug options. */
            if (dump_options) {
                print_usage_message(program_name,usage_debug_text);
                exit(OKAY);
            }
            break;
        }
        case 'M':
            show_form_used =  TRUE;
            break;
        case 'x':               /* Select which -x option, get value. */
            {
                const char *path = 0;
                const char *abi = 0;
                /*  -x name=<path> meaning name dwarfdump.conf file -x
                    abi=<abi> meaning select abi from dwarfdump.conf
                    file. Must always select abi to use dwarfdump.conf */
                if (strncmp(dwoptarg, "name=", 5) == 0) {
                    path = do_uri_translation(&dwoptarg[5],"-x name=");
                    if (strlen(path) < 1) {
                        goto badopt;
                    }
                    esb_empty_string(&config_file_path);
                    esb_append(&config_file_path,path);
                } else if (strncmp(dwoptarg, "abi=", 4) == 0) {
                    abi = do_uri_translation(&dwoptarg[4],"-x abi=");
                    if (strlen(abi) < 1) {
                        goto badopt;
                    }
                    config_file_abi = abi;
                    break;
                } else if (strncmp(dwoptarg, "groupnumber=", 12) == 0) {
                    /*  By default prints the lowest
                        groupnumber in the object.
                        Default is  -x groupnumber=0
                        For group 1 (standard base dwarfdata)
                            -x groupnumber=1
                        For group 1 (DWARF5 .dwo sections and dwp data)
                            -x groupnumber=2 */
                        long int gnum = 0;
                        int res = 0;

                        res = get_number_value(dwoptarg+12,&gnum);
                        if (res == DW_DLV_OK) {
                            group_number = gnum;
                        } else {
                            goto badopt;
                        }
                } else if (strncmp(dwoptarg, "tied=", 5) == 0) {
                    const char *tiedpath = 0;
                    tiedpath = do_uri_translation(&dwoptarg[5],"-x tied=");
                    if (strlen(tiedpath) < 1) {
                        goto badopt;
                    }
                    esb_empty_string(&config_file_tiedpath);
                    esb_append(&config_file_tiedpath,tiedpath);
                    break;
                } else if (strncmp(dwoptarg, "line5=", 6) == 0) {
                    if (strlen(dwoptarg) < 6) {
                        goto badopt;
                    }
                    if (!strcmp(&dwoptarg[6],"std")) {
                        glflags.gf_line_flag_selection =  singledw5;
                    } else if (!strcmp(&dwoptarg[6],"s2l")){
                        glflags.gf_line_flag_selection= s2l;
                    } else if (!strcmp(&dwoptarg[6],"orig")){
                        glflags.gf_line_flag_selection= orig;
                    } else if (!strcmp(&dwoptarg[6],"orig2l")) {
                        glflags.gf_line_flag_selection= orig2l;
                    } else {
                        goto badopt;
                    }
                    break;
                } else if (strcmp(dwoptarg, "nosanitizestrings") == 0) {
                    no_sanitize_string_garbage = TRUE;
                    break;
                } else if (strcmp(dwoptarg,"noprintsectiongroups") == 0){
                    glflags.gf_section_groups_flag = FALSE;
                    break;
                } else {
                badopt:
                    fprintf(stderr, "-x name=<path-to-conf> \n");
                    fprintf(stderr, " and  \n");
                    fprintf(stderr, "-x abi=<abi-in-conf> \n");
                    fprintf(stderr, " and  \n");
                    fprintf(stderr, "-x tied=<tied-file-path> \n");
                    fprintf(stderr, " and  \n");
                    fprintf(stderr, "-x line5={std,s2l,orig,orig2l} \n");
                    fprintf(stderr, " and  \n");
                    fprintf(stderr, "-x nosanitizestrings \n");
                    fprintf(stderr, "are legal, not -x %s\n", dwoptarg);
                    usage_error = TRUE;
                    break;
                }
            }
            break;
        case 'C':
            glflags.gf_suppress_check_extensions_tables = TRUE;
            break;
        case 'g':
            glflags.gf_use_old_dwarf_loclist = TRUE;
            /*info_flag = TRUE;  removed  from -g. Nov 2015 */
            suppress_check_dwarf();
            break;
        case 'i':
            glflags.gf_info_flag = TRUE;
            glflags.gf_types_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'I':
            glflags.gf_gdbindex_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'n':
            glflags.gf_suppress_nested_name_search = TRUE;
            break;
        case 'l':
            glflags.gf_line_flag = TRUE;
            suppress_check_dwarf();
            /* Enable to suppress offsets printing */
            if (dwoptarg) {
                switch (dwoptarg[0]) {
                /* -ls : suppress <pc> addresses */
                case 's': glflags.gf_line_print_pc = FALSE; break;
                default: usage_error = TRUE; break;
                }
            }
            break;
        case 'f':
            glflags.gf_frame_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'H':
            {
                int break_val =  atoi(dwoptarg);
                if (break_val > 0) {
                    break_after_n_units = break_val;
                }
            }
            break;
        case 'F':
            glflags.gf_eh_frame_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'b':
            glflags.gf_abbrev_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'p':
            glflags.gf_pubnames_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'P':
            /* List of CUs per compiler */
            glflags.gf_producer_children_flag = TRUE;
            break;
        case 'r':
            glflags.gf_aranges_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'N':
            glflags.gf_ranges_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'R':
            glflags.gf_generic_1200_regs = TRUE;
            break;
        case 'm':
            glflags.gf_macinfo_flag = TRUE; /* DWARF2,3,4 */
            glflags.gf_macro_flag   = TRUE; /* DWARF5 */
            suppress_check_dwarf();
            break;
        case 'c':
            /* Specify compiler name. */
            if (dwoptarg) {
                if ('s' == dwoptarg[0]) {
                    /* -cs : Check SNC compiler */
                    glflags.gf_check_snc_compiler = TRUE;
                    glflags.gf_check_all_compilers = FALSE;
                }
                else {
                    if ('g' == dwoptarg[0]) {
                        /* -cg : Check GCC compiler */
                        glflags.gf_check_gcc_compiler = TRUE;
                        glflags.gf_check_all_compilers = FALSE;
                    }
                    else {
                        /*  Assume a compiler version to check,
                            most likely a substring of a compiler name.  */
                        if ((compilers_targeted_count+1) < COMPILER_TABLE_MAX) {
                            Compiler *pCompiler = 0;
                            const char *cmp = 0;
                            cmp = do_uri_translation(dwoptarg,"-c<compiler name>");
                            /* First compiler at position [1] */
                            compilers_targeted_count++;
                            pCompiler = &compilers_targeted[compilers_targeted_count];
                            reset_compiler_entry(pCompiler);
                            pCompiler->name = cmp;
                            glflags.gf_check_all_compilers = FALSE;
                        } else {
                            fprintf(stderr, "Compiler table max %d exceeded, "
                                "limiting the tracked compilers to %d\n",
                                COMPILER_TABLE_MAX,COMPILER_TABLE_MAX);
                        }
                    }
                }
            } else {
                glflags.gf_loc_flag = TRUE;
                suppress_check_dwarf();
            }
            break;
        case 'Q':
            /* Q suppresses section data printing. */
            glflags.gf_do_print_dwarf = FALSE;
            break;
        case 'q':
            /* Suppress uri-did-transate notification */
            glflags.gf_do_print_uri_in_input = FALSE;
            break;
        case 's':
            glflags.gf_string_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'S':
            /* -S option: strings for 'any' and 'match' */
            {
                const char *tempstr = 0;
                boolean serr = TRUE;
                glflags.gf_search_is_on = TRUE;
                /* 'v' option, to print number of occurrences */
                /* -S[v]match|any|regex=text*/
                if (dwoptarg[0] == 'v') {
                    ++dwoptarg;
                    glflags.gf_search_print_results = TRUE;
                }
                /* -S match=<text>*/
                if (strncmp(dwoptarg,"match=",6) == 0) {
                    search_match_text = makename(&dwoptarg[6]);
                    tempstr = remove_quotes_pair(search_match_text);
                    search_match_text = do_uri_translation(tempstr,"-S match=");
                    if (strlen(search_match_text) > 0) {
                        serr = FALSE;
                    }
                }
                else {
                    /* -S any=<text>*/
                    if (strncmp(dwoptarg,"any=",4) == 0) {
                        search_any_text = makename(&dwoptarg[4]);
                        tempstr = remove_quotes_pair(search_any_text);
                        search_any_text = do_uri_translation(tempstr,"-S any=");
                        if (strlen(search_any_text) > 0) {
                            serr = FALSE;
                        }
                    }
#ifdef HAVE_REGEX
                    else {
                        /* -S regex=<regular expression>*/
                        if (strncmp(dwoptarg,"regex=",6) == 0) {
                            search_regex_text = makename(&dwoptarg[6]);
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
                                    serr = FALSE;
                                }
                            }
                        }
                    }
#endif /* HAVE_REGEX */
                }
                if (serr) {
                    fprintf(stderr,"-S any=<text> or -S match=<text> or -S regex=<text>\n");
                    fprintf(stderr, "is allowed, not -S %s\n",dwoptarg);
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
            glflags.gf_do_print_dwarf = TRUE;
            /*  This is sort of useless unless printing,
                but harmless, so we do not insist we
                are printing with suppress_check_dwarf(). */
            dense = TRUE;
            break;
        case 'D':
            /* Do not emit offset in output */
            glflags.gf_display_offsets = FALSE;
            break;
        case 'e':
            suppress_check_dwarf();
            ellipsis = TRUE;
            break;
        case 'E':
            /* Object Header information (but maybe really print) */
            glflags.gf_header_flag = TRUE;
            /* Selected printing of section info */
            if (dwoptarg) {
                switch (dwoptarg[0]) {
                case 'h': section_map[DW_HDR_HEADER] = TRUE; break;
                case 'i': section_map[DW_HDR_DEBUG_INFO] = TRUE;
                    section_map[DW_HDR_DEBUG_TYPES] = TRUE;
                    break;
                case 'l': section_map[DW_HDR_DEBUG_LINE]=TRUE;
                    break;
                case 'p': section_map[DW_HDR_DEBUG_PUBNAMES]=TRUE;
                    break;
                case 'a': section_map[DW_HDR_DEBUG_ABBREV]=TRUE;
                    break;
                case 'r': section_map[DW_HDR_DEBUG_ARANGES]=TRUE;
                    break;
                case 'f': section_map[DW_HDR_DEBUG_FRAME]=TRUE;
                    break;
                case 'o': section_map[DW_HDR_DEBUG_LOC]=TRUE;
                    break;
                case 'R': section_map[DW_HDR_DEBUG_RANGES]=TRUE;
                    break;
                    section_map[DW_HDR_DEBUG_RNGLISTS]=TRUE;
                    break;
                case 's': section_map[DW_HDR_DEBUG_STR]=TRUE;
                    break;

                /*  For both old macinfo and dwarf5  macro */
                case 'm': section_map[DW_HDR_DEBUG_MACINFO]=TRUE;
                    break;

                case 't': section_map[DW_HDR_DEBUG_PUBTYPES]=TRUE;
                    break;
                case 'x': section_map[DW_HDR_TEXT]=TRUE;
                    break;

                case 'I': section_map[DW_HDR_GDB_INDEX]=TRUE;
                    section_map[DW_HDR_DEBUG_CU_INDEX]=TRUE;
                    section_map[DW_HDR_DEBUG_TU_INDEX]=TRUE;
                    section_map[DW_HDR_DEBUG_NAMES]=TRUE;
                    break;

                /* case 'd', use the default section set */
                case 'd': set_all_section_defaults(section_map); break;
                    break;
                default: usage_error = TRUE; break;
                }
            } else {
                /* Display header and all sections info */
                set_all_sections_on(section_map);
            }
            break;
        case 'o':
            glflags.gf_reloc_flag = TRUE;
            if (dwoptarg) {
                switch (dwoptarg[0]) {
                case 'i':
                    reloc_map[DW_SECTION_REL_DEBUG_INFO]=TRUE;
                    reloc_map[DW_SECTION_REL_DEBUG_TYPES]=TRUE;
                    break;
                case 'l':
                    reloc_map[DW_SECTION_REL_DEBUG_LINE]=TRUE;
                    break;
                case 'p':
                    reloc_map[DW_SECTION_REL_DEBUG_PUBNAMES]=TRUE;
                    break;
                /*  Case a has no effect, no relocations can point out
                    of the abbrev section. */
                case 'a':
                    reloc_map[DW_SECTION_REL_DEBUG_ABBREV]=TRUE;
                    break;
                case 'r':
                    reloc_map[DW_SECTION_REL_DEBUG_ARANGES]=TRUE;
                    break;
                case 'f':
                    reloc_map[DW_SECTION_REL_DEBUG_FRAME]=TRUE;
                    break;
                case 'o':
                    reloc_map[DW_SECTION_REL_DEBUG_LOC]=TRUE;
                    reloc_map[DW_SECTION_REL_DEBUG_LOCLISTS]=TRUE;
                    break;
                case 'R':
                    reloc_map[DW_SECTION_REL_DEBUG_RANGES]=TRUE;
                    reloc_map[DW_SECTION_REL_DEBUG_RNGLISTS]=TRUE;
                    break;
                default: usage_error = TRUE; break;
                }
            } else {
                set_all_reloc_sections_on(reloc_map);
            }
            break;
        /* Output filename */
        case 'O':
            {
                const char *path = 0;
                /*  -O name=<filename> */
                usage_error = TRUE;
                if (strncmp(dwoptarg,"file=",5) == 0) {
                    path = do_uri_translation(&dwoptarg[5],"-O file=");
                    if (strlen(path) > 0) {
                        usage_error = FALSE;
                        output_file = path;
                    }
                }
            }
            break;
        case 'k':
            suppress_print_dwarf();
            oarg = dwoptarg[0];
            switch (oarg) {
            case 'a':
                glflags.gf_check_pubname_attr = TRUE;
                glflags.gf_check_attr_tag = TRUE;
                glflags.gf_check_tag_tree = TRUE;
                glflags.gf_check_type_offset = TRUE;
                glflags.gf_check_names = TRUE;
                glflags.gf_pubnames_flag = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                glflags.gf_gdbindex_flag = TRUE;
                glflags.gf_check_decl_file = TRUE;
                glflags.gf_check_macros = TRUE;
                glflags.gf_check_frames = TRUE;
                glflags.gf_check_frames_extended = FALSE;
                glflags.gf_check_locations = TRUE;
                glflags.gf_frame_flag = TRUE;
                glflags.gf_eh_frame_flag = TRUE;
                glflags.gf_check_ranges = TRUE;
                glflags.gf_check_lines = TRUE;
                glflags.gf_check_fdes = TRUE;
                glflags.gf_check_harmless = TRUE;
                glflags.gf_check_aranges = TRUE;
                glflags.gf_aranges_flag = TRUE;  /* Aranges section */
                glflags.gf_check_abbreviations = TRUE;
                glflags.gf_check_dwarf_constants = TRUE;
                glflags.gf_check_di_gaps = TRUE;
                glflags.gf_check_forward_decl = TRUE;
                glflags.gf_check_self_references = TRUE;
                glflags.gf_check_attr_encoding = TRUE;
                glflags.gf_print_usage_tag_attr = TRUE;
                glflags.gf_check_duplicated_attributes = TRUE;
                break;
            /* Abbreviations */
            case 'b':
                glflags.gf_check_abbreviations = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                break;
            /* DWARF constants */
            case 'c':
                glflags.gf_check_dwarf_constants = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                break;
            /* Display check results */
            case 'd':
                glflags.gf_check_show_results = TRUE;
                break;
            /* Check duplicated attributes */
            case 'D':
                glflags.gf_check_duplicated_attributes = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                break;
            case 'e':
                glflags.gf_check_pubname_attr = TRUE;
                glflags.gf_pubnames_flag = TRUE;
                glflags.gf_check_harmless = TRUE;
                glflags.gf_check_fdes = TRUE;
                break;
            /* Attributes encoding usage */
            case 'E':
                glflags.gf_check_attr_encoding = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                break;
            case 'f':
                glflags.gf_check_harmless = TRUE;
                glflags.gf_check_fdes = TRUE;
                break;
            /* files-lines */
            case 'F':
                glflags.gf_check_decl_file = TRUE;
                glflags.gf_check_lines = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                break;
            /* Check debug info gaps */
            case 'g':
                glflags.gf_check_di_gaps = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                break;
            /* Print just global (unique) errors */
            case 'G':
                glflags.gf_print_unique_errors = TRUE;
                break;
            /* Locations list */
            case 'l':
                glflags.gf_check_locations = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                glflags.gf_loc_flag = TRUE;
                break;
            /* Ranges */
            case 'm':
                glflags.gf_check_ranges = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                break;
            /* Aranges */
            case 'M':
                glflags.gf_check_aranges = TRUE;
                glflags.gf_aranges_flag = TRUE;
                break;
            /* invalid names */
            case 'n':
                glflags.gf_check_names = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                break;
            case 'r':
                glflags.gf_check_attr_tag = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                glflags.gf_check_harmless = TRUE;
                break;
            /* forward declarations in DW_AT_specification */
            case 'R':
                glflags.gf_check_forward_decl = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                break;
            /* Check verbose mode */
            case 's':
                glflags.gf_check_verbose_mode = FALSE;
                break;
            /*  self references in:
                DW_AT_specification, DW_AT_type, DW_AT_abstract_origin */
            case 'S':
                glflags.gf_check_self_references = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                break;
            case 't':
                glflags.gf_check_tag_tree = TRUE;
                glflags.gf_check_harmless = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                break;
#ifdef HAVE_USAGE_TAG_ATTR
            /* Tag-Tree and Tag-Attr usage */
            case 'u':
                glflags.gf_print_usage_tag_attr = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_types_flag = TRUE;
                if (dwoptarg[1]) {
                    if ('f' == dwoptarg[1]) {
                        /* -kuf : Full report */
                        glflags.gf_print_usage_tag_attr_full = TRUE;
                    } else {
                        usage_error = TRUE;
                    }
                }
                break;
#endif /* HAVE_USAGE_TAG_ATTR */
            case 'w':
                glflags.gf_check_macros = TRUE;
                glflags.gf_macro_flag = TRUE;
                glflags.gf_macinfo_flag = TRUE;
                break;
            case 'y':
                glflags.gf_check_type_offset = TRUE;
                glflags.gf_check_harmless = TRUE;
                glflags.gf_check_decl_file = TRUE;
                glflags.gf_info_flag = TRUE;
                glflags.gf_pubtypes_flag = TRUE;
                glflags.gf_check_ranges = TRUE;
                glflags.gf_check_aranges = TRUE;
                break;
            /* Summary for each compiler */
            case 'i':
                glflags.gf_print_summary_all = TRUE;
                break;
            /* Frames check */
            case 'x':
                glflags.gf_check_frames = TRUE;
                glflags.gf_frame_flag = TRUE;
                glflags.gf_eh_frame_flag = TRUE;
                if (dwoptarg[1]) {
                    if ('e' == dwoptarg[1]) {
                        /* -xe : Extended frames check */
                        glflags.gf_check_frames = FALSE;
                        glflags.gf_check_frames_extended = TRUE;
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
            glflags.gf_cu_name_flag = TRUE;
            tstr = do_uri_translation(dwoptarg,"-u<cu name>");
            esb_append(&cu_name,tstr);
            }
            break;
        case 'U':
            glflags.gf_uri_options_translation = FALSE;
            break;
        case 't':
            oarg = dwoptarg[0];
            switch (oarg) {
            case 'a':
                /* all */
                glflags.gf_static_func_flag =  TRUE;
                glflags.gf_static_var_flag = TRUE;
                suppress_check_dwarf();
                break;
            case 'f':
                /* .debug_static_func */
                glflags.gf_static_func_flag = TRUE;
                suppress_check_dwarf();
                break;
            case 'v':
                /* .debug_static_var */
                glflags.gf_static_var_flag = TRUE;
                suppress_check_dwarf();
                break;
            default:
                usage_error = TRUE;
                break;
            }
            break;
        case 'y':               /* .debug_pubtypes */
            /* Also for SGI-only, and obsolete, .debug_typenames */
            suppress_check_dwarf();
            glflags.gf_pubtypes_flag = TRUE;
            break;
        case 'w':               /* .debug_weaknames */
            glflags.gf_weakname_flag = TRUE;
            suppress_check_dwarf();
            break;
        case 'z':
            fprintf(stderr, "-z is no longer supported:ignored\n");
            break;
        case 'G':
            glflags.gf_show_global_offsets = TRUE;
            break;
        case 'W':
            /* Search results in wide format */
            glflags.gf_search_wide_format = TRUE;
            if (dwoptarg) {
                if ('c' == dwoptarg[0]) {
                    /* -Wc : Display children tree */
                    glflags.gf_display_children_tree = TRUE;
                } else {
                    if ('p' == dwoptarg[0]) {
                        /* -Wp : Display parent tree */
                        glflags.gf_display_parent_tree = TRUE;
                    } else {
                        usage_error = TRUE;
                    }
                }
            }
            else {
                /* -W : Display parent and children tree */
                glflags.gf_display_children_tree = TRUE;
                glflags.gf_display_parent_tree = TRUE;
            }
            break;
        default:
            usage_error = TRUE;
            break;
        }
    }

    init_conf_file_data(&g_config_file_data);
    if (config_file_abi && glflags.gf_generic_1200_regs) {
        printf("Specifying both -R and -x abi= is not allowed. Use one "
            "or the other.  -x abi= ignored.\n");
        config_file_abi = FALSE;
    }
    if (glflags.gf_generic_1200_regs) {
        init_generic_config_1200_regs(&g_config_file_data);
    }
    if (config_file_abi &&
        (glflags.gf_frame_flag || glflags.gf_eh_frame_flag)) {
        int res = find_conf_file_and_read_config(
            esb_get_string(&config_file_path),
            config_file_abi,
            config_file_defaults,
            &g_config_file_data);

        if (res > 0) {
            printf
                ("Frame not configured due to error(s). Giving up.\n");
            glflags.gf_eh_frame_flag = FALSE;
            glflags.gf_frame_flag = FALSE;
        }
    }
    if (usage_error || (dwoptind != (argc - 1))) {
        print_usage_message(program_name,usage_text);
        exit(FAILED);
    }
    /*  FIXME: it seems silly to be printing section names
        where the section does not exist in the object file.
        However we continue the long-standard practice
        of printing such by default in most cases. For now. */
    if (group_number == DW_GROUPNUMBER_DWO) {
        /*  For split-dwarf/DWO some sections make no sense.
            This prevents printing of meaningless headers where no
            data can exist. */
        glflags.gf_pubnames_flag = FALSE;
        glflags.gf_eh_frame_flag = FALSE;
        glflags.gf_frame_flag    = FALSE;
        glflags.gf_macinfo_flag  = FALSE;
        glflags.gf_aranges_flag  = FALSE;
        glflags.gf_ranges_flag   = FALSE;
        glflags.gf_static_func_flag = FALSE;
        glflags.gf_static_var_flag = FALSE;
        glflags.gf_weakname_flag = FALSE;
    }
    if (group_number > DW_GROUPNUMBER_BASE) {
        /* These no longer apply, no one uses. */
        glflags.gf_static_func_flag = FALSE;
        glflags.gf_static_var_flag = FALSE;
        glflags.gf_weakname_flag = FALSE;
        glflags.gf_pubnames_flag = FALSE;
    }

    if (glflags.gf_do_check_dwarf) {
        /* Reduce verbosity when checking (checking means checking-only). */
        verbose = 1;
    }
    return do_uri_translation(argv[dwoptind],"file-to-process");
}

/* ARGSUSED */
static void
print_error_maybe_continue(UNUSEDARG Dwarf_Debug dbg,
    const char * msg,
    int dwarf_code,
    Dwarf_Error lerr,
    Dwarf_Bool do_continue)
{
    printf("\n");

    if (dwarf_code == DW_DLV_ERROR) {
        char * errmsg = dwarf_errmsg(lerr);

        /*  We now (April 2016) guarantee the
            error number is in
            the error string so we do not need to print
            the dwarf_errno() value to show the number. */
        if (do_continue) {
            printf( "%s ERROR:  %s:  %s. Attempting to continue.\n",
                program_name, msg, errmsg);
        } else {
            printf( "%s ERROR:  %s:  %s\n",
                program_name, msg, errmsg);
        }
    } else if (dwarf_code == DW_DLV_NO_ENTRY) {
        printf("%s NO ENTRY:  %s: \n", program_name, msg);
    } else if (dwarf_code == DW_DLV_OK) {
        printf("%s:  %s \n", program_name, msg);
    } else {
        printf("%s InternalError:  %s:  code %d\n",
            program_name, msg, dwarf_code);
    }

    /* Display compile unit name */
    PRINT_CU_INFO();
}

void
print_error(Dwarf_Debug dbg,
    const char * msg,
    int dwarf_code,
    Dwarf_Error lerr)
{
    print_error_maybe_continue(dbg,msg,dwarf_code,lerr,FALSE);
    if (dbg) {
        Dwarf_Error ignored_err = 0;
        /*  If dbg was never initialized dwarf_finish
            can do nothing useful. There is no
            global-state for libdwarf to clean up. */
        if (dwarf_code == DW_DLV_ERROR) {
            dwarf_dealloc(dbg,lerr,DW_DLA_ERROR);
        }
        dwarf_finish(dbg, &ignored_err);
    }
    exit(FAILED);
}
/* ARGSUSED */
void
print_error_and_continue(Dwarf_Debug dbg,
    const char * msg,
    int dwarf_code,
    Dwarf_Error lerr)
{
    print_error_maybe_continue(dbg,
        msg,dwarf_code,lerr,TRUE);
}

/*  Predicate function. Returns 'true' if the CU should
    be skipped as the DW_AT_name of the CU
    does not match the command-line-supplied
    cu name.  Else returns false.*/
boolean
should_skip_this_cu(Dwarf_Debug dbg, Dwarf_Die cu_die)
{
    Dwarf_Half tag = 0;
    Dwarf_Attribute attrib = 0;
    Dwarf_Half theform = 0;
    int dares = 0;
    int tres = 0;
    int fres = 0;
    Dwarf_Error lerr = 0;

    tres = dwarf_tag(cu_die, &tag, &lerr);
    if (tres != DW_DLV_OK) {
        print_error(dbg, "dwarf_tag in aranges",
            tres, lerr);
    }
    dares = dwarf_attr(cu_die, DW_AT_name, &attrib,
        &lerr);
    if (dares != DW_DLV_OK) {
        print_error(dbg, "dwarf_attr arange"
            " derived die has no name",
            dares, lerr);
        }
    fres = dwarf_whatform(attrib, &theform, &lerr);
    if (fres == DW_DLV_OK) {
        if (theform == DW_FORM_string
            || theform == DW_FORM_strp) {
            char * temps = 0;
            int sres = dwarf_formstring(attrib, &temps,
                &lerr);
            if (sres == DW_DLV_OK) {
                char *lcun = esb_get_string(&cu_name);
                char *p = temps;
                if (lcun[0] != '/') {
                    p = strrchr(temps, '/');
                    if (p == NULL) {
                        p = temps;
                    } else {
                        p++;
                    }
                }
                /* Ignore case if Windows */
#if _WIN32
                if (stricmp(lcun, p)) {
                    /* skip this cu. */
                    return TRUE;
                }
#else
                if (strcmp(lcun, p)) {
                    /* skip this cu. */
                    return TRUE;
                }
#endif /* _WIN32 */

            } else {
                print_error(dbg,
                    "arange: string missing",
                    sres, lerr);
            }
        }
    } else {
        print_error(dbg,
            "dwarf_whatform unexpected value.",
            fres, lerr);
    }
    dwarf_dealloc(dbg, attrib, DW_DLA_ATTR);
    return FALSE;
}

/* Returns the cu of the CU. In case of error, give up, do not return. */
int
get_cu_name(Dwarf_Debug dbg, Dwarf_Die cu_die,
    Dwarf_Off dieprint_cu_offset,
    char * *short_name, char * *long_name)
{
    Dwarf_Attribute name_attr = 0;
    Dwarf_Error lerr = 0;
    int ares;

    ares = dwarf_attr(cu_die, DW_AT_name, &name_attr, &lerr);
    if (ares == DW_DLV_ERROR) {
        print_error(dbg, "hassattr on DW_AT_name", ares, lerr);
    } else {
        if (ares == DW_DLV_NO_ENTRY) {
            *short_name = "<unknown name>";
            *long_name = "<unknown name>";
        } else {
            /* DW_DLV_OK */
            /*  The string return is valid until the next call to this
                function; so if the caller needs to keep the returned
                string, the string must be copied (makename()). */
            char *filename = 0;

            esb_empty_string(&esb_long_cu_name);
            get_attr_value(dbg, DW_TAG_compile_unit,
                cu_die, dieprint_cu_offset,
                name_attr, NULL, 0, &esb_long_cu_name,
                0 /*show_form_used*/,0 /* verbose */);
            *long_name = esb_get_string(&esb_long_cu_name);
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
            esb_empty_string(&esb_short_cu_name);
            esb_append(&esb_short_cu_name,filename);
            *short_name = esb_get_string(&esb_short_cu_name);
        }
    }
    dwarf_dealloc(dbg, name_attr, DW_DLA_ATTR);
    return ares;
}

/*  Returns the producer of the CU
    Caller must ensure producernameout is
    a valid, constructed, empty esb_s instance before calling.
    Never returns DW_DLV_ERROR.  */
int
get_producer_name(Dwarf_Debug dbg, Dwarf_Die cu_die,
    Dwarf_Off dieprint_cu_offset,
    struct esb_s *producernameout)
{
    Dwarf_Attribute producer_attr = 0;
    Dwarf_Error pnerr = 0;

    int ares = dwarf_attr(cu_die, DW_AT_producer,
        &producer_attr, &pnerr);
    if (ares == DW_DLV_ERROR) {
        print_error(dbg, "hassattr on DW_AT_producer", ares, pnerr);
    }
    if (ares == DW_DLV_NO_ENTRY) {
        /*  We add extra quotes so it looks more like
            the names for real producers that get_attr_value
            produces. */
        esb_append(producernameout,"\"<CU-missing-DW_AT_producer>\"");
    } else {
        /*  DW_DLV_OK */
        /*  The string return is valid until the next call to this
            function; so if the caller needs to keep the returned
            string, the string must be copied (makename()). */
        get_attr_value(dbg, DW_TAG_compile_unit,
            cu_die, dieprint_cu_offset,
            producer_attr, NULL, 0, producernameout,
            0 /*show_form_used*/,0 /* verbose */);
    }
    /*  If ares is error or missing case,
        producer_attr will be left
        NULL by the call,
        which is safe when calling dealloc(). */
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
        if (glflags.gf_check_decl_file ||
            glflags.gf_check_ranges ||
            glflags.gf_check_locations) {
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
checking_this_compiler(void)
{
    /*  This flag has been update by 'update_compiler_target()'
        and indicates if the current CU is in a targeted compiler
        specified by the user. Default value is TRUE, which
        means test all compilers until a CU is detected. */
    return current_cu_is_checked_compiler;
}

static int
hasprefix(const char *sample, const char *prefix)
{
    unsigned prelen = strlen(prefix);
    if (strncmp(sample,prefix,prelen) == 0) {
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
        /* Internally the strings do not include quotes */
        boolean snc_compiler = hasprefix(CU_producer,"SN")? TRUE : FALSE;
        boolean gcc_compiler = hasprefix(CU_producer,"GNU")?TRUE : FALSE;
        current_cu_is_checked_compiler = glflags.gf_check_all_compilers ||
            (snc_compiler && glflags.gf_check_snc_compiler) ||
            (gcc_compiler && glflags.gf_check_gcc_compiler) ;
    }

    /* Check for already detected compiler */
    for (index = 1; index <= compilers_detected_count; ++index) {
        if (
#if _WIN32
            !stricmp(compilers_detected[index].name,CU_producer)
#else
            !strcmp(compilers_detected[index].name,CU_producer)
#endif /* _WIN32 */
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
        exit(FAILED);
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
void
reset_overall_CU_error_data(void)
{
   strcpy(CU_name,default_cu_producer);
   strcpy(CU_producer,default_cu_producer);
   DIE_offset = 0;
   DIE_overall_offset = 0;
   DIE_CU_offset = 0;
   DIE_CU_overall_offset = 0;
   CU_base_address = 0;
   CU_high_address = 0;
   CU_low_address = 0;
}


static boolean
cu_data_is_set(void)
{
    if (strcmp(CU_name,default_cu_producer) ||
        strcmp(CU_producer,default_cu_producer)) {
        return 1;
    }
    if (DIE_offset  || DIE_overall_offset) {
        return 1;
    }
    if (CU_base_address || CU_low_address || CU_high_address) {
        return 1;
    }
    return 0;
}

/*  Print CU basic information but
    use the local DIE for the offsets. */
void PRINT_CU_INFO(void)
{
    Dwarf_Unsigned loff = DIE_offset;
    Dwarf_Unsigned goff = DIE_overall_offset;
    char lbuf[50];
    char hbuf[50];

    if (current_section_id == DEBUG_LINE ||
        current_section_id == DEBUG_FRAME ||
        current_section_id == DEBUG_FRAME_EH_GNU ||
        current_section_id == DEBUG_ARANGES ||
        current_section_id == DEBUG_MACRO ||
        current_section_id == DEBUG_MACINFO ) {
        /*  These sections involve the CU die, so
            use the CU offsets.
            The DEBUG_MAC* cases are logical but
            not yet useful (Dec 2015).
            In other cases the local DIE offset makes
            more sense. */
        loff = DIE_CU_offset;
        goff = DIE_CU_overall_offset;
    }
    if (!cu_data_is_set()) {
        return;
    }
    printf("\n");
    printf("CU Name = %s\n",sanitized(CU_name));
    printf("CU Producer = %s\n",sanitized(CU_producer));
    printf("DIE OFF = 0x%" DW_PR_XZEROS DW_PR_DUx
        " GOFF = 0x%" DW_PR_XZEROS DW_PR_DUx,
        loff,goff);
    /* We used to print leftover and incorrect values at times. */
    if ( need_CU_high_address) {
        strcpy(hbuf,"unknown   ");
    } else {
        snprintf(hbuf,sizeof(hbuf),
            "0x%"  DW_PR_XZEROS DW_PR_DUx,CU_high_address);
    }
    if ( need_CU_base_address) {
        strcpy(lbuf,"unknown   ");
    } else {
        snprintf(lbuf,sizeof(lbuf),
            "0x%"  DW_PR_XZEROS DW_PR_DUx,CU_low_address);
    }
#if 0 /* Old format print */
    printf(", Low PC = 0x%08" DW_PR_DUx ", High PC = 0x%08" DW_PR_DUx ,
        CU_low_address,CU_high_address);
#endif
    printf(", Low PC = %s, High PC = %s", lbuf,hbuf);
    printf("\n");
}

void DWARF_CHECK_COUNT(Dwarf_Check_Categories category, int inc)
{
    compilers_detected[0].results[category].checks += inc;
    compilers_detected[0].results[total_check_result].checks += inc;
    if (current_compiler > 0 && current_compiler <  COMPILER_TABLE_MAX) {
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
    if (current_compiler > 0 && current_compiler <  COMPILER_TABLE_MAX) {
        compilers_detected[current_compiler].results[category].errors += inc;
        compilers_detected[current_compiler].results[total_check_result].errors
            += inc;
    }
}

void PRINT_CHECK_RESULT(char *str,
    Compiler *pCompiler, Dwarf_Check_Categories category)
{
    Dwarf_Check_Result result = pCompiler->results[category];
    printf("%-24s%10d  %10d\n", str, result.checks, result.errors);
}

void DWARF_CHECK_ERROR_PRINT_CU()
{
    if (glflags.gf_check_verbose_mode) {
        if (glflags.gf_print_unique_errors) {
            if (!glflags.gf_found_error_message) {
                PRINT_CU_INFO();
            }
        } else {
            PRINT_CU_INFO();
        }
    }
    check_error++;
    glflags.gf_record_dwarf_error = TRUE;
}

/*  Sometimes is useful, just to know the kind of errors in an object file;
    not much interest in the number of errors; the specific case is just to
    have a general idea about the DWARF quality in the file */
char ** set_unique_errors = NULL;
unsigned int set_unique_errors_entries = 0;
unsigned int set_unique_errors_size = 0;
#define SET_UNIQUE_ERRORS_DELTA 64

/*  Create the space to store the unique error messages */
void allocate_unique_errors_table(void)
{
    if (!set_unique_errors) {
        set_unique_errors = (char **)
            malloc(SET_UNIQUE_ERRORS_DELTA * sizeof(char*));
        set_unique_errors_size = SET_UNIQUE_ERRORS_DELTA;
        set_unique_errors_entries = 0;
    }
}

#ifdef TESTING
/* Just for debugging purposes, dump the unique errors table */
void dump_unique_errors_table(void)
{
    unsigned int index;
    printf("*** Unique Errors Table ***\n");
    printf("Delta  : %d\n",SET_UNIQUE_ERRORS_DELTA);
    printf("Size   : %d\n",set_unique_errors_size);
    printf("Entries: %d\n",set_unique_errors_entries);
    for (index = 0; index < set_unique_errors_entries; ++index) {
        printf("%3d: '%s'\n",index,set_unique_errors[index]);
    }
}
#endif

/*  Release the space used to store the unique error messages */
void release_unique_errors_table(void)
{
    unsigned int index;
    for (index = 0; index < set_unique_errors_entries; ++index) {
        free(set_unique_errors[index]);
    }
    free(set_unique_errors);
}

/*  Returns TRUE if the text is already in the set; otherwise FALSE */
boolean add_to_unique_errors_table(char * error_text)
{
    unsigned int index;
    size_t len;
    char * stored_text;
    char * filtered_text;
    char * start = NULL;
    char * end = NULL;
    char * pattern = "0x";
    char * white = " ";
    char * question = "?";

    /* Create a copy of the incoming text */
    filtered_text = makename(error_text);
    len = strlen(filtered_text);

    /*  Remove from the error_text, any hexadecimal numbers (start with 0x),
        because for some errors, an additional information is given in the
        form of addresses; we are interested just in the general error. */
    start = strstr(filtered_text,pattern);
    while (start) {
        /* We have found the start of the pattern; look for a space */
        end = strstr(start,white);
        if (!end) {
            /* Preserve any line terminator */
            end = filtered_text + len -1;
        }
        memset(start,*question,end - start);
        start = strstr(filtered_text,pattern);
    }

    /* Check if the error text is already in the table */
    for (index = 0; index < set_unique_errors_entries; ++index) {
        stored_text = set_unique_errors[index];
        if (!strcmp(stored_text,filtered_text)) {
            return TRUE;
        }
    }

    /* Store the new text; check if we have space to store the error text */
    if (set_unique_errors_entries + 1 == set_unique_errors_size) {
        set_unique_errors_size += SET_UNIQUE_ERRORS_DELTA;
        set_unique_errors = (char **)realloc(set_unique_errors,
            set_unique_errors_size * sizeof(char*));
    }

    set_unique_errors[set_unique_errors_entries] = filtered_text;
    ++set_unique_errors_entries;

    return FALSE;
}

/*  Print a DWARF error message and if in "reduced" output only print one
    error of each kind; this feature is usefull, when we are interested only
    in the kind of errors and not on the number of errors. */
static void
print_dwarf_check_error(char *format,...)
{
    static boolean do_init = TRUE;
    boolean found = FALSE;
    char * error_text = NULL;
    va_list ap;

    if (do_init) {
        esb_constructor(&dwarf_error_line);
        do_init = FALSE;
    }
    esb_empty_string(&dwarf_error_line);

    /*  "The object ap may be passed as an argument to another
        function; if that function invokes the va_arg()
        macro with parameter ap, the value of ap in the calling
        function is unspecified and shall be passed to the va_end()
        macro prior to any further reference to ap."
        Single Unix Specification. */
    /* 'esb_append_printf_ap', does the necessary adjustments to the
        extensible string buffer. */
    va_start(ap,format);
    esb_append_printf_ap(&dwarf_error_line,format,ap);
    va_end(ap);

    error_text = esb_get_string(&dwarf_error_line);
    if (glflags.gf_print_unique_errors) {
        found = add_to_unique_errors_table(error_text);
        if (!found) {
            printf("%s",error_text);
        }
    } else {
        printf("%s",error_text);
    }

    /* To indicate if the current error message have been found or not */
    glflags.gf_found_error_message = found;
}

void DWARF_CHECK_ERROR(Dwarf_Check_Categories category,
    const char *str)
{

    if (checking_this_compiler()) {
        DWARF_ERROR_COUNT(category,1);
        if (glflags.gf_check_verbose_mode) {
            print_dwarf_check_error("\n*** DWARF CHECK: %s ***\n", str);
        }
        DWARF_CHECK_ERROR_PRINT_CU();
    }
}

void DWARF_CHECK_ERROR2(Dwarf_Check_Categories category,
    const char *str1, const char *str2)
{
    if (checking_this_compiler()) {
        DWARF_ERROR_COUNT(category,1);
        if (glflags.gf_check_verbose_mode) {
            print_dwarf_check_error("\n*** DWARF CHECK: %s: %s ***\n",
                str1, str2);
        }
        DWARF_CHECK_ERROR_PRINT_CU();
    }
}

void DWARF_CHECK_ERROR3(Dwarf_Check_Categories category,
    const char *str1, const char *str2, const char *strexpl)
{
    if (checking_this_compiler()) {
        DWARF_ERROR_COUNT(category,1);
        if (glflags.gf_check_verbose_mode) {
            print_dwarf_check_error("\n*** DWARF CHECK: %s -> %s: %s ***\n",
                str1, str2,strexpl);
        }
        DWARF_CHECK_ERROR_PRINT_CU();
    }
}

/*  The strings whose pointers are returned here
    from makename are never destructed, but
    that is ok since there are only about 10 created at most.  */
static const char *
do_uri_translation(const char *s,const char *context)
{
    struct esb_s str;
    char *finalstr = 0;
    if (!glflags.gf_uri_options_translation) {
        return makename(s);
    }
    esb_constructor(&str);
    translate_from_uri(s,&str);
    if (glflags.gf_do_print_uri_in_input) {
        if (strcmp(s,esb_get_string(&str))) {
            printf("Uri Translation on option %s\n",context);
            printf("    \'%s\'\n",s);
            printf("    \'%s\'\n",esb_get_string(&str));
        }
    }
    finalstr = makename(esb_get_string(&str));
    esb_destructor(&str);
    return finalstr;
}
