/*
Copyright (c) 2020, David Anderson
All rights reserved.

This software file is hereby placed in the public domain.
For use by anyone for any purpose.

*/
/* opscounttab.h */

struct dwarf_opscounttab_s {
    signed   char oc_opcount;
};

#define DWOPS_ARRAY_SIZE 256
extern struct dwarf_opscounttab_s dwarf_opscounttab[DWOPS_ARRAY_SIZE];
