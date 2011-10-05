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
/*  The address of the Free Software Foundation is
    Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
    Boston, MA 02110-1301, USA.  
    SGI has moved from the Crittenden Lane address.
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

/* print data in .debug_loc 
   There is no guarantee this will work because we are assuming
   that all bytes are valid loclist data, that there are no
   odd padding or garbage bytes.  In normal use one gets
   into here via an offset from .debug_info, so it could be
   that bytes not referenced from .debug_info are garbage
   or even zero padding.  So this can fail (error off) as such bytes
   can lead dwarf_get_loclist_entry() astray.

   It is also broken because we do not know what CU is involved
   so if address_size varies by CU we cannot know that here.
*/

void
print_locs(Dwarf_Debug dbg)
{
    Dwarf_Unsigned offset = 0;
    Dwarf_Addr hipc_offset = 0;
    Dwarf_Addr lopc_offset = 0;
    Dwarf_Ptr data = 0;
    Dwarf_Unsigned entry_len = 0;
    Dwarf_Unsigned next_entry = 0;
    int index = 0;
    int lres = 0;
    Dwarf_Half address_size = 0;

    error_message_data.current_section_id = DEBUG_LOC;
    if(!do_print_dwarf) {
        return;
    }

    /* This is sometimes wrong, we need a frame-specific size. */
    int fres = dwarf_get_address_size(dbg, &address_size, &err);
    if (fres != DW_DLV_OK) { 
        print_error(dbg, "dwarf_get_address_size", fres, err);
    }


    cout << endl;
    cout << ".debug_loc" << endl; 
    cout <<"Format <i o b e l>: "
        "index section-offset begin-addr end-addr length-of-block-entry";
    cout << endl;
    while ((lres = dwarf_get_loclist_entry(dbg, offset,
        &hipc_offset, &lopc_offset,
        &data, &entry_len,
        &next_entry,
        &err)) == DW_DLV_OK) {

        string exprstring;
        get_string_from_locs(dbg,data,entry_len,address_size,exprstring);
        if( display_offsets) {
            ++index;
            cout <<" <iobel> [" << IToDec(index,8);
            cout <<"] " << IToHex0N(offset,10);
            // We print this offset so it matches what the debug_info
            // loclist offset shows (so we can relate them).
            // This offset is the offset of the expression byte blob.
            if(verbose) {
                cout << string(" ") <<
                    BracketSurround(string("expr-off ") +
                        IToHex0N(next_entry - entry_len,10));
            }
        }
        cout <<" "<< IToHex0N(lopc_offset,10);
        cout <<" "<< IToHex0N(hipc_offset,10);
        cout <<" "<< IToDec(entry_len,8);
        cout <<" "<< exprstring;
        cout << endl;
        offset = next_entry;
    }
    if (lres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_get_loclist_entry", lres, err);
    }
}

