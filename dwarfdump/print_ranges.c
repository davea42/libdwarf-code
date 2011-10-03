/* 
  Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2009-2011 SN Systems Ltd. All rights reserved.
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
#include "naming.h"
#include "dwconf.h"
#include "esb.h"
#include "print_sections.h"

static struct esb_s esb_string;

/* Because we do not know what DIE is involved, if the
   object being printed has different address sizes 
   in different compilation units this will not work
   properly: anything could happen. */
extern void
print_ranges(Dwarf_Debug dbg)
{
    Dwarf_Unsigned off = 0;
    int group_number = 0;
    int wasdense = 0;

    current_section_id = DEBUG_RANGES;
    if (!do_print_dwarf) {
        return;
    }
    printf("\n.debug_ranges\n");

    /*  Turn off dense, we do not want  print_ranges_list_to_extra
        to use dense form here. */
    wasdense = dense;
    dense = 0;
    for(;;) {
        Dwarf_Ranges *rangeset = 0;
        Dwarf_Signed rangecount = 0;
        Dwarf_Unsigned bytecount = 0;

        /*  We do not know what DIE is involved, we use
            the older call here. */
        int rres = dwarf_get_ranges(dbg,off,&rangeset,
            &rangecount,&bytecount,&err);
        if(rres == DW_DLV_OK) {
            char *val = 0;
            printf(" Ranges group %d:\n",group_number);
            esb_empty_string(&esb_string);
            print_ranges_list_to_extra(dbg,off,
                rangeset,rangecount,bytecount,
                &esb_string);
            dwarf_ranges_dealloc(dbg,rangeset,rangecount);
            val = esb_get_string(&esb_string);
            printf("%s",val);
            ++group_number;
        } else if (rres == DW_DLV_NO_ENTRY) {
            printf("End of .debug_ranges.\n");
            break;
        } else { 
            /*  ERROR, which does not quite mean a real error,
                as we might just be misaligned reading things without
                a DW_AT_ranges offset.*/
            printf("End of .debug_ranges..\n");
            break;
        }
        off += bytecount;
    }
    dense = wasdense;
}
