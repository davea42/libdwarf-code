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

#ifndef ATTR_FORM_H
#define ATTR_FORM_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define AF_STD   1
#define AF_EXTEN 2

struct Three_Key_Entry_s {
    Dwarf_Half  key1; /* usually AT number */
    Dwarf_Half  key2; /* usually  FORM_CLASS number */
    Dwarf_Half  key3; /* usually actual DW_FORM number,
        but for a record from the preset table, this is zero
        as there the actual FORM is unknown. */
    Dwarf_Small std_or_exten; /* 1: std 2: extension */
    Dwarf_Small from_tables; /* 1 if found in preset table
        else 0 meaning found only in the DWARF. */
    Dwarf_Unsigned count; /* The number actually encountered */
};
typedef struct Three_Key_Entry_s Three_Key_Entry;

/* Returns DW_DLV_ERROR if out of memory, else DW_DLV_OK */
int make_3key(Dwarf_Half k1,
    Dwarf_Half k2,
    Dwarf_Half k3,
    Dwarf_Small std_or_exten,
    Dwarf_Small from_preset,
    Dwarf_Unsigned count,
    Three_Key_Entry ** out);

Dwarf_Unsigned three_key_entry_count(void *base);
void free_func_3key_entry(void *keystructptr);
int  std_compare_3key_entry(const void *l, const void *r);
int  build_attr_form_base_tree(int*errnum);
void destroy_attr_form_trees(void);
void record_attr_form_use(Dwarf_Debug dbg,
    Dwarf_Half tag, Dwarf_Half attr,
    Dwarf_Half fclass, Dwarf_Half form,
    int pd_dwarf_names_print_on_error,
    int die_stack_indent_level);

/*  The standard main tree for attr_form data.
    Starting out as simple global variables. */
extern void * threekey_attr_form_base; /* for attr-form combos */
void print_attr_form_usage(int poe);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ATTR_FORM_H */
