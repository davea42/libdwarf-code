/*
Copyright (c) 2020, David Anderson
All rights reserved.

Redistribution and use in source and binary forms, with
or without modification, are permitted provided that the
following conditions are met:

    Redistributions of source code must retain the above
    copyright notice, this list of conditions and the following
    disclaimer.

    Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include "dwarf_incl.h"
#include "dwarf_alloc.h"
#include "dwarf_error.h"
#include "dwarf_util.h"
#include "dwarfstring.h"
#include "dwarf_rnglists.h"

#define SIZEOFT8 1
#define SIZEOFT16 2
#define SIZEOFT32 4
#define SIZEOFT64 8


/*  Used in case of error, to clean up. */
static void
free_rnglists_chain(Dwarf_Debug dbg, Dwarf_Chain head)
{
    Dwarf_Chain cur = head;
    Dwarf_Chain next = 0;

    if(!head) {
        return;
    }
    for( ;cur; cur = next) {
        next = cur->ch_next;
        if (cur->ch_item) {
            free(cur->ch_item);
            cur->ch_item = 0;
            dwarf_dealloc(dbg,cur,DW_DLA_CHAIN);
        }
    }
}

/*  Reads the header. Determines the
    various offsets, including offset
    of the next header. Does no memory
    allocations here. */
static int
internal_read_header(Dwarf_Debug dbg,
    Dwarf_Unsigned contextnum,
    Dwarf_Unsigned sectionlength,
    Dwarf_Small *data,
    Dwarf_Small *end_data,
    Dwarf_Unsigned offset,
    Dwarf_Rnglists_Context  buildhere,
    Dwarf_Unsigned *next_offset,
    Dwarf_Error *error)
{
    Dwarf_Small *startdata = data;
    Dwarf_Unsigned arealen = 0;
    int length_size = 0;
    int exten_size = 0;
    Dwarf_Unsigned version = 0;
    unsigned address_size = 0;
    unsigned segment_selector_size=  0;
    Dwarf_Unsigned offset_entry_count = 0;
    Dwarf_Unsigned localoff = 0;
    Dwarf_Unsigned lists_len = 0;

    READ_AREA_LENGTH_CK(dbg,arealen,Dwarf_Unsigned,
        data,length_size,exten_size,
        error,
        sectionlength,end_data);
    if (arealen > sectionlength) {
        dwarfstring m;
        dwarfstring_constructor(&m);
        dwarfstring_append_printf_u(&m,
            "DW_DLE_SECTION_SIZE_ERROR: A .debug_rnglists "
            "area size of 0x%x ",arealen);
        dwarfstring_append_printf_u(&m,
            "at offset 0x%x ",offset);
        dwarfstring_append_printf_u(&m,
            "is larger than the entire section size of "
            "0x%x. Corrupt DWARF.",sectionlength);
        _dwarf_error_string(dbg,error,DW_DLE_SECTION_SIZE_ERROR,
            dwarfstring_string(&m));
        dwarfstring_destructor(&m);
        return DW_DLV_ERROR;
    }

    buildhere->rc_length = arealen +length_size+exten_size;
    buildhere->rc_dbg = dbg;
    buildhere->rc_index = contextnum;
    buildhere->rc_header_offset = offset;
    buildhere->rc_offset_size = length_size;
    buildhere->rc_extension_size = exten_size;
    READ_UNALIGNED_CK(dbg,version,Dwarf_Unsigned,data,
        SIZEOFT16,error,end_data);
    if (version != DW_CU_VERSION5) {
        dwarfstring m;
        dwarfstring_constructor(&m);
        dwarfstring_append_printf_u(&m,
            "DW_DLE_VERSION_STAMP_ERROR: The version should be 5 "
            "but we find %u instead.",version);
        _dwarf_error_string(dbg,error,DW_DLE_VERSION_STAMP_ERROR,
            dwarfstring_string(&m));
        dwarfstring_destructor(&m);
        return DW_DLV_ERROR;
    }
    buildhere->rc_version = version;
    data += SIZEOFT16;

    READ_UNALIGNED_CK(dbg,address_size,unsigned,data,
        SIZEOFT8,error,end_data);
    if (version != DW_CU_VERSION5) {
        dwarfstring m;
        dwarfstring_constructor(&m);
        dwarfstring_append_printf_u(&m,
            "DW_DLE_VERSION_STAMP_ERROR: The version should be 5 "
            "but we find %u instead.",version);
        _dwarf_error_string(dbg,error,DW_DLE_VERSION_STAMP_ERROR,
            dwarfstring_string(&m));
        dwarfstring_destructor(&m);
        return DW_DLV_ERROR;
    }
    if (address_size != 4 && address_size != 8 &&
        address_size != 2) {
        dwarfstring m;
        dwarfstring_constructor(&m);
        dwarfstring_append_printf_u(&m,
            " DW_DLE_ADDRESS_SIZE_ERROR: The address size "
            "of %u is not supported.",address_size);
        _dwarf_error_string(dbg,error,DW_DLE_ADDRESS_SIZE_ERROR,
            dwarfstring_string(&m));
        dwarfstring_destructor(&m);
        return DW_DLV_ERROR;
    }
    buildhere->rc_address_size = address_size;
    data++;

    READ_UNALIGNED_CK(dbg,segment_selector_size,unsigned,data,
        SIZEOFT8,error,end_data);
    buildhere->rc_segment_selector_size = segment_selector_size;
    data++;

    READ_UNALIGNED_CK(dbg,offset_entry_count,Dwarf_Unsigned,data,
        SIZEOFT32,error,end_data);
    buildhere->rc_offset_entry_count = offset_entry_count;
    data += SIZEOFT32;
    if (offset_entry_count ){
        buildhere->rc_offsets_array = data;
    }
    localoff = data - startdata;
    lists_len = address_size *offset_entry_count;

    data += lists_len;

    buildhere->rc_offsets_off_in_sect = offset+localoff;
    buildhere->rc_first_rnglist_offset = offset+localoff+
        lists_len;
    buildhere->rc_rnglists_header = startdata;
    buildhere->rc_endaddr = startdata +buildhere->rc_length;
    buildhere->rc_past_last_rnglist_offset =
        buildhere->rc_header_offset +buildhere->rc_length;
    *next_offset =  buildhere->rc_past_last_rnglist_offset;
    return DW_DLV_OK;
}


/*  We return a pointer to an array of contexts
    (not context pointers through *cxt if
    we succeed and are returning DW_DLV_OK.
    We never return DW_DLV_NO_ENTRY here. */
static int
internal_load_rnglists_contexts(Dwarf_Debug dbg,
    Dwarf_Rnglists_Context **cxt,
    Dwarf_Unsigned *count,
    Dwarf_Error *error)
{
    Dwarf_Unsigned offset = 0;
    Dwarf_Unsigned nextoffset = 0;
    Dwarf_Small  * data = dbg->de_debug_rnglists.dss_data;
    Dwarf_Unsigned section_size = dbg->de_debug_rnglists.dss_size;
    Dwarf_Small  * startdata = data;
    Dwarf_Small  * end_data = data +section_size;
    Dwarf_Chain curr_chain = 0;
    Dwarf_Chain prev_chain = 0;
    Dwarf_Chain head_chain = 0;
    int res = 0;
    Dwarf_Unsigned chainlength = 0;
    Dwarf_Rnglists_Context *fullarray = 0;
    Dwarf_Unsigned i = 0;

    for( ; data < end_data ; ) {
        Dwarf_Rnglists_Context newcontext = 0;

        /* sizeof the context struct, not sizeof a pointer */
        newcontext = malloc(sizeof(*newcontext));
        if (!newcontext) {
            free_rnglists_chain(dbg,head_chain);
            _dwarf_error_string(dbg,error,
                DW_DLE_ALLOC_FAIL,
                "DW_DLE_ALLOC_FAIL: Allocation of "
                "Rnglists_Context failed");
            return DW_DLV_ERROR;
        }
        memset(newcontext,0,sizeof(*newcontext));
        res = internal_read_header(dbg,chainlength,
            section_size,
            data,end_data,offset,
            newcontext,&nextoffset,error);
        if (res == DW_DLV_ERROR) {
            free(newcontext);
            free_rnglists_chain(dbg,head_chain);
        }
        curr_chain = (Dwarf_Chain)
            _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
        if (curr_chain == NULL) {
            free_rnglists_chain(dbg,head_chain);
            _dwarf_error_string(dbg, error, DW_DLE_ALLOC_FAIL,
                "DW_DLE_ALLOC_FAIL: allocating Rnglists_Context"
                " chain entry");
            return DW_DLV_ERROR;
        }
        curr_chain->ch_item = newcontext;
        ++chainlength;
        if (head_chain == NULL) {
            head_chain = prev_chain = curr_chain;
        } else {
            prev_chain->ch_next = curr_chain;
            prev_chain = curr_chain;
        }
        data = startdata+nextoffset;
        offset = nextoffset;
    }
    fullarray= (Dwarf_Rnglists_Context *)malloc(
        chainlength *sizeof(Dwarf_Rnglists_Context /*pointer*/));
    if (!fullarray) {
        free_rnglists_chain(dbg,head_chain);
        _dwarf_error_string(dbg,error,
            DW_DLE_ALLOC_FAIL,"Allocation of "
            "Rnglists_Context pointer array failed");
        return DW_DLV_ERROR;
    }
    curr_chain = head_chain;
    for( i = 0; i < chainlength; ++i) {
        fullarray[i] = (Dwarf_Rnglists_Context)curr_chain->ch_item;
        curr_chain->ch_item = 0;
        prev_chain = curr_chain;
        curr_chain = curr_chain->ch_next;
        dwarf_dealloc(dbg, prev_chain, DW_DLA_CHAIN);
    }
    /*  ASSERT: the chain is entirely dealloc'd
        and the array of pointers points to
        individually malloc'd Dwarf_Rnglists_Context_s */
    *cxt = fullarray;
    *count = chainlength;
    return DW_DLV_OK;
}



/*  Loads all the .debug_rnglists[.dwo]  headers and
    returns DW_DLV_NO_ENTRY if the section
    is missing or empty.
    Intended to be done quite early and
    done exactly once.
    With DW_DLV_OK it returns the number of
    rnglists headers in the section through
    rnglists_count. */
int dwarf_load_rnglists(
    Dwarf_Debug dbg,
    Dwarf_Unsigned *rnglists_count,
    UNUSEDARG Dwarf_Error *error)
{
    int res = DW_DLV_ERROR;
    Dwarf_Rnglists_Context *cxt = 0;
    Dwarf_Unsigned count = 0;

    if (dbg->de_rnglists_context) {
        *rnglists_count = dbg->de_rnglists_context_count;
        /* already done */
        return DW_DLV_OK;
    }
    if (!dbg->de_debug_rnglists.dss_size) {
        /* nothing there. */
        return DW_DLV_NO_ENTRY;
    }
    if (!dbg->de_debug_rnglists.dss_data) {
        res = _dwarf_load_section(dbg, &dbg->de_debug_rnglists,
            error);
        if (res != DW_DLV_OK) {
            return res;
        }
    }
    res = internal_load_rnglists_contexts(dbg,&cxt,&count,error);
    if (res == DW_DLV_ERROR) {
        return res;
    }
    dbg->de_rnglists_context = cxt;
    dbg->de_rnglists_context_count = count;
    if (rnglists_count) {
        *rnglists_count = count;
    }
    return DW_DLV_OK;
}

/*  Frees the memory in use in all rnglists contexts.
    Done by dwarf_finish() if the user code fails to call this. */
void
_dwarf_dealloc_rnglists(Dwarf_Debug dbg)
{
    Dwarf_Unsigned i = 0;
    Dwarf_Rnglists_Context * rngcon = 0;

    if (!dbg->de_rnglists_context) {
        return;
    }
    rngcon = dbg->de_rnglists_context;
    for( ; i < dbg->de_rnglists_context_count; ++i,++rngcon) {
        Dwarf_Rnglists_Context con = *rngcon;
        con->rc_offsets_array = 0;
        con->rc_offset_entry_count = 0;
        free(con);
    }
    free(dbg->de_rnglists_context);
    dbg->de_rnglists_context = 0;
    dbg->de_rnglists_context_count = 0;
}

int
dwarf_get_rnglist_context(
    UNUSEDARG Dwarf_Debug dbg,
    UNUSEDARG Dwarf_Unsigned context_index,
    UNUSEDARG Dwarf_Rnglists_Context *context_out,
    UNUSEDARG Dwarf_Small **first_byte_of_context,
    UNUSEDARG Dwarf_Small **first_byte_of_entries,
    UNUSEDARG Dwarf_Small **end_of_entries,
    UNUSEDARG Dwarf_Error *err)
{
    return DW_DLV_NO_ENTRY;
}

int
dwarf_get_rnglist_offset_index_value(
    Dwarf_Debug dbg,
    Dwarf_Unsigned context_index,
    Dwarf_Unsigned offsetentry_index,
    Dwarf_Unsigned * offset_value_out,
    Dwarf_Unsigned * global_offset_value_out,
    Dwarf_Error *error)
{
    Dwarf_Rnglists_Context con = 0;
    unsigned offset_len = 0;
    Dwarf_Small *offsetptr = 0;
    Dwarf_Unsigned targetoffset = 0;

    if (!dbg->de_rnglists_context_count) {
        return DW_DLV_NO_ENTRY;
    }
    if (context_index >= dbg->de_rnglists_context_count) {
        return DW_DLV_NO_ENTRY;
    }
    con = dbg->de_rnglists_context[context_index];

    if (offsetentry_index >= con->rc_offset_entry_count) {
        return DW_DLV_NO_ENTRY;
    }
    offset_len = con->rc_offset_size;
    offsetptr = con->rc_offsets_array +
        (offsetentry_index*offset_len);
    READ_UNALIGNED_CK(dbg,targetoffset,Dwarf_Unsigned,
        offsetptr,
        offset_len,error,con->rc_endaddr);
    if (offset_value_out) {
        *offset_value_out = targetoffset;
    }
    if (global_offset_value_out) {
        *global_offset_value_out = targetoffset +
            con->rc_offsets_off_in_sect;
    }
    return DW_DLV_OK;
}

/*  Enables printing of details about the Range List Table
    Headers, one header per call. Index starting at 0.
    Returns DW_DLV_NO_ENTRY if index is too high for the table.
    A .debug_rnglists section may contain any number
    of Range List Table Headers with their details.  */
int dwarf_get_rnglist_context_basics(
    Dwarf_Debug dbg,
    Dwarf_Unsigned context_index,
    Dwarf_Unsigned * header_offset,
    Dwarf_Small    * offset_size,
    Dwarf_Small    * extension_size,
    unsigned       * version, /* 5 */
    Dwarf_Small    * address_size,
    Dwarf_Small    * segment_selector_size,
    Dwarf_Unsigned * offset_entry_count,
    Dwarf_Unsigned * offset_of_offset_array,
    Dwarf_Unsigned * offset_of_first_rangeentry,
    Dwarf_Unsigned * offset_past_last_rangeentry,
    UNUSEDARG Dwarf_Error *error)
{
    Dwarf_Rnglists_Context con = 0;
    if (!dbg->de_rnglists_context_count) {
        return DW_DLV_NO_ENTRY;
    }
    if (context_index >= dbg->de_rnglists_context_count) {
        return DW_DLV_NO_ENTRY;
    }
    con = dbg->de_rnglists_context[context_index];

    if (header_offset) {
        *header_offset = con->rc_header_offset;
    }
    if (offset_size) {
        *offset_size = con->rc_offset_size;
    }
    if (offset_size) {
        *extension_size = con->rc_extension_size;
    }
    if (version) {
        *version = con->rc_version;
    }
    if (address_size) {
        *address_size = con->rc_address_size;
    }
    if (segment_selector_size) {
        *segment_selector_size = con->rc_segment_selector_size;
    }
    if (offset_entry_count) {
        *offset_entry_count = con->rc_offset_entry_count;
    }
    if (offset_of_offset_array) {
        *offset_of_offset_array = con->rc_offsets_off_in_sect;
    }
    if (offset_of_first_rangeentry) {
        *offset_of_first_rangeentry = con->rc_first_rnglist_offset;
    }
    if (offset_past_last_rangeentry) {
        *offset_past_last_rangeentry =
            con->rc_past_last_rnglist_offset;
    }
    return DW_DLV_OK;
}

/*  entry offset is offset_of_first_rangeentry.
    Stop when the returned *next_entry_offset
    is == offset_past_last_rangentry (from
    dwarf_get_rnglist_context_plus).
    This only makes sense within those ranges.
    This retrieves raw detail from the section,
    no base values or anything are added.
    So this returns raw individual entries
    for a single rnglist header, meaning a
    a single Dwarf_Rnglists_Context.

    This interface assumes there is no
    segment selector.
    It's not clear at present how to implement a segmented
    address space in .debug_rnglists .
    */
int dwarf_get_rnglist_rle(
    Dwarf_Debug dbg,
    Dwarf_Unsigned contextnumber,
    Dwarf_Unsigned entry_offset,
    Dwarf_Unsigned endoffset,
    Dwarf_Unsigned *entrylen,
    Dwarf_Unsigned *entry_kind,
    Dwarf_Unsigned *entry_operand1,
    Dwarf_Unsigned *entry_operand2,
    Dwarf_Error *err)
{
    Dwarf_Rnglists_Context con = 0;
    Dwarf_Small *data = 0;
    Dwarf_Small *enddata = 0;
    Dwarf_Small  code = 0;
    Dwarf_Unsigned val1 = 0;
    Dwarf_Unsigned val2 = 0;
    Dwarf_Unsigned count = 0;
    unsigned leblen = 0;
    unsigned address_size = 0;

    if (!dbg->de_rnglists_context_count) {
        return DW_DLV_NO_ENTRY;
    }
    data = dbg->de_debug_rnglists.dss_data +
        entry_offset;
    enddata = dbg->de_debug_rnglists.dss_data +
        endoffset;
    if (contextnumber >= dbg->de_rnglists_context_count) {
        return DW_DLV_NO_ENTRY;
    }
    con = dbg->de_rnglists_context[contextnumber];
    address_size = con->rc_address_size;
    con = dbg->de_rnglists_context[contextnumber];
    code = *data;
    ++data;
    ++count;
    switch(code) {
    case DW_RLE_end_of_list: break;
    case DW_RLE_base_addressx:{
        DECODE_LEB128_UWORD_LEN_CK(data,val1,leblen,
            dbg,err,enddata);
        count += leblen;
        }
        break;
    case DW_RLE_startx_endx:
    case DW_RLE_startx_length:
    case DW_RLE_offset_pair: {
        DECODE_LEB128_UWORD_LEN_CK(data,val1,leblen,
            dbg,err,enddata);
        count += leblen;
        DECODE_LEB128_UWORD_LEN_CK(data,val2,leblen,
            dbg,err,enddata);
        count += leblen;
        }
        break;
    case DW_RLE_base_address: {
        READ_UNALIGNED_CK(dbg,val1, Dwarf_Unsigned,
            data,address_size,err,enddata);
        data += address_size;
        count += address_size;
        }
        break;
    case DW_RLE_start_end: {
        READ_UNALIGNED_CK(dbg,val1, Dwarf_Unsigned,
            data,address_size,err,enddata);
        data += address_size;
        count += address_size;
        READ_UNALIGNED_CK(dbg,val2, Dwarf_Unsigned,
            data,address_size,err,enddata);
        data += address_size;
        count += address_size;
        }
        break;
    case DW_RLE_start_length: {
        READ_UNALIGNED_CK(dbg,val1, Dwarf_Unsigned,
            data,address_size,err,enddata);
        data += address_size;
        count += address_size;
        DECODE_LEB128_UWORD_LEN_CK(data,val2,leblen,
            dbg,err,enddata);
        count += leblen;
        }
        break;
    default: {
        dwarfstring m;

        dwarfstring_constructor(&m);
        dwarfstring_append_printf_u(&m,
            "DW_DLE_RNGLISTS_ERROR: "
            "The rangelists entry at .debug_rnglists"
            " offset 0x%x" ,entry_offset);
        dwarfstring_append_printf_u(&m,
            " has code 0x%x which is unknown",code);
        _dwarf_error_string(dbg,err,DW_DLE_RNGLISTS_ERROR,
            dwarfstring_string(&m));
        dwarfstring_destructor(&m);
        return DW_DLV_ERROR;
        }
        break;
    }
    *entrylen = count;
    *entry_kind = code;
    *entry_operand1 = val1;
    *entry_operand2 = val2;
    return DW_DLV_OK;
}
