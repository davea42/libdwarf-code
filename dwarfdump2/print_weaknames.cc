/* 
  Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2009-2010 SN Systems Ltd. All rights reserved.
  Portions Copyright 2008-2011 David Anderson. All rights reserved.

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

#include "print_sections.h"
#include "print_frames.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

/* Get all the data in .debug_weaknames */
extern void
print_weaknames(Dwarf_Debug dbg)
{
    Dwarf_Weak *weaknamebuf = NULL;
    Dwarf_Signed count = 0;
    Dwarf_Signed i = 0;
    Dwarf_Off die_off = 0;
    Dwarf_Off cu_off = 0;
    int wkres = 0;

    error_message_data.current_section_id = DEBUG_WEAKNAMES;
    if (!do_print_dwarf) {
        return;
    }
    cout << endl;
    cout << ".debug_weaknames" << endl;
    wkres = dwarf_get_weaks(dbg, &weaknamebuf, &count, &err);
    if (wkres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_get_weaks", wkres, err);
    } else if (wkres == DW_DLV_NO_ENTRY) {
        /* no weaknames */
    } else {
        Dwarf_Unsigned maxoff = get_info_max_offset(dbg);

        for (i = 0; i < count; i++) {
            int tnres;
            int cures3;
            char *name = NULL;

            Dwarf_Unsigned global_cu_off = 0;

            tnres = dwarf_weak_name_offsets(weaknamebuf[i],
                &name, &die_off, &cu_off,
                &err);
            deal_with_name_offset_err(dbg,
                "dwarf_weak_name_offsets",
                name, die_off, tnres, err);

            cures3 = dwarf_weak_cu_offset(weaknamebuf[i],
                &global_cu_off, &err);

            if (cures3 != DW_DLV_OK) {
                print_error(dbg, "dwarf_weakname_cu_offset",
                    cures3, err);
            }
            print_pubname_style_entry(dbg,
                "weakname",
                name, die_off, cu_off,
                global_cu_off, maxoff);

            /* print associated die too? */
        }
        dwarf_weaks_dealloc(dbg, weaknamebuf, count);
    }
}   /* print_weaknames() */

