

/*  A function as this code is used twice. */
static void
update_file_entry(Dwarf_File_Entry  cur_file_entry,
    Dwarf_File_Entry *file_entries,
    Dwarf_File_Entry *prev_file_entry,
    Dwarf_Sword      *file_entry_count)
{
    if (*file_entries == NULL) {
        *file_entries = cur_file_entry;
    } else {
        (*prev_file_entry)->fi_next = cur_file_entry;
    }
    *prev_file_entry = cur_file_entry;
    (*file_entry_count)++;
}

static void
update_chain_list( Dwarf_Chain chain_line,
    Dwarf_Chain *head_chain, Dwarf_Chain *curr_chain)
{
    if (*head_chain == NULL) {
        *head_chain = chain_line;
    } else {
        (*curr_chain)->ch_next = chain_line;
    }
    *curr_chain = chain_line;
}

/*  This is included in two source files, one for normal 
    libdwarf callers and one for dwarfdump which prints
    the internals.

    This way we have just one blob of code that reads
    the table operations.  */

static void 
free_chain_entries(Dwarf_Debug dbg,Dwarf_Chain head,int count)
{
    int i = 0;
    Dwarf_Chain curr_chain = head;
    for (i = 0; i < count; i++) {
        Dwarf_Chain t = curr_chain;
        curr_chain = curr_chain->ch_next;
        dwarf_dealloc(dbg, t, DW_DLA_CHAIN);
    }
}

/*  Read one line table program. For two-level line tables, this
    function is called once for each table. */
static int
read_line_table_program(Dwarf_Debug dbg,
    Dwarf_Small *line_ptr,
    Dwarf_Small *line_ptr_end,
    Dwarf_Small *orig_line_ptr,
    struct Line_Table_Prefix_s *prefix,
    Dwarf_Line_Context line_context,
    Dwarf_Line **linebuf,
    Dwarf_Signed *linebuf_count,
    Dwarf_File_Entry *file_entries,
    Dwarf_File_Entry prev_file_entry,
    Dwarf_Sword *file_entry_count,
    Dwarf_Half address_size,
    Dwarf_Bool doaddrs, /* Only true if SGI IRIX rqs calling. */
    Dwarf_Bool dolines, 
    Dwarf_Bool is_single_table,
    Dwarf_Bool is_actuals_table,
    /*  If logicals_count > 0 we are in the actuals table
        and are being handed the logicals table already read. */
    Dwarf_Line *logicals,
    Dwarf_Signed logicals_count,
    Dwarf_Error *error,
    int *err_count_out)
{
    Dwarf_Sword i = 0;
    Dwarf_File_Entry cur_file_entry = 0;

    struct Dwarf_Line_Registers_s regs;

    /*  This is a pointer to the current line being added to the line
        matrix. */
    Dwarf_Line curr_line = 0;

    /*  These variables are used to decode leb128 numbers. Leb128_num
        holds the decoded number, and leb128_length is its length in
        bytes. */
    Dwarf_Word leb128_num = 0;
    Dwarf_Word leb128_length = 0;
    Dwarf_Sword advance_line = 0;

    /*  This is the operand of the latest fixed_advance_pc extended
        opcode. */
    Dwarf_Half fixed_advance_pc = 0;

    /*  Counts the number of lines in the line matrix. */
    Dwarf_Sword line_count = 0;

    /*  This is the length of an extended opcode instr.  */
    Dwarf_Word instr_length = 0;


    /*  Used to chain together pointers to line table entries that are
        later used to create a block of Dwarf_Line entries. */
    Dwarf_Chain chain_line = NULL;
    Dwarf_Chain head_chain = NULL;
    Dwarf_Chain curr_chain = NULL;

    /*  This points to a block of Dwarf_Lines, a pointer to which is
        returned in linebuf. */
    Dwarf_Line *block_line = 0;

    /*  Mark a line record as being DW_LNS_set_address */
    Dwarf_Bool is_addr_set = false;

    /*  Initialize the one state machine variable that depends on the
        prefix.  */
    _dwarf_set_line_table_regs_default_values(&regs,prefix->pf_default_is_stmt);

    /* Start of statement program.  */
    while (line_ptr < line_ptr_end) {
        int type = 0;
        Dwarf_Small opcode = 0;

#ifdef PRINTING_DETAILS
        dwarf_printf(dbg,
            " [0x%06" DW_PR_DSx "] ",
            (Dwarf_Signed) (line_ptr - orig_line_ptr));
#endif /* PRINTING_DETAILS */
        opcode = *(Dwarf_Small *) line_ptr;
        line_ptr++;
        /* 'type' is the output */
        WHAT_IS_OPCODE(type, opcode, prefix->pf_opcode_base,
            prefix->pf_opcode_length_table, line_ptr,
            prefix->pf_std_op_count);

        if (type == LOP_DISCARD) {
            int oc = 0;
            int opcnt = prefix->pf_opcode_length_table[opcode];

#ifdef PRINTING_DETAILS
            dwarf_printf(dbg,
                "*** DWARF CHECK: DISCARD standard opcode %d "
                "with %d operands: "
                "not understood.", opcode, opcnt);
            *err_count_out += 1;
#endif /* PRINTING_DETAILS */
            for (oc = 0; oc < opcnt; oc++) {
                /*  Read and discard operands we don't
                    understand.
                    arbitrary choice of unsigned read.
                    signed read would work as well.    */
                Dwarf_Unsigned utmp2 = 0;

                DECODE_LEB128_UWORD(line_ptr, utmp2);
#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    " %" DW_PR_DUu
                    " (0x%" DW_PR_XZEROS DW_PR_DUx ")",
                    (Dwarf_Unsigned) utmp2,
                    (Dwarf_Unsigned) utmp2);
#endif /* PRINTING_DETAILS */
            }
#ifdef PRINTING_DETAILS
            dwarf_printf(dbg,"***\n");
#endif /* PRINTING_DETAILS */
        } else if (type == LOP_SPECIAL) {
            /*  This op code is a special op in the object, no matter
                that it might fall into the standard op range in this
                compile. That is, these are special opcodes between
                opcode_base and MAX_LINE_OP_CODE.  (including
                opcode_base and MAX_LINE_OP_CODE) */
#ifdef PRINTING_DETAILS
            char special[50];
            unsigned origop = opcode;
#endif /* PRINTING_DETAILS */
            Dwarf_Unsigned operation_advance = 0;

            opcode = opcode - prefix->pf_opcode_base;
            operation_advance = (opcode / prefix->pf_line_range);

            if (prefix->pf_maximum_ops_per_instruction < 2) {
                regs.lr_address = regs.lr_address + (operation_advance *
                    prefix->pf_minimum_instruction_length);
            } else {
                regs.lr_address = regs.lr_address + 
                    (prefix->pf_minimum_instruction_length *
                    ((regs.lr_op_index + operation_advance)/
                    prefix->pf_maximum_ops_per_instruction));
                regs.lr_op_index = (regs.lr_op_index +operation_advance)%
                    prefix->pf_maximum_ops_per_instruction;
            }

            regs.lr_line = regs.lr_line + prefix->pf_line_base +
                opcode % prefix->pf_line_range;
#ifdef PRINTING_DETAILS
            sprintf(special, "Specialop %3u", origop);
            print_line_detail(dbg,special,
                opcode,line_count+1, &regs,is_single_table, is_actuals_table);
#endif /* PRINTING_DETAILS */

            if (dolines) {
                curr_line =
                    (Dwarf_Line) _dwarf_get_alloc(dbg, DW_DLA_LINE, 1);
                if (curr_line == NULL) {
                    free_chain_entries(dbg,head_chain,line_count);
                    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                    return (DW_DLV_ERROR);
                }

                /* Mark a line record as being DW_LNS_set_address */
                curr_line->li_addr_line.li_l_data.li_is_addr_set = is_addr_set;
                is_addr_set = false;

                curr_line->li_address = regs.lr_address;
                curr_line->li_addr_line.li_l_data.li_file =
                    (Dwarf_Sword) regs.lr_file;
                curr_line->li_addr_line.li_l_data.li_line =
                    (Dwarf_Sword) regs.lr_line;
                curr_line->li_addr_line.li_l_data.li_column =
                    (Dwarf_Half) regs.lr_column;
                curr_line->li_addr_line.li_l_data.li_is_stmt = 
                    regs.lr_is_stmt;
                curr_line->li_addr_line.li_l_data.li_basic_block =
                    regs.lr_basic_block;
                curr_line->li_addr_line.li_l_data.li_end_sequence =
                    curr_line->li_addr_line.li_l_data.
                    li_epilogue_begin = regs.lr_epilogue_begin;
                curr_line->li_addr_line.li_l_data.li_prologue_end =
                    regs.lr_prologue_end;
                curr_line->li_addr_line.li_l_data.li_isa = regs.lr_isa;
                curr_line->li_addr_line.li_l_data.li_discriminator = 
                    regs.lr_discriminator;
                curr_line->li_addr_line.li_l_data.li_call_context = 
                    regs.lr_call_context;
                curr_line->li_addr_line.li_l_data.li_subprogram = 
                    regs.lr_subprogram;
                curr_line->li_context = line_context;
                curr_line->li_is_actuals_table = is_actuals_table;
                line_count++;

                chain_line = (Dwarf_Chain)
                    _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
                if (chain_line == NULL) {
                    free_chain_entries(dbg,head_chain,line_count);
                    _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                    return (DW_DLV_ERROR);
                }
                chain_line->ch_item = curr_line;
                update_chain_list(chain_line,&head_chain,&curr_chain);
            }

            regs.lr_basic_block = false;
            regs.lr_prologue_end = false;
            regs.lr_epilogue_begin = false;
            regs.lr_discriminator = 0;
        } else if (type == LOP_STANDARD) {
            switch (opcode) {

            case DW_LNS_copy:{

#ifdef PRINTING_DETAILS
                print_line_detail(dbg,"DW_LNS_copy",
                    opcode,line_count+1, &regs,is_single_table, is_actuals_table);
#endif /* PRINTING_DETAILS */
                if (dolines) {
                    curr_line = (Dwarf_Line) _dwarf_get_alloc(dbg,
                        DW_DLA_LINE, 1);
                    if (curr_line == NULL) {
                        free_chain_entries(dbg,head_chain,line_count);
                        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                        return (DW_DLV_ERROR);
                    }

                    /* Mark a line record as being DW_LNS_set_address */
                    curr_line->li_addr_line.li_l_data.li_is_addr_set =
                        is_addr_set;
                    is_addr_set = false;

                    curr_line->li_address = regs.lr_address;
                    curr_line->li_addr_line.li_l_data.li_file =
                        (Dwarf_Sword) regs.lr_file;
                    curr_line->li_addr_line.li_l_data.li_line =
                        (Dwarf_Sword) regs.lr_line;
                    curr_line->li_addr_line.li_l_data.li_column =
                        (Dwarf_Half) regs.lr_column;
                    curr_line->li_addr_line.li_l_data.li_is_stmt =
                        regs.lr_is_stmt;
                    curr_line->li_addr_line.li_l_data.
                        li_basic_block = regs.lr_basic_block;
                    curr_line->li_addr_line.li_l_data.
                        li_end_sequence = regs.lr_end_sequence;
                    curr_line->li_context = line_context;
                    curr_line->li_is_actuals_table = is_actuals_table;
                    curr_line->li_addr_line.li_l_data.
                        li_epilogue_begin = regs.lr_epilogue_begin;
                    curr_line->li_addr_line.li_l_data.
                        li_prologue_end = regs.lr_prologue_end;
                    curr_line->li_addr_line.li_l_data.li_isa = regs.lr_isa;
                    curr_line->li_addr_line.li_l_data.li_discriminator = 
                        regs.lr_discriminator;
                    curr_line->li_addr_line.li_l_data.li_call_context = 
                        regs.lr_call_context;
                    curr_line->li_addr_line.li_l_data.li_subprogram = 
                        regs.lr_subprogram;
                    line_count++;

                    chain_line = (Dwarf_Chain)
                        _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
                    if (chain_line == NULL) {
                        free_chain_entries(dbg,head_chain,line_count);
                        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                        return (DW_DLV_ERROR);
                    }
                    chain_line->ch_item = curr_line;
                    update_chain_list(chain_line,&head_chain,&curr_chain);
                }

                regs.lr_basic_block = false;
                regs.lr_prologue_end = false;
                regs.lr_epilogue_begin = false;
                regs.lr_discriminator = 0;
                }
                break;
            case DW_LNS_advance_pc:{
                Dwarf_Unsigned utmp2 = 0;

                DECODE_LEB128_UWORD(line_ptr, utmp2);

#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNS_advance_pc val %"
                    DW_PR_DSd " 0x%"
                    DW_PR_XZEROS DW_PR_DUx "\n",
                    (Dwarf_Signed) (Dwarf_Word) utmp2,
                    (Dwarf_Unsigned) (Dwarf_Word) utmp2);
#endif /* PRINTING_DETAILS */
                leb128_num = (Dwarf_Word) utmp2;
                regs.lr_address = regs.lr_address +
                    prefix->pf_minimum_instruction_length *
                    leb128_num;
                }
                break;
            case DW_LNS_advance_line:{
                Dwarf_Signed stmp = 0;

                DECODE_LEB128_SWORD(line_ptr, stmp);
                advance_line = (Dwarf_Sword) stmp;

#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNS_advance_line val %" DW_PR_DSd " 0x%"
                    DW_PR_XZEROS DW_PR_DSx "\n",
                    (Dwarf_Signed) advance_line,
                    (Dwarf_Signed) advance_line);
#endif /* PRINTING_DETAILS */
                regs.lr_line = regs.lr_line + advance_line;
                }
                break;
            case DW_LNS_set_file:{
                Dwarf_Unsigned utmp2 = 0;

                DECODE_LEB128_UWORD(line_ptr, utmp2);
                regs.lr_file = (Dwarf_Word) utmp2;
#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNS_set_file  %ld\n", (long) regs.lr_file);
#endif /* PRINTING_DETAILS */
                }
                break;
            case DW_LNS_set_column:{
                Dwarf_Unsigned utmp2 = 0;

                DECODE_LEB128_UWORD(line_ptr, utmp2);
                regs.lr_column = (Dwarf_Word) utmp2;
#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNS_set_column val %" DW_PR_DSd " 0x%"
                    DW_PR_XZEROS DW_PR_DSx "\n",
                    (Dwarf_Signed) regs.lr_column,
                    (Dwarf_Signed) regs.lr_column);
#endif /* PRINTING_DETAILS */
                }
                break;
            case DW_LNS_negate_stmt:{
                regs.lr_is_stmt = !regs.lr_is_stmt;
#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNS_negate_stmt\n");
#endif /* PRINTING_DETAILS */
                }
                break;
            case DW_LNS_set_basic_block:{
                regs.lr_basic_block = true;
#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNS_set_basic_block\n");
#endif /* PRINTING_DETAILS */
                }
                break;

            case DW_LNS_const_add_pc:{
                opcode = MAX_LINE_OP_CODE - prefix->pf_opcode_base;
                if (prefix->pf_maximum_ops_per_instruction < 2) {
                    Dwarf_Unsigned operation_advance =
                        (opcode / prefix->pf_line_range);
                    regs.lr_address = regs.lr_address +
                        prefix->pf_minimum_instruction_length *
                            operation_advance;
                } else {
                    Dwarf_Unsigned operation_advance =
                        (opcode / prefix->pf_line_range);
                    regs.lr_address = regs.lr_address + 
                        prefix->pf_minimum_instruction_length *
                        ((regs.lr_op_index + operation_advance)/
                        prefix->pf_maximum_ops_per_instruction);
                    regs.lr_op_index = (regs.lr_op_index +operation_advance)%
                        prefix->pf_maximum_ops_per_instruction;
                }
#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNS_const_add_pc new address 0x%"
                    DW_PR_XZEROS DW_PR_DSx "\n",
                    (Dwarf_Signed) regs.lr_address);
#endif /* PRINTING_DETAILS */
                }
                break;
            case DW_LNS_fixed_advance_pc:{
                READ_UNALIGNED(dbg, fixed_advance_pc, Dwarf_Half,
                    line_ptr, sizeof(Dwarf_Half));
                line_ptr += sizeof(Dwarf_Half);
                regs.lr_address = regs.lr_address + fixed_advance_pc;
                regs.lr_op_index = 0;
#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNS_fixed_advance_pc val %" DW_PR_DSd
                    " 0x%" DW_PR_XZEROS DW_PR_DSx
                    " new address 0x%" DW_PR_XZEROS DW_PR_DSx "\n",
                    (Dwarf_Signed) fixed_advance_pc,
                    (Dwarf_Signed) fixed_advance_pc,
                    (Dwarf_Signed) regs.lr_address);
#endif /* PRINTING_DETAILS */
                }
                break;

                /* New in DWARF3 */
            case DW_LNS_set_prologue_end:{
                regs.lr_prologue_end = true;
                }
                break;
                /* New in DWARF3 */
            case DW_LNS_set_epilogue_begin:{
                regs.lr_epilogue_begin = true;
#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNS_set_prologue_end set true.\n");
#endif /* PRINTING_DETAILS */
                }
                break;

                /* New in DWARF3 */
            case DW_LNS_set_isa:{
                Dwarf_Unsigned utmp2 = 0;

                DECODE_LEB128_UWORD(line_ptr, utmp2);
                regs.lr_isa = utmp2;

#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNS_set_isa new value 0x%"
                    DW_PR_XZEROS DW_PR_DUx ".\n",
                    (Dwarf_Unsigned) utmp2);
#endif /* PRINTING_DETAILS */
                if (regs.lr_isa != utmp2) {
                    /*  The value of the isa did not fit in our
                        local so we record it wrong. declare an
                        error. */
                    free_chain_entries(dbg,head_chain,line_count);
                    _dwarf_error(dbg, error,
                        DW_DLE_LINE_NUM_OPERANDS_BAD);
                    return (DW_DLV_ERROR);
                }
                }
                break;

                /*  Experimental two-level line tables */
                /*  DW_LNS_set_address_from_logical and
                    DW_LNS_set_subprogram
                    share the same opcode. Disambiguate by checking
                    is_actuals_table. */
            case DW_LNS_set_subprogram:
                if (is_actuals_table) {
                    /* DW_LNS_set_address_from_logical */
                    Dwarf_Signed stmp = 0;

                    DECODE_LEB128_SWORD(line_ptr, stmp);
                    advance_line = (Dwarf_Sword) stmp;
                    regs.lr_line = regs.lr_line + advance_line;
                    if (regs.lr_line >= 1 && 
                        regs.lr_line - 1 < logicals_count) {
                        regs.lr_address = 
                            logicals[regs.lr_line - 1]->li_address;
                        regs.lr_op_index = 0;
#ifdef PRINTING_DETAILS
                        dwarf_printf(dbg,"DW_LNS_set_address_from_logical "
                            "%" DW_PR_DSd " 0x%" DW_PR_XZEROS DW_PR_DSx,
                            stmp,stmp);
                        dwarf_printf(dbg,"  newaddr="
                            " 0x%" DW_PR_XZEROS DW_PR_DUx ".\n",
                            regs.lr_address);
#endif /* PRINTING_DETAILS */
                    } else {
#ifdef PRINTING_DETAILS
                        dwarf_printf(dbg,"DW_LNS_set_address_from_logical line is "
                            "%" DW_PR_DSd " 0x%" DW_PR_XZEROS DW_PR_DSx ".\n",
                            (Dwarf_Signed)regs.lr_line, 
                            (Dwarf_Signed)regs.lr_line);
#endif /* PRINTING_DETAILS */
                    }
                } else {
                    /* DW_LNS_set_subprogram, building logicals table.  */
                    Dwarf_Unsigned utmp2 = 0;

                    regs.lr_call_context = 0;
                    DECODE_LEB128_UWORD(line_ptr, utmp2);
                    regs.lr_subprogram = (Dwarf_Word) utmp2;
#ifdef PRINTING_DETAILS
                    dwarf_printf(dbg,"DW_LNS_set_subprogram "
                        "%" DW_PR_DSd " 0x%" DW_PR_XZEROS DW_PR_DSx "\n",
                        (Dwarf_Signed)utmp2,(Dwarf_Signed)utmp2);
#endif /* PRINTING_DETAILS */
                }
                break;

                /* Experimental two-level line tables */
            case DW_LNS_inlined_call: {
                Dwarf_Signed stmp = 0;

                DECODE_LEB128_SWORD(line_ptr, stmp);
                regs.lr_call_context = line_count + stmp;
                DECODE_LEB128_UWORD(line_ptr, regs.lr_subprogram);
            
#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,"DW_LNS_inlined_call "
                    "%" DW_PR_DSd " (0x%" DW_PR_XZEROS DW_PR_DSx "),"
                    "%" DW_PR_DSd " (0x%" DW_PR_XZEROS DW_PR_DSx ")",
                    stmp,stmp,
                    (Dwarf_Signed)regs.lr_subprogram,
                    (Dwarf_Signed)regs.lr_subprogram);
                dwarf_printf(dbg,"  callcontext="
                    "%" DW_PR_DSd " (0x%" DW_PR_XZEROS DW_PR_DSx ")\n",
                    (Dwarf_Signed)regs.lr_call_context,
                    (Dwarf_Signed)regs.lr_call_context);
#endif /* PRINTING_DETAILS */
                }
                break;

                /* Experimental two-level line tables */
            case DW_LNS_pop_context: {
                Dwarf_Unsigned logical_num = regs.lr_call_context;
                Dwarf_Chain logical_chain = head_chain;
                Dwarf_Line logical_line = 0;

                if (logical_num > 0 && logical_num <= line_count) {
                    for (i = 1; i < logical_num; i++) {
                        logical_chain = logical_chain->ch_next;
                    }
                    logical_line = (Dwarf_Line) logical_chain->ch_item;
                    regs.lr_file = 
                        logical_line->li_addr_line.li_l_data.li_file;
                    regs.lr_line = 
                        logical_line->li_addr_line.li_l_data.li_line;
                    regs.lr_column = 
                        logical_line->li_addr_line.li_l_data.li_column;
                    regs.lr_discriminator = 
                        logical_line->li_addr_line.li_l_data.li_discriminator;
                    regs.lr_is_stmt = 
                        logical_line->li_addr_line.li_l_data.li_is_stmt;
                    regs.lr_call_context = 
                        logical_line->li_addr_line.li_l_data.li_call_context;
                    regs.lr_subprogram = 
                        logical_line->li_addr_line.li_l_data.li_subprogram;
#ifdef PRINTING_DETAILS
                    dwarf_printf(dbg,"DW_LNS_pop_context set from logical "
                        "%" DW_PR_DUu " (0x%" DW_PR_XZEROS DW_PR_DUx ")\n",
                        logical_num,logical_num);
                } else {
                    dwarf_printf(dbg,"DW_LNS_pop_context does nothing, logical"
                        "%" DW_PR_DUu " (0x%" DW_PR_XZEROS DW_PR_DUx ")\n",
                        logical_num,logical_num);
#endif /* PRINTING_DETAILS */
                } 
                }
                break;
            } /* End switch (opcode) */

        } else if (type == LOP_EXTENDED) {
            Dwarf_Unsigned utmp3 = 0;
            Dwarf_Small ext_opcode = 0;

            DECODE_LEB128_UWORD(line_ptr, utmp3);
            instr_length = (Dwarf_Word) utmp3;
            /*  Dwarf_Small is a ubyte and the extended opcode is a
                ubyte, though not stated as clearly in the 2.0.0 spec as
                one might hope. */
            ext_opcode = *(Dwarf_Small *) line_ptr;
            line_ptr++;
            switch (ext_opcode) {

            case DW_LNE_end_sequence:{
                regs.lr_end_sequence = true;
                if (dolines) {
                    curr_line = (Dwarf_Line)
                        _dwarf_get_alloc(dbg, DW_DLA_LINE, 1);
                    if (curr_line == NULL) {
                        free_chain_entries(dbg,head_chain,line_count);
                        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                        return (DW_DLV_ERROR);
                    }

#ifdef PRINTING_DETAILS
                    print_line_detail(dbg,"DW_LNE_end_sequence extended",
                        ext_opcode, line_count+1,&regs,
                        is_single_table, is_actuals_table);
#endif /* PRINTING_DETAILS */
                    curr_line->li_address = regs.lr_address;
                    curr_line->li_addr_line.li_l_data.li_file =
                        (Dwarf_Sword) regs.lr_file;
                    curr_line->li_addr_line.li_l_data.li_line =
                        (Dwarf_Sword) regs.lr_line;
                    curr_line->li_addr_line.li_l_data.li_column =
                        (Dwarf_Half) regs.lr_column;
                    curr_line->li_addr_line.li_l_data.li_is_stmt =
                        regs.lr_is_stmt;
                    curr_line->li_addr_line.li_l_data.
                        li_basic_block = regs.lr_basic_block;
                    curr_line->li_addr_line.li_l_data.
                        li_end_sequence = regs.lr_end_sequence;
                    curr_line->li_context = line_context;
                    curr_line->li_is_actuals_table = is_actuals_table;
                    curr_line->li_addr_line.li_l_data.
                        li_epilogue_begin = regs.lr_epilogue_begin;
                    curr_line->li_addr_line.li_l_data.
                        li_prologue_end = regs.lr_prologue_end;
                    curr_line->li_addr_line.li_l_data.li_isa = regs.lr_isa;
                    curr_line->li_addr_line.li_l_data.li_discriminator = 
                        regs.lr_discriminator;
                    curr_line->li_addr_line.li_l_data.li_call_context = 
                        regs.lr_call_context;
                    curr_line->li_addr_line.li_l_data.li_subprogram = 
                        regs.lr_subprogram;
                    line_count++;
                    chain_line = (Dwarf_Chain)
                        _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
                    if (chain_line == NULL) {
                        free_chain_entries(dbg,head_chain,line_count);
                        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                        return (DW_DLV_ERROR);
                    }
                    chain_line->ch_item = curr_line;
                    update_chain_list(chain_line,&head_chain,&curr_chain);
                }
                _dwarf_set_line_table_regs_default_values(&regs,
                    prefix->pf_default_is_stmt);
                }
                break;

            case DW_LNE_set_address:{
                READ_UNALIGNED(dbg, regs.lr_address, Dwarf_Addr,
                    line_ptr, address_size);
                /* Mark a line record as being DW_LNS_set_address */
                is_addr_set = true;
                
#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNE_set_address address 0x%"
                    DW_PR_XZEROS DW_PR_DUx "\n",
                    (Dwarf_Unsigned) regs.lr_address);
#endif /* PRINTING_DETAILS */
                if (doaddrs) {
                    /* SGI IRIX rqs processing only. */
                    curr_line = (Dwarf_Line) _dwarf_get_alloc(dbg,
                        DW_DLA_LINE, 1);
                    if (curr_line == NULL) {
                        free_chain_entries(dbg,head_chain,line_count);
                        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                        return (DW_DLV_ERROR);
                    }

                    /* Mark a line record as being DW_LNS_set_address */
                    curr_line->li_addr_line.li_l_data.li_is_addr_set =
                        is_addr_set;
                    is_addr_set = false;
                    curr_line->li_address = regs.lr_address;
                    curr_line->li_addr_line.li_offset =
                        line_ptr - dbg->de_debug_line.dss_data;
                    line_count++;
                    chain_line = (Dwarf_Chain)
                        _dwarf_get_alloc(dbg, DW_DLA_CHAIN, 1);
                    if (chain_line == NULL) {
                        free_chain_entries(dbg,head_chain,line_count);
                        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                        return (DW_DLV_ERROR);
                    }
                    chain_line->ch_item = curr_line;

                    update_chain_list(chain_line,&head_chain,&curr_chain);
                }
                regs.lr_op_index = 0;
                line_ptr += address_size;
                }
                break;

            case DW_LNE_define_file:
                if (dolines) {
                    int res = 0;
                    cur_file_entry = (Dwarf_File_Entry)
                        _dwarf_get_alloc(dbg, DW_DLA_FILE_ENTRY, 1);
                    if (cur_file_entry == NULL) {
                        free_chain_entries(dbg,head_chain,line_count);
                        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
                        return (DW_DLV_ERROR);
                    }
                    cur_file_entry->fi_file_name = (Dwarf_Small *) line_ptr;
                    res = _dwarf_check_string_valid(dbg,
                        line_ptr,line_ptr,line_ptr_end,error);
                    if (res != DW_DLV_OK) {
                        free_chain_entries(dbg,head_chain,line_count);
                        return res;
                    }
                    line_ptr = line_ptr + strlen((char *) line_ptr) + 1;
                    cur_file_entry->fi_dir_index = (Dwarf_Sword)
                        _dwarf_decode_u_leb128(line_ptr, &leb128_length);
                    line_ptr = line_ptr + leb128_length;
                    cur_file_entry->fi_time_last_mod =
                        _dwarf_decode_u_leb128(line_ptr, &leb128_length);
                    line_ptr = line_ptr + leb128_length;
                    cur_file_entry->fi_file_length =
                        _dwarf_decode_u_leb128(line_ptr, &leb128_length);
                    line_ptr = line_ptr + leb128_length;
                    update_file_entry(cur_file_entry,file_entries,
                        &prev_file_entry,file_entry_count);
#ifdef PRINTING_DETAILS
                    dwarf_printf(dbg,
                        "DW_LNE_define_file %s \n", cur_file_entry->fi_file_name);
                    dwarf_printf(dbg,
                        "    dir index %d\n", (int) cur_file_entry->fi_dir_index);
                    {
                        time_t tt3 = (time_t) cur_file_entry->fi_time_last_mod;

                        /* ctime supplies newline */
                        dwarf_printf(dbg,
                            "    last time 0x%x %s",
                            (unsigned)tt3, ctime(&tt3));
                    }
                    dwarf_printf(dbg,
                        "    file length %ld 0x%lx\n",
                        (long) cur_file_entry->fi_file_length, 
                        (unsigned long) cur_file_entry->fi_file_length);
#endif /* PRINTING_DETAILS */
                 
                }
                break;
            case DW_LNE_set_discriminator:{
                /* New in DWARF4 */
                Dwarf_Unsigned utmp2 = 0;

                DECODE_LEB128_UWORD(line_ptr, utmp2);
                regs.lr_discriminator = (Dwarf_Word) utmp2;

#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNE_set_discriminator 0x%"
                    DW_PR_XZEROS DW_PR_DUx "\n",utmp2);
#endif /* PRINTING_DETAILS */
                }
                break;
            default:{
                /*  This is an extended op code we do not know about,
                    other than we know now many bytes it is
                    and the op code and the bytes of operand. */
                Dwarf_Unsigned remaining_bytes = instr_length -1;
                if (instr_length < 1 || remaining_bytes > DW_LNE_LEN_MAX) {
                    free_chain_entries(dbg,head_chain,line_count);
                    _dwarf_error(dbg, error,
                        DW_DLE_LINE_EXT_OPCODE_BAD);
                    return (DW_DLV_ERROR);
                }

#ifdef PRINTING_DETAILS
                dwarf_printf(dbg,
                    "DW_LNE extended op 0x%x ",ext_opcode);
                dwarf_printf(dbg,
                    "Bytecount: %" DW_PR_DUu , (Dwarf_Unsigned)instr_length);
                if (remaining_bytes > 0) {
                    dwarf_printf(dbg,
                        " linedata: 0x");
                    while (remaining_bytes > 0) {
                        dwarf_printf(dbg,
                            "%02x",(unsigned char)(*(line_ptr)));
                        line_ptr++;
                        remaining_bytes--;
                    }
                }
#else /* ! PRINTING_DETAILS */
                line_ptr += remaining_bytes;
#endif /* PRINTING_DETAILS */
                }
                break;
            } /* End switch. */
        }
    }
    block_line = (Dwarf_Line *)
        _dwarf_get_alloc(dbg, DW_DLA_LIST, line_count);
    if (block_line == NULL) {
        curr_chain = head_chain;
        /*  FIXME: chain cleanup should be a function and called at
            more places in this function.  */
        free_chain_entries(dbg,head_chain,line_count);
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return (DW_DLV_ERROR);
    }

    curr_chain = head_chain;
    for (i = 0; i < line_count; i++) {
        Dwarf_Chain t = 0;
        *(block_line + i) = curr_chain->ch_item;
        t = curr_chain;
        curr_chain = curr_chain->ch_next;
        dwarf_dealloc(dbg, t, DW_DLA_CHAIN);
    }

    *linebuf = block_line;
    *linebuf_count = line_count;
    return DW_DLV_OK;
}
