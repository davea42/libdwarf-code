/*
  Copyright (C) 2000-2004 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007-2012 David Anderson. All Rights Reserved.
  Portions Copyright (C) 2010-2012 SN Systems Ltd. All Rights Reserved.

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

static int
_dwarf_get_locdesc_c(Dwarf_Debug dbg,
    Dwarf_Unsigned locdesc_index,
    Dwarf_Loc_Head_c loc_head,
    Dwarf_Block * loc_block,
    Dwarf_Half address_size,
    Dwarf_Half offset_size,
    Dwarf_Small version_stamp,
    Dwarf_Addr lowpc,
    Dwarf_Addr highpc,
    Dwarf_Error * error)
{
    /* Offset of current operator from start of block. */
    Dwarf_Unsigned offset = 0;

    /* Used to chain the Dwarf_Loc_Chain_s structs. */
    Dwarf_Loc_Chain new_loc = NULL;
    Dwarf_Loc_Chain prev_loc = NULL;
    Dwarf_Loc_Chain head_loc = NULL;

    /* Count of the number of location operators. */
    Dwarf_Unsigned op_count = 0;

    /* Contiguous block of Dwarf_Loc's for Dwarf_Locdesc. */
    Dwarf_Loc *block_loc = 0;

    Dwarf_Locdesc_c locdesc = loc_head->ll_locdesc + locdesc_index;

    Dwarf_Unsigned i = 0;
    int res = 0;

    /* ***** BEGIN CODE ***** */

    offset = 0;
    op_count = 0;

    /* New loop getting Loc operators. Non DWO */
    while (offset <= loc_block->bl_len) {
        Dwarf_Unsigned nextoffset = 0;
        struct Dwarf_Loc_c_s temp_loc;

        res = _dwarf_read_loc_expr_op(dbg,loc_block,
            op_count,
            version_stamp,
            offset_size,
            address_size,
            offset,
            &nextoffset,
            &temp_loc,
            error);
        if (res == DW_DLV_ERROR) {
            return res;
        }
        if (res == DW_DLV_NO_ENTRY) {
            /* Normal end. */
            break;
        }
        op_count++;
        new_loc =
            (Dwarf_Loc_Chain) _dwarf_get_alloc(dbg, DW_DLA_LOC_CHAIN, 1);
        if (new_loc == NULL) {
            /*  Some memory may leak here.  */
            _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
            return DW_DLV_ERROR;
        }

        /* Copying all the fields. DWARF 2,3,4,5. */
        new_loc->lc_atom    = temp_loc.lr_atom;
        new_loc->lc_opnumber= temp_loc.lr_opnumber;
        new_loc->lc_number  = temp_loc.lr_number;
        new_loc->lc_number2 = temp_loc.lr_number2;
        new_loc->lc_number3 = temp_loc.lr_number3;
        new_loc->lc_offset  = temp_loc.lr_offset;
        offset = nextoffset;

        if (head_loc == NULL)
            head_loc = prev_loc = new_loc;
        else {
            prev_loc->lc_next = new_loc;
            prev_loc = new_loc;
        }
        offset = nextoffset;
    }
    block_loc =
        (Dwarf_Loc_c ) _dwarf_get_alloc(dbg, DW_DLA_LOC_C, op_count);
    if (block_loc == NULL) {
        /*  Some memory does leak here.  */
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return DW_DLV_ERROR;
    }

    new_loc = head_loc;
    for (i = 0; i < op_count; i++) {
        /* Copying only the fields needed by DWARF 2,3,4 */
        (block_loc + i)->lr_atom = new_loc->lc_atom;
        (block_loc + i)->lr_number = new_loc->lc_number;
        (block_loc + i)->lr_number2 = new_loc->lc_number2;
        (block_loc + i)->lr_number3 = new_loc->lc_number3;
        (block_loc + i)->lr_offset = new_loc->lc_offset;
        (block_loc + i)->lr_opnumber = new_loc->lc_opnumber;
        prev_loc = new_loc;
        new_loc = prev_loc->lc_next;
        dwarf_dealloc(dbg, prev_loc, DW_DLA_LOC_CHAIN);
    }
    if(hipc == 0 && lopc == 0) {
FIXME
    } else {
FIXME
    }
    locdesc->ld_lle_value = 
    locdesc->ld_cents = op_count;
    locdesc->ld_s = head_loc;
    locdesc->ld_from_loclist = loc_block->bl_from_loclist;
    locdesc->ld_section_offset = loc_block->bl_section_offset;
    locdesc->ld_lopc = lowpc;
    locdesc->ld_hipc = highpc;
    return DW_DLV_OK;
}

#define MAX_ADDR ((address_size == 8)?0xffffffffffffffffULL:0xffffffff)

static int
_dwarf_read_loc_section_dwo(Dwarf_Debug dbg,
    Dwarf_Block * return_block,
    Dwarf_Addr * lowpc,
    Dwarf_Addr * highpc,
    Dwarf_Bool *at_end,
    Dwarf_Off sec_offset,
    Dwarf_Half address_size,
    Dwarf_Error * error)
{
    Dwarf_Small *beg = dbg->de_debug_loc.dss_data + sec_offset;
    Dwarf_Small *locptr = 0;
    Dwarf_Small llecode = 0;
    Dwarf_Word leb128_length = 0;
    Dwarf_Unsigned expr_offset  = sec_offset;
    Dwarf_Unsigned expr_len = 0;

    if (sec_offset >= dbg->de_debug_loc.dss_size) {
        /* We're at the end. No more present. */
        return DW_DLV_NO_ENTRY;
    }
    memset(return_block,0,sizeof(*return_block));

    /* not the same as non-split loclist, but still a list. */
    return_block->bl_from_loclist = 2;

    llecode = *beg;
    locptr = beg +1;
    expr_offset++;
    switch(llecode) {
    case DW_LLE_end_of_list_entry:
        *at_end = TRUE;
        return_block->bl_section_offset = expr_offset;
        expr_offset++;
        break;
    case DW_LLE_base_address_selection_entry: {
        Dwarf_Unsigned addr_index = 0;

        addr_index = _dwarf_decode_u_leb128(locptr, &leb128_length);
        locptr += leb128_length;
        return_block->bl_section_offset = expr_offset;
        *lowpc=addr_index;
        }
        break;
    case DW_LLE_start_end_entry: {
        Dwarf_Unsigned addr_indexs = 0;
        Dwarf_Unsigned addr_indexe= 0;

        addr_indexs= _dwarf_decode_u_leb128(locptr, &leb128_length);
        locptr += leb128_length;
        expr_offset += leb128_length;
        addr_indexe= _dwarf_decode_u_leb128(locptr, &leb128_length);
        locptr += leb128_length;
        expr_offset +=leb128_length;

        *lowpc=addr_indexs;
        *highpc=addr_indexe;

        expr_len= _dwarf_decode_u_leb128(locptr, &leb128_length);
        locptr += leb128_length;
        expr_offset  += leb128_length;

        return_block->bl_len = expr_len;
        return_block->bl_data = locptr;
        return_block->bl_section_offset = expr_offset;

        expr_offset += expr_len;
        if (expr_offset > dbg->de_debug_loc.dss_size) {
            _dwarf_error(NULL, error, DW_DLE_DEBUG_LOC_SECTION_SHORT);
            return DW_DLV_ERROR;
        }
        }
        break;
    case DW_LLE_start_length_entry: {
        Dwarf_Unsigned addr_index = 0;
        Dwarf_ufixed  range_length = 0;
        Dwarf_Unsigned expr_len = 0;

        addr_index= _dwarf_decode_u_leb128(locptr, &leb128_length);
        locptr += leb128_length;
        expr_offset +=leb128_length;

        READ_UNALIGNED(dbg, range_length, Dwarf_ufixed, locptr,
            sizeof(range_length));
        locptr += leb128_length;
        expr_offset +=leb128_length;

        expr_len= _dwarf_decode_u_leb128(locptr, &leb128_length);
        expr_offset += leb128_length;
        locptr += leb128_length;

        *lowpc = addr_index;
        *highpc = range_length;
        return_block->bl_len = expr_len;
        return_block->bl_data = locptr;
        return_block->bl_section_offset = expr_offset;
        /* exprblock_size can be zero, means no expression */

        expr_offset += expr_len;
        if (expr_offset > dbg->de_debug_loc.dss_size) {
            _dwarf_error(NULL, error, DW_DLE_DEBUG_LOC_SECTION_SHORT);
            return DW_DLV_ERROR;
        }
        }
        break;
    case DW_LLE_offset_pair_entry: {
        Dwarf_ufixed  startoffset = 0;
        Dwarf_ufixed  endoffset = 0;
        Dwarf_Unsigned expr_len = 0;

        READ_UNALIGNED(dbg, startoffset, Dwarf_ufixed, locptr,
            sizeof(startoffset));
        locptr += sizeof(startoffset);
        expr_offset += sizeof(startoffset);

        READ_UNALIGNED(dbg, endoffset, Dwarf_ufixed, locptr,
            sizeof(endoffset));
        locptr += sizeof(endoffset);
        expr_offset += sizeof(endoffset);
        *lowpc= startoffset;
        *highpc = endoffset;

        expr_len= _dwarf_decode_u_leb128(locptr, &leb128_length);
        locptr += leb128_length;
        expr_offset  += leb128_length;

        return_block->bl_len = expr_len;
        return_block->bl_data = locptr;
        return_block->bl_section_offset = expr_offset;

        expr_offset += expr_len;
        if (expr_offset > dbg->de_debug_loc.dss_size) {
            _dwarf_error(NULL, error, DW_DLE_DEBUG_LOC_SECTION_SHORT);
            return DW_DLV_ERROR;
        }
        }
        break;
    default:
        _dwarf_error(dbg,error,DW_DLE_LLE_CODE_UNKNOWN);
        return DW_DLV_ERROR;
    }
    return DW_DLV_OK;
}


static int
_dwarf_get_loclist_count_dwo(Dwarf_Debug dbg,
    Dwarf_Off loclist_offset,
    Dwarf_Half address_size,
    int *loclist_count, Dwarf_Error * error)
{
    int count = 0;
    Dwarf_Off offset = loclist_offset;

    for (;;) {
        Dwarf_Block b;
        Dwarf_Bool at_end = FALSE;
        Dwarf_Addr lowpc = 0;
        Dwarf_Addr highpc = 0;

        int res = _dwarf_read_loc_section_dwo(dbg, &b,
            &lowpc,
            &highpc,
            &at_end,
            offset, address_size,error);
        if (res != DW_DLV_OK) {
            return res;
        }
        if (at_end) {
            /* Count the end operator too. */
            count++;
            break;
        }
        offset = b.bl_len + b.bl_section_offset;
        count++;
    }
    *loclist_count = count;
    return DW_DLV_OK;
}


/* Helper routine to avoid code duplication.
*/
static int
_dwarf_setup_loc(Dwarf_Attribute attr,
    Dwarf_Debug *     dbg_ret,
    Dwarf_CU_Context *cucontext_ret,
    Dwarf_Half       *form_ret,
    Dwarf_Error      *error)
{
    Dwarf_Debug dbg = 0;
    Dwarf_Half form = 0;
    int blkres = DW_DLV_ERROR;

    if (attr == NULL) {
        _dwarf_error(NULL, error, DW_DLE_ATTR_NULL);
        return (DW_DLV_ERROR);
    }
    if (attr->ar_cu_context == NULL) {
        _dwarf_error(NULL, error, DW_DLE_ATTR_NO_CU_CONTEXT);
        return (DW_DLV_ERROR);
    }
    *cucontext_ret = attr->ar_cu_context;

    dbg = attr->ar_cu_context->cc_dbg;
    if (dbg == NULL) {
        _dwarf_error(NULL, error, DW_DLE_ATTR_DBG_NULL);
        return (DW_DLV_ERROR);
    }
    *dbg_ret = dbg;
    blkres = dwarf_whatform(attr, &form, error);
    if (blkres != DW_DLV_OK) {
        _dwarf_error(dbg, error, DW_DLE_LOC_EXPR_BAD);
        return blkres;
    }
    *form_ret = form;
    return DW_DLV_OK;
}


/*  New October 2015
    This interface requires the use of interface functions
    to get data from Dwarf_Locdesc_c.  The structures
    are not visible to callers. */
int
dwarf_get_loclist_c(Dwarf_Attribute attr,
    Dwarf_Loc_Head_c * ll_header_out,
    Dwarf_Signed    * listlen_out,
    Dwarf_Error     * error)
{
    Dwarf_Debug dbg;

    /*  Dwarf_Attribute that describes the DW_AT_location in die, if
        present. */
    Dwarf_Attribute loc_attr = attr;

    /* Dwarf_Block that describes a single location expression. */
    Dwarf_Block loc_block;

    /* A pointer to the current Dwarf_Locdesc read. */
    Dwarf_Locdesc_c locdesc = 0;

    Dwarf_Half form = 0;
    Dwarf_Addr lowpc = 0;
    Dwarf_Addr highpc = 0;
    Dwarf_Signed     listlen = 0;
    Dwarf_Locdesc_c  llbuf = 0;
    Dwarf_Loc_Head_c llhead = 0;
    Dwarf_CU_Context cucontext = 0;
    unsigned address_size = 0;
    int cuvstamp = 0;
    Dwarf_Bool is_cu = FALSE;

    int blkres = DW_DLV_ERROR;
    int setup_res = DW_DLV_ERROR;

    /* ***** BEGIN CODE ***** */
    setup_res = _dwarf_setup_loc(attr, &dbg,&cucontext, &form, error);
    if (setup_res != DW_DLV_OK) {
        return setup_res;
    }
    cuvstamp = cucontext->cc_version_stamp;
    address_size = cucontext->cc_address_size;
    /*  If this is a form_block then it's a location expression. If it's
        DW_FORM_data4 or DW_FORM_data8  in DWARF2 or DWARF3
        (or in DWARF4 or 5 a DW_FORM_sec_offset) it's a loclist offset */
    if (((cuvstamp == DW_CU_VERSION2 || cuvstamp == DW_CU_VERSION3) &&
        (form == DW_FORM_data4 || form == DW_FORM_data8)) ||
        ((cuvstamp == DW_CU_VERSION4 || cuvstamp == DW_CU_VERSION5) &&
        form == DW_FORM_sec_offset)) {
        /* Here we have a loclist to deal with. */
        setup_res = context_is_cu_not_tu(cucontext,&is_cu,error);
        if(setup_res != DW_DLV_OK) {
            return setup_res;
        }
        if (cu_context->cc_is_dwo) {
            /*  dwo loclist. If this were a skeleton CU
                (ie, in the base, not dwo/dwp) then
                it could not have a loclist.  */
            /*  A reference to .debug_loc.dwo, with an offset 
                in .debug_loc.dwo of a loclist */
            Dwarf_Unsigned loclist_offset = 0;
            int off_res  = DW_DLV_ERROR;
            int count_res = DW_DLV_ERROR;
            int loclist_count = 0;
            int lli = 0;
    
            off_res = _dwarf_get_loclist_header_start(dbg,
                attr, &loclist_offset, error);
            if (off_res != DW_DLV_OK) {
                return off_res;
            }
            count_res = _dwarf_get_loclist_count_dwo(dbg, loclist_offset,
                address_size, &loclist_count, error);
            if (count_res != DW_DLV_OK) {
                return count_res;
            }
            listlen = loclist_count;
            if (loclist_count == 0) {
                return DW_DLV_NO_ENTRY;
            }

            llhead = (Dwarf_Loc_Head_c)_dwarf_get_alloc(dbg, 
                DW_DLA_LOC_HEAD_C, 1);
            if (!llhead) {
                _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                return (DW_DLV_ERROR);
            }
            listlen = loclist_count;
            llbuf = (Dwarf_Locdesc_c)
                _dwarf_get_alloc(dbg, DW_DLA_LOCDESC_C, listlen);
            if (!llbuf) {
                dwarf_loc_head_c_dealloc(llhead);
                _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                return (DW_DLV_ERROR);
            }
            llhead->ll_locdesc = llbuf;
            llhead->ll_locdesc_count = listlen;
            llhead->ll_from_loclist = 2;
            llhead->ll_context = cucontext;
    
            /* New get loc ops, DWO version */
            for (lli = 0; lli < listlen; ++lli) {
                int lres = 0;
                blkres = _dwarf_read_loc_section_dwo(dbg, &loc_block,
                    &lowpc,
                    &highpc,
                    loclist_offset,
                    address_size,
                    error);
                if (blkres != DW_DLV_OK) {
                    dwarf_loc_head_c_dealloc(llhead);
                    return blkres;
                }
                /* Fills in the locdesc at index lli */
                lres = _dwarf_get_locdesc_c(dbg, &loc_block,
                    lli,
                    llhead,
                    address_size,
                    cucontext->cc_length_size,
                    cucontext->cc_version_stamp,
                    lowpc, highpc,
                    error);
                if (lres != DW_DLV_OK) {
                    dwarf_loc_head_c_dealloc(llhead);
                    /* low level error already set: let it be passed back */
                    return lres;
                }
    
                /* Now get to next loclist entry offset. */
                loclist_offset = loc_block.bl_section_offset +
                    loc_block.bl_len;
            }
        } 
        /*  Non-dwo loclist. If this were a skeleton CU
            (ie, in the base, not dwo/dwp) then
            it could not have a loclist.  */
        /*  A reference to .debug_loc, with an offset in .debug_loc of a
            loclist */
        Dwarf_Unsigned loclist_offset = 0;
        int off_res  = DW_DLV_ERROR;
        int count_res = DW_DLV_ERROR;
        int loclist_count = 0;
        int lli = 0;

        off_res = _dwarf_get_loclist_header_start(dbg,
            attr, &loclist_offset, error);
        if (off_res != DW_DLV_OK) {
            return off_res;
        }
        count_res = _dwarf_get_loclist_count(dbg, loclist_offset,
            address_size, &loclist_count, error);
        listlen = loclist_count;
        if (count_res != DW_DLV_OK) {
            return count_res;
        }
        if (loclist_count == 0) {
            return DW_DLV_NO_ENTRY;
        }
        llhead = (Dwarf_Loc_Head_c)_dwarf_get_alloc(dbg, 
            DW_DLA_LOC_HEAD_C, 1);
        if (!llhead) {
            _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
            return (DW_DLV_ERROR);
        }
        listlen = loclist_count;
        llbuf = (Dwarf_Locdesc_c)
            _dwarf_get_alloc(dbg, DW_DLA_LOCDESC_C, listlen);
        if (!llbuf) {
            dwarf_loc_head_c_dealloc(llhead);
            _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
            return (DW_DLV_ERROR);
        }
        llhead->ll_locdesc = llbuf;
        llhead->ll_locdesc_count = listlen;
        llhead->ll_from_loclist = 1;
        llhead->ll_context = cucontext;

        /* New locdex and Loc,  non-DWO */
        for (lli = 0; lli < listlen; ++lli) {
            int lres = 0;
            blkres = _dwarf_read_loc_section(dbg, &loc_block,
                &lowpc,
                &highpc,
                loclist_offset,
                address_size,
                error);
            if (blkres != DW_DLV_OK) {
                dwarf_loc_head_c_dealloc(llhead);
                return (blkres);
            }
            /* Fills in the locdesc at index lli */
            lres = _dwarf_get_locdesc_c(dbg, &loc_block,
                lli,
                llhead,
                address_size,
                cucontext->cc_length_size,
                cucontext->cc_version_stamp,
                lowpc, highpc,
                error);
            if (lres != DW_DLV_OK) {
                dwarf_loc_head_c_dealloc(llhead);
                /* low level error already set: let it be passed back */
                return lres;
            }

            /* Now get to next loclist entry offset. */
            loclist_offset = loc_block.bl_section_offset +
                loc_block.bl_len;
        }
    } else {
        llhead = (Dwarf_Loc_Head_c)
            _dwarf_get_alloc(dbg, DW_DLA_LOC_HEAD_C, 1);
        if (!llhead) {
            _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
            return (DW_DLV_ERROR);
        }
        if( form == DW_FORM_exprloc) {
            blkres = dwarf_formexprloc(loc_attr,&loc_block.bl_len,
                &loc_block.bl_data,error);
            if(blkres != DW_DLV_OK) {
                dwarf_loc_head_c_dealloc(llhead);
                return blkres;
            }
            loc_block.bl_from_loclist = 0;
            loc_block.bl_section_offset  =
                (char *)loc_block.bl_data -
                (char *)dbg->de_debug_info.dss_data;
        } else {
            Dwarf_Block *tblock = 0;
            blkres = dwarf_formblock(loc_attr, &tblock, error);
            if (blkres != DW_DLV_OK) {
                dwarf_loc_head_c_dealloc(llhead);
                return (blkres);
            }
            loc_block = *tblock;
            /*  We copied tblock contents to the stack var, so can dealloc
                tblock now.  Avoids leaks. */
            dwarf_dealloc(dbg, tblock, DW_DLA_BLOCK);
        }
        listlen = 1; /* One by definition of a location entry. */
        lowpc = 0;   /* HACK */
        highpc = (Dwarf_Unsigned) (-1LL); /* HACK */
        llbuf = (Dwarf_Locdesc_c)
            _dwarf_get_alloc(dbg, DW_DLA_LOCDESC_C, listlen);
        if (!llbuf) {
            dwarf_loc_head_c_dealloc(llhead);
            _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
            return (DW_DLV_ERROR);
        }
        llhead->ll_locdesc = llbuf;
        llhead->ll_locdesc_count = listlen;
        llhead->ll_from_loclist = 0;
        llhead->ll_context = cucontext;

        /*  An empty location description (block length 0) means the
            code generator emitted no variable, the variable was not
            generated, it was unused or perhaps never tested after being
            set. Dwarf2, section 2.4.1 In other words, it is not an
            error, and we don't test for block length 0 specially here. */
        /* Fills in the locdesc at index 0 */
        blkres = _dwarf_get_locdesc_c(dbg, &loc_block,
            0, /* fake locdesc is index 0 */
            llhead,
            address_size,
            cucontext->cc_length_size,
            cucontext->cc_version_stamp,
            lowpc, highpc,
            error);
        if (blkres != DW_DLV_OK) {
            dwarf_loc_head_c_dealloc(llhead);
            /* low level error already set: let it be passed back */
            return blkres;
        }
    }

    *llhead_out = llhead;
    *listlen_out = listlen;
    return (DW_DLV_OK);
}

