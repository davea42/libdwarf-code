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
#include <stdio.h> /* FILE decl for dd_esb.h, printf etc */

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "dd_defined_types.h"
#include "dd_checkutil.h"
#include "dd_glflags.h"
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
#include "dd_tsearchbal.h"

/*  Adds new record with count 1 (set by caller)
    or adds one to count. 
    There is no notion of 'legal' for this set of
    counts. */
static int
find_legal_and_update(Three_Key_Entry *ke,
    void **treeptr,
    unsigned char *caller_free_ke,
    unsigned char *table_found)
{
    void           *ret = 0;
    Three_Key_Entry *re = 0;

    ke->count = 1;
    *caller_free_ke = FALSE;
    ret = dwarf_tsearch(ke,treeptr,std_compare_3key_entry);
    if (!ret) {
        /* Something badly wrong. ?? */
        *caller_free_ke = TRUE;
        return TK_ERROR;
    }
    re = *(Three_Key_Entry**)ret;
    if (re == ke) {
        /* already gave count 1 */
        *caller_free_ke = FALSE;
        if (table_found) {
            *table_found = re->from_tables;
        }
        return TK_OK;
    }
#if 0
printf("dadebug update entry count from %lu 0x%x 0x%x line %d\n",
(unsigned long)re->count,
re->key1,re->key2,__LINE__);
#endif
    re->count++;
#if 0
printf("dadebug now count is %lu 0x%x 0x%x line %d\n",
(unsigned long)re->count,
re->key1,re->key2,__LINE__);
#endif
    *caller_free_ke = TRUE;
    if (table_found) {
        *table_found = re->from_tables;
    }
    return TK_OK;
}

/*  Much of updating tag_tag and tag_tree counts
    does not apply here. */
void
record_tag_usage(Dwarf_Half tag)
{
    Three_Key_Entry *tkp = 0;
    int mres = 0; 
    unsigned char caller_free_tkp = FALSE;

    mres = make_3key(tag,0,0,0,0,1,&tkp);
    if (mres == DW_DLV_ERROR) {
        /* out of memory, pretend ok */
        return;
    }
#if 0
printf("dadebug now record_tag_usage on 0x%x\n",tag);
#endif
    find_legal_and_update(tkp,&threekey_tag_use_base,
        &caller_free_tkp,0);
    if (caller_free_tkp) {
        free_func_3key_entry(tkp);
    }
    return;
}


/*  Return TRUE to suppress any errors about unknown combos 
    Return FALSE to cause a message. 
    Maybe should have a third case for error. */
int
legal_tag_attr_combination(Dwarf_Half tag, Dwarf_Half attr)
{
    Three_Key_Entry *tkp = 0;  
    int keyres = 0;
    int mres = 0;
    unsigned char caller_free_tkp = FALSE;
    unsigned char table_id = 0;
    Dwarf_Bool retval = TK_ERROR;

#if 0
printf("dadebug legal_tag_attr tag 0x%x attr 0x%x\n",
tag,attr);
#endif
    mres = make_3key(tag,attr,0,0,0,1,&tkp);
    if (mres == DW_DLV_ERROR) {
        /* out of memory, pretend ok */
        return TK_ERROR;
    }
    keyres = find_legal_and_update(tkp,&threekey_tag_attr_base,
        &caller_free_tkp,&table_id); 
    if (keyres == TK_ERROR) {
        if (caller_free_tkp){
            free_func_3key_entry(tkp);
        }
        return TK_ERROR;
    }
    if (glflags.gf_suppress_check_extensions_tables) {
        if (table_id == AF_STD) {
            retval = TK_OK;
        } else {
            retval = TK_SHOW_MESSAGE;
        }
    } else {
        if (table_id) {
            retval = TK_OK;
        } else {
            retval = TK_SHOW_MESSAGE;
        }
    }
    if (caller_free_tkp) {    
        free_func_3key_entry(tkp);
    }
    return retval;
}

/*  Return TK_OK or TK_SHOW_MESSAGE or TK_ERROR */ 
int
legal_tag_tree_combination(Dwarf_Half tag_parent,
    Dwarf_Half tag_child)
{
    Three_Key_Entry *tkp = 0;
    int keyres = 0;
    int mres = 0;
    unsigned char caller_free_tkp = FALSE;
    unsigned char table_id = 0;
    Dwarf_Bool retval = TK_ERROR;


    mres = make_3key(tag_parent,tag_child,0,0,0,1,&tkp);

    if (mres == DW_DLV_ERROR) {
        return TK_ERROR;
    }
    keyres = find_legal_and_update(tkp,&threekey_tag_tag_base,
        &caller_free_tkp,&table_id);
    if (keyres == TK_ERROR) {
        if (caller_free_tkp){
            free_func_3key_entry(tkp);
        }
        return TK_ERROR;
    }
    if (glflags.gf_suppress_check_extensions_tables) {
        if (table_id == AF_STD) {
            retval = TK_OK;
        } else {
            retval = TK_SHOW_MESSAGE;
        }
    } else {
        if (table_id) {
            retval = TK_OK;
        } else {
            retval = TK_SHOW_MESSAGE;
        }
    }
    if (caller_free_tkp) {
        free_func_3key_entry(tkp);
    }
    return retval;
}

/* Print a detailed tag and attributes usage */
int
print_tag_attributes_usage(void)
{
    /*  Traverse the tag-tree table to print its usage and
        then use the DW_TAG value as an index into the
        tag_attr table to print its
        associated usage all together. */
    Dwarf_Unsigned tag_tag_count = 0;
    Dwarf_Unsigned tag_attr_count = 0;
    Dwarf_Unsigned tag_count = 0;

    printf("\n*** TAGS AND ATTRIBUTES USAGE ***\n");
    /*  extract all tag_tree  records
        (also called tag_tag sometimes) to a list, 
        sort by tag number and child tag number. */
    /*  Loop the list, printing tag (and name)
           and within that a line for each child tag and count.
    */
    tag_tag_count = three_key_entry_count(threekey_tag_tag_base);
    tag_attr_count = three_key_entry_count(threekey_tag_attr_base);
    tag_count = three_key_entry_count(threekey_tag_use_base);

    dd_print_tag_tree_results(tag_tag_count);
    dd_print_tag_attr_results(tag_attr_count);
    dd_print_tag_use_results(tag_count);


#if 0
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
        print_attr_form_usage();
    }
#endif
    return DW_DLV_OK;

}
