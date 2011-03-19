/* 
  Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2009-2010 SN Systems Ltd. All rights reserved.
  Portions Copyright 2008-2010 David Anderson. All rights reserved.

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



$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/print_sections.c,v 1.69 2006/04/17 00:09:56 davea Exp $ */
/* The address of the Free Software Foundation is
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA.  
 * SGI has moved from the Crittenden Lane address.
 */

#include "globals.h"
#include "naming.h"
#include "dwconf.h"
#include "esb.h"

#include "print_sections.h"

/*
 * Print line number information:
 *      filename
 *      new basic-block
 *      [line] [address] <new statement>
 */


static void
print_source_intro(Dwarf_Die cu_die)
{
    Dwarf_Off off = 0;
    int ores = dwarf_dieoffset(cu_die, &off, &err);

    if (ores == DW_DLV_OK) {
        printf
            ("Source lines (from CU-DIE at .debug_info offset %" DW_PR_DUu "):\n",
             (Dwarf_Unsigned) off);
    } else {
        printf("Source lines (for the CU-DIE at unknown location):\n");
    }
}


extern void
print_line_numbers_this_cu(Dwarf_Debug dbg, Dwarf_Die cu_die)
{
    Dwarf_Signed linecount = 0;
    Dwarf_Line *linebuf = NULL;
    Dwarf_Signed i = 0;
    Dwarf_Addr pc = 0;
    Dwarf_Unsigned lineno = 0;
    Dwarf_Signed column = 0;
    string filename;
    Dwarf_Bool newstatement = 0;
    Dwarf_Bool lineendsequence = 0;
    Dwarf_Bool new_basic_block = 0;
    int lres = 0;
    int sres = 0;
    int ares = 0;
    int lires = 0;
    int cores = 0;
    int line_errs = 0;

    printf("\n.debug_line: line number info for a single cu\n");
    if (verbose > 1) {
        int errcount = 0;
        print_source_intro(cu_die);
        print_one_die(dbg, cu_die,
            /* print_information= */ 1,
            /* indent level */0, 
            /* srcfiles= */ 0, /* cnt= */ 0, 
            /* ignore_die_stack= */TRUE);
        lines_result.checks++;
        lres = dwarf_print_lines(cu_die, &err,&errcount);
        if(errcount > 0) {
             lines_result.errors += errcount;
             lines_result.checks += (errcount-1);
        }
        if (lres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_srclines details", lres, err);
        }
        return;
    }
    if(check_lines) {
        lines_result.checks++;
        dwarf_check_lineheader(cu_die,&line_errs);
        if(line_errs > 0) {
             lines_result.errors += line_errs;
             lines_result.checks += (line_errs-1);
        }
    }
    lres = dwarf_srclines(cu_die, &linebuf, &linecount, &err);
    if (lres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_srclines", lres, err);
    } else if (lres == DW_DLV_NO_ENTRY) {
        /* no line information is included */
    } else {
        print_source_intro(cu_die);
        if (verbose) {
            print_one_die(dbg, cu_die, 
                /* print_information= */ TRUE,
                /* indent_level= */ 0,
                /* srcfiles= */ 0, /* cnt= */ 0, 
                /* ignore_die_stack= */TRUE);
        }
        printf
            ("<source>\t[row,column]\t<pc>\t//<new statement or basic block\n");

        for (i = 0; i < linecount; i++) {
            Dwarf_Line line = linebuf[i];
            int nsres;

            sres = dwarf_linesrc(line, &filename, &err);
            ares = dwarf_lineaddr(line, &pc, &err);
            if (sres == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_linesrc", sres, err);
            }
            if (sres == DW_DLV_NO_ENTRY) {
                filename = "<unknown>";
            }
            if (ares == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_lineaddr", ares, err);
            }
            if (ares == DW_DLV_NO_ENTRY) {
                pc = 0;
            }
            lires = dwarf_lineno(line, &lineno, &err);
            if (lires == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_lineno", lires, err);
            }
            if (lires == DW_DLV_NO_ENTRY) {
                lineno = -1LL;
            }
            cores = dwarf_lineoff(line, &column, &err);
            if (cores == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_lineoff", cores, err);
            }
            if (cores == DW_DLV_NO_ENTRY) {
                column = -1LL;
            }
            printf("%s:\t[%3" DW_PR_DUu ",%2" DW_PR_DSd "]\t0x%" DW_PR_DUx ,
                filename, lineno,
                column, (Dwarf_Unsigned)pc);
            if (sres == DW_DLV_OK)
                dwarf_dealloc(dbg, filename, DW_DLA_STRING);

            nsres = dwarf_linebeginstatement(line, &newstatement, &err);
            if (nsres == DW_DLV_OK) {
                if (newstatement) {
                    printf("\t// new statement");
                }
            } else if (nsres == DW_DLV_ERROR) {
                print_error(dbg, "linebeginstatment failed", nsres,
                            err);
            }
            nsres = dwarf_lineblock(line, &new_basic_block, &err);
            if (nsres == DW_DLV_OK) {
                if (new_basic_block) {
                    printf("\t// new basic block");
                }
            } else if (nsres == DW_DLV_ERROR) {
                print_error(dbg, "lineblock failed", nsres, err);
            }
            nsres = dwarf_lineendsequence(line, &lineendsequence, &err);
            if (nsres == DW_DLV_OK) {
                if (lineendsequence) {
                    printf("\t// end of text sequence");
                }
            } else if (nsres == DW_DLV_ERROR) {
                print_error(dbg, "lineblock failed", nsres, err);
            }
            printf("\n");

        }
        dwarf_srclines_dealloc(dbg, linebuf, linecount);
    }
}

