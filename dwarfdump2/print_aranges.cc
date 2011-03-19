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

/* get all the data in .debug_aranges */
extern void
print_aranges(Dwarf_Debug dbg)
{
    Dwarf_Signed count = 0;
    Dwarf_Arange *arange_buf = NULL;

    cout <<  endl;
    cout << ".debug_aranges" << endl;
    int ares = dwarf_get_aranges(dbg, &arange_buf, &count, &err);
    if (ares == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_get_aranges", ares, err);
    } else if (ares == DW_DLV_NO_ENTRY) {
        /* no arange is included */
    } else {
        Dwarf_Off cu_die_offset = 0;
        Dwarf_Addr start = 0;
        Dwarf_Unsigned length = 0;
        Dwarf_Unsigned segment = 0;
        Dwarf_Unsigned segment_entry_size = 0;
        for (Dwarf_Signed i = 0; i < count; i++) {
            int aires = dwarf_get_arange_info_b(arange_buf[i],
                                          &segment,&segment_entry_size,
                                          &start, &length,
                                          &cu_die_offset, &err);
            if (aires != DW_DLV_OK) {
                print_error(dbg, "dwarf_get_arange_info", aires, err);
            } else {
                int dres;
                if (check_aranges) {
                    Dwarf_Off cuhdroff = 0;
                    Dwarf_Off cudieoff3 = 0;
                    dres = dwarf_get_arange_cu_header_offset(
                        arange_buf[i],&cuhdroff,&err);
                    if(dres == DW_DLV_OK) {
                        Dwarf_Off cudieoff2 = 0;
                        dres = dwarf_get_cu_die_offset_given_cu_header_offset(
                            dbg,cuhdroff,&cudieoff2,&err);
                        if(dres == DW_DLV_OK) {
                            if(cudieoff2 != cu_die_offset) {
                                string details = 
                                    string("cu_die offsets  mismatch,  ") 
                                    + IToHex(cu_die_offset,0) +
                                    string(" from arange data") +
                                    IToHex(cudieoff2,0);
                                DWARF_CHECK_ERROR(aranges_result,
                                    details);
                            }
                        } else {
                            print_error(dbg, 
                                "dwarf_get_cu_die_offset_given...", 
                                dres, err);
                        }
                    } else {
                        print_error(dbg, "dwarf_get_arange_cu_header_offset", 
                        dres, err);
                    }
                    dres = dwarf_get_cu_die_offset(arange_buf[i],&cudieoff3,
                        &err);
                    if(dres == DW_DLV_OK) {
                        if(dres == DW_DLV_OK) {
                            if(cudieoff3 != cu_die_offset) {
                                string details = 
                                    string("cu_die offsets (b) mismatch,  ") 
                                    + IToHex(cu_die_offset,0) +
                                    string(" from arange data") +
                                    IToHex(cudieoff3,0);
                                DWARF_CHECK_ERROR(aranges_result,
                                    details);
                            }
                        } else {
                            print_error(dbg, 
                                "dwarf_get_cu_die_offset_given...", 
                                dres, err);
                        }
                    } else {
                        print_error(dbg, "dwarf_get_cu_die_offset failed ", 
                            dres, err);
                    }
                }


                Dwarf_Die cu_die = NULL;
                dres = dwarf_offdie(dbg, cu_die_offset, &cu_die, &err);
                if (dres != DW_DLV_OK) {
                    print_error(dbg, "dwarf_offdie", dres, err);
                } else {
                    DieHolder hcu_die(dbg,cu_die);
                    if (cu_name_flag) {
                        if (should_skip_this_cu(hcu_die,err)) {
                             continue;
                        }
                    }
                    cout << endl;
                    if(segment_entry_size != 0) {
                        cout << "arange starts at seg,off" << 
                            segment << "," <<
                            IToHex(start);
                    } else {
                        cout << "arange starts at " << IToHex(start);
                    }
                    cout << ", length of " << length;
                    cout << ", cu_die_offset = " << cu_die_offset;
                    /* Get the offset of the cu header itself in the
                       section, but not for end-entries. */
                    if (start || length) {
                        Dwarf_Off off = 0;
                        int cures3 =
                            dwarf_get_arange_cu_header_offset(arange_buf[i],
                                                              &off,
                                                              &err);
                        if (cures3 != DW_DLV_OK) {
                            print_error(dbg, "dwarf_get_cu_hdr_offset",
                                        cures3, err);
                        }
                        if (verbose) {
                            cout << " cuhdr " << off;
                        }
                    }
                    cout << endl;
                    if(start || length) {
                        SrcfilesHolder hsrcfiles;
                        print_one_die(hcu_die, true,
                            /* indent_level= */0,
                            hsrcfiles,
                            /* ignore_die_printed_flag= */true);
                    }
                }
            }
            /* print associated die too? */
            dwarf_dealloc(dbg, arange_buf[i], DW_DLA_ARANGE);
        }
        dwarf_dealloc(dbg, arange_buf, DW_DLA_LIST);
    }
}

