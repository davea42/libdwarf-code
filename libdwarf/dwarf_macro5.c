/*
  Copyright (C) 2015-2015 David Anderson. All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.
    
  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, write the Free Software
  Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston MA 02110-1301,
  USA.
    
*/

#include "config.h"
#include "dwarf_incl.h"
#include <stdio.h>
#include <limits.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include "dwarf_macro5.h"

# if 0
#define LEFTPAREN '('
#define RIGHTPAREN ')'
#define SPACE ' '
#endif
#define TRUE 1
#define FALSE 0

static const Dwarf_Small dwarf_udata_string_form[]  = {DW_FORM_udata,DW_FORM_string};
static const Dwarf_Small dwarf_udata_udata_form[]   = {DW_FORM_udata,DW_FORM_udata};
static const Dwarf_Small dwarf_udata_strp_form[]    = {DW_FORM_udata,DW_FORM_strp};
static const Dwarf_Small dwarf_udata_strp_sup_form[] = {DW_FORM_udata,DW_FORM_strp_sup};
static const Dwarf_Small dwarf_secoffset_form[]     = {DW_FORM_sec_offset};
static const Dwarf_Small dwarf_udata_strx_form[]    = {DW_FORM_udata,DW_FORM_strx};

#if 0
/* We do not presently use this here. It's a view of DW2 macros. */
struct Dwarf_Macro_Forms_s dw2formsarray[] = {
    {0,0,0},
    {DW_MACINFO_define,2,dwarf_udata_string_form},
    {DW_MACINFO_undef,2,dwarf_udata_string_form},
    {DW_MACINFO_start_file,2,dwarf_udata_udata_form},
    {DW_MACINFO_end_file,0,0},
    {DW_MACINFO_vendor_ext,2,dwarf_udata_string_form},
};

/* Represents original DWARF 2,3,4 macro info */
static const struct Dwarf_Macro_OperationsList_s dwarf_default_macinfo_opslist= {
6, dw2formsarray 
};
#endif

struct Dwarf_Macro_Forms_s dw5formsarray[] = {
    {0,0,0},
    {DW_MACRO_define,2,dwarf_udata_string_form},
    {DW_MACRO_undef,2,dwarf_udata_string_form},
    {DW_MACRO_start_file,2,dwarf_udata_udata_form},
    {DW_MACRO_end_file,0,0},

    {DW_MACRO_define_strp,2,dwarf_udata_strp_form},
    {DW_MACRO_undef_strp,2,dwarf_udata_strp_form},
    {DW_MACRO_import,1,dwarf_secoffset_form},

    {DW_MACRO_define_sup,2,dwarf_udata_strp_sup_form},
    {DW_MACRO_undef_sup,2,dwarf_udata_strp_sup_form},
    {DW_MACRO_import_sup,1,dwarf_secoffset_form},

    {DW_MACRO_define_strx,2,dwarf_udata_strx_form},
    {DW_MACRO_undef_strx,2,dwarf_udata_strx_form},
};

/* Represents DWARF 5 macro info */
/* .debug_macro predefined, in order by value  */
static const struct Dwarf_Macro_OperationsList_s dwarf_default_macro_opslist = {
13, dw5formsarray 
};

static int
validate_opcode(Dwarf_Debug dbg,
   struct Dwarf_Macro_Forms_s *curform,
   Dwarf_Error * error)
{
    unsigned i = 0;
    struct Dwarf_Macro_Forms_s *stdfptr = 0;
    if (curform->mf_code >= DW_MACRO_lo_user) {
        /* Nothing to check. user level. */
        return DW_DLV_OK;
    }
    if (curform->mf_code > DW_MACRO_undef_strx) {
        _dwarf_error(dbg, error, DW_DLE_MACRO_OPCODE_BAD);
        return (DW_DLV_ERROR);
    }
    if (!curform->mf_code){
        _dwarf_error(dbg, error, DW_DLE_MACRO_OPCODE_BAD);
        return (DW_DLV_ERROR);
    }
    stdfptr = &dwarf_default_macro_opslist.mol_data[curform->mf_code];

    if (curform->mf_formcount != stdfptr->mf_formcount) {
        _dwarf_error(dbg, error, DW_DLE_MACRO_OPCODE_FORM_BAD);
        return (DW_DLV_ERROR);
    }
    for(i = 0; i < curform->mf_formcount; ++i) {
        if (curform->mf_formbytes[i] != stdfptr->mf_formbytes[1]) {
            _dwarf_error(dbg, error, DW_DLE_MACRO_OPCODE_FORM_BAD);
            return (DW_DLV_ERROR);
        }
    }
    return DW_DLV_OK;
}

static int
read_operands_table(Dwarf_Macro_Context macro_context,
     Dwarf_Small * macro_header,
     Dwarf_Small * macro_data,
     Dwarf_Small * section_base,
     Dwarf_Unsigned section_size,
     Dwarf_Error *error)
{
    Dwarf_Small* macro_data_start = macro_data;
    Dwarf_Small* table_data_start = macro_data;
    Dwarf_Unsigned local_size = 0;
    Dwarf_Unsigned cur_offset = 0;
    Dwarf_Small operand_table_count = 0;
    unsigned i = 0;
    unsigned j = 0;
    struct Dwarf_Macro_Forms_s *curformentry = 0;
    Dwarf_Debug dbg = 0;

    dbg = macro_context->mc_dbg;
    cur_offset = (1+ macro_data) - macro_header;
    if (cur_offset >= section_size) {
        _dwarf_error(dbg, error, DW_DLE_MACRO_OFFSET_BAD);
        return (DW_DLV_ERROR);
    }
    READ_UNALIGNED(dbg,operand_table_count,Dwarf_Small,
        macro_data,sizeof(Dwarf_Small));
    macro_data += sizeof(Dwarf_Small);
    /* Estimating minimum size */
    local_size = operand_table_count * 4;

    cur_offset = (local_size+ macro_data) - section_base;
    if (cur_offset >= section_size) {
        _dwarf_error(dbg, error, DW_DLE_MACRO_OFFSET_BAD);
        return (DW_DLV_ERROR);
    }
    /* first, get size of table. */
    table_data_start = macro_data;
    for (i = 0; i < operand_table_count; ++i) {
        Dwarf_Small opcode_number = 0;
        Dwarf_Unsigned formcount = 0;
        Dwarf_Word uleblen = 0;
        READ_UNALIGNED(dbg,opcode_number,Dwarf_Small,
            macro_data,sizeof(Dwarf_Small));
        macro_data += sizeof(Dwarf_Small);

        formcount = _dwarf_decode_u_leb128(macro_data,
            &uleblen);
        macro_data += uleblen;
        cur_offset = (formcount+ macro_data) - section_base;
        if (cur_offset >= section_size) {
            _dwarf_error(dbg, error, DW_DLE_MACRO_OFFSET_BAD);
            return (DW_DLV_ERROR);
        }
        macro_data += formcount;
    }
    /* reset for reread. */
    macro_data = table_data_start;
    /* allocate table */
    macro_context->mc_opcode_forms =  (struct Dwarf_Macro_Forms_s *)
        calloc(operand_table_count,
            sizeof(struct Dwarf_Macro_Forms_s));
    macro_context->mc_opcode_count = operand_table_count;
    if(!macro_context->mc_opcode_forms) {
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return DW_DLV_ERROR;
    }
  
    curformentry = macro_context->mc_opcode_forms;
    for (i = 0; i < operand_table_count; ++i,++curformentry) {
        Dwarf_Small opcode_number = 0;
        Dwarf_Unsigned formcount = 0;
        Dwarf_Word uleblen = 0;

        cur_offset = (2 + macro_data) - section_base;
        if (cur_offset >= section_size) {
            _dwarf_error(dbg, error, DW_DLE_MACRO_OFFSET_BAD);
            return (DW_DLV_ERROR);
        }
        READ_UNALIGNED(dbg,opcode_number,Dwarf_Small,
            macro_data,sizeof(Dwarf_Small));
        macro_data += sizeof(Dwarf_Small);
        formcount = _dwarf_decode_u_leb128(macro_data,
            &uleblen);
        curformentry->mf_code = opcode_number;
        curformentry->mf_formcount = formcount;
        macro_data += uleblen;
        cur_offset = (formcount+ macro_data) - section_base;
        if (cur_offset >= section_size) {
            _dwarf_error(dbg, error, DW_DLE_MACRO_OFFSET_BAD);
            return (DW_DLV_ERROR);
        }
        curformentry->mf_formbytes = macro_data;
        macro_data += formcount;
        int res = validate_opcode(macro_context->mc_dbg,curformentry, error);
        if(res != DW_DLV_OK) {
            return res;
        }
    }
    return DW_DLV_OK;
}

int
_dwarf_internal_macro_context(Dwarf_Die die,
    Dwarf_Unsigned  * version_out,
    Dwarf_Macro_Context * macro_context_out,
    Dwarf_Error * error)
{
    Dwarf_Macro_Context macro_context = 0;
    Dwarf_CU_Context   cu_context = 0;
    Dwarf_Unsigned mac_offset = 0;

    /*  The Dwarf_Debug this die belongs to. */
    Dwarf_Debug dbg = 0;
    int resattr = DW_DLV_ERROR;
    int lres = DW_DLV_ERROR;
    Dwarf_Half address_size = 0;
    int res = DW_DLV_ERROR;
    Dwarf_Unsigned macro_offset = 0;
    Dwarf_Unsigned line_table_offset = 0;
    Dwarf_Small * macro_header = 0;
    Dwarf_Small * macro_data = 0;
    Dwarf_Half version = 0;
    Dwarf_Small flags = 0;
    Dwarf_Small offset_size = 4;
    Dwarf_Unsigned cur_offset = 0;
    Dwarf_Unsigned section_size = 0;
    Dwarf_Small *section_base = 0;
    Dwarf_Attribute macro_attr = 0;

    /*  ***** BEGIN CODE ***** */
    if (error != NULL) {
        *error = NULL;
    }

    CHECK_DIE(die, DW_DLV_ERROR);
    cu_context = die->di_cu_context;
    dbg = cu_context->cc_dbg;

    res = _dwarf_load_section(dbg, &dbg->de_debug_macro,error);
    if (res != DW_DLV_OK) {
        return res;
    }
    if (!dbg->de_debug_macro.dss_size) {
        return (DW_DLV_NO_ENTRY);
    }

    address_size = _dwarf_get_address_size(dbg, die);
    resattr = dwarf_attr(die, DW_AT_macros, &macro_attr, error);
    if (resattr == DW_DLV_NO_ENTRY) {
        resattr = dwarf_attr(die, DW_AT_GNU_macros, &macro_attr, error);
    }
    if (resattr != DW_DLV_OK) {
        return resattr;
    }
    lres = dwarf_global_formref(macro_attr, &macro_offset, error);
    if (lres != DW_DLV_OK) {
        return lres;
    }
    section_base = dbg->de_debug_macro.dss_data;
    section_size = dbg->de_debug_macro.dss_size;
    /*  The '3'  ensures the header initial bytes present too. */
    if ((3+macro_offset) >= section_size) {
        _dwarf_error(dbg, error, DW_DLE_MACRO_OFFSET_BAD);
        return (DW_DLV_ERROR);
    }
    macro_header = macro_offset + section_base;
    macro_data = macro_header;
    
    macro_context = (Dwarf_Macro_Context) 
        _dwarf_get_alloc(dbg,DW_DLA_MACRO_CONTEXT,1);
    if (!macro_context) {
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return DW_DLV_ERROR;
    }

    READ_UNALIGNED(dbg,version, Dwarf_Half,
        macro_data,sizeof(Dwarf_Half));
    macro_data += sizeof(Dwarf_Half);
    READ_UNALIGNED(dbg,flags, Dwarf_Small,
        macro_data,sizeof(Dwarf_Small));
    macro_data += sizeof(Dwarf_Small);

    macro_context->mc_section_offset = macro_offset;
    macro_context->mc_version_number = version;
    macro_context->mc_flags = flags;
    macro_context->mc_dbg = dbg;
    macro_context->mc_offset_size_flag =
        flags& MACRO_OFFSET_SIZE_FLAG?TRUE:FALSE;
    macro_context->mc_debug_line_offset_flag =
        flags& MACRO_LINE_OFFSET_FLAG?TRUE:FALSE;
    macro_context->mc_operands_table_flag =
        flags& MACRO_OP_TABLE_FLAG?TRUE:FALSE;
    offset_size = macro_context->mc_offset_size_flag?8:4;
    macro_context->mc_offset_size = offset_size;
    if (macro_context->mc_debug_line_offset_flag) {
        cur_offset = (offset_size+ macro_data) - section_base;
        if (cur_offset >= section_size) {
            dwarf_dealloc_macro_context(macro_context);
            _dwarf_error(dbg, error, DW_DLE_MACRO_OFFSET_BAD);
            return (DW_DLV_ERROR);
        }
        READ_UNALIGNED(dbg,line_table_offset,Dwarf_Unsigned,
             macro_data,offset_size);
        macro_data += offset_size;
        macro_context->mc_debug_line_offset = line_table_offset;
    }
    if (macro_context->mc_operands_table_flag) {
        res = read_operands_table(macro_context,
            macro_header,
            macro_data,
            section_base,
            section_size,
            error);
        if (res != DW_DLV_OK) {
            dwarf_dealloc_macro_context(macro_context);
            return res;
        }
    }
    *version_out = version;
    *macro_context_out = macro_context;
    return DW_DLV_OK;
}

/*  The base interface to the .debug_macro section data
    for a specific CU.

    The version number passed back by *version_out
    may be 4 (a gnu extension of DWARF)  or 5. */
int
dwarf_get_macro_context(Dwarf_Die cu_die,
    Dwarf_Unsigned  * version_out,
    Dwarf_Macro_Context * macro_context,
    Dwarf_Error * error)
{
    int res = 0;

    res =  _dwarf_internal_macro_context(cu_die,version_out,
        macro_context, error);
   
    return res;
}

int dwarf_get_macro_section_name(Dwarf_Debug dbg,
   const char **sec_name_out,
   Dwarf_Error *error)
{
    struct Dwarf_Section_s *sec = 0;

    sec = &dbg->de_debug_macro;
    if (sec->dss_size == 0) {
        /* We don't have such a  section at all. */
        return DW_DLV_NO_ENTRY;
    }
    *sec_name_out = sec->dss_name;
    return DW_DLV_OK;
}

void
dwarf_dealloc_macro_context(Dwarf_Macro_Context mc)
{
     Dwarf_Debug dbg = mc->mc_dbg;
     dwarf_dealloc(dbg,mc,DW_DLA_MACRO_CONTEXT);
}

void 
_dwarf_macro_constructor(Dwarf_Debug dbg, void *m)
{
    /* Nothing to do */
    Dwarf_Macro_Context mc= (Dwarf_Macro_Context)m;
    /* Arbitrary sentinal. For debugging. */
    mc->mc_sentinal = 0xada;
    
}
void 
_dwarf_macro_destructor(void *m)
{
    Dwarf_Macro_Context mc= (Dwarf_Macro_Context)m;
    if (mc->mc_opcode_forms) {
        free(mc->mc_opcode_forms);
        mc->mc_opcode_forms = 0;
        memset(m,0,sizeof(*mc));
    }
    /* Just a recognizable sentinal. For debugging.  No real meaning. */
    mc->mc_sentinal = 0xdeadbeef;
}


