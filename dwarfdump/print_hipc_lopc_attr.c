/*
Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
Portions Copyright 2009-2018 SN Systems Ltd. All rights reserved.
Portions Copyright 2007-2020 David Anderson. All rights reserved.

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
    Free Software Foundation, Inc., 51 Franklin St, Fifth
    Floor, Boston, MA 02110-1301, USA.  SGI has moved from
    the Crittenden Lane address.  */


#include "globals.h"
#ifdef HAVE_STDINT_H
#include <stdint.h> /* For uintptr_t */
#endif /* HAVE_STDINT_H */
#include "naming.h"
#include "esb.h"                /* For flexible string buffer. */
#include "esb_using_functions.h"
#include "sanitized.h"
#include "print_frames.h"  /* for print_location_operations() . */
#include "macrocheck.h"
#include "helpertree.h"
#include "tag_common.h"

/* Is this a PU has been invalidated by the SN Systems linker? */
#define IsInvalidCode(low,high) \
    ((low == max_address) || (low == 0 && high == 0))


/*  Most types of CU can have highpc and/or lowpc.
    DW_TAG_type_unit will not. */
static Dwarf_Bool
cu_tag_type_may_have_lopc_hipc(int tag)
{
    if (tag == DW_TAG_compile_unit) {
        return TRUE;
    }
    if (tag == DW_TAG_partial_unit) {
        return TRUE;
    }
    if (tag == DW_TAG_skeleton_unit) {
        return TRUE;
    }
    return FALSE;
}

/*  This updates values in glflags used for reporting
    in error cases. */
static void
update_cu_base_addresses(Dwarf_Debug dbg,
    Dwarf_Attribute attrib,
    Dwarf_Half attr,
    Dwarf_Half tag,
    Dwarf_Half theform,
    Dwarf_Bool bSawLowp,
    Dwarf_Addr lowAddrp,
    Dwarf_Error *err)
{

    /* Update base address for CU */
    if (attr == DW_AT_low_pc) {
        if (glflags.need_CU_base_address &&
            cu_tag_type_may_have_lopc_hipc(tag)) {
            int lres = dwarf_formaddr(attrib,
                &glflags.CU_base_address, err);
            DROP_ERROR_INSTANCE(dbg,lres,*err);
            if (lres == DW_DLV_OK) {
                glflags.need_CU_base_address = FALSE;
                glflags.CU_low_address =
                    glflags.CU_base_address;
            }
        } else if (!glflags.CU_low_address) {
            /*  We take the first non-zero address
                as meaningful. Even if no such in CU DIE. */
            int fres = dwarf_formaddr(attrib,
                &glflags.CU_low_address, err);
            DROP_ERROR_INSTANCE(dbg,fres,*err);
            if (fres == DW_DLV_OK) {
                /*  Stop looking for base. Bogus, but
                    there is none available, so stop. */
                glflags.need_CU_base_address = FALSE;
            }
        }
    }

    /* Update high address for CU */
    if (attr == DW_AT_high_pc) {
        if (glflags.need_CU_high_address ) {
            /*  This is bogus in that it accepts the first
                high address in the CU, from any TAG */
            if (theform != DW_FORM_addr &&
                !dwarf_addr_form_is_indexed(theform)) {
                /*  New in DWARF4: other forms
                (of class constant) are not an address
                but are instead offset from pc. */
                Dwarf_Unsigned hpcoff = 0;
                int show_form_here = 0;

                int ares = dd_get_integer_and_name(
                    dbg,
                    attrib,
                    &hpcoff,
                    /* attrname */ (const char *) NULL,
                    /* err_string */ ( struct esb_s *) NULL,
                    (encoding_type_func) 0,
                    err,show_form_here);
                if (ares == DW_DLV_OK) {
                    if (bSawLowp) {
                        glflags.CU_high_address =
                            lowAddrp + hpcoff;
                    }
                }
            } else {
                int ares = dwarf_formaddr(attrib,
                    &glflags.CU_high_address, err);
                DROP_ERROR_INSTANCE(dbg,ares,*err);
                if (ares == DW_DLV_OK) {
                    glflags.need_CU_high_address = FALSE;
                }
            }
        }
    }
}

int
print_hipc_lopc_attribute(Dwarf_Debug dbg,
    Dwarf_Half tag,
    Dwarf_Die die,
    int die_indent_level,
    Dwarf_Unsigned dieprint_cu_goffset,
    char ** srcfiles,
    Dwarf_Signed cnt,
    Dwarf_Attribute attrib,
    Dwarf_Half attr,
    Dwarf_Unsigned max_address,
    Dwarf_Bool *bSawLowp,
    Dwarf_Addr *lowAddrp,
    Dwarf_Bool *bSawHighp,
    Dwarf_Addr *highAddrp,
    struct esb_s *valname,
    Dwarf_Error *err)
{
    Dwarf_Half theform =0;
    int rv = 0;
    /* For DWARF4, the high_pc offset from the low_pc */
    Dwarf_Unsigned highpcOff = 0;
    Dwarf_Bool offsetDetected = FALSE;
    char highpcstrbuf[ESB_FIXED_ALLOC_SIZE];
    struct esb_s highpcstr;

    esb_constructor_fixed(&highpcstr,highpcstrbuf,
        sizeof(highpcstrbuf));
    rv = dwarf_whatform(attrib,&theform,err);
    /*  Depending on the form and the attribute,
        process the form. */
    if (rv == DW_DLV_ERROR) {
        print_error_and_continue(dbg, "in print_attribute "
            "dwarf_whatform cannot"
            " Find attr form",
            rv, *err);
        return rv;
    } else if (rv == DW_DLV_NO_ENTRY) {
        return rv;
    }
    /*  Determine if the high pc is really an offset,
        set offset_detected flag if so. */
    if (theform != DW_FORM_addr &&
        !dwarf_addr_form_is_indexed(theform)) {
        /*  New in DWARF4: other forms
            (of class constant) are not an address
            but are instead offset from pc.
            One could test for DWARF4 here
            before adding this string, but that
            seems unnecessary as this
            could not happen with DWARF3 or earlier.
            A normal consumer would have to
            add this value to
            DW_AT_low_pc to get a true pc. */
        esb_append(&highpcstr,"<offset-from-lowpc>");
        /*  Update the high_pc value if we
            are checking the ranges */
        if ( glflags.gf_check_ranges && attr == DW_AT_high_pc) {
            /* Get the offset value */
            int show_form_here = 0;
            int ares = dd_get_integer_and_name(dbg,
                attrib,
                &highpcOff,
                /* attrname */ (const char *) NULL,
                /* err_string */ ( struct esb_s *) NULL,
                (encoding_type_func) 0,
                err,show_form_here);
            if (ares != DW_DLV_OK) {
                if (ares == DW_DLV_NO_ENTRY) {
                    print_error_and_continue(dbg,
                        "dd_get_integer_and_name"
                        " No Entry for DW_AT_high_pc/DW_AT_low_pc",
                        ares, *err);
                } else {
                    print_error_and_continue(dbg,
                        "dd_get_integer_and_name"
                        " Failed for DW_AT_high_pc/DW_AT_low_pc",
                        ares, *err);
                }
                return ares;
            }
            offsetDetected = TRUE;
        }
    }
    rv = get_attr_value(dbg, tag, die,
        die_indent_level,
        dieprint_cu_goffset,
        attrib, srcfiles, cnt,
        &highpcstr,glflags.show_form_used,
        glflags.verbose,err);
    if (rv == DW_DLV_ERROR) {
        return rv;
    }
    esb_empty_string(valname);
    esb_append(valname, esb_get_string(&highpcstr));
    esb_destructor(&highpcstr);

    /* Update base and high addresses for CU */
    if (glflags.seen_CU &&
        (glflags.need_CU_base_address
        || glflags.need_CU_high_address)) {
        /* updating glflags data for checking/reporting later. */
        update_cu_base_addresses(dbg,attrib,
            attr, tag, theform,
            *bSawLowp,
            *lowAddrp,
            err);
    }

    /*  Record the low and high addresses as we have them.
        Push the calculated low/high values
        back to caller using *bSawLowp, *lowAddrp,
        *bSawHighp, *highAddrp.
        For DWARF4 and later allow the high_pc value as
        an offset */
    if ((glflags.gf_check_decl_file ||
        glflags.gf_check_ranges ||
        glflags.gf_check_locations) &&
            (theform == DW_FORM_addr ||
            dwarf_addr_form_is_indexed(theform) ||
            offsetDetected)) {

        int cres = 0;
        Dwarf_Addr addr = 0;
        /* Calculate the real high_pc value */
        if (offsetDetected && glflags.seen_PU_base_address) {
            addr = *lowAddrp + highpcOff;
            cres = DW_DLV_OK;
        } else {
            if (theform == DW_FORM_addr ||
                dwarf_addr_form_is_indexed(theform)) {
                cres = dwarf_formaddr(attrib, &addr, err);
            } else {
                /* Bogus. FIXME */
                cres = DW_DLV_NO_ENTRY;
            }
        }
        if (cres == DW_DLV_OK) {
            if (attr == DW_AT_low_pc) {
                *lowAddrp = addr;
                *bSawLowp = TRUE;
                /*  Record the base address of the last seen PU
                    to be used when checking line information */
                if (glflags.seen_PU &&
                    !glflags.seen_PU_base_address) {
                    glflags.seen_PU_base_address = TRUE;
                    glflags.PU_base_address = addr;
                }
            } else { /* DW_AT_high_pc */
                *highAddrp = addr;
                *bSawHighp = TRUE;
                /*  Record the high address of the last seen PU
                    to be used when checking line information */
                if (glflags.seen_PU &&
                    !glflags.seen_PU_high_address) {
                    glflags.seen_PU_high_address = TRUE;
                    glflags.PU_high_address = addr;
                }
            }
        } else  if (cres == DW_DLV_ERROR) {
            int msgnum = dwarf_errno(*err);

            if (msgnum == DW_DLE_MISSING_NEEDED_DEBUG_ADDR_SECTION) {
                print_error_and_continue(dbg,
                    "Some checks cannot be done because "
                    "the .debug_addr section is not present",
                    cres,*err);
                DROP_ERROR_INSTANCE(dbg,cres,*err);
                return DW_DLV_OK;
            }
            return cres;
        }

        /* We have now both low_pc and high_pc values */
        if (*bSawLowp && *bSawHighp) {
            /*  We need to decide if this PU is
                valid, as the SN Linker marks a stripped
                function by setting lowpc to -1;
                also for discarded comdat, both lowpc
                and highpc are zero */
            if (glflags.need_PU_valid_code) {
                glflags.need_PU_valid_code = FALSE;
                /*  To ignore a PU as invalid code,
                    only consider the lowpc and
                    highpc values associated with the
                    DW_TAG_subprogram; other
                    instances of lowpc and highpc,
                    must be ignored (lexical blocks) */
                glflags.in_valid_code = TRUE;
                if (IsInvalidCode(*lowAddrp,*highAddrp) &&
                    tag == DW_TAG_subprogram) {
                    glflags.in_valid_code = FALSE;
                }
            }
            /*  We have a low_pc/high_pc pair;
                check if they are valid */
            if (glflags.in_valid_code) {
                DWARF_CHECK_COUNT(ranges_result,1);
                if (*lowAddrp != max_address &&
                    *lowAddrp > *highAddrp) {
                    DWARF_CHECK_ERROR(ranges_result,
                        ".debug_info: Incorrect values "
                        "for low_pc/high_pc");
                    if (glflags.gf_check_verbose_mode &&
                        PRINTING_UNIQUE) {
                        printf("Low = 0x%" DW_PR_XZEROS DW_PR_DUx
                            ", High = 0x%"
                            DW_PR_XZEROS DW_PR_DUx "\n",
                            *lowAddrp,*highAddrp);
                    }
                }
                if (glflags.gf_check_decl_file ||
                    glflags.gf_check_ranges ||
                    glflags.gf_check_locations) {
                    AddEntryIntoBucketGroup(glflags.pRangesInfo,0,
                        *lowAddrp,
                        *lowAddrp,*highAddrp,NULL,FALSE);
                }
            }
            *bSawLowp = FALSE;
            *bSawHighp = FALSE;
        }
    }
    return DW_DLV_OK;
}
