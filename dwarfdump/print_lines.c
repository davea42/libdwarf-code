/*
  Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2009-2011 SN Systems Ltd. All rights reserved.
  Portions Copyright 2008-2015 David Anderson. All rights reserved.
  Portions Copyright 2015-2015 Google, Inc. All Rights Reserved

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

#include "globals.h"
#include "naming.h"
#include "dwconf.h"
#include "esb.h"
#include "uri.h"
#include <ctype.h>
#include <time.h>

#include "print_sections.h"

/*
    Print line number information:
        [line] [address] <new statement>
        new basic-block
        filename
*/


static void
print_source_intro(Dwarf_Die cu_die)
{
    int ores = 0;
    Dwarf_Off off = 0;

    ores = dwarf_dieoffset(cu_die, &off, &err);
    if (ores == DW_DLV_OK) {
        int lres = 0;
        const char *sec_name = 0;

        lres = dwarf_get_die_section_name_b(cu_die,
            &sec_name,&err);
        if (lres != DW_DLV_OK ||  !sec_name || !strlen(sec_name)) {
            sec_name = ".debug_info";
        }
        printf("Source lines (from CU-DIE at %s offset 0x%"
            DW_PR_XZEROS DW_PR_DUx "):\n",
            sec_name,
            (Dwarf_Unsigned) off);
    } else {
        printf("Source lines (for the CU-DIE at unknown location):\n");
    }
}

static void
record_line_error(const char *where, Dwarf_Error err)
{
    char tmp_buff[500];
    if (check_lines && checking_this_compiler()) {
        snprintf(tmp_buff, sizeof(tmp_buff),
            "Error getting line details calling %s dwarf error is %s",
            where,dwarf_errmsg(err));
        DWARF_CHECK_ERROR(lines_result,tmp_buff);
    }
}

static void
process_line_table(Dwarf_Debug dbg,
    const char *sec_name,
    Dwarf_Line *linebuf, Dwarf_Signed linecount,
    Dwarf_Bool is_logicals_table, Dwarf_Bool is_actuals_table)
{
    char *padding = 0;
    Dwarf_Signed i = 0;
    Dwarf_Addr pc = 0;
    Dwarf_Unsigned lineno = 0;
    Dwarf_Unsigned logicalno = 0;
    Dwarf_Unsigned column = 0;
    Dwarf_Unsigned call_context = 0;
    string subprog_name = 0;
    string subprog_filename = 0;
    Dwarf_Unsigned subprog_line = 0;

    Dwarf_Error err = 0;

    Dwarf_Bool newstatement = 0;
    Dwarf_Bool lineendsequence = 0;
    Dwarf_Bool new_basic_block = 0;
    int sres = 0;
    int ares = 0;
    int lires = 0;
    int cores = 0;
    Dwarf_Addr elf_max_address = 0;

    Dwarf_Bool SkipRecord = FALSE;

    current_section_id = DEBUG_LINE;
    struct esb_s lastsrc;

    /* line_flag is TRUE */
    esb_constructor(&lastsrc);

    get_address_size_and_max(dbg,0,&elf_max_address,&err);
    /* Padding for a nice layout */
    padding = line_print_pc ? "            " : "";
    if (do_print_dwarf) {
        /* Check if print of <pc> address is needed. */
        printf("\n");
        if (is_logicals_table) {
            printf("Logicals Table:\n");
            printf("%sNS new statement, PE prologue end, "
                "EB epilogue begin\n",padding);
            printf("%sDI=val discriminator value\n",
                padding);
            printf("%sCC=val context, SB=val subprogram\n",
                padding);
        } else if (is_actuals_table) {
            printf("Actuals Table:\n");
            printf("%sBB new basic block, ET end of text sequence\n"
                "%sIS=val ISA number\n",padding,padding);

        } else {
            /* Standard DWARF line table. */
            printf("%sNS new statement, BB new basic block, "
                "ET end of text sequence\n",padding);
            printf("%sPE prologue end, EB epilogue begin\n",padding);
            printf("%sIS=val ISA number, DI=val discriminator value\n",
                padding);
        }
        if (is_logicals_table || is_actuals_table) {
            printf("[ row]  ");
        }
        if (line_print_pc) {
            printf("<pc>        ");
        }
        if (is_logicals_table) {
            printf("[lno,col] NS PE EB DI= CC= SB= uri: \"filepath\"\n");
        } else if (is_actuals_table) {
            printf("[logical] BB ET IS=\n");
        } else {
            printf("[row,col] NS BB ET PE EB IS= DI= uri: \"filepath\"\n");
        }
    }
    for (i = 0; i < linecount; i++) {
        Dwarf_Line line = linebuf[i];
        string filename = 0;
        int nsres = 0;
        Dwarf_Bool found_line_error = FALSE;
        Dwarf_Bool has_is_addr_set = FALSE;
        char *where = NULL;

        if (check_decl_file && checking_this_compiler()) {
            /* A line record with addr=0 was detected */
            if (SkipRecord) {
                /* Skip records that do not have ís_addr_set' */
                ares = dwarf_line_is_addr_set(line, &has_is_addr_set, &err);
                if (ares == DW_DLV_OK && has_is_addr_set) {
                    SkipRecord = FALSE;
                }
                else {
                    /*  Keep ignoring records until we have
                        one with 'is_addr_set' */
                    continue;
                }
            }
        }

        if (check_lines && checking_this_compiler()) {
            DWARF_CHECK_COUNT(lines_result,1);
        }

        filename = "<unknown>";
        if (!is_actuals_table) {
            sres = dwarf_linesrc(line, &filename, &err);
            if (sres == DW_DLV_ERROR) {
                /* Do not terminate processing */
                where = "dwarf_linesrc()";
                record_line_error(where,err);
                found_line_error = TRUE;
            }
        }

        pc = 0;
        ares = dwarf_lineaddr(line, &pc, &err);

        if (ares == DW_DLV_ERROR) {
            /* Do not terminate processing */
            where = "dwarf_lineaddr()";
            record_line_error(where,err);
            found_line_error = TRUE;
            pc = 0;
        }
        if (ares == DW_DLV_NO_ENTRY) {
            pc = 0;
        }

        if (is_actuals_table) {
            lires = dwarf_linelogical(line, &logicalno, &err);
            if (lires == DW_DLV_ERROR) {
                /* Do not terminate processing */
                where = "dwarf_linelogical()";
                record_line_error(where,err);
                found_line_error = TRUE;
            }
            if (lires == DW_DLV_NO_ENTRY) {
                logicalno = -1LL;
            }
            column = 0;
        } else {
            lires = dwarf_lineno(line, &lineno, &err);
            if (lires == DW_DLV_ERROR) {
                /* Do not terminate processing */
                where = "dwarf_lineno()";
                record_line_error(where,err);
                found_line_error = TRUE;
            }
            if (lires == DW_DLV_NO_ENTRY) {
                lineno = -1LL;
            }
            cores = dwarf_lineoff_b(line, &column, &err);
            if (cores == DW_DLV_ERROR) {
                /* Do not terminate processing */
                where = "dwarf_lineoff()";
                record_line_error(where,err);
                found_line_error = TRUE;
            }
            if (cores == DW_DLV_NO_ENTRY) {
                /*  Zero was always the correct default, meaning
                    the left edge. DWARF2/3/4 spec sec 6.2.2 */
                column = 0;
            }
        }

        /*  Process any possible error condition, though
            we won't be at the first such error. */
        if (check_decl_file && checking_this_compiler()) {
            DWARF_CHECK_COUNT(decl_file_result,1);
            if (found_line_error) {
                DWARF_CHECK_ERROR2(decl_file_result,where,dwarf_errmsg(err));
            } else if (do_check_dwarf) {
                /*  Check the address lies with a valid [lowPC:highPC]
                    in the .text section*/
                if (IsValidInBucketGroup(pRangesInfo,pc)) {
                    /* Valid values; do nothing */
                } else {
                    /*  At this point may be we are dealing with
                        a linkonce symbol. The problem we have here
                        is we have consumed the deug_info section
                        and we are dealing just with the records
                        from the .debug_line, so no PU_name is
                        available and no high_pc. Traverse the linkonce
                        table if try to match the pc value with
                        one of those ranges.
                    */
                    if (check_lines && checking_this_compiler()) {
                        DWARF_CHECK_COUNT(lines_result,1);
                    }
                    if (FindAddressInBucketGroup(pLinkonceInfo,pc)){
                        /* Valid values; do nothing */
                    } else {
                        /*  The SN Systems Linker generates
                            line records
                            with addr=0, when dealing with linkonce
                            symbols and no stripping */
                        if (pc) {
                            char addr_tmp[100];
                            if (check_lines && checking_this_compiler()) {
                                snprintf(addr_tmp,sizeof(addr_tmp),
                                    "%s: Address"
                                    " 0x%" DW_PR_XZEROS DW_PR_DUx
                                    " outside a valid .text range",
                                    sec_name,pc);
                                DWARF_CHECK_ERROR(lines_result,
                                    addr_tmp);
                            }
                        } else {
                            SkipRecord = TRUE;
                        }
                    }
                }
                /*  Check the last record for the .debug_line,
                    the one created by DW_LNE_end_sequence,
                    is the same as the high_pc
                    address for the last known user program
                    unit (PU) */
                if ((i + 1 == linecount) &&
                    seen_PU_high_address &&
                    !is_logicals_table) {
                    /*  Ignore those PU that have been stripped
                        by the linker; their low_pc values are
                        set to -1 (snc linker only) */
                    /*  It is perfectly sensible for a compiler
                        to leave a few bytes of NOP or other stuff
                        after the last instruction in a subprogram,
                        for cache-alignment or other purposes, so
                        a mismatch here is not necessarily
                        an error.  */

                    if (check_lines && checking_this_compiler()) {
                        DWARF_CHECK_COUNT(lines_result,1);
                        if ((pc != PU_high_address) &&
                            (PU_base_address != elf_max_address)) {
                            char addr_tmp[100];
                            snprintf(addr_tmp,sizeof(addr_tmp),
                                "%s: Address"
                                " 0x%" DW_PR_XZEROS DW_PR_DUx
                                " may be incorrect"
                                " as DW_LNE_end_sequence address",
                                sec_name,pc);
                            DWARF_CHECK_ERROR(lines_result,
                                addr_tmp);
                        }
                    }
                }
            }
        }

        /* Display the error information */
        if (found_line_error || record_dwarf_error) {
            if (check_verbose_mode && PRINTING_UNIQUE) {
                /* Print the record number for better error description */
                printf("Record = %"  DW_PR_DUu
                    " Addr = 0x%" DW_PR_XZEROS DW_PR_DUx
                    " [%4" DW_PR_DUu ",%2" DW_PR_DUu "] '%s'\n",
                    i, pc,lineno,column,filename);
                /* The compilation unit was already printed */
                if (!check_decl_file) {
                    PRINT_CU_INFO();
                }
            }
            record_dwarf_error = FALSE;
            /* Due to a fatal error, skip current record */
            if (found_line_error) {
                continue;
            }
        }
        if (do_print_dwarf) {
            if (is_logicals_table || is_actuals_table) {
                printf("[%4" DW_PR_DUu "]  ", i + 1);
            }
            /* Check if print of <pc> address is needed. */
            if (line_print_pc) {
                printf("0x%" DW_PR_XZEROS DW_PR_DUx "  ", pc);
            }
            if (is_actuals_table) {
                printf("[%7" DW_PR_DUu "]", logicalno);
            } else {
                printf("[%4" DW_PR_DUu ",%2" DW_PR_DUu "]", lineno, column);
            }
        }

        if (!is_actuals_table) {
            nsres = dwarf_linebeginstatement(line, &newstatement, &err);
            if (nsres == DW_DLV_OK) {
                if (newstatement && do_print_dwarf) {
                    printf(" %s","NS");
                }
            } else if (nsres == DW_DLV_ERROR) {
                print_error(dbg, "linebeginstatment failed", nsres, err);
            }
        }

        if (!is_logicals_table) {
            nsres = dwarf_lineblock(line, &new_basic_block, &err);
            if (nsres == DW_DLV_OK) {
                if (new_basic_block && do_print_dwarf) {
                    printf(" %s","BB");
                }
            } else if (nsres == DW_DLV_ERROR) {
                print_error(dbg, "lineblock failed", nsres, err);
            }
            nsres = dwarf_lineendsequence(line, &lineendsequence, &err);
            if (nsres == DW_DLV_OK) {
                if (lineendsequence && do_print_dwarf) {
                    printf(" %s", "ET");
                }
            } else if (nsres == DW_DLV_ERROR) {
                print_error(dbg, "lineendsequence failed", nsres, err);
            }
        }

        if (do_print_dwarf) {
            Dwarf_Bool prologue_end = 0;
            Dwarf_Bool epilogue_begin = 0;
            Dwarf_Unsigned isa = 0;
            Dwarf_Unsigned discriminator = 0;
            int disres = dwarf_prologue_end_etc(line,
                &prologue_end,&epilogue_begin,
                &isa,&discriminator,&err);
            if (disres == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_prologue_end_etc() failed",
                    disres, err);
            }
            if (prologue_end && !is_actuals_table) {
                printf(" PE");
            }
            if (epilogue_begin && !is_actuals_table) {
                printf(" EB");
            }
            if (isa && !is_logicals_table) {
                printf(" IS=0x%" DW_PR_DUx, isa);
            }
            if (discriminator && !is_actuals_table) {
                printf(" DI=0x%" DW_PR_DUx, discriminator);
            }
            if (is_logicals_table) {
                call_context = 0;
                disres = dwarf_linecontext(line, &call_context, &err);
                if (disres == DW_DLV_ERROR) {
                    print_error(dbg, "dwarf_linecontext() failed",
                        disres, err);
                }
                if (call_context) {
                    printf(" CC=%" DW_PR_DUu, call_context);
                }
                subprog_name = 0;
                disres = dwarf_line_subprog(line, &subprog_name,
                    &subprog_filename, &subprog_line, &err);
                if (disres == DW_DLV_ERROR) {
                    print_error(dbg, "dwarf_line_subprog() failed",
                        disres, err);
                }
                if (subprog_name && strlen(subprog_name)) {
                    /*  We do not print an empty name.
                        Clutters things up. */
                    printf(" SB=\"%s\"", subprog_name);
                }
            }
        }

        if (!is_actuals_table) {
            if (i > 0 &&  verbose < 3  &&
                strcmp(filename,esb_get_string(&lastsrc)) == 0) {
                /* Do not print name. */
            } else {
                struct esb_s urs;
                esb_constructor(&urs);
                esb_append(&urs, " uri: \"");
                translate_to_uri(filename,&urs);
                esb_append(&urs,"\"");
                if (do_print_dwarf) {
                    printf("%s",esb_get_string(&urs));
                }
                esb_destructor(&urs);
                esb_empty_string(&lastsrc);
                esb_append(&lastsrc,filename);
            }
            if (sres == DW_DLV_OK) {
                dwarf_dealloc(dbg, filename, DW_DLA_STRING);
            }
        }

        if (do_print_dwarf) {
            printf("\n");
        }
    }
    esb_destructor(&lastsrc);
}

/* Here we test the interfaces into Dwarf_Line_Context. */
static void
print_line_context_record(Dwarf_Debug dbg,
    Dwarf_Line_Context line_context)
{
    int vres = 0;
    Dwarf_Unsigned lsecoff = 0;
    Dwarf_Unsigned version = 0;
    Dwarf_Signed count = 0;
    Dwarf_Signed i = 0;
    const char *name = 0;
    struct esb_s bufr;
    Dwarf_Small table_count = 0;

    esb_constructor(&bufr);
    printf("Line Context data\n");
    vres = dwarf_srclines_table_offset(line_context,&lsecoff,&err);
    if (vres != DW_DLV_OK) {
        print_error(dbg,"Error accessing line context"
            "Something broken.",
            vres,err);
        return;
    }
    printf(" Line Section Offset 0x%"
        DW_PR_XZEROS DW_PR_DUx "\n", lsecoff);
    vres = dwarf_srclines_version(line_context,&version,
        &table_count, &err);
    if (vres != DW_DLV_OK) {
        print_error(dbg,"Error accessing line context"
            "Something broken.",
            vres,err);
        return;
    }
    printf(" version number      0x%" DW_PR_DUx " %" DW_PR_DUu "\n",
        version,version);
    printf(" number of line tables  %d.\n", table_count);


    vres = dwarf_srclines_comp_dir(line_context,&name,&err);
    if (vres != DW_DLV_OK) {
        print_error(dbg,"Error accessing line context"
            "Something broken.",
            vres,err);
        return;
    }
    if (name) {
        printf(" Compilation directory: %s\n",name);
    } else {
        printf(" Compilation directory: <unknown no DW_AT_comp_dir>\n");
    }

    vres = dwarf_srclines_include_dir_count(line_context,&count,&err);
    if (vres != DW_DLV_OK) {
        print_error(dbg,"Error accessing line context"
            "Something broken.",
            vres,err);
        return;
    }
    printf(" include directory count 0x%"
        DW_PR_DUx " %" DW_PR_DSd "\n",
        (Dwarf_Unsigned)count,count);
    for(i = 1; i <= count; ++i) {
        vres = dwarf_srclines_include_dir_data(line_context,i,
            &name,&err);
        if (vres != DW_DLV_OK) {
            print_error(dbg,"Error accessing line context"
                "Something broken.",
                vres,err);
            return;
        }
        printf("  [%2" DW_PR_DSd "]  \"%s\"\n",i,name);
    }

    vres = dwarf_srclines_files_count(line_context,&count,&err);
    if (vres != DW_DLV_OK) {
        print_error(dbg,"Error accessing line context"
            "Something broken.",
            vres,err);
        return;
    }
    printf( " files count 0x%"
        DW_PR_DUx " %" DW_PR_DUu "\n",
        count,count);
    for(i = 1; i <= count; ++i) {
        Dwarf_Unsigned dirindex = 0;
        Dwarf_Unsigned modtime = 0;
        Dwarf_Unsigned flength = 0;

        vres = dwarf_srclines_files_data(line_context,i,
            &name,&dirindex, &modtime,&flength,&err);
        if (vres != DW_DLV_OK) {
            print_error(dbg,"Error accessing line context"
                "Something broken.",
                vres,err);
            return;
        }
        esb_empty_string(&bufr);
        if (name) {
            esb_empty_string(&bufr);
            esb_append(&bufr,"\"");
            esb_append(&bufr,name);
            esb_append(&bufr,"\"");
        } else {
            esb_append(&bufr,"<ERROR:NULL name in files list>");
        }
        printf("  [%2" DW_PR_DSd "]  %-24s ,",
            i,esb_get_string(&bufr));
        printf(" directory index  %2" DW_PR_DUu ,modtime);
        printf(",  file length %2" DW_PR_DUu ,flength);
        if (modtime) {
            time_t tt3 = (time_t)modtime;

            /* ctime supplies newline */
            printf(
                "file mod time 0x%x %s", (unsigned)tt3, ctime(&tt3));
        } else {
            printf("  file mod time 0\n");
        }
    }
    esb_destructor(&bufr);

    vres = dwarf_srclines_subprog_count(line_context,&count,&err);
    if (vres != DW_DLV_OK) {
        print_error(dbg,"Error accessing line context"
            "Something broken.",
            vres,err);
        return;
    }
    if (count == 0) {
        return;
    }
    printf(" subprograms count (experimental) 0x%"
        DW_PR_DUx " %" DW_PR_DUu "\n",
        count,count);
    for(i = 1; i <= count; ++i) {
        Dwarf_Unsigned decl_file = 0;
        Dwarf_Unsigned decl_line = 0;
        vres = dwarf_srclines_subprog_data(line_context,i,
            &name,&decl_file, &decl_line,&err);
        if (vres != DW_DLV_OK) {
            print_error(dbg,"Error accessing line context"
                "Something broken.",
                vres,err);
            return;
        }
        printf("  [%2" DW_PR_DSd "]  \"%s\""
            ", fileindex %2" DW_PR_DUu
            ", lineindex  %2" DW_PR_DUu
            "\n",
            i,name,decl_file,decl_line);
    }
}

extern void
print_line_numbers_this_cu(Dwarf_Debug dbg, Dwarf_Die cu_die)
{
    Dwarf_Unsigned lineversion = 0;
    Dwarf_Signed linecount = 0;
    Dwarf_Line *linebuf = NULL;
    Dwarf_Signed linecount_actuals = 0;
    Dwarf_Line *linebuf_actuals = NULL;
    Dwarf_Small  table_count = 0;
    int lres = 0;
    int line_errs = 0;
    Dwarf_Line_Context line_context = 0;
    const char *sec_name = 0;

    current_section_id = DEBUG_LINE;

    /* line_flag is TRUE */


    lres = dwarf_get_line_section_name_from_die(cu_die,
        &sec_name,&err);
    if (lres != DW_DLV_OK || !sec_name || !strlen(sec_name)) {
        sec_name = ".debug_line";
    }
    if (do_print_dwarf) {
        printf("\n%s: line number info for a single cu\n", sec_name);
    } else {
        /* We are checking, not printing. */
        Dwarf_Half tag = 0;
        int tres = dwarf_tag(cu_die, &tag, &err);
        if (tres != DW_DLV_OK) {
            /*  Something broken here. */
            print_error(dbg,"Unable to see CU DIE tag "
                "though we could see it earlier. Something broken.",
                tres,err);
            return;
        } else if (tag == DW_TAG_type_unit) {
            /*  Not checking since type units missing
                address or range in CU header. */
            return;
        }
    }

    if (verbose > 1) {
        int errcount = 0;
        print_source_intro(cu_die);
        print_one_die(dbg, cu_die,
            /* print_information= */ 1,
            /* indent level */0,
            /* srcfiles= */ 0, /* cnt= */ 0,
            /* ignore_die_stack= */TRUE);
        DWARF_CHECK_COUNT(lines_result,1);
        lres = dwarf_print_lines(cu_die, &err,&errcount);
        if (errcount > 0) {
            DWARF_ERROR_COUNT(lines_result,errcount);
            DWARF_CHECK_COUNT(lines_result,(errcount-1));
        }
        if (lres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_srclines details", lres, err);
        }
        return;
    }

    if (check_lines && checking_this_compiler()) {
        DWARF_CHECK_COUNT(lines_result,1);
        dwarf_check_lineheader(cu_die,&line_errs);
        if (line_errs > 0) {
            DWARF_CHECK_ERROR_PRINT_CU();
            DWARF_ERROR_COUNT(lines_result,line_errs);
            DWARF_CHECK_COUNT(lines_result,(line_errs-1));
        }
    }
    /*  The following is complicated by a desire to test
        various line table interface functions.  Hence
        we test line_flag_selection.

        Normal code should pick an interface
        (for most  the best choice is what we here call
        line_flag_selection == std)
        and use just that interface set.

        Sorry about the length of the code that
        results from having so many interfaces.  */
    if (line_flag_selection == std) {
        lres = dwarf_srclines_b(cu_die,&lineversion,
            &table_count,&line_context,
            &err);
        if(lres == DW_DLV_OK) {
            lres = dwarf_srclines_from_linecontext(line_context,
                &linebuf, &linecount,&err);
        }
    } else if (line_flag_selection == orig) {
        /* DWARF2,3,4, ok for 5. */
        /* Useless for experimental line tables */
        lres = dwarf_srclines(cu_die,
            &linebuf, &linecount, &err);
        if(lres == DW_DLV_OK && linecount ){
            table_count++;
        }
    } else if (line_flag_selection == orig2l) {
        lres = dwarf_srclines_two_level(cu_die,
            &lineversion,
            &linebuf, &linecount,
            &linebuf_actuals, &linecount_actuals,
            &err);
        if(lres == DW_DLV_OK && linecount){
            table_count++;
        }
        if(lres == DW_DLV_OK && linecount_actuals){
            table_count++;
        }
    } else if (line_flag_selection == s2l) {
        lres = dwarf_srclines_b(cu_die,&lineversion,
            &table_count,&line_context,
            &err);
        if(lres == DW_DLV_OK) {
            lres = dwarf_srclines_two_level_from_linecontext(line_context,
                &linebuf, &linecount,
                &linebuf_actuals, &linecount_actuals,
                &err);
        }
    }
    if (lres == DW_DLV_ERROR) {
        /* Do not terminate processing */
        if (check_decl_file) {
            DWARF_CHECK_COUNT(decl_file_result,1);
            DWARF_CHECK_ERROR2(decl_file_result,"dwarf_srclines",
                dwarf_errmsg(err));
            record_dwarf_error = FALSE;  /* Clear error condition */
        } else {
            print_error(dbg, "dwarf_srclines", lres, err);
        }
    } else if (lres == DW_DLV_NO_ENTRY) {
        /* no line information is included */
    } else if (table_count > 0) {
        /* DW_DLV_OK */
        if(line_context && verbose) {
            print_line_context_record(dbg,line_context);
        }
        if (do_print_dwarf) {
            print_source_intro(cu_die);
            if (verbose) {
                print_one_die(dbg, cu_die,
                    /* print_information= */ TRUE,
                    /* indent_level= */ 0,
                    /* srcfiles= */ 0, /* cnt= */ 0,
                    /* ignore_die_stack= */TRUE);
            }
        }
        if(line_flag_selection == std || line_flag_selection == s2l) {
            if (table_count == 0 || table_count == 1) {
                /* ASSERT: is_single_table == true */
                Dwarf_Bool is_logicals = FALSE;
                Dwarf_Bool is_actuals = FALSE;
                process_line_table(dbg,sec_name, linebuf, linecount,
                    is_logicals,is_actuals);
            } else {
                Dwarf_Bool is_logicals = TRUE;
                Dwarf_Bool is_actuals = FALSE;
                process_line_table(dbg,sec_name, linebuf, linecount,
                    is_logicals, is_actuals);
                process_line_table(dbg,sec_name, linebuf_actuals,
                    linecount_actuals,
                    !is_logicals, !is_actuals);
            }
            dwarf_srclines_dealloc_b(line_context);
        } else if (line_flag_selection == orig) {
            Dwarf_Bool is_logicals = FALSE;
            Dwarf_Bool is_actuals = FALSE;
            process_line_table(dbg,sec_name, linebuf, linecount,
                is_logicals, is_actuals);
            dwarf_srclines_dealloc(dbg,linebuf,linecount);
        } else if (line_flag_selection == orig2l) {
            if (table_count == 1 || table_count == 0) {
                Dwarf_Bool is_logicals = FALSE;
                Dwarf_Bool is_actuals = FALSE;
                process_line_table(dbg,sec_name, linebuf, linecount,
                    is_logicals, is_actuals);
            } else {
                Dwarf_Bool is_logicals = TRUE;
                Dwarf_Bool is_actuals = FALSE;
                process_line_table(dbg,sec_name, linebuf, linecount,
                    is_logicals, is_actuals);
                process_line_table(dbg,sec_name, linebuf_actuals, linecount_actuals,
                    !is_logicals, !is_actuals);
            }
            dwarf_srclines_dealloc(dbg,linebuf,linecount);
        }
        /* end, table_count > 0 */
    } else {
        /* DW_DLV_OK */
        /*  table_count == 0. no lines in table.
            Just a line table header. */
        if (do_print_dwarf) {
            int ores = 0;
            Dwarf_Unsigned off = 0;

            print_source_intro(cu_die);
            if (verbose) {
                print_one_die(dbg, cu_die,
                    /* print_information= */ TRUE,
                    /* indent_level= */ 0,
                    /* srcfiles= */ 0, /* cnt= */ 0,
                    /* ignore_die_stack= */TRUE);
            }
            if(line_context) {
                if (verbose > 2) {
                    print_line_context_record(dbg,line_context);
                }
                ores = dwarf_srclines_table_offset(line_context,
                    &off,&err);
                if (ores != DW_DLV_OK) {
                    print_error(dbg,"dwarf_srclines_table_offset fail",ores,err);
                } else {
                    printf(" Line table is present (offset 0x%"
                        DW_PR_XZEROS DW_PR_DUx
                        ") but no lines present\n", off);
                }
            } else {
                printf(" Line table is present but no lines present\n");
            }
        }
        if(line_flag_selection == std ||
            line_flag_selection == s2l) {
            dwarf_srclines_dealloc_b(line_context);
        } else {
            /* Original allocation. */
            dwarf_srclines_dealloc(dbg,linebuf,linecount);
        }
        /* end, linecounttotal == 0 */
    }
}
