/*
  Copyright 2018-2018 David Anderson. All rights reserved.

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

/*  The address of the Free Software Foundation is
    Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
    Boston, MA 02110-1301, USA.
    SGI has moved from the Crittenden Lane address.
*/

#include "globals.h"
#include "esb.h"

#define TRUE  1
#define FALSE 0



void get_true_section_name(Dwarf_Debug dbg,
    const char *standard_name,
    struct esb_s *name_out,
    Dwarf_Bool add_compr)
{
    Dwarf_Small marked_compressed = 0;
    Dwarf_Small marked_shf_compressed = 0;
    const char *stdname =  standard_name;
    const char *actualname = 0;
    int cres = 0;
    Dwarf_Error tnameerr = 0;

    cres = dwarf_get_real_section_name(dbg,
            stdname, &actualname, &marked_compressed,
            &marked_shf_compressed,&tnameerr);
    if (cres == DW_DLV_OK) {
        esb_append(name_out,actualname);
        if (add_compr) {
            if (marked_compressed) {
                esb_append(name_out," compressed");
            }
            if (marked_shf_compressed) {
                esb_append(name_out," SHF_COMPRESSED");
            }
        }
        return;
    } else if (cres == DW_DLV_NO_ENTRY) {
        esb_append(name_out,stdname);
        return;
    }  
    /* DW_DLV_ERROR */
    esb_append(name_out,stdname);
    /* avoid error leak */
    dwarf_dealloc(dbg,tnameerr,DW_DLA_ERROR); 
    tnameerr = 0;
    esb_append(name_out," (Error accessing section name)");
    return;
}
