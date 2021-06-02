/*
Copyright (c) 2020, David Anderson
All rights reserved.

This software file is hereby placed in the public domain.
For use by anyone for any purpose.
*/

/* This uses this condensed table to make
   a simple fast-access C table.
   Build and run with
   cc -I ../libdwarf buildopscounttab.c dwarf_names.c -o buildop
   ./buildop >opscounttab.c

*/

#include <stdio.h>
#include "dwarf.h"
#include "dwarf_names.h"
#include "opscounttab.h"
struct ops_table_s {
    unsigned char ot_first;
    unsigned char ot_last;
    signed   char ot_opcount;
};
/*  Must match libdwarf.h macros */
#define DW_DLV_OK 0
#define DW_DLV_ERROR -1

struct ops_table_s optabsource[]= {
{DW_OP_addr  ,         0                       , 1 },
{DW_OP_deref  ,        0                       , 0 },
{DW_OP_const1u,        DW_OP_consts            , 1},
{DW_OP_dup,            DW_OP_over              , 0},
{DW_OP_pick,           0                       , 1},
{DW_OP_swap,           DW_OP_plus              , 0},
{DW_OP_plus_uconst,    0                       , 1},
{DW_OP_shl,            DW_OP_xor               , 0},
{DW_OP_bra ,           0                       , 1},
{DW_OP_eq,             DW_OP_ne                , 0},
{DW_OP_skip,           0                       , 1},
{DW_OP_lit0  ,         DW_OP_lit31             , 0},
{DW_OP_reg0  ,         DW_OP_reg31             , 0},
{DW_OP_breg0 ,         DW_OP_breg31            , 1},
{DW_OP_regx  ,         DW_OP_fbreg             , 1},
{DW_OP_bregx,          0                       , 2},
{DW_OP_piece,          DW_OP_xderef_size       , 1},
{DW_OP_nop  ,          DW_OP_push_object_address ,0},
{DW_OP_call2,          DW_OP_call_ref          , 1},
{DW_OP_form_tls_address, DW_OP_call_frame_cfa  , 0},
{DW_OP_bit_piece,      DW_OP_implicit_value    , 2},
{DW_OP_stack_value,            0               , 0},
{DW_OP_implicit_pointer,       0               , 2},
{DW_OP_addrx,          DW_OP_constx            , 1},
{DW_OP_entry_value,             0              , 2},
{DW_OP_const_type,              0              , 3},
{DW_OP_regval_type, DW_OP_deref_type           , 2},
{DW_OP_xderef_type,    0, 0},
{DW_OP_convert, DW_OP_reinterpret             , 1},
{DW_OP_GNU_push_tls_address,  0,0 },
{DW_OP_GNU_uninit,              0              , 0},
{DW_OP_GNU_encoded_addr,        0              , 1},
{DW_OP_GNU_implicit_pointer,DW_OP_GNU_entry_value , 2},
{DW_OP_GNU_const_type,          0              , 3},
{DW_OP_GNU_regval_type,DW_OP_GNU_deref_type    , 2},
{DW_OP_GNU_convert,    DW_OP_GNU_variable_value, 1},
{0,0}
};

int main()
{
    struct ops_table_s *op;
    int inindex = 0;
    int outindex = 0;
    int f = 0;
    int l = 0;
    int c = 0;
    int res = 0;
    int lastop = 0;

    printf("/*  Generated expression ops table,\n");
    printf("    do not edit. */\n");
    printf("#include \"opscounttab.h\"\n");
    printf("\n");
    printf("struct dwarf_opscounttab_s dwarf_opscounttab[] = {\n");
    for ( ;  ; ++inindex) {
        const char *opn = 0;

        op = &optabsource[inindex];
        f = op->ot_first;
        if (!f) {
            break;
        }
        if (lastop && f < lastop) {
            printf("FAILED buildopscounttab on OP,out of sequence"
                " f=0x%x lastop=0x%x\n",
                (unsigned)f,(unsigned)lastop);
            return 1; /* effectively exit(1) */
        }
        l = op->ot_last;
        c = op->ot_opcount;
        while (f > outindex) {
            printf("{/* %-26s 0x%02x*/ %d},\n","unused",outindex,-1);
            ++outindex;
        }
        if (!l) {
            res = dwarf_get_OP_name(f,&opn);
            if (res != DW_DLV_OK) {
                printf("FAILED buildopscounttab on OP 0x%x\n",
                    f);
                return 1; /* effectively exit(1) */
            }
            lastop = f;
            printf("{/* %-26s 0x%02x*/ %d},\n",opn,f,c);
            ++outindex;
        } else {
            int j = f;
            for ( ; j <= l; ++j) {
                res = dwarf_get_OP_name(j,&opn);
                if (res != DW_DLV_OK) {
                    printf("FAILED buildopscounttab on OP 0x%x\n",
                        f);
                    return 1; /* effectively exit(1); */
                }
                printf("{/* %-26s 0x%2x*/ %d},\n",opn,j,c);
                ++outindex;
                lastop = j;
            }
        }
    }
    while (outindex < DWOPS_ARRAY_SIZE) {
        printf("{/* %-26s 0x%02x*/ %d},\n","unused",outindex,-1);
        ++outindex;
    }
    printf("};\n");
    return 0;
}
