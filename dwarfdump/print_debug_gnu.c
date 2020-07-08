/*
  Copyright (C) 2020 David Anderson. All Rights Reserved.
  Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that the
  following conditions are met:

    Redistributions of source code must retain the above
    copyright notice, this list of conditions and the following
    disclaimer.

    Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*  To print .debug_gnu_pubnames, .debug_gnu_typenames */
#include "globals.h"
#ifdef HAVE_STDINT_H
#include <stdint.h> /* For uintptr_t */
#endif /* HAVE_STDINT_H */
#include "naming.h"
#include "esb.h"                /* For flexible string buffer. */
#include "esb_using_functions.h"
#include "sanitized.h"
#include  "print_debug_gnu.h"

#define TRUE 1
#define FALSE 0

char *ikind_types[8] = {
    "none",
    "type",
    "variable",
    "function",
    "other",
    "unknown5",
    "unknown6",
    "unknown7" };


static int
print_block_entries(
    UNUSEDARG Dwarf_Debug dbg,
    UNUSEDARG Dwarf_Bool for_pubnames,
    UNUSEDARG struct esb_s * secname,
    Dwarf_Gnu_Index_Head head,
    Dwarf_Unsigned blocknum,
    Dwarf_Unsigned entrycount,
    Dwarf_Error *error)
{
    Dwarf_Unsigned i = 0;
    int res = 0;


    printf("    [   ] offset     Kind        Name\n");

    for ( ; i < entrycount; ++i) {
        Dwarf_Unsigned offset_in_debug_info = 0;
        const char *name = 0;
        unsigned char flag = 0;
        unsigned char staticorglobal = 0;
        unsigned char typeofentry = 0;
        /*  flag is all 8 bits and staticorglobal
            and typeofentry were extracted from the flag.
            Present here so we can check all 8 bits
            are correct (lowest 4 should be zero).  */

        res = dwarf_get_gnu_index_block_entry(head,
            blocknum,i,&offset_in_debug_info,
            &name,&flag,&staticorglobal,&typeofentry,
            error);
        if (res == DW_DLV_ERROR) {
            return res;
        }
        if (res == DW_DLV_NO_ENTRY) {
            printf("  ERROR: Block %" DW_PR_DUu
                " entry %" DW_PR_DUu
                " does not exist though entry count"
                " is %" DW_PR_DUu
                ", something is wrong\n",
                blocknum,
                i,entrycount);
            glflags.gf_count_major_errors++;
            return res;
        }
        printf("    [%3" DW_PR_DUu "] 0x%" DW_PR_XZEROS DW_PR_DUx,
            i,offset_in_debug_info);
        printf(" %s,%-8s",
            staticorglobal?"s":"g",
            ikind_types[0x7 & typeofentry]);
        printf(" %s",sanitized(name));
        printf("\n");
        if (flag&0xf) {
            printf("  ERROR: Block %" DW_PR_DUu
                " entry %" DW_PR_DUu " flag 0x%x. "
                "The lower bits are non-zero "
                "so there may be a corruption problem.",
                blocknum,i, flag);
            glflags.gf_count_major_errors++;
            printf("\n");
        }
#if 0
        If it has a dwo, this needs to look up in the dwo object.
        {
            Dwarf_Die die = 0;
            Dwarf_Bool is_info = TRUE;
            res = dwarf_offdie_b(dbg,offset_in_debug_info,
                is_info, &die,error);
            if (res != DW_DLV_OK) {
                printf("  ERROR: Block %" DW_PR_DUu
                    " entry %" DW_PR_DUu " offset 0x%"
                    DW_PR_DUx
                    " is not a valid DIE offset in .debug_info",
                    blocknum,i, offset_in_debug_info);
                if (res == DW_DLV_ERROR) {
                    dwarf_dealloc_error(dbg,*error);
                    *error = 0;
                } else {
                }
                glflags.gf_count_major_errors++;
                printf("\n");
            }else {
                /* Look into DIE? */
                dwarf_dealloc_die(die);
            }
        }
#endif
    }
    return DW_DLV_OK;
}

static int
print_all_blocks(Dwarf_Debug dbg,
    Dwarf_Bool           for_pubnames,
    struct esb_s        *secname,
    Dwarf_Gnu_Index_Head head,
    Dwarf_Unsigned       block_count,
    Dwarf_Error         *error)
{
    Dwarf_Unsigned i = 0;
    int res = 0;

    for ( ; i < block_count; ++i) {
        Dwarf_Unsigned block_length            = 0;
        Dwarf_Half version                     = 0;
        Dwarf_Unsigned offset_into_debug_info  = 0;
        Dwarf_Unsigned size_of_debug_info_area = 0;
        Dwarf_Unsigned entrycount              = 0;

        res = dwarf_get_gnu_index_block(head,i,
            &block_length,&version,
            &offset_into_debug_info,
            &size_of_debug_info_area,
            &entrycount,error);
        if (res == DW_DLV_NO_ENTRY) {
            printf("  ERROR: Block %" DW_PR_DUu
                " does not exist though block count"
                " is %" DW_PR_DUu
                ", something is wrong\n",
                i,block_count);
            glflags.gf_count_major_errors++;
            return res;
        }
        if (res == DW_DLV_ERROR) {
            return res;
        }
        printf("  Blocknumber                         : "
            "%" DW_PR_DUu "\n",i);
        printf("  Block length                        : "
            "%" DW_PR_DUu "\n",block_length);
        printf("  Version                             : "
            "%u\n",version);
        printf("  Offset into .debug_info section     : "
            "0x%" DW_PR_XZEROS DW_PR_DUx "\n",offset_into_debug_info);
        printf("  Size of area in .debug_info section : "
            "%" DW_PR_DUu "\n",size_of_debug_info_area);
        printf("  Number of entries in block          : "
            "%" DW_PR_DUu "\n",entrycount);
        res = print_block_entries(dbg,for_pubnames,
            secname,head,i,entrycount,error);
        if (res == DW_DLV_ERROR) {
            return res;
        }
#if 0
        {
        Dwarf_Unsigned cu_die_offset           = 0;

        /* If has dwo, look there */
        res = dwarf_get_cu_die_offset_given_cu_header_offset_b(
            dbg,offset_into_debug_info,/*is_info = */ TRUE,
            &cu_die_offset,error);
        if (res != DW_DLV_OK) {
            printf("  ERROR: Block %" DW_PR_DUu
                " has an invalid .debug_info offset of "
                "0x%" DW_PR_DUx
                ", something is wrong\n",
                i,offset_into_debug_info);
            if (res == DW_DLV_ERROR) {
                dwarf_dealloc_error(dbg,*error);
                *error = 0;
            }
            glflags.gf_count_major_errors++;
            return res;
        } else {
            Dwarf_Die die = 0;
            Dwarf_Bool is_info = TRUE;
            res = dwarf_offdie_b(dbg,cu_die_offset,
                is_info, &die,error);
            if (res != DW_DLV_OK) {
                printf("  ERROR: Block %" DW_PR_DUu
                    " cu DIE offset 0x%" DW_PR_DUx
                    " is not a valid DIE offset in .debug_info\n",
                    i, cu_die_offset);
                if (res == DW_DLV_ERROR) {
                    dwarf_dealloc_error(dbg,*error);
                    *error = 0;
                }
                glflags.gf_count_major_errors++;
            } else {
                /* look into die */
                dwarf_dealloc_die(die);
            }

        }
        }
#endif

    }
    return DW_DLV_OK;
}



int
print_debug_gnu(UNUSEDARG Dwarf_Debug dbg,
    UNUSEDARG Dwarf_Error *error)
{
    int res = 0;
    Dwarf_Gnu_Index_Head head = 0;
    Dwarf_Bool for_pubnames = TRUE;
    Dwarf_Unsigned block_count = 0;
    const char *stdname = 0;
    char buf[DWARF_SECNAME_BUFFER_SIZE];
    struct esb_s truename;
    unsigned int i = 0;

    for(i = 0; i < 2; i++) {
        esb_constructor_fixed(&truename,buf,
            DWARF_SECNAME_BUFFER_SIZE);
        if(!i) {
            glflags.current_section_id = DEBUG_GNU_PUBNAMES;
            for_pubnames = TRUE;
            stdname =  ".debug_gnu_pubnames";
        } else {
            for_pubnames = FALSE;
            glflags.current_section_id = DEBUG_GNU_PUBTYPES;
            stdname = ".debug_gnu_pubtypes";
        }
        get_true_section_name(dbg,stdname, &truename,TRUE);
        res = dwarf_get_gnu_index_head(dbg,for_pubnames,
            &head, &block_count,error);
        if (res == DW_DLV_ERROR) {
            glflags.gf_count_major_errors++;
            printf("ERROR: problem reading %s. %s\n",
                sanitized(esb_get_string(&truename)),
                dwarf_errmsg(*error));
            dwarf_dealloc_error(dbg,*error);
            *error = 0;
            continue;
        } else if (res == DW_DLV_NO_ENTRY) {
            continue;
        }
        printf("\n%s with %" DW_PR_DUu
            " blocks of names\n",
            sanitized(esb_get_string(&truename)),
            block_count);
        res = print_all_blocks(dbg,for_pubnames,
            &truename, head,block_count,error);
        if (res == DW_DLV_ERROR) {
            glflags.gf_count_major_errors++;
            printf("ERROR: problem reading %s. %s\n",
                sanitized(esb_get_string(&truename)),
                dwarf_errmsg(*error));
            dwarf_dealloc_error(dbg,*error);
            *error = 0;
        } else if (res == DW_DLV_NO_ENTRY) {
            /* impossible */
        } else {
            /* normal */
        }
        esb_destructor(&truename);
    }
    return DW_DLV_OK;
}
