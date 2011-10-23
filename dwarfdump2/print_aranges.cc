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


static void
do_checking(Dwarf_Debug dbg, Dwarf_Arange *arange_buf,Dwarf_Signed i,
    Dwarf_Off cu_die_offset,Dwarf_Bool first_cu,
    Dwarf_Off cu_die_offset_prev, Dwarf_Die cu_die )
{
    Dwarf_Off cuhdroff = 0;
    Dwarf_Off cudieoff3 = 0;
    int dres = dwarf_get_arange_cu_header_offset(
        arange_buf[i],&cuhdroff,&err);
    if(dres == DW_DLV_OK) {
        Dwarf_Off cudieoff2 = 0;

        /* Get the CU offset for easy error reporting */
        if (first_cu || cu_die_offset != cu_die_offset_prev) {
            cu_die_offset_prev = cu_die_offset;
            dres = dwarf_die_offsets(cu_die,
                &error_message_data.DIE_overall_offset,
                &error_message_data.DIE_offset,&err);
            if (dres != DW_DLV_OK) {
                print_error(dbg, "dwarf_die_offsets", dres, err);
            }
        }
        dres = dwarf_get_cu_die_offset_given_cu_header_offset(
            dbg,cuhdroff,&cudieoff2,&err);
        if(dres == DW_DLV_OK) {
            /* Get the CU offset for easy error reporting */
            dwarf_die_offsets(cu_die,
                &error_message_data.DIE_overall_offset,
                &error_message_data.DIE_offset,&err);
            DWARF_CHECK_COUNT(aranges_result,1);
            if(cudieoff2 != cu_die_offset) {
                cout << "Error, cu_die offsets mismatch,  " <<
                    IToHex(cu_die_offset) << " != " <<
                    IToHex(cudieoff2) << " from arange data";
                DWARF_CHECK_ERROR(aranges_result,
                    " dwarf_get_cu_die_offset_given_cu..."
                    " gets wrong offset");
            }
        } else {
            print_error(dbg, "dwarf_get_cu_die_offset_given...", dres, err);
        }
    } else {
        print_error(dbg, "dwarf_get_arange_cu_header_offset", dres, err);
    }
    dres = dwarf_get_cu_die_offset(arange_buf[i],&cudieoff3,
        &err);
    if(dres == DW_DLV_OK) {
        DWARF_CHECK_COUNT(aranges_result,1);
        if(cudieoff3 != cu_die_offset) {
            cout << "Error, cu_die offsets (b) mismatch ,  "<<
                IToHex(cu_die_offset) << " != " <<
                IToHex(cudieoff3) << " from arange data";
            DWARF_CHECK_ERROR(aranges_result,
                " dwarf_get_cu_die_offset "
                " gets wrong offset");
        }
    } else {
        print_error(dbg, "dwarf_get_cu_die_offset failed ", 
            dres,err);           
    }
}

/* get all the data in .debug_aranges */
extern void
print_aranges(Dwarf_Debug dbg)
{
    Dwarf_Signed count = 0;
    Dwarf_Arange *arange_buf = NULL;
    Dwarf_Off prev_off = 0; /* Holds previous CU offset */
    bool first_cu = true;
    Dwarf_Off cu_die_offset_prev = 0;

    /* Reset the global state, so we can traverse the debug_info */
    error_message_data.seen_CU = false;
    error_message_data.need_CU_name = true;
    error_message_data.need_CU_base_address = true;
    error_message_data.need_CU_high_address = true;
 
    
    error_message_data.current_section_id = DEBUG_ARANGES;
    if (do_print_dwarf) {
        cout << endl;
        cout << ".debug_aranges" << endl;
    }
    int ares = dwarf_get_aranges(dbg, &arange_buf, &count, &err);
    if (ares == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_get_aranges", ares, err);
    } else if (ares == DW_DLV_NO_ENTRY) {
        /* no arange is included */
    } else {
        Dwarf_Unsigned segment = 0;
        Dwarf_Unsigned segment_entry_size = 0;
        Dwarf_Addr start = 0;
        Dwarf_Unsigned length = 0;
        Dwarf_Off cu_die_offset = 0;
        Dwarf_Die cu_die = 0;
        for (Dwarf_Signed i = 0; i < count; i++) {
            int aires = dwarf_get_arange_info_b(arange_buf[i],
                &segment,&segment_entry_size,
                &start, &length,
                &cu_die_offset, &err);
            if (aires != DW_DLV_OK) {
                print_error(dbg, "dwarf_get_arange_info", aires, err);
            } else {
                int dres;
                string producer_name;
                /*  Get basic locations for error reporting */
                dres = dwarf_offdie(dbg, cu_die_offset, &cu_die, &err);
                if (dres != DW_DLV_OK) {
                    print_error(dbg, "dwarf_offdie", dres, err);
                }
                DieHolder hcu_die(dbg,cu_die);

                if (cu_name_flag) {
                    if(should_skip_this_cu(hcu_die,err)) {
                        continue;
                    }
                }
                /* Get producer name for this CU and update compiler list */
                get_producer_name(hcu_die,err,producer_name);
                update_compiler_target(producer_name);
                if (!checking_this_compiler()) {
                    continue;
                }


                if (check_aranges) {
                    do_checking(dbg,arange_buf,i,cu_die_offset,first_cu,
                        cu_die_offset_prev,cu_die);
                }

                if(start || length) {
                    Dwarf_Off off = 0;
                    int cures3 = dwarf_get_arange_cu_header_offset(
                        arange_buf[i], &off, &err);
                    if (cures3 != DW_DLV_OK) {
                        print_error(dbg, "dwarf_get_cu_hdr_offset",
                            cures3, err);
                    }

                    /* Print the CU information if different.  */
                    if (prev_off != off || first_cu) {
                        first_cu = false;
                        prev_off = off;
                        /*  We are faking the indent level. We do not know
                            what level it is, really. 

                            If do_check_dwarf we do not want to do 
                            the die print call as it will do 
                            check/print we may not have asked for. 
                            And if we did ask for debug_info checks
                            this will do the checks a second time!
                            So only call print_one_die if printing.
                        */
                        if(do_print_dwarf){
                            /* There is no die if its a set-end entry */
                            SrcfilesHolder hsrcfiles;
                            print_one_die(hcu_die, 
                                /* print_information= */ 1,
                                /* indent_level = */0,
                                hsrcfiles,
                                /* ignore_die_printed_flag= */true);
                        }
                        /* Reset the state, so we can traverse the debug_info */
                        error_message_data.seen_CU = false;
                        error_message_data.need_CU_name = true;
                        if (do_print_dwarf) {
                            cout << endl;
                        }
                    }

                    if (do_print_dwarf) {
                        /* Print current aranges record */
                        if(segment_entry_size) {
                            cout << endl;
                            cout <<
                                "arange starts at seg,off " <<
                                IToHex0N(segment,10) <<
                                "," <<  IToHex0N(start,10);
                        } else {
                            cout << endl;
                            cout <<
                                "arange starts at " <<
                                IToHex0N(start,10);
                        }
                        cout << ", length of " << IToHex0N(length,10) <<
                            ", cu_die_offset = "<< IToHex0N(cu_die_offset,10);
                    }
                    if (verbose && do_print_dwarf) {
                        cout << " cuhdr "<< IToHex0N(off,10) << endl;
                    }
                } else {
                    /*  Must be a range end. We really do want to print
                        this as there is a real record here, an
                        'arange end' record. */
                    if (do_print_dwarf) {
                        cout <<  endl;
                        cout << "arange end";
                    }
                }/* end start||length test */

            } // End aires DW_DLV_OK test
            /* print associated die too? */
            dwarf_dealloc(dbg, arange_buf[i], DW_DLA_ARANGE);
        }
        dwarf_dealloc(dbg, arange_buf, DW_DLA_LIST);
    }
}

