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
#include <vector>
#include "naming.h"
#include "dwconf.h"

#include "print_frames.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

static void
print_source_intro(Dwarf_Die cu_die)
{
    Dwarf_Off off = 0;
    int ores = dwarf_dieoffset(cu_die, &off, &err);

    if (ores == DW_DLV_OK) {
        cout << "Source lines (from CU-DIE at .debug_info offset ";
        cout << off;
        cout << "):" << endl;
    } else {
        cout <<"Source lines (for the CU-DIE at unknown location):" <<
            endl;
    }
}


/*
 * Print line number information:
 *      filename
 *      new basic-block
 *      [line] [address] <new statement>
 */

void
print_line_numbers_this_cu(DieHolder & hcudie)
{
    Dwarf_Die cu_die = hcudie.die();
    Dwarf_Debug dbg = hcudie.dbg();

    cout << endl;
    cout << ".debug_line: line number info for a single cu"<< endl;
    if (verbose > 1) {
        int errcount = 0;
        print_source_intro(cu_die);
        SrcfilesHolder hsrcfiles;
        print_one_die(hcudie, /* print_information= */ 1,
             /* indent_level= */ 0,
             hsrcfiles,
             /* ignore_die_printed_flag= */true);
        lines_result.checks++;
        int lres = dwarf_print_lines(cu_die, &err,&errcount);
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
        int line_errs = 0;
        dwarf_check_lineheader(cu_die,&line_errs);
        if(line_errs > 0) {
            lines_result.errors += line_errs;
            lines_result.checks += (line_errs-1);
        }
    }
    Dwarf_Signed linecount = 0;
    Dwarf_Line *linebuf = NULL;
    int lres = dwarf_srclines(cu_die, &linebuf, &linecount, &err);
    if (lres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_srclines", lres, err);
    } else if (lres == DW_DLV_NO_ENTRY) {
        /* no line information is included */
    } else {
        print_source_intro(cu_die);
        if (verbose) {
            SrcfilesHolder hsrcfiles;
            print_one_die(hcudie, /* print_information= */ 1,
                /* indent_level= */ 0,
                hsrcfiles,
                /* ignore_die_printed_flag= */true);
        }
        cout <<
            "<source>\t[row,column]\t<pc>\t//<new statement or basic block"<<
            endl;

        for (Dwarf_Signed i = 0; i < linecount; i++) {
            Dwarf_Line line = linebuf[i];
            char *filenamearg = 0;
            string filename("<unknown>");
            int sres = dwarf_linesrc(line, &filenamearg, &err);
            if (sres == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_linesrc", sres, err);
            }
            if (sres == DW_DLV_OK) {
                filename = filenamearg;
            }
            Dwarf_Addr pc = 0;
            int ares = dwarf_lineaddr(line, &pc, &err);
            if (ares == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_lineaddr", ares, err);
            }
            if (ares == DW_DLV_NO_ENTRY) {
                pc = 0;
            }
            Dwarf_Unsigned lineno = 0;
            int lires = dwarf_lineno(line, &lineno, &err);
            if (lires == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_lineno", lires, err);
            }
            if (lires == DW_DLV_NO_ENTRY) {
                lineno = -1LL;
            }
            Dwarf_Signed column = 0;
            int cores = dwarf_lineoff(line, &column, &err);
            if (cores == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_lineoff", cores, err);
            }
            if (cores == DW_DLV_NO_ENTRY) {
                column = -1LL;
            }
            cout <<filename << ":\t[" << IToDec(lineno,3)<<
                  "," << IToDec(column,2) <<
                  "]\t" << IToHex(pc);
            if (sres == DW_DLV_OK) {
                dwarf_dealloc(dbg, filenamearg, DW_DLA_STRING);
            }
            Dwarf_Bool newstatement = 0;
            int nsres = dwarf_linebeginstatement(line, &newstatement, &err);
            if (nsres == DW_DLV_OK) {
                if (newstatement) {
                    cout << "\t// new statement";
                }
            } else if (nsres == DW_DLV_ERROR) {
                print_error(dbg, "linebeginstatment failed", nsres,
                            err);
            }
            Dwarf_Bool new_basic_block = 0;
            nsres = dwarf_lineblock(line, &new_basic_block, &err);
            if (nsres == DW_DLV_OK) {
                if (new_basic_block) {
                    cout << "\t// new basic block";
                }
            } else if (nsres == DW_DLV_ERROR) {
                print_error(dbg, "lineblock failed", nsres, err);
            }
            Dwarf_Bool lineendsequence = 0;
            nsres = dwarf_lineendsequence(line, &lineendsequence, &err);
            if (nsres == DW_DLV_OK) {
                if (lineendsequence) {
                    cout << "\t// end of text sequence";
                }
            } else if (nsres == DW_DLV_ERROR) {
                print_error(dbg, "lineblock failed", nsres, err);
            }
            cout << endl;
        }
        dwarf_srclines_dealloc(dbg, linebuf, linecount);
    }
}

