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

#include "print_sections.h"
#include "print_frames.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

/* print data in .debug_abbrev */
extern void
print_abbrevs(Dwarf_Debug dbg)
{
    Dwarf_Abbrev ab;
    Dwarf_Unsigned offset = 0;
    Dwarf_Unsigned length = 0;
    Dwarf_Unsigned attr_count = 0;
    Dwarf_Half attr = 0;
    Dwarf_Signed form = 0;
    Dwarf_Off off = 0;
    string child_name;
    Dwarf_Unsigned abbrev_num = 1;
    Dwarf_Signed child_flag = 0;
    int abres = 0;

    cout << endl;
    cout << ".debug_abbrev" << endl;
    while ((abres = dwarf_get_abbrev(dbg, offset, &ab,
                                     &length, &attr_count,
                                     &err)) == DW_DLV_OK) {

        if (attr_count == 0) {
            /* Simple innocuous zero : null abbrev entry */
            if (dense) {
                cout << BracketSurround(IToDec(abbrev_num));
                cout << BracketSurround(IToDec(offset));
                // 0 is abbrev_code.
                cout << BracketSurround(IToDec(0));
                cout << BracketSurround("null .debug_abbrev entry") << endl;
            } else {
                cout << BracketSurround(IToDec(abbrev_num,4));
                cout << BracketSurround(IToDec(offset,5));
                // 0 is abbrev_code.
                cout << BracketSurround(string("code: ") +IToDec(0,2));
                cout << " null .debug_abbrev entry" << endl;
            }

            offset += length;
            ++abbrev_num;
            dwarf_dealloc(dbg, ab, DW_DLA_ABBREV);
            continue;
        }
        Dwarf_Half tag = 0;
        int tres = dwarf_get_abbrev_tag(ab, &tag, &err);
        if (tres != DW_DLV_OK) {
            dwarf_dealloc(dbg, ab, DW_DLA_ABBREV);
            print_error(dbg, "dwarf_get_abbrev_tag", tres, err);
        }
        Dwarf_Unsigned abbrev_code = 0;
        tres = dwarf_get_abbrev_code(ab, &abbrev_code, &err);
        if (tres != DW_DLV_OK) {
            dwarf_dealloc(dbg, ab, DW_DLA_ABBREV);
            print_error(dbg, "dwarf_get_abbrev_code", tres, err);
        }
        if (dense) {
            cout << BracketSurround(IToDec(abbrev_num));
            cout << BracketSurround(IToDec(offset));
            cout << BracketSurround(IToDec(abbrev_code));
            cout << BracketSurround(get_TAG_name(tag,
               dwarf_names_print_on_error));
        }
        else {
            cout << BracketSurround(IToDec(abbrev_num,4));
            cout << BracketSurround(IToDec(offset,5));
            cout << BracketSurround(string("code: ") +IToDec(abbrev_code,2));
            cout << " ";
            cout << LeftAlign(20,get_TAG_name(tag,
               dwarf_names_print_on_error));
        }
        ++abbrev_num;
        int acres = dwarf_get_abbrev_children_flag(ab, &child_flag, &err);
        if (acres == DW_DLV_ERROR) {
            dwarf_dealloc(dbg, ab, DW_DLA_ABBREV);
            print_error(dbg, "dwarf_get_abbrev_children_flag", acres,
                        err);
        }
        if (acres == DW_DLV_NO_ENTRY) {
            child_flag = 0;
        }
        child_name = get_children_name(child_flag,
            dwarf_names_print_on_error);
        cout << child_name;
        if (!dense) {
            cout << endl;
        }
        /* Abbrev just contains the format of a die, which debug_info
           then points to with the real data. So here we just print the 
           given format. */
        for (Dwarf_Unsigned i = 0; i < attr_count; i++) {
            int aeres =
                dwarf_get_abbrev_entry(ab, i, &attr, &form, &off, &err);
            if (aeres == DW_DLV_ERROR) {
                dwarf_dealloc(dbg, ab, DW_DLA_ABBREV);
                print_error(dbg, "dwarf_get_abbrev_entry", aeres, err);
            }
            if (aeres == DW_DLV_NO_ENTRY) {
                attr = -1LL;
                form = -1LL;
            }
            if (dense) {
                cout << " " << BracketSurround(IToDec(off));
                cout << get_AT_name(attr,dwarf_names_print_on_error);
                cout << BracketSurround(get_FORM_name((Dwarf_Half) form,
                           dwarf_names_print_on_error));
            } else {
                cout << "      " << BracketSurround(IToDec(off,5));
                cout << "\t";
                cout << LeftAlign(28,get_AT_name(
                     attr,dwarf_names_print_on_error));
                cout << get_FORM_name(form,
                           dwarf_names_print_on_error);
                cout << endl;
            }
        }
        dwarf_dealloc(dbg, ab, DW_DLA_ABBREV);
        offset += length;
        if (dense) {
            cout << endl;
        }
    }
    if (abres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_get_abbrev", abres, err);
    }
}

