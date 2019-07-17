/*
  Copyright (c) 2009-2019 David Anderson.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the example nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY David Anderson ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL David
  Anderson BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
/*  findfuncbypc.c
    This is an example of code reading dwarf .debug_info.
    It is kept simple to expose essential features.

    It does not do all possible error reporting or error handling.
    It does to a bit of error checking as a help in ensuring
    that some code works properly... for error checks.


    To use, try
        make
        ./findfuncbypc --pc=0x10000 ./findfuncbypc
*/
#include "config.h"
#ifdef HAVE_UNUSED_ATTRIBUTE
#define  UNUSEDARG __attribute__ ((unused))
#else
#define  UNUSEDARG
#endif
/* Windows specific header files */
#if defined(_WIN32) && defined(HAVE_STDAFX_H)
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */

#include <sys/types.h> /* For open() */
#include <sys/stat.h>  /* For open() */
#include <fcntl.h>     /* For open() */
#include <stdlib.h>     /* For exit() */
#ifdef HAVE_UNISTD_H
#include <unistd.h>     /* For close() */
#elif defined(_WIN32) && defined(_MSC_VER)
#include <io.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <string.h>
#ifdef HAVE_STDINT_H
#include <stdint.h> /* For uintptr_t */
#endif /* HAVE_STDINT_H */
#include "dwarf.h"
#include "libdwarf.h"

#ifndef O_RDONLY
/*  This is for a Windows environment */
# define O_RDONLY _O_RDONLY
#endif

#ifdef _O_BINARY
/*  This is for a Windows environment */
#define O_BINARY _O_BINARY
#else
# ifndef O_BINARY
# define O_BINARY 0  /* So it does nothing in Linux/Unix */
# endif
#endif /* O_BINARY */

struct srcfilesdata {
    char ** srcfiles;
    Dwarf_Signed srcfilescount;
    int srcfilesres;
};

#define TRUE 1
#define FALSE 0

static void read_cu_list(Dwarf_Debug dbg);
static int examine_die_data(Dwarf_Debug dbg, Dwarf_Die die,
    int level,
    struct target_data_s *td,
    Dwarf_Error *errp);
static int examine_comp_dir(Dwarf_Debug dbg,Dwarf_Die die,
    int level, struct target_data_s *td,Dwarf_Error *errp)
static int get_die_and_siblings(Dwarf_Debug dbg, 
    Dwarf_Die in_die,
    int is_info, int in_level,
    struct target_data_s *td,
    Dwarf_Error *errp);

struct target_data_s {
    Dwarf_Unsigned td_target_pc;

    /*  cu die data. */
    Dwarf_Unsigned td_cu_lowpc;
    Dwarf_Unsigned td_cu_highpc;
    int            td_cu_haslowhighpc;
    Dwarf_Die      td_cu_die;
    struct srcfilesdata td_cu_srcfiles;

    /*  DIE data of appropriate DIE */
    /*  From DW_AT_name */
    char *         td_subprog_name;
    Dwarf_Die      td_suprog_die;
    Dwarf_Unsigned td_subprog_lowpc;
    Dwarf_Unsigned td_subprog_highpc;
    int            td_subprog_haslowhighpc;
} target_data;
/* Adding return codes to DW_DLV, relevant to our purposes here. */
#define NOT_THIS_CU 10
#define IN_THIS_CU 11
#define FOUND_SUBPROG 12

#if 0
DW_UT_compile                   0x01  /* DWARF5 */
DW_UT_type                      0x02  /* DWARF5 */
DW_UT_partial                   0x03  /* DWARF5 */
#endif

static int unittype      = DW_UT_compile;
static Dwarf_Bool g_is_info = TRUE;

int cu_version_stamp = 0;
int cu_offset_size = 0;

static int
startswithextractnum(const char *arg,const char *lookfor, Dwarf_Unsigned *numout)
{
    const char *s = 0;
    unsigned prefixlen = strlen(lookfor);
    Dwarf_Unsigned v = 0;
    if(strncmp(arg,lookfor,prefixlen)) {
        return FALSE;
    }
    s = arg+prefixlen;
    /*  We are not making any attempt to deal with garbage.
        Pass in good data... */
    v = atol(s);
    *numout = v;
    return TRUE;
}

int
main(int argc, char **argv)
{
    Dwarf_Debug dbg = 0;
    const char *filepath = 0;
    int res = DW_DLV_ERROR;
    Dwarf_Error error;
    Dwarf_Handler errhand = 0;
    Dwarf_Ptr errarg = 0;
    Dwarf_Error *errp  = 0;
    int i = 0;
    #define MACHO_PATH_LEN 2000
    char macho_real_path[MACHO_PATH_LEN];

    macho_real_path[0] = 0;
    for(i = 1; i < (argc-1) ; ++i) {
        if(startswithextractnum(argv[i],"--pc=",
            &target_data.td_target_pc)) {
            /* done */
        } else {
            printf("Unknown argument \"%s\", give up \n",argv[i]);
            exit(1);
        }
    }
    filepath = argv[i];
    errp = &error;
    res = dwarf_init_path(filepath,
        macho_real_path,
        MACHO_PATH_LEN,
        DW_DLC_READ,
        DW_GROUPNUMBER_ANY,errhand,errarg,&dbg,
        0,0,0,errp);
    if(res != DW_DLV_OK) {
        printf("Giving up, cannot do DWARF processing\n");
        exit(1);
    }
    read_cu_list(dbg);
    res = dwarf_finish(dbg,errp);
    if(res != DW_DLV_OK) {
        printf("dwarf_finish failed!\n");
    }
    return 0;
}

static void
resetsrcfiles(Dwarf_Debug dbg,struct srcfilesdata *sf)
{
    Dwarf_Signed sri = 0;
    for (sri = 0; sri < sf->srcfilescount; ++sri) {
        dwarf_dealloc(dbg, sf->srcfiles[sri], DW_DLA_STRING);
    }
    dwarf_dealloc(dbg, sf->srcfiles, DW_DLA_LIST);
    sf->srcfilesres = DW_DLV_ERROR;
    sf->srcfiles = 0;
    sf->srcfilescount = 0;
}
static void resetsubprog(struct target_data_s *td)
{
    td->td_subprog_haslowhighpc = FALSE;
    if (td->td_subprog_name) {
        dwarf_dealloc(dbg,td->td_subprog_name,DW_DLA_STRING);
        td->subprog_name  = 0;
    }
    if (td->tc_subprog_die) {
        dwarf_dealloc(dbg,td->tc_subprog_die,DW_DLA_DIE);
        td->tc_subprog_die = 0;
    }

}
static void
reset_target_data(Dwarf_Debug dbg,struct target_data_s *td)
{
    if (td->tc_cu_die) {
        dwarf_dealloc(dbg,td->tc_cu_die,DW_DLA_DIE);
        td->tc_cu_die = 0;
    }
    td->td_cu_haslowhighpc = FALSE;
    resetsrcfiles(dbg,&td->td_cu_srcfiles);
    resetsubprog(dbg,td);
FIXME
}

static void
read_cu_list(Dwarf_Debug dbg)
{
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Unsigned abbrev_offset = 0;
    Dwarf_Half     address_size = 0;
    Dwarf_Half     version_stamp = 0;
    Dwarf_Half     offset_size = 0;
    Dwarf_Half     extension_size = 0;
    Dwarf_Unsigned typeoffset = 0;
    Dwarf_Half     header_cu_type = unittype;
    Dwarf_Bool     is_info = g_is_info;
    Dwarf_Error error;
    int cu_number = 0;
    Dwarf_Error *errp  = 0;
    struct target_data_s target_data;

    errp = &error;
    for(;;++cu_number) {
        Dwarf_Die no_die = 0;
        Dwarf_Die cu_die = 0;
        int res = DW_DLV_ERROR;
        Dwarf_Sig8     signature;
    
        memset(&signature,0, sizeof(signature));
        reset_target_data(dbg,&target_data);
        res = dwarf_next_cu_header_d(dbg,is_info,&cu_header_length,
            &version_stamp, &abbrev_offset,
            &address_size, &offset_size,
            &extension_size,&signature,
            &typeoffset, 0,
            &header_cu_type,errp);
        if(res == DW_DLV_ERROR) {
            char *em = errp?dwarf_errmsg(error):"An error next cu her";
            printf("Error in dwarf_next_cu_header: %s\n",em);
            exit(1);
        }
        if(res == DW_DLV_NO_ENTRY) {
            /* Done. */
            return;
        }
        cu_version_stamp = version_stamp;
        cu_offset_size   = offset_size;
        /* The CU will have a single sibling, a cu_die. */
        res = dwarf_siblingof_b(dbg,no_die,is_info,
            &cu_die,errp);
        if(res == DW_DLV_ERROR) {
            char *em = errp?dwarf_errmsg(error):"An error";
            printf("Error in dwarf_siblingof_b on CU die: %s\n",em);
            exit(1);
        }
        if(res == DW_DLV_NO_ENTRY) {
            /* Impossible case. */
            printf("no entry! in dwarf_siblingof on CU die \n");
            exit(1);
        }

        target_data.td_cu_die = cudie;
        res = get_die_and_siblings(dbg,cu_die,is_info,0,&sf,errp);
        if (res == FOUND_SUBPROG) {
            print_target_info(dbg,&target_data);
            reset_target_data(dbg,&target_data);
            return;
        }
        else if (res == IN_THIS_CU) {
            /* This should not happen. Something wrong. */
            FIXME
        }
        else if (res == NOT_THIS_CU) {
            /* This is normal. Keep looking */
        }
        else if (res == DW_DLV_ERROR) {
            /* Lets give up, something badly wrong. */
            FIXME
        }
        else if (res == DW_DLV_NO_ENTRY) {
            /* This is odd. Assume normal. */
        } else {
            /* DW_DLV_OK. normal. */
        }
        reset_target_data(dbg,&target_data);
    }
}

/*  Recursion, following DIE tree. 
    On initial call in_die is a cu_die (in_level 0 )
*/
static int
get_die_and_siblings(Dwarf_Debug dbg, Dwarf_Die in_die,
    int is_info,int in_level,
    struct target_data_s *td,
    Dwarf_Error *errp)
{
    int res = DW_DLV_ERROR;
    Dwarf_Die cur_die=in_die;
    Dwarf_Die child = 0;

    res = examine_die_data(dbg,in_die,in_level,td,errp);
    if (res == DW_DLV_ERROR) {
        printf("Error in die access , level %d \n",in_level);
        exit(1);
    }
    else if (res == DW_DLV_NO_ENTRY) {
        printf("Error in die access NO ENTRY? , level %d \n",in_level);
        exit(1);
    }
    else if ( res == NOT_THIS_CU) { 
        return res;
    } 
    else if ( res == IN_THIS_CU) { 
        /*  Fall through to examine details. */
    } 
    else if ( res == FOUND_SUBPROG) { 
        return res;
    } else {
        /* DW_DLV_OK */
        /*  Fall through to examine details. */
    }
     
    for(;;) {
        Dwarf_Die sib_die = 0;
        res = dwarf_child(cur_die,&child,errp);
        if(res == DW_DLV_ERROR) {
            printf("Error in dwarf_child , level %d \n",in_level);
            exit(1);
        }
        if(res == DW_DLV_OK) {
            get_die_and_siblings(dbg,child,is_info,
                in_level+1,td,errp);
            /* No longer need 'child' die. */
            dwarf_dealloc(dbg,child,DW_DLA_DIE);
            child = 0;
        }
        /* res == DW_DLV_NO_ENTRY or DW_DLV_OK */
        res = dwarf_siblingof_b(dbg,cur_die,is_info,&sib_die,errp);
        if(res == DW_DLV_ERROR) {
            char *em = errp?dwarf_errmsg(error):"Error siblingof_b";
            printf("Error in dwarf_siblingof_b , level %d :%s \n",
                in_level,em);
            exit(1);
        }
        if(res == DW_DLV_NO_ENTRY) {
            /* Done at this level. */
            break;
        }
        /* res == DW_DLV_OK */
        if(cur_die != in_die) {
            dwarf_dealloc(dbg,cur_die,DW_DLA_DIE);
            cur_die = 0;
        }
        cur_die = sib_die;
        res = examine_die_data(dbg,cur_die,in_level,td,errp);
    }
    return;
}
static void
get_addr(Dwarf_Attribute attr,Dwarf_Addr *val)
{
    Dwarf_Error error = 0;
    int res;
    Dwarf_Addr uval = 0;
    Dwarf_Error *errp  = 0;

    errp = &error;
    res = dwarf_formaddr(attr,&uval,errp);
    if(res == DW_DLV_OK) {
        *val = uval;
        return;
    }
    return;
}
static void
get_number(Dwarf_Attribute attr,Dwarf_Unsigned *val)
{
    Dwarf_Error error = 0;
    int res;
    Dwarf_Signed sval = 0;
    Dwarf_Unsigned uval = 0;
    Dwarf_Error *errp  = 0;

    errp = &error;
    res = dwarf_formudata(attr,&uval,errp);
    if(res == DW_DLV_OK) {
        *val = uval;
        return;
    }
    res = dwarf_formsdata(attr,&sval,errp);
    if(res == DW_DLV_OK) {
        *val = sval;
        return;
    }
    return;
}

static int
getlowhighpc(Dwarf_Debug dbg,Dwarf_Die die,
    int  *found_hi_low,
    Dwarf_Addr *lowpc_out,
    Dwarf_Addr *highpc_out,
    Dwarf_Error*error)
{
    Dwarf_Addr hipc = 0;
    *found_hi_low = FALSE;
    res = dwarf_lowpc(die,lowpc_out,error);  
    if (res == DW_DLV_OK) {
      res = dwarf_highpc_b(die,&hipc,&form,&formclass,error);
      if (res == DW_DLV_OK) {
          if (formclass == DW_FORM_CLASS_CONSTANT) {
              hipc += *lowpc_out;
          }
          *hipc_out = hipc;
          *found_hi_low = TRUE;
      }
    }
    return res;
}


static int,
check_subprog(Dwarf_Debug dbg,Dwarf_Die die,
    UNUSEDARG int level,
    struct target_data_s *td,
    char **name_out,
    Dwarf_Addr * lowpc_out,
    Dwarf_Addr * highpc_out,
    Dwarf_Unsigned *linenum_out,
    Dwarf_Error *errp)
{
    int res;
    Dwarf_Error error = 0;
    Dwarf_Attribute *attrbuf = 0;
    Dwarf_Addr lowpc = 0;
    Dwarf_Addr highpc = 0;
    Dwarf_Signed attrcount = 0;
    Dwarf_Signed i;
    Dwarf_Unsigned filenum = 0;
    Dwarf_Unsigned linenum = 0;
    char *filename = 0;
    Dwarf_Error *errp = 0;
    Dwarf_Addr highpc = 0;
    Dwarf_Half form = 0;
    Dwarf_Addr lowpc = 0;
    enum Dwarf_Form_Class formclass;
    int have_pc_range = FALSE;

    res = getlowhighpc(dbg,die,&have_pc_range,&lowpc,&highpc,errp);
    if (res != DW_DLV_OK) {
        return res;
    }
    if (have_pc_range) {
        if (target_data.td_target_pc < lowpc || 
            target_data.td_target_pc >= highpc) {
            /* We have no interest in this subprogram */
            return DW_DLV_OK;
        } else {
            int res2 = DW_DLV_OK;
            target_data.td_subprog_lowpc = lowpc;
            target_data.td_subprog_highpc = highpc;
            target_data.td_subprog_haslowhighpc = have_pc_range;
            res2 = dwarf_diename(die,name_out,errp);
            if (res2 == DW_DLV_OK) {
               target_data.td_subprog_name = *name_out;
            }
             
            return FOUND_SUBPROG;
        }
    }
    return DW_DLV_OK;
}

static int
examine_comp_dir(Dwarf_Debug dbg,Dwarf_Die die,
    int level, struct target_data_s *td,Dwarf_Error *errp)
{
    int res = 0;
    int res2 = 0;
    Dwarf_Attribute *attrbuf = 0;
    Dwarf_Signed attrcount = 0;
    Dwarf_Signed i = 0;
    int have_pc_range = FALSE;
    Dwarf_Addr lowpc = 0;
    Dwarf_Addr highpc = 0;
    Dwarf_Unsigned version = 0;
    Dwarf_Small table_count = 0;
    Dwarf_Line_Context linecontext = 0;
    Dwarf_Line * linebuf = 0;
    Dwarf_Signed linecount = 0;
    Dwarf_Signed baseindex = 0;
    Dwarf_Signed filescount = 0;
    Dwarf_Signed endindex = 0;

    res = getlowhighpc(dbg,die,&have_pc_range,
            &lowpc,&highpc,errp);
    if (res == DW_DLV_OK) {
        if (have_pc_range) {
            if (target_data.td_target_pc < lowpc ||
               target_data.td_target_pc >= highpc) {
               /* We have no interest in this CU */
               res =  NOT_THIS_CU;
               return res;
        } else {
            target_data.td_cu_lowpc = lowpc;
            target_data.td_cu_highpc = highpc;
            target_data.td_cu_haslowhighpc =
                have_pc_range;
            res = IN_THIS_CU;
        }
    }

    /* dwarf pdf page 101 */
    sf->srcfilesres = dwarf_srcfiles(die,&sf->srcfiles,
        &sf->srcfilescount,
        errp);
    if (sf->srcfilesres == DW_DLV_ERROR) {
FIXME
    }

    /* see pdf, page 95 */
    res2 = dwarf_srclines_comp_dir(linecontext,
FIXME  

    /* see pdf, page 92 */
    res2 = dwarf_srclines_b(die,&version,&table_count,
        &linecontext,errp);
FIXME

    /* see dwarf_srclines_dealloc_b()  page 91*/
    res2 = dwarf_srclines_from_linecontext(linecontext,
        &linebuf,&linecount,errp);

    /* files_indexes are page 96 */
    int dwarf_srclines_files_indexes(linecontext,
        &baseindex,
        &filescount,&endindex,errp);

#if 0
    res2 = dwarf_attrlist(die,&attrbuf,&attrcount,errp);
    if(res2 != DW_DLV_OK) {
        return;
    }

    for(i = 0; i < attrcount ; ++i) {
        Dwarf_Half aform;
        res = dwarf_whatattr(attrbuf[i],&aform,errp);
        if(res == DW_DLV_OK) {
            if(aform == DW_AT_comp_dir) {
                char *name = 0;
                res = dwarf_formstring(attrbuf[i],&name,errp);
                if(res == DW_DLV_OK) {
                    printf(    "<%3d> compilation directory : \"%s\"\n",
                        level,name);
                }
            }
            if(aform == DW_AT_stmt_list) {
                /* Offset of stmt list for this CU in .debug_line */
            }
        }
        dwarf_dealloc(dbg,attrbuf[i],DW_DLA_ATTR);
    }
    dwarf_dealloc(dbg,attrbuf,DW_DLA_LIST);
#endif
}

static void
examine_die_data(Dwarf_Debug dbg, Dwarf_Die die,
    int level,
    struct target_data_s *td,
    Dwarf_Error *errp)
{
    char *name = 0;
    Dwarf_Error error = 0;
    Dwarf_Half tag = 0;
    const char *tagname = 0;
    int res = 0;
    Dwarf_Attribute attr = 0;
    Dwarf_Half formnum = 0;
    const char *formname = 0;

    errp = &error;
    res = dwarf_tag(die,&tag,errp);
    if(res != DW_DLV_OK) {
        printf("Error in dwarf_tag , level %d \n",level);
        exit(1);
    }
    if( tag == DW_TAG_subprogram) {
        res = check_subprog(dbg,die,level,sf,0,0,0,0,errp);
        if (res == FOUND_SUBPROG) {
            td->td_subprog_die == die;
            return res;
        } 
        else if (res = DW_DLV_ERROR)  {
            return res;
        }
        else if (res =  DW_DLV_NO_ENTRY) {
            /* impossible? */
            return res;
        }
        else if (res =  NOT_THIS_CU) {
            /* impossible */
            return res;
        }
        else if (res =  IN_THIS_CU) {
            /* impossible */
            return res;
        } else {
            /* DW_DLV_OK */
        }
        return DW_DLV_OK;
    } else if(tag == DW_TAG_compile_unit ||
        tag == DW_TAG_partial_unit ||
        tag == DW_TAG_type_unit) {
        int have_pc_range = FALSE;
        Dwarf_Addr lowpc = 0;
        Dwarf_Addr highpc = 0;

        res = examine_comp_dir(dbg,die,sf);
        res = getlowhighpc(dbg,die,&have_pc_range,
            &lowpc,&highpc,errp);
        if (res == DW_DLV_OK) {
            if (have_pc_range) {
                if (target_data.td_target_pc < lowpc || 
                    target_data.td_target_pc >= highpc) {
                    /* We have no interest in this CU */
                    res =  NOT_THIS_CU;
                } else {
                   target_data.td_cu_die = die;
                   target_data.td_cu_lowpc = lowpc;
                   target_data.td_cu_highpc = highpc;
                   target_data.td_cu_haslowhighpc = 
                       have_pc_range;
                   res = IN_THIS_CU;
                }
            }
        }
        return res;
    }
    /*  Keep looking */
    return DW_DLV_OK;
}
