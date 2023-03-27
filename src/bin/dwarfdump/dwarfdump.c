/*
Copyright (C) 2000,2002,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.
Portions Copyright (C) 2007-2021 David Anderson. All Rights Reserved.
Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
Portions Copyright 2012 SN Systems Ltd. All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of version 2 of the GNU General
  Public License as published by the Free Software Foundation.

  This program is distributed in the hope that it would be
  useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  Further, this software is distributed without any warranty
  that it is free of the rightful claim of any third person
  regarding infringement or the like.  Any license provided
  herein, whether implied or otherwise, applies only to this
  software file.  Patent licenses, if any, provided herein
  do not apply to combinations of this program with other
  software, or any other product whatsoever.

  You should have received a copy of the GNU General Public
  License along with this program; if not, write the Free
  Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
  Boston MA 02110-1301, USA.

*/

/* The address of the Free Software Foundation is
   Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
   Boston, MA 02110-1301, USA.
   SGI has moved from the Crittenden Lane address.
*/

#include <config.h>

#include <stddef.h> /* NULL size_t */
#include <stdio.h>  /* stdout stderr fprintf() printf() sprintf() */
#include <stdlib.h> /* exit() free() malloc() qsort() realloc() */
#include <string.h> /* memset() strcmp() stricmp()
    strlen() strrchr() strstr() */

/* Windows specific header files */
#ifdef _WIN32
#ifdef HAVE_STDAFX_H
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */
#include <io.h> /* close() dup2() */
#elif defined HAVE_UNISTD_H
#include <unistd.h> /* close() dup2() */
#endif /* _WIN32 */

#ifdef HAVE_FCNTL_H
#include <fcntl.h> /* O_RDONLY open() */
#endif /* HAVE_FCNTL_H */

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "dd_globals.h"
#include "dd_makename.h"
#include "dd_macrocheck.h"
#include "dd_dwconf.h"
#include "dd_dwconf_using_functions.h"
#include "dd_common.h"
#include "dd_helpertree.h"
#include "dd_esb.h"                /* For flexible string buffer. */
#include "dd_esb_using_functions.h"
#include "dd_sanitized.h"
#include "dd_tag_common.h"
#include "dd_addrmap.h"
#include "dd_attr_form.h"
#include "print_debug_gnu.h"
#include "dd_naming.h" /* for get_FORM_name() */
#include "dd_command_options.h"
#include "dd_compiler_info.h"
#include "dd_safe_strcpy.h"
#include "dd_minimal.h"

#ifndef O_RDONLY
/*  This is for a Windows environment */
# define O_RDONLY _O_RDONLY
#endif

#ifdef _O_BINARY
/*  This is for a Windows environment */
#define O_BINARY _O_BINARY
#else
# ifndef O_BINARY
# define O_BINARY 0  /* So it does nothing in Linux/Unix */
# endif
#endif /* O_BINARY */

#define BYTES_PER_INSTRUCTION 4

#define LOCAL_PTR_ARY_COUNT 50

/* Build section information */
void build_linkonce_info(Dwarf_Debug dbg);

struct glflags_s glflags;

/* Functions used to manage the unique errors table */
static void allocate_unique_errors_table(void);
static void release_unique_errors_table(void);
#ifdef TESTING
static void dump_unique_errors_table(void);
#endif
static Dwarf_Bool add_to_unique_errors_table(char * error_text);

static struct esb_s esb_short_cu_name;
static struct esb_s esb_long_cu_name;
static struct esb_s dwarf_error_line;
static int global_basefd = -1;
static int global_tiedfd = -1;
static struct esb_s global_file_name;
static struct esb_s global_tied_file_name;

static int process_one_file(
    const char * file_name,
    const char * tied_file_name,
    char *       tempbuf,
    unsigned int tempbuflen,
    struct dwconf_s *conf);

static int print_gnu_debuglink(Dwarf_Debug dbg,Dwarf_Error *err);

static int
open_a_file(const char * name)
{
    /* Set to a file number that cannot be legal. */
    int fd = -1;
    fd = open(name, O_RDONLY | O_BINARY);
    return fd;
}

static void
close_a_file(int f)
{
    if (f != -1) {
        close(f);
    }
}

void
global_destructors(void)
{
    makename_destructor();
    uri_data_destructor();
    esb_destructor(&esb_long_cu_name);
    esb_destructor(&esb_short_cu_name);
    esb_destructor(&dwarf_error_line);
    /*  Global flags initialization and esb-buffers destruction. */
    reset_global_flags();
    esb_destructor(&global_file_name);
    esb_destructor(&global_tied_file_name);
    free_all_dwconf(glflags.config_file_data);
    sanitized_string_destructor();
    ranges_esb_string_destructor();
    close_a_file(global_basefd);
    close_a_file(global_tiedfd);
#ifdef _WIN32
    /* Close the null device used during formatting printing */
    esb_close_null_device();
#endif /* _WIN32 */
    if (glflags.gf_global_debuglink_paths) {
        unsigned int i = 0;

        for ( ; i < glflags.gf_global_debuglink_count ; ++i ) {
            free(glflags.gf_global_debuglink_paths[i]);
            glflags.gf_global_debuglink_paths[i] =  0;
        }
        free(glflags.gf_global_debuglink_paths);
        glflags.gf_global_debuglink_paths = 0;
    }
    glflags.gf_global_debuglink_count = 0;
}

static void
check_for_notes(void)
{
    long int ect = glflags.gf_count_macronotes;
    const char *w = "was";
    const char *e = "MACRONOTE";
    if (!ect) {
        return;
    }
    if (ect > 1) {
        w = "were";
        e = "MACRONOTEs";
    }
    printf("There %s %ld DWARF %s reported: "
        "see MACRONOTE above.\n",
        w, ect,e);
}
static void
check_for_major_errors(void)
{
    long int ect = glflags.gf_count_major_errors;
    const char *w = "was";
    const char *e = "error";
    if (!ect) {
        return;
    }
    if (ect > 1) {
        w = "were";
        e = "errors";
    }
    printf("There %s %ld DWARF %s reported: "
        "see ERROR above.\n",
        w, ect,e);
}

static void
flag_data_pre_allocation(void)
{
    memset(glflags.section_high_offsets_global,0,
        sizeof(*glflags.section_high_offsets_global));
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
        glflags.pRangesInfo = AllocateBucketGroup(KIND_RANGES_INFO);
        glflags.pLinkonceInfo =
            AllocateBucketGroup(KIND_LINKONCE_INFO);
        glflags.pVisitedInfo = AllocateBucketGroup(KIND_VISITED_INFO);
    }
    /* Create the unique error table */
    if (glflags.gf_print_unique_errors) {
        allocate_unique_errors_table();
    }
    /* Allocate range array to be used by all CUs */
    if (glflags.gf_check_ranges) {
        allocate_range_array_info();
    }
}

static void
flag_data_post_cleanup(void)
{
    if (glflags.pRangesInfo) {
        ReleaseBucketGroup(glflags.pRangesInfo);
        glflags.pRangesInfo = 0;
    }
    if (glflags.pLinkonceInfo) {
        ReleaseBucketGroup(glflags.pLinkonceInfo);
        glflags.pLinkonceInfo = 0;
    }
    if (glflags.pVisitedInfo) {
        ReleaseBucketGroup(glflags.pVisitedInfo);
        glflags.pVisitedInfo = 0;
    }
    /* Release range array to be used by all CUs */
    if (glflags.gf_check_ranges) {
        release_range_array_info();
    }
    /* Delete the unique error set */
    if (glflags.gf_print_unique_errors) {
        release_unique_errors_table();
    }
    clean_up_compilers_detected();
    destruct_abbrev_array();
}

void _dwarf_alloc_tree_counts(Dwarf_Unsigned *allocount,
    Dwarf_Unsigned *allosum,
    Dwarf_Unsigned *treecount,
    Dwarf_Unsigned *treesum,
    Dwarf_Unsigned *earlydealloccount,
    Dwarf_Unsigned *earlydeallocsize,
    Dwarf_Unsigned *unused1,
    Dwarf_Unsigned *unused2,
    Dwarf_Unsigned *unused3);

/*
   Iterate through dwarf and print all info.
*/
int
main(int argc, char *argv[])
{
    const char     *file_name = 0;
    unsigned        ftype = 0;
    unsigned        endian = 0;
    unsigned        offsetsize = 0;
    Dwarf_Unsigned  filesize = 0;
    int             errcode = 0;
    char           *temp_path_buf = 0;
    size_t          temp_path_buf_len = 0;
    int             res = 0;
    /* path_source will be DW_PATHSOURCE_basic  */
    unsigned char   path_source = DW_PATHSOURCE_unspecified;

#ifdef _WIN32
    /*  Open the null device used during formatting printing */
    if (!esb_open_null_device()) {
        printf("ERROR dwarfdump: Unable to open null device.\n");
        exit(EXIT_FAILURE);
    }
#endif /* _WIN32 */

    /*  Global flags initialization and esb-buffers construction. */
    init_global_flags();

    set_checks_off();
    uri_data_constructor();
    esb_constructor(&esb_short_cu_name);
    esb_constructor(&esb_long_cu_name);
    esb_constructor(&dwarf_error_line);
#ifdef _WIN32
    /*  Often we redirect the output to a file, but we have found
        issues due to the buffering associated with stdout.
        Some issues were fixed just by the use of 'fflush',
        but the main issued remained.
        The stdout stream is buffered, so will only display
        what's in the buffer after it reaches a newline
        (or when it's told to).
        We have a few options to print immediately:
        - Print to stderr instead using fprintf.
        - Print to stdout and flush stdout whenever
            we need it to using fflush.
        - We can also disable buffering on stdout by using setbuf:
            setbuf(stdout,NULL);
            Make stdout unbuffered; this seems to work for all cases.
        The problem is no longer present. Now, for practical
        purposes, there is no stderr output, all is stdout.
        September 2018.  */

    /*  Calling setbuf() with NULL argument, it turns off
        all buffering for the specified stream.
        Then writing to and/or reading from the stream
        will be exactly as directed by the program.
        But if dwarfdump is used over a network drive,
        it shows a dramatic
        slowdown when sending the output to a file.
        An operation that takes
        couple of seconds, it was taking few hours. */
    /*  setbuf(stdout,NULL); */
    /*  Redirect stderr to stdout. */
    /*  No more redirect needed. We only use stdout */
#endif /* _WIN32 */

    file_name = process_args(argc, argv);
    /*  print_version_details already done in
        dd_command_options.c */
    print_args(argc,argv);

    /*  Redirect stdout  to an specific file */
    if (glflags.output_file) {
        if (NULL == freopen(glflags.output_file,"w",stdout)) {
            printf("ERROR dwarfdump: Unable to redirect "
                "output to '%s'\n",
                glflags.output_file);
            global_destructors();
            exit(EXIT_FAILURE);
        }
        /* Record version and arguments in the output file */
        print_version_details(argv[0]);
        print_args(argc,argv);
    }

    /*  Allow the user to hide some warnings by using
        command line options */
    {
        Dwarf_Cmdline_Options wcmd;
        /* The struct has just one field!. */
        wcmd.check_verbose_mode = glflags.gf_check_verbose_mode;
        dwarf_record_cmdline_options(wcmd);
    }
    if (glflags.gf_check_functions) {
        static const Dwarf_Signed stab[] =
            {0,1,-1,100,-100,-10000000,10000000};
        int i = 0;
        int len = sizeof(stab)/sizeof(stab[0]);
        char vbuf[100];

        vbuf[0] = 0;
        DWARF_CHECK_COUNT(check_functions_result,1);
        for (i = 0; i < len; ++i) {
            Dwarf_Signed basevalue = 0;
            Dwarf_Signed decodedvalue = 0;
            Dwarf_Unsigned silen = 0;
            int leblen = 0;

            basevalue = stab[i];
            memset(vbuf,0,sizeof(vbuf));
            res = dwarf_encode_signed_leb128(basevalue,
                &leblen,
                vbuf,(int)sizeof(vbuf));
            if (res == DW_DLV_ERROR) {
                DWARF_CHECK_ERROR(check_functions_result,
                    "Got error encoding Encoding Dwarf_Signed");
                break;
            }
            res = dwarf_decode_signed_leb128(
                vbuf,&silen, &decodedvalue,
                vbuf + sizeof(vbuf));
            if (res == DW_DLV_ERROR) {
                DWARF_CHECK_ERROR(check_functions_result,
                    "Got error encoding Decoding Dwarf_Signed");
                break;
            }
            if ( decodedvalue != basevalue) {
                DWARF_CHECK_ERROR(check_functions_result,
                    "Decode Dwarf_signed does not match"
                    "starting value");
                break;
            }
        }
    }
    /* ======= BEGIN FINDING NAMES AND OPENING FDs ===== */
    /*  The 200+2 etc is more than suffices for the expansion that a
        MacOS dsym or a GNU debuglink might need, we hope. */
    temp_path_buf_len = strlen(file_name)*3 + 200 + 2;
    temp_path_buf = malloc(temp_path_buf_len);
    if (!temp_path_buf) {
        printf("%s ERROR:  Unable to malloc %lu bytes "
            "for possible path string %s.\n",
            glflags.program_name,(unsigned long)temp_path_buf_len,
            file_name);
        global_destructors();
        exit(EXIT_FAILURE);
    }
    temp_path_buf[0] = 0;
    /*  This data scan is to find Elf objects and
        unknown objects early.  If the user
        asks for certain options
        that will rule out handling GNU_debuglink
        on that object.  This does not concern itself
        with dSYM or debuglink at all. */
    res = dwarf_object_detector_path_b(file_name,
        0,0,
        0,0,
        &ftype,&endian,&offsetsize,&filesize,
        &path_source,&errcode);
    if (res != DW_DLV_OK) {
        if (res == DW_DLV_ERROR) {
            printf("%s ERROR:  Can't open/use %s. Error %s\n",
                glflags.program_name, sanitized(file_name),
                dwarf_errmsg_by_number(errcode));
        } else {
            printf("%s ERROR:  Can't open %s. "
                "There is no valid object file with that name)\n",
                glflags.program_name, sanitized(file_name));
        }
        global_destructors();
        free(temp_path_buf);
        exit(EXIT_FAILURE);
    }
    esb_append(&global_file_name,file_name);
    temp_path_buf[0] = 0;
    global_basefd = open_a_file(esb_get_string(
        &global_file_name));
    if (global_basefd == -1) {
        printf("%s ERROR:  can't open.. %s\n",
            glflags.program_name,
            esb_get_string(&global_file_name));
        global_destructors();
        free(temp_path_buf);
        exit(EXIT_FAILURE);
    }

    if (esb_string_len(glflags.config_file_tiedpath) > 0) {
        unsigned         tftype = 0;
        unsigned         tendian = 0;
        unsigned         toffsetsize = 0;
        Dwarf_Unsigned   tfilesize = 0;
        const char * tied_file_name = 0;
        /* path_source will be DW_PATHSOURCE_basic  */
        unsigned char    tpath_source = 0;

        temp_path_buf[0] = 0;
        tied_file_name = esb_get_string(glflags.config_file_tiedpath);
        /*  A genuine tiedpath cannot be dsym or debuglink. */
        res = dwarf_object_detector_path_b (tied_file_name,
            0,0,
            0,0,
            &tftype,&tendian,&toffsetsize,&tfilesize,
            &tpath_source,&errcode);
        if (res != DW_DLV_OK) {
            if (res == DW_DLV_ERROR) {
                char *errmsg = dwarf_errmsg_by_number(errcode);
                printf("%s ERROR:  can't open tied file"
                    ".. %s: %s\n",
                    glflags.program_name, sanitized(tied_file_name),
                    errmsg);
            } else {
                printf(
                    "%s ERROR: tied file not an object file '%s'.\n",
                    glflags.program_name, sanitized(tied_file_name));
            }
            glflags.gf_count_major_errors++;
            global_destructors();
            free(temp_path_buf);
            exit(EXIT_FAILURE);
        }
        if (ftype != tftype || endian != tendian ||
            offsetsize != toffsetsize) {
            printf("%s ERROR:  tied file \'%s\' and "
                "main file \'%s\' not "
                "the same kind of object!\n",
                glflags.program_name,
                sanitized(tied_file_name),
                esb_get_string(&global_file_name));
            free(temp_path_buf);
            global_destructors();
            glflags.gf_count_major_errors++;
            exit(EXIT_FAILURE);
        }
        esb_append(&global_tied_file_name,tied_file_name);
        global_tiedfd = open_a_file(esb_get_string(
            &global_tied_file_name));
        if (global_tiedfd == -1) {
            printf("%s ERROR:  can't open tied file"
                "... %s\n",
                glflags.program_name,
                sanitized(esb_get_string(&global_tied_file_name)));
            global_destructors();
            glflags.gf_count_major_errors++;
            free(temp_path_buf);
            exit(EXIT_FAILURE);
        }
    }
    /* ======= end FINDING NAMES AND OPENING FDs ===== */
    temp_path_buf[0] = 0;
    /* ======= BEGIN PROCESSING OBJECT FILES BY TYPE ===== */
    if ((ftype == DW_FTYPE_ELF && (glflags.gf_reloc_flag ||
        glflags.gf_header_flag)) ||
        ftype == DW_FTYPE_ARCHIVE) {
        printf("ERROR Can't process %s: archives and "
            "printing elf headers not supported in this dwarfdump "
            "--disable-libelf build.\n",
            file_name);
        glflags.gf_count_major_errors++;
    } else if (ftype == DW_FTYPE_ELF ||
        ftype ==  DW_FTYPE_MACH_O  ||
        ftype == DW_FTYPE_PE  ) {
        flag_data_pre_allocation();
        close_a_file(global_basefd);
        global_basefd = -1;
        close_a_file(global_tiedfd);
        global_tiedfd = -1;
        process_one_file(
            esb_get_string(&global_file_name),
            esb_get_string(&global_tied_file_name),
            temp_path_buf, temp_path_buf_len,
            glflags.config_file_data);
        flag_data_post_cleanup();
    } else {
        printf("ERROR Can't process %s: unhandled format\n",
            file_name);
        glflags.gf_count_major_errors++;
    }
    free(temp_path_buf);
    temp_path_buf = 0;
    temp_path_buf_len = 0;
    /* ======= END PROCESSING OBJECT FILES BY TYPE ===== */

    /*  These cleanups only necessary once all
        objects processed. */
    /*  In case of a serious DWARF error
        we  try to get here, we try not
        to  exit(1) by using print_error() */
    check_for_major_errors();
    check_for_notes();
    flag_data_post_cleanup();
    global_destructors();
    free(temp_path_buf);
    /*  As the tool have reached this point, it means there are
        no internal errors and we should return an OKAY condition,
        regardless if the file being processed has
        minor errors. */
    exit(0);
}

void
print_any_harmless_errors(Dwarf_Debug dbg)
{
    /*  We do not need to initialize the local array,
        libdwarf does it. */
    const char *buf[LOCAL_PTR_ARY_COUNT];
    unsigned totalcount = 0;
    unsigned i = 0;
    unsigned printcount = 0;
    int res = dwarf_get_harmless_error_list(dbg,
        LOCAL_PTR_ARY_COUNT,buf,
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

/* Print a summary of search results */
static void
print_search_results(void)
{
    const char *search_type = 0;
    const char *search_text = 0;
    if (glflags.search_any_text) {
        search_type = "any";
        search_text = glflags.search_any_text;
    } else {
        if (glflags.search_match_text) {
            search_type = "match";
            search_text = glflags.search_match_text;
        } else {
            search_type = "regex";
            search_text = glflags.search_regex_text;
        }
    }
    fflush(stdout);
    printf("\nSearch type      : '%s'\n",search_type);
    printf("Pattern searched : '%s'\n",search_text);
    printf("Occurrences Found: %d\n",glflags.search_occurrences);
    fflush(stdout);
}

/*  This is for dwarf_print_lines(), we are not using
    the userdata because dwarfdump does not need it. */
static void
printf_callback_for_libdwarf(void *userdata,
    const char *data)
{
    (void)userdata;
    printf("%s",sanitized(data));
}

int
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
        return dres;
    }
    if (max) {
        *max = (lsize == 8 ) ? 0xffffffffffffffffULL : 0xffffffff;
    }
    if (size) {
        *size = lsize;
    }
    return DW_DLV_OK;
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
    dwarf_get_section_max_offsets_d(dbg,
        &glflags.section_high_offsets_global->debug_info_size,
        &glflags.section_high_offsets_global->debug_abbrev_size,
        &glflags.section_high_offsets_global->debug_line_size,
        &glflags.section_high_offsets_global->debug_loc_size,
        &glflags.section_high_offsets_global->debug_aranges_size,
        &glflags.section_high_offsets_global->debug_macinfo_size,
        &glflags.section_high_offsets_global->debug_pubnames_size,
        &glflags.section_high_offsets_global->debug_str_size,
        &glflags.section_high_offsets_global->debug_frame_size,
        &glflags.section_high_offsets_global->debug_ranges_size,
        &glflags.section_high_offsets_global->debug_pubtypes_size,
        &glflags.section_high_offsets_global->debug_types_size,
        &glflags.section_high_offsets_global->debug_macro_size,
        &glflags.section_high_offsets_global->debug_str_offsets_size,
        &glflags.section_high_offsets_global->debug_sup_size,
        &glflags.section_high_offsets_global->debug_cu_index_size,
        &glflags.section_high_offsets_global->debug_tu_index_size,
        &glflags.section_high_offsets_global->debug_names_size,
        &glflags.section_high_offsets_global->debug_loclists_size,
        &glflags.section_high_offsets_global->debug_rnglists_size);

}

/*  Set limits for Ranges Information.
    The linker may
    put parts of the text(code) in additional sections
    such as .init .fini __libc_freeres_fn
    .rodata __libc_subfreeres __libc_atexit too. */
#define LIKELYNAMESMAX 3
static const char *likely_ns[LIKELYNAMESMAX] = {
/*  .text is first as it is often the only thing.See below. */
".init",
".text",
".fini"
};
#define ORIGLKLYTEXTINDEX  1
struct likely_names_s {
    const char *   name;
    int            origindex;
    Dwarf_Unsigned low;
    Dwarf_Unsigned size;
    Dwarf_Unsigned end;
};
static struct likely_names_s likely_names[LIKELYNAMESMAX];
#if 0 /* FOR DEBUG ONLY */
static void
printlnrec(const char *msg,struct likely_names_s * ln,
    int line,char * fn)
{
    printf("%s: name %s origindx %d "
        "low 0x%lx "
        "size 0x%lx "
        "end 0x%lx "
        " line  %d %s\n",msg,
        ln->name,ln->origindex,
        (unsigned long)ln->low,
        (unsigned long)ln->size,
        (unsigned long)ln->end,line,fn);
}
#endif /* 0 */

static int
likelycmp(const void *l_in, const void *r_in)
{
    struct likely_names_s *l = (struct likely_names_s *)l_in;
    struct likely_names_s *r = (struct likely_names_s *)r_in;

    if (l->low < r->low) {
        return -1;
    }
    if (l->low > r->low ) {
        return 1;
    }
    if (l->end < r->end) {
        return -1;
    }
    if (l->end > r->end) {
        return 1;
    }
    return 0;
}

/*  This is a bit slow, but happens only once for a dbg.
    It is not as much help as I expected in avoiding
    line table content CHECK warnings because, so far,
    those come from .init csu code and the DWARF has
    no subprogram information nor any high/low pc
    information at all.  */
static int
calculate_likely_limits_of_code(Dwarf_Debug dbg,
    Dwarf_Unsigned *lower,
    Dwarf_Unsigned *size)
{
    struct likely_names_s * ln = 0;
    int ct = 0;
    Dwarf_Unsigned baselow = 0;
    Dwarf_Unsigned basesize = 0;
    Dwarf_Unsigned baseend = 0;
    int lnindex = 0;
    int lncount = 0;

    memset(likely_names,0,sizeof(likely_names));
    for (ct = 0 ; ct < LIKELYNAMESMAX; ct++) {
        Dwarf_Unsigned clow = 0;
        Dwarf_Unsigned csize = 0;
        int res = 0;
        Dwarf_Error err = 0;
        /* Just looks for .text and .init and  .fini for ranges. */
        const char *name = likely_ns[ct];

        ln = likely_names + lnindex;
        res = dwarf_get_section_info_by_name(dbg,name,
            &clow,&csize,&err);
        if (res == DW_DLV_ERROR) {
            dwarf_dealloc_error(dbg,err);
            return res;
        }
        if (res == DW_DLV_NO_ENTRY) {
            continue;
        }
        ln->name = name;
        ln->low = clow;
        ln->size = csize;
        ln->end = csize +clow;
        ln->origindex = ct;
        if (ct == ORIGLKLYTEXTINDEX) {
            basesize = csize;
            baselow  = clow;
        }
        ++lnindex;
    }
    if (!lnindex) {
        return DW_DLV_NO_ENTRY;
    }
    if (lnindex == 1) {
        *lower = baselow;
        *size  = basesize;
        return DW_DLV_OK;
    }
    lncount = lnindex;
    qsort(likely_names,lncount,sizeof(struct likely_names_s),
        likelycmp);
    ln = likely_names;
    baselow =ln->low;
    basesize =ln->size;
    baseend = ln->end;
    for (lnindex = 1; lnindex<lncount; ++lnindex) {
        ln = likely_names+lnindex;
        if (ln->end > baseend) {
            baseend = ln->end;
            basesize = (baseend - baselow);
        }
    }
    *lower = baselow;
    *size  = basesize;
    return DW_DLV_OK;
}
/*  Given a file which is an object type
    we think we can read, process the dwarf data.  */
static int
process_one_file(
    const char * file_name,
    const char * tied_file_name,
    char *       temp_path_buf,
    unsigned int temp_path_buf_len,
    struct dwconf_s *l_config_file_data)
{
    Dwarf_Debug dbg = 0;
    Dwarf_Debug dbgtied = 0;
    int dres = 0;
    struct Dwarf_Printf_Callback_Info_s printfcallbackdata;
    Dwarf_Half elf_address_size = 0;      /* Target pointer size */
    Dwarf_Error onef_err = 0;
    const char *title = 0;
    unsigned char path_source = 0;
    int localerrno = 0;

    /*  If using a tied file group number should be
        2 DW_GROUPNUMBER_DWO
        but in a dwp or separate-split-dwarf object then
        0 DW_GROUPNUMBER_ANY will find the .dwo data
        automatically. */
    {
        /*  This will go for the real main file, whether
            an underlying dSYM or via debuglink or
            if those find nothing then the original. */
        char *tb = temp_path_buf;
        unsigned tblen = temp_path_buf_len;
        title = "dwarf_init_path_dl fails.";
        if (glflags.gf_no_follow_debuglink) {
            tb = 0;
            tblen = 0;
        }
        dres = dwarf_init_path_dl(file_name,
            tb,tblen,
            glflags.group_number,
            NULL, NULL, &dbg,
            glflags.gf_global_debuglink_paths,
            glflags.gf_global_debuglink_count,
            &path_source,
            &onef_err);
    }
    if (dres == DW_DLV_NO_ENTRY) {
        if (glflags.group_number > 0) {
            printf("No DWARF information present in %s "
                "for section group %d \n",
                file_name,glflags.group_number);
        } else {
            printf("No DWARF information present in %s\n",file_name);
        }
        return dres;
    }
    if (dres == DW_DLV_ERROR) {
        /* Prints error, cleans up Dwarf_Error data. */
        print_error_and_continue(
            title,dres,onef_err);
        DROP_ERROR_INSTANCE(dbg,dres,onef_err);
        return DW_DLV_NO_ENTRY;
    }
    if (path_source == DW_PATHSOURCE_dsym) {
        printf("Filename by dSYM is %s\n",
            sanitized(temp_path_buf));
    } else if (path_source == DW_PATHSOURCE_debuglink) {
        printf("Filename by debuglink is %s\n",
            sanitized(temp_path_buf));
        glflags.gf_gnu_debuglink_flag = TRUE;
    } else { /* Nothing to print yet. */ }
    if (tied_file_name && strlen(tied_file_name)) {
        {
            /*  The tied file we define as group 1, BASE.
                Cannot follow debuglink or dSYM,
                is a tied file */
            dres = dwarf_init_path(tied_file_name,
                0,0,  /* ignore dSYM & debuglink */
                DW_GROUPNUMBER_BASE,
                0,0,
                &dbgtied,
                &onef_err);
            /* path_source = DW_PATHSOURCE_basic; */
        }
        if (dres == DW_DLV_NO_ENTRY) {
            printf("No DWARF information present in tied file: %s\n",
                tied_file_name);
            return dres;
        }
        if (dres == DW_DLV_ERROR) {
            /*  Prints error, cleans up Dwarf_Error data.
                Never returns*/
            print_error(dbg,
                "dwarf_init_path on tied_file",
                dres, onef_err);
        }
    }

    memset(&printfcallbackdata,0,sizeof(printfcallbackdata));
    printfcallbackdata.dp_fptr = printf_callback_for_libdwarf;
    dwarf_register_printf_callback(dbg,&printfcallbackdata);
    if (dbgtied) {
        dwarf_register_printf_callback(dbgtied,&printfcallbackdata);
    }
    memset(&printfcallbackdata,0,sizeof(printfcallbackdata));

    dbgsetup(dbg,l_config_file_data);
    dbgsetup(dbgtied,l_config_file_data);
    dres = get_address_size_and_max(dbg,&elf_address_size,0,
        &onef_err);
    if (dres != DW_DLV_OK) {
        print_error(dbg,"Unable to read address"
            " size so unable to continue",
            dres,onef_err);
    }
    if (glflags.gf_check_tag_attr ||
        glflags.gf_print_usage_tag_attr) {
        dres = build_attr_form_base_tree(&localerrno);
        if (dres != DW_DLV_OK) {
            simple_err_return_msg_either_action(dres,
                "ERROR: Failed to initialize attribute/form"
                " tables properly");
        }
    }

    /*  Ok for dbgtied to be NULL. */
    dres = dwarf_set_tied_dbg(dbg,dbgtied,&onef_err);
    if (dres != DW_DLV_OK) {
        print_error(dbg, "dwarf_set_tied_dbg() failed",
            dres, onef_err);
    }

    /*  Get .text and .debug_ranges info if in check mode.
        Depending on the section count and layout
        it is possible this
        will not get all the sections it wants to:
        See calculate_likely_limits_of_code().  */
    if (glflags.gf_do_check_dwarf) {
        Dwarf_Addr lower = 0;
        Dwarf_Addr upper = 0;
        Dwarf_Unsigned size = 0;
        Dwarf_Debug dbg_with_code = dbg;
        int res = 0;

        if (dbgtied) {
            /*  Assuming tied is exectuable main is dwo/dwp */
            dbg_with_code = dbgtied;
        }
        res = calculate_likely_limits_of_code(dbg_with_code,
            &lower,&size);
        upper = lower + size;
        /*  Set limits for Ranges Information.
            Some objects have CUs for startup code
            and the expanded range here turns out
            not to actually help.   */
        if (res == DW_DLV_OK && glflags.pRangesInfo) {
            /*  Recording high/low for .test .init .fini */
            SetLimitsBucketGroup(glflags.pRangesInfo,lower,upper);
            AddEntryIntoBucketGroup(glflags.pRangesInfo,
                1,
                lower,lower,
                upper,
                ".text",
                TRUE);
        }
        /*  Build section information for linkonce.
            linkonce is related to, for example,
            gcc .gnu.linkonce.?? sections we have no reason to think
            this really works, we have no test cases as of 2022. */
        build_linkonce_info(dbg);
    }
    if (glflags.gf_section_groups_flag) {
        int res = 0;
        Dwarf_Error err = 0;

        res = print_section_groups_data(dbg,&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing section groups had a problem.",
                res,err);
            DROP_ERROR_INSTANCE(dbg,res,err);
        }
        /*  If groupnum > 2 this turns off some
            of the gf_flags here so we don't print
            section names of things we do not
            want to print. */
        update_section_flags_per_groups();
    }
    reset_overall_CU_error_data();
    if (glflags.gf_info_flag || glflags.gf_line_flag ||
        glflags.gf_types_flag ||
        glflags.gf_check_macros || glflags.gf_macinfo_flag ||
        glflags.gf_macro_flag ||
        glflags.gf_cu_name_flag || glflags.gf_search_is_on ||
        glflags.gf_producer_children_flag) {
        Dwarf_Error err = 0;
        int res = 0;

        reset_overall_CU_error_data();
        res = print_infos(dbg,TRUE,&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing .debug_info had a problem.",
                res,err);
            DROP_ERROR_INSTANCE(dbg,res,err);
        }
        reset_overall_CU_error_data();
        res = print_infos(dbg,FALSE,&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing .debug_types had a problem.",
                res,err);
            DROP_ERROR_INSTANCE(dbg,res,err);
        }
        {
            set_global_section_sizes(dbg);
            /*  The statistics are for ALL of the
                DWARF5 (and DWARF4 with .debug_macro)
                across all CUs.  */
            if (macro_check_tree) {
                /* debug_macro_size is to check the section end */
                print_macrocheck_statistics("DWARF5 .debug_macro",
                    &macro_check_tree,
                    /* DWARF5 */ TRUE,
                    glflags.section_high_offsets_global->
                        debug_macro_size);
            }
        }
        if (glflags.gf_check_macros) {
            if (macinfo_check_tree) {
                /* debug_macinfo_size is to check the section end */
                print_macrocheck_statistics("DWARF2 .debug_macinfo",
                    &macinfo_check_tree,
                    /* DWARF5 */ FALSE,
                    glflags.section_high_offsets_global->
                        debug_macinfo_size);
            }
        }
        clear_macrocheck_statistics(&macro_check_tree);
        clear_macrocheck_statistics(&macinfo_check_tree);
    }
    if (glflags.gf_gdbindex_flag) {
        int res = 0;
        Dwarf_Error err = 0;

        reset_overall_CU_error_data();
        /*  By definition if gdb_index is present
            then "cu" and "tu" will not be. And vice versa.  */
        res = print_gdb_index(dbg,&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing the gdb index section had a problem "
                ,res,err);
        }
        res = print_debugfission_index(dbg,"cu",&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing the debugfission cu section "
                "had a problem "
                ,res,err);
        }
        res = print_debugfission_index(dbg,"tu",&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing the debugfission tu section "
                "had a problem "
                ,res,err);
        }
    }
    if (glflags.gf_pubnames_flag) {
        int res = 0;
        Dwarf_Error err = 0;

        reset_overall_CU_error_data();
        res = print_pubnames_style(dbg,DW_GL_GLOBALS,&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing pubnames data had a problem ",res,err);
            DROP_ERROR_INSTANCE(dbg,res,err);
        }
    }
    if (glflags.gf_debug_addr_flag) {
        Dwarf_Error err = 0;
        int res = 0;

        reset_overall_CU_error_data();
        res = print_debug_addr(dbg,&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing the .debug_addr section"
                " had a problem.",res,err);
            DROP_ERROR_INSTANCE(dbg,res,err);
        }
    }
    if (glflags.gf_abbrev_flag) {
        Dwarf_Error err = 0;
        int res = 0;

        reset_overall_CU_error_data();
        res = print_abbrevs(dbg,&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing the .debug_abbrev section"
                " had a problem.",res,err);
            DROP_ERROR_INSTANCE(dbg,res,err);
        }
    }
    if (glflags.gf_string_flag) {
        Dwarf_Error err = 0;
        int res = 0;

        reset_overall_CU_error_data();
        res = print_strings(dbg,&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing the .debug_str section"
                " had a problem.",res,err);
            DROP_ERROR_INSTANCE(dbg,res,err);
        }
    }
    if (glflags.gf_aranges_flag) {
        Dwarf_Error err = 0;
        int res = 0;

        reset_overall_CU_error_data();
        res = print_aranges(dbg,&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing the aranges section"
                " had a problem.",res,err);
            DROP_ERROR_INSTANCE(dbg,res,err);
        }
    }
    if (glflags.gf_ranges_flag) {
        reset_overall_CU_error_data();
        print_ranges(dbg);
        /* Never returns DW_DLV_ERROR */
    }
    if (glflags.gf_print_raw_loclists) {
        int res = 0;
        Dwarf_Error err = 0;
        /*  This for DWARF5 loclists.
            No raw printing of .debug_loc available. */

        reset_overall_CU_error_data();
        res = print_raw_all_loclists(dbg,&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing the raw .debug_loclists section"
                " had a problem.",res,err);
            DROP_ERROR_INSTANCE(dbg,res,err);
        }
    }
    if (glflags.gf_print_raw_rnglists) {
        int res = 0;
        Dwarf_Error err = 0;

        reset_overall_CU_error_data();
        res = print_raw_all_rnglists(dbg,&err);
        if (res == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing the raw .debug_rnglists section"
                " had a problem.",res,err);
            DROP_ERROR_INSTANCE(dbg,res,err);
        }
    }
    if (glflags.gf_frame_flag || glflags.gf_eh_frame_flag) {
        int sres = 0;
        Dwarf_Error err = 0;
        int want_eh = 0;
        /*  These three shared .eh_frame and .debug_frame
            as they are about the DIEs, not about frames. */
        Dwarf_Die cu_die_for_print_frames = 0;
        void *map_lowpc_to_name = 0;
        void *lowpcSet = 0;

        reset_overall_CU_error_data();
        if (glflags.gf_frame_flag) {
            want_eh = 0;
            sres = print_frames(dbg,want_eh,
                l_config_file_data,
                &cu_die_for_print_frames,
                &map_lowpc_to_name,
                &lowpcSet,
                &err);
            if (sres == DW_DLV_ERROR) {
                print_error_and_continue(
                    "printing standard frame data had a problem.",
                    sres,err);
                DROP_ERROR_INSTANCE(dbg,sres,err);
            }
        }
        if (glflags.gf_eh_frame_flag) {
            want_eh = 1;
            sres = print_frames(dbg, want_eh,
                l_config_file_data,
                &cu_die_for_print_frames,
                &map_lowpc_to_name,
                &lowpcSet,
                &err);
            if (sres == DW_DLV_ERROR) {
                print_error_and_continue(
                    "printing eh frame data had a problem.",sres,
                    err);
                DROP_ERROR_INSTANCE(dbg,sres,err);
            }
        }
        addr_map_destroy(lowpcSet);
        addr_map_destroy(map_lowpc_to_name);
        if (cu_die_for_print_frames) {
            dwarf_dealloc_die(cu_die_for_print_frames);
        }
    }
    if (glflags.gf_static_func_flag) {
        int sres = 0;
        Dwarf_Error err = 0;

        reset_overall_CU_error_data();
        sres = print_pubnames_style(dbg,DW_GL_FUNCS,&err);
        if (sres == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing SGI static funcs had a problem.",sres,err);
            DROP_ERROR_INSTANCE(dbg,sres,err);
        }
    }
    if (glflags.gf_static_var_flag) {
        int sres = 0;
        Dwarf_Error err = 0;

        reset_overall_CU_error_data();
        sres = print_pubnames_style(dbg,DW_GL_VARS,&err);
        if (sres == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing SGI static vars had a problem.",sres,err);
            DROP_ERROR_INSTANCE(dbg,sres,err);
        }
    }
    /*  DWARF_PUBTYPES is the standard typenames dwarf section.
        SGI_TYPENAME is the same concept but is SGI specific ( it was
        defined 10 years before dwarf pubtypes). */

    if (glflags.gf_pubtypes_flag) {
        Dwarf_Error err = 0;
        int tres = 0;

        reset_overall_CU_error_data();
        tres = print_pubnames_style(dbg,DW_GL_PUBTYPES,&err);
        if (tres == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing pubtypes had a problem.",tres,err);
            DROP_ERROR_INSTANCE(dbg,tres,err);
        }
        reset_overall_CU_error_data();
        tres = print_pubnames_style(dbg,DW_GL_TYPES,&err);
        if (tres == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing SGI typenames had a problem.",tres,err);
            DROP_ERROR_INSTANCE(dbg,tres,err);
        }
    }
    if (glflags.gf_weakname_flag) {
        Dwarf_Error err = 0;
        int res3 = 0;

        reset_overall_CU_error_data();
        res3 = print_pubnames_style(dbg,DW_GL_WEAKS,&err);
        if (res3 == DW_DLV_ERROR) {
            print_error_and_continue(
                "printing weaknames had a problem.",res3,err);
            DROP_ERROR_INSTANCE(dbg,res3,err);
        }
    }
    if (glflags.gf_reloc_flag) {
        /* Harmless, though likely not needed here. */
        reset_overall_CU_error_data();
    }
    if (glflags.gf_debug_names_flag) {
        int nres = 0;
        Dwarf_Error err = 0;
        reset_overall_CU_error_data();
        nres = print_debug_names(dbg,&err);
        if (nres == DW_DLV_ERROR) {
            print_error_and_continue(
                "print .debug_names section failed", nres, err);
            DROP_ERROR_INSTANCE(dbg,nres,err);
        }
    }

    /*  Print search results */
    if (glflags.gf_search_print_results && glflags.gf_search_is_on) {
        /* No dwarf errors possible in this function. */
        print_search_results();
    }

    /*  The right time to do this is unclear. But we need to do it. */
    if (glflags.gf_check_harmless) {
        /* No dwarf errors possible in this function. */
        print_any_harmless_errors(dbg);
    }

    /*  Print error report only if errors have been detected
        Print error report if the -kd option.
        No errors possible in this function. */
    print_checks_results();

    /*  Print the detailed attribute usage space
        and free the attributes_encoding data allocated.
        Option -kE
        Also prints the attr/formclass/form reports
        from attr_form.c  See build_attr_form_base()
        call above and record_attr_form_use() in print_die.c */
    if (glflags.gf_check_attr_encoding ) {
        int ares = 0;
        Dwarf_Error aerr = 0;

        ares = print_attributes_encoding(dbg,&aerr);
        if (ares == DW_DLV_ERROR) {
            print_error_and_continue(
                "print attributes encoding failed", ares, aerr);
            DROP_ERROR_INSTANCE(dbg,ares,aerr);
        }
    }

    /*  Print the tags and attribute usage  -ku or -kuf */
    if (glflags.gf_print_usage_tag_attr) {
        int tres = 0;
        Dwarf_Error err = 0;

        tres = print_tag_attributes_usage();
        if (tres == DW_DLV_ERROR) {
            print_error_and_continue(
                "print tag attributes usage failed", tres, err);
            DROP_ERROR_INSTANCE(dbg,tres,err);
        }
    }

    if (glflags.gf_print_str_offsets) {
        /*  print the .debug_str_offsets section, if any. */
        int lres = 0;
        Dwarf_Error err = 0;

        lres = print_str_offsets_section(dbg,&err);
        if (lres == DW_DLV_ERROR) {
            print_error_and_continue(
                "print .debug_str_offsets failed", lres, err);
            DROP_ERROR_INSTANCE(dbg,lres,err);
        }
    }

    /*  prints nothing unless section .gnu_debuglink is present.
        Lets print for a few critical sections.  */
    if (glflags.gf_gnu_debuglink_flag) {
        int lresdl = 0;
        Dwarf_Error err = 0;

        lresdl = print_gnu_debuglink(dbg,&err);
        if (lresdl == DW_DLV_ERROR) {
            print_error_and_continue(
                "print gnu_debuglink data failed", lresdl, err);
            DROP_ERROR_INSTANCE(dbg,lresdl,err);
            err = 0;
        }
    }
    if (glflags.gf_debug_gnu_flag) {
        int lres = 0;
        Dwarf_Error err = 0;

        lres = print_debug_gnu(dbg,&err);
        if (lres == DW_DLV_ERROR) {
            print_error_and_continue(
                "print .debug_gnu* section failed", lres, err);
            DROP_ERROR_INSTANCE(dbg,lres,err);
            err = 0;
        }
    }
    if (glflags.gf_debug_sup_flag) {
        int lres = 0;
        Dwarf_Error err = 0;

        lres = print_debug_sup(dbg,&err);
        if (lres == DW_DLV_ERROR) {
            print_error_and_continue(
                "print .debug_sup* section failed", lres, err);
            DROP_ERROR_INSTANCE(dbg,lres,err);
        }
    }
    if (glflags.gf_debug_addr_missing) {
        printf("\nERROR: At some point "
            "the .debug_addr section was needed but missing, "
            "meaning some frame information was missing "
            "relevant function names. See the dwarfdump "
            " option --file-tied=</path/to/executable>.");
        glflags.gf_count_major_errors++;
    }
    if (glflags.gf_error_code_search_by_address) {
        printf("\nERROR: At some point "
            "There was some data corruption in frame data "
            "so at least the following error occurred: "
            "%s .\n",
            dwarf_errmsg_by_number(
            glflags.gf_error_code_search_by_address));
        glflags.gf_count_major_errors++;
    }

    /*  Could finish dbg first. Either order ok. */
    if (dbgtied) {
        dres = dwarf_finish(dbgtied);
        if (dres != DW_DLV_OK) {
            print_error_and_continue(
                "dwarf_finish failed on tied dbg", dres, onef_err);
            DROP_ERROR_INSTANCE(dbg,dres,onef_err);
        }
        dbgtied = 0;
    }
    groups_restore_subsidiary_flags();
    dres = dwarf_finish(dbg);
    if (dres != DW_DLV_OK) {
        print_error_and_continue(
            "dwarf_finish failed", dres, onef_err);
        DROP_ERROR_INSTANCE(dbg,dres,onef_err);
        dbg = 0;
    }
    printf("\n");
    destroy_attr_form_trees();
    destruct_abbrev_array();
    esb_close_null_device();
    release_range_array_info();
    helpertree_clear_statistics(&helpertree_offsets_base_info);
    helpertree_clear_statistics(&helpertree_offsets_base_types);
    return 0;
}

/*  ==============START of dwarfdump error print functions. */
int
simple_err_return_msg_either_action(int res,const char *msg)
{
    const char *etype = "No-entry";
    if (res == DW_DLV_ERROR) {
        etype="Major error";
    }
    glflags.gf_count_major_errors++;
    printf("%s fails. %s\n",msg,etype);
    return res;
}
int
simple_err_return_action(int res,const char *msg)
{
    if (res == DW_DLV_ERROR) {
        const char *etype = "Major error";
        glflags.gf_count_major_errors++;
        printf("%s %s\n",msg, etype);
    }
    return res;
}

int
simple_err_only_return_action(int res,const char *msg)
{
    const char *etype="Major error";
    /*const char *msg = "\nERROR: dwarf_get_address_size() fails."; */

    glflags.gf_count_major_errors++;
    printf("%s %s\n",msg,etype);
    return res;
}

/*   Does not increment glflags.gf_count_major_errors
    unless a return code is not one of the three
    standard values.
*/
static void
print_error_maybe_continue( const char * msg,
    int dwarf_ret_val,
    Dwarf_Error lerr,
    Dwarf_Bool do_continue)
{
    unsigned long realmajorerr = glflags.gf_count_major_errors;
    printf("\n");
    if (dwarf_ret_val == DW_DLV_ERROR) {
        /* We do not dwarf_dealloc the error here. */
        char * errmsg = dwarf_errmsg(lerr);

        /*  We now (April 2016) guarantee the
            error number is in
            the error string so we do not need to print
            the dwarf_errno() value to show the number. */
        if (do_continue) {
            printf(
                "%s ERROR:  %s:  %s. "
                "Attempting to continue.\n",
                glflags.program_name, msg, errmsg);
        } else {
            printf( "%s ERROR:  %s:  %s\n",
                glflags.program_name, msg, errmsg);
        }
    } else if (dwarf_ret_val == DW_DLV_NO_ENTRY) {
        printf("%s NO ENTRY:  %s: \n",
            glflags.program_name, msg);
    } else if (dwarf_ret_val == DW_DLV_OK) {
        printf("%s:  %s \n", glflags.program_name, msg);
    } else {
        printf("%s ERROR InternalError:  %s:  code %d\n",
            glflags.program_name, msg, dwarf_ret_val);
        ++realmajorerr;
    }
    /* Display compile unit name */
    PRINT_CU_INFO();
    glflags.gf_count_major_errors = realmajorerr;
}

void
print_error(Dwarf_Debug dbg,
    const char * msg,
    int dwarf_ret_val,
    Dwarf_Error lerr)
{
    print_error_maybe_continue(msg,dwarf_ret_val,lerr,FALSE);
    glflags.gf_count_major_errors++;
    if (dwarf_ret_val == DW_DLV_ERROR) {
        /*  If dbg was never initialized
            this still cleans up the Error data. */
        DROP_ERROR_INSTANCE(dbg,dwarf_ret_val,lerr);
        dwarf_finish(dbg);
        check_for_major_errors();
        check_for_notes();
    }
    global_destructors();
    flag_data_post_cleanup();
    destroy_attr_form_trees();
    exit(EXIT_FAILURE);
}
/* ARGSUSED */
void
print_error_and_continue(const char * msg,
    int dwarf_ret_val,
    Dwarf_Error lerr)
{
    glflags.gf_count_major_errors++;
    print_error_maybe_continue(msg,dwarf_ret_val,lerr,TRUE);
}
/*  ==============END of dwarfdump error print functions. */

static Dwarf_Bool
is_a_string_form(int sf)
{
    switch(sf){
    case DW_FORM_string:
    case DW_FORM_GNU_strp_alt:
    case DW_FORM_strp_sup:
    case DW_FORM_GNU_str_index:
    case DW_FORM_strx:
    case DW_FORM_strx1:
    case DW_FORM_strx2:
    case DW_FORM_strx3:
    case DW_FORM_strx4:
    case DW_FORM_strp:
    case DW_FORM_line_strp:
        /*  There is some hope we can actually get
            the string itself, depending on
            other factors */
        return TRUE;
    default: break;
    }
    /* Nope. No string is possible */
    return FALSE;
}
/*  Always sets the return argument *should_skip,
    whether it returns DW_DLV_NO_ENTRY or
    DW_DLV_ERROR or DW_DLV_OK.
    determines if the CU should be
    skipped as the DW_AT_name of the CU
    does not match the command-line-supplied
    cu name.  The two callers ignore the
    return value.
    This suppresses any errors it finds, no
    Dwarf_Error is lost and none is returned. */
int
should_skip_this_cu(Dwarf_Debug dbg, Dwarf_Bool*should_skip,
    Dwarf_Die cu_die)
{
    Dwarf_Half tag = 0;
    Dwarf_Attribute attrib = 0;
    Dwarf_Half theform = 0;
    Dwarf_Error skperr;
    int dares = 0;
    int tres = 0;
    int fres = 0;

    tres = dwarf_tag(cu_die, &tag, &skperr);
    if (tres != DW_DLV_OK) {
        print_error_and_continue("ERROR: "
        "Cannot get the TAG of the cu_die to check "
        " if we should skip this CU or not.",
            tres, skperr);
        *should_skip = FALSE;
        DROP_ERROR_INSTANCE(dbg,tres,skperr);
        return tres;
    }
    dares = dwarf_attr(cu_die, DW_AT_name, &attrib, &skperr);
    if (dares != DW_DLV_OK) {
        print_error_and_continue("should skip this cu? "
            " cu die has no DW_AT_name attribute!",
            dares, skperr);
        *should_skip = FALSE;
        DROP_ERROR_INSTANCE(dbg,dares,skperr);
        return dares;
    }
    fres = dwarf_whatform(attrib, &theform, &skperr);
    if (fres == DW_DLV_OK) {
        if (is_a_string_form(theform)) {
            char * temps = 0;
            int sres = dwarf_formstring(attrib, &temps,
                &skperr);
            if (sres == DW_DLV_OK) {
                char *lcun = esb_get_string(glflags.cu_name);
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
                    dwarf_dealloc_attribute(attrib);
                    attrib = 0;
                    *should_skip = TRUE;
                    return DW_DLV_OK;
                }
#else
                if (strcmp(lcun, p)) {
                    /* skip this cu. */
                    dwarf_dealloc_attribute(attrib);
                    attrib = 0;
                    *should_skip = TRUE;
                    return DW_DLV_OK;
                }
#endif /* _WIN32 */

            } else if (sres == DW_DLV_ERROR) {
                struct esb_s m;
                int dwarf_names_print_on_error = 1;

                dwarf_dealloc_attribute(attrib);
                attrib = 0;
                esb_constructor(&m);
                esb_append(&m,"In determining if we should "
                    "skip this CU dwarf_formstring "
                    "gets an error on form ");
                esb_append(&m,get_FORM_name(theform,
                    dwarf_names_print_on_error));
                esb_append(&m,".");

                print_error_and_continue(
                    esb_get_string(&m),
                    sres, skperr);
                *should_skip = FALSE;
                esb_destructor(&m);
                return sres;
            } else {
                /* DW_DLV_NO_ENTRY on the string itself */
                dwarf_dealloc_attribute(attrib);
                attrib = 0;
                *should_skip = FALSE;
                return sres;
            }
        }
    } else if (fres == DW_DLV_ERROR) {
        /*  DW_DLV_ERROR */
        print_error_and_continue(
            "dwarf_whatform failed on a CU_die when"
            " attempting to determine if this CU should"
            " be skipped.",
            fres, skperr);
        DROP_ERROR_INSTANCE(dbg,fres,skperr);
        fres = DW_DLV_NO_ENTRY;
    } else  {
        /* DW_DLV_NO_ENTRY, nothing to print */
    }
    dwarf_dealloc_attribute(attrib);
    attrib = 0;
    *should_skip = FALSE;
    return fres;
}

/*  Returns the cu of the CUn the name fields when it can,
    else a no-entry
    else DW_DLV_ERROR.  */
int
get_cu_name(Dwarf_Debug dbg, Dwarf_Die cu_die,
    Dwarf_Off dieprint_cu_offset,
    char * *short_name, char * *long_name,
    Dwarf_Error *lerr)
{
    Dwarf_Attribute name_attr = 0;
    int ares = 0;

    ares = dwarf_attr(cu_die, DW_AT_name, &name_attr, lerr);
    if (ares == DW_DLV_ERROR) {
        print_error_and_continue(
            "dwarf_attr fails on DW_AT_name on the CU die",
            ares, *lerr);
        return ares;
    } else if (ares == DW_DLV_NO_ENTRY) {
        *short_name = "<unknown name>";
        *long_name = "<unknown name>";
    } else {
        /* DW_DLV_OK */
        /*  The string return is valid until the next call to this
            function; so if the caller needs to keep the returned
            string, the string must be copied (makename()). */
        char *filename = 0;

        esb_empty_string(&esb_long_cu_name);
        ares = get_attr_value(dbg, DW_TAG_compile_unit,
            cu_die, dieprint_cu_offset,
            name_attr, NULL, 0, &esb_long_cu_name,
            0 /*show_form_used*/,0 /* verbose */,lerr);
        if (ares != DW_DLV_OK)  {
            *short_name = "<unknown name>";
            *long_name = "<unknown name>";
            return ares;
        }
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
        dwarf_dealloc_attribute(name_attr);
    }
    return ares;
}

/*  Returns the producer of the CU
    Caller must ensure producernameout is
    a valid, constructed, empty esb_s instance before calling.
    */
int
get_producer_name(Dwarf_Debug dbg, Dwarf_Die cu_die,
    Dwarf_Off dieprint_cu_offset,
    struct esb_s *producernameout,
    Dwarf_Error *err)
{
    Dwarf_Attribute producer_attr = 0;
    int ares = 0;
    /*  See also glflags.c for "<unknown>" as default producer
        string */

    if (!cu_die) {
        glflags.gf_count_major_errors++;
        esb_append(producernameout,
            "\"<ERROR: CU-missing-DW_AT_producer (null cu_die)>\"");
        return DW_DLV_NO_ENTRY;
    }
    ares = dwarf_attr(cu_die, DW_AT_producer,
        &producer_attr, err);
    if (ares == DW_DLV_ERROR) {
        glflags.gf_count_major_errors++;
        esb_append(producernameout,
            "\"<ERROR: CU-DW_AT_producer-error>\"");
        return ares;
    }
    if (ares == DW_DLV_NO_ENTRY) {
        /*  We add extra quotes so it looks more like
            the names for real producers that get_attr_value
            produces. */
        /* Same string is in glflags.c */
        esb_append(producernameout,
            "\"<ERROR: CU-missing-DW_AT_producer>\"");
        dwarf_dealloc_attribute(producer_attr);
        return ares;
    }
    /*  DW_DLV_OK */
    ares = get_attr_value(dbg, DW_TAG_compile_unit,
        cu_die, dieprint_cu_offset,
        producer_attr, NULL, 0, producernameout,
        0 /*show_form_used*/,0 /* verbose */,err);
    dwarf_dealloc_attribute(producer_attr);
    return ares;
}

void
print_secname(Dwarf_Debug dbg,const char *secname)
{
    if (glflags.gf_do_print_dwarf) {
        struct esb_s truename;
        char buf[DWARF_SECNAME_BUFFER_SIZE];

        esb_constructor_fixed(&truename,buf,sizeof(buf));
        get_true_section_name(dbg,secname,
            &truename,TRUE);
        printf("\n%s\n",sanitized(esb_get_string(&truename)));
        esb_destructor(&truename);
    }
}

/*  We'll check for errors when checking.
    print only if printing (as opposed to checking). */
static int
print_gnu_debuglink(Dwarf_Debug dbg, Dwarf_Error *error)
{
    int         res = 0;
    char *      name = 0;
    unsigned char *crcbytes = 0;
    char *      link_path = 0;
    unsigned    link_path_len = 0;
    unsigned    buildidtype = 0;
    char       *buildidowner = 0;
    unsigned char *buildidbyteptr = 0;
    unsigned    buildidlength = 0;
    char      **paths_array = 0;
    unsigned    paths_array_length = 0;

    res = dwarf_gnu_debuglink(dbg,
        &name,
        &crcbytes,
        &link_path,     /* free this */
        &link_path_len,
        &buildidtype,
        &buildidowner,
        &buildidbyteptr, &buildidlength,
        &paths_array,  /* free this */
        &paths_array_length,
        error);
    if (res == DW_DLV_NO_ENTRY) {
        return res;
    }
    if (res == DW_DLV_ERROR) {
        print_secname(dbg,".gnu_debuglink");
        return res;
    }
    if (crcbytes) {
        print_secname(dbg,".gnu_debuglink");
        /* Done with error checking, so print if we are printing. */
        if (glflags.gf_do_print_dwarf)  {
            printf(" Debuglink name  : %s",sanitized(name));
            {
                unsigned char *crc = 0;
                unsigned char *end = 0;

                crc = crcbytes;
                end = crcbytes +4;
                printf("   crc 0X: ");
                for (; crc < end; crc++) {
                    printf("%02x ", *crc);
                }
            }
            printf("\n");
            if (link_path_len) {
                printf(" Debuglink target: %s\n",
                    sanitized(link_path));
            }
        }
    }
    if (buildidlength) {
        print_secname(dbg,".note.gnu.build-id");
        if (glflags.gf_do_print_dwarf)  {
            printf(" Build-id  type     : %u\n", buildidtype);
            printf(" Build-id  ownername: %s\n",
                sanitized(buildidowner));
            printf(" Build-id  length   : %u\n",buildidlength);
            printf(" Build-id           : ");
            {
                const unsigned char *cur = 0;
                const unsigned char *end = 0;

                cur = buildidbyteptr;
                end = cur + buildidlength;
                for (; cur < end; cur++) {
                    printf("%02x", (unsigned char)*cur);
                }
            }
            printf("\n");
        }
    }
    if (paths_array_length) {
        unsigned i = 0;

        printf(" Possible "
            ".gnu_debuglink/.note.gnu.build-id pathnames for\n");
        printf(" an alternate object file with more detailed "
            "DWARF\n");
        for ( ; i < paths_array_length; ++i) {
            char *path = paths_array[i];
            char           outpath[2000];
            unsigned long  outpathlen = sizeof(outpath);
            unsigned int   ftype = 0;
            unsigned int   endian = 0;
            unsigned int   offsetsize = 0;
            Dwarf_Unsigned filesize = 0;
            /*  See DW_PATHSOURCE_dsym for what
                pathsource might be set to in the
                call below. */
            unsigned char  pathsource = 0;
            int            errcode  = 0;

            printf("  [%u] %s\n",i,sanitized(path));
            res = dwarf_object_detector_path_b(path,
                outpath,outpathlen,
                0,0,
                &ftype,&endian,&offsetsize,
                &filesize, &pathsource, &errcode);
            if (res == DW_DLV_NO_ENTRY) {
                if (glflags.verbose) {
                    printf(" file above does not exist\n");
                }
                continue;
            }
            if (res == DW_DLV_ERROR) {
                printf("       access attempt of the above leads"
                    " to error %s\n",
                    dwarf_errmsg_by_number(errcode));
                continue;
            }
            switch(ftype) {
            case DW_FTYPE_ELF:
                printf("       file above is an Elf object\n");
                break;
            case DW_FTYPE_MACH_O:
                printf("       file above is a Mach-O object\n");
                break;
            case DW_FTYPE_PE:
                printf("       file above is a PE object");
                break;
            case DW_FTYPE_ARCHIVE:
                if (glflags.verbose) {
                    printf("       file above is an archive "
                        "so ignore it.\n");
                }
                continue;
            default:
                if (glflags.verbose) {
                    printf("       file above is not"
                        " any object type we recognize\n");
                }
                continue;
            }
        }
        printf("\n");
    }
    free(link_path);
    free(paths_array);
    return DW_DLV_OK;
}

/* GCC linkonce names */
char *lo_text           = ".text."; /*".gnu.linkonce.t.";*/
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
    for (section_index = 1;
        section_index < nCount;
        ++section_index) {
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
                    AddEntryIntoBucketGroup(glflags.pLinkonceInfo,
                        section_index,
                        section_addr,section_addr,
                        section_addr + section_size,
                        section_name,
                        TRUE);
                    break;
                }
            }
        } else if (res == DW_DLV_ERROR) {
            dwarf_dealloc_error(dbg,error);
            error = 0;
        }
    }
    if (dump_linkonce_info) {
        /*  Unlikely this is ever useful...at present. */
        PrintBucketGroup(glflags.pLinkonceInfo);
    }
}

/* Check for specific TAGs and initialize some
    information used by '-k' options */
void
tag_specific_globals_setup(Dwarf_Debug dbg,
    Dwarf_Half val,int die_indent_level)
{
    switch (val) {
    /*  DW_TAG_type unit will not have addresses */
    /*  DW_TAG_skeleton unit will have addresses, but
        likely no children. But they are useful
        as marking glflags.seen_CU TRUE is useful */
    case DW_TAG_partial_unit:
    case DW_TAG_compile_unit:
    case DW_TAG_type_unit:
    case DW_TAG_skeleton_unit:
        /* To help getting the compile unit name */
        glflags.seen_CU = TRUE;
        /*  If we are checking line information, build
            the table containing the pairs LowPC and HighPC */
        if (glflags.gf_check_decl_file ||
            glflags.gf_check_ranges ||
            glflags.gf_check_locations) {
            Dwarf_Debug td = 0;

            if (!dbg) {
                ResetBucketGroup(glflags.pRangesInfo);
            } else {
                /*  Only returns DW_DLV_OK */
                dwarf_get_tied_dbg(dbg,&td,0);
                /*  With a tied-dbg we do not have
                    detailed address ranges, so
                    do not reset the single .text-size
                    bucket group */
                if (!td) {
                    ResetBucketGroup(glflags.pRangesInfo);
                }
            }
        }
        /*  The following flag indicate that only
            low_pc and high_pc
            values found in DW_TAG_subprograms
            are going to be considered when
            building the address table used to check
            ranges, lines, etc */
        glflags.need_PU_valid_code = TRUE;
        break;

    case DW_TAG_subprogram:
        /* Keep track of a PU */
        if (die_indent_level == 1) {
            /*  A DW_TAG_subprogram can be nested,
                when is used to
                declare a member function for a
                local class; process the DIE
                only if we are at level zero in the DIEs tree */
            glflags.seen_PU = TRUE;
            glflags.seen_PU_base_address = FALSE;
            glflags.seen_PU_high_address = FALSE;
            glflags.PU_name[0] = 0;
            glflags.need_PU_valid_code = TRUE;
        }
        break;
    default: break;
    }
}

/*  Print CU basic information but
    use the local DIE for the offsets. */
void
PRINT_CU_INFO(void)
{
    Dwarf_Unsigned loff = glflags.DIE_offset;
    Dwarf_Unsigned goff = glflags.DIE_section_offset;
    char lbuf[50];
    char hbuf[50];

    if (glflags.current_section_id == DEBUG_LINE ||
        glflags.current_section_id == DEBUG_FRAME ||
        glflags.current_section_id == DEBUG_FRAME_EH_GNU ||
        glflags.current_section_id == DEBUG_ARANGES ||
        glflags.current_section_id == DEBUG_MACRO ||
        glflags.current_section_id == DEBUG_PUBNAMES ||
        glflags.current_section_id == DEBUG_MACINFO ) {
        /*  These sections involve the CU die, so
            use the CU offsets.
            The DEBUG_MAC* cases are logical but
            not yet useful (Dec 2015).
            In other cases the local DIE offset makes
            more sense. */
        loff = glflags.DIE_CU_offset;
        goff = glflags.DIE_CU_overall_offset;
    }
    if (!cu_data_is_set()) {
        return;
    }
    printf("\n");
    printf("CU Name = %s\n",sanitized(glflags.CU_name));
    printf("CU Producer = %s\n",sanitized(glflags.CU_producer));
    printf("DIE OFF = 0x%" DW_PR_XZEROS DW_PR_DUx
        " GOFF = 0x%" DW_PR_XZEROS DW_PR_DUx,
        loff,goff);
    /* We used to print leftover and incorrect values at times. */
    if (glflags.need_CU_high_address) {
        dd_safe_strcpy(hbuf,sizeof(hbuf),"unknown   ",10);
    } else {
        /* safe, hbuf is large enough. */
        sprintf(hbuf,
            "0x%"  DW_PR_XZEROS DW_PR_DUx,glflags.CU_high_address);
    }
    if (glflags.need_CU_base_address) {
        dd_safe_strcpy(lbuf,sizeof(lbuf),"unknown   ",10);
    } else {
        /* safe, lbuf is large enough. */
        sprintf(lbuf,
            "0x%"  DW_PR_XZEROS DW_PR_DUx,glflags.CU_low_address);
    }
    printf(", Low PC = %s, High PC = %s", lbuf,hbuf);
    printf("\n");
}

void
DWARF_CHECK_ERROR_PRINT_CU(void)
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
    glflags.check_error++;
    glflags.gf_record_dwarf_error = TRUE;
}

/*  Sometimes is useful, just to know the kind of errors
    in an object file; not much interest in the number
    of errors; the specific case is just to have a general
    idea about the DWARF quality in the file */

char ** set_unique_errors = NULL;
unsigned int set_unique_errors_entries = 0;
unsigned int set_unique_errors_size = 0;
#define SET_UNIQUE_ERRORS_DELTA 64

/*  Create the space to store the unique error messages */
void
allocate_unique_errors_table(void)
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
void
dump_unique_errors_table(void)
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
void
release_unique_errors_table(void)
{
#if 0
    The pointed-to entries are all saved in makename,
    so let its destructor do the work.
    unsigned int index;
    for (index = 0; index < set_unique_errors_entries; ++index) {
        free(set_unique_errors[index]);
    }
#endif
    free(set_unique_errors);
    set_unique_errors = 0;
    set_unique_errors_entries = 0;
    set_unique_errors_size = 0;
}

/*  Returns TRUE if the text is already in the set; otherwise FALSE */
Dwarf_Bool
add_to_unique_errors_table(char * error_text)
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
    if (!filtered_text) {
        /* Lets not do anything. */
        return TRUE;
    }
    len = strlen(filtered_text);

    /*  Remove from the error_text, any hexadecimal
        numbers (start with 0x),
        because for some errors, an additional
        information is given in the
        form of addresses; we are interested just in
        the general error. */
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

    /*  Store the new text; check if we have space
        to store the error text */
    if (set_unique_errors_entries + 1 == set_unique_errors_size) {
        set_unique_errors_size += SET_UNIQUE_ERRORS_DELTA;
        set_unique_errors = (char **)realloc(set_unique_errors,
            set_unique_errors_size * sizeof(char*));
    }

    set_unique_errors[set_unique_errors_entries] = filtered_text;
    ++set_unique_errors_entries;

    return FALSE;
}

/*
    Print a DWARF error message and if in "reduced" output
    only print one error of each kind; this feature is useful,
    when we are interested only in the kind of errors and
    not on the number of errors.

    PRECONDITION: if s3 non-null so are s1,s2.
        If  s2 is non-null so is s1.
        s1 is always non-null. */

static void
print_dwarf_check_error(const char *s1,
    const char *s2,
    const char *s3)
{
    static Dwarf_Bool do_init = TRUE;
    Dwarf_Bool found = FALSE;
    char * error_text = NULL;
    static char *leader =  "\n*** DWARF CHECK: ";
    static char *trailer = " ***\n";

    if (do_init) {
        esb_constructor(&dwarf_error_line);
        do_init = FALSE;
    }
    esb_empty_string(&dwarf_error_line);
    esb_append(&dwarf_error_line,leader);
    if (s3) {
        esb_append(&dwarf_error_line,s1);
        esb_append(&dwarf_error_line," -> ");
        esb_append(&dwarf_error_line,s2);
        esb_append(&dwarf_error_line,": ");
        esb_append(&dwarf_error_line,s3);
    } else if (s2) {
        esb_append(&dwarf_error_line,s1);
        esb_append(&dwarf_error_line,": ");
        esb_append(&dwarf_error_line,s2);
    } else {
        esb_append(&dwarf_error_line,s1);
    }
    esb_append(&dwarf_error_line,trailer);

    error_text = esb_get_string(&dwarf_error_line);
    if (glflags.gf_print_unique_errors) {
        found = add_to_unique_errors_table(error_text);
        if (!found) {
            printf("%s",error_text);
        }
    } else {
        printf("%s",error_text);
    }

    /*  To indicate if the current error message has
        been found or not */
    glflags.gf_found_error_message = found;
}

void
DWARF_CHECK_ERROR3(Dwarf_Check_Categories category,
    const char *str1, const char *str2, const char *strexpl)
{
    if (checking_this_compiler()) {
        DWARF_ERROR_COUNT(category,1);
        if (glflags.gf_check_verbose_mode) {
            print_dwarf_check_error(str1, str2,strexpl);
        }
        /* does glflags.check_error++; */
        /* sets glflags.gf_record_dwarf_error = TRUE; */
        DWARF_CHECK_ERROR_PRINT_CU();
    }
}

/*  This is too much to put in the DROP_ERROR_INSTANCE macro,
    so we put it here rather arbitrarily.  */
void
report_caller_error_drop_error(int dwdlv,
    int line, char *fname)
{
    printf("\nERROR in dwarfdump:"
        " The value passed to the macro DROP_ERROR_INSTANCE "
        "is not one of the three allowed values, but is "
        "%d. dwarfdump has a bug. "
        " See line %d file %s\n",dwdlv,line,fname);
    glflags.gf_count_major_errors++;

}
void
dd_minimal_count_global_error(void)
{
    glflags.gf_count_major_errors++;
}
