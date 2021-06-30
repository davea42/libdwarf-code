/*  Copyright (c) 2021 David Anderson
    This test code is hereby placed in the public domain
    for anyone to use in any way.  */
#include "config.h"
#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for free(). */
#endif /* HAVE_STDLIB_H */
#include <stdio.h> /* For debugging. */
#ifdef HAVE_STDINT_H
#include <stdint.h> /* For uintptr_t */
#endif /* HAVE_STDINT_H */
#if defined(_WIN32) && defined(HAVE_STDAFX_H)
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */
#ifdef HAVE_STRING_H
#include <string.h>  /* strcpy() strlen() */
#endif
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif
#include "libdwarf_private.h"
#include "dwarf.h"
#include "libdwarf.h"
#include "dwarf_base_types.h"
#include "dwarf_opaque.h"
#include "dwarf_error.h"

#if 0
struct Dwarf_Obj_Access_Interface_s {
    /*  object is a void* as it hides the data the
        object access routines
        need (which varies by library in use and object format).
    */
    void* object;
    const Dwarf_Obj_Access_Methods * methods;
};
#endif

static  Dwarf_Obj_Access_Section oursection =
{0,0,1,".debug_info",0,1,0};

static
int gsinfo(void* obj UNUSEDARG, 
    Dwarf_Half section_index,
    Dwarf_Obj_Access_Section* return_section,
    int* error)
{
     /*  Here section indexes start at 0. 
         In Elf the zero index is always empty.
         See the comments in libdwarf.h:
         struct Dwarf_Obj_Access_Methods_s */
     if (section_index == 0) {
         /* Fake up section data */
         *return_section = oursection;
         return DW_DLV_OK;
     }
     *error = DW_DLE_INIT_ACCESS_WRONG;
     return DW_DLV_ERROR;
}
static
Dwarf_Endianness gborder(void * obj UNUSEDARG)
{
     return DW_OBJECT_MSB;
}
static
Dwarf_Small glensize(void * obj UNUSEDARG)
{
     return 4;
}
static
Dwarf_Small gptrsize(void * obj UNUSEDARG)
{
     return 4;
}
static
Dwarf_Unsigned gseccount(void* obj UNUSEDARG)
{
     return 1;
}
static
int gloadsec(void * obj UNUSEDARG,
    Dwarf_Half secindex UNUSEDARG,
    Dwarf_Small**rdata  UNUSEDARG,
    int *error)
{
    *error = 1;
    return DW_DLV_ERROR;
}
const Dwarf_Obj_Access_Methods methods= { gsinfo,  
    gborder,
    glensize,
    gptrsize,
    gseccount,
    gloadsec,
    0 /* no relocating anything */
    };


Dwarf_Obj_Access_Interface iface = {
    0,&methods
};

/*  testing interfaces useful for embedding
    libdwarf inside another program or library. */
int main()
{
    int res = 0;
    int errcount = 0;
    Dwarf_Debug dbg = 0;
    Dwarf_Error error = 0;

    res = dwarf_object_init_b(&iface,
        0,0,DW_GROUPNUMBER_ANY,&dbg,
        &error);
    if (res != DW_DLV_OK) {
        if (res == DW_DLV_NO_ENTRY) {
            printf("FAIL Cannot dwarf_object_init_b() NO ENTRY. \n");
        }
        if (res == DW_DLV_ERROR) {
            printf("FAIL Cannot dwarf_object_init_b(). \n");
            printf("msg: %s\n",dwarf_errmsg(error));
        }
        exit(1);
    }
    dwarf_object_finish(dbg,&error);
    if (res != DW_DLV_OK) {
        if (res == DW_DLV_NO_ENTRY) {
            printf("FAIL Cannot dwarf_object_finish() NO ENTRY. \n");
        }
        if (res == DW_DLV_ERROR) {
            printf("FAIL Cannot  dwarf_object_finish(). \n");
            printf("msg: %s\n",dwarf_errmsg(error));
        }
        exit(1);
    }


    if (errcount) {
       printf("FAIL objectaccess.c\n");
       exit(1);
    }
    return 0;
}
