/*
  Copyright (C) 2000,2004 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2002-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2008-2017 David Anderson, Inc. All rights reserved.
  Portions Copyright 2012 SN Systems Ltd. All rights reserved.

  This program is free software; you can redistribute it
  and/or modify it under the terms of version 2.1 of the
  GNU Lesser General Public License as published by the Free
  Software Foundation.

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

  You should have received a copy of the GNU Lesser General
  Public License along with this program; if not, write the
  Free Software Foundation, Inc., 51 Franklin Street - Fifth
  Floor, Boston MA 02110-1301, USA.

*/

#include "config.h"
#include "libdwarfdefs.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#ifdef HAVE_STDINT_H
#include <stdint.h> /* For uintptr_t */
#endif /* HAVE_STDINT_H */
#include "pro_incl.h"
#include "dwarf.h"
#include "libdwarf.h"
#include "pro_opaque.h"
#include "pro_error.h"
#include "pro_encode_nm.h"
#include "pro_alloc.h"
#include "pro_line.h"
#include "memcpy_swap.h"
#include "pro_section.h"        /* for MAGIC_SECT_NO */
#include "pro_reloc_symbolic.h"
#include "pro_reloc_stream.h"
#include "dwarf_tsearch.h"
#include "dwarfstring.h"

#define IS_64BITPTR(dbg) ((dbg)->de_flags & DW_DLC_POINTER64 ? 1 : 0)
#define ISA_IA64(dbg) ((dbg)->de_flags & DW_DLC_ISA_IA64 ? 1 : 0)

struct isa_relocs_s {
   const char *name_;
   int         reloc32_;
   int         reloc64_;
   int         segrel_; /* only used if IRIX */
};

#ifndef TRUE
#define TRUE 1
#endif /*TRUE*/
#ifndef FALSE
#define FALSE 0
#endif /*FALSE*/

/*  Some of these may be the wrong relocation for DWARF
    relocations. FIXME. Most will be unusable without
    additional effort as they have not been tested.
*/
#define R_MIPS_32		2
#define R_MIPS_64		18
#define R_MIPS_SCN_DISP		32
#define R_386_32                 1
#define R_386_64                 0  /* impossible */
#define R_X86_64_32             10
#define R_X86_64_64              1
#define R_SPARC_UA32            23
#define R_SPARC_UA64            54
#define R_ARM_ABS32              2
#define R_ARM_ABS64              0 /* impossible */
#define R_AARCH64_ABS32        258
#define R_AARCH64_ABS64        257
#define R_IA64_DIR32LSB       0x25
#define R_IA64_DIR64LSB       0x27
#define R_PPC_REL32             26
#define R_PPC_REL64             44
#define R_PPC64_REL32         R_PPC_REL32
#define R_PPC64_REL64           44


static struct isa_relocs_s isa_relocs[] = {
{"irix",  R_MIPS_32,R_MIPS_64,R_MIPS_SCN_DISP},
{"mips",  R_MIPS_32,R_MIPS_64,0},
{"x86",   R_386_32, R_386_64,0},
{"x86_64",R_X86_64_32,R_X86_64_64,0},
{"ia64",  R_IA64_DIR32LSB,R_IA64_DIR64LSB,0},
{"arm64", R_AARCH64_ABS32,R_AARCH64_ABS64,0},
{"arm",   R_ARM_ABS32,R_ARM_ABS64,0},
{"ppc",   R_PPC_REL32,R_PPC_REL64,0},
{"ppc64", R_PPC64_REL32,R_PPC64_REL64,0},
{"sparc", R_SPARC_UA32,R_SPARC_UA64,0},
/*  The last entry MUST be all zeros. */
{0,0,0,0}
};


static int common_init(Dwarf_P_Debug dbg, Dwarf_Unsigned flags,
    const char *abiname, const char *dwarf_version,
    const char *extrainfo,
    int *error_ret);

/*  This function sets up a new dwarf producing region.
    flags: Indicates type of access method, one of DW_DLC* macros
    func(): Used to create a new object file, a call back function
    errhand(): Error Handler provided by user
    errarg: Argument to errhand()
    error: returned error value */
    /*  We want the following to have an elf section number that matches
        'nothing' */
static struct Dwarf_P_Section_Data_s init_sect = {
    MAGIC_SECT_NO, 0, 0, 0, 0
};
static struct Dwarf_P_Section_Data_s init_sect_debug_str = {
    MAGIC_SECT_NO, 0, 0, 0, 0
};
static struct Dwarf_P_Section_Data_s init_sect_debug_line_str = {
    MAGIC_SECT_NO, 0, 0, 0, 0
};

/*  New April 2014.
    Replaces all previous producer init functions.
    It adds a string to select the relevant ABI/ISA and
    a string defining the selected DWARF version to
    output.
    There are some overlaps between the flags and the ISA/ABI
    string choices. ( it is neither strictly ABI nor strictly
    ISA name, but a useful name for both.)
    Generally, the function inteprets these
    in a tolerant fashion, so inconsistencies in the
    selections are not noticed...but they may have a surprising
    effect.

    The extra string is a way to allow new options without
    changing the interface. The idea is the caller might
    supply a list of such things as one string, comma-separated.
    The interface is not intended to allow spaces or tabs in the
    names, so don't do that  :-)
    If no extra strings are needed (none are defined initially)
    then pass a NULL pointer or an empty string as the 'extra'
    parameter.
    */
int
dwarf_producer_init(Dwarf_Unsigned flags,
    Dwarf_Callback_Func func,
    Dwarf_Handler errhand,
    Dwarf_Ptr errarg,
    void * user_data,
    const char *isa_name, /* See isa_reloc_s. */
    const char *dwarf_version, /* V2 V3 V4 or V5. */
    UNUSEDARG const char *extra, /* Extra input strings, comma separated. */
    Dwarf_P_Debug *dbg_returned,
    Dwarf_Error * error)
{
    Dwarf_P_Debug dbg = 0;
    int res = 0;
    int err_ret = 0;
    dbg = (Dwarf_P_Debug) _dwarf_p_get_alloc(NULL,
        sizeof(struct Dwarf_P_Debug_s));
    if (dbg == NULL) {
        DWARF_P_DBG_ERROR(dbg, DW_DLE_DBG_ALLOC,
            DW_DLV_ERROR);
    }
    memset((void *) dbg, 0, sizeof(struct Dwarf_P_Debug_s));
    /* For the time being */
    if (func == NULL) {
        DWARF_P_DBG_ERROR(dbg, DW_DLE_NO_CALLBACK_FUNC,
            DW_DLV_ERROR);
    }
    dbg->de_callback_func = func;
    dbg->de_errhand = errhand;
    dbg->de_errarg = errarg;
    dbg->de_user_data = user_data;
    res = common_init(dbg, flags,isa_name,dwarf_version,
        extra,&err_ret);
    if (res != DW_DLV_OK) {
        DWARF_P_DBG_ERROR(dbg, err_ret, DW_DLV_ERROR);
    }
    *dbg_returned = dbg;
    return DW_DLV_OK;
}

int
dwarf_pro_set_default_string_form(Dwarf_P_Debug dbg,
   int form,
   UNUSEDARG Dwarf_Error * error)
{
    if (form != DW_FORM_string &&
        form != DW_FORM_strp) {
        _dwarf_p_error(dbg, error, DW_DLE_BAD_STRING_FORM);
        return DW_DLV_ERROR;
    }
    dbg->de_debug_default_str_form = form;
    return DW_DLV_OK;
}

static int
set_reloc_numbers(Dwarf_P_Debug dbg,
    UNUSEDARG Dwarf_Unsigned flags,
    const char *abiname)
{
    struct isa_relocs_s *isap = 0;
    if (!abiname) {
        return DW_DLV_NO_ENTRY;
    }
    for(isap = &isa_relocs[0];  ;isap++) {
        if (!isap->name_) {
            /* No more names known. Never found the one we wanted. */
            return DW_DLV_NO_ENTRY;
        }
        if (!strcmp(abiname,isap->name_)) {
            if (dbg->de_pointer_size == 4) {
                dbg->de_ptr_reloc = isap->reloc32_;
            } else {
                dbg->de_ptr_reloc = isap->reloc64_;
            }
            if (dbg->de_dwarf_offset_size == 4) {
                dbg->de_offset_reloc = isap->reloc32_;
            } else {
                dbg->de_offset_reloc = isap->reloc64_;
            }
            /*  segrel only meaningful for IRIX, otherwise
                harmless, unused. */
            dbg->de_exc_reloc = isap->segrel_;
            return DW_DLV_OK;
        }
    }
    /* UNREACHED */
}

/*  This is the Daniel J Bernstein hash function
    originally posted to Usenet news.
    http://en.wikipedia.org/wiki/List_of_hash_functions or
    http://stackoverflow.com/questions/10696223/reason-for-5381-number-in-djb-hash-function).
    See Also DWARF5 Section 7.33
*/
static DW_TSHASHTYPE
_dwarf_string_hashfunc(const char *str)
{
    DW_TSHASHTYPE up = 0;
    DW_TSHASHTYPE hash = 5381;
    int c  = 0;

    /*  Extra parens suppress warning about assign in test. */
    while ((c = *str++)) {
        hash = hash * 33 + c ;
    }
    up = hash;
    return up;
}
static DW_TSHASHTYPE
key_simple_string_hashfunc(const void *keyp)
{
    struct Dwarf_P_debug_str_entry_s* mt =
        (struct Dwarf_P_debug_str_entry_s*) keyp;
    const char *str = 0;

    if (mt->dse_has_table_offset) {
        /*  ASSERT: mt->dse_dbg->de_debug_str->ds_data not zero. */
        str = (const char *)mt->dse_dbg->de_debug_str->ds_data +
            mt->dse_table_offset;
    } else {
        /*  ASSERT: dse_name != 0 */
        str = (const char *)mt->dse_name;
    }
    return _dwarf_string_hashfunc(str);
}

/* ===== START  Initialization using string=value,string2=valu2 (etc) */
/*  in the producer_init extras string.
    Handles hex and decimal. Not octal.
    Used a very small number of times, so performance
    not an issue. */

/*  err will be used...shortly */
static int
translatetosigned(char *s,Dwarf_Signed *v, UNUSEDARG int *err)
{
    unsigned char *cp = (unsigned char *)s;
    unsigned char *digits = (unsigned char *)s;
    int signmult = 1;
    Dwarf_Signed l = 0;

    if (*cp == '0' && 
        (*(cp+1) == 'x'|| (*(cp+1) == 'X'))) {
        digits += 2;
        cp = digits;
        for( ; *cp; cp++) {
            l = l << 4;
            switch (*cp) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                l += (*cp - '0');
                break; 
            case 'a':
            case 'A':
                l += 10;
                break;
            case 'b':
            case 'B':
                l += 11;
                break;
            case 'c':
            case 'C':
                l += 12;
                break;
            case 'd':
            case 'D':
                l += 13;
                break;
            case 'e':
            case 'E':
                l += 14;
                break;
            case 'f':
            case 'F':
                l += 15;
                break;
            default:
                /* FIXME *err */
                return DW_DLV_ERROR;
            }
        }
        *v = l;
        return DW_DLV_OK;
    } else if (*cp == '-') {
        signmult = -1;
        digits += 1;
    }

    cp = digits;
    for( ; *cp; cp++) {
        l = l * 10;
        /* Duff's Device */
        switch (*cp) {
        case '9': 
        case '8': 
        case '7': 
        case '6':
        case '5':
        case '4':
        case '3':
        case '2':
        case '1':
        case '0':
            l +=  (*cp - '0');
            break;
        default:
            /* FIXME *err */
            return DW_DLV_ERROR;
        }
    }
    *v = signmult * l;
    return DW_DLV_OK;
}

static int
update_named_field(Dwarf_P_Debug dbg, dwarfstring *cmsname,dwarfstring *cmsvalue,
    int *err)
{
     char *name = dwarfstring_string(cmsname);
     char *value = dwarfstring_string(cmsvalue);
     Dwarf_Signed v = 0; 
     int res;

     res = translatetosigned(value,&v,err);
     if (res != DW_DLV_OK) {
         return res;
     }
     if ( dwarfstring_strlen(cmsvalue) == 0) {
         return DW_DLV_NO_ENTRY;
     }
 
     /*  The value in the string is a number,
         but always quite a small number. */
     if (!strcmp(name,"default_is_stmt")) {
         dbg->de_line_inits.pi_default_is_stmt = (unsigned)v; 
     } else if (!strcmp(name,"minimum_instruction_length")) { 
         dbg->de_line_inits.pi_minimum_instruction_length = (unsigned)v;
     } else if (!strcmp(name,"maximum_operations_per_instruction")) { 
         dbg->de_line_inits.pi_maximum_operations_per_instruction = (unsigned)v;
     } else if (!strcmp(name,"maximum_opcode_base")) { 
         dbg->de_line_inits.pi_opcode_base = (unsigned)v;
     } else if (!strcmp(name,"line_base")) { 
         dbg->de_line_inits.pi_line_base = (int)v;
     } else if (!strcmp(name,"line_range")) { 
         dbg->de_line_inits.pi_line_range = (int)v;
     } else if (!strcmp(name,"linetable_version")) { 
         dbg->de_line_inits.pi_linetable_version = (unsigned)v;
     } else if (!strcmp(name,"segment_selector_size")) { 
         dbg->de_line_inits.pi_segment_selector_size = (unsigned)v;
     } else if (!strcmp(name,"segment_size")) { 
         dbg->de_line_inits.pi_segment_size = (unsigned)v;
     } else {
         /* FIXME *err */
         return DW_DLV_ERROR;
     }
     return DW_DLV_OK;
}
static int
update_named_value(Dwarf_P_Debug dbg, dwarfstring*cms,
    int *err)
{
     char * str = dwarfstring_string(cms);
     char *cp = str;
     char * value_start = 0;
     dwarfstring cmsname;
     dwarfstring cmsvalue;
     unsigned slen = 0;
     int res = 0;

     dwarfstring_constructor(&cmsname);
     dwarfstring_constructor(&cmsvalue);
     for ( ; *cp && *cp != '=' && *cp != ' '; cp++) { }
     if (! *cp) {
         /* Ignore this, it's empty or has no =value clause */
         dwarfstring_destructor(&cmsname);
         dwarfstring_destructor(&cmsvalue);
         /* FIXME *err */
         return DW_DLV_ERROR; 
     }
     if (*cp == ' ') { 
         /* Ignore this, trailing spaces, no = is a typo */
         dwarfstring_destructor(&cmsname);
         dwarfstring_destructor(&cmsvalue);
         /* FIXME *err */
         return DW_DLV_ERROR; 
     }
     slen = cp - str;
     dwarfstring_append_length(&cmsname,str,slen);

     cp++;
     value_start = cp;
     for ( ; *cp && *cp != ' '; cp++) { }
     slen = cp - value_start;
     if (slen) {
         dwarfstring_append_length(&cmsvalue,value_start,slen);
     } else {
         dwarfstring_destructor(&cmsname);
         dwarfstring_destructor(&cmsvalue);
         /* Missing its value */
         /* FIXME *err */
         return DW_DLV_ERROR;
     }

     res = update_named_field(dbg,&cmsname,&cmsvalue,err);
     dwarfstring_destructor(&cmsname);
     dwarfstring_destructor(&cmsvalue);
     return res; 
}

/*  Publicly visible in in libdwarf to enable easy testing
    of the code here. */
int
_dwarf_log_extra_flagstrings(Dwarf_P_Debug dbg,
  const char *extra,
  int *err)
{
    unsigned comma_count = 0;
    const char *cstart = extra;
    unsigned curcomma = 0;
    const char *cp = 0;
    int res = 0;

    if (!extra || !*extra) {
        return DW_DLV_NO_ENTRY;
    }
    for (cp = extra; *cp;cp++) {
        if (*cp == ',') {
             ++comma_count;
        }
    }

    for ( ; curcomma <= comma_count; ) {
        dwarfstring cms;
        dwarfstring_constructor(&cms);
        for (cp = cstart; *cp;cp++) {
            if (*cp == ' ') {
               ++cstart;
               continue;
            }
            if (*cp == 0 || *cp == ',') {
                /* end of a control string */
                unsigned len = cp - cstart;
                if (len == 0) { 
                    /* empty comma phrase */
                    if (*cp  == ',') {
                        cstart = cp+1;
                    }
                    break;
                } else {
                    dwarfstring_append_length(&cms,(char *)cstart,len);
                    if (*cp  == ',') {
                        cstart = cp+1;
                    }
                    res = update_named_value(dbg,&cms,err);
                    if (res == DW_DLV_ERROR) {
                        dwarfstring_destructor(&cms);
                        return res;
                    }
                }
            }
        }
        dwarfstring_destructor(&cms);
    }
    return DW_DLV_OK;
}

/* ===== end  Initialization using string=value,string2=valu2 (etc) */

static int
common_init(Dwarf_P_Debug dbg, Dwarf_Unsigned flags, const char *abiname,
    const char *dwarf_version,
    const char *extra,
    int *err_ret)
{
    unsigned int k = 0;
    int res = 0;

    dbg->de_version_magic_number = PRO_VERSION_MAGIC;
    dbg->de_n_debug_sect = 0;
    dbg->de_debug_sects = &init_sect;
    dbg->de_debug_str = &init_sect_debug_str;
    dbg->de_debug_line_str = &init_sect_debug_line_str;
    dbg->de_current_active_section = &init_sect;
    dbg->de_flags = flags;


    /* DW_DLC_POINTER32 assumed. */
    dbg->de_pointer_size = 4;
    /* Standard DWARF 64bit offset, length field 12 bytes */
    dbg->de_dwarf_offset_size = 4;
    dbg->de_elf_offset_size = 4;
    dbg->de_64bit_extension = 0;

    dbg->de_big_endian = (dbg->de_flags&DW_DLC_TARGET_BIGENDIAN)?
        TRUE:FALSE;
    /*  DW_DLC_POINTER64 is identical to DW_DLC_SIZE_64 */
    if(dbg->de_flags & DW_DLC_POINTER64) {
        dbg->de_pointer_size = 8;
    }
    if(dbg->de_flags & DW_DLC_OFFSET64) {
        dbg->de_pointer_size = 8;
        dbg->de_dwarf_offset_size = 4;
        dbg->de_64bit_extension = 1;
        dbg->de_elf_offset_size = 8;
    } else {
        if(dbg->de_flags & DW_DLC_IRIX_OFFSET64) {
            dbg->de_pointer_size = 8;
            dbg->de_big_endian = TRUE;
            dbg->de_dwarf_offset_size = 8;
            dbg->de_64bit_extension = 0;
            dbg->de_elf_offset_size = 8;
        }
    }
    _dwarf_init_default_line_header_vals(dbg);
    res = _dwarf_log_extra_flagstrings(dbg,extra,err_ret);
    if (res == DW_DLV_ERROR) {
       return res;
    }

    if(abiname && (!strcmp(abiname,"irix"))) {
        dbg->de_irix_exc_augmentation = 1;
    } else {
        dbg->de_irix_exc_augmentation = 0;
    }
    /*  We must set reloc numbers even if doing symbolic
        relocations because we use the numbers up until
        we are generating debug.  A zero is interpreted
        as no relocations.  So ensure we have real
        relocations. */
    res = set_reloc_numbers(dbg,flags,abiname);
    if (res != DW_DLV_OK) {
        *err_ret = DW_DLE_BAD_ABINAME;
        return DW_DLV_ERROR;
    }
    dbg->de_output_version = 2;
    if(dwarf_version) {
        if (!strcmp(dwarf_version,"V2")) {
            dbg->de_output_version = 2;
        } else if (!strcmp(dwarf_version,"V3")) {
            dbg->de_output_version = 3;
        } else if (!strcmp(dwarf_version,"V4")) {
            dbg->de_output_version = 4;
        } else if (!strcmp(dwarf_version,"V5")) {
            dbg->de_output_version = 5;
        } else {
            *err_ret = DW_DLE_VERSION_STAMP_ERROR;
            return DW_DLV_ERROR;
        }
    }
    if (flags & DW_DLC_SYMBOLIC_RELOCATIONS) {
        dbg->de_relocation_record_size =
            sizeof(struct Dwarf_Relocation_Data_s);
    } else {
        /*  This is only going to work when the HOST == TARGET,
            surely? */
#ifdef DWARF_WITH_LIBELF
#if HAVE_ELF64_GETEHDR
        dbg->de_relocation_record_size =
            ((dbg->de_pointer_size == 8)? sizeof(REL64) : sizeof(REL32));
#else
        dbg->de_relocation_record_size = sizeof(REL32);
#endif
#else /* DWARF_WITH_LIBELF */
        *err_ret = DW_DLE_NO_STREAM_RELOC_SUPPORT;
        return DW_DLV_ERROR;
#endif /* DWARF_WITH_LIBELF */


    }

    /* For .debug_str creation. */
    dwarf_initialize_search_hash(&dbg->de_debug_str_hashtab,
        key_simple_string_hashfunc,0);
    dbg->de_debug_default_str_form = DW_FORM_string;
    dwarf_initialize_search_hash(&dbg->de_debug_line_str_hashtab,
        key_simple_string_hashfunc,0);

    /* FIXME: conditional on the DWARF version target,
        dbg->de_output_version. */
    if (dbg->de_dwarf_offset_size == 8) {
        dbg->de_ar_data_attribute_form = DW_FORM_data8;
        dbg->de_ar_ref_attr_form = DW_FORM_ref8;
    } else {
        dbg->de_ar_data_attribute_form = DW_FORM_data4;
        dbg->de_ar_ref_attr_form = DW_FORM_ref4;
    }

    if (flags & DW_DLC_SYMBOLIC_RELOCATIONS) {
        dbg->de_relocate_by_name_symbol =
            _dwarf_pro_reloc_name_symbolic;
        dbg->de_relocate_pair_by_symbol =
            _dwarf_pro_reloc_length_symbolic;
        dbg->de_transform_relocs_to_disk =
            _dwarf_symbolic_relocs_to_disk;
    } else {
#ifdef DWARF_WITH_LIBELF
        if (IS_64BITPTR(dbg)) {
            dbg->de_relocate_by_name_symbol =
                _dwarf_pro_reloc_name_stream64;
        } else {
            dbg->de_relocate_by_name_symbol =
                _dwarf_pro_reloc_name_stream32;
        }
        dbg->de_relocate_pair_by_symbol = 0;
        dbg->de_transform_relocs_to_disk = _dwarf_stream_relocs_to_disk;
#else /* DWARF_WITH_LIBELF */
        *err_ret = DW_DLE_NO_STREAM_RELOC_SUPPORT;
        return DW_DLV_ERROR;
#endif /* DWARF_WITH_LIBELF */
    }
    for (k = 0; k < NUM_DEBUG_SECTIONS; ++k) {

        Dwarf_P_Per_Reloc_Sect prel = &dbg->de_reloc_sect[k];

        prel->pr_slots_per_block_to_alloc = DEFAULT_SLOTS_PER_BLOCK;
    }
    /* First assume host, target same endianness  FIXME */
    dbg->de_same_endian = 1;
    dbg->de_copy_word =  _dwarf_memcpy_noswap_bytes;
#ifdef WORDS_BIGENDIAN
    /* host is big endian, so what endian is target? */
    if (flags & DW_DLC_TARGET_LITTLEENDIAN) {
        dbg->de_same_endian = 0;
        dbg->de_copy_word = _dwarf_memcpy_swap_bytes;
    }
#else /* little endian */
    /* host is little endian, so what endian is target? */
    if (flags & DW_DLC_TARGET_BIGENDIAN) {
        dbg->de_same_endian = 0;
        dbg->de_copy_word = _dwarf_memcpy_swap_bytes;
    }
#endif /* !WORDS_BIGENDIAN */
    return DW_DLV_OK;
}
