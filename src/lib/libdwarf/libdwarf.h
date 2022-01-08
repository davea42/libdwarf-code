/*
  Copyright (C) 2000-2010 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2008-2022 David Anderson. All rights reserved.
  Portions Copyright 2008-2010 Arxan Technologies, Inc. All rights reserved.
  Portions Copyright 2010-2012 SN Systems Ltd. All rights reserved.

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

#ifndef _LIBDWARF_H
#define _LIBDWARF_H

#ifdef DW_API
#undef DW_API
#endif /* DW_API */

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef LIBDWARF_BUILD
#define DW_API __declspec(dllexport)
#else
#define DW_API __declspec(dllimport)
#endif /* LIBDWARF_BUILD */
#elif (defined(__SUNPRO_C)  || defined(__SUNPRO_CC))
#ifdef PIC
#define DW_API __global
#else
#define DW_API
#endif /* PIC */
#elif (defined(__GNUC__) && __GNUC__ >= 4) || \
    defined(__INTEL_COMPILER)
#ifdef PIC
#define DW_API __attribute__ ((visibility("default")))
#else
#define DW_API
#endif /* PIC */
#else
#define DW_API
#endif /* _WIN32 || __CYGWIN__ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! @file */
/*! @page libdwarf.h
    @tableofcontents
*/
/*
    libdwarf.h
    $Revision: #9 $ $Date: 2008/01/17 $

    For libdwarf consumers (reading DWARF2 and later)

    The interface is defined as having 8-byte signed and unsigned
    values so it can handle 64-or-32bit target on 64-or-32bit host.
    Dwarf_Ptr is the native size: it represents pointers on
    the host machine (not the target!).

    This contains declarations for types and all producer
    and consumer functions.

    Function declarations are written on a single line each here
    so one can use grep  to each declaration in its entirety.
    The declarations are a little harder to read this way, but...
*/
/*! @section defines  Defines and Types
*/

/* Sematic Version identity for this libdwarf.h */
#define DW_LIBDWARF_VERSION_MAJOR 0
#define DW_LIBDWARF_VERSION_MINOR 3
#define DW_LIBDWARF_VERSION_MICRO 3
#define DW_LIBDWARF_VERSION "0.3.3"
#define DW_PATHSOURCE_unspecified 0
#define DW_PATHSOURCE_basic     1
#define DW_PATHSOURCE_dsym      2 /* MacOS dSYM */
#define DW_PATHSOURCE_debuglink 3 /* GNU debuglink */
#ifndef DW_FTYPE_UNKNOWN
#define DW_FTYPE_UNKNOWN    0
#define DW_FTYPE_ELF        1  /* Unix/Linux/etc */
#define DW_FTYPE_MACH_O     2  /* MacOS. */
#define DW_FTYPE_PE         3  /* Windows */
#define DW_FTYPE_ARCHIVE    4  /* unix archive */
#endif /* DW_FTYPE_UNKNOWN */
/* standard return values for functions */
#define DW_DLV_NO_ENTRY -1
#define DW_DLV_OK        0
#define DW_DLV_ERROR     1
/*  These support opening DWARF5 split dwarf objects and
    Elf SHT_GROUP blocks of DWARF sections. */
#define DW_GROUPNUMBER_ANY  0
#define DW_GROUPNUMBER_BASE 1
#define DW_GROUPNUMBER_DWO  2

/*  Special values for offset_into_exception_table field
    of dwarf fde's. */
/*  The following value indicates that there is no
    Exception table offset
    associated with a dwarf frame. */
#define DW_DLX_NO_EH_OFFSET         (-1LL)
/*  The following value indicates that the producer
    was unable to analyse the
    source file to generate Exception tables for this function. */
#define DW_DLX_EH_OFFSET_UNAVAILABLE  (-2LL)

/* The augmenter string for CIE */
#define DW_CIE_AUGMENTER_STRING_V0  "z"

/*  Taken as meaning 'undefined value', this is not
    a column or register number.
    Only present at libdwarf runtime. Never on disk.
    DW_FRAME_* Values present on disk are in dwarf.h
*/
#define DW_FRAME_UNDEFINED_VAL          1034

/*  Taken as meaning 'same value' as caller had, not a column
    or register number
    Only present at libdwarf runtime. Never on disk.
    DW_FRAME_* Values present on disk are in dwarf.h
*/
#define DW_FRAME_SAME_VAL               1035

/* dwarf_pcline function, slide arguments
*/
#define DW_DLS_BACKWARD   -1  /* slide backward to find line */
#define DW_DLS_NOSLIDE     0  /* match exactly without sliding */
#define DW_DLS_FORWARD     1  /* slide forward to find line */

/*  Defined larger than necessary. 
    struct Dwarf_Debug_Fission_Per_CU_s,
    being visible, will be difficult to change:
    binary compatibility. The count is for arrays
    inside the struct, the struct itself is
    a single struct.  */
#define DW_FISSION_SECT_COUNT 12

/*! @defgroup basicdatatypes Basic Library Datatypes Group
    @{
    @typedef Dwarf_Unsigned
    The basic unsigned data type.
    Intended to be an unsigned 64bit value.
    @typedef Dwarf_Signed
    The basic signed data type.
    Intended to be a signed 64bit value.
    @typedef Dwarf_Off
    Used for offsets. It should be same size as Dwarf_Unsigned.
    @typedef Dwarf_Addr
    Used when a data item is a an address represented
    in DWARF. 64 bits. Must be as large as the largest
    object address size.
    @typedef Dwarf_Half
    Many libdwarf values (attribute codes, for example)
    are defined by the standard to be 16 bits, and
    this datatype reflects that (the type must be
    at least 16 bits wide).
    @typedef Dwarf_Bool
    A TRUE(non-zero)/FALSE(zero) data item.
    @typedef Dwarf_Ptr
    A generic pointer type. It uses void *
    so it cannot be added-to or subtracted-from.
    @typedef Dwarf_Small
    Used for small unsigned integers and
    used as Dwarf_Small* for pointers and
    it supports pointer addition and subtraction
    conveniently.
*/
typedef unsigned long long Dwarf_Unsigned;
typedef signed   long long Dwarf_Signed;
typedef unsigned long long Dwarf_Off;
typedef unsigned long long Dwarf_Addr;
    /*  Dwarf_Bool as int is wasteful, but for compatibility
        it must stay as int, not unsigned char. */
typedef int                Dwarf_Bool;   /* boolean type */
typedef unsigned short     Dwarf_Half;   /* 2 byte unsigned value */
typedef unsigned char      Dwarf_Small;  /* 1 byte unsigned value */
/*  If sizeof(Dwarf_Half) is greater than 2
    we believe libdwarf still works properly. */

typedef void*        Dwarf_Ptr;          /* host machine pointer */
/*! @}   endgroup basicdatatypes */

/*! @defgroup enums Enumerators
    @{
    @enum Dwarf_Ranges_Entry_Type
    The dwr_addr1/addr2 data is either an offset (DW_RANGES_ENTRY)
    or an address (dwr_addr2 in DW_RANGES_ADDRESS_SELECTION) or
    both are zero (DW_RANGES_END).
    For DWARF5 each table starts with a header
    followed by range list entries defined
    as here.
    Dwarf_Ranges* apply to DWARF2,3, and 4.
    Not to DWARF5 (the data is different and
    in a new DWARF5 section).
*/
enum Dwarf_Ranges_Entry_Type { DW_RANGES_ENTRY,
    DW_RANGES_ADDRESS_SELECTION,
    DW_RANGES_END
};

/*! @enum Dwarf_Form_Class
    The dwarf specification separates FORMs into
    different classes.  To do the separation properly
    requires 4 pieces of data as of DWARF4 (thus the
    function arguments listed here).
    The DWARF4 specification class definition suffices to
    describe all DWARF versions.
    See section 7.5.4, Attribute Encodings.
    A return of DW_FORM_CLASS_UNKNOWN means we
    could not properly figure
    out what form-class it is.

    DW_FORM_CLASS_FRAMEPTR is MIPS/IRIX only, and refers
    to the DW_AT_MIPS_fde attribute (a reference to the
    .debug_frame section).

    DWARF5:
    DW_FORM_CLASS_LOCLISTSPTR  is like DW_FORM_CLASS_LOCLIST
    except that LOCLISTSPTR is aways a section offset,
    never an index, and LOCLISTSPTR is only referenced
    by DW_AT_loclists_base.
    Note DW_FORM_CLASS_LOCLISTSPTR spelling to distinguish
    from DW_FORM_CLASS_LOCLISTPTR.

    DWARF5:
    DW_FORM_CLASS_RNGLISTSPTR  is like DW_FORM_CLASS_RNGLIST
    except that RNGLISTSPTR is aways a section offset,
    never an index. DW_FORM_CLASS_RNGLISTSPTR is only
    referenced by DW_AT_rnglists_base.
*/
enum Dwarf_Form_Class {
    DW_FORM_CLASS_UNKNOWN = 0,
    DW_FORM_CLASS_ADDRESS = 1,
    DW_FORM_CLASS_BLOCK   = 2,
    DW_FORM_CLASS_CONSTANT =3,
    DW_FORM_CLASS_EXPRLOC = 4,
    DW_FORM_CLASS_FLAG    = 5,
    DW_FORM_CLASS_LINEPTR = 6,
    DW_FORM_CLASS_LOCLISTPTR=7,    /* DWARF2,3,4 only */
    DW_FORM_CLASS_MACPTR  = 8,     /* DWARF2,3,4 only */
    DW_FORM_CLASS_RANGELISTPTR=9,  /* DWARF2,3,4 only */
    DW_FORM_CLASS_REFERENCE=10,
    DW_FORM_CLASS_STRING  = 11,
    DW_FORM_CLASS_FRAMEPTR= 12,  /* MIPS/IRIX DWARF2 only */
    DW_FORM_CLASS_MACROPTR= 13,    /* DWARF5 */
    DW_FORM_CLASS_ADDRPTR = 14,    /* DWARF5 */
    DW_FORM_CLASS_LOCLIST = 15,    /* DWARF5 */
    DW_FORM_CLASS_LOCLISTSPTR=16,  /* DWARF5 */
    DW_FORM_CLASS_RNGLIST    =17,  /* DWARF5 */
    DW_FORM_CLASS_RNGLISTSPTR=18,  /* DWARF5 */
    DW_FORM_CLASS_STROFFSETSPTR=19 /* DWARF5 */
};
/*! @}   endgroupenums*/

/*! @defgroup allstructs Defined and Opaque Structs Group
    @{

    @typedef Dwarf_Form_Data16
    a container for a DW_FORM_data16 data item.
    We have no integer types suitable so this special
    struct is used instead.  It is up to consumers/producers
    to deal with the contents.
*/
typedef struct Dwarf_Form_Data16_s {
    unsigned char fd_data[16];
} Dwarf_Form_Data16;

/*! @typedef Dwarf_Sig8
    Used for signatures where ever they appear.
    It is not a string, it
    is 8 bytes of a signature one would use to find
    a type unit. See dwarf_formsig8()
*/
struct Dwarf_Sig8_s  {
    char signature[8];
};
typedef struct Dwarf_Sig8_s Dwarf_Sig8;

/* Refers to on an uninterpreted block of data
   Used with certain location information functions,
   a frame expression function, expanded
   frame instructions, and
   DW_FORM_block<> functions.
*/
typedef struct Dwarf_Block_s {
    Dwarf_Unsigned  bl_len;  /* length of block bl_data points at */
    Dwarf_Ptr       bl_data; /* uninterpreted data */

    /*  See libdwarf.h DW_LKIND*  */
    Dwarf_Small     bl_from_loclist;

    /* Section (not CU) offset which 'data' comes from. */
    Dwarf_Unsigned  bl_section_offset;
} Dwarf_Block;

/*  This provides access to Dwarf_Locdesc_c, a single
    location description */
typedef struct Dwarf_Locdesc_c_s * Dwarf_Locdesc_c;

/*  This provides access to Dwarf_Locdesc_c, a single
    location list entry (or for a locexpr, the fake
    Loc_Head for the locexpr) */
typedef struct Dwarf_Loc_Head_c_s * Dwarf_Loc_Head_c;

/*  This provides access to data from sections
    .debug_gnu_pubtypes or .debug_gnu_pubnames.
    These are not standard DWARF, and can appear
    with gcc -gdwarf-5
    */
typedef struct Dwarf_Gnu_Index_Head_s * Dwarf_Gnu_Index_Head;

/*! @typedef Dwarf_Dsc_Head
    Access to DW_AT_discr_list
    array of discriminant values.
*/
typedef struct Dwarf_Dsc_Head_s * Dwarf_Dsc_Head;

/*! @typedef Dwarf_Frame_Instr_Head
    The basis for access to DWARF frame instructions
    (FDE or CIE)
    in full detail.
*/
typedef struct Dwarf_Frame_Instr_Head_s * Dwarf_Frame_Instr_Head;

typedef void (* dwarf_printf_callback_function_type)
    (void * /*user_pointer*/, const char * /*linecontent*/);

struct Dwarf_Printf_Callback_Info_s {
    void *                        dp_user_pointer;
    dwarf_printf_callback_function_type dp_fptr;
    char *                        dp_buffer;
    unsigned int                  dp_buffer_len;
    int                           dp_buffer_user_provided;
    void *                        dp_reserved;
};
/*! @typedef Dwarf_Cmdline_Options.

    check_verbose_mode defaults to FALSE.
    If a libdwarf-calling program sets
    this TRUE it means some errors in Line Table
    headers get a much more detailed description
    of the error which is reported the caller via
    printf_callback() function (the caller
    can do something with the message).
    Or the libdwarf calling code can call
    dwarf_record_cmdline_options() to set
    the new value.
*/
typedef struct Dwarf_Cmdline_Options_s {
    Dwarf_Bool check_verbose_mode;
} Dwarf_Cmdline_Options;

/*! @typedef Dwarf_Str_Offsets_Table
    Provides an access to the .debug_str_offsets
    section indepenently of other DWARF sections.
    Mainly of use in examining the .debug_str_offsets
    section content for problems.
*/
typedef struct Dwarf_Str_Offsets_Table_s *  Dwarf_Str_Offsets_Table;

/*! @typedef Dwarf_Ranges
    Details of of non-contiguous address ranges
    of DIEs for DWARF2, DWARF3, and DWARF4.
    Sufficient for older dwarf, but

*/
typedef struct Dwarf_Ranges_s {
    Dwarf_Addr dwr_addr1;
    Dwarf_Addr dwr_addr2;
    enum Dwarf_Ranges_Entry_Type  dwr_type;
} Dwarf_Ranges;


/*! @typedef Dwarf_Regtable_Entry3
    For each index i (naming a hardware register with dwarf number
    i) the following is true and defines the value of that register:

        If dw_regnum is Register DW_FRAME_UNDEFINED_VAL
            it is not DWARF register number but
            a place holder indicating the register
            has no defined value.
        If dw_regnum is Register DW_FRAME_SAME_VAL
            it  is not DWARF register number but
            a place holder indicating the register has the same
            value in the previous frame.

            DW_FRAME_UNDEFINED_VAL, DW_FRAME_SAME_VAL and
            DW_FRAME_CFA_COL3 are only present at libdwarf runtime.
            Never on disk.
            DW_FRAME_* Values present on disk are in dwarf.h
            Because DW_FRAME_SAME_VAL and DW_FRAME_UNDEFINED_VAL
            and DW_FRAME_CFA_COL3 are definable at runtime
            consider the names symbolic in this comment,
            not absolute.

        Otherwise: the register number is a DWARF register number
            (see ABI documents for how this translates to hardware/
            software register numbers in the machine hardware)
            and the following applies:

        In a cfa-defining entry (rt3_cfa_rule) the regnum is the
        CFA 'register number'. Which is some 'normal' register,
        not DW_FRAME_CFA_COL3, nor DW_FRAME_SAME_VAL, nor
        DW_FRAME_UNDEFINED_VAL.

        If dw_value_type == DW_EXPR_OFFSET (the only
        possible case for dwarf2):
            If dw_offset_relevant is non-zero, then
                the value is stored at at the address
                CFA+N where N is a signed offset.
                dw_regnum is the cfa register rule which means
                one ignores dw_regnum and uses the CFA appropriately.
                So dw_offset is a signed value, really,
                and must be printed/evaluated as such.
                Rule: Offset(N)
            If dw_offset_relevant is zero, then the
                value of the register
                is the value of (DWARF) register number dw_regnum.
                Rule: register(R)
        If dw_value_type  == DW_EXPR_VAL_OFFSET
            the  value of this register is CFA +N where
            N is a signed offset.
            dw_regnum is the cfa register rule which means
            one ignores dw_regnum and uses the CFA appropriately.
            Rule: val_offset(N)
        If dw_value_type  == DW_EXPR_EXPRESSION
            The value of the register is the value at the address
            computed by evaluating the DWARF expression E.
            Rule: expression(E)
            The expression E byte stream is pointed to by
            block.bl_data.
            The expression length in bytes is given by
            block.bl_len.
        If dw_value_type  == DW_EXPR_VAL_EXPRESSION
            The value of the register is the value
            computed by evaluating the DWARF expression E.
            Rule: val_expression(E)
            The expression E byte stream is pointed to
            by block.bl_data.
            The expression length in bytes is given by
            block.bl_len.
        Other values of dw_value_type are an error.
*/

typedef struct Dwarf_Regtable_Entry3_s {
    Dwarf_Small         dw_offset_relevant;
    Dwarf_Small         dw_value_type;
    Dwarf_Half          dw_regnum;
    Dwarf_Unsigned      dw_offset;
    Dwarf_Unsigned      dw_args_size; /* Not dealt with.  */
    Dwarf_Block         dw_block;
} Dwarf_Regtable_Entry3;

/*! @typedef  Dwarf_Regtable3
    This structs provides a way for applications
    to select the number of frame registers
    and to select names for them.

    rt3_rules and rt3_reg_table_size must be filled in before
    calling libdwarf.  Filled in with a pointer to an array
    (pointer and array  set up by the calling application)
    of rt3_reg_table_size Dwarf_Regtable_Entry3_s structs.
    libdwarf does not allocate or deallocate space for the
    rules, you must do so.   libdwarf will initialize the
    contents rules array, you do not need to do so (though
    if you choose to initialize the array somehow that is ok:
    libdwarf will overwrite your initializations with its own).
*/
typedef struct Dwarf_Regtable3_s {
    struct Dwarf_Regtable_Entry3_s   rt3_cfa_rule;
    Dwarf_Half                       rt3_reg_table_size;
    struct Dwarf_Regtable_Entry3_s * rt3_rules;
} Dwarf_Regtable3;

/* Opaque types for Consumer Library. */
/*! @typedef Dwarf_Error
    &error is used in most calls to return error details
    when the call returns DW_DLV_ERROR.
*/
typedef struct Dwarf_Error_s*      Dwarf_Error;
/*! @typedef Dwarf_Debug
    An open Dwarf_Debug points to data that libdwarf
    maintains to support libdwarf calls.
*/
typedef struct Dwarf_Debug_s*      Dwarf_Debug;
typedef struct Dwarf_Die_s*        Dwarf_Die;
typedef struct Dwarf_Line_s*       Dwarf_Line;
typedef struct Dwarf_Global_s*     Dwarf_Global;
typedef struct Dwarf_Func_s*       Dwarf_Func;
typedef struct Dwarf_Type_s*       Dwarf_Type;
typedef struct Dwarf_Var_s*        Dwarf_Var;
typedef struct Dwarf_Weak_s*       Dwarf_Weak;
typedef struct Dwarf_Error_s*      Dwarf_Error;
typedef struct Dwarf_Attribute_s*  Dwarf_Attribute;
typedef struct Dwarf_Abbrev_s*     Dwarf_Abbrev;
typedef struct Dwarf_Fde_s*        Dwarf_Fde;
typedef struct Dwarf_Cie_s*        Dwarf_Cie;
typedef struct Dwarf_Arange_s*     Dwarf_Arange;
typedef struct Dwarf_Gdbindex_s*   Dwarf_Gdbindex;
typedef struct Dwarf_Xu_Index_Header_s  *Dwarf_Xu_Index_Header;
typedef struct Dwarf_Line_Context_s     *Dwarf_Line_Context;
typedef struct Dwarf_Macro_Context_s    *Dwarf_Macro_Context;
typedef struct Dwarf_Dnames_Head_s      *Dwarf_Dnames_Head;

/*! @typedef Dwarf_Handler
    Used in rare cases (mainly tiny programs)
    with dwarf_init_path() etc
    initialization calls.
*/
typedef void  (*Dwarf_Handler)(Dwarf_Error dw_error,
    Dwarf_Ptr dw_errarg);

/*! @struct Dwarf_Macro_Details_s

    This applies to DWARF2, DWARF3, and DWARF4
    compilation units.
    DWARF5 .debug_macro has its own function interface
    which does not use this struct.
*/
struct Dwarf_Macro_Details_s {
    Dwarf_Off    dmd_offset; /* offset, in the section,
        of this macro info */
    Dwarf_Small  dmd_type;   /* the type, DW_MACINFO_define etc*/
    Dwarf_Signed dmd_lineno; /* the source line number where
        applicable and vend_def number if
        vendor_extension op */

    Dwarf_Signed dmd_fileindex;/* the source file index:
        applies to define undef start_file */
    char *       dmd_macro;  /* macro name (with value for defineop)
        string from vendor ext */
};
/*! @typedef Dwarf_Rnglists_Head
    Used for access to a set of DWARF5 debug_rnglists
    entries.
*/
typedef struct Dwarf_Rnglists_Head_s * Dwarf_Rnglists_Head;
typedef struct Dwarf_Obj_Access_Interface_a_s
    Dwarf_Obj_Access_Interface_a;
typedef struct Dwarf_Obj_Access_Methods_a_s
    Dwarf_Obj_Access_Methods_a;
typedef struct Dwarf_Obj_Access_Section_a_s
    Dwarf_Obj_Access_Section_a;
typedef struct Dwarf_Macro_Details_s Dwarf_Macro_Details;
typedef struct Dwarf_Debug_Fission_Per_CU_s
    Dwarf_Debug_Fission_Per_CU;

/*  Used in the get_section interface function
    in Dwarf_Obj_Access_Section_a_s.  Since libdwarf
    depends on standard DWARF section names an object
    format that has no such names (but has some
    method of setting up 'sections equivalents')
    must arrange to return standard DWARF section
    names in the 'name' field.  libdwarf does
    not free the strings in 'name'.
    For non-elf many of the fields should be zero.*/
struct Dwarf_Obj_Access_Section_a_s {
    /*  Having an accurate section name makes
        debugging of libdwarf easier.
        and is essential to find the .debug_ sections.  */
    const char*    as_name;
    Dwarf_Unsigned as_type;
    Dwarf_Unsigned as_flags;
    /*  addr is the virtual address of the first byte of
        the section data.  Usually zero when the address
        makes no sense for a given section. */
    Dwarf_Addr     as_addr;
    Dwarf_Unsigned as_offset; /* file offset of section */

    /* Size in bytes of the section. */
    Dwarf_Unsigned as_size;

    /*  Set link to zero if it is meaningless.  If non-zero
        it should be a link to a rela section or from symtab
        to strtab.  In Elf it is sh_link. */
    Dwarf_Unsigned as_link;

    /*  The section header index of the section to which the
        relocation applies. In Elf it is sh_info. */
    Dwarf_Unsigned as_info;

    Dwarf_Unsigned as_addralign;
    /*  Elf sections that are tables have a non-zero entrysize so
        the count of entries can be calculated even without
        the right structure definition. If your object format
        does not have this data leave this zero. */
    Dwarf_Unsigned as_entrysize;
};

/*! @struct Dwarf_Obj_Access_Methods_a_s:
    The functions we need to access object data
    from libdwarf are declared here.

    In these function pointer declarations
    'void *obj' is intended to be a pointer (the object field in
    Dwarf_Obj_Access_Interface_s)
    that hides the library-specific and object-specific
    data that makes
    it possible to handle multiple object formats
    and multiple libraries.
    It's not required that one handles multiple such
    in a single libdwarf
    archive/shared-library (but not ruled out either).
    See  dwarf_elf_object_access_internals_t and dwarf_elf_access.c
    for an example.

    Usually the struct is statically defined
    and the function pointers are set at
    compile time.

    The om_get_filesize member is new September 4, 2021.
    Its postion is NOT at the end of the list.
    The member names all now have om_ prefix.
*/
/*
    om_get_section_info

    Get address, size, and name info about a section.

    Parameters
    section_index - Zero-based index.
    return_section - Pointer to a structure in which
        section info will be placed.   Caller must
        provide a valid pointer to a
        structure area.  The structure's contents
        will be overwritten
        by the call to get_section_info.
    error - A pointer to an integer in which an error
        code may be stored.

    Return
    DW_DLV_OK - Everything ok.
    DW_DLV_ERROR - Error occurred. Use 'error' to determine the
        libdwarf defined error.
    DW_DLV_NO_ENTRY - No such section.  */
/*
    om_get_byte_order

    Get whether the object file represented by
    this interface is big-endian
    (DW_END_big) or little endian (DW_END_little).

    Parameters
    obj - Equivalent to 'this' in OO languages.

    Return
    Endianness of object. Cannot fail.  */
/*
    om_get_length_size

    Get the size of a length field in the underlying object file.
    libdwarf currently supports * 4 and 8 byte sizes, but may
    support larger in the future.
    Perhaps the return type should be an enumeration?

    Parameters
    obj - Equivalent to 'this' in OO languages.

    Return
    Size of length. Cannot fail.  */
/*
    om_get_pointer_size

    Get the size of a pointer field in the underlying object file.
    libdwarf currently supports  4 and 8 byte sizes.
    Perhaps the return type should be an enumeration?

    Return
    Size of pointer. Cannot fail.  */
/*
    om_get_filesize

    Returns a value that is a sanity check on
    offsets libdwarf reads.  Must be larger than any section size
    libdwarf might read.  It need not be a tight bound.
    In dwarf_init_path() etc libdwarf uses the object file
    size as the value.

    Return
    A value at least as large as any section libdwarf
    might read.  */
/*
    om_get_section_count

    Get the number of sections in the object file.

    Parameters

    Return
    Number of sections */
/*
    om_load_section

    Get a pointer to an array of bytes that
    represent the section.

    Parameters
    section_index - Zero-based index.
    return_data - The address of a pointer to
        which the section data block
        will be assigned.
    error - Pointer to an integer for returning
        libdwarf-defined error numbers.

    Return
    DW_DLV_OK - No error.
    DW_DLV_ERROR - Error. Use 'error' to indicate
        a libdwarf-defined error number.
    DW_DLV_NO_ENTRY - No such section.  */
/*
    om_relocate_a_section
    If relocations are not supported leave this pointer NULL.

    Get a pointer to an array of bytes that represent
    the section.

    Parameters
    section_index - Zero-based index of the
        section to be relocated.
    error - Pointer to an integer for returning libdwarf-defined
        error numbers.

    Return
    DW_DLV_OK - No error.
    DW_DLV_ERROR - Error. Use 'error' to indicate
        a libdwarf-defined
        error number.
    DW_DLV_NO_ENTRY - No such section.  */
struct Dwarf_Obj_Access_Methods_a_s {
    int    (*om_get_section_info)(void* obj,
        Dwarf_Half section_index,
        Dwarf_Obj_Access_Section_a* return_section,
        int* error);
    Dwarf_Small      (*om_get_byte_order)(void* obj);
    Dwarf_Small      (*om_get_length_size)(void* obj);
    Dwarf_Small      (*om_get_pointer_size)(void* obj);
    Dwarf_Unsigned   (*om_get_filesize)(void* obj);

    Dwarf_Unsigned   (*om_get_section_count)(void* obj);
    int              (*om_load_section)(void* obj,
        Dwarf_Half section_index,
        Dwarf_Small** return_data, int* error);
    int              (*om_relocate_a_section)(void* obj,
        Dwarf_Half section_index,
        Dwarf_Debug dbg,
        int* error);
};

/*  struct Dwarf_Obj_Access_Interface_a_s is allocated
    and deallocated by your code when you are using
    the libdwarf Object File Interface
    [dwarf_object_init_b() directly.
*/
struct Dwarf_Obj_Access_Interface_a_s {
    void*                             ai_object;
    const Dwarf_Obj_Access_Methods_a *ai_methods;
};

/*  User code must allocate this struct, zero it,
    and pass a pointer to it
    into dwarf_get_debugfission_for_cu .  */
struct Dwarf_Debug_Fission_Per_CU_s  {
    /*  Do not free the string. It contains "cu" or "tu". */
    /*  If this is not set (ie, not a CU/TU in  DWP Package File)
        then pcu_type will be NULL.  */
    const char   * pcu_type;
    /*  pcu_index is the index (range 1 to N )
        into the tu/cu table of offsets and the table
        of sizes.  1 to N as the zero index is reserved
        for special purposes.  Not a value one
        actually needs. */
    Dwarf_Unsigned pcu_index;
    Dwarf_Sig8     pcu_hash;  /* 8 byte  */
    /*  [0] has offset and size 0.
        [1]-[8] are DW_SECT_* indexes and the
        values are  the offset and size
        of the respective section contribution
        of a single .dwo object. When pcu_size[n] is
        zero the corresponding section is not present. */
    Dwarf_Unsigned pcu_offset[DW_FISSION_SECT_COUNT];
    Dwarf_Unsigned pcu_size[DW_FISSION_SECT_COUNT];
    Dwarf_Unsigned unused1;
    Dwarf_Unsigned unused2;
};

/*! @typedef Dwarf_Rnglists_Head
    Used for access to a set of DWARF5 debug_rnglists
    entries.
*/
typedef struct Dwarf_Rnglists_Head_s * Dwarf_Rnglists_Head;

/*! @} endgroup allstructs */


/*! @defgroup framedefines Default frame #define values
    @{
*/
/*  Special values for offset_into_exception_table field
    of dwarf fde's. */
/*  The following value indicates that there is no
    Exception table offset
    associated with a dwarf frame. */
#define DW_DLX_NO_EH_OFFSET         (-1LL)
/*  The following value indicates that the producer
    was unable to analyse the
    source file to generate Exception tables for this function. */
#define DW_DLX_EH_OFFSET_UNAVAILABLE  (-2LL)

/* The augmenter string for CIE */
#define DW_CIE_AUGMENTER_STRING_V0  "z"

/*  ***IMPORTANT NOTE, TARGET DEPENDENCY ****
    DW_REG_TABLE_SIZE must be at least as large as
    the number of registers
    (DW_FRAME_LAST_REG_NUM) as defined in dwarf.h
    Preferably identical to DW_FRAME_LAST_REG_NUM.
    Ensure [0-DW_REG_TABLE_SIZE] does not overlap
    DW_FRAME_UNDEFINED_VAL or DW_FRAME_SAME_VAL.
    Also ensure DW_FRAME_REG_INITIAL_VALUE is set to what
    is appropriate to your cpu.
    For various CPUs  DW_FRAME_UNDEFINED_VAL is correct
    as the value for DW_FRAME_REG_INITIAL_VALUE.

    For consumer apps, this can be set dynamically: see
    dwarf_set_frame_rule_table_size(); */
#ifndef DW_REG_TABLE_SIZE
#define DW_REG_TABLE_SIZE  66
#endif

/* For MIPS, DW_FRAME_SAME_VAL is the correct default value
   for a frame register value. For other CPUS another value
   may be better, such as DW_FRAME_UNDEFINED_VAL.
   See dwarf_set_frame_rule_table_size
*/
#ifndef DW_FRAME_REG_INITIAL_VALUE
#define DW_FRAME_REG_INITIAL_VALUE DW_FRAME_SAME_VAL
#endif

/* Taken as meaning 'undefined value', this is not
   a column or register number.
   Only present at libdwarf runtime in the consumer
   interfaces. Never on disk.
   DW_FRAME_* Values present on disk are in dwarf.h
   Ensure this is > DW_REG_TABLE_SIZE (the reg table
   size is changeable at runtime with the *reg3() interfaces,
   and this value must be greater than the reg table size).
*/
#define DW_FRAME_UNDEFINED_VAL          1034

/* Taken as meaning 'same value' as caller had, not a column
   or register number.
   Only present at libdwarf runtime in the consumer
   interfaces. Never on disk.
   DW_FRAME_* Values present on disk are in dwarf.h
   Ensure this is > DW_REG_TABLE_SIZE (the reg table
   size is changeable at runtime with the *reg3() interfaces,
   and this value must be greater than the reg table size).
*/
#define DW_FRAME_SAME_VAL               1035

/* For DWARF3 and later consumer interfaces,
   make the CFA a column with no real table number.
   See the libdwarf documentation
   on Dwarf_Regtable3 and  dwarf_get_fde_info_for_reg3().
   and  dwarf_get_fde_info_for_all_regs3()
   CFA Must be higher than any register count for *any* ABI
   (ensures maximum applicability with minimum effort).
   Ensure this is > DW_REG_TABLE_SIZE (the reg table
   size is changeable at runtime with the *reg3() interfaces,
   and this value must be greater than the reg table size).
   Only present at libdwarf runtime in the consumer
   interfaces. Never on disk.
*/
#define DW_FRAME_CFA_COL3               1436

/* The following are all needed to evaluate DWARF3 register rules.
   These have nothing to do simply printing
   frame instructions.
*/
#define DW_EXPR_OFFSET         0 /* offset is from CFA reg */
#define DW_EXPR_VAL_OFFSET     1
#define DW_EXPR_EXPRESSION     2
#define DW_EXPR_VAL_EXPRESSION 3
/*! @} */


/*! @defgroup dwdla  DW_DLA #define values
   
    These identify the various allocate/dealloc
    types.  The allocation happens within libdwarf,
    and the deallocation is usually done by
    user code. 
    @{
*/
#define DW_DLA_STRING          0x01  /* char* */
#define DW_DLA_LOC             0x02  /* Dwarf_Loc */
#define DW_DLA_LOCDESC         0x03  /* Dwarf_Locdesc */
#define DW_DLA_ELLIST          0x04  /* Dwarf_Ellist (not used)*/
#define DW_DLA_BOUNDS          0x05  /* Dwarf_Bounds (not used) */
#define DW_DLA_BLOCK           0x06  /* Dwarf_Block */
#define DW_DLA_DEBUG           0x07  /* Dwarf_Debug */
#define DW_DLA_DIE             0x08  /* Dwarf_Die */
#define DW_DLA_LINE            0x09  /* Dwarf_Line */
#define DW_DLA_ATTR            0x0a  /* Dwarf_Attribute */
#define DW_DLA_TYPE            0x0b  /* Dwarf_Type  (not used) */
#define DW_DLA_SUBSCR          0x0c  /* Dwarf_Subscr (not used) */
#define DW_DLA_GLOBAL          0x0d  /* Dwarf_Global */
#define DW_DLA_ERROR           0x0e  /* Dwarf_Error */
#define DW_DLA_LIST            0x0f  /* a list */
#define DW_DLA_LINEBUF         0x10  /* Dwarf_Line* (not used) */
#define DW_DLA_ARANGE          0x11  /* Dwarf_Arange */
#define DW_DLA_ABBREV          0x12  /* Dwarf_Abbrev */
#define DW_DLA_FRAME_INSTR_HEAD   0x13  /* Dwarf_Frame_Instr_Head */
#define DW_DLA_CIE             0x14  /* Dwarf_Cie */
#define DW_DLA_FDE             0x15  /* Dwarf_Fde */
#define DW_DLA_LOC_BLOCK       0x16  /* Dwarf_Loc */

#define DW_DLA_FRAME_OP        0x17 /* Dwarf_Frame_Op (not used) */
#define DW_DLA_FUNC            0x18  /* Dwarf_Func */
#define DW_DLA_TYPENAME        0x19  /* Dwarf_Type */
#define DW_DLA_VAR             0x1a  /* Dwarf_Var */
#define DW_DLA_WEAK            0x1b  /* Dwarf_Weak */
#define DW_DLA_ADDR            0x1c  /* Dwarf_Addr sized entries */
#define DW_DLA_RANGES          0x1d  /* Dwarf_Ranges */
/* 0x1e (30) to 0x34 (52) reserved for internal to libdwarf types. */
/* .debug_gnu_typenames/pubnames, 2020 */
#define DW_DLA_GNU_INDEX_HEAD  0x35

#define DW_DLA_RNGLISTS_HEAD   0x36  /* .debug_rnglists DW5 */
#define DW_DLA_GDBINDEX        0x37  /* Dwarf_Gdbindex */
#define DW_DLA_XU_INDEX        0x38  /* Dwarf_Xu_Index_Header */
#define DW_DLA_LOC_BLOCK_C     0x39  /* Dwarf_Loc_c*/
#define DW_DLA_LOCDESC_C       0x3a  /* Dwarf_Locdesc_c */
#define DW_DLA_LOC_HEAD_C      0x3b  /* Dwarf_Loc_Head_c */
#define DW_DLA_MACRO_CONTEXT   0x3c  /* Dwarf_Macro_Context */
/*  0x3d (61) is for libdwarf internal use.               */
#define DW_DLA_DSC_HEAD        0x3e  /* Dwarf_Dsc_Head */
#define DW_DLA_DNAMES_HEAD     0x3f  /* Dwarf_Dnames_Head */

/* struct Dwarf_Str_Offsets_Table_s */
#define DW_DLA_STR_OFFSETS     0x40
/*! @} */

/*! @defgroup dwdle  DW_DLE #define Error Numbers

    These identify the various error codes that have been
    used.  Not all of them are still use.
    We do not recycle obsolete codes into new uses.
    The codes 1 through 22 are historic and it is
    unlikely they are used anywhere in the library.
    @{
*/
/* libdwarf error numbers */
#define DW_DLE_NE      0  /* no error */
#define DW_DLE_VMM     1  /* dwarf format/library version mismatch */
#define DW_DLE_MAP     2  /* memory map failure */
#define DW_DLE_LEE     3  /* libelf error */
#define DW_DLE_NDS     4  /* no debug section */
#define DW_DLE_NLS     5  /* no line section */
#define DW_DLE_ID      6  /* invalid descriptor for query */
#define DW_DLE_IOF     7  /* I/O failure */
#define DW_DLE_MAF     8  /* memory allocation failure */
#define DW_DLE_IA      9  /* invalid argument */
#define DW_DLE_MDE     10 /* mangled debugging entry */
#define DW_DLE_MLE     11 /* mangled line number entry */
#define DW_DLE_FNO     12 /* file not open */
#define DW_DLE_FNR     13 /* file not a regular file */
#define DW_DLE_FWA     14 /* file open with wrong access */
#define DW_DLE_NOB     15 /* not an object file */
#define DW_DLE_MOF     16 /* mangled object file header */
#define DW_DLE_EOLL    17 /* end of location list entries */
#define DW_DLE_NOLL    18 /* no location list section */
#define DW_DLE_BADOFF  19 /* Invalid offset */
#define DW_DLE_EOS     20 /* end of section  */
#define DW_DLE_ATRUNC  21 /* abbreviations section appears truncated*/
#define DW_DLE_BADBITC 22 /* Address size passed to dwarf bad,*/
    /* It is not an allowed size (64 or 32) */
    /* Error codes defined by the current Libdwarf Implementation. */
#define DW_DLE_DBG_ALLOC                        23
#define DW_DLE_FSTAT_ERROR                      24
#define DW_DLE_FSTAT_MODE_ERROR                 25
#define DW_DLE_INIT_ACCESS_WRONG                26
#define DW_DLE_ELF_BEGIN_ERROR                  27
#define DW_DLE_ELF_GETEHDR_ERROR                28
#define DW_DLE_ELF_GETSHDR_ERROR                29
#define DW_DLE_ELF_STRPTR_ERROR                 30
#define DW_DLE_DEBUG_INFO_DUPLICATE             31
#define DW_DLE_DEBUG_INFO_NULL                  32
#define DW_DLE_DEBUG_ABBREV_DUPLICATE           33
#define DW_DLE_DEBUG_ABBREV_NULL                34
#define DW_DLE_DEBUG_ARANGES_DUPLICATE          35
#define DW_DLE_DEBUG_ARANGES_NULL               36
#define DW_DLE_DEBUG_LINE_DUPLICATE             37
#define DW_DLE_DEBUG_LINE_NULL                  38
#define DW_DLE_DEBUG_LOC_DUPLICATE              39
#define DW_DLE_DEBUG_LOC_NULL                   40
#define DW_DLE_DEBUG_MACINFO_DUPLICATE          41
#define DW_DLE_DEBUG_MACINFO_NULL               42
#define DW_DLE_DEBUG_PUBNAMES_DUPLICATE         43
#define DW_DLE_DEBUG_PUBNAMES_NULL              44
#define DW_DLE_DEBUG_STR_DUPLICATE              45
#define DW_DLE_DEBUG_STR_NULL                   46
#define DW_DLE_CU_LENGTH_ERROR                  47
#define DW_DLE_VERSION_STAMP_ERROR              48
#define DW_DLE_ABBREV_OFFSET_ERROR              49
#define DW_DLE_ADDRESS_SIZE_ERROR               50
#define DW_DLE_DEBUG_INFO_PTR_NULL              51
#define DW_DLE_DIE_NULL                         52
#define DW_DLE_STRING_OFFSET_BAD                53
#define DW_DLE_DEBUG_LINE_LENGTH_BAD            54
#define DW_DLE_LINE_PROLOG_LENGTH_BAD           55
#define DW_DLE_LINE_NUM_OPERANDS_BAD            56
#define DW_DLE_LINE_SET_ADDR_ERROR              57
#define DW_DLE_LINE_EXT_OPCODE_BAD              58
#define DW_DLE_DWARF_LINE_NULL                  59
#define DW_DLE_INCL_DIR_NUM_BAD                 60
#define DW_DLE_LINE_FILE_NUM_BAD                61
#define DW_DLE_ALLOC_FAIL                       62
#define DW_DLE_NO_CALLBACK_FUNC                 63
#define DW_DLE_SECT_ALLOC                       64
#define DW_DLE_FILE_ENTRY_ALLOC                 65
#define DW_DLE_LINE_ALLOC                       66
#define DW_DLE_FPGM_ALLOC                       67
#define DW_DLE_INCDIR_ALLOC                     68
#define DW_DLE_STRING_ALLOC                     69
#define DW_DLE_CHUNK_ALLOC                      70
#define DW_DLE_BYTEOFF_ERR                      71
#define DW_DLE_CIE_ALLOC                        72
#define DW_DLE_FDE_ALLOC                        73
#define DW_DLE_REGNO_OVFL                       74
#define DW_DLE_CIE_OFFS_ALLOC                   75
#define DW_DLE_WRONG_ADDRESS                    76
#define DW_DLE_EXTRA_NEIGHBORS                  77
#define DW_DLE_WRONG_TAG                        78
#define DW_DLE_DIE_ALLOC                        79
#define DW_DLE_PARENT_EXISTS                    80
#define DW_DLE_DBG_NULL                         81
#define DW_DLE_DEBUGLINE_ERROR                  82
#define DW_DLE_DEBUGFRAME_ERROR                 83
#define DW_DLE_DEBUGINFO_ERROR                  84
#define DW_DLE_ATTR_ALLOC                       85
#define DW_DLE_ABBREV_ALLOC                     86
#define DW_DLE_OFFSET_UFLW                      87
#define DW_DLE_ELF_SECT_ERR                     88
#define DW_DLE_DEBUG_FRAME_LENGTH_BAD           89
#define DW_DLE_FRAME_VERSION_BAD                90
#define DW_DLE_CIE_RET_ADDR_REG_ERROR           91
#define DW_DLE_FDE_NULL                         92
#define DW_DLE_FDE_DBG_NULL                     93
#define DW_DLE_CIE_NULL                         94
#define DW_DLE_CIE_DBG_NULL                     95
#define DW_DLE_FRAME_TABLE_COL_BAD              96
#define DW_DLE_PC_NOT_IN_FDE_RANGE              97
#define DW_DLE_CIE_INSTR_EXEC_ERROR             98
#define DW_DLE_FRAME_INSTR_EXEC_ERROR           99
#define DW_DLE_FDE_PTR_NULL                    100
#define DW_DLE_RET_OP_LIST_NULL                101
#define DW_DLE_LINE_CONTEXT_NULL               102
#define DW_DLE_DBG_NO_CU_CONTEXT               103
#define DW_DLE_DIE_NO_CU_CONTEXT               104
#define DW_DLE_FIRST_DIE_NOT_CU                105
#define DW_DLE_NEXT_DIE_PTR_NULL               106
#define DW_DLE_DEBUG_FRAME_DUPLICATE           107
#define DW_DLE_DEBUG_FRAME_NULL                108
#define DW_DLE_ABBREV_DECODE_ERROR             109
#define DW_DLE_DWARF_ABBREV_NULL               110
#define DW_DLE_ATTR_NULL                       111
#define DW_DLE_DIE_BAD                         112
#define DW_DLE_DIE_ABBREV_BAD                  113
#define DW_DLE_ATTR_FORM_BAD                   114
#define DW_DLE_ATTR_NO_CU_CONTEXT              115
#define DW_DLE_ATTR_FORM_SIZE_BAD              116
#define DW_DLE_ATTR_DBG_NULL                   117
#define DW_DLE_BAD_REF_FORM                    118
#define DW_DLE_ATTR_FORM_OFFSET_BAD            119
#define DW_DLE_LINE_OFFSET_BAD                 120
#define DW_DLE_DEBUG_STR_OFFSET_BAD            121
#define DW_DLE_STRING_PTR_NULL                 122
#define DW_DLE_PUBNAMES_VERSION_ERROR          123
#define DW_DLE_PUBNAMES_LENGTH_BAD             124
#define DW_DLE_GLOBAL_NULL                     125
#define DW_DLE_GLOBAL_CONTEXT_NULL             126
#define DW_DLE_DIR_INDEX_BAD                   127
#define DW_DLE_LOC_EXPR_BAD                    128
#define DW_DLE_DIE_LOC_EXPR_BAD                129
#define DW_DLE_ADDR_ALLOC                      130
#define DW_DLE_OFFSET_BAD                      131
#define DW_DLE_MAKE_CU_CONTEXT_FAIL            132
#define DW_DLE_REL_ALLOC                       133
#define DW_DLE_ARANGE_OFFSET_BAD               134
#define DW_DLE_SEGMENT_SIZE_BAD                135
#define DW_DLE_ARANGE_LENGTH_BAD               136
#define DW_DLE_ARANGE_DECODE_ERROR             137
#define DW_DLE_ARANGES_NULL                    138
#define DW_DLE_ARANGE_NULL                     139
#define DW_DLE_NO_FILE_NAME                    140
#define DW_DLE_NO_COMP_DIR                     141
#define DW_DLE_CU_ADDRESS_SIZE_BAD             142
#define DW_DLE_INPUT_ATTR_BAD                  143
#define DW_DLE_EXPR_NULL                       144
#define DW_DLE_BAD_EXPR_OPCODE                 145
#define DW_DLE_EXPR_LENGTH_BAD                 146
#define DW_DLE_MULTIPLE_RELOC_IN_EXPR          147
#define DW_DLE_ELF_GETIDENT_ERROR              148
#define DW_DLE_NO_AT_MIPS_FDE                  149
#define DW_DLE_NO_CIE_FOR_FDE                  150
#define DW_DLE_DIE_ABBREV_LIST_NULL            151
#define DW_DLE_DEBUG_FUNCNAMES_DUPLICATE       152
#define DW_DLE_DEBUG_FUNCNAMES_NULL            153
#define DW_DLE_DEBUG_FUNCNAMES_VERSION_ERROR   154
#define DW_DLE_DEBUG_FUNCNAMES_LENGTH_BAD      155
#define DW_DLE_FUNC_NULL                       156
#define DW_DLE_FUNC_CONTEXT_NULL               157
#define DW_DLE_DEBUG_TYPENAMES_DUPLICATE       158
#define DW_DLE_DEBUG_TYPENAMES_NULL            159
#define DW_DLE_DEBUG_TYPENAMES_VERSION_ERROR   160
#define DW_DLE_DEBUG_TYPENAMES_LENGTH_BAD      161
#define DW_DLE_TYPE_NULL                       162
#define DW_DLE_TYPE_CONTEXT_NULL               163
#define DW_DLE_DEBUG_VARNAMES_DUPLICATE        164
#define DW_DLE_DEBUG_VARNAMES_NULL             165
#define DW_DLE_DEBUG_VARNAMES_VERSION_ERROR    166
#define DW_DLE_DEBUG_VARNAMES_LENGTH_BAD       167
#define DW_DLE_VAR_NULL                        168
#define DW_DLE_VAR_CONTEXT_NULL                169
#define DW_DLE_DEBUG_WEAKNAMES_DUPLICATE       170
#define DW_DLE_DEBUG_WEAKNAMES_NULL            171
#define DW_DLE_DEBUG_WEAKNAMES_VERSION_ERROR   172
#define DW_DLE_DEBUG_WEAKNAMES_LENGTH_BAD      173
#define DW_DLE_WEAK_NULL                       174
#define DW_DLE_WEAK_CONTEXT_NULL               175
#define DW_DLE_LOCDESC_COUNT_WRONG             176
#define DW_DLE_MACINFO_STRING_NULL             177
#define DW_DLE_MACINFO_STRING_EMPTY            178
#define DW_DLE_MACINFO_INTERNAL_ERROR_SPACE    179
#define DW_DLE_MACINFO_MALLOC_FAIL             180
#define DW_DLE_DEBUGMACINFO_ERROR              181
#define DW_DLE_DEBUG_MACRO_LENGTH_BAD          182
#define DW_DLE_DEBUG_MACRO_MAX_BAD             183
#define DW_DLE_DEBUG_MACRO_INTERNAL_ERR        184
#define DW_DLE_DEBUG_MACRO_MALLOC_SPACE        185
#define DW_DLE_DEBUG_MACRO_INCONSISTENT        186
#define DW_DLE_DF_NO_CIE_AUGMENTATION          187
#define DW_DLE_DF_REG_NUM_TOO_HIGH             188
#define DW_DLE_DF_MAKE_INSTR_NO_INIT           189
#define DW_DLE_DF_NEW_LOC_LESS_OLD_LOC         190
#define DW_DLE_DF_POP_EMPTY_STACK              191
#define DW_DLE_DF_ALLOC_FAIL                   192
#define DW_DLE_DF_FRAME_DECODING_ERROR         193
#define DW_DLE_DEBUG_LOC_SECTION_SHORT         194
#define DW_DLE_FRAME_AUGMENTATION_UNKNOWN      195
#define DW_DLE_PUBTYPE_CONTEXT                 196 /* Unused. */
#define DW_DLE_DEBUG_PUBTYPES_LENGTH_BAD       197
#define DW_DLE_DEBUG_PUBTYPES_VERSION_ERROR    198
#define DW_DLE_DEBUG_PUBTYPES_DUPLICATE        199
#define DW_DLE_FRAME_CIE_DECODE_ERROR          200
#define DW_DLE_FRAME_REGISTER_UNREPRESENTABLE  201
#define DW_DLE_FRAME_REGISTER_COUNT_MISMATCH   202
#define DW_DLE_LINK_LOOP                       203
#define DW_DLE_STRP_OFFSET_BAD                 204
#define DW_DLE_DEBUG_RANGES_DUPLICATE          205
#define DW_DLE_DEBUG_RANGES_OFFSET_BAD         206
#define DW_DLE_DEBUG_RANGES_MISSING_END        207
#define DW_DLE_DEBUG_RANGES_OUT_OF_MEM         208
#define DW_DLE_DEBUG_SYMTAB_ERR                209
#define DW_DLE_DEBUG_STRTAB_ERR                210
#define DW_DLE_RELOC_MISMATCH_INDEX            211
#define DW_DLE_RELOC_MISMATCH_RELOC_INDEX      212
#define DW_DLE_RELOC_MISMATCH_STRTAB_INDEX     213
#define DW_DLE_RELOC_SECTION_MISMATCH          214
#define DW_DLE_RELOC_SECTION_MISSING_INDEX     215
#define DW_DLE_RELOC_SECTION_LENGTH_ODD        216
#define DW_DLE_RELOC_SECTION_PTR_NULL          217
#define DW_DLE_RELOC_SECTION_MALLOC_FAIL       218
#define DW_DLE_NO_ELF64_SUPPORT                219
#define DW_DLE_MISSING_ELF64_SUPPORT           220
#define DW_DLE_ORPHAN_FDE                      221
#define DW_DLE_DUPLICATE_INST_BLOCK            222
#define DW_DLE_BAD_REF_SIG8_FORM               223
#define DW_DLE_ATTR_EXPRLOC_FORM_BAD           224
#define DW_DLE_FORM_SEC_OFFSET_LENGTH_BAD      225
#define DW_DLE_NOT_REF_FORM                    226
#define DW_DLE_DEBUG_FRAME_LENGTH_NOT_MULTIPLE 227
#define DW_DLE_REF_SIG8_NOT_HANDLED            228
#define DW_DLE_DEBUG_FRAME_POSSIBLE_ADDRESS_BOTCH 229
#define DW_DLE_LOC_BAD_TERMINATION             230
#define DW_DLE_SYMTAB_SECTION_LENGTH_ODD       231
#define DW_DLE_RELOC_SECTION_SYMBOL_INDEX_BAD  232
#define DW_DLE_RELOC_SECTION_RELOC_TARGET_SIZE_UNKNOWN  233
#define DW_DLE_SYMTAB_SECTION_ENTRYSIZE_ZERO   234
#define DW_DLE_LINE_NUMBER_HEADER_ERROR        235
#define DW_DLE_DEBUG_TYPES_NULL                236
#define DW_DLE_DEBUG_TYPES_DUPLICATE           237
#define DW_DLE_DEBUG_TYPES_ONLY_DWARF4         238
#define DW_DLE_DEBUG_TYPEOFFSET_BAD            239
#define DW_DLE_GNU_OPCODE_ERROR                240
#define DW_DLE_DEBUGPUBTYPES_ERROR             241
#define DW_DLE_AT_FIXUP_NULL                   242
#define DW_DLE_AT_FIXUP_DUP                    243
#define DW_DLE_BAD_ABINAME                     244
#define DW_DLE_TOO_MANY_DEBUG                  245
#define DW_DLE_DEBUG_STR_OFFSETS_DUPLICATE     246
#define DW_DLE_SECTION_DUPLICATION             247
#define DW_DLE_SECTION_ERROR                   248
#define DW_DLE_DEBUG_ADDR_DUPLICATE            249
#define DW_DLE_DEBUG_CU_UNAVAILABLE_FOR_FORM   250
#define DW_DLE_DEBUG_FORM_HANDLING_INCOMPLETE  251
#define DW_DLE_NEXT_DIE_PAST_END               252
#define DW_DLE_NEXT_DIE_WRONG_FORM             253
#define DW_DLE_NEXT_DIE_NO_ABBREV_LIST         254
#define DW_DLE_NESTED_FORM_INDIRECT_ERROR      255
#define DW_DLE_CU_DIE_NO_ABBREV_LIST           256
#define DW_DLE_MISSING_NEEDED_DEBUG_ADDR_SECTION 257
#define DW_DLE_ATTR_FORM_NOT_ADDR_INDEX        258
#define DW_DLE_ATTR_FORM_NOT_STR_INDEX         259
#define DW_DLE_DUPLICATE_GDB_INDEX             260
#define DW_DLE_ERRONEOUS_GDB_INDEX_SECTION     261
#define DW_DLE_GDB_INDEX_COUNT_ERROR           262
#define DW_DLE_GDB_INDEX_COUNT_ADDR_ERROR      263
#define DW_DLE_GDB_INDEX_INDEX_ERROR           264
#define DW_DLE_GDB_INDEX_CUVEC_ERROR           265
#define DW_DLE_DUPLICATE_CU_INDEX              266
#define DW_DLE_DUPLICATE_TU_INDEX              267
#define DW_DLE_XU_TYPE_ARG_ERROR               268
#define DW_DLE_XU_IMPOSSIBLE_ERROR             269
#define DW_DLE_XU_NAME_COL_ERROR               270
#define DW_DLE_XU_HASH_ROW_ERROR               271
#define DW_DLE_XU_HASH_INDEX_ERROR             272
/* ..._FAILSAFE_ERRVAL is an aid when out of memory. */
#define DW_DLE_FAILSAFE_ERRVAL                 273
#define DW_DLE_ARANGE_ERROR                    274
#define DW_DLE_PUBNAMES_ERROR                  275
#define DW_DLE_FUNCNAMES_ERROR                 276
#define DW_DLE_TYPENAMES_ERROR                 277
#define DW_DLE_VARNAMES_ERROR                  278
#define DW_DLE_WEAKNAMES_ERROR                 279
#define DW_DLE_RELOCS_ERROR                    280
#define DW_DLE_ATTR_OUTSIDE_SECTION            281
#define DW_DLE_FISSION_INDEX_WRONG             282
#define DW_DLE_FISSION_VERSION_ERROR           283
#define DW_DLE_NEXT_DIE_LOW_ERROR              284
#define DW_DLE_CU_UT_TYPE_ERROR                285
#define DW_DLE_NO_SUCH_SIGNATURE_FOUND         286
#define DW_DLE_SIGNATURE_SECTION_NUMBER_WRONG  287
#define DW_DLE_ATTR_FORM_NOT_DATA8             288
#define DW_DLE_SIG_TYPE_WRONG_STRING           289
#define DW_DLE_MISSING_REQUIRED_TU_OFFSET_HASH 290
#define DW_DLE_MISSING_REQUIRED_CU_OFFSET_HASH 291
#define DW_DLE_DWP_MISSING_DWO_ID              292
#define DW_DLE_DWP_SIBLING_ERROR               293
#define DW_DLE_DEBUG_FISSION_INCOMPLETE        294
#define DW_DLE_FISSION_SECNUM_ERR              295
#define DW_DLE_DEBUG_MACRO_DUPLICATE           296
#define DW_DLE_DEBUG_NAMES_DUPLICATE           297
#define DW_DLE_DEBUG_LINE_STR_DUPLICATE        298
#define DW_DLE_DEBUG_SUP_DUPLICATE             299
#define DW_DLE_NO_SIGNATURE_TO_LOOKUP          300
#define DW_DLE_NO_TIED_ADDR_AVAILABLE          301
#define DW_DLE_NO_TIED_SIG_AVAILABLE           302
#define DW_DLE_STRING_NOT_TERMINATED           303
#define DW_DLE_BAD_LINE_TABLE_OPERATION        304
#define DW_DLE_LINE_CONTEXT_BOTCH              305
#define DW_DLE_LINE_CONTEXT_INDEX_WRONG        306
#define DW_DLE_NO_TIED_STRING_AVAILABLE        307
#define DW_DLE_NO_TIED_FILE_AVAILABLE          308
#define DW_DLE_CU_TYPE_MISSING                 309
#define DW_DLE_LLE_CODE_UNKNOWN                310
#define DW_DLE_LOCLIST_INTERFACE_ERROR         311
#define DW_DLE_LOCLIST_INDEX_ERROR             312
#define DW_DLE_INTERFACE_NOT_SUPPORTED         313
#define DW_DLE_ZDEBUG_REQUIRES_ZLIB            314
#define DW_DLE_ZDEBUG_INPUT_FORMAT_ODD         315
#define DW_DLE_ZLIB_BUF_ERROR                  316
#define DW_DLE_ZLIB_DATA_ERROR                 317
#define DW_DLE_MACRO_OFFSET_BAD                318
#define DW_DLE_MACRO_OPCODE_BAD                319
#define DW_DLE_MACRO_OPCODE_FORM_BAD           320
#define DW_DLE_UNKNOWN_FORM                    321
#define DW_DLE_BAD_MACRO_HEADER_POINTER        322
#define DW_DLE_BAD_MACRO_INDEX                 323
#define DW_DLE_MACRO_OP_UNHANDLED              324
#define DW_DLE_MACRO_PAST_END                  325
#define DW_DLE_LINE_STRP_OFFSET_BAD            326
#define DW_DLE_STRING_FORM_IMPROPER            327
#define DW_DLE_ELF_FLAGS_NOT_AVAILABLE         328
#define DW_DLE_LEB_IMPROPER                    329
#define DW_DLE_DEBUG_LINE_RANGE_ZERO           330
#define DW_DLE_READ_LITTLEENDIAN_ERROR         331
#define DW_DLE_READ_BIGENDIAN_ERROR            332
#define DW_DLE_RELOC_INVALID                   333
#define DW_DLE_INFO_HEADER_ERROR               334
#define DW_DLE_ARANGES_HEADER_ERROR            335
#define DW_DLE_LINE_OFFSET_WRONG_FORM          336
#define DW_DLE_FORM_BLOCK_LENGTH_ERROR         337
#define DW_DLE_ZLIB_SECTION_SHORT              338
#define DW_DLE_CIE_INSTR_PTR_ERROR             339
#define DW_DLE_FDE_INSTR_PTR_ERROR             340
#define DW_DLE_FISSION_ADDITION_ERROR          341
#define DW_DLE_HEADER_LEN_BIGGER_THAN_SECSIZE  342
#define DW_DLE_LOCEXPR_OFF_SECTION_END         343
#define DW_DLE_POINTER_SECTION_UNKNOWN         344
#define DW_DLE_ERRONEOUS_XU_INDEX_SECTION      345
#define DW_DLE_DIRECTORY_FORMAT_COUNT_VS_DIRECTORIES_MISMATCH 346
#define DW_DLE_COMPRESSED_EMPTY_SECTION        347
#define DW_DLE_SIZE_WRAPAROUND                 348
#define DW_DLE_ILLOGICAL_TSEARCH               349
#define DW_DLE_BAD_STRING_FORM                 350
#define DW_DLE_DEBUGSTR_ERROR                  351
#define DW_DLE_DEBUGSTR_UNEXPECTED_REL         352
#define DW_DLE_DISCR_ARRAY_ERROR               353
#define DW_DLE_LEB_OUT_ERROR                   354
#define DW_DLE_SIBLING_LIST_IMPROPER           355
#define DW_DLE_LOCLIST_OFFSET_BAD              356
#define DW_DLE_LINE_TABLE_BAD                  357
#define DW_DLE_DEBUG_LOClISTS_DUPLICATE        358
#define DW_DLE_DEBUG_RNGLISTS_DUPLICATE        359
#define DW_DLE_ABBREV_OFF_END                  360
#define DW_DLE_FORM_STRING_BAD_STRING          361
#define DW_DLE_AUGMENTATION_STRING_OFF_END     362
#define DW_DLE_STRING_OFF_END_PUBNAMES_LIKE    363
#define DW_DLE_LINE_STRING_BAD                 364
#define DW_DLE_DEFINE_FILE_STRING_BAD          365
#define DW_DLE_MACRO_STRING_BAD                366
#define DW_DLE_MACINFO_STRING_BAD              367
#define DW_DLE_ZLIB_UNCOMPRESS_ERROR           368
#define DW_DLE_IMPROPER_DWO_ID                 369
#define DW_DLE_GROUPNUMBER_ERROR               370
#define DW_DLE_ADDRESS_SIZE_ZERO               371
#define DW_DLE_DEBUG_NAMES_HEADER_ERROR        372
#define DW_DLE_DEBUG_NAMES_AUG_STRING_ERROR    373
#define DW_DLE_DEBUG_NAMES_PAD_NON_ZERO        374
#define DW_DLE_DEBUG_NAMES_OFF_END             375
#define DW_DLE_DEBUG_NAMES_ABBREV_OVERFLOW     376
#define DW_DLE_DEBUG_NAMES_ABBREV_CORRUPTION   377
#define DW_DLE_DEBUG_NAMES_NULL_POINTER        378
#define DW_DLE_DEBUG_NAMES_BAD_INDEX_ARG       379
#define DW_DLE_DEBUG_NAMES_ENTRYPOOL_OFFSET    380
#define DW_DLE_DEBUG_NAMES_UNHANDLED_FORM      381
#define DW_DLE_LNCT_CODE_UNKNOWN               382
#define DW_DLE_LNCT_FORM_CODE_NOT_HANDLED      383
#define DW_DLE_LINE_HEADER_LENGTH_BOTCH        384
#define DW_DLE_STRING_HASHTAB_IDENTITY_ERROR   385
#define DW_DLE_UNIT_TYPE_NOT_HANDLED           386
#define DW_DLE_GROUP_MAP_ALLOC                 387
#define DW_DLE_GROUP_MAP_DUPLICATE             388
#define DW_DLE_GROUP_COUNT_ERROR               389
#define DW_DLE_GROUP_INTERNAL_ERROR            390
#define DW_DLE_GROUP_LOAD_ERROR                391
#define DW_DLE_GROUP_LOAD_READ_ERROR           392
#define DW_DLE_AUG_DATA_LENGTH_BAD             393
#define DW_DLE_ABBREV_MISSING                  394
#define DW_DLE_NO_TAG_FOR_DIE                  395
#define DW_DLE_LOWPC_WRONG_CLASS               396
#define DW_DLE_HIGHPC_WRONG_FORM               397
#define DW_DLE_STR_OFFSETS_BASE_WRONG_FORM     398
#define DW_DLE_DATA16_OUTSIDE_SECTION          399
#define DW_DLE_LNCT_MD5_WRONG_FORM             400
#define DW_DLE_LINE_HEADER_CORRUPT             401
#define DW_DLE_STR_OFFSETS_NULLARGUMENT        402
#define DW_DLE_STR_OFFSETS_NULL_DBG            403
#define DW_DLE_STR_OFFSETS_NO_MAGIC            404
#define DW_DLE_STR_OFFSETS_ARRAY_SIZE          405
#define DW_DLE_STR_OFFSETS_VERSION_WRONG       406
#define DW_DLE_STR_OFFSETS_ARRAY_INDEX_WRONG   407
#define DW_DLE_STR_OFFSETS_EXTRA_BYTES         408
#define DW_DLE_DUP_ATTR_ON_DIE                 409
#define DW_DLE_SECTION_NAME_BIG                410
#define DW_DLE_FILE_UNAVAILABLE                411
#define DW_DLE_FILE_WRONG_TYPE                 412
#define DW_DLE_SIBLING_OFFSET_WRONG            413
#define DW_DLE_OPEN_FAIL                       414
#define DW_DLE_OFFSET_SIZE                     415
#define DW_DLE_MACH_O_SEGOFFSET_BAD            416
#define DW_DLE_FILE_OFFSET_BAD                 417
#define DW_DLE_SEEK_ERROR                      418
#define DW_DLE_READ_ERROR                      419
#define DW_DLE_ELF_CLASS_BAD                   420
#define DW_DLE_ELF_ENDIAN_BAD                  421
#define DW_DLE_ELF_VERSION_BAD                 422
#define DW_DLE_FILE_TOO_SMALL                  423
#define DW_DLE_PATH_SIZE_TOO_SMALL             424
#define DW_DLE_BAD_TYPE_SIZE                   425
#define DW_DLE_PE_SIZE_SMALL                   426
#define DW_DLE_PE_OFFSET_BAD                   427
#define DW_DLE_PE_STRING_TOO_LONG              428
#define DW_DLE_IMAGE_FILE_UNKNOWN_TYPE         429
#define DW_DLE_LINE_TABLE_LINENO_ERROR         430
#define DW_DLE_PRODUCER_CODE_NOT_AVAILABLE     431
#define DW_DLE_NO_ELF_SUPPORT                  432
#define DW_DLE_NO_STREAM_RELOC_SUPPORT         433
#define DW_DLE_RETURN_EMPTY_PUBNAMES_ERROR     434
#define DW_DLE_SECTION_SIZE_ERROR              435
#define DW_DLE_INTERNAL_NULL_POINTER           436
#define DW_DLE_SECTION_STRING_OFFSET_BAD       437
#define DW_DLE_SECTION_INDEX_BAD               438
#define DW_DLE_INTEGER_TOO_SMALL               439
#define DW_DLE_ELF_SECTION_LINK_ERROR          440
#define DW_DLE_ELF_SECTION_GROUP_ERROR         441
#define DW_DLE_ELF_SECTION_COUNT_MISMATCH      442
#define DW_DLE_ELF_STRING_SECTION_MISSING      443
#define DW_DLE_SEEK_OFF_END                    444
#define DW_DLE_READ_OFF_END                    445
#define DW_DLE_ELF_SECTION_ERROR               446
#define DW_DLE_ELF_STRING_SECTION_ERROR        447
#define DW_DLE_MIXING_SPLIT_DWARF_VERSIONS     448
#define DW_DLE_TAG_CORRUPT                     449
#define DW_DLE_FORM_CORRUPT                    450
#define DW_DLE_ATTR_CORRUPT                    451
#define DW_DLE_ABBREV_ATTR_DUPLICATION         452
#define DW_DLE_DWP_SIGNATURE_MISMATCH          453
#define DW_DLE_CU_UT_TYPE_VALUE                454
#define DW_DLE_DUPLICATE_GNU_DEBUGLINK         455
#define DW_DLE_CORRUPT_GNU_DEBUGLINK           456
#define DW_DLE_CORRUPT_NOTE_GNU_DEBUGID        457
#define DW_DLE_CORRUPT_GNU_DEBUGID_SIZE        458
#define DW_DLE_CORRUPT_GNU_DEBUGID_STRING      459
#define DW_DLE_HEX_STRING_ERROR                460
#define DW_DLE_DECIMAL_STRING_ERROR            461
#define DW_DLE_PRO_INIT_EXTRAS_UNKNOWN         462
#define DW_DLE_PRO_INIT_EXTRAS_ERR             463
#define DW_DLE_NULL_ARGS_DWARF_ADD_PATH        464
#define DW_DLE_DWARF_INIT_DBG_NULL             465
#define DW_DLE_ELF_RELOC_SECTION_ERROR         466
#define DW_DLE_USER_DECLARED_ERROR             467
#define DW_DLE_RNGLISTS_ERROR                  468
#define DW_DLE_LOCLISTS_ERROR                  469
#define DW_DLE_SECTION_SIZE_OR_OFFSET_LARGE    470
#define DW_DLE_GDBINDEX_STRING_ERROR           471
#define DW_DLE_GNU_PUBNAMES_ERROR              472
#define DW_DLE_GNU_PUBTYPES_ERROR              473
#define DW_DLE_DUPLICATE_GNU_DEBUG_PUBNAMES    474
#define DW_DLE_DUPLICATE_GNU_DEBUG_PUBTYPES    475
#define DW_DLE_DEBUG_SUP_STRING_ERROR          476
#define DW_DLE_DEBUG_SUP_ERROR                 477
#define DW_DLE_LOCATION_ERROR                  478
#define DW_DLE_DEBUGLINK_PATH_SHORT            479
#define DW_DLE_SIGNATURE_MISMATCH              480
#define DW_DLE_MACRO_VERSION_ERROR             481
#define DW_DLE_NEGATIVE_SIZE                   482
#define DW_DLE_UDATA_VALUE_NEGATIVE            483
#define DW_DLE_DEBUG_NAMES_ERROR               484
#define DW_DLE_CFA_INSTRUCTION_ERROR           485
#define DW_DLE_MACHO_CORRUPT_HEADER            486
#define DW_DLE_MACHO_CORRUPT_COMMAND           487
#define DW_DLE_MACHO_CORRUPT_SECTIONDETAILS    488
#define DW_DLE_RELOCATION_SECTION_SIZE_ERROR   489
#define DW_DLE_SYMBOL_SECTION_SIZE_ERROR       490
#define DW_DLE_PE_SECTION_SIZE_ERROR           491

/*! @note DW_DLE_LAST MUST EQUAL LAST ERROR NUMBER */
#define DW_DLE_LAST        491
#define DW_DLE_LO_USER     0x10000
/*! @} */

/*! @section initfinish Initialization And Finish Operations */

/*! @defgroup initfunctions Libdwarf Initialization Functions
    @{
*/

/*! @brief Initialization based on path, the most common
    initialization.

    @param dw_path
    Pass in the path to the object file to open.
    @param dw_true_path_out_buffer
    Pass in NULL or the name of a string buffer
    (The buffer should be initialized with an initial NUL byte)
    The returned string will be null-terminated.
    The path actually used is copied to true_path_out.
    If true_path_buffer len is zero or true_path_out_buffer
    is zero  then the Special MacOS processing will not
    occur, nor will the GNU_debuglink processing occur.
    In case GNU debuglink data was followed or MacOS
    dSYM applies the true_path_out
    will not match path.
    So consider the value put in true_path_out the
    actual file name.
    @param dw_true_path_bufferlen
    Pass in the length in bytes of the buffer.
    @param dw_groupnumber
    The value passed in should be DW_GROUPNUMBER_ANY
    unless one wishes to other than a standard
    group.
    @param dw_errhand
    Pass in NULL unless one wishes libdwarf to call
    this error handling function (which you must write)
    instead of passing meaningfull values to the
    dw_error argument.
    @param dw_errarg
    If dw_errorhand is non-null, then this value
    (a pointer or integer that means something
    to you) is passed to the dw_errhand function
    in case that is helpful to you.
    @param dw_dbg
    On success, *dw_dbg is set to a pointer to
    a new Dwarf_Debug structure to be used in
    calls to libdwarf functions.
    @param dw_error
    In case return is DW_DLV_ERROR
    dw_error is set to point to
    the error details.
    @return DW_DLV_OK etc.

    @see dwarf_init_path_dl dwarf_init_b
    @see exampleinit
*/
DW_API int dwarf_init_path(const char * dw_path,
    char *            dw_true_path_out_buffer,
    unsigned int      dw_true_path_bufferlen,
    unsigned int      dw_groupnumber,
    Dwarf_Handler     dw_errhand,
    Dwarf_Ptr         dw_errarg,
    Dwarf_Debug*      dw_dbg,
    Dwarf_Error*      dw_error);

/*! @brief Initialization following GNU debuglink section data.

    Sets the true-path with DWARF if there is
    appropriate debuglink data available.

    @param dw_path
    Pass in the path to the object file to open.
    @param dw_true_path_out_buffer
    Pass in NULL or the name of a string buffer.
    @param dw_true_path_bufferlen
    Pass in the length in bytes of the buffer.
    @param dw_groupnumber
    The value passed in should be DW_GROUPNUMBER_ANY
    unless one wishes to other than a standard
    group.
    @param dw_errhand
    Pass in NULL, normally.
    If non-null one wishes libdwarf to call
    this error handling function (which you must write)
    instead of passing meaningful values to the
    dw_error argument.
    @param dw_errarg
    Pass in NULL, normally.
    If dw_errorhand is non-null, then this value
    (a pointer or integer that means something
    to you) is passed to the dw_errhand function
    in case that is helpful to you.
    @param dw_dbg
    On success, *dw_dbg is set to a pointer to
    a new Dwarf_Debug structure to be used in
    calls to libdwarf functions.
    @param dw_dl_path_array
    debuglink processing allows a user-specified set of file paths
    and this argument allows one to specify these.
    @param dw_dl_path_array_size
    Specify the size of the dw_dl_path_array.
    @param dw_dl_path_source
    returns  DW_PATHSOURCE_basic or other such value so the caller can
    know how the true-path was resolved.
    @param dw_error
    In case return is DW_DLV_ERROR
    dw_error is set to point to
    the error details.
    @return DW_DLV_OK etc.

    @see exampleinit_dl
    */
DW_API int dwarf_init_path_dl(const char * dw_path,
    char *            dw_true_path_out_buffer,
    unsigned int      dw_true_path_bufferlen,
    unsigned int      dw_groupnumber,
    Dwarf_Handler     dw_errhand,
    Dwarf_Ptr         dw_errarg,
    Dwarf_Debug*      dw_dbg,
    char **           dw_dl_path_array,
    unsigned int      dw_dl_path_array_size,
    unsigned char   * dw_dl_path_source,
    Dwarf_Error*      dw_error);

/*! @brief Initialization based on Unix/Linux (etc) path
    This version allows specifying any number of debuglink
    global paths to search on for debuglink targets.

    @param dw_fd
    An open Unix/Linux/etc fd on the object file.
    @param dw_groupnumber
    The value passed in should be DW_GROUPNUMBER_ANY
    unless one wishes to other than a standard
    group.
    @param dw_errhand
    Pass in NULL unless one wishes libdwarf to call
    this error handling function (which you must write)
    instead of passing meaningfull values to the
    dw_error argument.
    @param dw_errarg
    If dw_errorhand is non-null, then this value
    (a pointer or integer that means something
    to you) is passed to the dw_errhand function
    in case that is helpful to you.
    @param dw_dbg
    On success, *dw_dbg is set to a pointer to
    a new Dwarf_Debug structure to be used in
    calls to libdwarf functions.
    @param dw_error
    In case return is DW_DLV_ERROR
    dw_error is set to point to
    the error details.
    @return
    DW_DLV_OK etc.
*/
DW_API int dwarf_init_b(int dw_fd,
    unsigned int      dw_groupnumber,
    Dwarf_Handler     dw_errhand,
    Dwarf_Ptr         dw_errarg,
    Dwarf_Debug*      dw_dbg,
    Dwarf_Error*      dw_error);

/*! @brief Close the initialized dw_dbg and
    free all data libdwarf has for this dw_dbg.
    @param dw_dbg
    Close the dbg.
    @return
    May return DW_DLV_ERROR if something
    is very wrong: no further information is available..
    May return DW_DLV_NO_ENTRY
    but no further information is available.
    Normally returns DW_DLV_OK.
*/
DW_API int dwarf_finish(Dwarf_Debug dw_dbg);

/*! @brief Used to access DWARF information in memory
    or in an object format unknown to libdwarf.

    @see jitreader

    @param dw_obj
    A data structure filled out by the caller so libdwarf
    can access DWARF data not in a supported object file format.
    @param dw_errhand
    Pass in NULL normally.
    @param dw_errarg
    Pass in NULL normally.
    @param dw_groupnumber
    The value passed in should be DW_GROUPNUMBER_ANY
    unless one wishes to other than a standard
    group (quite unlikely for this interface).
    @param dw_dbg
    On success, *dw_dbg is set to a pointer to
    a new Dwarf_Debug structure to be used in
    calls to libdwarf functions.
    @param dw_error
    In case return is DW_DLV_ERROR
    dw_error is set to point to
    the error details.
    @return
    The usual value: DW_DLV_OK etc.
*/
DW_API int dwarf_object_init_b(Dwarf_Obj_Access_Interface_a* dw_obj,
    Dwarf_Handler dw_errhand,
    Dwarf_Ptr     dw_errarg,
    unsigned int  dw_groupnumber,
    Dwarf_Debug*  dw_dbg,
    Dwarf_Error*  dw_error);

/*! @brief Used to close the object_init dw_dbg.

    Close the dw_dbg opened by dwarf_object_init_b().

    @param dw_dbg
    Must be an open Dwarf_Debug opened by
    dwarf_object_init_b().
    The init call dw_obj data is not freed
    by the call to dwarf_object_finish.
    @return
    The return value DW_DLV_OK etc is pretty useless, there
    is not much you can do with it.
*/
DW_API int dwarf_object_finish(Dwarf_Debug dw_dbg);

/*! @brief Use with split dwarf.

    @param dw_basedbg
    Pass in an open dbg, on an object file
    with (normally) lots of DWARF..
    @param dw_tied_dbg
    Pass in an open dbg on an executable
    which has minimal DWARF to save space
    in the executable.
    @param dw_error
    In case return is DW_DLV_ERROR
    dw_error is set to point to
    the error details.
    @return DW_DLV_OK etc.

    @see example2
    @see example3
*/
DW_API int dwarf_set_tied_dbg(Dwarf_Debug dw_basedbg,
    Dwarf_Debug  dw_tied_dbg,
    Dwarf_Error* dw_error);

/*! @brief Use with split dwarf.

    Given a base Dwarf_Debug this returns
    the tied Dwarf_Debug.
    Unlikely anyone uses this call as
    you had the tied and base dbg when calling
    dwarf_set_tied_dbg().
*/
DW_API int dwarf_get_tied_dbg(Dwarf_Debug dw_dbg,
    Dwarf_Debug * dw_tieddbg_out,
    Dwarf_Error * dw_error);
/*! @}
*/
/*! @defgroup compilationunit Compilation Unit (CU) Access
    @{
*/
/*! @brief Returns information on the next CU header.

    The library keeps track of where it is in the object file
    and it knows where to find 'next'.
    @param dw_dbg
    The currend Dwarf_Debug of interest.
    @param dw_is_info
    Pass in TRUE if reading through .debug_info
    Pass in FALSE if reading through DWARF4
    .debug_types.
    @param dw_cu_header_length
    Returns the length of the just-read CU header.
    @param dw_version_stamp
    Returns the version number (2 to 5) of the CU
    header just read.
    @param dw_abbrev_offset
    Returns the .debug_abbrev offset from the
    the CU header just read.
    @param dw_address_size
    Returns the address size specified for this CU,
    usually either 4 or 8.
    @param dw_length_size
    Returns the offset size (the length
    of the size field from the header)
    specified for this CU, either 4 or 4.
    @param dw_extension_size
    If the section is standard 64bit DWARF
    then this value is 4. Else the value is zero.
    @param dw_type_signature
    If the CU is  DW_UT_skeleton DW_UT_split_compile,
    DW_UT_split_type or DW_UT_type this is the
    type signature from the CU_header
    is compied into this field.
    @param dw_typeoffset
    For DW_UT_split_type or DW_UT_type this is the
    type offset from the CU header.
    @param dw_next_cu_header_offset
    The offset in the section of the next CU
    (unless there is a compiler bug this is rarely
    of interest).
    @param dw_header_cu_type
    Returns DW_UT_compile, or other DW_UT value.
    @param dw_error
    In case return is DW_DLV_ERROR
    dw_error is set to point to
    the error details.
    @return
    Returns DW_DLV_OK on success.
    Returns DW_DLV_NO_ENTRY if all CUs have been read.
*/
DW_API int dwarf_next_cu_header_d(Dwarf_Debug dw_dbg,
    Dwarf_Bool      dw_is_info,
    Dwarf_Unsigned* dw_cu_header_length,
    Dwarf_Half*     dw_version_stamp,
    Dwarf_Off*      dw_abbrev_offset,
    Dwarf_Half*     dw_address_size,
    Dwarf_Half*     dw_length_size,
    Dwarf_Half*     dw_extension_size,
    Dwarf_Sig8*     dw_type_signature,
    Dwarf_Unsigned* dw_typeoffset,
    Dwarf_Unsigned* dw_next_cu_header_offset,
    Dwarf_Half    * dw_header_cu_type,
    Dwarf_Error*    dw_error);

/*! @brief Retrieve the first DIE or the next sibling.

    @param dw_dbg
    The Dwarf_Debug one is operating on.
    @param dw_die
    Immediately after calling dwarf_next_cu_header_d
    pass in NULL to retrieve the CU DIE.
    Or pass in a known DIE and this will retrieve
    the next sibling in the chain.
    @param dw_is_info
    Pass TRUE or FALSE to match the applicable
    dwarf_next_cu_header_d call.
    @param dw_return_siblingdie
    The DIE returned through the pointer.
    @param dw_error
    The usual error information, if any.
    @return
    Returns DW_DLV_OK etc.

    @see dwarf_get_die_infotypes
*/
DW_API int dwarf_siblingof_b(Dwarf_Debug dw_dbg,
    Dwarf_Die        dw_die,
    Dwarf_Bool       dw_is_info,
    Dwarf_Die*       dw_return_siblingdie,
    Dwarf_Error*     dw_error);

/*! @brief Some CU-relative facts.

    Any Dwarf_Die will work.
    The values returned through the pointers
    are about the CU for a DIE
    @param dw_die
    Some open Dwarf_Die.
    @param dw_version
    Returns the DWARF version: 2,4,5, or 5
    @param dw_is_info
    Returns non-zero if the CU is .debug_info.
    Returns zero if the CU is .debug_types (DWARF4).
    @param dw_is_dwo
    Returns ton-zero if the CU is a dwo/dwp object and
    zero if it is a standard object.
    @param dw_offset_size
    Returns offset size, 4 and 8 are possible.
    @param dw_address_size
    Almost always returns 4 or 8. Could be 2
    in unusual circumstances.
    @param dw_extension_size
    The sum of dw_offset_size and dw_extension_size
    are the count of the initial bytes of the CU.
    Standard lengths are 4 and 12.
    For 1990's SGI objects the length could be 8.
    @param dw_signature
    Returns a pointer to an 8 byte signature.
    @param dw_offset_of_length
    Returns the section offset of the initial
    byte of the CU.
    @param dw_total_byte_length
    Returns the total length of the CU including
    the length field and the content of the CU.
    @param dw_error
    The usual Dwarf_Error*.
    @return
    Returns DW_DLV_OK etc.
*/

DW_API int dwarf_cu_header_basics(Dwarf_Die dw_die,
    Dwarf_Half     * dw_version,
    Dwarf_Bool     * dw_is_info,
    Dwarf_Bool     * dw_is_dwo,
    Dwarf_Half     * dw_offset_size,
    Dwarf_Half     * dw_address_size,
    Dwarf_Half     * dw_extension_size,
    Dwarf_Sig8    ** dw_signature,
    Dwarf_Off      * dw_offset_of_length,
    Dwarf_Unsigned * dw_total_byte_length,
    Dwarf_Error    * dw_error);

/*! @brief Get the child DIE, if any.
    The child may be the first of a list of
    sibling DIEs.

    @param dw_die
    We will return the first child of this DIE.
    @param dw_return_childdie
    Returns the first child through the pointer.
    For subsequent dies siblings of the first, use
    dwarf_siblingof_b().
    @param dw_error
    The usual Dwarf_Error*.
    @return
    Returns DW_DLV_OK etc. Returns DW_DLV_NO_ENTRY
    if dw_die has no children.
*/

DW_API int dwarf_child(Dwarf_Die dw_die,
    Dwarf_Die*    dw_return_childdie,
    Dwarf_Error*  dw_error);

/*! @brief  Deallocate (free) a DIE.
    @param dw_die
    Frees (deallocs) memory associated with this Dwarf_Die.
*/
DW_API void dwarf_dealloc_die( Dwarf_Die dw_die);

/*! @brief Given a has signature, retrieve the applicable CU die

    @param dw_dbg
    @param dw_hash_sig
    A pointer to an 8 byte signature to be looked up.
    in .debug_names.
    @param dw_sig_type
    Valid type requests are "cu" and "tu"
    @param dw_returned_CU_die
    Returnes the found CU DIE if one is found.
    @param dw_error
    The usual Dwarf_Error*.
    @return
    DW_DLV_OK means dw_returned_CU_die was set.
    DW_DLV_NO_ENTRY means  the signature could
    not be found.
*/
DW_API int dwarf_die_from_hash_signature(Dwarf_Debug dw_dbg,
    Dwarf_Sig8 *  dw_hash_sig,
    const char *  dw_sig_type,
    Dwarf_Die*    dw_returned_CU_die,
    Dwarf_Error*  dw_error);

/*! @brief Finding die given global (not CU-relative) offset.

    This works whether or not the target section
    has had  dwarf_next_cu_header_d() applied,
    the CU the offset exists in has
    been seen at all, or the target offset is one
    libdwarf has seen before.

    @param dw_dbg
    The applicable Dwarf_Debug
    @param dw_offset
    The global offset of the DIE in the appropriate
    section.
    @param dw_is_info
    Pass TRUE if the target is .debug_info, else
    pass FALSE if the target is .debug_types.
    @param dw_return_die
    On  success this returns a DIE pointer to
    the found DIE.
    @param dw_error
    The usual Dwarf_Error*.
    @return
    DW_DLV_OK means dw_returned_die was found
    DW_DLV_NO_ENTRY is only possible if the offset
    is to a null DIE, and that is very unusual.
    Otherwise expect DW_DLV_ERROR.

*/
DW_API int dwarf_offdie_b(Dwarf_Debug dw_dbg,
    Dwarf_Off        dw_offset,
    Dwarf_Bool       dw_is_info,
    Dwarf_Die*       dw_return_die,
    Dwarf_Error*     dw_error);

/*! @brief Retrieves a DIE from a DW_UT_split_type
    or DW_UT_type CU.

returns DIE and
    is_info flag if it finds the referenced
    DW_UT_split_type or DW_UT_type CU.

    @param dw_dbg
    The applicable Dwarf_Debug
    @param dw_ref
    A pointer to a Dwarf_Sig8 struct whose
    content defines what is being searched for.
    @param dw_die_out
    If found, this returns the found DIE itself.
    @param dw_is_info
    If found, this returns section (.debug_is_info
    or .debug_is_types).
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_find_die_given_sig8(Dwarf_Debug dw_dbg,
    Dwarf_Sig8 * dw_ref,
    Dwarf_Die  * dw_die_out,
    Dwarf_Bool * dw_is_info,
    Dwarf_Error* dw_error);

/*! @brief Returns the is_info flag.

    So client software knows if a DIE is in debug_info or debug_types.
    @param dw_die
    The DIE being queried.
    @return
    If non-zero the flag means the DIE is in .debug_info.
    Otherwise it means the DIE is in .debug_types.
*/
DW_API Dwarf_Bool dwarf_get_die_infotypes_flag(Dwarf_Die dw_die);
/*! @} */

/*! @defgroup dieentry Debugging Information Entry (DIE) Access
    This is the main interface to attributes of a DIE.
    @{
*/

/*! @brief  
    So we can associate a DIE's abbreviations with the contents
    the abbreviations section.
    Useful for detailed printing and analysis of
    abbreviations

    @param dw_die
    The DIE of interest
    @param dw_abbrev_offset
    On success is set to the global offset
    in the .debug_abbrev section of the abbreviations
    for the DIE.
    @param dw_abbrev_count
    On success is set to the count of abbreviations
    in the .debug_abbrev section of the abbreviations
    for the DIE.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_die_abbrev_global_offset(Dwarf_Die dw_die,
    Dwarf_Off       * dw_abbrev_offset,
    Dwarf_Unsigned  * dw_abbrev_count,
    Dwarf_Error*      dw_error);

/*! @brief  Get TAG value of DIE. 
    @param dw_die
    The DIE of interest
    @param dw_return_tag
    On success, set to the DW_TAG value of the DIE.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_tag(Dwarf_Die dw_die,
    Dwarf_Half*      dw_return_tag,
    Dwarf_Error*     dw_error);

/*! @brief Returns the global section offset of the DIE.

    @param dw_die
    The DIE of interest
    @param dw_return_offset
    On success the offset refers to the section of the DIE
    itself, which may be .debug_offset or .debug_types.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_dieoffset(Dwarf_Die dw_die,
    Dwarf_Off*       dw_return_offset,
    Dwarf_Error*     dw_error);

/*! @brief Extract address given address index. DWARF5 

    @param dw_die
    The DIE of interest
    @param dw_index
    An index into .debug_addr.  This will look first for
    .debug_addr in the dbg object DIE and if not there
    will look in the tied object if that is available.
    @param dw_return_addr
    On success the address is returned through the pointer.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_debug_addr_index_to_addr(Dwarf_Die dw_die,
    Dwarf_Unsigned  dw_index,
    Dwarf_Addr    * dw_return_addr,
    Dwarf_Error   * dw_error);

/*! @brief Informs if a DW_FORM is an indexed form 

    Reading a CU DIE with DW_AT_low_pc an indexed value can
    be problematic as several different FORMs are indexed.
    Some in DWARF5 others being extensions to DWARF4
    and DWARF5. Indexed forms interact with DW_AT_addr_base
    in a DIE making this a very relevant distinction.
*/
DW_API Dwarf_Bool dwarf_addr_form_is_indexed(int dw_form);

/*! @brief Returns the CU DIE offset given any DIE

    Returns
    the global debug_info section offset of the CU die
    in the CU containing the given_die
    (the passed in DIE can be any DIE).

    @see dwarf_get_cu_die_offset_given_cu_header_offset_b

    @param dw_die
    The die being queried.
    @param dw_return_offset
    Returns the section offset of the CU DIE for dw_die.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_CU_dieoffset_given_die(Dwarf_Die dw_die,
    Dwarf_Off*       dw_return_offset,
    Dwarf_Error*     dw_error);

/*! @brief Returns the CU DIE section offset given CU header offset

    Returns the CU die global offset if one knows the
    CU header global offset.
    @see dwarf_CU_dieoffset_given_die

    @param dw_dbg
    The Dwarf_Debug of interest.
    @param dw_in_cu_header_offset
    The CU header offset.
    @param dw_is_info
    If TRUE the CU header offset is in .debug_info.
    Otherwise the CU header offset is in .debug_types.
    @param dw_out_cu_die_offset
    The CU DIE offset returned through this pointer.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_get_cu_die_offset_given_cu_header_offset_b(
    Dwarf_Debug  dw_dbg,
    Dwarf_Off    dw_in_cu_header_offset,
    Dwarf_Bool   dw_is_info,
    Dwarf_Off *  dw_out_cu_die_offset,
    Dwarf_Error *dw_error);

/*! @brief returns the CU relative offset of the DIE.

    @see dwarf_CU_dieoffset_given_die

    @param dw_die
    The DIE being queried.
    @param dw_return_offset
    Returns the CU relative offset of this DIE.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_die_CU_offset(Dwarf_Die dw_die,
    Dwarf_Off*       dw_return_offset,
    Dwarf_Error*     dw_error);

/*! @brief Returns the offset length of the entire CU of a DIE.
    @param dw_die
    The DIE being queried.
    @param dw_return_CU_header_offset
    On success returns the section offset of the CU
    this DIE is in.
    @param dw_return_CU_length_bytes
    On success returns the CU length of the CU
    this DIE is in, including the CU length, header,
    and all DIEs.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_die_CU_offset_range(Dwarf_Die dw_die,
    Dwarf_Off*   dw_return_CU_header_offset,
    Dwarf_Off*   dw_return_CU_length_bytes,
    Dwarf_Error* dw_error);

/*! @brief Given DIE and attribute number return a Dwarf_attribute

    Returns DW_DLV_NO_ENTRY if the DIE has no attribute dw_attrnum.
   
    @param dw_die
    The DIE of interest.
    @param dw_attrnum
    An attribute number, for example DW_AT_name.
    @param dw_returned_attr
    On success a Dwarf_Attribute pointer is returned
    and it should eventually be deallocated.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.

*/
DW_API int dwarf_attr(Dwarf_Die dw_die,
    Dwarf_Half        dw_attrnum,
    Dwarf_Attribute * dw_returned_attr,
    Dwarf_Error*      dw_error);

/*! @brief Given DIE and attribute number return a string

    Returns DW_DLV_NO_ENTRY if the DIE has no attribute dw_attrnum.
   
    @param dw_die
    The DIE of interest.
    @param dw_attrnum
    An attribute number, for example DW_AT_name.
    @param dw_ret_name
    On success a pointer to the string
    is returned. 
    Do not free the string.
    Many attributes allow various forms that directly or
    indirectly contain strings and this
    follows all of them to their string.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.

*/
DW_API int dwarf_die_text(Dwarf_Die dw_die,
    Dwarf_Half    dw_attrnum,
    char       ** dw_ret_name,
    Dwarf_Error * dw_error);

/*! @brief Return the string from a DW_AT_name attribute

    Returns DW_DLV_NO_ENTRY if the DIE has no attribute
    DW_AT_name

    @param dw_die
    The DIE of interest.
    @param dw_diename
    On success a pointer to the string
    is returned. 
    Do not free the string.
    Various forms directly or
    indirectly contain strings and this
    follows all of them to their string.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_diename(Dwarf_Die dw_die,
    char   **        dw_diename,
    Dwarf_Error*     dw_error);

/*! @ brief Return the DIE abbrev code

    The Abbrev code for a DIE is an integer assigned
    by the compiler within a particular CU.
    For .debug_names abbreviations the
    situration is different.

    Returns the  abbrev code of the die. Cannot fail.

    @param dw_die
    The DIE of interest.
    @return
    The abbrev code. of the DIE.
*/
DW_API int dwarf_die_abbrev_code(Dwarf_Die dw_die);

/*! @brief Returns TRUE if the DIE has children

    @param dw_die
    A DIE.
    @param dw_ab_has_child
    Sets TRUE though the pointer if the DIE 
    has children.
    Otherwise sets FALSE.
    @return
    Returns TRUE if the DIE has a child DIE.
    Else returns FALSE.

*/
DW_API int dwarf_die_abbrev_children_flag(Dwarf_Die dw_die,
    Dwarf_Half * dw_ab_has_child);

/*  !brief Validate a sibling DIE.

    To be documented.

    There is no Dwarf_Error argument here, 
    it does not report any errors it finds..

    @param dw_sibling
    A Dwarf_Die
    @param dw_offset
    A DIE offset is returned through the pointer.
    @return
    Returns DW_DLV_OK if it validates.
    Otherwise returns DW_DLV_ERROR.
*/
DW_API int dwarf_validate_die_sibling(Dwarf_Die dw_sibling,
    Dwarf_Off* dw_offset);

/* convenience functions, alternative to using dwarf_attrlist */

/*! @brief Tells whether a DIE has a particular attribute.

    @param dw_die
    The DIE of interest.
    @param dw_attrnum
    The attribute number we are asking about,
    DW_AT_name for example.
    @param dw_returned_bool
    On success is set TRUE if dw_die has
    dw_attrnum.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.

*/
DW_API int dwarf_hasattr(Dwarf_Die dw_die,
    Dwarf_Half   dw_attrnum,
    Dwarf_Bool * dw_returned_bool,
    Dwarf_Error* dw_error);

/*! @brief Returns an array of DIE children offsets

    Given a DIE offset and dw_is_info,
    returns an array of DIE offsets of the childred
    of DIE.
    @param dw_dbg
    The Dwarf_Debug of interest.
    @param dw_offset
    A DIE offset.
    @param dw_is_info
    If TRUE says to use the offset in .debug_info.
    Else .debug_types.
    @param dw_offbuf
    A pointer to an array of offsets is returned
    through the pointer.
    @param dw_offcount
    The number of elements in offbuf.
    IF the DIE has no children it could
    be zero, in which case offbuf
    and dw_offcount are not touched.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
    DW_DLV_NO_ENTRY means there are no children of the DIE,
    hence no list of child offsets.

    @see exampleoffsetlist
*/
DW_API int dwarf_offset_list(Dwarf_Debug dw_dbg,
    Dwarf_Off         dw_offset,
    Dwarf_Bool        dw_is_info,
    Dwarf_Off      ** dw_offbuf,
    Dwarf_Unsigned *  dw_offcount,
    Dwarf_Error    *  dw_error);

/*! @brief Get the address size applying to a DIE

    @param dw_die
    The DIE of interest.
    @param dw_addr_size
    On success, returns the address size that applies
    to dw_die. Normally 4 or 8.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_get_die_address_size(Dwarf_Die dw_die,
    Dwarf_Half  * dw_addr_size,
    Dwarf_Error * dw_error);

/* Get both offsets (local and global) */
/*! @brief Return section and CU-local offsets of a DIE
    @param dw_die
    The DIE of interest.
    @param dw_global_offset
    On success returns the offset of the DIE in
    its section.
    @param dw_local_offset
    On success returns the offset of the DIE within
    its CU.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_die_offsets(Dwarf_Die dw_die,
    Dwarf_Off*    dw_global_offset,
    Dwarf_Off*    dw_local_offset,
    Dwarf_Error*  dw_error);

/*! @brief Get the version and offset size

    The values returned apply to the CU this DIE
    belongs to.
    This is useful as preparation for calling
    dwarf_get_form_class

    @param dw_die
    The DIE of interest.
    @param dw_version
    Returns the version of the CU this DIE is contained in.
    Standard version numbers are 2 through 5.
    @param dw_offset_size
    Returns the offset_size (4 or 8) of the CU
    this DIE is contained in.
*/
DW_API int dwarf_get_version_of_die(Dwarf_Die dw_die,
    Dwarf_Half * dw_version,
    Dwarf_Half * dw_offset_size);

/*! @brief Returns the DW_AT_low_pc  value

    @param dw_die
    The DIE of interest.
    @param dw_returned_addr
    On success returns, through the pointer,
    the address DW_AT_low_pc defines.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.

*/
DW_API int dwarf_lowpc(Dwarf_Die dw_die,
    Dwarf_Addr * dw_returned_addr,
    Dwarf_Error* dw_error);

/*  When the highpc attribute is of class  'constant'
    it is not an address, it is an offset from the
    base address (such as lowpc) of the function.
    This is therefore a required interface for DWARF4
    style DW_AT_highpc.  */

/*! @brief Returns the DW_AT_hipc  address value

    Calculating the high pc involves several elements
    which we don't describe here. See the DWARF5 standard.
    This is accessing the DW_AT_high_pc attribute.
    @param dw_die
    The DIE of interest.
    @param dw_return_addr
    On success returns the  high-pc address for this DIE.
    @param dw_return_form
    On success returns the actual FORM for this attribute.
    @param dw_return_class
    On success returns the FORM CLASS for this attribute.
    @param dw_error
    The usual error detail return pointer.
    @return
    Returns DW_DLV_OK etc.
*/
DW_API int dwarf_highpc_b(Dwarf_Die dw_die,
    Dwarf_Addr  *           dw_return_addr,
    Dwarf_Half  *           dw_return_form,
    enum Dwarf_Form_Class * dw_return_class,
    Dwarf_Error *           dw_error);

/*  If 'die' contains the DW_AT_type attribute,
    it returns the offset referenced by the attribute. */
DW_API int dwarf_dietype_offset(Dwarf_Die /*die*/,
    Dwarf_Off   * /*return_off*/,
    Dwarf_Error * /*error*/);

DW_API int dwarf_bytesize(Dwarf_Die /*die*/,
    Dwarf_Unsigned * /*returned_size*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_bitsize(Dwarf_Die /*die*/,
    Dwarf_Unsigned * /*returned_size*/,
    Dwarf_Error*     /*error*/);

/*  If the attribute is DW_AT_data_bit_offset
    (DWARF4, DWARF5) the returned bit offset
    has one meaning.
    If the attribute is DW_AT_bit_offset
    (DWARF2, DWARF3) the meaning is quite different. */
DW_API int dwarf_bitoffset(Dwarf_Die /*die*/,
    Dwarf_Half     * /*attribute*/,
    Dwarf_Unsigned * /*returned_offset*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_srclang(Dwarf_Die /*die*/,
    Dwarf_Unsigned * /*returned_lang*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_arrayorder(Dwarf_Die /*die*/,
    Dwarf_Unsigned * /*returned_order*/,
    Dwarf_Error*     /*error*/);

/*! @} */
/*! @defgroup attrform Attribute and Attribute-Form Details
    @{
*/
/*! @brief Gets the full list of attributes

    @param dw_die
    The DIE from which to pull attributes.
    @param dw_attrbuf
    The pointer is set to point to an array
    of Dwarf_Attribute (pointers to attribute data).
    This array must eventually be deallocated.
    @param dw_attrcount
    The number of entries in the array of pointers.
    There is no null-pointer to terminate the list,
    use this count.
    @param dw_error
    A place to return error details.
    @return
    If it returns DW_DLV_ERROR and dw_error is non-null
    it creates an Dwarf_Error and places it in this argument.
    Usually returns DW_DLV_OK.
    @see example1
*/
DW_API int dwarf_attrlist(Dwarf_Die dw_die,
    Dwarf_Attribute** dw_attrbuf,
    Dwarf_Signed   * dw_attrcount,
    Dwarf_Error*     dw_error);

DW_API int dwarf_hasform(Dwarf_Attribute /*attr*/,
    Dwarf_Half       /*form*/,
    Dwarf_Bool *     /*returned_bool*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_whatform(Dwarf_Attribute /*attr*/,
    Dwarf_Half *     /*returned_final_form*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_whatform_direct(Dwarf_Attribute /*attr*/,
    Dwarf_Half *     /*returned_initial_form*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_whatattr(Dwarf_Attribute /*attr*/,
    Dwarf_Half *     /*returned_attr_num*/,
    Dwarf_Error*     /*error*/);

/*
    The following are concerned with the Primary Interface: getting
    the actual data values. One function per 'kind' of FORM.  */
/*  dwarf_formref returns, thru return_offset, a CU-relative offset
    ( in .debug_info or .debug_types)
    and does not allow DW_FORM_ref_addr*/
DW_API int dwarf_formref(Dwarf_Attribute /*attr*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Bool     * /*is_info*/,
    Dwarf_Error*     /*error*/);
/*  dwarf_global_formref returns, thru return_offset,
    a debug_info-relative offset and does allow all reference forms
    if the output is not a DIE offset then call
    dwarf_global_formref.  If it is a DIE offset then
    call dwarf_global_formref_b instead so you know whether it is
    in .debug_types or .debug_info.
*/
DW_API int dwarf_global_formref(Dwarf_Attribute /*attr*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_global_formref_b(Dwarf_Attribute /*attr*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Bool   *   /*offset_is_info*/,
    Dwarf_Error*     /*error*/);

/*  dwarf_formsig8 returns in the caller-provided 8 byte area
    the 8 bytes of a DW_FORM_ref_sig8.  Not a string.  */
DW_API int dwarf_formsig8(Dwarf_Attribute /*attr*/,
    Dwarf_Sig8 * /*returned sig bytes*/,
    Dwarf_Error*     /*error*/);
/*  dwarf_formsig8_const returns in the caller-provided 8 byte area
    the 8 bytes of a form const (DW_FORM_data8).  Not a string.  */
DW_API int dwarf_formsig8_const(Dwarf_Attribute /*attr*/,
    Dwarf_Sig8 * /*returned sig bytes*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_formaddr(Dwarf_Attribute /*attr*/,
    Dwarf_Addr   *   /*returned_addr*/,
    Dwarf_Error*     /*error*/);

/*  Part of DebugFission.  So a consumer can get the index when
    the object with the actual .debug_addr section is
    elsewhere. And so a print application can
    print the index.  New May 2014*/
DW_API int dwarf_get_debug_addr_index(Dwarf_Attribute /*attr*/,
    Dwarf_Unsigned * /*return_index*/,
    Dwarf_Error * /*error*/);

DW_API int dwarf_formflag(Dwarf_Attribute /*attr*/,
    Dwarf_Bool *     /*returned_bool*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_formdata16(Dwarf_Attribute /*attr*/,
    Dwarf_Form_Data16  * /*returned_val*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_formudata(Dwarf_Attribute /*attr*/,
    Dwarf_Unsigned  * /*returned_val*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_formsdata(Dwarf_Attribute     /*attr*/,
    Dwarf_Signed  *  /*returned_val*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_formblock(Dwarf_Attribute /*attr*/,
    Dwarf_Block    ** /*returned_block*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_formstring(Dwarf_Attribute /*attr*/,
    char   **        /*returned_string*/,
    Dwarf_Error*     /*error*/);

/*  DebugFission.  So a DWARF print application can
    get the string index (DW_FORM_strx) and print it.
    A convenience function.
    New May 2014. */
DW_API int dwarf_get_debug_str_index(Dwarf_Attribute /*attr*/,
    Dwarf_Unsigned * /*return_index*/,
    Dwarf_Error * /*error*/);

DW_API int dwarf_formexprloc(Dwarf_Attribute /*attr*/,
    Dwarf_Unsigned * /*return_exprlen*/,
    Dwarf_Ptr  * /*block_ptr*/,
    Dwarf_Error * /*error*/);

DW_API enum Dwarf_Form_Class dwarf_get_form_class(
    Dwarf_Half /* dwversion */,
    Dwarf_Half /* attrnum */,
    Dwarf_Half /*offset_size */,
    Dwarf_Half /*form*/);

DW_API int dwarf_attr_offset(Dwarf_Die /*die*/,
    Dwarf_Attribute /*attr of above die*/,
    Dwarf_Off     * /*returns offset thru this ptr */,
    Dwarf_Error   * /*error*/);

/*  On success returns DW_DLV_OK
    and creates an array of Dwarf_Signed values
    from the block of sleb numbers.
    No ugly cast needed to know if
    dwarf_uncompress_integer_block_a() succeeds or not. */
DW_API int dwarf_uncompress_integer_block_a(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned     /*input_length_in_bytes*/,
    void             * /*input_block*/,
    Dwarf_Unsigned   * /*value_count*/,
    Dwarf_Signed    ** /*value_array*/,
    Dwarf_Error      * /*error*/);

/*  Call this passing in return value from
    dwarf_uncompress_integer_block_a()
    to free the space the decompression allocated. */
DW_API void dwarf_dealloc_uncompressed_block(Dwarf_Debug, void *);

/* Convert local offset into global offset */
DW_API int dwarf_convert_to_global_offset(Dwarf_Attribute /*attr*/,
    Dwarf_Off        /*offset*/,
    Dwarf_Off*       /*ret_offset*/,
    Dwarf_Error*     /*error*/);
DW_API void dwarf_dealloc_attribute(Dwarf_Attribute /*attr*/);

/*! @} */

/*! @defgroup discriminant Decoding a Discriminant List

    Given a block containing a discriminant list
    the following functions enable decoding the block.

    @{
*/
DW_API int dwarf_discr_list(Dwarf_Debug /*dbg*/,
    Dwarf_Small    * /*blockpointer*/,
    Dwarf_Unsigned   /*blocklen*/,
    Dwarf_Dsc_Head * /*dsc_head_out*/,
    Dwarf_Unsigned * /*dsc_array_length_out*/,
    Dwarf_Error    * /*error*/);

/*  Access to a DW_AT_discr_list
    unsigned entry. Callers must know which is the appropriate
    one of the following two interfaces, though both
    will work. */
DW_API int dwarf_discr_entry_u(Dwarf_Dsc_Head /* dsc */,
    Dwarf_Unsigned   /*entrynum*/,
    Dwarf_Half     * /*out_type*/,
    Dwarf_Unsigned * /*out_discr_low*/,
    Dwarf_Unsigned * /*out_discr_high*/,
    Dwarf_Error    * /*error*/);

/*  Access to a DW_AT_discr_list
    signed entry. */
DW_API int dwarf_discr_entry_s(Dwarf_Dsc_Head /* dsc */,
    Dwarf_Unsigned   /*entrynum*/,
    Dwarf_Half     * /*out_type*/,
    Dwarf_Signed   * /*out_discr_low*/,
    Dwarf_Signed   * /*out_discr_high*/,
    Dwarf_Error    * /*error*/);

/*! @} */

/*! @defgroup linetable Line Table For a CU
    @{
*/

DW_API int dwarf_srclines_b(Dwarf_Die /*die*/,
    Dwarf_Unsigned     * /*version_out*/,
    Dwarf_Small        * /*table_count*/,
    Dwarf_Line_Context * /*linecontext*/,
    Dwarf_Error        * /*error*/);

/*  Functions passing in a Dwarf_Line_Context  are only
    available if dwarf_srclines_b() was used to access
    line table information.  */
/*  New October 2015.  Returns line details.
    Works for DWARF2,3,4,5.  If linecount
    returned is zero this is a line table with no lines.*/
DW_API int dwarf_srclines_from_linecontext(Dwarf_Line_Context,
    Dwarf_Line  **   /*linebuf*/,
    Dwarf_Signed *   /*linecount*/,
    Dwarf_Error  *   /* error*/);

/*  Returns line details.
    Works for DWARF2,3,4,5 and for experimental
    two-level line tables. A single level table will
    have *linebuf_actuals and *linecount_actuals set
    to 0. */
/*  If we have two-level line tables, this will return the
    logicals table in linebuf and the actuals table in
    linebuf_actuals. For old-style (one-level) tables, it
    will return the single table through linebuf, and the
    value returned through linecount_actuals will be 0.
    The actual version number is returned through version.
    For two-level line tables, the version returned will
    be 0xf006. This interface can return data from two-level
    line tables, which are experimental.
    Most users will not wish to use
    dwarf_srclines_two_level_from_linecontext */
DW_API int dwarf_srclines_two_level_from_linecontext(
    Dwarf_Line_Context,
    Dwarf_Line  **   /*linebuf */,
    Dwarf_Signed *   /*linecount*/,
    Dwarf_Line  **   /*linebuf_actuals*/,
    Dwarf_Signed *   /*linecount_actuals*/,
    Dwarf_Error  *   /* error*/);

/*  dwarf_srclines_dealloc_b(), created October 2015, is the
    appropriate method for deallocating everything
    and dwarf_srclines_from_linecontext(),
    dwarf_srclines_twolevel_from_linecontext(),
    and dwarf_srclines_b()  allocate.  */
DW_API void dwarf_srclines_dealloc_b(Dwarf_Line_Context /*context*/);

/*  New October 2015.
    The offset is in the relevent .debug_line or .debug_line.dwo
    section (and in a split dwarf package file includes)
    the base line table offset). */
DW_API int dwarf_srclines_table_offset(Dwarf_Line_Context /*context*/,
    Dwarf_Unsigned * /*offset*/,
    Dwarf_Error  * /* error*/);

/*  New October 2015. */
/*  Compilation Directory name for the current CU.
    section (and in a split dwarf package file includes)
    the base line table offset).  Do not free() the string,
    it is in a dwarf section. */
DW_API int dwarf_srclines_comp_dir(Dwarf_Line_Context /*context*/,
    const char ** /*compilation_directory*/,
    Dwarf_Error  *  /*error*/);

/*  New October 2015.  Part of the two-level line table extension. */
/*  Count is the real count of suprogram array entries. */
DW_API int dwarf_srclines_subprog_count(Dwarf_Line_Context /*contxt*/,
    Dwarf_Signed * /*count*/,
    Dwarf_Error  * /*error*/);

/*  New October 2015. */
/*  Index starts with 1, last is 'count' */
DW_API int dwarf_srclines_subprog_data(Dwarf_Line_Context /*context*/,
    Dwarf_Signed     /*index*/,
    const char **    /*name*/,
    Dwarf_Unsigned * /*decl_file*/,
    Dwarf_Unsigned * /*decl_line*/,
    Dwarf_Error   *  /*error*/);

/*  New March 2018. */
/*  Count is the real count of files array entries.
    Since DWARF 2,3,4 are zero origin indexes and
    DWARF5 and later are one origin, this function
    replaces dwarf_srclines_files_count(). */
DW_API int dwarf_srclines_files_indexes(Dwarf_Line_Context /*contxt*/,
    Dwarf_Signed  *  /*baseindex*/,
    Dwarf_Signed  *  /*count*/,
    Dwarf_Signed  *  /*endindex*/,
    Dwarf_Error   *  /*error*/);

/*  New March 2018.
    Has the md5ptr field so cases where DW_LNCT_MD5
    is present can return pointer to the MD5 value.
    With DWARF 5 index starts with 0.
    See dwarf_srclines_files_indexes() which makes
    indexing through the files easy. */
DW_API int dwarf_srclines_files_data_b(Dwarf_Line_Context /*context*/,
    Dwarf_Signed     /*index_in*/,
    const char **    /*name*/,
    Dwarf_Unsigned * /*directory_index*/,
    Dwarf_Unsigned * /*last_mod_time*/,
    Dwarf_Unsigned * /*file_length*/,
    Dwarf_Form_Data16 ** /*md5ptr*/,
    Dwarf_Error    * /*error*/);

/*  New October 2015. */
/*  Count is the real count of include array entries. */
DW_API int dwarf_srclines_include_dir_count(Dwarf_Line_Context
    /*line_context*/,
    Dwarf_Signed *  /*count*/,
    Dwarf_Error  * /* error*/);

/*  New October 2015. */
/*  Index starts with 1, last is 'count' */
DW_API int dwarf_srclines_include_dir_data( Dwarf_Line_Context
    /*line_context*/,
    Dwarf_Signed    /*index*/,
    const char **   /*name*/,
    Dwarf_Error   * /* error*/);

/*  New October 2015. */
/*  The DWARF version number of this compile-unit
    in the .debug_lines section and the number of
    actual tables:0 (header with no lines),
    1 (standard table), or 2 (experimental). */
DW_API int dwarf_srclines_version(Dwarf_Line_Context /*line_context*/,
    Dwarf_Unsigned * /*version*/,
    Dwarf_Small    * /*table_count*/,
    Dwarf_Error    * /*error*/);

DW_API int dwarf_get_line_section_name(Dwarf_Debug /*dbg*/,
    const char ** /*section_name_out*/,
    Dwarf_Error * /*error*/);
DW_API int dwarf_get_line_section_name_from_die(Dwarf_Die /*die*/,
    const char ** /*section_name_out*/,
    Dwarf_Error * /*error*/);

/*  While 'filecount' is signed, the value
    returned through the pointer is never negative.
    Original libdwarf from 199x.  */
DW_API int dwarf_srcfiles(Dwarf_Die /*die*/,
    char***          /*srcfiles*/,
    Dwarf_Signed *   /*filecount*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_linebeginstatement(Dwarf_Line /*line*/,
    Dwarf_Bool  *    /*returned_bool*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_lineendsequence(Dwarf_Line /*line*/,
    Dwarf_Bool  *    /*returned_bool*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_lineno(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*returned_lineno*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_line_srcfileno(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*ret_fileno*/,
    Dwarf_Error *    /*error*/);

/* Is the line address from DW_LNS_set_address? */
DW_API int dwarf_line_is_addr_set(Dwarf_Line /*line*/,
    Dwarf_Bool *     /*is_addr_set*/,
    Dwarf_Error *    /*error*/);

DW_API int dwarf_lineaddr(Dwarf_Line /*line*/,
    Dwarf_Addr *     /*returned_addr*/,
    Dwarf_Error*     /*error*/);

/*  dwarf_lineoff_b correctly returns an unsigned column number
    through the pointer returned_lineoffset.
    dwarf_lineoff_b() is new in December 2011.  */
DW_API int dwarf_lineoff_b(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*returned_lineoffset*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_linesrc(Dwarf_Line /*line*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_lineblock(Dwarf_Line /*line*/,
    Dwarf_Bool  *    /*returned_bool*/,
    Dwarf_Error*     /*error*/);

/*  We gather these into one call as it's likely one
    will want all or none of them.  */
DW_API int dwarf_prologue_end_etc(Dwarf_Line /* line */,
    Dwarf_Bool  *    /*prologue_end*/,
    Dwarf_Bool  *    /*eplogue_begin*/,
    Dwarf_Unsigned * /* isa */,
    Dwarf_Unsigned * /* discriminator */,
    Dwarf_Error *    /*error*/);
/* End line table operations */

/*  Two-level line tables:
    When reading from an actuals table, dwarf_line_logical()
    returns the logical row number for the line. */
DW_API int dwarf_linelogical(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*returned_logical*/,
    Dwarf_Error*     /*error*/);

/*  Two-level line tables:
    When reading from a logicals table, dwarf_linecontext()
    returns the logical row number corresponding the the
    calling context for an inlined call. */
DW_API int dwarf_linecontext(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*returned_context*/,
    Dwarf_Error*     /*error*/);

/*  Two-level line tables:
    When reading from a logicals table, dwarf_line_subprogno()
    returns the index in the subprograms table of the inlined
    subprogram. */
DW_API int dwarf_line_subprogno(Dwarf_Line /*line*/,
    Dwarf_Unsigned * /*ret_subprogno*/,
    Dwarf_Error *    /*error*/);

/*  Two-level line tables:
    When reading from a logicals table, dwarf_line_subprog()
    returns the name of the inlined subprogram, its declaration
    filename, and its declaration line number, if available. */
DW_API int dwarf_line_subprog(Dwarf_Line /*line*/,
    char   **        /*returned_subprog_name*/,
    char   **        /*returned_filename*/,
    Dwarf_Unsigned * /*returned_lineno*/,
    Dwarf_Error *    /*error*/);
/*  dwarf_check_lineheader lets dwarfdump get detailed messages
    about some compiler errors we detect.
    We return the count of detected errors through the
    pointer.  */
DW_API int dwarf_check_lineheader_b(Dwarf_Die /*cu_die*/,
    int         * /*errcount_out*/,
    Dwarf_Error * /*error*/);

/*  dwarf_print_lines() prints line information
    for a CU in great detail.
    Does not use printf.
    Instead it calls back to the application using a function
    pointer once per line-to-print.  The lines passed back
    already have any needed
    newlines.

    Failing to call the dwarf_register_printf_callback()
    function will prevent the lines from being passed back
    but such omission is not an error.

    The return value is the previous set of callback values.
*/
DW_API int dwarf_print_lines(Dwarf_Die /*die*/,Dwarf_Error * /*er*/,
    int * /*error_count_out */);

/*  If called with a NULL newvalues pointer, it simply returns
    the current set of values for this Dwarf_Debug. */
DW_API struct  Dwarf_Printf_Callback_Info_s
    dwarf_register_printf_callback(Dwarf_Debug /*dbg*/,
    struct  Dwarf_Printf_Callback_Info_s * /*newvalues*/);

/*! @} */
/*! @defgroup ranges Ranges .debug_ranges DWARF3, DWARF4
    @{
*/
/*  Adds return of the final offset to accommodate
    DWARF4 GNU split-dwarf. Other than for split-dwarf
    the realoffset will be set by the function
    to be the same as rangesoffset.
*/
DW_API int dwarf_get_ranges_b(Dwarf_Debug /*dbg*/,
    Dwarf_Off       /*rangesoffset*/,
    Dwarf_Die       /*diepointer */,
    Dwarf_Off *     /*realoffset */,
    Dwarf_Ranges ** /*rangesbuf*/,
    Dwarf_Signed *  /*listlen*/,
    Dwarf_Unsigned * /*bytecount*/,
    Dwarf_Error *   /*error*/);
DW_API void dwarf_dealloc_ranges(Dwarf_Debug /*dbg*/,
    Dwarf_Ranges * /*rangesbuf*/,
    Dwarf_Signed /*rangecount*/);
/*! @} */
/*! @defgroup rnglists Rnglists .debug_rnglists DWARF5
    @{
*/

/*  For DWARF5 DW_AT_ranges:
    DW_FORM_sec_offset DW_FORM_rnglistx */
DW_API int dwarf_rnglists_get_rle_head(Dwarf_Attribute /*attr*/,
    Dwarf_Half            /*theform*/,
    Dwarf_Unsigned        /*index_or_offset_value*/,
    Dwarf_Rnglists_Head * /*head_out*/,
    Dwarf_Unsigned *      /*count_of_entries_in_head*/,
    Dwarf_Unsigned *      /*global_offset_of_rle_set*/,
    Dwarf_Error    *      /*error*/);

/*  Get the rnglist entries details */
DW_API int dwarf_get_rnglists_entry_fields_a(Dwarf_Rnglists_Head,
    Dwarf_Unsigned   /*entrynum*/,
    unsigned int   * /*entrylen*/,
    unsigned int   * /*rle_value_out*/,
    Dwarf_Unsigned * /*raw1*/,
    Dwarf_Unsigned * /*raw2*/,
    Dwarf_Bool     * /*debug_addr_unavailable*/,
    Dwarf_Unsigned * /*cooked1*/,
    Dwarf_Unsigned * /*cooked2*/,
    Dwarf_Error *    /*err*/);

DW_API void dwarf_dealloc_rnglists_head(Dwarf_Rnglists_Head);

/*  Loads all the rnglists headers and
    returns DW_DLV_NO_ENTRY if the section
    is missing or empty.
    Intended to be done quite early and
    it is automatically
    done if .debug_info is loaded.
    Doing it more than once is never necessary
    or harmful. There is no deallocation call
    made visible, deallocation happens
    when dwarf_finish() is called.
    With DW_DLV_OK it returns the number of
    rnglists headers in the section through
    rnglists_count. */
DW_API int dwarf_load_rnglists(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned * /*rnglists_count*/,
    Dwarf_Error * /*err*/);

/*  Retrieve the offset from the context-index'th
    rangelists context  and the offsetentry_index
    element of the array of offsets.
    If an index is too large to be correct
    this returns DW_DLV_NO_ENTRY.
    If all is correct it returns DW_DLV_OK and
    sets *offset_value_out to the offset of
    the range list from the base of the offset
    array, and *global_offset_value_out is set
    to the .debug_rnglists section offset of
    the range list. */
DW_API int dwarf_get_rnglist_offset_index_value(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned   /*context_index*/,
    Dwarf_Unsigned   /*offsetentry_index*/,
    Dwarf_Unsigned * /*offset_value_out*/,
    Dwarf_Unsigned * /*global_offset_value_out*/,
    Dwarf_Error * /*error*/);

/*  Used by dwarfdump to print basic data from the
    data generated to look at a specific rangelist
    as returned by  dwarf_rnglists_index_get_rle_head()
    or dwarf_rnglists_offset_get_rle_head. */
DW_API int dwarf_get_rnglist_head_basics(Dwarf_Rnglists_Head /*head*/,
    Dwarf_Unsigned * /*rle_count*/,
    Dwarf_Unsigned * /*rnglists_version*/,
    Dwarf_Unsigned * /*rnglists_index_returned*/,
    Dwarf_Unsigned * /*bytes_total_in_rle*/,
    Dwarf_Half     * /*offset_size*/,
    Dwarf_Half     * /*address_size*/,
    Dwarf_Half     * /*segment_selector_size*/,
    Dwarf_Unsigned * /*overall offset_of_this_context*/,
    Dwarf_Unsigned * /*total_length of this context*/,
    Dwarf_Unsigned * /*offset_table_offset*/,
    Dwarf_Unsigned * /*offset_table_entrycount*/,
    Dwarf_Bool     * /*rnglists_base_present*/,
    Dwarf_Unsigned * /*rnglists_base*/,
    Dwarf_Bool     * /*rnglists_base_address_present*/,
    Dwarf_Unsigned * /*rnglists_base_address*/,
    Dwarf_Bool     * /*rnglists_debug_addr_base_present*/,
    Dwarf_Unsigned * /*rnglists_debug_addr_base*/,
    Dwarf_Error    * /*error*/);

/*  Enables printing of details about the Range List Table
    Headers, one header per call. Index starting at 0.
    Returns DW_DLV_NO_ENTRY if index is too high for the table.
    A .debug_rnglists section may contain any number
    of Range List Table Headers with their details.  */
DW_API int dwarf_get_rnglist_context_basics(Dwarf_Debug  /*dbg*/,
    Dwarf_Unsigned  /*index*/,
    Dwarf_Unsigned * /*header_offset*/,
    Dwarf_Small  *   /*offset_size*/,
    Dwarf_Small  *   /*extension_size*/,
    unsigned int *   /*version*/, /* 5 */
    Dwarf_Small  *   /*address_size*/,
    Dwarf_Small  *   /*segment_selector_size*/,
    Dwarf_Unsigned * /*offset_entry_count*/,
    Dwarf_Unsigned * /*offset_of_offset_array*/,
    Dwarf_Unsigned * /*offset_of_first_rangeentry*/,
    Dwarf_Unsigned * /*offset_past_last_rangeentry*/,
    Dwarf_Error *    /*err*/);

/*  If no error, returns DW_DLV_OK and sets
    the entry length,kind, and operands through
    the pointers. If any missing operands assign
    zero back through tye operand pointers. */
DW_API int dwarf_get_rnglist_rle(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned /*contextnumber*/,
    Dwarf_Unsigned /*entry_offset*/,
    Dwarf_Unsigned /*endoffset*/,
    unsigned int   * /*entrylen*/,
    unsigned int   * /*entry_kind*/,
    Dwarf_Unsigned * /*entry_operand1*/,
    Dwarf_Unsigned * /*entry_operand2*/,
    Dwarf_Error * /*err*/);
/*! @} */
/*! @defgroup locations Data Locations DWARF2-DWARF5
    @{
*/
/*  BEGIN: loclist_c interfaces
    DWARF 2,3,4
    This works for any attribute that identifies
    a loclist or a locexpr. When the attribute is a locexpr
    a single loclist (created by libdwarf)
    is attached to loclist_head. */
DW_API int dwarf_get_loclist_c(Dwarf_Attribute /*attr*/,
    Dwarf_Loc_Head_c * /*loclist_head*/,
    Dwarf_Unsigned   * /*locCount*/,
    Dwarf_Error      * /*error*/);

#define DW_LKIND_expression   0 /* DWARF2,3,4*/
#define DW_LKIND_loclist      1 /* DWARF 2,3,4 */
#define DW_LKIND_GNU_exp_list 2 /* GNU DWARF4 .dwo extension */
#define DW_LKIND_loclists     5 /* DWARF5 loclists */
#define DW_LKIND_unknown     99

/* DWARF2 kind is 2. DWARF3/4 kind is 3, DWARF5 kind is 5 */
DW_API int dwarf_get_loclist_head_kind(Dwarf_Loc_Head_c /*ll_header*/,
    unsigned int  * /*lkind*/,
    Dwarf_Error   * /*error*/);

/*  Cooked value means the values from the location
    description (raw values) after base values applied.
    debug_addr_unavailable non-zero means the record from a
    skeleton unit could not be accessed from
    the .dwo section or dwp object so the
    cooked values could not be calculated. */
DW_API int dwarf_get_locdesc_entry_d(Dwarf_Loc_Head_c /*head*/,
    Dwarf_Unsigned    /*index*/,
    /* identifies type of locdesc entry*/
    Dwarf_Small    *  /*lle_value_out*/,
    Dwarf_Unsigned *  /*rawlowpc*/,
    Dwarf_Unsigned *  /*rawhipc*/,
    Dwarf_Bool     *  /*debug_addr_unavailable*/,
    Dwarf_Addr     *  /*lowpc_out (cooked value) */,
    Dwarf_Addr     *  /*hipc_out (cooked value) */,
    Dwarf_Unsigned *  /*loclist_count_out*/,
    Dwarf_Locdesc_c * /*locentry_out*/,
    Dwarf_Small    *  /*loclist_source_out*/, /* 0,1, or 2 */
    Dwarf_Unsigned *  /*expression_offset_out*/,
    Dwarf_Unsigned *  /*locdesc_offset_out*/,
    Dwarf_Error    *  /*error*/);

/* New June 2020 for DWARF5 (and all earlier). */
DW_API int dwarf_get_location_op_value_d(Dwarf_Locdesc_c /*locdesc*/,
    Dwarf_Unsigned   /*index*/,
    Dwarf_Small    * /*operator_out*/,
    Dwarf_Unsigned * /*operand1*/,
    Dwarf_Unsigned * /*operand2*/,
    Dwarf_Unsigned * /*operand3*/,
    Dwarf_Unsigned * /*rawop1*/,
    Dwarf_Unsigned * /*rawop2*/,
    Dwarf_Unsigned * /*rawop3*/,
    Dwarf_Unsigned * /*offset_for_branch*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_loclist_from_expr_c(Dwarf_Debug /*dbg*/,
    Dwarf_Ptr      /*expression_in*/,
    Dwarf_Unsigned /*expression_length*/,
    Dwarf_Half     /*address_size*/,
    Dwarf_Half     /*offset_size*/,
    Dwarf_Small    /*dwarf_version*/,
    Dwarf_Loc_Head_c* /*loc_head*/,
    Dwarf_Unsigned  * /*listlen*/,
    Dwarf_Error     * /*error*/);

/*! @brief frees all memory allocated for Dwarf_Loc_Head_c
    @param dw_head
    A head pointer
*/
DW_API void dwarf_loc_head_c_dealloc(Dwarf_Loc_Head_c dw_head);

/*  These interfaces allow reading the .debug_loclists
    section. Independently of DIEs.
    Normal use of .debug_loclists uses
    dwarf_get_loclist_c() to open access to any kind of location
    or loclist and uses dwarf_loc_head_c_dealloc() to
    deallocate that memory once one is finished with
    that data. So for most purposes you do not need
    to use these functions
    See dwarf_get_loclist_c() to open a Dwarf_Loc_Head_c
    on any type of location list or expression. */

/*  Loads all the loclists headers and
    returns DW_DLV_NO_ENTRY if the section
    is missing or empty.
    Intended to be done quite early and
    it is automatically
    done if .debug_info is loaded.
    Doing it more than once is never necessary
    or harmful. There is no deallocation call
    made visible, deallocation happens
    when dwarf_finish() is called.
    With DW_DLV_OK it returns the number of
    loclists headers in the section through
    loclists_count. */
DW_API int dwarf_load_loclists(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned * /*loclists_count*/,
    Dwarf_Error * /*err*/);

/*  Retrieve the offset from the context-index'th
    loclists context  and the offsetentry_index
    element of the array of offsets.
    If an index is too large to be correct
    this returns DW_DLV_NO_ENTRY.
    If all is correct it returns DW_DLV_OK and
    sets *offset_value_out to the offset of
    the range list from the base of the offset
    array, and *global_offset_value_out is set
    to the .debug_loclists section offset of
    the range list. */
DW_API int dwarf_get_loclist_offset_index_value(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned   /*context_index*/,
    Dwarf_Unsigned   /*offsetentry_index*/,
    Dwarf_Unsigned * /*offset_value_out*/,
    Dwarf_Unsigned * /*global_offset_value_out*/,
    Dwarf_Error * /*error*/);

/*  Used by dwarfdump to print basic data from the
    data generated to look at a specific loclist
    as returned by  dwarf_loclists_index_get_lle_head()
    or dwarf_loclists_offset_get_lle_head. */
DW_API int dwarf_get_loclist_head_basics(Dwarf_Loc_Head_c /*head*/,
    Dwarf_Small    * /*lkind*/,
    Dwarf_Unsigned * /*lle_count*/,
    Dwarf_Unsigned * /*loclists_version*/,
    Dwarf_Unsigned * /*loclists_index_returned*/,
    Dwarf_Unsigned * /*bytes_total_in_rle*/,
    Dwarf_Half     * /*offset_size*/,
    Dwarf_Half     * /*address_size*/,
    Dwarf_Half     * /*segment_selector_size*/,
    Dwarf_Unsigned * /*overall offset_of_this_context*/,
    Dwarf_Unsigned * /*total_length of this context*/,
    Dwarf_Unsigned * /*offset_table_offset*/,
    Dwarf_Unsigned * /*offset_table_entrycount*/,
    Dwarf_Bool     * /*loclists_base_present*/,
    Dwarf_Unsigned * /*loclists_base*/,
    Dwarf_Bool     * /*loclists_base_address_present*/,
    Dwarf_Unsigned * /*loclists_base_address*/,
    Dwarf_Bool     * /*loclists_debug_addr_base_present*/,
    Dwarf_Unsigned * /*loclists_debug_addr_base*/,
    Dwarf_Unsigned * /*offset_this_lle_area*/,
    Dwarf_Error    * /*error*/);

DW_API int dwarf_get_loclist_context_basics(Dwarf_Debug  /*dbg*/,
    Dwarf_Unsigned  /*index*/,
    Dwarf_Unsigned * /*header_offset*/,
    Dwarf_Small  *   /*offset_size*/,
    Dwarf_Small  *   /*extension_size*/,
    unsigned int *   /*version*/, /* 5 */
    Dwarf_Small  *   /*address_size*/,
    Dwarf_Small  *   /*segment_selector_size*/,
    Dwarf_Unsigned * /*offset_entry_count*/,
    Dwarf_Unsigned * /*offset_of_offset_array*/,
    Dwarf_Unsigned * /*offset_of_first_locentry*/,
    Dwarf_Unsigned * /*offset_past_last_locentry*/,
    Dwarf_Error *    /*err*/);

DW_API int dwarf_get_loclist_lle( Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned   /*contextnumber*/,
    Dwarf_Unsigned   /*entry_offset*/,
    Dwarf_Unsigned   /*endoffset*/,
    unsigned int *   /*entrylen*/,
    unsigned int *   /*entry_kind*/,
    Dwarf_Unsigned * /*entry_operand1*/,
    Dwarf_Unsigned * /*entry_operand2*/,
    Dwarf_Unsigned * /*expr_ops_blocksize*/,
    Dwarf_Unsigned * /*expr_ops_offset*/,
    Dwarf_Small   ** /*expr_opsdata*/,
    Dwarf_Error * /*err*/);
/*! @} */

/*! @defgroup macro Macro .debug_macro DWARF5 data access
    @{
*/

/*  BEGIN: DWARF5 .debug_macro  interfaces
    NEW November 2015.  */
DW_API int dwarf_get_macro_context(Dwarf_Die /*die*/,
    Dwarf_Unsigned      * /*version_out*/,
    Dwarf_Macro_Context * /*macro_context*/,
    Dwarf_Unsigned      * /*macro_unit_offset_out*/,
    Dwarf_Unsigned      * /*macro_ops_count_out*/,
    Dwarf_Unsigned      * /*macro_ops_data_length_out*/,
    Dwarf_Error         * /*error*/);

/*  Just like dwarf_get_macro_context, but instead of using
    DW_AT_macros or DW_AT_GNU_macros to get the offset we just
    take the offset given. */
DW_API int dwarf_get_macro_context_by_offset(Dwarf_Die /*die*/,
    Dwarf_Unsigned        /*offset*/,
    Dwarf_Unsigned      * /*version_out*/,
    Dwarf_Macro_Context * /*macro_context*/,
    Dwarf_Unsigned      * /*macro_ops_count_out*/,
    Dwarf_Unsigned      * /*macro_ops_data_length*/,
    Dwarf_Error         * /*error*/);

/*  New December 2020. Sometimes its necessary to know
    a context total length including macro 5 header */
DW_API int dwarf_macro_context_total_length(Dwarf_Macro_Context
    /* head*/,
    Dwarf_Unsigned * /*mac_total_len*/,
    Dwarf_Error    * /*error*/);

DW_API void dwarf_dealloc_macro_context(Dwarf_Macro_Context /*mc*/);
DW_API int dwarf_get_macro_section_name(Dwarf_Debug /*dbg*/,
    const char ** /*sec_name_out*/,
    Dwarf_Error * /*err*/);

DW_API int dwarf_macro_context_head(Dwarf_Macro_Context /*head*/,
    Dwarf_Half     * /*version*/,
    Dwarf_Unsigned * /*mac_offset*/,
    Dwarf_Unsigned * /*mac_len*/,
    Dwarf_Unsigned * /*mac_header_len*/,
    unsigned int   * /*flags*/,
    Dwarf_Bool     * /*has_line_offset*/,
    Dwarf_Unsigned * /*line_offset*/,
    Dwarf_Bool     * /*has_offset_size_64*/,
    Dwarf_Bool     * /*has_operands_table*/,
    Dwarf_Half     * /*opcode_count*/,
    Dwarf_Error    * /*error*/);

/*  Returns data from the operands table
    in the macro unit header. The last op has
    0 as opcode_number,operand_count and operand_array */
DW_API int dwarf_macro_operands_table(Dwarf_Macro_Context /*head*/,
    Dwarf_Half    /*index*/, /* 0 to opcode_count -1 */
    Dwarf_Half  * /*opcode_number*/,
    Dwarf_Half  * /*operand_count*/,
    const Dwarf_Small ** /*operand_array*/,
    Dwarf_Error * /*error*/);

/*  Access to the macro operations, 0 to macro_ops_count_out-1
    Where the last of these will have macro_operator 0 (which appears
    in the ops data and means end-of-ops).
    op_start_section_offset is the section offset of
    the macro operator (which is a single unsigned byte,
    and is followed by the macro operand data). */
DW_API int dwarf_get_macro_op(Dwarf_Macro_Context /*macro_context*/,
    Dwarf_Unsigned   /*op_number*/,
    Dwarf_Unsigned * /*op_start_section_offset*/,
    Dwarf_Half     * /*macro_operator*/,
    Dwarf_Half     * /*forms_count*/,
    const Dwarf_Small **  /*formcode_array*/,
    Dwarf_Error    * /*error*/);

DW_API int dwarf_get_macro_defundef(Dwarf_Macro_Context
    /*macro_context*/,
    Dwarf_Unsigned   /*op_number*/,
    Dwarf_Unsigned * /*line_number*/,
    Dwarf_Unsigned * /*index*/,
    Dwarf_Unsigned * /*offset*/,
    Dwarf_Half     * /*forms_count*/,
    const char    ** /*macro_string*/,
    Dwarf_Error    * /*error*/);
DW_API int dwarf_get_macro_startend_file(Dwarf_Macro_Context
    /*context*/,
    Dwarf_Unsigned   /*op_number*/,
    Dwarf_Unsigned * /*line_number*/,
    Dwarf_Unsigned * /*name_index_to_line_tab*/,
    const char    ** /*src_file_name*/,
    Dwarf_Error    * /*error*/);
DW_API int dwarf_get_macro_import(Dwarf_Macro_Context
    /*macro_context*/,
    Dwarf_Unsigned   /*op_number*/,
    Dwarf_Unsigned * /*target_offset*/,
    Dwarf_Error    * /*error*/);

/*! @} */
/*! @defgroup macinfo Macrinfo .debug_macinfo DWARF2-4 data access
    @{
*/
DW_API char* dwarf_find_macro_value_start(char * /*macro_string*/);

DW_API int dwarf_get_macro_details(Dwarf_Debug /*dbg*/,
    Dwarf_Off              /*macro_offset*/,
    Dwarf_Unsigned         /*maximum_count*/,
    Dwarf_Signed         * /*entry_count*/,
    Dwarf_Macro_Details ** /*details*/,
    Dwarf_Error *          /*err*/);

/*! @} */

/*! @defgroup frame Frame .debug_frame and .eh_frame Access
    @{
*/
/*  Consumer op on  gnu .eh_frame info */
DW_API int dwarf_get_fde_list_eh(Dwarf_Debug      /*dbg*/,
    Dwarf_Cie**      /*cie_data*/,
    Dwarf_Signed*    /*cie_element_count*/,
    Dwarf_Fde**      /*fde_data*/,
    Dwarf_Signed*    /*fde_element_count*/,
    Dwarf_Error*     /*error*/);

/*  consumer operations on frame info: .debug_frame */
DW_API int dwarf_get_fde_list(Dwarf_Debug /*dbg*/,
    Dwarf_Cie**      /*cie_data*/,
    Dwarf_Signed*    /*cie_element_count*/,
    Dwarf_Fde**      /*fde_data*/,
    Dwarf_Signed*    /*fde_element_count*/,
    Dwarf_Error*     /*error*/);

/*  Release storage gotten by dwarf_get_fde_list_eh() or
    dwarf_get_fde_list() */
DW_API void dwarf_dealloc_fde_cie_list(Dwarf_Debug /*dbg*/,
    Dwarf_Cie *  /*cie_data*/,
    Dwarf_Signed /*cie_element_count*/,
    Dwarf_Fde *  /*fde_data*/,
    Dwarf_Signed /*fde_element_count*/);

DW_API int dwarf_get_fde_range(Dwarf_Fde /*fde*/,
    Dwarf_Addr*      /*low_pc*/,
    Dwarf_Unsigned*  /*func_length*/,
    Dwarf_Small    **/*fde_bytes*/,
    Dwarf_Unsigned*  /*fde_byte_length*/,
    Dwarf_Off*       /*cie_offset*/,
    Dwarf_Signed*    /*cie_index*/,
    Dwarf_Off*       /*fde_offset*/,
    Dwarf_Error*     /*error*/);

/*  Useful for IRIX only:  see dwarf_get_cie_augmentation_data()
    dwarf_get_fde_augmentation_data() for GNU .eh_frame. */
DW_API int dwarf_get_fde_exception_info(Dwarf_Fde /*fde*/,
    Dwarf_Signed*    /* offset_into_exception_tables */,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_get_cie_of_fde(Dwarf_Fde /*fde*/,
    Dwarf_Cie *      /*cie_returned*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_get_cie_info_b(Dwarf_Cie /*cie*/,
    Dwarf_Unsigned * /*bytes_in_cie*/,
    Dwarf_Small*     /*version*/,
    char        **   /*augmenter*/,
    Dwarf_Unsigned*  /*code_alignment_factor*/,
    Dwarf_Signed*    /*data_alignment_factor*/,
    Dwarf_Half*      /*return_address_register_rule*/,
    Dwarf_Small   ** /*initial_instructions*/,
    Dwarf_Unsigned*  /*initial_instructions_length*/,
    Dwarf_Half*      /*offset_size*/,
    Dwarf_Error*     /*error*/);

/* dwarf_get_cie_index new September 2009. */
DW_API int dwarf_get_cie_index(Dwarf_Cie /*cie*/,
    Dwarf_Signed* /*index*/,
    Dwarf_Error* /*error*/ );

/*!
    @brief Used with dwarf_expand_frame_instructions() but
    see that function's comments above.
*/
DW_API int dwarf_get_fde_instr_bytes(Dwarf_Fde /*fde*/,
    Dwarf_Small    ** /*outinstrs*/,
    Dwarf_Unsigned * /*outlen*/,
    Dwarf_Error    * /*error*/);

DW_API int dwarf_get_fde_info_for_all_regs3(Dwarf_Fde /*fde*/,
    Dwarf_Addr       /*pc_requested*/,
    Dwarf_Regtable3* /*reg_table*/,
    Dwarf_Addr*      /*row_pc*/,
    Dwarf_Error*     /*error*/);

/*  See discussion of dw_value_type, libdwarf.h. */
/*  dwarf_get_fde_info_for_reg3_b() is useful on a single column, but
    it is inefficient to iterate across all table_columns using this
    function.  Instead call dwarf_get_fde_info_for_all_regs3()
    and index into the table it fills in. */
DW_API int dwarf_get_fde_info_for_reg3_b(Dwarf_Fde /*fde*/,
    Dwarf_Half       /*table_column*/,
    Dwarf_Addr       /*pc_requested*/,
    Dwarf_Small    * /*value_type*/,
    Dwarf_Unsigned * /*offset_relevant*/,
    Dwarf_Unsigned * /*register*/,
    Dwarf_Unsigned * /*offset */,
    Dwarf_Block  *   /*block_content */,
    Dwarf_Addr   *   /*row_pc_out*/,
    Dwarf_Bool   *   /*has_more_rows */,
    Dwarf_Addr   *   /* subsequent_pc */,
    Dwarf_Error  *   /*error*/);

/*  Use this  to get the cfa.
    New function, June 11, 2016*/
DW_API int dwarf_get_fde_info_for_cfa_reg3_b(Dwarf_Fde /*fde*/,
    Dwarf_Addr       /*pc_requested*/,
    Dwarf_Small  *   /*value_type*/,
    Dwarf_Unsigned *   /*offset_relevant*/,
    Dwarf_Unsigned*    /*register*/,
    Dwarf_Unsigned*    /*offset */,
    Dwarf_Block   *  /*block*/,
    Dwarf_Addr*      /*row_pc_out*/,
    Dwarf_Bool  *    /* has_more_rows */,
    Dwarf_Addr  *    /* subsequent_pc */,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_get_fde_for_die(Dwarf_Debug /*dbg*/,
    Dwarf_Die        /*subr_die */,
    Dwarf_Fde  *     /*returned_fde*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_get_fde_n(Dwarf_Fde* /*fde_data*/,
    Dwarf_Unsigned   /*fde_index*/,
    Dwarf_Fde  *     /*returned_fde*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_get_fde_at_pc(Dwarf_Fde* /*fde_data*/,
    Dwarf_Addr       /*pc_of_interest*/,
    Dwarf_Fde  *     /*returned_fde*/,
    Dwarf_Addr*      /*lopc*/,
    Dwarf_Addr*      /*hipc*/,
    Dwarf_Error*     /*error*/);

/*  GNU .eh_frame augmentation information,
    raw form, see
    Linux Standard Base Core Specification version 3.0 . */
DW_API int dwarf_get_cie_augmentation_data(Dwarf_Cie /* cie*/,
    Dwarf_Small **   /* augdata */,
    Dwarf_Unsigned * /* augdata_len */,
    Dwarf_Error*     /*error*/);
/*  GNU .eh_frame augmentation information,
    raw form, see
    Linux Standard Base Core Specification version 3.0 . */
DW_API int dwarf_get_fde_augmentation_data(Dwarf_Fde /* fde*/,
    Dwarf_Small **   /* augdata */,
    Dwarf_Unsigned * /* augdata_len */,
    Dwarf_Error*     /*error*/);

/*!
    @brief Expands CIE or FDE instructions for detailed examination.
    Called for CIE initial instructions and
    FDE instructions.
    Call dwarf_get_fde_instr_bytes() or
    dwarf_get_cie_info_b() to get the initial instruction bytes
    and instructions byte count you wish to expand.
    @param dw_cie
    The cie relevant to the instructions.
    @param dw_instructionspointer
    points to the instructions
    @param dw_length_in_bytes
    byte length of the instruction sequence.
    @param dw_head
    The address of an allocated dw_head
    @param dw_instr_count
    Returns the numer of inststructions in the byte stream
    @param dw_error
    Error return details
    @return On success returns DW_DLV_OK
*/
DW_API int dwarf_expand_frame_instructions(Dwarf_Cie dw_cie,
    Dwarf_Small     * dw_instructionspointer,
    Dwarf_Unsigned    dw_length_in_bytes,
    Dwarf_Frame_Instr_Head * dw_head,
    Dwarf_Unsigned  * dw_instr_count,
    Dwarf_Error     * dw_error);

/*!
    @brief Returns information about a single instruction
    Fields_description means a sequence of up to three
    letters including u,s,r,c,d,b, terminated by NUL byte.
    It is a string but we test individual bytes instead
    of using string compares. Do not free any of the
    returned values.
    @param dw_head
    A head record
    @param dw_instr_index
    index 0 < i < instr_count
    @param dw_instr_offset_in_instrs
    Returns the byte offset of this instruction
    within instructions.
    @param dw_cfa_operation
    Returns a DW_CFA opcode.
    @param dw_fields_description
    Returns a string. Do not free.
    @param dw_u0
    May be set to an unsigned value
    @param dw_u1
    May be set to an unsigned value
    @param dw_s0
    May be set to a signed value
    @param dw_s1
    May be set to a signed value
    @param dw_code_alignment_factor
    May be set by the call
    @param dw_data_alignment_factor
    May be set by the call
    @param dw_expression_block
    Pass in a pointer to a block
    @param dw_error
    If DW_DLV_ERROR and the argument is non-NULL, returns
    details about the error.
    @return On success returns DW_DLV_OK
    If there is no such instruction with that index
    it returns DW_DLV_NO_ENTRY
    On error it returns DW_DLV_ERROR and if dw_error
    is NULL it pushes back a pointer to a Dwarf_Error
    to the caller.

    Frame expressions have a variety of formats
    and content. The dw_fields parameter is set to
    a pointer to
    a short string with some set of the letters
    s,u,r,d,c,b which enables determining exactly
    which values the call sets.
    Some examples:
    A @c s in fields[0] means s0 is a signed number.

    A @c b somewhere in fields means the expression block
    passed in has been filled in.

    A @c r in fields[1] means u1 is set to a register number.

    A @c d in fields means data_alignment_factor is set

    A @c c in fields means code_alignment_factor is set
    There are just nine strings possible and together they
    describe all possible frame instructions.
*/
DW_API int dwarf_get_frame_instruction(
    Dwarf_Frame_Instr_Head  dw_head,
    Dwarf_Unsigned     dw_instr_index,
    Dwarf_Unsigned  *  dw_instr_offset_in_instrs,
    Dwarf_Small     *  dw_cfa_operation,
    const char      ** dw_fields_description,
    Dwarf_Unsigned  *  dw_u0,
    Dwarf_Unsigned  *  dw_u1,
    Dwarf_Signed    *  dw_s0,
    Dwarf_Signed    *  dw_s1,
    Dwarf_Unsigned  *  dw_code_alignment_factor,
    Dwarf_Signed    *  dw_data_alignment_factor,
    Dwarf_Block     *  dw_expression_block,
    Dwarf_Error     *  dw_error);
/*! @brief Expands CIE or FDE instructions for detailed examination.
    Called for CIE initial instructions and
    FDE instructions. This is the same as
    dwarf_get_frame_instruction() except that it adds a
    dw_u2 field which contains an address-space identifier
    if the letter @c a appears in dw_fields_description.
    The dw_u2 field is non-standard and only applies
    to two Heterogenous Debugging
    frame instructions defined by LLVM.

    The return values are the same except here we have:
    an @c a in fields[2] means dw_u2 is an address-space
    identifier for the LLVM CFA instruction.
*/
DW_API int dwarf_get_frame_instruction_a(
    Dwarf_Frame_Instr_Head dw_/* head*/,
    Dwarf_Unsigned     dw_instr_index,
    Dwarf_Unsigned  *  dw_instr_offset_in_instrs,
    Dwarf_Small     *  dw_cfa_operation,
    const char      ** dw_fields_description,
    Dwarf_Unsigned  *  dw_u0,
    Dwarf_Unsigned  *  dw_u1,
    Dwarf_Unsigned  *  dw_u2,
    Dwarf_Signed    *  dw_s0,
    Dwarf_Signed    *  dw_s1,
    Dwarf_Unsigned  *  dw_code_alignment_factor,
    Dwarf_Signed    *  dw_data_alignment_factor,
    Dwarf_Block     *  dw_expression_block,
    Dwarf_Error     *  dw_error);

/*!
    @brief Deallocates the frame instruction data in dw_head
    @param dw_head
    A head pointer.
    Frees all data created by dwarf_expand_frame_instructions()
    and makes the head pointer stale. The caller should
    set  to NULL.
*/
DW_API void dwarf_dealloc_frame_instr_head(Dwarf_Frame_Instr_Head
    dw_head);
/*  Used by dwarfdump -v to print fde offsets from debugging
    info.  */
DW_API int dwarf_fde_section_offset(Dwarf_Debug /*dbg*/,
    Dwarf_Fde         /*in_fde*/,
    Dwarf_Off *       /*fde_off*/,
    Dwarf_Off *       /*cie_off*/,
    Dwarf_Error *     /*err*/);

/* Used by dwarfdump -v to print cie offsets from debugging
   info.
*/
DW_API int dwarf_cie_section_offset(Dwarf_Debug /*dbg*/,
    Dwarf_Cie     /*in_cie*/,
    Dwarf_Off *   /*cie_off */,
    Dwarf_Error * /*err*/);

/*  The 'set' calls here return the original (before any change
    by these set routines) of the respective fields. */
/*  Additional interface with correct 'initial' spelling. */
/*  It is likely you will want to call the following 6 functions
    before accessing any frame information.  All are useful
    to tailor handling of pseudo-registers needed to turn
    frame operation references into simpler forms and to
    reflect ABI specific data.  Of course altering libdwarf.h
    and dwarf.h allow the same capabilities, but header changes
    in the distribution would require you re-integrate your
    libdwarf.h changes into the distributed libdwarf.h ...
    so use the following functions instead.*/
DW_API Dwarf_Half dwarf_set_frame_rule_initial_value(Dwarf_Debug,
    Dwarf_Half /*value*/);
DW_API Dwarf_Half dwarf_set_frame_rule_table_size(Dwarf_Debug,
    Dwarf_Half /*value*/);
DW_API Dwarf_Half dwarf_set_frame_cfa_value(Dwarf_Debug /*dbg*/,
    Dwarf_Half /*value*/);
DW_API Dwarf_Half dwarf_set_frame_same_value(Dwarf_Debug /*dbg*/,
    Dwarf_Half /*value*/);
DW_API Dwarf_Half dwarf_set_frame_undefined_value(Dwarf_Debug /*dbg*/,
    Dwarf_Half /*value*/);
/*  dwarf_set_default_address_size only sets 'value' if value is
    greater than zero. */
DW_API Dwarf_Small dwarf_set_default_address_size(Dwarf_Debug /*dbg*/,
    Dwarf_Small /* value */);
/*! @} */

/*! @defgroup abbrev Abbreviations .debug_abbrev Section Details
    @{
*/
/* abbreviation section operations */
DW_API int dwarf_get_abbrev(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned   /*offset*/,
    Dwarf_Abbrev  *  /*returned_abbrev*/,
    Dwarf_Unsigned*  /*length*/,
    Dwarf_Unsigned*  /*attr_count*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_get_abbrev_tag(Dwarf_Abbrev /*abbrev*/,
    Dwarf_Half*      /*return_tag_number*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_get_abbrev_code(Dwarf_Abbrev /*abbrev*/,
    Dwarf_Unsigned*  /*return_code_number*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_get_abbrev_children_flag(Dwarf_Abbrev /*abbrev*/,
    Dwarf_Signed*    /*return_flag*/,
    Dwarf_Error*     /*error*/);

/*  New August 2019.
    Most uses will call with filter_outliers non-zero.
    In that case impossible values return DW_DLV_ERROR.
    Those doing extra things (like dwarfdump) will
    call with filter_outliers zero to get the raw data
    (effectively); */
DW_API int dwarf_get_abbrev_entry_b(Dwarf_Abbrev /*abbrev*/,
    Dwarf_Unsigned   /*indx*/,
    Dwarf_Bool       /*filter_outliers*/,
    Dwarf_Unsigned * /*returned_attr_num*/,
    Dwarf_Unsigned * /*returned_form*/,
    Dwarf_Signed   * /*returned_implicit_const*/,
    Dwarf_Off      * /*offset*/,
    Dwarf_Error    * /*error*/);

/*! @} */
/*! @defgroup string String Section .debug_str Details
    @{
*/
/* consumer string section operation */
DW_API int dwarf_get_str(Dwarf_Debug /*dbg*/,
    Dwarf_Off        /*offset*/,
    char**           /*string*/,
    Dwarf_Signed *   /*strlen_of_string*/,
    Dwarf_Error*     /*error*/);

/*! @} */
/*! @defgroup str_offsets Str_offsets section .debug_str_offsets details
    @{
*/
/*  Allows applications to print the .debug_str_offsets
    section.
    Beginning at starting_offset zero,
    returns data about the first table found.
    The value *next_table_offset is the value
    of the next table (if any), one byte past
    the end of the table whose data is returned..
    Returns DW_DLV_NO_ENTRY if the starting offset
    is past the end of valid data.

    There is no guarantee that there are no non-0 nonsense
    bytes in the section outside of useful tables,
    so this can fail and return nonsense or
    DW_DLV_ERROR  if such garbage exists.
*/

/*  Allocates a struct Dwarf_Str_Offsets_Table_s for the section
    and returns DW_DLV_OK and sets a pointer to the struct through
    the table_data pointer if successful.

    @return
    If there is no .debug_str_offsets section it returns
    DW_DLV_NO_ENTRY
*/
DW_API int dwarf_open_str_offsets_table_access(Dwarf_Debug dw_dbg,
    Dwarf_Str_Offsets_Table * dw_table_data,
    Dwarf_Error             * dw_error);

/*  Close access, free table_data. */
DW_API int dwarf_close_str_offsets_table_access(
    Dwarf_Str_Offsets_Table  dw_table_data,
    Dwarf_Error            * dw_error);

/*! @brief call this to iterate through the
    offsets table(s).
    Call till it returns DW_DLV_NO_ENTRY (normal end)
    or DW_DLV_ERROR (error) and stop.
    @return
    Returns DW_DLV_NO_ENTRY if there are no more entries.
*/
DW_API int dwarf_next_str_offsets_table(Dwarf_Str_Offsets_Table
    dw_table_data,
    Dwarf_Unsigned * /*unit_length*/,
    Dwarf_Unsigned * /*unit_length_offset*/,
    Dwarf_Unsigned * /*table_start_offset*/,
    Dwarf_Half     * /*entry_size*/,
    Dwarf_Half     * /*version*/,
    Dwarf_Half     * /*padding*/,
    Dwarf_Unsigned * /*table_value_count*/,
    Dwarf_Error    * /*error*/);

/*  Valid index values n:  0 <= n <  table_entry_count
    for the active table */
DW_API int dwarf_str_offsets_value_by_index(Dwarf_Str_Offsets_Table,
    Dwarf_Unsigned   /*index_to_entry*/,
    Dwarf_Unsigned * /*entry_value*/,
    Dwarf_Error    * /*error*/);

/*  After all str_offsets read this reports final
    wasted-bytes count. */
DW_API int dwarf_str_offsets_statistics(Dwarf_Str_Offsets_Table,
    Dwarf_Unsigned * /*wasted_byte_count*/,
    Dwarf_Unsigned * /*table_count*/,
    Dwarf_Error    * /*error*/);

/*! @} */
/*! @defgroup dwarferror Dwarf_Error Functions
    These functions aid in understanding errors
    when a function returns DW_DLV_ERROR.
    @{
*/
/*! @brief What DW_DLE code does the error have?
    @param dw_error
    The dw_error should be non-null and a valid Dwarf_Error.
    @return
    A DW_DLE value of some kind. For example: DW_DLE_DIE_NULL.
*/
DW_API Dwarf_Unsigned dwarf_errno(Dwarf_Error dw_error);
/*! @brief What message string is in the error?
    @param dw_error
    The dw_error should be non-null and a valid Dwarf_Error.
    @return
    A string with a message related to the error.
*/
DW_API char* dwarf_errmsg(Dwarf_Error dw_error);
/*! @brief What message string is associated with the error number.
    @param dw_errornum
    The dw_error should be an integer from the DW_DLE set.
    For example, DW_DLE_DIE_NULL.
    @return
    The generic string describing that error number.
*/
DW_API char* dwarf_errmsg_by_number(Dwarf_Unsigned dw_errornum);

/*! @brief Creating an error.
    This is very rarely helpful.  It lets the library
    user create a Dwarf_Error and associate any string
    with that error. Your code could then return
    DW_DLV_ERROR to your caller when your intent
    is to let your caller clean up whatever seems wrong.
    @param dw_dbg
    The relevant Dwarf_Debugr.
    @param dw_error
    a Dwarf_Error is returned through this pointer.
    @param dw_errmsg
    The message string you provide.
*/
DW_API void  dwarf_error_creation(Dwarf_Debug dw_dbg ,
    Dwarf_Error * dw_error, char * dw_errmsg);

/*! @brief Free (dealloc) an Dwarf_Error somethimg created.
    @param dw_dbg
    The relevant Dwarf_Debug pointer.
    @param dw_error
    A pointer to a Dwarf_Error.  The pointer is then stale
    so you should immediately zero that pointer passed
    in.
*/
DW_API void dwarf_dealloc_error(Dwarf_Debug dw_dbg,
    Dwarf_Error dw_error);
/*! @} */

/*! @defgroup dwarfdealloc Generic dwarf_dealloc Function
    @{
*/
/*! @brief The generic dealloc (free) function.
    It requires you know the correct DW_DLA value
    to pass in.

    Many convenience functions are easier to use
    than this, such as dwarf_dealloc_die(),
    dwarf_dealloc_attribute() etc.

    @param dw_dbg
    Must be a valid open Dwarf_Debug.
    and must be the dw_dbg that the error
    was created on.
    If it is not the dealloc will do nothing.
    @param dw_space
    Must be an address returned directly
    by a libdwarf call that the
    call specifies as requiring
    dealloc/free.  If it is not
    a segfault or address fault is possible.
    @param dw_type
    Must be a correct naming of the DW_DLA type.
    If it is not the dealloc will do nothing.
*/
DW_API void dwarf_dealloc(Dwarf_Debug dw_dbg,
    void* dw_space, Dwarf_Unsigned dw_type);
/*! @} */
/*! @defgroup debugsup Access to Section .debug_sup
    @{
*/
/* For DWARF5 */
DW_API int dwarf_get_debug_sup(Dwarf_Debug /*dbg*/,
    Dwarf_Half     * /*version*/,
    Dwarf_Small    * /*is_supplementary*/,
    char          ** /*filename*/,
    Dwarf_Unsigned * /*checksum_len*/,
    Dwarf_Small   ** /*checksum*/,
    Dwarf_Error * /*error*/);
/*! @} */

/*! @defgroup debugnames Access to .debug_names Fast Access DWARF5
    @{
*/
/*  .debug_names names table interfaces. DWARF5.
    By Sections 6.1 and 6.1.1,
    "a name index is maintained in a separate object
    file section named .debug_names."
    It supercedes .debug_pubnames and .debug_pubtypes,
    which, also were wholeprogram lookup information.
    Nonetheless the following does not assume a single
    name index in an object file.
*/
DW_API int dwarf_dnames_header(Dwarf_Debug /*dbg*/,
    Dwarf_Off           /*starting_offset*/,
    Dwarf_Dnames_Head * /*dn_out*/,
    Dwarf_Off         * /*offset_of_next_table*/,
    Dwarf_Error *       /*error*/);

/*  Frees all the malloc data associated with dn */
DW_API void dwarf_dealloc_dnames(Dwarf_Dnames_Head dn);

DW_API int dwarf_dnames_sizes(Dwarf_Dnames_Head /*dn*/,
    /* The counts are entry counts, not byte sizes. */
    Dwarf_Unsigned * /*comp_unit_count*/,
    Dwarf_Unsigned * /*local_type_unit_count*/,
    Dwarf_Unsigned * /*foreign_type_unit_count*/,
    Dwarf_Unsigned * /*bucket_count*/,
    Dwarf_Unsigned * /*name_count*/,

    /* The following are counted in bytes */
    Dwarf_Unsigned * /*abbrev_table_size*/,
    Dwarf_Unsigned * /*entry_pool_size*/,
    Dwarf_Unsigned * /*augmentation_string_size*/,
    char          ** /*augmentation_string*/,
    Dwarf_Unsigned * /*section_size*/,
    Dwarf_Half     * /*table_version*/,
    Dwarf_Half     * /*offset_size*/,
    Dwarf_Error *    /*error*/);

/* get each list entry one at a time */
DW_API int dwarf_dnames_cu_table(Dwarf_Dnames_Head /*dn*/,
    const char        * /*type ("cu" "tu") */,
    /*  index number 0 to k-1 or 0 to t+f-1
        depending on type. */
    Dwarf_Unsigned      /*index_number*/,
    Dwarf_Unsigned    * /*offset (of cu/tu header)*/,
    Dwarf_Sig8        * /*sig (if signature) */,
    Dwarf_Error       * /*error*/);

/* Each bucket, one at a time */
DW_API int dwarf_dnames_bucket(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned      /*bucket_number*/,
    Dwarf_Unsigned    * /*index (of name entry*/,
    Dwarf_Unsigned    * /*indexcount (of name entries in bucket)*/,
    Dwarf_Error *       /*error*/);

/*  Each Name Table entry, one at a time.
    attr_array is an array of attribute/form
    pairs. So [0] is an attribute, [1] is
    its form. And so on. So attr_count returned
    is always a multiple of two. The last entry
    is 0,0 ending the list.
    It is not an error if array_size is zero or
    small. Check the returned attr_count to
    know now much of array filled in and
    if the array you provided is
    large enough. Possibly 40 (so 20 attributes)
    is large enough. */
DW_API int dwarf_dnames_name(Dwarf_Dnames_Head /*dn*/,
    Dwarf_Unsigned      /*name_index*/,
    Dwarf_Unsigned    * /*bucket_number */,
    Dwarf_Unsigned    * /*hash value*/,
    Dwarf_Unsigned    * /*offset_to_debug_str*/,
    char *            * /*ptrtostr (or null)*/,
    Dwarf_Unsigned    * /*offset_in_entrypool*/,
    /*  Following fields are from Entry Pool */
    Dwarf_Unsigned    * /* abbrev_number (from entrypool) */,
    Dwarf_Half        * /* abbrev_tag (from entrypool) */,
    Dwarf_Unsigned      /* array_size (of following) */,
    Dwarf_Half        * /* attr_array
        (caller provides array space, array need
        not be initialized) */,
    Dwarf_Unsigned    * /* attr_count
        (attr_array entries used count) */,
    Dwarf_Error *       /*error*/);

/*  A consumer wanting to use the lookup here
    to get CU or DIE information without
    using above calls to read the whole
    table has not been specified as it's not
    clear what would be good for that use case. */

/* end of .debug_names interfaces. */

/*! @} */

/*! @defgroup aranges Access to .debug_aranges Fast Access
    @{ */
DW_API int dwarf_get_aranges(Dwarf_Debug /*dbg*/,
    Dwarf_Arange**   /*aranges*/,
    Dwarf_Signed *   /*arange_count*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_get_arange(Dwarf_Arange* /*aranges*/,
    Dwarf_Unsigned   /*arange_count*/,
    Dwarf_Addr       /*address*/,
    Dwarf_Arange *   /*returned_arange*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_get_cu_die_offset(Dwarf_Arange /*arange*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);

DW_API int dwarf_get_arange_cu_header_offset(Dwarf_Arange /*arange*/,
    Dwarf_Off*       /*return_cu_header_offset*/,
    Dwarf_Error*     /*error*/);

/*  DWARF2,3,4 interface.
    New for DWARF4, entries may have segment information.
    *segment is only meaningful if *segment_entry_size is non-zero. */
DW_API int dwarf_get_arange_info_b(Dwarf_Arange     /*arange*/,
    Dwarf_Unsigned*  /*segment*/,
    Dwarf_Unsigned*  /*segment_entry_size*/,
    Dwarf_Addr    *  /*start*/,
    Dwarf_Unsigned*  /*length*/,
    Dwarf_Off     *  /*cu_die_offset*/,
    Dwarf_Error   *  /*error*/ );
/*! @} */

/*! @defgroup pubnames Access to .debug_pubnames plus. Fast Access
    @{ */

/*  global name space operations (.debug_pubnames access)
    The pubnames and similar sections are rarely used. Few compilers
    emit them. They are DWARF 2,3,4 only., not DWARF 5.  */

/*  New March 2019. Special for dwarfdump.
    Sets a flag in the dbg. Always returns DW_DLV_OK
    and never touches error */
DW_API int dwarf_return_empty_pubnames(Dwarf_Debug /*dbg*/,
    int /* flag */,
    Dwarf_Error* /*error*/);

DW_API int dwarf_get_globals(Dwarf_Debug /*dbg*/,
    Dwarf_Global**   /*globals*/,
    Dwarf_Signed *   /*number_of_globals*/,
    Dwarf_Error*     /*error*/);

DW_API void dwarf_globals_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Global*    /*globals*/,
    Dwarf_Signed     /*number_of_globals*/);

/*  The following four are closely related. */
DW_API int dwarf_globname(Dwarf_Global /*glob*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_global_die_offset(Dwarf_Global /*global*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error *    /*error*/);
DW_API int dwarf_global_cu_offset(Dwarf_Global /*global*/,
    Dwarf_Off*       /*return_offset, offset of cu header*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_global_name_offsets(Dwarf_Global /*global*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset, offset of
        cu die*/,
    Dwarf_Error*     /*error*/);

/*  New February 2019. For more complete dwarfdump printing.
    For each CU represented in .debug_pubnames, etc,
    there is a .debug_pubnames header.  For any given
    Dwarf_Global this returns the content of the applicable
    header. */
DW_API int dwarf_get_globals_header(Dwarf_Global /*global*/,
    Dwarf_Off      * /*offset_pub_header*/,
    Dwarf_Unsigned * /*length_size*/,
    Dwarf_Unsigned * /*length_pub*/,
    Dwarf_Unsigned * /*version*/,
    Dwarf_Unsigned * /*header_info_offset*/,
    Dwarf_Unsigned * /*info_length*/,
    Dwarf_Error*   /*error*/);

/* Static function name operations.  */
DW_API int dwarf_get_funcs(Dwarf_Debug    /*dbg*/,
    Dwarf_Func**     /*funcs*/,
    Dwarf_Signed *   /*number_of_funcs*/,
    Dwarf_Error*     /*error*/);
DW_API void dwarf_funcs_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Func*      /*funcs*/,
    Dwarf_Signed     /*number_of_funcs*/);

/* The following four are closely related */
DW_API int dwarf_funcname(Dwarf_Func /*func*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_func_die_offset(Dwarf_Func /*func*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_func_cu_offset(Dwarf_Func /*func*/,
    Dwarf_Off*       /*return_offset of the
        cu header*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_func_name_offsets(Dwarf_Func /*func*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset of the
        cu die*/,
    Dwarf_Error*     /*error*/);

/*  jUser-defined type name operations,
    SGI IRIX .debug_typenames section.

    Same content as DWARF3 .debug_pubtypes, but
    defined years before .debug_pubtypes was defined.
    SGI IRIX only. */
DW_API int dwarf_get_types(Dwarf_Debug    /*dbg*/,
    Dwarf_Type**     /*types*/,
    Dwarf_Signed *   /*number_of_types*/,
    Dwarf_Error*     /*error*/);
DW_API void dwarf_types_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Type*      /*types*/,
    Dwarf_Signed     /*number_of_types*/);

/*  The fourth gives all the values that the next
    three combined do. */
DW_API int dwarf_typename(Dwarf_Type /*type*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_type_die_offset(Dwarf_Type /*type*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_type_cu_offset(Dwarf_Type /*type*/,
    Dwarf_Off*       /*return_offset is offset of
        cu_header */,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_type_name_offsets(Dwarf_Type    /*type*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset is offset of
        cu_die */,
    Dwarf_Error*     /*error*/);

/*  User-defined type name operations, DWARF3  .debug_pubtypes
    section.  */
DW_API int dwarf_get_pubtypes(Dwarf_Debug    /*dbg*/,
    Dwarf_Type**     /*types*/,
    Dwarf_Signed *   /*number_of_types*/,
    Dwarf_Error*     /*error*/);
DW_API void dwarf_pubtypes_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Type*      /*pubtypes*/,
    Dwarf_Signed     /*number_of_pubtypes*/);

/*  The first three present the same information
    as the fourth here does in one call.
    Probably best to use the fourth one and ignore the
    first three. cu_offsset is cu_header offset. */
DW_API int dwarf_pubtypename(Dwarf_Type /*type*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_pubtype_type_die_offset(Dwarf_Type /*type*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_pubtype_cu_offset(Dwarf_Type /*type*/,
    Dwarf_Off*       /*return_offset is offset
        of cu_header */,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_pubtype_name_offsets(Dwarf_Type    /*type*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset is offset of
        cu_die */,
    Dwarf_Error*     /*error*/);

/* File-scope static variable name operations.  */
DW_API int dwarf_get_vars(Dwarf_Debug    /*dbg*/,
    Dwarf_Var**      /*vars*/,
    Dwarf_Signed *   /*number_of_vars*/,
    Dwarf_Error*     /*error*/);
DW_API void dwarf_vars_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Var*       /*vars*/,
    Dwarf_Signed     /*number_of_vars*/);

/*  The following four closely related. */
DW_API int dwarf_varname(Dwarf_Var /*var*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_var_die_offset(Dwarf_Var /*var*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_var_cu_offset(Dwarf_Var /*var*/,
    Dwarf_Off*       /*return_offset of
        the cu header*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_var_name_offsets(Dwarf_Var /*var*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset of the
        cu die */,
    Dwarf_Error*     /*error*/);

/* weak name operations.  */
DW_API int dwarf_get_weaks(Dwarf_Debug    /*dbg*/,
    Dwarf_Weak**     /*weaks*/,
    Dwarf_Signed *   /*number_of_weaks*/,
    Dwarf_Error*     /*error*/);
DW_API void dwarf_weaks_dealloc(Dwarf_Debug /*dbg*/,
    Dwarf_Weak*      /*weaks*/,
    Dwarf_Signed     /*number_of_weaks*/);

/*  The following four closely related. */
DW_API int dwarf_weakname(Dwarf_Weak /*weak*/,
    char   **        /*returned_name*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_weak_die_offset(Dwarf_Weak /*weak*/,
    Dwarf_Off*       /*return_offset*/,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_weak_cu_offset(Dwarf_Weak /*weak*/,
    Dwarf_Off*       /*return_offset of
        the CU header */,
    Dwarf_Error*     /*error*/);
DW_API int dwarf_weak_name_offsets(Dwarf_Weak    /*weak*/,
    char   **        /*returned_name*/,
    Dwarf_Off*       /*die_offset*/,
    Dwarf_Off*       /*cu_offset of
        the CU die */,
    Dwarf_Error*     /*error*/);
/*! @} */

/*! @defgroup gnupubnames Access GNU .debug_gnu_pubnames Fast Access
    @{
*/
/*  BEGIN: .debug_gnu_pubnames .debug_gnu_typenames access,
    calling these  Gnu_Index as a general reference.  */
DW_API int dwarf_get_gnu_index_head(Dwarf_Debug /*dbg*/,
    /*  The following arg false to select gnu_pubtypes */
    Dwarf_Bool             /*for_gdb_pubnames*/ ,
    Dwarf_Gnu_Index_Head * /*index_head_out*/,
    Dwarf_Unsigned       * /*index_block_count_out*/,
    Dwarf_Error * /*error*/);
/*  Frees all resources used for the indexes. */
DW_API void dwarf_gnu_index_dealloc(Dwarf_Gnu_Index_Head /*head*/);

DW_API int dwarf_get_gnu_index_block(Dwarf_Gnu_Index_Head /*head*/,
    Dwarf_Unsigned     /*number*/,
    Dwarf_Unsigned   * /*block_length */,
    Dwarf_Half       * /*version */,
    Dwarf_Unsigned   * /*offset_into_debug_info*/,
    Dwarf_Unsigned   * /*size_of_debug_info_area*/,
    Dwarf_Unsigned   * /*count_of_index_entries*/,
    Dwarf_Error      * /*error*/);

DW_API int dwarf_get_gnu_index_block_entry(
    Dwarf_Gnu_Index_Head /*head*/,
    Dwarf_Unsigned    /*blocknumber*/,
    Dwarf_Unsigned    /*entrynumber*/,
    Dwarf_Unsigned  * /*offset_in_debug_info*/,
    const char     ** /*name_string*/,
    unsigned char   * /*flagbyte*/,
    unsigned char   * /*staticorglobal*/,
    unsigned char   * /*typeofentry*/,
    Dwarf_Error     * /*error*/);
/* END: debug_gnu_pubnames/typenames access, */

/*! @} */

/*! @defgroup gnudebuglink Access GNU .gnu_debuglink, build-id.
    When DWARF is separate from a normal shared object.
    Has nothing to do with split-dwarf/debug-fission.
    @{
*/

/*  New October 2019.  Access to the GNU section named
    .gnu_debuglink  and/or the section .note.gnu.build-id.
    See
    https://sourceware.org/gdb/onlinedocs/gdb/
        Separate-Debug-Files.html

    If no debuglink then name_returned,crc_returned and
    debuglink_path_returned will get set 0 through the pointers.

    If no .note.gnu.build-id then  buildid_length_returned,
    and buildid_returned will be set 0 through the pointers.

    Caller frees space returned by debuglink_fullpath_returned.

    See libdwarf2.1.mm revision 3.13 or later for
    additional important details.
*/

DW_API int dwarf_gnu_debuglink(Dwarf_Debug /*dbg*/,
    char     **    /*debuglink_path_returned */,
    unsigned char ** /*crc_returned from the debuglink section*/,
    char     **    /*debuglink_fullpath_returned free this*/,
    unsigned int * /*debuglink_path_count_returned */,
    unsigned int * /*buildid_type_returned */,
    char     **    /*buildid_owner_name_returned*/,
    unsigned char ** /*buildid_returned*/,
    unsigned int * /*buildid_length_returned*/,
    char     ***   /*paths_returned*/,
    unsigned int * /*paths_length_returned*/,
    Dwarf_Error*   /*error*/);

/*  Only useful inside dwarfexample/dwdebuglink.c
    so we can show all that is going on.
*/
DW_API int dwarf_add_debuglink_global_path(Dwarf_Debug /*dbg*/,
    const char *pathname,
    Dwarf_Error* /*error*/);

/*  crc32 used for debuglink crc calculation.
    Caller passes pointer to array of 4 unsigned char
    provided by the caller and if this returns
    DW_DLV_OK that is filled in. */
DW_API int dwarf_crc32(Dwarf_Debug /*dbg*/,
    unsigned char * /*crcbuf*/,
    Dwarf_Error * /*error*/);

/*  Public interface to the real crc calculation
    just in case some find it useful. */
DW_API unsigned int dwarf_basic_crc32(const unsigned char * /*buf*/,
    unsigned long /*len*/, unsigned int /*init*/);
/*! @} */

/*! @defgroup gdbindex Gdb Index Fast Access to .dwo or .dwp
    If .gdb_index present in an executable then
    the content here helps quickly find their way
    into the .dwo or .dwp .debug_cu_index or .debug_tu_index
    split dwarf sections.
    @{
*/

/*  .gdb_index section operations.
    A GDB extension. Completely different than
    .debug_gnu_pubnames or .debug_gnu_pubtypes sections.
    The section is in some executables and if present
    is used to quickly map an address or name to
    a skeleton CU or TU.  If present then there are
    .dwo or .dwp files somewhere to make detailed
    debugging possible (up to user code to
    find it/them and deal with them).

    Version 8 built by gdb, so type entries are ok as is.
    Version 7 built by the 'gold' linker and type index
    entries for a CU must be derived othewise, the
    type index is not correct... ? FIXME
    */

/*  Creates a Dwarf_Gdbindex, returning it and
    its values through the pointers. */
DW_API int dwarf_gdbindex_header(Dwarf_Debug /*dbg*/,
    Dwarf_Gdbindex * /*gdbindexptr*/,
    Dwarf_Unsigned * /*version*/,
    Dwarf_Unsigned * /*cu_list_offset*/,
    Dwarf_Unsigned * /*types_cu_list_offset*/,
    Dwarf_Unsigned * /*address_area_offset*/,
    Dwarf_Unsigned * /*symbol_table_offset*/,
    Dwarf_Unsigned * /*constant_pool_offset*/,
    Dwarf_Unsigned * /*section_size*/,
    const char    ** /*section_name*/,
    Dwarf_Error    * /*error*/);

DW_API int dwarf_gdbindex_culist_array(Dwarf_Gdbindex /*gdbindexptr*/,
    Dwarf_Unsigned       * /*list_length*/,
    Dwarf_Error          * /*error*/);

/*  entryindex: 0 to list_length-1 */
DW_API int dwarf_gdbindex_culist_entry(Dwarf_Gdbindex /*gdbindexptr*/,
    Dwarf_Unsigned   /*entryindex*/,
    Dwarf_Unsigned * /*cu_offset*/,
    Dwarf_Unsigned * /*cu_length*/,
    Dwarf_Error    * /*error*/);

DW_API int dwarf_gdbindex_types_culist_array(Dwarf_Gdbindex
    /*ndexptr*/,
    Dwarf_Unsigned            * /*types_list_length*/,
    Dwarf_Error               * /*error*/);

/*  entryindex: 0 to types_list_length -1 */
DW_API int dwarf_gdbindex_types_culist_entry(
    Dwarf_Gdbindex   /*gdbindexptr*/,
    Dwarf_Unsigned   /*entryindex*/,
    Dwarf_Unsigned * /*cu_offset*/,
    Dwarf_Unsigned * /*tu_offset*/,
    Dwarf_Unsigned * /*type_signature*/,
    Dwarf_Error    * /*error*/);

DW_API int dwarf_gdbindex_addressarea(Dwarf_Gdbindex /*gdbindexptr*/,
    Dwarf_Unsigned            * /*addressarea_list_length*/,
    Dwarf_Error               * /*error*/);

/*    entryindex: 0 to addressarea_list_length-1 */
DW_API int dwarf_gdbindex_addressarea_entry(Dwarf_Gdbindex
    /*gdbindexptr*/,
    Dwarf_Unsigned   /*entryindex*/,
    Dwarf_Unsigned * /*low_adddress*/,
    Dwarf_Unsigned * /*high_address*/,
    Dwarf_Unsigned * /*cu_index*/,
    Dwarf_Error    * /*error*/);

DW_API int dwarf_gdbindex_symboltable_array(Dwarf_Gdbindex
    /*gdbindexptr*/,
    Dwarf_Unsigned            * /*symtab_list_length*/,
    Dwarf_Error               * /*error*/);

/*  entryindex: 0 to symtab_list_length-1 */
DW_API int dwarf_gdbindex_symboltable_entry(Dwarf_Gdbindex
    /*gdbindexptr*/,
    Dwarf_Unsigned   /*entryindex*/,
    Dwarf_Unsigned * /*string_offset*/,
    Dwarf_Unsigned * /*cu_vector_offset*/,
    Dwarf_Error    * /*error*/);

DW_API int dwarf_gdbindex_cuvector_length(Dwarf_Gdbindex /*gdbindex*/,
    Dwarf_Unsigned   /*cuvector_offset*/,
    Dwarf_Unsigned * /*innercount*/,
    Dwarf_Error    * /*error*/);

DW_API int dwarf_gdbindex_cuvector_inner_attributes(
    Dwarf_Gdbindex /*index*/,
    Dwarf_Unsigned   /*cuvector_offset*/,
    Dwarf_Unsigned   /*innerindex*/,
    /* The attr_value is a field of bits. For expanded version
        use  dwarf_gdbindex_cuvector_instance_expand_value() */
    Dwarf_Unsigned * /*attr_value*/,
    Dwarf_Error    * /*error*/);

DW_API int dwarf_gdbindex_cuvector_instance_expand_value(
    Dwarf_Gdbindex,
    Dwarf_Unsigned   /*value*/,
    Dwarf_Unsigned * /*cu_index*/,
    Dwarf_Unsigned * /*symbol_kind*/,
    Dwarf_Unsigned * /*is_static*/,
    Dwarf_Error    * /*error*/);

/*  The strings in the pool follow (in memory) the cu index
    set and are NUL terminated. */

DW_API int dwarf_gdbindex_string_by_offset(Dwarf_Gdbindex
    /*gdbindexptr*/,
    Dwarf_Unsigned   /*stringoffset*/,
    const char    ** /*string_ptr*/,
    Dwarf_Error   *  /*error*/);

DW_API void dwarf_gdbindex_free(Dwarf_Gdbindex /*gdbindexptr*/);

/*! @} */

/*! @defgroup splitdwarf  Split Dwarf (Debug Fission) Fast Access
    @{
*/
/*  START debugfission dwp .debug_cu_index
    and .debug_tu_index operations. */

DW_API int dwarf_get_xu_index_header(Dwarf_Debug /*dbg*/,
    const char *  section_type, /* "tu" or "cu" */
    Dwarf_Xu_Index_Header *     /*xuhdr*/,
    Dwarf_Unsigned *            /*version_number*/,
    Dwarf_Unsigned *            /*offsets_count L*/,
    Dwarf_Unsigned *            /*units_count N*/,
    Dwarf_Unsigned *            /*hash_slots_count M*/,
    const char     **           /*sect_name*/,
    Dwarf_Error *               /*err*/);

DW_API int dwarf_get_xu_index_section_type(Dwarf_Xu_Index_Header
    /*xuhdr*/,
    /*  the function returns a pointer to
        the immutable string "tu" or "cu" via this arg.
        Do not free.  */
    const char ** /*typename*/,
    /*  the function returns a pointer to
        the immutable section name. Do not free.
        .debug_cu_index or .debug_tu_index */
    const char ** /*sectionname*/,
    Dwarf_Error * /*err*/);

/*  Index values 0 to M-1 are valid. */
DW_API int dwarf_get_xu_hash_entry(Dwarf_Xu_Index_Header /*xuhdr*/,
    Dwarf_Unsigned     /*index*/,

    /*  Returns the hash value. 64  bits.  */
    Dwarf_Sig8 *      /*hash_value*/,

    /* returns the index into rows of offset/size tables. */
    Dwarf_Unsigned *  /*index_to_sections*/,
    Dwarf_Error *     /*err*/);

/*  Columns 0 to L-1,  valid. */
DW_API int dwarf_get_xu_section_names(Dwarf_Xu_Index_Header /*xuhdr*/,
    /* Row index defined to be row zero. */
    Dwarf_Unsigned  /*column_index*/,
    Dwarf_Unsigned* /*DW_SECT_ number*/,
    const char **   /*DW_SECT_ name*/,
    Dwarf_Error *   /*err*/);

    /* Rows 1 to N col 0 to L-1  are valid */
DW_API int dwarf_get_xu_section_offset(Dwarf_Xu_Index_Header /*xuhd*/,
    Dwarf_Unsigned  /*row_index*/,
    Dwarf_Unsigned  /*column_index*/,
    Dwarf_Unsigned* /*sec_offset*/,
    Dwarf_Unsigned* /*sec_size*/,
    Dwarf_Error *   /*err*/);

DW_API void dwarf_xu_header_free(Dwarf_Xu_Index_Header /*xuhdr*/);
/*  For any Dwarf_Die in a compilation unit, return
    the debug fission table data through
    percu_out.   Usually applications
    will pass in the CU die.
    Calling code should zero all of the
    struct Dwarf_Debug_Fission_Per_CU_s before calling this.
    If there is no debugfission data this returns
    DW_DLV_NO_ENTRY (only .dwp objects have debugfission data).  */
DW_API int dwarf_get_debugfission_for_die(Dwarf_Die /* die */,
    Dwarf_Debug_Fission_Per_CU * /* percu_out */,
    Dwarf_Error * /* err */);

/*  Given a key (hash signature)  from a .o,
    find the per-cu information
    for the CU with that key. */
DW_API int dwarf_get_debugfission_for_key(Dwarf_Debug /*dbg*/,
    Dwarf_Sig8 *                 /*key, hash signature */,
    const char * key_type        /*"cu" or "tu" */,
    Dwarf_Debug_Fission_Per_CU * /*percu_out */,
    Dwarf_Error *                /*err */);

/*  END debugfission dwp .debug_cu_index
    and .debug_tu_index operations. */

/*! @} */

/*! @defgroup harmless Harmless Error recording

    The harmless error list is a circular buffer of
    errors we note but which do not stop us from processing
    the object.  Created so dwarfdump or other tools
    can report such inconsequential errors without causing
    anything to stop early.
    @{
*/
/*! @brief  Default size of the libdwarf-internal circular list */
#define DW_HARMLESS_ERROR_CIRCULAR_LIST_DEFAULT_SIZE 4

/*! @brief Get the harmless error count and content

    User code supplies size of array of pointers dw_errmsg_ptrs_array
    in count and the array of pointers (the pointers themselves
    need not be initialized).
    The pointers returned in the array of pointers
    are invalidated by ANY call to libdwarf.
    Use them before making another libdwarf call!
    The array of string pointers passed in always has
    a final null pointer, so if there are N pointers the
    and M actual strings, then MIN(M,N-1) pointers are
    set to point to error strings.  The array of pointers
    to strings always terminates with a NULL pointer.
    Do not free the strings.  Every string is null-terminated.

    Each call empties the error list (discarding all current entries).
    and fills in your array 

    @param dw_dbg
    The applicable Dwarf_Debug.
    @param dw_count
    The number of string buffers. 
    If count is passed as zero no elements of the array are touched.
    @param dw_errmsg_ptrs_array
    A pointer to a user-created array of
    pointer to const char.
    @param dw_newerr_count
    If non-NULL the count of harmless errors
    pointers since the last call is returned through the pointer.
    If dw_count is greater than zero the first dw_count
    of the pointers in the user-created array point
    to null-terminated strings. Do not free the strings.
    print or copy the strings before any other libdwarf call.

    @return
    Returns DW_DLV_NO_ENTRY if no harmless errors
    were noted so far.  Returns DW_DLV_OK if there are
    harmless errors.  Never returns DW_DLV_ERROR.

    If DW_DLV_NO_ENTRY is returned none of the arguments
    other than dw_dbg are touched or used.
    */
DW_API int dwarf_get_harmless_error_list(Dwarf_Debug dw_dbg,
    unsigned int   dw_count,
    const char **  dw_errmsg_ptrs_array,
    unsigned int * dw_newerr_count);

/*! @brief The size of the circular list of strings 
    libdwarf holds internally may be set
    and reset as needed.  If it is shortened excess
    messages are simply dropped.  It returns the previous
    size. If zero passed in the size is unchanged
    and it simply returns the current size.  

    @param dw_dbg
    The applicable Dwarf_Debug.
    @param dw_maxcount
    Set the new internal buffer count to a number greater
    than zero.
    @return
    returns the current size of the internal circular
    buffer if dw_maxcount is zero.
    If dw_maxcount is greater than zero the internal
    array is adjusted to hold that many and the
    previous number of harmless errors possible in
    the circular buffer is returned.
    */
DW_API unsigned int dwarf_set_harmless_error_list_size(
    Dwarf_Debug  dw_dbg,
    unsigned int dw_maxcount);

/*  Insertion is only for testing the harmless error code, it is not
    useful otherwise. */
DW_API void dwarf_insert_harmless_error(Dwarf_Debug dw_dbg,
    char * dw_newerror);
/*! @} */

/*! @defgroup Naming Names DW_TAG AT etc as strings 
    @{
*/
/*  When the val_in is known these, for example, dwarf_get_TAG_name
    functions return the string corresponding to the val_in passed in
    through the pointer s_out and the value returned is DW_DLV_OK.
    The strings are in static storage
    and must not be freed.
    If DW_DLV_NO_ENTRY is returned the val_in is not known and
    *s_out is not set.  DW_DLV_ERROR is never returned.*/

/* The following copied from a generated dwarf_names.h */

/*  dwarf_get_LLEX_name is for a GNU extension.
    Not defined by the GNU folks nor a DWARF standard
    but it seemed essential. */
/* BEGIN FILE */
DW_API int dwarf_get_ACCESS_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_ADDR_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_AT_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_ATCF_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_ATE_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_CC_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_CFA_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_children_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_CHILDREN_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_DEFAULTED_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_DS_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_DSC_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_EH_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_END_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_FORM_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_FRAME_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_GNUIKIND_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_GNUIVIS_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_ID_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_IDX_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_INL_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_ISA_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_LANG_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_LLE_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_LLEX_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_LNCT_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_LNE_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_LNS_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_MACINFO_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_MACRO_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_OP_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_ORD_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_RLE_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_SECT_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_TAG_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_UT_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_VIRTUALITY_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
DW_API int dwarf_get_VIS_name(unsigned int /*val_in*/,
    const char ** /*s_out */);
/* END FILE */

DW_API int dwarf_get_FORM_CLASS_name(enum Dwarf_Form_Class /*fc*/,
    const char ** /*s_out*/);
/*! @} */

/*! @defgroup objectsections Object Sections Data
    @{
*/
/*  Section name access.  Because sections might
    now end with .dwo or be .zdebug  or might not.
*/
DW_API int dwarf_get_die_section_name(Dwarf_Debug /*dbg*/,
    Dwarf_Bool    /*is_info*/,
    const char ** /*sec_name*/,
    Dwarf_Error * /*error*/);

DW_API int dwarf_get_die_section_name_b(Dwarf_Die /*die*/,
    const char ** /*sec_name*/,
    Dwarf_Error * /*error*/);

DW_API int dwarf_get_real_section_name(Dwarf_Debug /*dbg*/,
    const char * /*std_section_name*/,
    const char ** /*actual_sec_name_out*/,
    Dwarf_Small * /*marked_compressed*/,  /* .zdebug... */
    Dwarf_Small * /*marked_zlib_compressed */, /* ZLIB string */
    Dwarf_Small * /*marked_shf_compressed*/, /* SHF_COMPRESSED */
    Dwarf_Unsigned * /*compressed_length*/,
    Dwarf_Unsigned * /*uncompressed_length*/,
    Dwarf_Error * /*error*/);

DW_API int dwarf_get_frame_section_name(Dwarf_Debug /*dbg*/,
    const char ** /*section_name_out*/,
    Dwarf_Error * /*error*/);

DW_API int dwarf_get_frame_section_name_eh_gnu(Dwarf_Debug /*dbg*/,
    const char ** /*section_name_out*/,
    Dwarf_Error * /*error*/);
DW_API int dwarf_get_aranges_section_name(Dwarf_Debug /*dbg*/,
    const char ** /*section_name_out*/,
    Dwarf_Error * /*error*/);
DW_API int dwarf_get_ranges_section_name(Dwarf_Debug /*dbg*/,
    const char ** /*section_name_out*/,
    Dwarf_Error * /*error*/);

/*  These two get the offset or address size as defined
    by the object format (not by DWARF). */
DW_API int dwarf_get_offset_size(Dwarf_Debug /*dbg*/,
    Dwarf_Half  *    /*offset_size*/,
    Dwarf_Error *    /*error*/);
DW_API int dwarf_get_address_size(Dwarf_Debug /*dbg*/,
    Dwarf_Half  *    /*addr_size*/,
    Dwarf_Error *    /*error*/);

DW_API int dwarf_get_string_section_name(Dwarf_Debug /*dbg*/,
    const char ** /*section_name_out*/,
    Dwarf_Error * /*error*/);

/* Giving a section name, get its size and address */
DW_API int dwarf_get_section_info_by_name(Dwarf_Debug /*dbg*/,
    const char *     /*section_name*/,
    Dwarf_Addr*      /*section_addr*/,
    Dwarf_Unsigned*  /*section_size*/,
    Dwarf_Error*     /*error*/);

/* Giving a section index, get its size and address */
DW_API int dwarf_get_section_info_by_index(Dwarf_Debug /*dbg*/,
    int              /*section_index*/,
    const char **    /*section_name*/,
    Dwarf_Addr*      /*section_addr*/,
    Dwarf_Unsigned*  /*section_size*/,
    Dwarf_Error*     /*error*/);

/*  Get section count, of object file sections. */
DW_API int dwarf_get_section_count(Dwarf_Debug /*dbg*/);

/*  This is a hack so clients can verify offsets.
    Added so that one can detect broken offsets
    (which happened in an IRIX executable larger than 2GB
    with MIPSpro 7.3.1.3 toolchain.).
    This has 21 arguments, which is...unusual.
*/
DW_API int dwarf_get_section_max_offsets_d(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned * /*debug_info_size*/,
    Dwarf_Unsigned * /*debug_abbrev_size*/,
    Dwarf_Unsigned * /*debug_line_size*/,
    Dwarf_Unsigned * /*debug_loc_size*/,
    Dwarf_Unsigned * /*debug_aranges_size*/,

    Dwarf_Unsigned * /*debug_macinfo_size*/,
    Dwarf_Unsigned * /*debug_pubnames_size*/,
    Dwarf_Unsigned * /*debug_str_size*/,
    Dwarf_Unsigned * /*debug_frame_size*/,
    Dwarf_Unsigned * /*debug_ranges_size*/,

    Dwarf_Unsigned * /*debug_pubtypes_size*/,
    Dwarf_Unsigned * /*debug_types_size*/,
    Dwarf_Unsigned * /*debug_macro_size*/,
    Dwarf_Unsigned * /*debug_str_offsets_size*/,
    Dwarf_Unsigned * /*debug_sup_size*/,

    Dwarf_Unsigned * /*debug_cu_index_size*/,
    Dwarf_Unsigned * /*debug_tu_index_size*/,
    Dwarf_Unsigned * /*debug_names_size*/,
    Dwarf_Unsigned * /*debug_loclists_size*/,
    Dwarf_Unsigned * /*debug_rnglists_size*/);

/*  Allows callers to find out what groups (dwo or COMDAT)
    are in the object and how much to allocate so one can get the
    group-section map data. */
DW_API int dwarf_sec_group_sizes(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned * /*section_count_out*/,
    Dwarf_Unsigned * /*group_count_out*/,
    Dwarf_Unsigned * /*selected_group_out*/,
    Dwarf_Unsigned * /*map_entry_count_out*/,
    Dwarf_Error    * /*error*/);

/*  New May 2017. Reveals the map between group numbers and
    section numbers.
    Caller must allocate the arrays with space for 'map_entry_count'
    values and this function fills in the array entries.
    Output ordered by group number and section number.
    */
DW_API int dwarf_sec_group_map(Dwarf_Debug /*dbg*/,
    Dwarf_Unsigned   /*map_entry_count*/,
    Dwarf_Unsigned * /*group_numbers_array*/,
    Dwarf_Unsigned * /*sec_numbers_array*/,
    const char    ** /*sec_names_array*/,
    Dwarf_Error    * /*error*/);
/*! @} */

/*! @defgroup leb LEB Encode and Decode
    @{
*/
DW_API int dwarf_encode_leb128(Dwarf_Unsigned /*val*/,
    int * /*nbytes*/,
    char * /*space*/,
    int /*splen*/);
DW_API int dwarf_encode_signed_leb128(Dwarf_Signed /*val*/,
    int * /*nbytes*/,
    char * /*space*/,
    int /*splen*/);
/*  Same for LEB decoding routines.
    caller sets endptr to an address one past the last valid
    address the library should be allowed to
    access. */
DW_API int dwarf_decode_leb128(char * /*leb*/,
    Dwarf_Unsigned * /*leblen*/,
    Dwarf_Unsigned * /*outval*/,
    char           * /*endptr*/);
DW_API int dwarf_decode_signed_leb128(char * /*leb*/,
    Dwarf_Unsigned * /*leblen*/,
    Dwarf_Signed   * /*outval*/,
    char           * /*endptr*/);
/*! @} */

/*! @defgroup miscellaneous Miscellaneous Functions
    @{
*/
/*  Returns the version string. Example: "0.3.0"
    which is a Semantic Version identifier.
    Before September 2021 the version string was
    a date, for example "20210528",
    which is in ISO date format. */
DW_API const char * dwarf_package_version(void);
/*  stringcheck zero is default and means do all
    string length validity checks.
    Call with parameter value 1 to turn off many such checks (and
    increase performance).
    Call with zero for safest running.
    Actual value saved and returned is only 8 bits! Upper bits
    ignored by libdwarf (and zero on return).
    Returns previous value.  */
DW_API int dwarf_set_stringcheck(int /*stringcheck*/);

/*  'apply' defaults to 1 and means do all
    'rela' relocations on reading in a dwarf object section with
    such relocations.
    Call with parameter value 0 to turn off application of
    such relocations.
    Since the static linker leaves 'bogus' data in object sections
    with a 'rela' relocation section such data cannot be read
    sensibly without processing the relocations.  Such relocations
    do not exist in executables and shared objects (.so), the
    relocations only exist in plain .o relocatable object files.
    Actual value saved and returned is only 8 bits! Upper bits
    ignored by libdwarf (and zero on return).
    Returns previous value.  */
DW_API int dwarf_set_reloc_application(int /*apply*/);

/*  dwarf_get_endian_copy_function new. December 2019. */
DW_API void (*dwarf_get_endian_copy_function(Dwarf_Debug /*dbg*/))
    (void *, const void * /*src*/, unsigned long /*srclen*/);

/*  A global flag in libdwarf. Applies to all Dwarf_Debug */
DW_API extern Dwarf_Cmdline_Options dwarf_cmdline_options;

/*! @brief Tell libdwarf add verbosity
    By default the flag is zero.
    If non-zero and there is an error when reading
    a line table header
    create a detailed string
    and use the printf callback so the
    client code an print it save it.
*/
DW_API void dwarf_record_cmdline_options(
    Dwarf_Cmdline_Options dd_options);

/*!  @brief Eliminate libdwarf tracking of allocations
    Independent of any Dwarf_Debug and applicable
    to all whenever the setting is changed.
    Defaults to non-zero.
    If zero applied libdwarf will run somewhat faster.
    If zero is applied then library memory allocations
    will not all be tracked and dwarf_finish() will
    be unable to free/dealloc some things.
    User code can do the necessary deallocs
    (as documented), but the normal guarantee
    that libdwarf will clean up is revoked.
*/
DW_API int dwarf_set_de_alloc_flag(int dw_v);

/*! @}
*/

/*! @defgroup objectdetector Determine Object Type of a File
    @{
*/
DW_API int dwarf_object_detector_path_b(const char * /*path*/,
    char         *   /* outpath_buffer*/,
    unsigned long    /* outpathlen*/,
    char **          /* gl_pathnames*/,
    unsigned int     /* gl_pathcount*/,
    unsigned int *   /* ftype*/,
    unsigned int *   /* endian*/,
    unsigned int *   /* offsetsize*/,
    Dwarf_Unsigned * /* filesize*/,
    unsigned char *  /* pathsource*/,
    int * /*errcode*/);

/* Solely looks for dSYM */
DW_API int dwarf_object_detector_path_dSYM(const char * /*path*/,
    char *         /* outpath*/,
    unsigned long  /* outpath_len*/,
    char **        /* gl_pathnames*/,
    unsigned int   /* gl_pathcount*/,
    unsigned int * /* ftype*/,
    unsigned int * /* endian*/,
    unsigned int * /* offsetsize*/,
    Dwarf_Unsigned  * /* filesize*/,
    unsigned char  *  /* pathsource*/,
    int *             /* errcode*/);

DW_API int dwarf_object_detector_fd(int /*fd*/,
    unsigned int * /*ftype*/,
    unsigned int * /*endian*/,
    unsigned int * /*offsetsize*/,
    Dwarf_Unsigned  * /*filesize*/,
    int *  /*errcode*/);

/*! @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _LIBDWARF_H */
