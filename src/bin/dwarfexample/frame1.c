/*
Copyright (c) 2009-2021 David Anderson.  All rights reserved.
This example code is hereby placed in the Public Domain
for anyone to use for any purpose.
*/

/*  frame1.c
    It specifically calls dwarf_expand_frame_instructions()
    to verify that works without crashing!

    To use, try
        make
        ./frame1 frame1

    gcc/clang may produce .eh_frame without .debug_frame.
    To read .eh_frame call dwarf_get_fde_list_eh()
    below instead of dwarf_get_fde_list() .
*/

#include <config.h>

#include <stddef.h> /* NULL */
#include <stdio.h>  /* printf() snprintf() */
#include <stdlib.h> /* exit() free() malloc() */
#include <string.h> /* strcat() strcmp() strcpy() strncmp() */

#ifdef _WIN32
#include <io.h> /* close() open() */
#elif defined HAVE_UNISTD_H
#include <unistd.h> /* close() */
#endif /* _WIN32 */

#ifdef HAVE_FCNTL_H
#include <fcntl.h> /* open() O_RDONLY */
#endif /* HAVE_FCNTL_H */

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"

#ifdef _O_BINARY
/*  This is for a Windows environment */
#define O_BINARY _O_BINARY
#else
# ifndef O_BINARY
# define O_BINARY 0  /* So it does nothing in Linux/Unix */
# endif
#endif /* O_BINARY */

static void read_frame_data(Dwarf_Debug dbg,const char *sec);
static void print_fde_instrs(Dwarf_Debug dbg, Dwarf_Fde fde,
    Dwarf_Error *error);
static void print_regtable(Dwarf_Regtable3 *tab3);
static void print_cie_instrs(Dwarf_Cie cie,Dwarf_Error *error);
static void print_fde_selected_regs( Dwarf_Fde fde);
static void print_reg(int r);

static int just_print_selected_regs = 0;

/*  Depending on the ABI we set INITIAL_VAL
    differently.  For ia64 initial value is
    UNDEF_VAL, for MIPS and others initial
    value is SAME_VAL.
    Here we'll set it UNDEF_VAL
    as that way we'll see when first set. */
#define UNDEF_VAL 2000
#define SAME_VAL 2001
#define CFA_VAL 2002
#define INITIAL_VAL UNDEF_VAL

/* Dumping a dwarf-expression as a byte stream. */
static void
dump_block(char *prefix, Dwarf_Small *data, Dwarf_Unsigned len)
{
    Dwarf_Small *end_data = data + len;
    Dwarf_Small *cur = data;
    int i = 0;

    printf("%s", prefix);
    for (; cur < end_data; ++cur, ++i) {
        if (i > 0 && i % 4 == 0)
            printf(" ");
        printf("%02x", 0xff & *cur);

    }
}

/*  Don't really want getopt here. yet.
    so hand do options. */

int
main(int argc, char **argv)
{

    Dwarf_Debug dbg = 0;
    int fd = -1;
    const char *filepath = "<stdin>";
    int res = DW_DLV_ERROR;
    Dwarf_Error error;
    Dwarf_Handler errhand = 0;
    Dwarf_Ptr errarg = 0;
    int regtabrulecount = 0;
    int curopt = 0;

    for (curopt = 1;curopt < argc; ++curopt) {
        if (strncmp(argv[curopt],"--",2)) {
            break;
        }
        if (!strcmp(argv[curopt],"--just-print-selected-regs")) {
            just_print_selected_regs++;
            continue;
        }
        if (!strcmp(argv[curopt],"--suppress-de-alloc-tree")) {
            /* Do nothing, ignore the argument */
            continue;
        }
    }

    if (curopt == argc ) {
        fd = 0; /* stdin */
    } else if (curopt == (argc-1)) {
        filepath = argv[curopt];
        fd = open(filepath,O_RDONLY|O_BINARY);
        if (fd < 0) {
            printf("Unable to open %s, giving up.\n",filepath);
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Too many args, giving up. \n");
        exit(EXIT_FAILURE);
    }
    res = dwarf_init_b(fd,DW_GROUPNUMBER_ANY,
        errhand,errarg, &dbg,&error);
    if (res != DW_DLV_OK) {
        printf("Giving up, dwarf_init failed, "
            "cannot do DWARF processing\n");
        if (res == DW_DLV_ERROR) {
            printf("Error code %s\n",dwarf_errmsg(error));
        }
        dwarf_dealloc_error(dbg,error);
        exit(EXIT_FAILURE);
    }
    /*  Do this setting after init before any real operations.
        These return the old values, but here we do not
        need to know the old values.  The sizes and
        values here are higher than most ABIs and entirely
        arbitrary.

        The setting of initial_value to
        the same as undefined-value (the other possible choice being
        same-value) is arbitrary, different ABIs do differ, and
        you have to know which is right.

        In dwarfdump we get the SAME_VAL, UNDEF_VAL,
        INITIAL_VAL CFA_VAL from dwconf_s struct.   */
    regtabrulecount=1999;
    dwarf_set_frame_undefined_value(dbg, UNDEF_VAL);
    dwarf_set_frame_rule_initial_value(dbg, INITIAL_VAL);
    dwarf_set_frame_same_value(dbg,SAME_VAL);
    dwarf_set_frame_cfa_value(dbg,CFA_VAL);
    dwarf_set_frame_rule_table_size(dbg,regtabrulecount);

    read_frame_data(dbg,".debug_frame");
    read_frame_data(dbg,".eh_frame");
    res = dwarf_finish(dbg);
    if (res != DW_DLV_OK) {
        printf("dwarf_finish failed!\n");
    }
    close(fd);
    return 0;
}

static void
read_frame_data(Dwarf_Debug dbg,const char *sect)
{
    Dwarf_Error error;
    Dwarf_Signed cie_element_count = 0;
    Dwarf_Signed fde_element_count = 0;
    Dwarf_Cie *cie_data = 0;
    Dwarf_Fde *fde_data = 0;
    int res = DW_DLV_ERROR;
    Dwarf_Signed fdenum = 0;

    /*  If you wish to read .eh_frame data, use
        dwarf_get_fde_list_eh() instead.
        Get debug_frame with dwarf_get_fde_list. */
    printf(" Print %s\n",sect);
    if (!strcmp(sect,".eh_frame"))
    {
        res = dwarf_get_fde_list_eh(dbg,&cie_data,&cie_element_count,
            &fde_data,&fde_element_count,&error);
    } else {
        res = dwarf_get_fde_list(dbg,&cie_data,&cie_element_count,
            &fde_data,&fde_element_count,&error);
    }
    if (res == DW_DLV_NO_ENTRY) {
        printf("No %s data present\n",sect);
        return;
    }
    if ( res == DW_DLV_ERROR) {
        printf("Error reading frame data ");
        exit(EXIT_FAILURE);
    }
    printf( "%" DW_PR_DSd " cies present. "
        "%" DW_PR_DSd " fdes present. \n",
        cie_element_count,fde_element_count);
    /*if (fdenum >= fde_element_count) {
        printf("Want fde %d but only %" DW_PR_DSd " present\n",fdenum,
            fde_element_count);
        exit(EXIT_FAILURE);
    }*/

    for (fdenum = 0; fdenum < fde_element_count; ++fdenum) {
        Dwarf_Cie cie = 0;

        res = dwarf_get_cie_of_fde(fde_data[fdenum],&cie,&error);
        if (res != DW_DLV_OK) {
            printf("Error accessing cie of fdenum %" DW_PR_DSd
                " to get its cie\n",fdenum);
            exit(EXIT_FAILURE);
        }
        printf("Print cie of fde %" DW_PR_DSd  "\n",fdenum);
        print_cie_instrs(cie,&error);
        printf("Print fde %" DW_PR_DSd  "\n",fdenum);
        if (just_print_selected_regs) {
            print_fde_selected_regs(fde_data[fdenum]);
        } else {
            print_fde_instrs(dbg,fde_data[fdenum],&error);
        }
    }

    /* Done with the data. */
    dwarf_dealloc_fde_cie_list(dbg,cie_data,cie_element_count,
        fde_data, fde_element_count);
    return;
}

/* Simply shows the instructions at hand for this fde. */
static void
print_cie_instrs(Dwarf_Cie cie,Dwarf_Error *error)
{
    int res = DW_DLV_ERROR;
    Dwarf_Unsigned bytes_in_cie = 0;
    Dwarf_Small version = 0;
    char *augmentation = 0;
    Dwarf_Unsigned code_alignment_factor = 0;
    Dwarf_Signed data_alignment_factor = 0;
    Dwarf_Half   return_address_register_rule = 0;
    Dwarf_Small   *instrp = 0;
    Dwarf_Unsigned instr_len = 0;
    Dwarf_Half offset_size = 0;

    res = dwarf_get_cie_info_b(cie,&bytes_in_cie,
        &version, &augmentation, &code_alignment_factor,
        &data_alignment_factor, &return_address_register_rule,
        &instrp,&instr_len,&offset_size,error);
    if (res != DW_DLV_OK) {
        printf("Unable to get cie info!\n");
        exit(EXIT_FAILURE);
    }
}

static void
print_fde_col(Dwarf_Signed k,
    Dwarf_Addr   jsave,
    Dwarf_Small  value_type,
    Dwarf_Signed offset_relevant,
    Dwarf_Signed reg_used,
    Dwarf_Signed offset,
    Dwarf_Block *block,
    Dwarf_Addr   row_pc,
    Dwarf_Bool   has_more_rows,
    Dwarf_Addr   subsequent_pc)
{
    char *type_title = "";
    Dwarf_Unsigned rule_id = k;

    printf(" pc=0x%" DW_PR_DUx ,jsave);
    if (row_pc != jsave) {
        printf(" row_pc=0x%" DW_PR_DUx ,row_pc);
    }
    printf(" col=%" DW_PR_DSd " ",k);
    switch(value_type) {
    case DW_EXPR_OFFSET:
        type_title = "off";
        goto preg2;
    case DW_EXPR_VAL_OFFSET:
        type_title = "valoff";

        preg2:
        printf("<%s ", type_title);
        if (reg_used == SAME_VAL) {
            printf(" SAME_VAL");
            break;
        } else if (reg_used == INITIAL_VAL) {
            printf(" INITIAL_VAL");
            break;
        }
        print_reg(rule_id);

        printf("=");
        if (offset_relevant == 0) {
            print_reg(reg_used);
            printf(" ");
        } else {
            printf("%02" DW_PR_DSd , offset);
            printf("(");
            print_reg(reg_used);
            printf(") ");
        }
        break;
    case DW_EXPR_EXPRESSION:
        type_title = "expr";
        goto pexp2;
    case DW_EXPR_VAL_EXPRESSION:
        type_title = "valexpr";

        pexp2:
        printf("<%s ", type_title);
        print_reg(rule_id);
        printf("=");
        printf("expr-block-len=%" DW_PR_DUu , block->bl_len);
        {
            char pref[40];

            strcpy(pref, "<");
            strcat(pref, type_title);
            strcat(pref, "bytes:");
            /*  The data being dumped comes direct from
                libdwarf so libdwarf validated it. */
            dump_block(pref, block->bl_data, block->bl_len);
            printf("%s", "> \n");
#if 0
            if (glflags.verbose) {
                struct esb_s exprstring;
                esb_constructor(&exprstring);
                get_string_from_locs(dbg,
                    block_ptr,offset,addr_size,
                    offset_size,version,&exprstring);
                printf("<expr:%s>",esb_get_string(&exprstring));
                esb_destructor(&exprstring);
            }
#endif
        }
        break;
    default:
        printf("Internal error in libdwarf, value type %d\n",
            value_type);
        exit(EXIT_FAILURE);
    }
    printf(" more=%d",has_more_rows);
    printf(" next=0x%" DW_PR_DUx,subsequent_pc);
    printf("%s", "> ");
    printf("\n");
}

/*  In dwarfdump we use
    dwarf_get_fde_info_for_cfa_reg3_b() to get subsequent pc
    and avoid incrementing pc by for the next cfa,
    using has_more_rows and subsequent_pc passed back.

    Here, to verify function added in May 2018,
    we instead use dwarf_get_fde_info_for_reg3_b()
    which has the has_more_rows and subsequent_pc functions
    for the case where one is tracking a particular register
    and not closely watching the CFA value itself. */

static const Dwarf_Block dwblockzero;
static void
print_fde_selected_regs( Dwarf_Fde fde)
{
    Dwarf_Error oneferr = 0;
    /* Arbitrary column numbers for testing. */
    static int selected_cols[] = {1,3,5};
    static int selected_cols_count =
        sizeof(selected_cols)/sizeof(selected_cols[0]);
    Dwarf_Signed k = 0;
    int fres = 0;

    Dwarf_Addr low_pc = 0;
    Dwarf_Unsigned func_length = 0;
    Dwarf_Small *fde_bytes = NULL;
    Dwarf_Unsigned fde_bytes_length = 0;
    Dwarf_Off cie_offset = 0;
    Dwarf_Signed cie_index = 0;
    Dwarf_Off fde_offset = 0;
    Dwarf_Fde curfde = fde;
    Dwarf_Cie cie = 0;
    Dwarf_Addr jsave = 0;
    Dwarf_Addr high_addr = 0;
    Dwarf_Addr next_jsave = 0;
    Dwarf_Bool has_more_rows = 0;
    Dwarf_Addr subsequent_pc = 0;
    Dwarf_Error error = 0;
    int res = 0;

    fres = dwarf_get_fde_range(curfde,
        &low_pc, &func_length,
        &fde_bytes,
        &fde_bytes_length,
        &cie_offset, &cie_index,
        &fde_offset, &oneferr);

    if (fres == DW_DLV_ERROR) {
        printf("FAIL: dwarf_get_fde_range err %" DW_PR_DUu
            " line %d\n",
            dwarf_errno(oneferr),__LINE__);
        exit(EXIT_FAILURE);
    }
    if (fres == DW_DLV_NO_ENTRY) {
        printf("No fde range data available\n");
        return;
    }
    res = dwarf_get_cie_of_fde(fde,&cie,&error);
    if (res != DW_DLV_OK) {
        printf("Error getting cie from fde\n");
        exit(EXIT_FAILURE);
    }

    high_addr = low_pc + func_length;
    /*  Could check has_more_rows here instead of high_addr,
        If we initialized has_more_rows to 1 above. */
    for (jsave = low_pc ; next_jsave < high_addr;
        jsave = next_jsave) {
        next_jsave = jsave+1;
        printf("\n");
        for (k = 0; k < selected_cols_count ; ++k ) {
            Dwarf_Unsigned reg = 0;
            Dwarf_Unsigned offset_relevant = 0;
            int fires = 0;
            Dwarf_Small value_type = 0;
            Dwarf_Block block;
            Dwarf_Unsigned offset;
            Dwarf_Addr row_pc = 0;

            block = dwblockzero;
            fires = dwarf_get_fde_info_for_reg3_b(curfde,
                selected_cols[k],
                jsave,
                &value_type,
                &offset_relevant,
                &reg,
                &offset,
                &block,
                &row_pc,
                &has_more_rows,
                &subsequent_pc,
                &oneferr);
            if (fires == DW_DLV_ERROR) {
                printf("FAIL: reading reg err %" DW_PR_DUu " line %d",
                    dwarf_errno(oneferr),__LINE__);
                exit(EXIT_FAILURE);
            }
            if (fires == DW_DLV_NO_ENTRY) {
                continue;
            }
            print_fde_col(
                selected_cols[k],jsave,
                value_type,offset_relevant,
                reg,offset,&block,row_pc,
                has_more_rows, subsequent_pc);
            if (has_more_rows) {
                next_jsave = subsequent_pc;
            } else {
                next_jsave = high_addr;
            }
        }
    }
}

static int
print_frame_instrs(Dwarf_Debug dbg,
    Dwarf_Frame_Instr_Head frame_instr_head,
    Dwarf_Unsigned frame_instr_count,
    Dwarf_Error *error)
{
    Dwarf_Unsigned i = 0;

    printf("\nPrint %" DW_PR_DUu " frame instructions\n",
        frame_instr_count);
    for ( ; i < frame_instr_count; ++i) {
        int res = 0;
        Dwarf_Unsigned  instr_offset_in_instrs = 0;
        Dwarf_Small     cfa_operation = 0;
        const char     *fields= 0;
        Dwarf_Unsigned  u0 = 0;
        Dwarf_Unsigned  u1 = 0;
        Dwarf_Signed    s0 = 0;
        Dwarf_Signed    s1 = 0;
        Dwarf_Block     expression_block;
        Dwarf_Unsigned  code_alignment_factor = 0;
        Dwarf_Signed    data_alignment_factor = 0;
        const char     *op_name = 0;

        expression_block = dwblockzero;
        res = dwarf_get_frame_instruction(frame_instr_head,
            i,&instr_offset_in_instrs, &cfa_operation,
            &fields, &u0,&u1,&s0,&s1,
            &code_alignment_factor,
            &data_alignment_factor,
            &expression_block,error);
        if (res != DW_DLV_OK) {
            if (res == DW_DLV_ERROR) {
                printf("ERROR reading frame instruction "
                    "%" DW_PR_DUu "\n",
                    frame_instr_count);
                dwarf_dealloc_error(dbg,*error);
                *error = 0;
            } else {
                printf("NO ENTRY reading frame instruction "
                    " %" DW_PR_DUu "\n",frame_instr_count);
            }
            break;
        }
        dwarf_get_CFA_name(cfa_operation,&op_name);
        printf("[%2" DW_PR_DUu "]  %" DW_PR_DUu " %s ",i,
            instr_offset_in_instrs,op_name);
        switch(fields[0]) {
        case 'u': {
            if (!fields[1]) {
                printf("%" DW_PR_DUu "\n",u0);
            }
            if (fields[1] == 'c') {
                Dwarf_Unsigned final =
                    u0*code_alignment_factor;
                printf("%" DW_PR_DUu ,final);
#if 0
                if (glflags.verbose) {
                    printf("  (%" DW_PR_DUu " * %" DW_PR_DUu,
                        u0,code_alignment_factor);

                }
#endif
                printf("\n");
            }
        }
        break;
        case 'r': {
            if (!fields[1]) {
                printf("r%" DW_PR_DUu "\n",u0);
                break;
            }
            if (fields[1] == 'u') {
                if (!fields[2]) {
                    printf("%" DW_PR_DUu ,u1);
                    printf("\n");
                    break;
                }
                if (fields[2] == 'd') {
                    Dwarf_Signed final =
                        (Dwarf_Signed)u0 *
                        data_alignment_factor;
                    printf("%" DW_PR_DUu ,final);
                    printf("\n");
                }
            }
            if (fields[1] == 'r') {
                printf("r%" DW_PR_DUu "\n",u0);
                printf(" ");
                printf("r%" DW_PR_DUu "\n",u1);
                printf("\n");
            }
            if (fields[1] == 's') {
                if (fields[2] == 'd') {
                    Dwarf_Signed final = s1 * data_alignment_factor;
                    printf("r%" DW_PR_DUu "\n",u0);
                    printf("%" DW_PR_DSd , final);
#if 0
                    if (glflags.verbose) {
                        printf("  (%" DW_PR_DSd " * %" DW_PR_DSd,
                            s1,data_alignment_factor);
                    }
#endif
                    printf("\n");
                }
            }
            if (fields[1] == 'b') {
                /* rb */
                printf("r%" DW_PR_DUu "\n",u0);
                printf("%" DW_PR_DUu  ,u0);
                printf(" expr block len %" DW_PR_DUu "\n",
                    expression_block.bl_len);
                dump_block("    ", expression_block.bl_data,
                    (Dwarf_Signed) expression_block.bl_len);
                printf("\n");
#if 0
                if (glflags.verbose) {
                    print_expression(dbg,die,&expression_block,
                        addr_size,offset_size,
                        version);
                }
#endif
            }
        }
        break;
        case 's': {
            if (fields[1] == 'd') {
                Dwarf_Signed final = s0*data_alignment_factor;

                printf(" %" DW_PR_DSd ,final);
#if 0
                if (glflags.verbose) {
                    printf("  (%" DW_PR_DSd " * %" DW_PR_DSd,
                        s0,data_alignment_factor);
                }
#endif
                printf("\n");
            }
        }
        break;
        case 'b': {
            if (!fields[1]) {
                printf(" expr block len %" DW_PR_DUu "\n",
                    expression_block.bl_len);
                dump_block("    ", expression_block.bl_data,
                    (Dwarf_Signed) expression_block.bl_len);
                printf("\n");
#if 0
                if (glflags.verbose) {
                    print_expression(dbg,die,&expression_block,
                        addr_size,offset_size,
                        version);
                }
#endif
            }
        }
        break;
        case 0:
            printf("\n");
        break;
        default:
            printf("UNKNOWN FIELD 0x%x\n",fields[0]);
        }
    }
    return DW_DLV_OK;
}

/* Just prints the instructions in the fde. */
static void
print_fde_instrs(Dwarf_Debug dbg,
    Dwarf_Fde fde, Dwarf_Error *error)
{
    int res;
    Dwarf_Addr lowpc = 0;
    Dwarf_Unsigned func_length = 0;
    Dwarf_Small * fde_bytes;
    Dwarf_Unsigned fde_byte_length = 0;
    Dwarf_Off cie_offset = 0;
    Dwarf_Signed cie_index = 0;
    Dwarf_Off fde_offset = 0;
    Dwarf_Addr arbitrary_addr = 0;
    Dwarf_Addr actual_pc = 0;
    Dwarf_Regtable3 tab3;
    int oldrulecount = 0;
    Dwarf_Small  *outinstrs = 0;
    Dwarf_Unsigned instrslen = 0;
    Dwarf_Cie cie = 0;

    res = dwarf_get_fde_range(fde,&lowpc,&func_length,&fde_bytes,
        &fde_byte_length,&cie_offset,&cie_index,&fde_offset,error);
    if (res != DW_DLV_OK) {
        printf("Problem getting fde range \n");
        exit(EXIT_FAILURE);
    }

    arbitrary_addr = lowpc + (func_length/2);
    printf("function low pc 0x%" DW_PR_DUx
        "  and length 0x%" DW_PR_DUx
        "  and midpoint addr we choose 0x%" DW_PR_DUx
        "\n",
        lowpc,func_length,arbitrary_addr);

    /*  1 is arbitrary. We are winding up getting the
        rule count here while leaving things unchanged. */
    oldrulecount = dwarf_set_frame_rule_table_size(dbg,1);
    dwarf_set_frame_rule_table_size(dbg,oldrulecount);

    tab3.rt3_reg_table_size = oldrulecount;
    tab3.rt3_rules = (struct Dwarf_Regtable_Entry3_s *) malloc(
        sizeof(struct Dwarf_Regtable_Entry3_s)* oldrulecount);
    if (!tab3.rt3_rules) {
        printf("Unable to malloc for %d rules\n",oldrulecount);
        exit(EXIT_FAILURE);
    }

    res = dwarf_get_fde_info_for_all_regs3(fde,arbitrary_addr ,
        &tab3,&actual_pc,error);
    printf("function actual addr of row 0x%" DW_PR_DUx "\n",
        actual_pc);

    if (res != DW_DLV_OK) {
        printf("dwarf_get_fde_info_for_all_regs3 failed!\n");
        exit(EXIT_FAILURE);
    }
    print_regtable(&tab3);

    res = dwarf_get_fde_instr_bytes(fde,&outinstrs,&instrslen,error);
    if (res != DW_DLV_OK) {
        printf("dwarf_get_fde_instr_bytes failed!\n");
        exit(EXIT_FAILURE);
    }
    res = dwarf_get_cie_of_fde(fde,&cie,error);
    if (res != DW_DLV_OK) {
        printf("Error getting cie from fde\n");
        exit(EXIT_FAILURE);
    }

    {
        Dwarf_Frame_Instr_Head frame_instr_head = 0;
        Dwarf_Unsigned frame_instr_count = 0;
        res = dwarf_expand_frame_instructions(cie,
            outinstrs,instrslen,
            &frame_instr_head,
            &frame_instr_count,
            error);
        if (res != DW_DLV_OK) {
            printf("dwarf_expand_frame_instructions failed!\n");
            exit(EXIT_FAILURE);
        }
        printf("Frame op count: %" DW_PR_DUu "\n",frame_instr_count);
        print_frame_instrs(dbg,frame_instr_head,
            frame_instr_count, error);

        dwarf_dealloc_frame_instr_head(frame_instr_head);
    }
    free(tab3.rt3_rules);
}

static void
print_reg(int r)
{
    switch(r) {
    case SAME_VAL:
        printf(" %d SAME_VAL ",r);
        break;
    case UNDEF_VAL:
        printf(" %d UNDEF_VAL ",r);
        break;
    case CFA_VAL:
        printf(" %d (CFA) ",r);
        break;
    default:
        printf(" r%d ",r);
        break;
    }
}

static void
print_one_regentry(const char *prefix,
    struct Dwarf_Regtable_Entry3_s *entry)
{
    int is_cfa = !strcmp("cfa",prefix);
    printf("%s ",prefix);
    printf("type: %d %s ",
        entry->dw_value_type,
        (entry->dw_value_type == DW_EXPR_OFFSET)? "DW_EXPR_OFFSET":
        (entry->dw_value_type == DW_EXPR_VAL_OFFSET)?
            "DW_EXPR_VAL_OFFSET":
        (entry->dw_value_type == DW_EXPR_EXPRESSION)?
            "DW_EXPR_EXPRESSION":
        (entry->dw_value_type == DW_EXPR_VAL_EXPRESSION)?
            "DW_EXPR_VAL_EXPRESSION":
            "Unknown");
    switch(entry->dw_value_type) {
    case DW_EXPR_OFFSET:
        print_reg(entry->dw_regnum);
        printf(" offset_rel? %d ",entry->dw_offset_relevant);
        if (entry->dw_offset_relevant) {
            printf(" offset  %" DW_PR_DSd " " ,
                entry->dw_offset);
            if (is_cfa) {
                printf("defines cfa value");
            } else {
                printf("address of value is CFA plus signed offset");
            }
            if (!is_cfa  && entry->dw_regnum != CFA_VAL) {
                printf(" compiler botch, regnum != CFA_VAL");
            }
        } else {
            printf("value in register");
        }
        break;
    case DW_EXPR_VAL_OFFSET:
        print_reg(entry->dw_regnum);
        printf(" offset  %" DW_PR_DSd " " ,
            entry->dw_offset);
        if (is_cfa) {
            printf("does this make sense? No?");
        } else {
            printf("value at CFA plus signed offset");
        }
        if (!is_cfa  && entry->dw_regnum != CFA_VAL) {
            printf(" compiler botch, regnum != CFA_VAL");
        }
        break;
    case DW_EXPR_EXPRESSION:
        print_reg(entry->dw_regnum);
        printf(" offset_rel? %d ",entry->dw_offset_relevant);
        printf(" offset  %" DW_PR_DUu " " ,
            entry->dw_offset);
        printf("Block ptr set? %s ",
            entry->dw_block.bl_data?"yes":"no");
        printf(" Value is at address given by expr val ");
        /* printf(" block-ptr  0x%" DW_PR_DUx " ",
            (Dwarf_Unsigned)entry->dw_block_ptr); */
        break;
    case DW_EXPR_VAL_EXPRESSION:
        printf(" expression byte len  %" DW_PR_DUu " " ,
            entry->dw_block.bl_len);
        printf("Block ptr set? %s ",
            entry->dw_block.bl_data?"yes":"no");
        printf(" Value is expr val ");
        if (!entry->dw_block.bl_data) {
            printf("Compiler or libdwarf botch, "
                "NULL block data pointer. ");
        }
        /* printf(" block-ptr  0x%" DW_PR_DUx " ",
            (Dwarf_Unsigned)entry->dw_block.bl_data); */
        break;
    default: break;
    }
    printf("\n");
}

static void
print_regtable(Dwarf_Regtable3 *tab3)
{
    int r;
    /* We won't print too much. A bit arbitrary. */
    int max = 10;
    if (max > tab3->rt3_reg_table_size) {
        max = tab3->rt3_reg_table_size;
    }
    print_one_regentry("cfa",&tab3->rt3_cfa_rule);

    for (r = 0; r < max; r++) {
        char rn[30];
        snprintf(rn,sizeof(rn),"reg %d",r);
        print_one_regentry(rn,tab3->rt3_rules+r);
    }
}
