/*
Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
Portions Copyright 2009-2011 SN Systems Ltd. All rights reserved.
Portions Copyright 2008-2020 David Anderson. All rights reserved.

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

#include <string.h> /* strlen() */

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "dd_globals.h"
#include "dd_naming.h"
#include "dd_esb.h"
#include "dd_esb_using_functions.h"
#include "print_sections.h"
#include "dd_sanitized.h"

/*  This unifies the code for some error checks to
    avoid code duplication.
*/
static void
check_info_offset_sanity(
    const char *sec,
    const char *field,
    char *global,
    Dwarf_Unsigned offset, Dwarf_Unsigned maxoff)
{
    if (maxoff == 0) {
        /* Lets make a heuristic check. */
        if (offset > 0xffffffff) {
            printf("Warning: section %s %s %s offset 0x%"
                DW_PR_XZEROS DW_PR_DUx " "
                "exceptionally large \n",
                sec, field, global, offset);
        }
        return;
    }
    if (offset >= maxoff) {
        printf("Warning: section %s %s %s offset 0x%"
            DW_PR_XZEROS  DW_PR_DUx " "
                "larger than max of 0x%" DW_PR_DUx "\n",
                sec, field, global,  offset, maxoff);
    }
}

/* Unified pubnames style output.
   The error checking here against maxoff may be useless
   (in that libdwarf may return an error if the offset is bad
   and we will not get called here).
   But we leave it in nonetheless as it looks sensible.
   In at least one gigantic executable such offsets turned out wrong.
*/
static int
print_pubname_style_entry(Dwarf_Debug dbg,
    const char *line_title,
    char *name,
    Dwarf_Unsigned die_off,
    Dwarf_Unsigned cu_die_off,
    Dwarf_Unsigned global_cu_offset,
    Dwarf_Unsigned maxoff,Dwarf_Error *err)
{
    Dwarf_Die die = NULL;
    Dwarf_Off die_CU_off = 0;
    int dres = 0;
    int ddres = 0;
    int cudres = 0;
    Dwarf_Bool is_info = TRUE;

    /*  get die at die_off  For this old section
        in DWARF only .debug_info could be appropriate,
        never .debug_types. Hence is_info = TRUE */
    dres = dwarf_offdie_b(dbg, die_off, is_info, &die, err);
        /*  Some llvm version puts the global die offset into
            pubnames with the result that
            we will get an error here but we just
            let that create an error,
            papering it over here by subtracting out
            the applicable debug_info CU header offset
            is problematic. */
    if (dres != DW_DLV_OK) {
        struct esb_s details;
        esb_constructor(&details);
        esb_append(&details,line_title);
        esb_append(&details," dwarf_offdie : "
            "die offset does not reference valid DIE ");
        esb_append_printf_u(&details,"at offset 0x%"
            DW_PR_DUx, die_off);
        esb_append(&details,".");
        print_error_and_continue(dbg,
            esb_get_string(&details), dres, *err);
        esb_destructor(&details);
        return dres;
    }

    /* get offset of die from its cu-header */
    ddres = dwarf_die_CU_offset(die, &die_CU_off, err);
    if (ddres != DW_DLV_OK) {
        struct esb_s details;
        esb_constructor(&details);
        esb_append(&details,line_title);
        esb_append(&details," cannot get CU die offset");
        print_error_and_continue(dbg,
            esb_get_string(&details), dres, *err);
        esb_destructor(&details);
        die_CU_off = 0;
        dwarf_dealloc(dbg, die, DW_DLA_DIE);
        return ddres;
    }

    /* Get die at offset cu_die_off to check its existence. */
    {
        Dwarf_Die cu_die = NULL;
        cudres = dwarf_offdie_b(dbg, cu_die_off,is_info,
            &cu_die, err);
        if (cudres != DW_DLV_OK) {
            struct esb_s details;

            dwarf_dealloc(dbg, die, DW_DLA_DIE);
            esb_constructor(&details);
            esb_append(&details,line_title);
            esb_append(&details," dwarf_offdie: "
                "cu die offset  does not reference valid CU DIE.  ");
            esb_append_printf_u(&details,"0x%"  DW_PR_DUx,
                cu_die_off);
            esb_append(&details,".");
            /* does not return */
            print_error_and_continue(dbg,
                esb_get_string(&details), cudres, *err);
            esb_destructor(&details);
            return cudres;
        } else {
            /* It exists, all is well. */
            dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
        }
    }
    /* Display offsets */
    if (glflags.gf_display_offsets) {
        /* Print 'name' at the end for better layout */
        printf("%s die-in-sect 0x%" DW_PR_XZEROS DW_PR_DUx
            ", cu-in-sect 0x%" DW_PR_XZEROS DW_PR_DUx ","
            " die-in-cu 0x%" DW_PR_XZEROS DW_PR_DUx
            ", cu-header-in-sect 0x%" DW_PR_XZEROS DW_PR_DUx ,
            line_title,
            die_off, cu_die_off,
            (Dwarf_Unsigned) die_CU_off,
            /*  Following is absolute offset of the
                beginning of the cu */
            (Dwarf_Signed) (die_off - die_CU_off));
    }

    if ((die_off - die_CU_off) != global_cu_offset) {
        struct esb_s details;

        dwarf_dealloc(dbg, die, DW_DLA_DIE);
        esb_constructor(&details);
        esb_append(&details,"\nERROR: ");
        esb_append(&details,line_title);
        esb_append(&details,"has improper die offset:");
        esb_append_printf_u(&details," global cu offset 0x%x ",
            global_cu_offset);
        esb_append_printf_u(&details," does not match die_off 0x%x ",
            die_off);
        esb_append_printf_u(&details," minus die_CU_off  0x%x ",
            die_CU_off);
        esb_append(&details,"\n.");
        simple_err_return_msg_either_action(DW_DLV_ERROR,
            esb_get_string(&details));
        return DW_DLV_NO_ENTRY;
    }

    /* Display offsets */
    if (glflags.gf_display_offsets && glflags.verbose) {
        printf(" cuhdr 0x%" DW_PR_XZEROS DW_PR_DUx ,
            global_cu_offset);
    }

    /* Print 'name' at the end for better layout */
    printf(" '%s'\n",name);
    dwarf_dealloc(dbg, die, DW_DLA_DIE);
    check_info_offset_sanity(line_title,
        "die offset", name, die_off, maxoff);
    check_info_offset_sanity(line_title,
        "die cu offset", name, die_CU_off, maxoff);
    check_info_offset_sanity(line_title,
        "cu offset", name,
        (die_off - die_CU_off), maxoff);
    return DW_DLV_OK;
}

static void
print_globals_header(
    Dwarf_Off      pub_section_hdr_offset,
    Dwarf_Unsigned length_size, /* from pubnames header */
    Dwarf_Unsigned length, /* from pubnames header */
    Dwarf_Unsigned version,
    Dwarf_Off info_header_offset,
    Dwarf_Unsigned info_length)
{
    printf("Pub section offset   0x%" DW_PR_XZEROS DW_PR_DUx
        " (%" DW_PR_DUu ")\n",
        pub_section_hdr_offset,pub_section_hdr_offset);
    printf("  offset size        0x%" DW_PR_XZEROS DW_PR_DUx
        " (%" DW_PR_DUu ")\n",
        length_size,
        length_size);
    printf("  length             0x%" DW_PR_XZEROS DW_PR_DUx
        " (%" DW_PR_DUu ")\n",
        length,
        length);
    printf("  version            0x%" DW_PR_XZEROS DW_PR_DUx
        " (%" DW_PR_DUu ")\n",
        version,
        version);
    printf("  info hdr offset    0x%" DW_PR_XZEROS DW_PR_DUx
        " (%" DW_PR_DUu ")\n",
        info_header_offset,
        info_header_offset);
    printf("  info hdr length    0x%" DW_PR_XZEROS DW_PR_DUx
        " (%" DW_PR_DUu ")\n",
        info_length,
        info_length);
}

/* Get all the data in .debug_pubnames */
int
print_pubnames(Dwarf_Debug dbg,Dwarf_Error *err)
{
    Dwarf_Global *globbuf = NULL;
    Dwarf_Signed count = 0;
    /* Offset to previous CU */
    int res = 0;
    Dwarf_Addr elf_max_address = 0;
    char buf[DWARF_SECNAME_BUFFER_SIZE];
    struct esb_s truename;
    char sanbuf[ESB_FIXED_ALLOC_SIZE];
    struct esb_s sanitname;

    glflags.current_section_id = DEBUG_PUBNAMES;
    res = get_address_size_and_max(dbg,0,&elf_max_address,err);
    if (res != DW_DLV_OK) {
        simple_err_return_msg_either_action(res,
            "print_pubnames call to get address size and max address"
            " fails.");
        return res;
    }
    if (glflags.verbose) {
        /* For best testing! */
        res = dwarf_return_empty_pubnames(dbg,1);
        if (res != DW_DLV_OK) {
            simple_err_return_msg_either_action(res,
                "ERROR: Erroneous "
                "libdwarf call "
                "of dwarf_return_empty_pubnames: "
                "dwarfdump internal error");
            return res;
        }
    }
    res = dwarf_get_globals(dbg, &globbuf, &count, err);

    /*  Do get_true_section_name() after loading section
        so it reports on compressed data */
    esb_constructor_fixed(&truename,buf,sizeof(buf));
    get_true_section_name(dbg,".debug_pubnames",
        &truename,TRUE);
    esb_constructor_fixed(&sanitname,sanbuf,sizeof(sanbuf));
    /*  Sanitized cannot be safely reused,there is a static buffer,
        so we make a safe copy. */
    esb_append(&sanitname,sanitized(esb_get_string(&truename)));
    esb_destructor(&truename);

    if (glflags.gf_do_print_dwarf && count > 0) {
        printf("\n%s\n",esb_get_string(&sanitname));
    }
    if (res == DW_DLV_ERROR) {
        simple_err_return_msg_either_action(res,"ERROR: "
            "dwarf_get_globals failed in print_pubnames().");
        esb_destructor(&sanitname);
        return res;
        /* fall through to end*/
    }
    if (res == DW_DLV_NO_ENTRY) {
        esb_destructor(&sanitname);
        return res;
    }
    res = print_all_pubnames_style_records(dbg,
        "global",
        esb_get_string(&sanitname),
        globbuf,count,err);
    esb_destructor(&sanitname);
    dwarf_globals_dealloc(dbg,globbuf,count);
    dwarf_return_empty_pubnames(dbg,0);
    return res;
}

int
print_all_pubnames_style_records(Dwarf_Debug dbg,
    const char *linetitle,
    const char * section_true_name,
    Dwarf_Global *globbuf,
    Dwarf_Signed count,
    Dwarf_Error *err)
{
    Dwarf_Unsigned maxoff = get_info_max_offset(dbg);
    Dwarf_Unsigned lastcudieoff = 0;
    Dwarf_Addr elf_max_address = 0;
    Dwarf_Signed i = 0;
    int ares = 0;
    Dwarf_Bool is_info = TRUE;

    ares = get_address_size_and_max(dbg,0,&elf_max_address,err);
    if (ares != DW_DLV_OK) {
        simple_err_return_msg_either_action(ares,
            "ERROR: print_pubnames style call to get "
            "address size and max address"
            " fails.");
        return ares;
    }

    for (i = 0; i < count; i++) {
        int nres = 0;
        int cures3 = 0;
        Dwarf_Off die_off = 0;
        Dwarf_Off cu_die_off = 0;
        Dwarf_Off prev_cu_off = elf_max_address;
        Dwarf_Off global_cu_off = 0;
        char *name = 0;

        /*  Turns the cu-local die_off in globbuf
            entry into a global die_off.  The cu_off
            returned is the offset of the CU DIE,
            not the CU header. */
        nres = dwarf_global_name_offsets(globbuf[i],
            &name, &die_off, &cu_die_off,
            err);
        if (nres != DW_DLV_OK) {
            struct esb_s m;

            esb_constructor(&m);
            esb_append_printf_i(&m,
                "ERROR: dwarf_global_name_offsets for globals index "
                " %d ", i);
            esb_append_printf_i(&m," with globals count %d.",
                count);
            simple_err_return_msg_either_action(nres,
                esb_get_string(&m));
            esb_destructor(&m);
            return nres;
        }
        if (glflags.verbose) {
            /*  We know no die_off can be zero
                (except for the fake global created when
                the debug_pubnames for a CU has no actual entries)
                we do not need to check for i==0 to detect
                this is the initial global record and
                we want to print this pubnames section CU header. */
            if (lastcudieoff != cu_die_off) {
                Dwarf_Off pub_section_hdr_offset = 0;
                Dwarf_Unsigned pub_offset_size = 0;
                Dwarf_Unsigned pub_length = 0;
                Dwarf_Unsigned pub_version = 0;
                Dwarf_Off info_header_offset = 0;
                Dwarf_Unsigned info_length = 0;
                nres = dwarf_get_globals_header(globbuf[i],
                    &pub_section_hdr_offset,
                    &pub_offset_size,
                    &pub_length,&pub_version,
                    &info_header_offset,&info_length,err);
                if (nres != DW_DLV_OK) {
                    struct esb_s msge;

                    esb_constructor(&msge);
                    esb_append(&msge,
                        "ERROR Access dwarf_get_globals_header ");
                    esb_append_printf_i(&msge,
                        " for index %d in ",i);
                    esb_append(&msge,section_true_name);
                    esb_append(&msge,".");
                    simple_err_return_msg_either_action(nres,
                        esb_get_string(&msge));
                    esb_destructor(&msge);
                    return nres;
                }
                if (glflags.gf_do_print_dwarf) {
                    print_globals_header(pub_section_hdr_offset,
                        pub_offset_size,
                        pub_length,
                        pub_version,
                        info_header_offset,
                        info_length);
                }
            }
        }
        lastcudieoff = cu_die_off;
        if (glflags.verbose) {
            /* If verbose we can see a zero die_off. */
            if (!die_off  && !strlen(name)) {
                /*  A different and impossible cu die offset in case
                    of an empty pubnames CU. */
                continue;
            }
        }
        /*  This gets the CU header offset, which
            is the offset that die_off needs to be added to
            to calculate the DIE offset. Note that
            dwarf_global_name_offsets already did that
            addition properly so this call is just so
            we can print the CU header offset. */
        cures3 = dwarf_global_cu_offset(globbuf[i],
            &global_cu_off, err);
        if (cures3 != DW_DLV_OK) {
            struct esb_s msge;

            esb_constructor(&msge);
            esb_append(&msge,"ERROR Access dwarf_global_cu_offset ");
            esb_append_printf_i(&msge," for index %d in ",i);
            esb_append(&msge,section_true_name);
            esb_append(&msge,".");
            simple_err_return_msg_either_action(cures3,
                esb_get_string(&msge));
            esb_destructor(&msge);
            return cures3;
        }

        if (glflags.gf_check_pubname_attr) {
            Dwarf_Bool has_attr = 0;
            int dres = 0;
            Dwarf_Die die = 0;

            /*  We are processing a new set of pubnames
                for a different CU; get the producer ID,
                at 'cu_off' to see if we need to skip
                these pubnames */
            if (cu_die_off != prev_cu_off) {
                char proname[100];
                struct esb_s producername;
                Dwarf_Die lcudie = 0;

                /* Record offset for previous CU */
                prev_cu_off = cu_die_off;
                dres = dwarf_offdie_b(dbg, cu_die_off,
                    is_info, &lcudie, err);
                if (dres != DW_DLV_OK) {
                    struct esb_s msge;

                    esb_constructor(&msge);
                    esb_append(&msge,
                        "ERROR Accessing dwarf_offdie ");
                    esb_append_printf_i(&msge,
                        " for index %d in ",i);
                    esb_append(&msge,section_true_name);
                    esb_append(&msge,".");
                    simple_err_return_msg_either_action(dres,
                        esb_get_string(&msge));
                    esb_destructor(&msge);
                    return dres;
                }
                /*  Get producer name for this CU
                    and update compiler list */
                esb_constructor_fixed(&producername,proname,
                    sizeof(proname));
                dres = get_producer_name(dbg,lcudie,cu_die_off,
                    &producername,err);
                dwarf_dealloc(dbg,lcudie,DW_DLA_DIE);
                if (dres == DW_DLV_ERROR) {
                    esb_destructor(&producername);
                    return dres;
                }
                update_compiler_target(
                    esb_get_string(&producername));
                glflags.DIE_CU_overall_offset = cu_die_off;
                esb_destructor(&producername);
            }

            /* get die at die_off */
            dres = dwarf_offdie_b(dbg, die_off, is_info, &die, err);
            if (dres != DW_DLV_OK) {
                struct esb_s msge;

                esb_constructor(&msge);
                esb_append(&msge,"ERROR Accessing dwarf_offdie ");
                esb_append_printf_i(&msge," for index %d in ",i);
                esb_append(&msge,section_true_name);
                esb_append_printf_u(&msge," with die offset 0x%x ",
                    die_off);
                esb_append(&msge,".");
                simple_err_return_msg_either_action(dres,
                    esb_get_string(&msge));
                esb_destructor(&msge);
                return dres;
            }
            ares =
                dwarf_hasattr(die, DW_AT_external,
                &has_attr, err);
            if (ares == DW_DLV_ERROR) {
                struct esb_s msgd;

                esb_constructor(&msgd);
                esb_append(&msgd,"ERROR: hasattr on DW_AT_external"
                    " from ");
                esb_append(&msgd,section_true_name);
                esb_append(&msgd," fails.");
                dwarf_dealloc(dbg, die, DW_DLA_DIE);
                /* print_error does not return */
                simple_err_return_msg_either_action(ares,
                    esb_get_string(&msgd));
                esb_destructor(&msgd);
                return ares;
            }
            /*  DW_DLV_NO_ENTRY is odd. Check that if checking. */
            /*  Check for specific compiler */
            if (checking_this_compiler()) {
                DWARF_CHECK_COUNT(pubname_attr_result,1);
                if (ares == DW_DLV_OK && has_attr) {
                    /* Should the value of flag be examined? */
                } else {
                    DWARF_CHECK_ERROR2(pubname_attr_result,name,
                        "pubname does not have DW_AT_external");
                }
            }
            dwarf_dealloc(dbg, die, DW_DLA_DIE);
        }

        /* Now print name, after the test */
        if (glflags.gf_do_print_dwarf ||
            (glflags.gf_record_dwarf_error &&
            glflags.gf_check_verbose_mode)) {
            int res = 0;

            res  = print_pubname_style_entry(dbg,
                linetitle,
                name, die_off, cu_die_off,
                global_cu_off, maxoff,err);
            if (res != DW_DLV_OK) {
                return res;
            }
            glflags.gf_record_dwarf_error = FALSE;
        }
    }
    return DW_DLV_OK;
}
