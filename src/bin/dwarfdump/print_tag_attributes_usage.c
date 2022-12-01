/*
Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
Portions Copyright 2009-2018 SN Systems Ltd. All rights reserved.
Portions Copyright 2007-2021 David Anderson. All rights reserved.

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

#include <config.h>

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "dd_globals.h"
#include "dd_naming.h"
#include "dd_esb.h"                /* For flexible string buffer. */
#include "dd_esb_using_functions.h"
#include "dd_sanitized.h"
#include "print_frames.h"  /* for print_expression_operations() . */
#include "dd_macrocheck.h"
#include "dd_helpertree.h"
#include "dd_tag_common.h"
#include "dd_attr_form.h"

static int pd_dwarf_names_print_on_error = 1;

#ifdef HAVE_USAGE_TAG_ATTR
/*  Record TAGs usage */
static unsigned int tag_usage[DW_TAG_last] = {0};

void
record_tag_usage(int tag)
{
    if (tag < DW_TAG_last) {
        ++tag_usage[tag];
    }
}
#endif /* HAVE_USAGE_TAG_ATTR */

#include "dwarfdump-ta-table.h"
#include "dwarfdump-ta-ext-table.h"

int
legal_tag_attr_combination(Dwarf_Half tag, Dwarf_Half attr)
{
    if (tag <= 0) {
        return FALSE;
    }
    if (tag < ATTR_TREE_ROW_COUNT) {
        int index = attr / BITS_PER_WORD;
        if (index < ATTR_TREE_COLUMN_COUNT) {
            unsigned bitflag = ((unsigned)1) <<
                (attr % BITS_PER_WORD);
            int known = ((tag_attr_combination_table[tag][index]
                & bitflag) > 0 ? TRUE : FALSE);
            if (known) {
#ifdef HAVE_USAGE_TAG_ATTR
                /* Record usage of pair (tag,attr) */
                if ( glflags.gf_print_usage_tag_attr) {
                    Usage_Tag_Attr *usage_ptr = usage_tag_attr[tag];
                    while (usage_ptr->attr) {
                        if (attr == usage_ptr->attr) {
                            ++usage_ptr->count;
                            break;
                        }
                        ++usage_ptr;
                    }
                }
#endif /* HAVE_USAGE_TAG_ATTR */
                return TRUE;
            }
        }
    }
    /*  DW_AT_MIPS_fde  used to return TRUE as that was
        convenient for SGI/MIPS users. */
    if (!glflags.gf_suppress_check_extensions_tables) {
        int r = 0;
        for (; r < ATTR_TREE_EXT_ROW_COUNT; ++r ) {
            int c = 1;
            if (tag != tag_attr_combination_ext_table[r][0]) {
                continue;
            }
            for (; c < ATTR_TREE_EXT_COLUMN_COUNT ; ++c) {
                if (tag_attr_combination_ext_table[r][c] == attr) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

#include "dwarfdump-tt-table.h"
#include "dwarfdump-tt-ext-table.h"

/*  Look only at valid table entries
    The check here must match the building-logic in
    tag_tree.c
    And must match the tags defined in dwarf.h
    The tag_tree_combination_table is a table of bit flags.  */
int
legal_tag_tree_combination(Dwarf_Half tag_parent,
    Dwarf_Half tag_child)
{
    if (tag_parent <= 0) {
        return FALSE;
    }
    if (tag_parent < TAG_TREE_ROW_COUNT) {
        int index = tag_child / BITS_PER_WORD;
        if (index < TAG_TREE_COLUMN_COUNT) {
            unsigned bitflag = ((unsigned)1) <<
                (tag_child % BITS_PER_WORD);
            int known = ((tag_tree_combination_table[tag_parent]
                [index] & bitflag) > 0 ? TRUE : FALSE);
            if (known) {
#ifdef HAVE_USAGE_TAG_ATTR
                /* Record usage of pair (tag_parent,tag_child) */
                if ( glflags.gf_print_usage_tag_attr) {
                    Usage_Tag_Tree *usage_ptr =
                        usage_tag_tree[tag_parent];
                    while (usage_ptr->tag) {
                        if (tag_child == usage_ptr->tag) {
                            ++usage_ptr->count;
                            break;
                        }
                        ++usage_ptr;
                    }
                }
#endif /* HAVE_USAGE_TAG_ATTR */
                return TRUE;
            }
        }
    }
    if (!glflags.gf_suppress_check_extensions_tables) {
        int r = 0;
        for (; r < TAG_TREE_EXT_ROW_COUNT; ++r ) {
            int c = 1;
            if (tag_parent != tag_tree_combination_ext_table[r][0]) {
                continue;
            }
            for (; c < TAG_TREE_EXT_COLUMN_COUNT ; ++c) {
                if (tag_tree_combination_ext_table[r][c] ==
                    tag_child) {
                    return TRUE;
                }
            }
        }
    }
    return (FALSE);
}

/* Print a detailed tag and attributes usage */
int
print_tag_attributes_usage(void)
{
#ifdef HAVE_USAGE_TAG_ATTR
    /*  Traverse the tag-tree table to print its usage and
        then use the DW_TAG value as an index into the
        tag_attr table to print its
        associated usage all together. */
    Dwarf_Bool print_header = TRUE;
    Rate_Tag_Tree *tag_rate;
    Rate_Tag_Attr *atr_rate;
    Usage_Tag_Tree *usage_tag_tree_ptr;
    Usage_Tag_Attr *usage_tag_attr_ptr;
    Dwarf_Unsigned total_tags = 0;
    Dwarf_Unsigned total_atrs = 0;
    Dwarf_Half total_found_tags = 0;
    Dwarf_Half total_found_atrs = 0;
    Dwarf_Half total_legal_tags = 0;
    Dwarf_Half total_legal_atrs = 0;
    float rate_1;
    float rate_2;
    int tag;
    printf("\n*** TAGS AND ATTRIBUTES USAGE ***\n");
    for (tag = 1; tag < DW_TAG_last; ++tag) {
        /* Print usage of children TAGs */
        if ( glflags.gf_print_usage_tag_attr_full || tag_usage[tag]) {
            usage_tag_tree_ptr = usage_tag_tree[tag];
            if (usage_tag_tree_ptr && print_header) {
                total_tags += tag_usage[tag];
                printf("%6d %s\n",
                    tag_usage[tag],
                    get_TAG_name(tag,pd_dwarf_names_print_on_error));
                print_header = FALSE;
            }
            while (usage_tag_tree_ptr && usage_tag_tree_ptr->tag) {
                if ( glflags.gf_print_usage_tag_attr_full ||
                    usage_tag_tree_ptr->count) {
                    total_tags += usage_tag_tree_ptr->count;
                    printf("%6s %6d %s\n",
                        " ",
                        usage_tag_tree_ptr->count,
                        get_TAG_name(usage_tag_tree_ptr->tag,
                            pd_dwarf_names_print_on_error));
                    /* Record the tag as found */
                    if (usage_tag_tree_ptr->count) {
                        ++rate_tag_tree[tag].found;
                    }
                }
                ++usage_tag_tree_ptr;
            }
        }
        /* Print usage of attributes */
        if ( glflags.gf_print_usage_tag_attr_full || tag_usage[tag]) {
            usage_tag_attr_ptr = usage_tag_attr[tag];
            if (usage_tag_attr_ptr && print_header) {
                total_tags += tag_usage[tag];
                printf("%6d %s\n",
                    tag_usage[tag],
                    get_TAG_name(tag,pd_dwarf_names_print_on_error));
            }
            while (usage_tag_attr_ptr && usage_tag_attr_ptr->attr) {
                if ( glflags.gf_print_usage_tag_attr_full ||
                    usage_tag_attr_ptr->count) {
                    total_atrs += usage_tag_attr_ptr->count;
                    printf("%6s %6d %s\n",
                        " ",
                        usage_tag_attr_ptr->count,
                        get_AT_name(usage_tag_attr_ptr->attr,
                            pd_dwarf_names_print_on_error));
                    /* Record the attribute as found */
                    if (usage_tag_attr_ptr->count) {
                        ++rate_tag_attr[tag].found;
                    }
                }
                ++usage_tag_attr_ptr;
            }
        }
        print_header = TRUE;
    }
    printf("** Summary **\n"
        "Number of standard tags      : %10" /*DW_PR_XZEROS*/
        DW_PR_DUu "\n"  /* TAGs */
        "Number of standard attributes: %10" /*DW_PR_XZEROS*/
        DW_PR_DUu "\n"  /* ATRs */,
        total_tags,
        total_atrs);

    total_legal_tags = 0;
    total_found_tags = 0;
    total_legal_atrs = 0;
    total_found_atrs = 0;

    /* Print percentage of TAGs covered */
    printf("\n*** STANDARD TAGS AND ATTRIBUTES USAGE RATE ***\n");
    printf("%-32s %-16s %-16s\n"," ","Tags","Attributes");
    printf("%-32s legal found rate legal found rate\n","TAG name");
    for (tag = 1; tag < DW_TAG_last; ++tag) {
        tag_rate = &rate_tag_tree[tag];
        atr_rate = &rate_tag_attr[tag];
        if ( glflags.gf_print_usage_tag_attr_full ||
            tag_rate->found || atr_rate->found) {
            rate_1 = tag_rate->legal ?
                (float)((tag_rate->found * 100) / tag_rate->legal):0;
            rate_2 = atr_rate->legal ?
                (float)((atr_rate->found * 100) / atr_rate->legal):0;
            /* Skip not defined DW_TAG values (See dwarf.h) */
            if (usage_tag_tree[tag]) {
                total_legal_tags += tag_rate->legal;
                total_found_tags += tag_rate->found;
                total_legal_atrs += atr_rate->legal;
                total_found_atrs += atr_rate->found;
                printf("%-32s %5d %5d %3.0f%% %5d %5d %3.0f%%\n",
                    get_TAG_name(tag,pd_dwarf_names_print_on_error),
                    tag_rate->legal,tag_rate->found,rate_1,
                    atr_rate->legal,atr_rate->found,rate_2);
            }
        }
    }

    /* Print a whole summary */
    rate_1 = total_legal_tags ?
        (float)((total_found_tags * 100) / total_legal_tags) : 0;
    rate_2 = total_legal_atrs ?
        (float)((total_found_atrs * 100) / total_legal_atrs) : 0;
    printf("%-32s %5d %5d %3.0f%% %5d %5d %3.0f%%\n",
        "** Summary **",
        total_legal_tags,total_found_tags,rate_1,
        total_legal_atrs,total_found_atrs,rate_2);
    if (glflags.gf_check_tag_attr ||
        glflags.gf_check_attr_encoding ||
        glflags.gf_print_usage_tag_attr) {
        print_attr_form_usage(pd_dwarf_names_print_on_error);
    }
#endif /* HAVE_USAGE_TAG_ATTR */
    return DW_DLV_OK;
}
