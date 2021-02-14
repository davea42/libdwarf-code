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

#include "globals.h"
#define DW_TSHASHTYPE long /* this type not needed */
#include "dwarf_tsearch.h"
#include "attr_form.h"
#include "dwarfdump-af-table.h"

const Three_Key_Entry threekeyzero;

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

Dwarf_Unsigned counting_global;
static void
count_3key_entry(UNUSEDARG const void * vptr,
    DW_VISIT x,
    UNUSEDARG int level)
{
    if (x == dwarf_preorder) {
        ++counting_global;
    }
}

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

static int
insert_new_tab_entry(void *tree,
    struct af_table_s * tab,
    int *errnum)
{
    Three_Key_Entry *e = 0;
    Three_Key_Entry *re = 0;
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
    re = dwarf_tsearch(e,tree,std_compare_3key_entry);
    if (!re) {
        *errnum = DW_DLE_ALLOC_FAIL;
        return res;
    }
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

    for (tab = &attr_formclass_table[0]; ; tab++) {
        if (!tab->attr && !tab->formclass && !tab->section) {
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
void * threekey_attr_count_base; /* for attr only */
void * threekey_form_count_base; /* for form only */
