/*
  Copyright (C) 2021 David Anderson. All Rights Reserved.

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

#include <stdio.h>  /* printf() */
#include <stdlib.h> /* calloc() free() malloc() */

/* Windows specific header files */
#if defined(_WIN32) && defined(HAVE_STDAFX_H)
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "dd_globals.h"
#include "dd_tsearchbal.h"
#include "dd_naming.h"
#include "dd_attr_form.h"
#include "dwarfdump-af-table.h"

#if 0
static void
print_3key_record(int num,Three_Key_Entry *e)
{
    printf("3key %d 0x%x 0x%x 0x%x st %d%d ct %lu\n",
        num,e->key1,e->key2,e->key3,
        e->std_or_exten,e->from_tables,
        (unsigned long)e->count);
}
#endif /* 0 */

int
make_3key(Dwarf_Half k1,
    Dwarf_Half k2,
    Dwarf_Half k3,
    Dwarf_Small std_or_exten,
    Dwarf_Small from_preset,
    Dwarf_Unsigned count,
    Three_Key_Entry ** out)
{
    Three_Key_Entry *e =
        (Three_Key_Entry *)malloc(sizeof(Three_Key_Entry));
    if (!e) {
        return DW_DLV_ERROR; /* out of memory */
    }
    e->key1 = k1;
    e->key2 = k2;
    e->key3 = k3;
    e->std_or_exten = std_or_exten;
    e->from_tables  = from_preset;
    e->count        = count;
    *out = e;
    return DW_DLV_OK;
}

void
free_func_3key_entry(void *keystructptr)
{
    Three_Key_Entry *e = keystructptr;
    free(e);
}

int
std_compare_3key_entry(const void *l_in, const void *r_in)
{
    const Three_Key_Entry *l = l_in;
    const Three_Key_Entry *r = r_in;
    if (l->key1 < r->key1) {
        return -1;
    }
    if (l->key1 > r->key1) {
        return 1;
    }
    if (l->key2 < r->key2) {
        return -1;
    }
    if (l->key2 > r->key2) {
        return 1;
    }
    if (l->key3 < r->key3) {
        return -1;
    }
    if (l->key3 > r->key3) {
        return 1;
    }
    return 0;

}

static Dwarf_Unsigned counting_global;
static void
count_3key_entry(const void * vptr,
    DW_VISIT x,
    int level)
{
    (void)vptr;
    (void)level;
    if (x == dwarf_preorder || x == dwarf_leaf) {
        ++counting_global;
    }
}

/*  Tree argument expected is threekey_attr_form_base for example */
Dwarf_Unsigned
three_key_entry_count(void *base)
{
    Dwarf_Unsigned count = 0;

    counting_global = 0;
    dwarf_twalk(base,count_3key_entry);
    count = counting_global;
    counting_global = 0;
    return count;
}

/*  tree argument expected is
    &threekey_attr_form_base for example */
static int
insert_new_tab_entry(void *tree,
    struct af_table_s * tab,
    int *errnum)
{
    Three_Key_Entry *e = 0;
    Three_Key_Entry *re = 0;
    void *ret = 0;
    int res = 0;

    res = make_3key(tab->attr,tab->formclass,0,
        tab->section,
        1 /* is from preset data */,
        0 /* count is zero during preset   */,
        &e);
    if (res != DW_DLV_OK) {
        *errnum = DW_DLE_ALLOC_FAIL;
        return res;
    }
    ret = dwarf_tsearch(e,tree,std_compare_3key_entry);
    if (!ret) {
        *errnum = DW_DLE_ALLOC_FAIL;
        return res;
    }
    re = *(Three_Key_Entry **)ret;
    if (re == e) {
        /* Normal. Added. */
        return DW_DLV_OK;
    }
    /*  A full duplicate in the table. Oops.
        Not a great choice of error code. */
    *errnum = DW_DLE_ATTR_FORM_BAD;
    return DW_DLV_ERROR;
}
/*  This is for dwarfdump to call at runtime.
    Returns DW_DLV_OK on success  */
int
build_attr_form_base_tree(int*errnum)
{
    struct af_table_s * tab = 0;
    int res;
    void *tree = &threekey_attr_form_base;

    if (threekey_attr_form_base) {
        /*  Do not init again if a tied file */
        return DW_DLV_OK;
    }
    for (tab = &attr_formclass_table[0]; ; tab++) {
        if ((!tab->attr) &&
            (!tab->formclass) &&
            (!tab->section)) {
            /* Done */
            break;
        }
        res  = insert_new_tab_entry(tree,tab,errnum);
        if (res != DW_DLV_OK) {
            return res;
        }
    }
    return DW_DLV_OK;
}

/*  The standard main tree for attr_form data.
    Starting out as a simple global variable.
    In general, pass &threekey_attr_form_base
    (for example) to tsearch calls. */
void * threekey_attr_form_base;

void
destroy_attr_form_trees(void)
{
    if (!threekey_attr_form_base) {
        return;
    }
    dwarf_tdestroy(threekey_attr_form_base,
        free_func_3key_entry);
    threekey_attr_form_base = 0;
}

/*  SKIP_AF_CHECK defined means this is in scripts/ddbuild.sh
    and this checking makes no sense and will not compile. */
#ifndef SKIP_AF_CHECK
static Dwarf_Bool
legal_attr_formclass_combination(Dwarf_Half attr,
    Dwarf_Half fc)
{
    Three_Key_Entry *e = 0;
    Three_Key_Entry *re = 0;
    void *ret = 0;
    int res = 0;

    res = make_3key(attr,fc,0,0,0,0,&e);
    if (res!= DW_DLV_OK) {
        /*  Hiding some sort of botch/malloc issue */
        return TRUE;
    }
    ret = dwarf_tfind(e,&threekey_attr_form_base,
        std_compare_3key_entry);
    if (!ret) {
        /*  Surprising combo. */
        free(e);
        return FALSE;
    }
    re = *(Three_Key_Entry **)ret;
    if (!glflags.gf_suppress_check_extensions_tables) {
        free(e);
        return TRUE;
    }
    if (re->std_or_exten == AF_STD) {
        free(e);
        return TRUE;
    }
    free(e);
    return FALSE;
}

static void
check_attr_formclass_combination(Dwarf_Debug dbg,
    Dwarf_Half tag,
    Dwarf_Half attrnum,
    Dwarf_Half fc,
    int pd_dwarf_names_print_on_error,
    int die_stack_indent_level)
{
    const char *tagname = "<AT invalid>";
    const char *formclassname = "<FORM_CLASS invalid>";
    DWARF_CHECK_COUNT(attr_formclass_result,1);
    if (legal_attr_formclass_combination(attrnum,fc)) {
        /* OK */
    } else {
        /* Report errors only if tag-attr check is on */
        if (glflags.gf_check_tag_attr) {
            tagname = get_AT_name(attrnum,
                pd_dwarf_names_print_on_error);
            tag_specific_globals_setup(dbg,tag,
                die_stack_indent_level);
            formclassname = get_FORM_CLASS_name(fc,
                pd_dwarf_names_print_on_error);

            DWARF_CHECK_ERROR3(attr_formclass_result,tagname,
                formclassname,
                "check the attr-formclass combination");
        } else { /* Nothing to do. */ }
    }
}
#endif /* SKIP_AF_CHECK  */

void
record_attr_form_use(
    Dwarf_Debug dbg,
    Dwarf_Half tag,
    Dwarf_Half attr,
    Dwarf_Half fclass,
    Dwarf_Half form,
    int pd_dwarf_names_print_on_error,
    int die_stack_indent_level)
{
    Three_Key_Entry *e =  0;
    Three_Key_Entry *re =  0;
    void *ret =  0;
    Dwarf_Small std_or_exten = 0;
    int res = 0;

    (void)dbg;
    (void)tag;
    (void)pd_dwarf_names_print_on_error;
    (void)die_stack_indent_level;

    if (attr >= DW_AT_lo_user) {
        std_or_exten = AF_EXTEN;
    } else if (form > DW_FORM_addrx4) {
        /*  There is no lo_user code for FORMs,
            they really are limited. */
        std_or_exten = AF_EXTEN;
    } else {
        std_or_exten = AF_STD;
    }
#ifndef SKIP_AF_CHECK
    check_attr_formclass_combination(dbg,
        tag,attr,fclass,pd_dwarf_names_print_on_error,
        die_stack_indent_level);
#endif /* SKIP_AF_CHECK */
    res = make_3key(attr,fclass,form,std_or_exten,0,1,&e);
    if (res!= DW_DLV_OK) {
        /*  Could print something */
        return;
    }
    ret = dwarf_tsearch(e,&threekey_attr_form_base,
        std_compare_3key_entry);
    if (!ret) {
        free_func_3key_entry(e);
        /*  Could print something */
        return;
    }
    re = *(Three_Key_Entry **)ret;
    if (re == e) {
        /*  Brand new entry. Done.
            local malloc is in the tree. */
        return;
    }
    /* Was already entered.*/
    ++re->count;
    /* Clean out the local malloc */
    free_func_3key_entry(e);
    return;
}

static Dwarf_Unsigned recordcount = 0;
static Dwarf_Unsigned recordmax = 0;
static Three_Key_Entry * tkarray = 0;
static void
extract_3key_entry(const void * vptr,
    DW_VISIT x,
    int level)
{
    (void)level;
    if (x == dwarf_preorder || x == dwarf_leaf) {
        Three_Key_Entry *m = *(Three_Key_Entry **)vptr;
        if (recordcount >= recordmax) {
            /* Should never happen */
            return;
        }
        tkarray[recordcount] = *m;
        ++recordcount;
    }
}

static int
qsortformclass(const void * e1in, const void * e2in)
{
    Three_Key_Entry *e1 = (Three_Key_Entry *)e1in;
    Three_Key_Entry *e2 = (Three_Key_Entry *)e2in;

    if (e1->key2 < e2->key2) {
        return -1;
    }
    if (e1->key2 > e2->key2) {
        return 1;
    }
    return 0;
}

static int
qsortform(const void * e1in, const void * e2in)
{
    Three_Key_Entry *e1 = (Three_Key_Entry *)e1in;
    Three_Key_Entry *e2 = (Three_Key_Entry *)e2in;

    if (e1->key3 < e2->key3) {
        return -1;
    }
    if (e1->key3 > e2->key3) {
        return 1;
    }
    return 0;
}

static int
qsortcountattr(const void * e1in, const void * e2in)
{
    Three_Key_Entry *e1 = (Three_Key_Entry *)e1in;
    Three_Key_Entry *e2 = (Three_Key_Entry *)e2in;

    if (e1->count < e2->count) {
        return 1;
    }
    if (e1->count > e2->count) {
        return -1;
    }
    if (e1->key1 < e2->key1) {
        return -1;
    }
    if (e1->key1 > e2->key1) {
        return 1;
    }
    if (e1->key3 < e2->key3) {
        return -1;
    }
    if (e1->key3 > e2->key3) {
        return 1;
    }
    if (e1->key2 < e2->key2) {
        return -1;
    }
    if (e1->key2 > e2->key2) {
        return 1;
    }
    return 0;
}

void
print_attr_form_usage(int pd_dwarf_names_print_on_error)
{
    Three_Key_Entry  *tk_l  = 0;
    Dwarf_Unsigned    i     = 0;
    unsigned          curform = 0;
    Dwarf_Unsigned    formtotal = 0;
    unsigned          curattr = 0;
    Dwarf_Unsigned    attrtotal = 0;
    Dwarf_Unsigned    j     = 0;
    float             total = 0.0f;
    float             pct   = 0.0f;
    Dwarf_Bool        startnoted = FALSE;
    const char *      localformat=  NULL;
    Dwarf_Unsigned    localsum = 0;

    recordmax = three_key_entry_count(threekey_attr_form_base);
    if (!recordmax) {
        return;
    }
    tk_l = (Three_Key_Entry *)calloc(recordmax+1,
        sizeof(Three_Key_Entry));
    tkarray=tk_l;
    if (!tk_l) {
        printf("ERROR: unable to malloc attr/form array "
            " for a summary report \n");
        glflags.gf_count_major_errors++;
        return;
    }
    /* Reset the file-global! */
    recordcount = 0;
    dwarf_twalk(threekey_attr_form_base,extract_3key_entry);
    if (recordcount != recordmax) {
        printf("ERROR: unable to fill in attr/form array "
            " for a summary report, count %lu != walk %lu \n",
            (unsigned long)recordcount,
            (unsigned long)recordmax);
        glflags.gf_count_major_errors++;
        free(tk_l);
        tkarray = 0;
        return;
    }
    for (i = 0; i < recordmax; ++i) {
        Three_Key_Entry * tke = tk_l+i;

        if (!tke->key3) {
            /* Skip table building data */
            continue;
        }
        total += (float)tke->count;
    }
    qsort(tk_l,recordmax,sizeof(Three_Key_Entry),
        qsortcountattr);
    printf("\n*** ATTRIBUTES AND FORMS USAGE ***\n");
    printf("Full record count                    : %8" DW_PR_DUu "\n",
        recordmax);
    printf("Total number of objectfile attributes: %8.0f\n", total);
    printf("[]                                        "
        "                found rate\n");
    localformat="[%3u] %-30s %-20s %7" DW_PR_DUu " %.0f%%\n";
    localsum = 0;
    for (i = 0; i < recordmax; ++i) {
        Three_Key_Entry * tke = tk_l+i;

        if (!tke->key3) {
            /* Skip table building data */
            continue;
        }
        pct = ( (float)tke->count / total)*100.0f;
        printf(localformat,
            (unsigned)i,
            get_AT_name(tke->key1,pd_dwarf_names_print_on_error),
            get_FORM_name(tke->key3,pd_dwarf_names_print_on_error),
            tke->count,pct);
        localsum += tke->count;
    }
    printf(localformat, (unsigned)(recordmax),
        "Sum found:","",localsum,100.0f);

    qsort(tk_l,recordmax,sizeof(Three_Key_Entry),
        qsortformclass);
    j = 0;
    /* Re-using the following two */
    curform = 0;
    formtotal = 0;
    startnoted = FALSE;
    printf("\n*** COUNT BY FORMCLASS ***\n");
    printf("[]                                 found rate\n");
    localsum = 0;
    localformat="[%2u] %-28s %6" DW_PR_DUu " %.0f%%\n";
    for (i = 0; i < recordmax; ++i) {
        Three_Key_Entry * tke = tk_l+i;

        if (!tke->key3) {
            /* Skip table building data */
            continue;
        }
        if (!startnoted) {
            curform = tke->key2;
            formtotal = tke->count;
            startnoted = TRUE;
            continue;
        }
        if (curform != tke->key2) {
            pct = ( (float)formtotal / total)*100.0f;
            printf(localformat,
                (unsigned)j,
                get_FORM_CLASS_name(curform,
                pd_dwarf_names_print_on_error),
                formtotal,pct);
            localsum += formtotal;
            curform = tke->key2;
            formtotal = tke->count;
            ++j;
            continue;
        }
        formtotal += tke->count;
    }
    if (formtotal) {
        pct = ( (float)formtotal / total)*100.0f;
        printf(localformat,
            (unsigned)j,
            get_FORM_CLASS_name(curform,
            pd_dwarf_names_print_on_error),
            formtotal,pct);
        localsum += formtotal;
    }
    printf(localformat, (unsigned)(j+1),
        "Sum found:",localsum,100.0f);

    /* Re-using the following two */
    curform = 0;
    formtotal = 0;
    startnoted = FALSE;
    qsort(tk_l,recordmax,sizeof(Three_Key_Entry),
        qsortform);
    j = 0;
    printf("\n*** COUNT BY FORM ***\n");
    printf("[]                         found rate\n");
    localformat="[%2u] %-20s %6" DW_PR_DUu " %.0f%%\n";
    localsum = 0;
    for (i = 0; i < recordmax; ++i) {
        Three_Key_Entry * tke = tk_l+i;

        if (!tke->key3) {
            /* Skip table building data */
            continue;
        }
        if (!startnoted) {
            curform = tke->key3;
            formtotal = tke->count;
            startnoted = TRUE;
            continue;
        }
        if (curform != tke->key3) {
            pct = ( (float)formtotal / total)*100.0f;
            printf(localformat,
                (unsigned)j,
                get_FORM_name(curform,
                pd_dwarf_names_print_on_error),
                formtotal,pct);
            localsum += formtotal;
            curform = tke->key3;
            formtotal = tke->count;
            ++j;
            continue;
        }
        formtotal += tke->count;
    }
    if (formtotal) {
        pct = ( (float)formtotal / total)*100.0f;
        printf(localformat,
            (unsigned)j,
            get_FORM_name(curform,
            pd_dwarf_names_print_on_error),
            formtotal,pct);
        localsum += formtotal;
    }
    printf(localformat, (unsigned)(j+1),
        "Sum found:",localsum,100.0f);

    j = 0;
    curattr = 0;
    attrtotal = 0;
    startnoted = FALSE;
    printf("\n*** COUNT BY ATTRIBUTE ***\n");
    printf("[]                                   found rate\n");
    localsum = 0;
    localformat="[%2u] %-30s %6" DW_PR_DUu " %.0f%%\n";
    for (i = 0; i < recordmax; ++i) {
        Three_Key_Entry * tke = tk_l+i;

        if (!tke->key3) {
            /* Skip table building data */
            continue;
        }
        if (!startnoted) {
            curattr = tke->key1;
            attrtotal = tke->count;
            startnoted = TRUE;
            continue;
        }
        if (curattr != tke->key1) {
            pct = ( (float)attrtotal / total)*100.0f;
            printf(localformat,
                (unsigned)j,
                get_AT_name(curattr,
                    pd_dwarf_names_print_on_error),
                attrtotal,pct);
            localsum += attrtotal;
            curattr = tke->key1;
            attrtotal = tke->count;
            ++j;
            continue;
        }
        formtotal += tke->count;
    }
    if (attrtotal) {
        pct = ( (float)attrtotal / total)*100.0f;
        printf(localformat,
            (unsigned)j,
            get_AT_name(curattr,pd_dwarf_names_print_on_error),
            attrtotal,pct);
        localsum += attrtotal;
    }
    printf(localformat, (unsigned)(j+1),
        "Sum found:",localsum,100.0f);
    free(tk_l);
    tkarray = 0;
}
