/*
Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
Portions Copyright 2009-2010 SN Systems Ltd. All rights reserved.
Portions Copyright 2008-2011 David Anderson. All rights reserved.

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

#include <config.h>

#include "dwarf.h"
#include "libdwarf.h"
#include "dd_globals.h"
#include "dd_naming.h"
#include "dd_esb.h"
#include "dd_esb_using_functions.h"
#include "print_sections.h"
#include "print_frames.h"
#include "dd_sanitized.h"

/* Get all the data in .debug_typenames or debug_pubtypes. */
int
print_types(Dwarf_Debug dbg, enum type_type_e type_type,
    Dwarf_Error *err)
{
    Dwarf_Type *globbuf = 0;
    Dwarf_Signed count = 0;
    char *name = 0;
    int gtres = 0;
    struct esb_s truename;
    char buf[DWARF_SECNAME_BUFFER_SIZE];
    struct esb_s sanitname;
    int (*get_types) (Dwarf_Debug, Dwarf_Type **, Dwarf_Signed *,
        Dwarf_Error *) = 0;
    void (*dealloctype) (Dwarf_Debug, Dwarf_Type *, Dwarf_Signed) =
        NULL;
    const char *linetitle = 0;
    /* We will, now only list either section when there is content. */

    if (type_type == DWARF_PUBTYPES) {
        name = ".debug_pubtypes";
        get_types = dwarf_get_pubtypes;
        dealloctype = dwarf_pubtypes_dealloc;
        linetitle = "pubtype";
    } else {
        /* SGI_TYPENAME */
        /*  No need to get the real section name, this
            section not used in modern compilers. */
        name = ".debug_typenames";
        get_types = dwarf_get_types;
        dealloctype = dwarf_types_dealloc;
        linetitle = "type";
    }
    esb_constructor_fixed(&truename,buf,sizeof(buf));
    get_true_section_name(dbg,name,
        &truename,TRUE);
    {
        esb_constructor(&sanitname);
        /*  Sanitized cannot be safely reused,
            there is a static buffer,
            so we make a safe copy. */
        esb_append(&sanitname,sanitized(esb_get_string(&truename)));
    }
    esb_destructor(&truename);
    if (glflags.verbose) {
        dwarf_return_empty_pubnames(dbg,1);
    }
    gtres = get_types(dbg, &globbuf, &count, err);
    if (gtres == DW_DLV_ERROR) {
        printf("\n%s\n",esb_get_string(&sanitname));
        esb_destructor(&sanitname);
        return gtres;
    } else if (gtres == DW_DLV_NO_ENTRY) {
        /* no types */
        esb_destructor(&sanitname);
        dwarf_return_empty_pubnames(dbg,0);
        return gtres;
    } else {
        int wkres = 0;
        int printed = FALSE;

        if (glflags.gf_do_print_dwarf && count > 0) {
            printf("\n%s\n",esb_get_string(&sanitname));
            printed = TRUE;
        }
        wkres = print_all_pubnames_style_records(dbg,
            linetitle,
            esb_get_string(&sanitname),
            (Dwarf_Global *)globbuf, count,
            err);
        dealloctype(dbg, globbuf, count);
        if (wkres == DW_DLV_ERROR) {
            if (!printed) {
                printf("\n%s\n",esb_get_string(&sanitname));
            }
            dwarf_return_empty_pubnames(dbg,0);
            esb_destructor(&sanitname);
            return wkres;
        }
    }
    esb_destructor(&sanitname);
    dwarf_return_empty_pubnames(dbg,0);
    return DW_DLV_OK;
}   /* print_types() */
