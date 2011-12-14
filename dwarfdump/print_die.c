/* 

  Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2009-2011 SN Systems Ltd. All rights reserved.
  Portions Copyright 2007-2011 David Anderson. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 51
  Franklin Street - Fifth Floor, Boston MA 02110-1301, USA.

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan


$ Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/print_die.c,v 1.51 2006/04/01 16:20:21 davea Exp $ */
/*  The address of the Free Software Foundation is
    Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
    Boston, MA 02110-1301, USA.  
    SGI has moved from the Crittenden Lane address.  */


#include "globals.h"
#include "naming.h"
#include "esb.h"                /* For flexible string buffer. */
#include "makename.h"           /* Non-duplicating string table. */
#include "print_frames.h"       /* for get_string_from_locs() . */
#include "tag_common.h" 

/*  Traverse a DIE and attributes to check self references */
static boolean traverse_one_die(Dwarf_Debug dbg, Dwarf_Attribute attrib,
    Dwarf_Die die, char **srcfiles,
    Dwarf_Signed cnt, int die_indent_level);
static boolean traverse_attribute(Dwarf_Debug dbg, Dwarf_Die die,
    Dwarf_Half attr, Dwarf_Attribute attr_in,
    boolean print_information,
    char **srcfiles, Dwarf_Signed cnt,
    int die_indent_level);
static void print_die_and_children_internal(Dwarf_Debug dbg, 
    Dwarf_Die in_die_in,
    Dwarf_Bool is_info,
    char **srcfiles, Dwarf_Signed cnt);
static int print_one_die_section(Dwarf_Debug dbg,Dwarf_Bool is_info);

/* Is this a PU has been invalidated by the SN Systems linker? */
#define IsInvalidCode(low,high) ((low == elf_max_address) || (low == 0 && high == 0))


static int get_form_values(Dwarf_Attribute attrib,
    Dwarf_Half * theform, Dwarf_Half * directform);
static void show_form_itself(int show_form,int verbose,
    int theform, int directform, struct esb_s * str_out);
static void print_exprloc_content(Dwarf_Debug dbg,Dwarf_Die die, Dwarf_Attribute attrib,
    int showhextoo, struct esb_s *esbp);
static boolean print_attribute(Dwarf_Debug dbg, Dwarf_Die die,
    Dwarf_Half attr,
    Dwarf_Attribute actual_addr,
    boolean print_information,
    int die_indent_level, char **srcfiles,
    Dwarf_Signed cnt);
static void get_location_list(Dwarf_Debug dbg, Dwarf_Die die,
    Dwarf_Attribute attr, struct esb_s *esbp);
static int legal_tag_attr_combination(Dwarf_Half tag, Dwarf_Half attr);
static int legal_tag_tree_combination(Dwarf_Half parent_tag, 
    Dwarf_Half child_tag);
static int _dwarf_print_one_expr_op(Dwarf_Debug dbg,Dwarf_Loc* expr,
    int index, struct esb_s *string_out);
static int formxdata_print_value(Dwarf_Debug dbg,Dwarf_Attribute attrib, 
    struct esb_s *esbp, Dwarf_Error * err, Dwarf_Bool hex_format);

/* esb_base is static so gets initialized to zeros.  
   It is not thread-safe or
   safe for multiple open producer instances for
   but that does not matter here in dwarfdump.

   The memory used by esb_base  and esb_extra is never freed.
*/
static struct esb_s esb_base;   
static struct esb_s esb_extra;   

static int dwarf_names_print_on_error = 1;

static int die_stack_indent_level = 0;
static boolean local_symbols_already_began = FALSE;

typedef const char *(*encoding_type_func) (unsigned,int doprintingonerr);

Dwarf_Off fde_offset_for_cu_low = DW_DLV_BADOFFSET;
Dwarf_Off fde_offset_for_cu_high = DW_DLV_BADOFFSET;

/* Indicators to record a pair [low,high], these
   are used in printing DIEs to accumulate the high
   and low pc across attributes and to record the pair
   as soon as both are known. Probably would be better to
   use variables as arguments to 
   print_attribute().  */
static Dwarf_Addr lowAddr = 0;
static Dwarf_Addr highAddr = 0;
static Dwarf_Bool bSawLow = FALSE;
static Dwarf_Bool bSawHigh = FALSE;

/* The following too is related to high and low pc
attributes of a function. It's misnamed, it really means
'yes, we have high and low pc' if it is TRUE. Defaulting to TRUE
seems bogus. */
static Dwarf_Bool in_valid_code = TRUE;

struct operation_descr_s {
    int op_code;
    int op_count;
    const char * op_1type;
};
struct operation_descr_s opdesc[]= {
    {DW_OP_addr,1,"addr" },
    {DW_OP_deref,0 },
    {DW_OP_const1u,1,"1u" },
    {DW_OP_const1s,1,"1s" },
    {DW_OP_const2u,1,"2u" },
    {DW_OP_const2s,1,"2s" },
    {DW_OP_const4u,1,"4u" },
    {DW_OP_const4s,1,"4s" },
    {DW_OP_const8u,1,"8u" },
    {DW_OP_const8s,1,"8s" },
    {DW_OP_constu,1,"uleb" },
    {DW_OP_consts,1,"sleb" },
    {DW_OP_dup,0,""},
    {DW_OP_drop,0,""},
    {DW_OP_over,0,""},
    {DW_OP_pick,1,"1u"},
    {DW_OP_swap,0,""},
    {DW_OP_rot,0,""},
    {DW_OP_xderef,0,""},
    {DW_OP_abs,0,""},
    {DW_OP_and,0,""},
    {DW_OP_div,0,""},
    {DW_OP_minus,0,""},
    {DW_OP_mod,0,""},
    {DW_OP_mul,0,""},
    {DW_OP_neg,0,""},
    {DW_OP_not,0,""},
    {DW_OP_or,0,""},
    {DW_OP_plus,0,""},
    {DW_OP_plus_uconst,1,"uleb"},
    {DW_OP_shl,0,""},
    {DW_OP_shr,0,""},
    {DW_OP_shra,0,""},
    {DW_OP_xor,0,""},
    {DW_OP_skip,1,"2s"},
    {DW_OP_bra,1,"2s"},
    {DW_OP_eq,0,""},
    {DW_OP_ge,0,""},
    {DW_OP_gt,0,""},
    {DW_OP_le,0,""},
    {DW_OP_lt,0,""},
    {DW_OP_ne,0,""},
    /* lit0 thru reg31 handled specially, no operands */
    /* breg0 thru breg31 handled specially, 1 operand */
    {DW_OP_regx,1,"uleb"},
    {DW_OP_fbreg,1,"sleb"},
    {DW_OP_bregx,2,"uleb"},
    {DW_OP_piece,1,"uleb"},
    {DW_OP_deref_size,1,"1u"},
    {DW_OP_xderef_size,1,"1u"},
    {DW_OP_nop,0,""},
    {DW_OP_push_object_address,0,""},
    {DW_OP_call2,1,"2u"},
    {DW_OP_call4,1,"4u"},
    {DW_OP_call_ref,1,"off"},
    {DW_OP_form_tls_address,0,""},
    {DW_OP_call_frame_cfa,0,""},
    {DW_OP_bit_piece,2,"uleb"},
    {DW_OP_implicit_value,2,"uleb"},
    {DW_OP_stack_value,0,""},
    {DW_OP_GNU_uninit,0,""},
    {DW_OP_GNU_encoded_addr,1,"addr"},
    {DW_OP_GNU_implicit_pointer,1,"addr" },
    {DW_OP_GNU_entry_value,1,"val" },
    /* terminator */
    {0,0,""} 
};

struct die_stack_data_s {
    Dwarf_Die die_;
    boolean already_printed_;
};
struct die_stack_data_s empty_stack_entry;

#define DIE_STACK_SIZE 800
static struct die_stack_data_s die_stack[DIE_STACK_SIZE];

#define SET_DIE_STACK_ENTRY(i,x) { die_stack[i].die_ = x; \
    die_stack[die_stack_indent_level].already_printed_ = FALSE; }
#define EMPTY_DIE_STACK_ENTRY(i) { die_stack[i] = empty_stack_entry; }

static int
print_as_info_or_cu()
{
    return (info_flag || cu_name_flag);
}


/* process each compilation unit in .debug_info */
void
print_infos(Dwarf_Debug dbg,Dwarf_Bool is_info)
{
    int nres = 0;
    if(is_info) {
        nres = print_one_die_section(dbg,TRUE);
        if (nres == DW_DLV_ERROR) {
            char * errmsg = dwarf_errmsg(err);
            Dwarf_Unsigned myerr = dwarf_errno(err);
    
            fprintf(stderr, "%s ERROR:  %s:  %s (%lu)\n",
                program_name, "attempting to print .debug_info",
                errmsg, (unsigned long) myerr);
            fprintf(stderr, "attempting to continue.\n");
        }
        return;
    }
    nres = print_one_die_section(dbg,FALSE);
    if (nres == DW_DLV_ERROR) {
        char * errmsg = dwarf_errmsg(err);
        Dwarf_Unsigned myerr = dwarf_errno(err);

        fprintf(stderr, "%s ERROR:  %s:  %s (%lu)\n",
            program_name, "attempting to print .debug_types",
            errmsg, (unsigned long) myerr);
        fprintf(stderr, "attempting to continue.\n");
    }
}
static  void
print_std_cu_hdr(Dwarf_Unsigned cu_header_length,
    Dwarf_Unsigned abbrev_offset,
    Dwarf_Half version_stamp,
    Dwarf_Half address_size)
{
    if(dense) {
        printf(" %s<0x%" DW_PR_XZEROS  DW_PR_DUx 
            ">", "cu_header_length",
            cu_header_length);
        printf(" %s<0x%04x>", "version_stamp", 
            version_stamp);
        printf(" %s<0x%"  DW_PR_XZEROS DW_PR_DUx 
            ">", "abbrev_offset", abbrev_offset);
        printf(" %s<0x%02x>", "address_size", 
            address_size);
    } else {
        printf("  %-16s = 0x%" DW_PR_XZEROS DW_PR_DUx 
            " %" DW_PR_DUu
            "\n", "cu_header_length", 
            cu_header_length,
            cu_header_length);
        printf("  %-16s = 0x%04x     %u\n", "version_stamp", 
            version_stamp,version_stamp);
        printf("  %-16s = 0x%" DW_PR_XZEROS DW_PR_DUx 
            " %" DW_PR_DUu
            "\n", "abbrev_offset",
            abbrev_offset,
            abbrev_offset);
        printf("  %-16s = 0x%02x       %u\n", "address_size", 
            address_size,address_size);
    }
}
static void
print_std_cu_signature(Dwarf_Sig8 *signature,Dwarf_Unsigned typeoffset)
{
    if(dense) {
        struct esb_s sig8str;
        esb_constructor(&sig8str);
        format_sig8_string(signature,&sig8str);
        printf(" %s<%s>", "signature",esb_get_string(&sig8str)); 
        printf(" %s<0x%" DW_PR_XZEROS DW_PR_DUx ">", 
            "typeoffset", typeoffset);
        esb_destructor(&sig8str);
    } else {
        struct esb_s sig8str;
        esb_constructor(&sig8str);
        format_sig8_string(signature,&sig8str);
        printf("  %-16s = %s\n", "signature",esb_get_string(&sig8str)); 
        printf("  %-16s = 0x%" DW_PR_XZEROS DW_PR_DUx " %" DW_PR_DUu "\n", 
            "typeoffset", 
            typeoffset,typeoffset);
        esb_destructor(&sig8str);
    }
}

static int
print_one_die_section(Dwarf_Debug dbg,Dwarf_Bool is_info)
{
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Unsigned abbrev_offset = 0;
    Dwarf_Half version_stamp = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Half extension_size = 0;
    Dwarf_Half length_size = 0;
    Dwarf_Sig8 signature;
    Dwarf_Unsigned typeoffset = 0;
    Dwarf_Unsigned next_cu_offset = 0;
    unsigned loop_count = 0;
    int nres = DW_DLV_OK;
    int   cu_count = 0;
    char * cu_short_name = NULL;
    char * cu_long_name = NULL;
    char * producer_name = NULL;

    current_section_id = DEBUG_INFO;

    if (print_as_info_or_cu() && do_print_dwarf) {
        if(is_info) {
            printf("\n.debug_info\n");
        }
    }

    /* Loop until it fails.  */
    for(;;++loop_count) {
        int sres = DW_DLV_OK;
        Dwarf_Die cu_die = 0;

        nres = dwarf_next_cu_header_c(dbg, 
            is_info,
            &cu_header_length, &version_stamp,
            &abbrev_offset, &address_size,
            &length_size,&extension_size,
            &signature, &typeoffset,
            &next_cu_offset, &err);
        if (nres == DW_DLV_NO_ENTRY) {
            return nres;
        }
        if( loop_count == 0 &&!is_info &&
            print_as_info_or_cu() && do_print_dwarf) {
            printf("\n.debug_types\n");
        }
        if (nres != DW_DLV_OK) {
            return nres;
        }
        if(cu_count >=  break_after_n_units) {
            printf("Break at %d\n",cu_count);
            break;
        }
        /*  Regardless of any options used, get basic
            information about the current CU: producer, name */
        sres = dwarf_siblingof_b(dbg, NULL,is_info, &cu_die, &err);
        if (sres != DW_DLV_OK) {
            print_error(dbg, "siblingof cu header", sres, err);
        }
        /* Get the CU offset for easy error reporting */
        dwarf_die_offsets(cu_die,&DIE_overall_offset,&DIE_offset,&err);

        if (cu_name_flag) {
            if(should_skip_this_cu(dbg,cu_die,err)) {
                dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
                cu_die = 0;
                ++cu_count;
                cu_offset = next_cu_offset;
                continue;
            }
        }

        /* Get producer name for this CU and update compiler list */
        get_producer_name(dbg,cu_die,err,&producer_name);
        update_compiler_target(producer_name);

        /*  Once the compiler table has been updated, see
            if we need to generate the list of CU compiled
            by all the producers contained in the elf file */
        if (producer_children_flag) {
            get_cu_name(dbg,cu_die,err,&cu_short_name,&cu_long_name);
            /* Add CU name to current compiler entry */
            add_cu_name_compiler_target(cu_long_name);
        }

        /*  If the current compiler is not requested by the
            user, then move to the next CU */
        if (!checking_this_compiler()) {
            dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
            ++cu_count;
            cu_offset = next_cu_offset;
            cu_die = 0;
            continue;
        }

        /*  We have not seen the compile unit  yet, reset these 
            error-reporting  globals. */
        seen_CU = FALSE;
        need_CU_name = TRUE;
        need_CU_base_address = TRUE;
        need_CU_high_address = TRUE;
        seen_PU_base_address = FALSE;
        seen_PU_high_address = FALSE;

        /*  Release the 'cu_die' created by the call
            to 'dwarf_siblingof' at the top of the main loop. */
        dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
        cu_die = 0; /* For debugging, stale die should be NULL. */

        if (info_flag && do_print_dwarf) {
            if(verbose) {
                if (dense) {
                    printf("<%s>", "cu_header");
                } else {
                    printf("\nCU_HEADER:\n");
                }
                print_std_cu_hdr(cu_header_length,abbrev_offset,
                    version_stamp,address_size);
                if(!is_info) {
                    print_std_cu_signature(&signature,typeoffset);
                }
                if(dense) {
                    printf("\n");
                }
            } else {
                if(!is_info) {
                    if(dense) {  
                        printf("<%s>", "cu_header");
                    } else {
                        printf("\nCU_HEADER:\n");
                    }
                    print_std_cu_signature(&signature,typeoffset);
                    if(dense) {
                        printf("\n");
                    }
                }
            }
        }

        /* Get abbreviation info for this CU */
        get_abbrev_array_info(dbg,abbrev_offset);

        /* process a single compilation unit in .debug_info. */
        sres = dwarf_siblingof_b(dbg, NULL,is_info, &cu_die, &err);
        if (sres == DW_DLV_OK) {
            if (print_as_info_or_cu() || search_is_on) {
                Dwarf_Signed cnt = 0;
                char **srcfiles = 0;
                int srcf = dwarf_srcfiles(cu_die,
                    &srcfiles, &cnt, &err);
                if (srcf != DW_DLV_OK) {
                    srcfiles = 0;
                    cnt = 0;
                }

                /* Get the CU offset for easy error reporting */
                dwarf_die_offsets(cu_die,&DIE_CU_overall_offset,
                    &DIE_CU_offset,&err);
                print_die_and_children(dbg, cu_die,is_info, srcfiles, cnt);
                if (srcf == DW_DLV_OK) {
                    int si = 0;
                    for (si = 0; si < cnt; ++si) {
                        dwarf_dealloc(dbg, srcfiles[si], DW_DLA_STRING);
                    }
                    dwarf_dealloc(dbg, srcfiles, DW_DLA_LIST);
                }
            }

            /* Dump Ranges Information */
            if (dump_ranges_info) {
                PrintBucketGroup(pRangesInfo,TRUE);
            }
    
            /*  Traverse the line section if in check mode */
            if (line_flag || check_decl_file) {
                print_line_numbers_this_cu(dbg, cu_die);
            }
            dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
            cu_die = 0;
        } else if (sres == DW_DLV_NO_ENTRY) {
            /* Do nothing I guess. */
        } else {
            print_error(dbg, "Regetting cu_die", sres, err);
        }
        ++cu_count;
        cu_offset = next_cu_offset;
    }
    return nres;
}

static void
print_a_die_stack(Dwarf_Debug dbg,char **srcfiles,Dwarf_Signed cnt,int lev)
{
    boolean print_information = TRUE;
    boolean ignore_die_stack = FALSE;
    print_one_die(dbg,die_stack[lev].die_,print_information,lev,srcfiles,cnt,
        ignore_die_stack);
}
extern void
print_die_and_children(Dwarf_Debug dbg, 
    Dwarf_Die in_die_in,
    Dwarf_Bool is_info,
    char **srcfiles, Dwarf_Signed cnt)
{
    print_die_and_children_internal(dbg, 
        in_die_in,is_info,srcfiles,cnt);
}

static void
print_die_stack(Dwarf_Debug dbg,char **srcfiles,Dwarf_Signed cnt)
{
    int lev = 0;
    boolean print_information = TRUE;
    boolean ignore_die_stack = FALSE;

    for(lev = 0; lev <= die_stack_indent_level; ++lev)
    {
        print_one_die(dbg,die_stack[lev].die_,print_information,
            lev,srcfiles,cnt,
            ignore_die_stack);
    }
}

/* recursively follow the die tree */
static void
print_die_and_children_internal(Dwarf_Debug dbg, 
    Dwarf_Die in_die_in,
    Dwarf_Bool is_info,
    char **srcfiles, Dwarf_Signed cnt)
{
    Dwarf_Die child = 0;
    Dwarf_Die sibling = 0;
    Dwarf_Error err = 0;
    int tres = 0;
    int cdres = 0;
    Dwarf_Die in_die = in_die_in;

    for (;;) {
        SET_DIE_STACK_ENTRY(die_stack_indent_level,in_die);

        /* Get the CU offset for easy error reporting */
        dwarf_die_offsets(in_die,&DIE_overall_offset,&DIE_offset,&err);

        if (check_tag_tree) {
            DWARF_CHECK_COUNT(tag_tree_result,1);
            if (die_stack_indent_level == 0) {
                Dwarf_Half tag = 0;

                tres = dwarf_tag(in_die, &tag, &err);
                if (tres != DW_DLV_OK) {
                    DWARF_CHECK_ERROR(tag_tree_result,
                        "Tag-tree root is not DW_TAG_compile_unit");
                } else if (tag == DW_TAG_compile_unit) {
                    /* OK */
                } else {
                    DWARF_CHECK_ERROR(tag_tree_result,
                        "tag-tree root is not DW_TAG_compile_unit");
                }
            } else {
                Dwarf_Half tag_parent = 0; 
                Dwarf_Half tag_child = 0;
                int pres = 0;
                int cres = 0;
                const char *ctagname = "<child tag invalid>";
                const char *ptagname = "<parent tag invalid>";

                pres = dwarf_tag(die_stack[die_stack_indent_level - 1].die_, 
                    &tag_parent, &err);
                cres = dwarf_tag(in_die, &tag_child, &err);
                if (pres != DW_DLV_OK)
                    tag_parent = 0;
                if (cres != DW_DLV_OK)
                    tag_child = 0;

                /* Check for specific compiler */
                if (checking_this_compiler()) {
                    /* Process specific TAGs. */
                    tag_specific_checks_setup(tag_child,die_stack_indent_level);
                    if (cres != DW_DLV_OK || pres != DW_DLV_OK) {
                        if (cres == DW_DLV_OK) {
                            ctagname = get_TAG_name(tag_child,
                                dwarf_names_print_on_error);
                        }
                        if (pres == DW_DLV_OK) {
                            ptagname = get_TAG_name(tag_parent,
                                dwarf_names_print_on_error);
                        }
                        DWARF_CHECK_ERROR3(tag_tree_result,ptagname,
                            ctagname,
                            "Tag-tree relation is not standard..");
                    } else if (legal_tag_tree_combination(tag_parent, 
                        tag_child)) {
                        /* OK */
                    } else {
                        DWARF_CHECK_ERROR3(tag_tree_result,
                            get_TAG_name(tag_parent,
                                dwarf_names_print_on_error),
                            get_TAG_name(tag_child,
                                dwarf_names_print_on_error),
                            "tag-tree relation is not standard.");
                    }
                }
            }
        }

        if (record_dwarf_error && check_verbose_mode) {
            record_dwarf_error = FALSE;
        }

        /* Here do pre-descent processing of the die. */
        {
            boolean retry_print_on_match = FALSE;
            boolean ignore_die_stack = FALSE;
            retry_print_on_match = print_one_die(dbg, in_die, 
                print_as_info_or_cu(), 
                die_stack_indent_level, srcfiles, cnt,ignore_die_stack);
            if(!print_as_info_or_cu() && retry_print_on_match) {
                if (display_parent_tree) {
                    print_die_stack(dbg,srcfiles,cnt);
                } else {
                    if (display_children_tree) {
                        print_a_die_stack(dbg,srcfiles,cnt,die_stack_indent_level);
                    }
                }
                if (display_children_tree) {
                    stop_indent_level = die_stack_indent_level;
                    info_flag = TRUE;
                }
            }
        }

        cdres = dwarf_child(in_die, &child, &err);

        /* Check for specific compiler */
        if (check_abbreviations && checking_this_compiler()) {
            Dwarf_Half ab_has_child;
            Dwarf_Bool bError = FALSE;
            Dwarf_Half tag = 0;
            tres = dwarf_die_abbrev_children_flag(in_die,&ab_has_child);
            if (tres == DW_DLV_OK) {
                DWARF_CHECK_COUNT(abbreviations_result,1);
                tres = dwarf_tag(in_die, &tag, &err);
                if (tres == DW_DLV_OK) {
                    switch (tag) {
                    case DW_TAG_array_type:
                    case DW_TAG_class_type:
                    case DW_TAG_compile_unit:
                    case DW_TAG_enumeration_type:
                    case DW_TAG_lexical_block:
                    case DW_TAG_namespace:
                    case DW_TAG_structure_type:
                    case DW_TAG_subprogram:
                    case DW_TAG_subroutine_type:
                    case DW_TAG_union_type:
                    case DW_TAG_entry_point:
                    case DW_TAG_inlined_subroutine:
                        break;
                    default:
                        bError = (cdres == DW_DLV_OK && !ab_has_child) ||
                            (cdres == DW_DLV_NO_ENTRY && ab_has_child);
                        if (bError) {
                            DWARF_CHECK_ERROR(abbreviations_result,
                                "check 'dw_children' flag combination.");
                        }
                        break;
                    }
                }
            }
        }


        /* child first: we are doing depth-first walk */
        if (cdres == DW_DLV_OK) {
            die_stack_indent_level++;
            SET_DIE_STACK_ENTRY(die_stack_indent_level,0);
            if(die_stack_indent_level >= DIE_STACK_SIZE ) {
                print_error(dbg,
                    "compiled in DIE_STACK_SIZE limit exceeded",
                    DW_DLV_OK,err);
            }
            print_die_and_children_internal(dbg, child,is_info, srcfiles, cnt);
            EMPTY_DIE_STACK_ENTRY(die_stack_indent_level);
            die_stack_indent_level--;
            if (die_stack_indent_level == 0)
                local_symbols_already_began = FALSE;
            dwarf_dealloc(dbg, child, DW_DLA_DIE);
            child = 0;
        } else if (cdres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_child", cdres, err);
        }

        /* Stop the display of all children */
        if (display_children_tree && info_flag && 
            stop_indent_level == die_stack_indent_level) {

            info_flag = FALSE;
        }

        cdres = dwarf_siblingof_b(dbg, in_die,is_info, &sibling, &err);
        if (cdres == DW_DLV_OK) {
            /*  print_die_and_children(dbg, sibling, srcfiles, cnt); We
                loop around to actually print this, rather than
                recursing. Recursing is horribly wasteful of stack
                space. */
        } else if (cdres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_siblingof", cdres, err);
        }

        /*  If we have a sibling, verify that its offset
            is next to the last processed DIE; 
            An incorrect sibling chain is a nasty bug.  */
        if (cdres == DW_DLV_OK && sibling && check_di_gaps && 
            checking_this_compiler()) {

            Dwarf_Off glb_off;
            DWARF_CHECK_COUNT(di_gaps_result,1);
            if (dwarf_validate_die_sibling(sibling,&glb_off) == DW_DLV_ERROR) {
                static char msg[128];
                Dwarf_Off sib_off;
                dwarf_dieoffset(sibling,&sib_off,&err);
                sprintf(msg,
                    "GSIB = 0x%" DW_PR_XZEROS  DW_PR_DUx 
                    " GOFF = 0x%" DW_PR_XZEROS DW_PR_DUx 
                    " Gap = %" DW_PR_DUu " bytes",
                    sib_off,glb_off,sib_off-glb_off);
                DWARF_CHECK_ERROR2(di_gaps_result,
                    "Incorrect sibling chain",msg);
            }
        }

        /*  Here do any post-descent (ie post-dwarf_child) processing of 
            the in_die. */

        EMPTY_DIE_STACK_ENTRY(die_stack_indent_level);
        if (in_die != in_die_in) {
            /*  Dealloc our in_die, but not the argument die, it belongs 
                to our caller. Whether the siblingof call worked or not. */
            dwarf_dealloc(dbg, in_die, DW_DLA_DIE);
            in_die = 0;
        }
        if (cdres == DW_DLV_OK) {
            /*  Set to process the sibling, loop again. */
            in_die = sibling;
        } else {
            /*  We are done, no more siblings at this level. */
            break;
        }
    }  /* end for loop on siblings */
    return;
}

/* Print one die on error and verbose or non check mode */
#define PRINTING_DIES (do_print_dwarf || (record_dwarf_error && check_verbose_mode))

/*  If print_information is FALSE, check the TAG and if it is a CU die
    print the information anyway. */
boolean
print_one_die(Dwarf_Debug dbg, Dwarf_Die die, 
    boolean print_information,
    int die_indent_level,
    char **srcfiles, Dwarf_Signed cnt, 
    boolean ignore_die_stack)
{
    Dwarf_Signed i = 0;
    Dwarf_Off offset = 0; 
    Dwarf_Off overall_offset = 0;
    const char * tagname = 0;
    Dwarf_Half tag = 0;
    Dwarf_Signed atcnt = 0;
    Dwarf_Attribute *atlist = 0;
    int tres = 0;
    int ores = 0;
    int atres = 0;
    int abbrev_code = dwarf_die_abbrev_code(die);
    boolean attribute_matched = FALSE;

    /* Print using indentation
    < 1><0x000854ff GOFF=0x00546047>    DW_TAG_pointer_type -> 34
    < 1><0x000854ff>    DW_TAG_pointer_type                 -> 18
        DW_TAG_pointer_type                                 ->  2
    */
    /* Attribute indent. */
    int nColumn = show_global_offsets ? 34 : 18;
  
    if (check_abbreviations && checking_this_compiler()) {
        validate_abbrev_code(dbg,abbrev_code);
    }

    if(!ignore_die_stack && die_stack[die_indent_level].already_printed_) {
        /* FALSE seems like a safe return. */
        return FALSE;
    }

    /* Reset indentation column if no offsets */
    if (!display_offsets) {
        nColumn = 2;
    }

    tres = dwarf_tag(die, &tag, &err);
    if (tres != DW_DLV_OK) {
        print_error(dbg, "accessing tag of die!", tres, err);
    }
    tagname = get_TAG_name(tag,dwarf_names_print_on_error);
    
    tag_specific_checks_setup(tag,die_indent_level);
    ores = dwarf_dieoffset(die, &overall_offset, &err);
    if (ores != DW_DLV_OK) {
        print_error(dbg, "dwarf_dieoffset", ores, err);
    }
    ores = dwarf_die_CU_offset(die, &offset, &err);
    if (ores != DW_DLV_OK) {
        print_error(dbg, "dwarf_die_CU_offset", ores, err);
    }

    if (dump_visited_info && check_self_references) {
        printf("<%2d><0x%" DW_PR_XZEROS DW_PR_DUx 
            " GOFF=0x%" DW_PR_XZEROS DW_PR_DUx "> ",
            die_indent_level, (Dwarf_Unsigned)offset,
            (Dwarf_Unsigned)overall_offset);
        printf("%*s%s\n",die_indent_level * 2 + 2," ",tagname);
    }

    /* Print the die */
    if (PRINTING_DIES && print_information) {
        if (!ignore_die_stack) {
            die_stack[die_indent_level].already_printed_ = TRUE;
        }
        if (die_indent_level == 0) {
            if (dense) {
                printf("\n");
            } else {
                printf("\nCOMPILE_UNIT<header overall offset = 0x%" 
                    DW_PR_XZEROS DW_PR_DUx ">:\n",
                    (Dwarf_Unsigned)(overall_offset - offset));
            }
        } else if (local_symbols_already_began == FALSE &&
            die_indent_level == 1 && !dense) {

            printf("\nLOCAL_SYMBOLS:\n");
            local_symbols_already_began = TRUE;
        }

        /* Print just the Tags and Attributes */
        if (!display_offsets) {
            /* Print using indentation */
            printf("%*s%s\n",die_stack_indent_level * 2 + 2," ",tagname);
        } else {
            if (dense) {
                if (show_global_offsets) {
                    if (die_indent_level == 0) {
                        printf("<%d><0x%" DW_PR_DUx "+0x%" DW_PR_DUx " GOFF=0x%" 
                            DW_PR_DUx ">", die_indent_level,
                            (Dwarf_Unsigned)(overall_offset - offset), 
                            (Dwarf_Unsigned)offset,
                                (Dwarf_Unsigned)overall_offset);
                        } else {
                        printf("<%d><0x%" DW_PR_DUx " GOFF=0x%" DW_PR_DUx ">", 
                            die_indent_level, 
                            (Dwarf_Unsigned)offset, 
                            (Dwarf_Unsigned)overall_offset);
                    }
                } else {
                    if (die_indent_level == 0) {
                        printf("<%d><0x%" DW_PR_DUx "+0x%" DW_PR_DUx ">", 
                            die_indent_level,
                            (Dwarf_Unsigned)(overall_offset - offset), 
                            (Dwarf_Unsigned)offset);
                    } else {
                        printf("<%d><0x%" DW_PR_DUx ">", die_indent_level, 
                            (Dwarf_Unsigned)offset);
                    }
                }
                printf("<%s>",tagname);
                if(verbose) {
                    printf(" <abbrev %d>",abbrev_code);
                }
            } else {
                if (show_global_offsets) {
                    printf("<%2d><0x%" DW_PR_XZEROS DW_PR_DUx 
                        " GOFF=0x%" DW_PR_XZEROS DW_PR_DUx ">", 
                        die_indent_level, (Dwarf_Unsigned)offset,
                        (Dwarf_Unsigned)overall_offset);
                } else {
                    printf("<%2d><0x%" DW_PR_XZEROS DW_PR_DUx ">", 
                        die_indent_level, 
                        (Dwarf_Unsigned)offset);
                }

                /* Print using indentation */
                printf("%*s%s",die_indent_level * 2 + 2," ",tagname);
                if(verbose) {
                    printf(" <abbrev %d>",abbrev_code);
                }
                fputs("\n",stdout);
            }
        }
    }

    atres = dwarf_attrlist(die, &atlist, &atcnt, &err);
    if (atres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_attrlist", atres, err);
    } else if (atres == DW_DLV_NO_ENTRY) {
        /* indicates there are no attrs.  It is not an error. */
        atcnt = 0;
    }

    /* Reset any loose references to low or high PC */
    bSawLow = FALSE;
    bSawHigh = FALSE;

    /* Get the CU offset for easy error reporting */
    dwarf_die_offsets(die,&DIE_CU_overall_offset,&DIE_CU_offset,&err);

    for (i = 0; i < atcnt; i++) {
        Dwarf_Half attr;
        int ares;

        ares = dwarf_whatattr(atlist[i], &attr, &err);
        if (ares == DW_DLV_OK) {
            /* Print using indentation */
            if (!dense && PRINTING_DIES && print_information) {
                printf("%*s",die_indent_level * 2 + 2 + nColumn," ");
            }

            {
                boolean attr_match = print_attribute(dbg, die, attr,
                    atlist[i],
                    print_information, die_indent_level, srcfiles, cnt);
                if(print_information == FALSE && attr_match) {
                    attribute_matched = TRUE;
                }
            }

            if (record_dwarf_error && check_verbose_mode) {
                record_dwarf_error = FALSE;
            }
        } else {
            print_error(dbg, "dwarf_whatattr entry missing", ares, err);
        }
    }

    for (i = 0; i < atcnt; i++) {
        dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
    }
    if (atres == DW_DLV_OK) {
        dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
    }

    if (PRINTING_DIES && dense && print_information) {
        printf("\n");
    }
    return attribute_matched;
}

/*  Encodings have undefined signedness. Accept either
    signedness.  The values are integer-like (they are defined
    in the DWARF specification), so the
    form the compiler uses (as long as it is
    a constant value) is a non-issue.

    The numbers need not be small (in spite of the
    function name), but the result should be an integer.

    If string_out is non-NULL, construct a string output, either
    an error message or the name of the encoding.
    The function pointer passed in is to code generated
    by a script at dwarfdump build time. The code for
    the val_as_string function is generated
    from dwarf.h.  See <build dir>/dwarf_names.c

    The known_signed bool is set true(nonzero) or false (zero)
    and *both* uval_out and sval_out are set to the value,
    though of course uval_out cannot represent a signed
    value properly and sval_out cannot represent all unsigned
    values properly.

    If string_out is non-NULL then attr_name and val_as_string
    must also be non-NULL.  */
static int
get_small_encoding_integer_and_name(Dwarf_Debug dbg,
    Dwarf_Attribute attrib,
    Dwarf_Unsigned * uval_out,
    char *attr_name,
    const char ** string_out,
    encoding_type_func val_as_string,
    Dwarf_Error * err,
    int show_form)
{
    Dwarf_Unsigned uval = 0;
    char buf[100];              /* The strings are small. */
    int vres = dwarf_formudata(attrib, &uval, err);

    if (vres != DW_DLV_OK) {
        Dwarf_Signed sval = 0;
        vres = dwarf_formsdata(attrib, &sval, err);
        if (vres != DW_DLV_OK) {
            vres = dwarf_global_formref(attrib,&uval,err);
            if (vres != DW_DLV_OK) {
                if (string_out != 0) {
                    snprintf(buf, sizeof(buf),
                        "%s has a bad form.", attr_name);
                    *string_out = makename(buf);
                }
                return vres;
            }
            *uval_out = uval;
        } else {
            uval =  (Dwarf_Unsigned) sval; 
            *uval_out = uval;
        }
    } else {
        *uval_out = uval;
    }
    if (string_out) {
        Dwarf_Half theform = 0;
        Dwarf_Half directform = 0;
        get_form_values(attrib,&theform,&directform);
        esb_empty_string(&esb_base);
        esb_append(&esb_base, val_as_string((Dwarf_Half) uval,
            dwarf_names_print_on_error));
        show_form_itself(show_form, verbose, theform, directform,&esb_base);
        *string_out = makename(esb_get_string(&esb_base));
    }
    return DW_DLV_OK;
}




/*  We need a 32-bit signed number here, but there's no portable
    way of getting that.  So use __uint32_t instead.  It's supplied
    in a reliable way by the autoconf infrastructure.  */

static void
get_FLAG_BLOCK_string(Dwarf_Debug dbg, Dwarf_Attribute attrib,
    struct esb_s*esbp)
{
    int fres = 0;
    Dwarf_Block *tempb = 0;
    __uint32_t * array = 0;
    Dwarf_Unsigned array_len = 0;
    __uint32_t * array_ptr;
    Dwarf_Unsigned array_remain = 0;
    char linebuf[100];

    /* first get compressed block data */
    fres = dwarf_formblock (attrib,&tempb, &err);
    if (fres != DW_DLV_OK) {
        print_error(dbg,"DW_FORM_blockn cannot get block\n",fres,err);
        return;
    }

    /* uncompress block into int array */
    array = dwarf_uncompress_integer_block(dbg,
        1, /* 'true' (meaning signed ints)*/
        32, /* bits per unit */
        tempb->bl_data,
        tempb->bl_len,
        &array_len, /* len of out array */
        &err);
    if (array == (void*) DW_DLV_BADOFFSET) {
        print_error(dbg,"DW_AT_SUN_func_offsets cannot uncompress data\n",0,err);
        return;
    }
    if (array_len == 0) {
        print_error(dbg,"DW_AT_SUN_func_offsets has no data\n",0,err);
        return;
    }
    
    /* fill in string buffer */
    array_remain = array_len;
    array_ptr = array;
    while (array_remain > 8) {
        /*  Print a full line */
        /*  If you touch this string, update the magic number 8 in
            the  += and -= below! */
        snprintf(linebuf, sizeof(linebuf), 
            "\n  0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x",
            array_ptr[0],           array_ptr[1],
            array_ptr[2],           array_ptr[3],
            array_ptr[4],           array_ptr[5],
            array_ptr[6],           array_ptr[7]);
        array_ptr += 8;
        array_remain -= 8;
        esb_append(&esb_base, linebuf);
    }

    /* now do the last line */
    if (array_remain > 0) {
        esb_append(&esb_base, "\n ");
        while (array_remain > 0) {
            snprintf(linebuf, sizeof(linebuf), " 0x%08x", *array_ptr);
            array_remain--;
            array_ptr++;
            esb_append(&esb_base, linebuf);
        }
    }
    
    /* free array buffer */
    dwarf_dealloc_uncompressed_block(dbg, array);

}

static const char *
get_rangelist_type_descr(Dwarf_Ranges *r)
{
    switch(r->dwr_type) {
    case DW_RANGES_ENTRY:             return "range entry";
    case DW_RANGES_ADDRESS_SELECTION: return "addr selection";
    case DW_RANGES_END:               return "range end";
    }
    /* Impossible. */
    return "Unknown";
}


void 
print_ranges_list_to_extra(Dwarf_Debug dbg,
    Dwarf_Unsigned off,
    Dwarf_Ranges *rangeset,
    Dwarf_Signed rangecount,
    Dwarf_Unsigned bytecount,
    struct esb_s *stringbuf)
{
    char tmp[200];
    Dwarf_Signed i;
    if(dense) {
        snprintf(tmp,sizeof(tmp),
            "< ranges: %" DW_PR_DSd " ranges at .debug_ranges offset %" 
            DW_PR_DUu " (0x%" DW_PR_XZEROS DW_PR_DUx ") "
            "(%" DW_PR_DUu " bytes)>",
            rangecount,
            off,
            off,
            bytecount);
        esb_append(stringbuf,tmp);
    } else {
        snprintf(tmp,sizeof(tmp),
            "\t\tranges: %" DW_PR_DSd " at .debug_ranges offset %" 
            DW_PR_DUu " (0x%" DW_PR_XZEROS DW_PR_DUx ") "
            "(%" DW_PR_DUu " bytes)\n",
            rangecount,
            off,
            off,
            bytecount);
        esb_append(stringbuf,tmp);
    }
    for(i = 0; i < rangecount; ++i) {
        Dwarf_Ranges * r = rangeset +i;
        const char *type = get_rangelist_type_descr(r);
        if(dense) {
            snprintf(tmp,sizeof(tmp),
                "<[%2" DW_PR_DSd 
                "] %s 0x%" DW_PR_XZEROS  DW_PR_DUx 
                " 0x%" DW_PR_XZEROS DW_PR_DUx ">",
                (Dwarf_Signed)i,
                type, 
                (Dwarf_Unsigned)r->dwr_addr1,
                (Dwarf_Unsigned)r->dwr_addr2);
        } else {
            snprintf(tmp,sizeof(tmp),
                "\t\t\t[%2" DW_PR_DSd 
                "] %-14s 0x%" DW_PR_XZEROS  DW_PR_DUx 
                " 0x%" DW_PR_XZEROS DW_PR_DUx "\n",
                (Dwarf_Signed)i,
                type, 
                (Dwarf_Unsigned)r->dwr_addr1,
                (Dwarf_Unsigned)r->dwr_addr2);
        }
        esb_append(stringbuf,tmp);
    }
}


static boolean
is_location_form(int form)
{
    if(form == DW_FORM_block1 ||
        form == DW_FORM_block2 ||
        form == DW_FORM_block4 ||
        form == DW_FORM_block ||
        form == DW_FORM_data4 ||
        form == DW_FORM_data8 ||
        form == DW_FORM_sec_offset) {
        return TRUE;
    }
    return FALSE;
}

static void
show_attr_form_error(Dwarf_Debug dbg,unsigned attr,unsigned form,struct esb_s *out)
{
    const char *n = 0;
    int res;
    char buf[30];
    esb_append(out,"ERROR: Attribute ");
    snprintf(buf,sizeof(buf),"%u",attr);
    esb_append(out,buf);
    esb_append(out," (");
    res = dwarf_get_AT_name(attr,&n);
    if(res != DW_DLV_OK) {
        n = "UknownAttribute";
    }
    esb_append(out,n);
    esb_append(out,") ");
    esb_append(out," has form ");
    snprintf(buf,sizeof(buf),"%u",form);
    esb_append(out,buf);
    esb_append(out," (");
    res = dwarf_get_FORM_name(form,&n);
    if(res != DW_DLV_OK) {
        n = "UknownForm";
    }
    esb_append(out,n);
    esb_append(out,"), a form which is not appropriate");
    print_error_and_continue(dbg,esb_get_string(out), DW_DLV_OK,err);
}

/*  Traverse an attribute and following any reference
    in order to detect self references to DIES (loop). */
static boolean
traverse_attribute(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attr,
    Dwarf_Attribute attr_in,
    boolean print_information,
    char **srcfiles, Dwarf_Signed cnt,
    int die_indent_level)
{
    Dwarf_Attribute attrib = 0;
    const char * atname = 0;
    const char * valname = 0;
    int tres = 0;
    Dwarf_Half tag = 0;
    boolean circular_reference = FALSE;
    Dwarf_Bool is_info = TRUE;

    is_info = dwarf_get_die_infotypes_flag(die);
    atname = get_AT_name(attr,dwarf_names_print_on_error);

    /*  The following gets the real attribute, even in the face of an 
        incorrect doubling, or worse, of attributes. */
    attrib = attr_in;
    /*  Do not get attr via dwarf_attr: if there are (erroneously) 
        multiple of an attr in a DIE, dwarf_attr will not get the
        second, erroneous one and dwarfdump will print the first one
        multiple times. Oops. */

    tres = dwarf_tag(die, &tag, &err);
    if (tres == DW_DLV_ERROR) {
        tag = 0;
    } else if (tres == DW_DLV_NO_ENTRY) {
        tag = 0;
    } else {
        /* ok */
    }

    switch (attr) {
    case DW_AT_specification:
    case DW_AT_abstract_origin:
    case DW_AT_type: {
        int res = 0;
        Dwarf_Off die_off = 0;
        Dwarf_Off ref_off = 0;
        Dwarf_Die ref_die = 0;

        ++die_indent_level;
        esb_empty_string(&esb_base);
        get_attr_value(dbg, tag, die, attrib, srcfiles, cnt, 
            &esb_base, show_form_used,verbose);
        valname = esb_get_string(&esb_base);

        /* Get the global offset for reference */
        res = dwarf_global_formref(attrib, &ref_off, &err);
        if (res != DW_DLV_OK) {
            int errno = dwarf_errno(err);
            if (errno == DW_DLE_REF_SIG8_NOT_HANDLED ) {
                // No need to stop, ref_sig8 refers out of
                // the current section.
                break;
            } else {
                print_error(dbg, "dwarf_global_formref fails in traversal", 
                    res, err);
            }
        }
        res = dwarf_dieoffset(die, &die_off, &err);
        if (res != DW_DLV_OK) {
            int errno = dwarf_errno(err);
            if (errno == DW_DLE_REF_SIG8_NOT_HANDLED ) {
                // No need to stop, ref_sig8 refers out of
                // the current section.
                break;
            } else {
                print_error(dbg, "dwarf_dieoffset fails in traversal", res, err);
            }
        }

        /* Follow reference chain, looking for self references */
        res = dwarf_offdie_b(dbg,ref_off,is_info,&ref_die,&err);
        if (res == DW_DLV_OK) {
            ++die_indent_level;
            /* Dump visited information */
            if (dump_visited_info) {
                Dwarf_Off off = 0;
                dwarf_die_CU_offset(die, &off, &err);
                /* Check above call return status? FIXME */

                printf("<%2d><0x%" DW_PR_XZEROS DW_PR_DUx 
                    " GOFF=0x%" DW_PR_XZEROS DW_PR_DUx "> ", 
                    die_indent_level, (Dwarf_Unsigned)off,
                    (Dwarf_Unsigned)die_off);
                printf("%*s%s -> %s\n",die_indent_level * 2 + 2,
                    " ",atname,valname);
            }
            circular_reference = traverse_one_die(dbg,attrib,ref_die,
                srcfiles,cnt,die_indent_level);
            DeleteKeyInBucketGroup(pVisitedInfo,ref_off);
            dwarf_dealloc(dbg,ref_die,DW_DLA_DIE);
            --die_indent_level;
            ref_die = 0;
        }
        }
        break;
    } /* End switch. */
    return circular_reference;
}

/* Traverse one DIE in order to detect self references to DIES. */
static boolean
traverse_one_die(Dwarf_Debug dbg, Dwarf_Attribute attrib, Dwarf_Die die,
    char **srcfiles, Dwarf_Signed cnt, int die_indent_level)
{
    Dwarf_Half tag = 0;
    Dwarf_Off overall_offset = 0;
    Dwarf_Signed atcnt = 0;
    int res = 0;
    boolean circular_reference = FALSE;
    boolean print_information = FALSE;

    res = dwarf_tag(die, &tag, &err);
    if (res != DW_DLV_OK) {
        print_error(dbg, "accessing tag of die!", res, err);
    }
    res = dwarf_dieoffset(die, &overall_offset, &err);
    if (res != DW_DLV_OK) {
        print_error(dbg, "dwarf_dieoffset", res, err);
    }

    /* Print visited information */
    if (dump_visited_info) {
        Dwarf_Off offset = 0; 
        const char * tagname = 0;
        res = dwarf_die_CU_offset(die, &offset, &err);
        if (res != DW_DLV_OK) {
            print_error(dbg, "dwarf_die_CU_offsetC", res, err);
        }
        tagname = get_TAG_name(tag,dwarf_names_print_on_error);
        printf("<%2d><0x%" DW_PR_XZEROS DW_PR_DUx 
            " GOFF=0x%" DW_PR_XZEROS  DW_PR_DUx "> ", 
            die_indent_level, (Dwarf_Unsigned)offset,
            (Dwarf_Unsigned)overall_offset);
        printf("%*s%s\n",die_indent_level * 2 + 2," ",tagname);
    }

    DWARF_CHECK_COUNT(self_references_result,1);
    if (FindKeyInBucketGroup(pVisitedInfo,overall_offset)) {
        char * valname = NULL;
        Dwarf_Half attr = 0;
        const char *atname = NULL;
        esb_empty_string(&esb_base);
        get_attr_value(dbg, tag, die, attrib, srcfiles, 
            cnt, &esb_base, show_form_used,verbose);
        valname = esb_get_string(&esb_base);
        dwarf_whatattr(attrib, &attr, &err);
        atname = get_AT_name(attr,dwarf_names_print_on_error);
  
        /* We have a self reference */
        DWARF_CHECK_ERROR3(self_references_result,
            "Invalid self reference to DIE: ",atname,valname);
        circular_reference = TRUE;
    } else {
        Dwarf_Signed i = 0;
        Dwarf_Attribute *atlist = 0;

        /* Add current DIE */
        AddEntryIntoBucketGroup(pVisitedInfo,overall_offset,
            0,0,0,NULL,FALSE);

        res = dwarf_attrlist(die, &atlist, &atcnt, &err);
        if (res == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_attrlist", res, err);
        } else if (res == DW_DLV_NO_ENTRY) {
            /* indicates there are no attrs.  It is not an error. */
            atcnt = 0;
        }

        for (i = 0; i < atcnt; i++) {
            Dwarf_Half attr;
            int ares;

            ares = dwarf_whatattr(atlist[i], &attr, &err);
            if (ares == DW_DLV_OK) {
                circular_reference = traverse_attribute(dbg, die, attr,
                    atlist[i],
                    print_information, srcfiles, cnt,
                    die_indent_level);
            } else {
                print_error(dbg, "dwarf_whatattr entry missing", 
                    ares, err);
            }
        }

        for (i = 0; i < atcnt; i++) {
            dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
        }
        if (res == DW_DLV_OK) {
            dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
        }

        /* Delete current DIE */
        DeleteKeyInBucketGroup(pVisitedInfo,overall_offset);
    }
    return circular_reference;
}


/*  Extracted this from print_attribute() 
    to get tolerable indents. 
    In other words to make it readable.
    It uses global data fields excessively, but so does
    print_attribute().
    The majority of the code here is checking for
    compiler errors. */
static void
print_range_attribute(Dwarf_Debug dbg, 
   Dwarf_Die die,
   Dwarf_Half attr,
   Dwarf_Attribute attr_in,
   Dwarf_Half theform,
   int dwarf_names_print_on_error,
   boolean print_information,
   int *append_extra_string)
{
    Dwarf_Error err = 0;
    Dwarf_Unsigned original_off = 0;
    int fres = 0;
    
    fres = dwarf_global_formref(attr_in, &original_off, &err);
    if( fres == DW_DLV_OK) {
        Dwarf_Ranges *rangeset = 0;
        Dwarf_Signed rangecount = 0;
        Dwarf_Unsigned bytecount = 0;
        int rres = dwarf_get_ranges_a(dbg,original_off,
            die,
            &rangeset, 
            &rangecount,&bytecount,&err);
        if(rres == DW_DLV_OK) {
            /* Ignore ranges inside a stripped function  */
            if (check_ranges && 
                in_valid_code && checking_this_compiler()) {
                Dwarf_Unsigned off = original_off;

                Dwarf_Signed index = 0;
                Dwarf_Addr base_address = CU_base_address;
                Dwarf_Addr lopc = 0;
                Dwarf_Addr hipc = 0;
                Dwarf_Bool bError = FALSE;
  
                /* Ignore last entry, is the end-of-list */
                for (index = 0; index < rangecount - 1; index++) {
                    Dwarf_Ranges *r = rangeset + index;
  
                    if (r->dwr_addr1 == elf_max_address) {
                        /* (0xffffffff,addr), use specific address (current PU address) */
                        base_address = r->dwr_addr2;
                    } else {
                        /* (offset,offset), update using CU address */
                        lopc = r->dwr_addr1 + base_address;
                        hipc = r->dwr_addr2 + base_address;
                        DWARF_CHECK_COUNT(ranges_result,1);

                        /*  Check the low_pc and high_pc 
                            are within a valid range in 
                            the .text section */
                        if (IsValidInBucketGroup(pRangesInfo,lopc) && 
                            IsValidInBucketGroup(pRangesInfo,hipc)) {
                            /* Valid values; do nothing */
                        } else {
                            /*  At this point may be we 
                                are dealing with a 
                                linkonce symbol */
                            if (IsValidInLinkonce(pLinkonceInfo,
                                PU_name,lopc,hipc)) {
                                /* Valid values; do nothing */
                            } else {
                                bError = TRUE;
                                DWARF_CHECK_ERROR(ranges_result,
                                    ".debug_ranges: Address outside a "
                                    "valid .text range");
                                if (check_verbose_mode) {
                                    printf(
                                        "Offset = 0x%" DW_PR_XZEROS DW_PR_DUx 
                                        ", Base = 0x%" DW_PR_XZEROS DW_PR_DUx 
                                        ", "
                                        "Low = 0x%" DW_PR_XZEROS DW_PR_DUx 
                                        " (0x%" DW_PR_XZEROS  DW_PR_DUx 
                                        "), High = 0x%" 
                                        DW_PR_XZEROS  DW_PR_DUx
                                        " (0x%" DW_PR_XZEROS DW_PR_DUx 
                                        ")\n",
                                        off,base_address,lopc,
                                        r->dwr_addr1,hipc,
                                        r->dwr_addr2);
                                }
                            }
                        }
                    }
                    /*  Each entry holds 2 addresses (offsets) */
                    off += elf_address_size * 2;
                }
                if (bError && check_verbose_mode) {
                    printf("\n");
                }
            }
            if(print_information) {
                *append_extra_string = 1;
                esb_empty_string(&esb_extra);
                print_ranges_list_to_extra(dbg,original_off,
                    rangeset,rangecount,bytecount,
                    &esb_extra);
            }
            dwarf_ranges_dealloc(dbg,rangeset,rangecount);
        } else if (rres == DW_DLV_ERROR) {
            if (do_print_dwarf) {
                printf("\ndwarf_get_ranges() "
                    "cannot find DW_AT_ranges at offset 0x%" 
                    DW_PR_XZEROS DW_PR_DUx 
                    " (0x%" DW_PR_XZEROS DW_PR_DUx ").",
                    original_off,
                    original_off);
            } else {
                DWARF_CHECK_COUNT(ranges_result,1);
                DWARF_CHECK_ERROR2(ranges_result,
                    get_AT_name(attr,
                        dwarf_names_print_on_error),
                    " cannot find DW_AT_ranges at offset");
            }
        } else {
            /* NO ENTRY */
            if (do_print_dwarf) {
                printf("\ndwarf_get_ranges() "
                    "finds no DW_AT_ranges at offset 0x%" 
                    DW_PR_XZEROS DW_PR_DUx 
                    " (0x%" DW_PR_XZEROS  DW_PR_DUx ").",
                    original_off,
                    original_off);
            } else {
                DWARF_CHECK_COUNT(ranges_result,1);
                DWARF_CHECK_ERROR2(ranges_result,
                    get_AT_name(attr,
                        dwarf_names_print_on_error),
                    " fails to find DW_AT_ranges at offset");
            }
        }
    } else {
        if (do_print_dwarf) {
            struct esb_s local;
            char tmp[100];

            snprintf(tmp,sizeof(tmp)," attr 0x%x form 0x%x ",
                (unsigned)attr,(unsigned)theform);
            esb_constructor(&local);
            esb_append(&local,
                " fails to find DW_AT_ranges offset");
            esb_append(&local,tmp);
            printf(" %s ",esb_get_string(&local));            
            esb_destructor(&local);
        } else { 
            DWARF_CHECK_COUNT(ranges_result,1);
            DWARF_CHECK_ERROR2(ranges_result,
                get_AT_name(attr,
                    dwarf_names_print_on_error),
                " fails to find DW_AT_ranges offset");
        }
    }
}

/*  A DW_AT_name in a CU DIE will likely have dots
    and be entirely sensible. So lets 
    not call things a possible error when they are not.
    Some assemblers allow '.' in an identifier too. 
    We should check for that, but we don't yet.

    We should check the compiler before checking
    for 'altabi.' too (FIXME).

    This is a heuristic, not all that reliable.

    Return 0 if it is a vaguely standard identifier.
    Else return 1, meaning 'it might be a file name
    or have '.' in it quite sensibly.'

    If we don't do the TAG check we might report "t.c"
    as a questionable DW_AT_name. Which would be silly.
*/
static int
dot_ok_in_identifier(int tag,Dwarf_Die die, const char *val)
{
    if (strncmp(val,"altabi.",7)) {
        /*  Ignore the names of the form 'altabi.name',
            which apply to one specific compiler.  */
        return 1;
    } 
    if(tag == DW_TAG_compile_unit || tag == DW_TAG_partial_unit ||
        tag == DW_TAG_imported_unit || tag == DW_TAG_type_unit) {
        return 1;
    }
    return 0;
}

static void
trim_quotes(const char *val,struct esb_s *es)
{
    if(val[0] == '"') {
        size_t l = strlen(val);
        if(l > 2 && val[l-1] == '"') {
            esb_appendn(es,val+1,l-2);  
            return;
        }
    }
    esb_append(es,val);
}

static int
have_a_search_match(const char *valname,const char *atname)
{
    /*  valname may have had quotes inserted, but search_match_text
        will not. So we need to use a new copy, not valname here.
        */
    struct esb_s esb_match;
    char *s2;
    esb_constructor(&esb_match);

    trim_quotes(valname,&esb_match);
    s2 = esb_get_string(&esb_match); 
    if (search_match_text ) {
        if(!strcmp(s2,search_match_text) ||
            !strcmp(atname,search_match_text)) {

            esb_destructor(&esb_match);
            return TRUE;
        }
    } 
    if (search_any_text) { 
        if(is_strstrnocase(s2,search_any_text) ||
            is_strstrnocase(atname,search_any_text)) {

            esb_destructor(&esb_match);
            return TRUE;
        }
    }
#ifdef HAVE_REGEX
    if (search_regex_text) { 
        if(!regexec(&search_re,s2,0,NULL,0) ||
            !regexec(&search_re,atname,0,NULL,0)) {

            esb_destructor(&esb_match);
            return TRUE;
        }
    }
#endif
    esb_destructor(&esb_match);
    return FALSE;
}


static boolean
print_attribute(Dwarf_Debug dbg, Dwarf_Die die, 
    Dwarf_Half attr,
    Dwarf_Attribute attr_in,
    boolean print_information,
    int die_indent_level,
    char **srcfiles, Dwarf_Signed cnt)
{
    Dwarf_Attribute attrib = 0;
    Dwarf_Unsigned uval = 0;
    const char * atname = 0;
    const char * valname = 0;
    int tres = 0;
    Dwarf_Half tag = 0;
    int append_extra_string = 0;
    boolean found_search_attr = FALSE;
    boolean bTextFound = FALSE;
    Dwarf_Bool is_info = FALSE;

    is_info = dwarf_get_die_infotypes_flag(die);
    esb_empty_string(&esb_extra);
    atname = get_AT_name(attr,dwarf_names_print_on_error);

    /*  The following gets the real attribute, even in the face of an 
        incorrect doubling, or worse, of attributes. */
    attrib = attr_in;
    /*  Do not get attr via dwarf_attr: if there are (erroneously) 
        multiple of an attr in a DIE, dwarf_attr will not get the
        second, erroneous one and dwarfdump will print the first one
        multiple times. Oops. */

    tres = dwarf_tag(die, &tag, &err);
    if (tres == DW_DLV_ERROR) {
        tag = 0;
    } else if (tres == DW_DLV_NO_ENTRY) {
        tag = 0;
    } else {
        /* ok */
    }
    if (check_attr_tag && checking_this_compiler()) {
        const char *tagname = "<tag invalid>";

        DWARF_CHECK_COUNT(attr_tag_result,1);
        if (tres == DW_DLV_ERROR) {
            DWARF_CHECK_ERROR3(attr_tag_result,tagname,
                get_AT_name(attr,dwarf_names_print_on_error),
                "check the tag-attr combination, dwarf_tag failed.");
        } else if (tres == DW_DLV_NO_ENTRY) {
            DWARF_CHECK_ERROR3(attr_tag_result,tagname,
                get_AT_name(attr,dwarf_names_print_on_error),
                "check the tag-attr combination, dwarf_tag NO ENTRY?.");
        } else if (legal_tag_attr_combination(tag, attr)) {
            /* OK */
        } else {
            tagname = get_TAG_name(tag,dwarf_names_print_on_error);
            tag_specific_checks_setup(tag,die_stack_indent_level);
            DWARF_CHECK_ERROR3(attr_tag_result,tagname,
                get_AT_name(attr,dwarf_names_print_on_error),
                "check the tag-attr combination");
        }
    }

    switch (attr) {
    case DW_AT_language:
        get_small_encoding_integer_and_name(dbg, attrib, &uval,
            "DW_AT_language", &valname,
            get_LANG_name, &err,
            show_form_used);
        break;
    case DW_AT_accessibility:
        get_small_encoding_integer_and_name(dbg, attrib, &uval,
            "DW_AT_accessibility",
            &valname, get_ACCESS_name,
            &err,
            show_form_used);
        break;
    case DW_AT_visibility:
        get_small_encoding_integer_and_name(dbg, attrib, &uval,
            "DW_AT_visibility",
            &valname, get_VIS_name,
            &err,
            show_form_used);
        break;
    case DW_AT_virtuality:
        get_small_encoding_integer_and_name(dbg, attrib, &uval,
            "DW_AT_virtuality",
            &valname,
            get_VIRTUALITY_name, &err,
            show_form_used);
        break;
    case DW_AT_identifier_case:
        get_small_encoding_integer_and_name(dbg, attrib, &uval,
            "DW_AT_identifier",
            &valname, get_ID_name,
            &err,
            show_form_used);
        break;
    case DW_AT_inline:
        get_small_encoding_integer_and_name(dbg, attrib, &uval,
            "DW_AT_inline", &valname,
            get_INL_name, &err,
            show_form_used);
        break;
    case DW_AT_encoding:
        get_small_encoding_integer_and_name(dbg, attrib, &uval,
            "DW_AT_encoding", &valname,
            get_ATE_name, &err,
            show_form_used);
        break;
    case DW_AT_ordering:
        get_small_encoding_integer_and_name(dbg, attrib, &uval,
            "DW_AT_ordering", &valname,
            get_ORD_name, &err,
            show_form_used);
        break;
    case DW_AT_calling_convention:
        get_small_encoding_integer_and_name(dbg, attrib, &uval,
            "DW_AT_calling_convention",
            &valname, get_CC_name,
            &err, 
            show_form_used);
        break;
    case DW_AT_discr_list:      /* DWARF3 */
        get_small_encoding_integer_and_name(dbg, attrib, &uval,
            "DW_AT_discr_list",
            &valname, get_DSC_name,
            &err, 
            show_form_used);
        break;
    case DW_AT_data_member_location:
        {   
            /*  Value is a constant or a location 
                description or location list. 
                If a constant, it could be signed or
                unsigned.  Telling whether a constant
                or a reference is nontrivial 
                since DW_FORM_data{4,8}
                could be either in DWARF{2,3}  */
            enum Dwarf_Form_Class fc = DW_FORM_CLASS_UNKNOWN;
            Dwarf_Half theform = 0;
            Dwarf_Half directform = 0;
            Dwarf_Half version = 0;
            Dwarf_Half offset_size = 0;
            int wres = 0;

            get_form_values(attrib,&theform,&directform);
            wres = dwarf_get_version_of_die(die ,
                &version,&offset_size);
            if(wres != DW_DLV_OK) {
                print_error(dbg,"Cannot get DIE context version number",wres,err);
                break;
            }
            fc = dwarf_get_form_class(version,attr,offset_size,theform); 
            if(fc == DW_FORM_CLASS_CONSTANT) {
                esb_empty_string(&esb_base);
                wres = formxdata_print_value(dbg,attrib,&esb_base, 
                    &err, FALSE);
                show_form_itself(show_form_used,verbose, theform, 
                    directform,&esb_base);
                valname = esb_get_string(&esb_base);
                if(wres == DW_DLV_OK){
                    /* String appended already. */
                    break;
                } else if (wres == DW_DLV_NO_ENTRY) {
                    print_error(dbg,"Cannot get DW_AT_data_member_location, how can it be NO_ENTRY? ",wres,err);
                    break;
                } else {
                    print_error(dbg,"Cannot get DW_AT_data_member_location ",wres,err);
                    break;
                }
            }
            /*  FALL THRU, this is a 
                a location description, or a reference
                to one, or a mistake. */
        }
        /*  FALL THRU to location description */
    case DW_AT_location:
    case DW_AT_vtable_elem_location:
    case DW_AT_string_length:
    case DW_AT_return_addr:
    case DW_AT_use_location:
    case DW_AT_static_link:
    case DW_AT_frame_base:
        {
            /*  The value is a location description 
                or location list. */
               
            Dwarf_Half theform = 0;
            Dwarf_Half directform = 0;
            get_form_values(attrib,&theform,&directform);
            esb_empty_string(&esb_base);
            if(is_location_form(theform)) {
                get_location_list(dbg, die, attrib, &esb_base);
                show_form_itself(show_form_used,verbose, 
                    theform, directform,&esb_base);
            } else if (theform == DW_FORM_exprloc)  {
                int showhextoo = 1;
                print_exprloc_content(dbg,die,attrib,showhextoo,&esb_base);
            } else {
                show_attr_form_error(dbg,attr,theform,&esb_base);
            }
            valname = esb_get_string(&esb_base);
        }
        break;
    case DW_AT_SUN_func_offsets:
        {
            /* value is a location description or location list */
            Dwarf_Half theform = 0;
            Dwarf_Half directform = 0;
            get_form_values(attrib,&theform,&directform);
            esb_empty_string(&esb_base);
            get_FLAG_BLOCK_string(dbg, attrib,&esb_base);
            show_form_itself(show_form_used,verbose, theform, 
                directform,&esb_base);
            valname = esb_get_string(&esb_base);
        }
        break;
    case DW_AT_SUN_cf_kind:
        {
            Dwarf_Half kind = 0;
            Dwarf_Unsigned tempud = 0;
            Dwarf_Error err = 0;
            int wres = 0;
            Dwarf_Half theform = 0;
            Dwarf_Half directform = 0;
            get_form_values(attrib,&theform,&directform);
            wres = dwarf_formudata (attrib,&tempud, &err);
            esb_empty_string(&esb_base);
            if(wres == DW_DLV_OK) {
                kind = tempud;
                esb_append(&esb_base, 
                    get_ATCF_name(kind,dwarf_names_print_on_error));
            } else if (wres == DW_DLV_NO_ENTRY) {
                esb_append(&esb_base,  "?");
            } else {
                print_error(dbg,"Cannot get formudata....",wres,err);
                esb_append(&esb_base,  "??");
            }
            show_form_itself(show_form_used,verbose, theform, 
                directform,&esb_base);
            valname = esb_get_string(&esb_base);
        }
        break;
    case DW_AT_upper_bound:
        {
            Dwarf_Half theform;
            int rv;
            rv = dwarf_whatform(attrib,&theform,&err);
            /* depending on the form and the attribute, process the form */
            if(rv == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_whatform Cannot find attr form",
                    rv, err);
            } else if (rv == DW_DLV_NO_ENTRY) {
                break;
            }

            esb_empty_string(&esb_base);
            switch (theform) {
            case DW_FORM_block1: {
                Dwarf_Half theform = 0;
                Dwarf_Half directform = 0;
                get_form_values(attrib,&theform,&directform);
                get_location_list(dbg, die, attrib, &esb_base);
                show_form_itself(show_form_used,verbose, theform, 
                    directform,&esb_base);
                valname = esb_get_string(&esb_base);
                }
                break;
            default:
                get_attr_value(dbg, tag, die,
                    attrib, srcfiles, cnt, &esb_base, show_form_used,verbose);
                valname = esb_get_string(&esb_base);
                break;
            }
            break;
        }
    case DW_AT_low_pc:  
    case DW_AT_high_pc:
        {
            Dwarf_Half theform;
            int rv;
            rv = dwarf_whatform(attrib,&theform,&err);
            /*  Depending on the form and the attribute, 
                process the form. */
            if(rv == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_whatform cannot Find attr form",
                    rv, err);
            } else if (rv == DW_DLV_NO_ENTRY) {
                break;
            }
            esb_empty_string(&esb_base);
            if( theform != DW_FORM_addr) {
                /*  New in DWARF4: other forms are not an address
                    but are instead offset from pc.
                    One could test for DWARF4 here before adding
                    this string, but that seems unnecessary as this
                    could not happen with DWARF3 or earlier. 
                    A normal consumer would have to add this value to
                    DW_AT_low_pc to get a true pc. */
                esb_append(&esb_base,"<offset-from-lowpc>");
            }
            get_attr_value(dbg, tag, die, attrib, srcfiles, cnt, 
                &esb_base,show_form_used,verbose);
            valname = esb_get_string(&esb_base);

            /* Update base and high addresses for CU */
            if (seen_CU && (need_CU_base_address || 
                need_CU_high_address)) {

                /* Update base address for CU */
                if (need_CU_base_address && attr == DW_AT_low_pc) {
                    dwarf_formaddr(attrib, &CU_base_address, &err);
                    need_CU_base_address = FALSE;
                }

                /* Update high address for CU */
                if (need_CU_high_address && attr == DW_AT_high_pc) {
                    dwarf_formaddr(attrib, &CU_high_address, &err);
                    need_CU_high_address = FALSE;
                }
            }

            /* Record the low and high addresses as we have them */
            if ((check_decl_file || check_ranges || 
                check_locations) && theform == DW_FORM_addr) {
                Dwarf_Addr addr = 0;
                dwarf_formaddr(attrib, &addr, &err);
                if (attr == DW_AT_low_pc) {
                    lowAddr = addr;
                    bSawLow = TRUE;
                    /*  Record the base address of the last seen PU
                        to be used when checking line information */
                    if (seen_PU && !seen_PU_base_address) {
                        seen_PU_base_address = TRUE;
                        PU_base_address = addr;
                    }
                } else {
                    highAddr = addr;
                    bSawHigh = TRUE;
                    /*  Record the high address of the last seen PU
                        to be used when checking line information */
                    if (seen_PU && !seen_PU_high_address) {
                        seen_PU_high_address = TRUE;
                        PU_high_address = addr;
                    }
                }
      
                /* We have now both low_pc and high_pc values */
                if (bSawLow && bSawHigh) {

                    /*  We need to decide if this PU is 
                        valid, as the SN Linker marks a stripped 
                        function by setting lowpc to -1;
                        also for discarded comdat, both lowpc 
                        and highpc are zero */
                    if (need_PU_valid_code) {
                        need_PU_valid_code = FALSE;
                
                        /*  To ignore a PU as invalid code, 
                            only consider the lowpc and
                            highpc values associated with the 
                            DW_TAG_subprogram; other
                            instances of lowpc and highpc, 
                            must be ignore (lexical blocks) */
                        in_valid_code = TRUE;
                        if (IsInvalidCode(lowAddr,highAddr) && 
                            tag == DW_TAG_subprogram) {
                            in_valid_code = FALSE;
                        }
                    }

                    /*  We have a low_pc/high_pc pair; 
                        check if they are valid */
                    if (in_valid_code) {
                        DWARF_CHECK_COUNT(ranges_result,1);
                        if (lowAddr != elf_max_address && 
                            lowAddr > highAddr) {
                            DWARF_CHECK_ERROR(ranges_result,
                                ".debug_info: Incorrect values "
                                "for low_pc/high_pc");
                            if (check_verbose_mode) {
                                printf("Low = 0x%" DW_PR_XZEROS DW_PR_DUx 
                                    ", High = 0x%" DW_PR_XZEROS DW_PR_DUx "\n",
                                    lowAddr,highAddr);
                            }
                        }
                        if (check_decl_file || check_ranges || 
                            check_locations) {
                            AddEntryIntoBucketGroup(pRangesInfo,0,lowAddr,
                                lowAddr,highAddr,NULL,FALSE);
                        }
                    }
                    bSawLow = FALSE;
                    bSawHigh = FALSE;
                }
            }
        }
        break;
    case DW_AT_ranges:
        {
            Dwarf_Half theform = 0;
            int rv;

            rv = dwarf_whatform(attrib,&theform,&err);
            if(rv == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_whatform cannot find Attr Form",
                    rv, err);
            } else if (rv == DW_DLV_NO_ENTRY) {
                break;
            }

            esb_empty_string(&esb_base);
            get_attr_value(dbg, tag,die, attrib, srcfiles, cnt, &esb_base,
                show_form_used,verbose);
            print_range_attribute(dbg, die, attr,attr_in, theform,
                dwarf_names_print_on_error,print_information,
                &append_extra_string);
            valname = esb_get_string(&esb_base);
        }
        break;
    case DW_AT_MIPS_linkage_name:
        esb_empty_string(&esb_base);
        get_attr_value(dbg, tag, die, attrib, srcfiles, 
            cnt, &esb_base, show_form_used,verbose);
        valname = esb_get_string(&esb_base);

        if (check_locations || check_ranges) {
            int local_show_form = 0;
            int local_verbose = 0;
            struct esb_s lesb;
            const char *name = 0;
            esb_constructor(&lesb);
            get_attr_value(dbg, tag, die, attrib, srcfiles, cnt, 
                &lesb, local_show_form,local_verbose);
            /*  Look for specific name forms, attempting to
                notice and report 'odd' identifiers. */
            name = esb_get_string(&lesb);
            safe_strcpy(PU_name,sizeof(PU_name),name,strlen(name));
        }
        break;
    case DW_AT_name:
    case DW_AT_GNU_template_name:
        esb_empty_string(&esb_base);
        get_attr_value(dbg, tag, die, attrib, srcfiles, cnt, 
            &esb_base, show_form_used,verbose);
        valname = esb_get_string(&esb_base);

        if (check_names && checking_this_compiler()) {
            int local_show_form = FALSE;
            int local_verbose = 0;
            struct esb_s lesb;
            const char *name = 0;
            esb_constructor(&lesb);
            get_attr_value(dbg, tag, die, attrib, srcfiles, cnt, 
                &lesb, local_show_form,local_verbose);
            /*  Look for specific name forms, attempting to
                notice and report 'odd' identifiers. */
            name = esb_get_string(&lesb);
            DWARF_CHECK_COUNT(names_result,1);
            if (!strcmp("\"(null)\"",name)) {
                DWARF_CHECK_ERROR(names_result,
                    "string attribute is \"(null)\".");
            } else {
                if (!dot_ok_in_identifier(tag,die,name) 
                    && !need_CU_name && strchr(name,'.')) {
                    /*  This is a suggestion there 'might' be
                        a surprising name, not a guarantee of an
                        error. */
                    DWARF_CHECK_ERROR(names_result,
                        "string attribute is invalid.");
                }
            }
            esb_destructor(&lesb);
        }

        /* If we are in checking mode and we do not have a PU name */
        if ((check_locations || check_ranges) && seen_PU && !PU_name[0]) {
            int local_show_form = FALSE;
            int local_verbose = 0;
            const char *name = 0;
            struct esb_s lesb;
            esb_constructor(&lesb);
            get_attr_value(dbg, tag, die, attrib, srcfiles, cnt, 
                &lesb, local_show_form,local_verbose);
            name = esb_get_string(&lesb);
            
            safe_strcpy(PU_name,sizeof(PU_name),name,strlen(name));
            esb_destructor(&lesb);
        }

        /* If we are processing the compile unit, record the name */
        if (seen_CU && need_CU_name) {
            /* Lets not get the form name included. */
            struct esb_s lesb;
            int local_show_form_used = FALSE;
            int local_verbose = 0;
            char *localname = 0;
            esb_constructor(&lesb);
            get_attr_value(dbg, tag, die, attrib, srcfiles, cnt, 
                &lesb, local_show_form_used,local_verbose);
            localname = esb_get_string(&lesb);
            safe_strcpy(CU_name,sizeof(CU_name),localname,strlen(localname));
            need_CU_name = FALSE;
            esb_destructor(&lesb);
        }
        break;
    
    case DW_AT_producer:
        esb_empty_string(&esb_base);
        get_attr_value(dbg, tag, die, attrib, srcfiles, cnt, 
            &esb_base, show_form_used,verbose);
        valname = esb_get_string(&esb_base);
        /* If we are in checking mode, identify the compiler */
        if (do_check_dwarf || search_is_on) {
            /*  Do not use show-form here! We just want the producer name, not
                the form name. */
            int show_form_local = FALSE;
            int local_verbose = 0;
            struct esb_s local_e;
            esb_constructor(&local_e);
            get_attr_value(dbg, tag, die, attrib, srcfiles, cnt, 
                &local_e, show_form_local,local_verbose);
            /* Check if this compiler version is a target */
            update_compiler_target(esb_get_string(&local_e));
            esb_destructor(&local_e);
        }
        break;


    /*  When dealing with linkonce symbols, the low_pc and high_pc
        are associated with a specific symbol; SNC always generate a name in
        the for of DW_AT_MIPS_linkage_name; GCC does not; instead it generates
        DW_AT_abstract_origin or DW_AT_specification; in that case we have to
        traverse this attribute in order to get the name for the linkonce */
    case DW_AT_specification:
    case DW_AT_abstract_origin:
    case DW_AT_type:
        esb_empty_string(&esb_base);
        get_attr_value(dbg, tag, die, attrib, srcfiles, cnt, &esb_base, 
            show_form_used,verbose);
        valname = esb_get_string(&esb_base);
        if (check_forward_decl || check_self_references) {
            Dwarf_Off die_off = 0;
            Dwarf_Off ref_off = 0;
            int res = 0;
            int suppress_check = 0;
      
            /* Get the global offset for reference */
            res = dwarf_global_formref(attrib, &ref_off, &err);
            if (res != DW_DLV_OK) {
                int myerr = dwarf_errno(err);
                if(myerr == DW_DLE_REF_SIG8_NOT_HANDLED) {
                    /*  DW_DLE_REF_SIG8_NOT_HANDLED */
                    /*  No offset available, it makes little sense
                        to delve into this sort of reference unless
                        we think a graph of self-refs *across*
                        type-units is possible. Hmm. FIXME? */
                    suppress_check = 1 ;
                    dwarf_dealloc(dbg,err,DW_DLA_ERROR);
                    err = 0;
                } else {
                    print_error(dbg, "dwarf_die_CU_offsetD", res, err);
                } 
            }
            res = dwarf_dieoffset(die, &die_off, &err);
            if (res != DW_DLV_OK) {
                print_error(dbg, "ref formwith no ref?!", res, err);
            }

            if (!suppress_check && check_self_references) {
                Dwarf_Die ref_die = 0;

                ResetBucketGroup(pVisitedInfo);
                AddEntryIntoBucketGroup(pVisitedInfo,die_off,0,0,0,NULL,FALSE);

                /* Follow reference chain, looking for self references */
                res = dwarf_offdie_b(dbg,ref_off,is_info,&ref_die,&err);
                if (res == DW_DLV_OK) {
                    ++die_indent_level;
                    struct esb_s copy_base;
                    if (dump_visited_info) {
                        Dwarf_Off off;
                        dwarf_die_CU_offset(die, &off, &err);
                        printf("<%2d><0x%" DW_PR_XZEROS DW_PR_DUx 
                            " GOFF=0x%" DW_PR_XZEROS DW_PR_DUx "> ", 
                            die_indent_level, (Dwarf_Unsigned)off,
                            (Dwarf_Unsigned)die_off);
                        printf("%*s%s -> %s\n",die_indent_level * 2 + 2,
                            " ",atname,valname);
                    }
                    /*  Because esb_base is global, lets not
                        let the traversal trash what we have here. */
                    esb_constructor(&copy_base); 
                    esb_append(&copy_base,esb_get_string(&esb_base));
                    esb_empty_string(&esb_base);
                    traverse_one_die(dbg,attrib,ref_die,srcfiles,cnt,die_indent_level);
                    esb_empty_string(&esb_base);
                    esb_append(&esb_base,esb_get_string(&copy_base));
                    esb_destructor(&copy_base);
                    dwarf_dealloc(dbg,ref_die,DW_DLA_DIE);
                    ref_die = 0;
                    --die_indent_level;
                }
                DeleteKeyInBucketGroup(pVisitedInfo,die_off);
            }

            if (!suppress_check && check_forward_decl) {
                if (attr == DW_AT_specification) {
                    /*  Check the DW_AT_specification does not make forward
                        references to DIEs. 
                        DWARF4 specifications, section 2.13.2,
                        but really they are legal, 
                        this test is probably wrong. */
                    DWARF_CHECK_COUNT(forward_decl_result,1);
                    if (ref_off > die_off) {
                        DWARF_CHECK_ERROR2(forward_decl_result,
                            "Invalid forward reference to DIE: ",valname);
                    }
                }
            }
        }
        /* If we are in checking mode and we do not have a PU name */
        if ((check_locations || check_ranges) && seen_PU && !PU_name[0]) {
            if (tag == DW_TAG_subprogram) {
                /* This gets the DW_AT_name if this DIE has one. */
                Dwarf_Addr low_pc =  0;
                static char proc_name[BUFSIZ];
                proc_name[0] = 0;
                get_proc_name(dbg,die,low_pc,proc_name,BUFSIZ,/*pcMap=*/0);
                if (proc_name[0]) {
                    safe_strcpy(PU_name,sizeof(PU_name),proc_name,
                        strlen(proc_name));
                }
            }
        }
        break;
    default:
        esb_empty_string(&esb_base);
        get_attr_value(dbg, tag,die, attrib, srcfiles, cnt, &esb_base,
            show_form_used,verbose);
        valname = esb_get_string(&esb_base);
        break;
    }
    if (!print_information) {
        if(have_a_search_match(valname,atname) )
        {
            if (search_wide_format) {
                found_search_attr = TRUE;
            } else {
                PRINT_CU_INFO();
                bTextFound = TRUE;
            }
        }
    }
    if ((PRINTING_DIES && print_information) || bTextFound) {
        /*  Print just the Tags and Attributes */
        if (!display_offsets) {
            printf("%-28s\n",atname);
        } else {
            if (dense) {
                printf(" %s<%s>", atname, valname);
                if(append_extra_string) {
                    char *v = esb_get_string(&esb_extra);
                    printf("%s", v);
                }
            } else {
                printf("%-28s%s\n", atname, valname);
                if( append_extra_string) {
                    char *v = esb_get_string(&esb_extra);
                    printf("%s", v);
                }
            }
        }
        /*  Due to redirection of stderr */
        fflush(stdout);
        bTextFound = FALSE;
    }
    return found_search_attr;
}


int
dwarfdump_print_one_locdesc(Dwarf_Debug dbg,
    Dwarf_Locdesc * llbuf,
    int skip_locdesc_header,
    struct esb_s *string_out)
{

    Dwarf_Locdesc *locd = 0;
    Dwarf_Half no_of_ops = 0;
    int i = 0;
    char small_buf[100];

    if (!skip_locdesc_header && (verbose || llbuf->ld_from_loclist)) {
        snprintf(small_buf, sizeof(small_buf), 
            "<lowpc=0x%" DW_PR_XZEROS DW_PR_DUx ">",
            (Dwarf_Unsigned) llbuf->ld_lopc);
        esb_append(string_out, small_buf);


        snprintf(small_buf, sizeof(small_buf), 
            "<highpc=0x%" DW_PR_XZEROS DW_PR_DUx ">",
            (Dwarf_Unsigned) llbuf->ld_hipc);
        esb_append(string_out, small_buf);
        if (display_offsets && verbose) {
            snprintf(small_buf, sizeof(small_buf),
                "<from %s offset 0x%" DW_PR_XZEROS  DW_PR_DUx ">",
                llbuf->ld_from_loclist ? ".debug_loc" : ".debug_info",
                llbuf->ld_section_offset);
            esb_append(string_out, small_buf);
        }
    }

    locd = llbuf;
    no_of_ops = llbuf->ld_cents;
    for (i = 0; i < no_of_ops; i++) {
        Dwarf_Loc * op = &locd->ld_s[i];

        int res = _dwarf_print_one_expr_op(dbg,op,i,string_out);
        if(res == DW_DLV_ERROR) {
            return res;
        }
    }
    return DW_DLV_OK;
}



static int
op_has_no_operands(int op)
{
    unsigned i = 0;
    if(op >= DW_OP_lit0 && op <= DW_OP_reg31) {
        return TRUE;
    }
    for( ;  ; ++i) {
        struct operation_descr_s *odp = opdesc+i;
        if(odp->op_code == 0) {
            break;
        }
        if(odp->op_code != op) {
            continue;
        }
        if (odp->op_count == 0) {
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

int
_dwarf_print_one_expr_op(Dwarf_Debug dbg,Dwarf_Loc* expr,int index,
    struct esb_s *string_out)
{
    /*  local_space_needed is intended to be 'more than big enough'
        for a short group of loclist entries.  */
    char small_buf[100];
    Dwarf_Small op;
    Dwarf_Unsigned opd1;  
    Dwarf_Unsigned opd2;
    const char * op_name;

    if (index > 0) {
        esb_append(string_out, " ");
    }

    op = expr->lr_atom;

    /*  We have valid operands whose values are bigger than the
        DW_OP_nop = 0x96; for example: DW_OP_GNU_push_tls_address = 0xe0
        Also, the function 'get_OP_name' handles this case, generating a
        name 'Unknown OP value'.  */
    if (op > DW_OP_hi_user) {
        print_error(dbg, "dwarf_op unexpected value!", DW_DLV_OK,
            err);
        return DW_DLV_ERROR;
    }
    op_name = get_OP_name(op,dwarf_names_print_on_error);
    esb_append(string_out, op_name);

    opd1 = expr->lr_number;
    if(op_has_no_operands(op)) {
        /* Nothing to add. */
    } else if (op >= DW_OP_breg0 && op <= DW_OP_breg31) {
        snprintf(small_buf, sizeof(small_buf),
            "%+" DW_PR_DSd , (Dwarf_Signed) opd1);
        esb_append(string_out, small_buf);
    } else {
        switch (op) {
        case DW_OP_addr:
            snprintf(small_buf, sizeof(small_buf), 
                " 0x%" DW_PR_XZEROS DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_const1s:
        case DW_OP_const2s:
        case DW_OP_const4s:
        case DW_OP_const8s:
        case DW_OP_consts:
        case DW_OP_skip:
        case DW_OP_bra:
        case DW_OP_fbreg:
            snprintf(small_buf, sizeof(small_buf),
                " %" DW_PR_DSd, (Dwarf_Signed) opd1);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_const1u:
        case DW_OP_const2u:
        case DW_OP_const4u:
        case DW_OP_const8u:
        case DW_OP_constu:
        case DW_OP_pick:
        case DW_OP_plus_uconst:
        case DW_OP_regx:
        case DW_OP_piece:
        case DW_OP_deref_size:
        case DW_OP_xderef_size:
            snprintf(small_buf, sizeof(small_buf), 
                " %" DW_PR_DUu , opd1);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_bregx:
            snprintf(small_buf, sizeof(small_buf), 
                "0x%08" DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            opd2 = expr->lr_number2;
            snprintf(small_buf, sizeof(small_buf),
                "+%" DW_PR_DSd , (Dwarf_Signed) opd2);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_call2:
            snprintf(small_buf, sizeof(small_buf), 
                " 0x%" DW_PR_XZEROS DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_call4:
            snprintf(small_buf, sizeof(small_buf), 
                " 0x%" DW_PR_XZEROS DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_call_ref:
            snprintf(small_buf, sizeof(small_buf), 
                " 0x%" DW_PR_XZEROS DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_bit_piece:
            snprintf(small_buf, sizeof(small_buf), 
                " 0x%" DW_PR_XZEROS  DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            opd2 = expr->lr_number2;
            snprintf(small_buf, sizeof(small_buf),
                " offset 0x%" DW_PR_DUx , (Dwarf_Signed) opd2);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_implicit_value:
            { 
#define IMPLICIT_VALUE_PRINT_MAX 12
                unsigned int print_len = 0;
                snprintf(small_buf, sizeof(small_buf), 
                    " 0x%" DW_PR_XZEROS DW_PR_DUx , opd1);
                esb_append(string_out, small_buf);
                /*  The other operand is a block of opd1 bytes. */
                /*  FIXME */
                print_len = opd1;
                if(print_len > IMPLICIT_VALUE_PRINT_MAX) {
                    print_len = IMPLICIT_VALUE_PRINT_MAX;
                }
#undef IMPLICIT_VALUE_PRINT_MAX
                if(print_len > 0) {
                    const unsigned char *bp = 0;
                    unsigned int i = 0;
                    opd2 = expr->lr_number2;
                    /*  This is a really ugly cast, a way
                        to implement DW_OP_implicit value in
                        this libdwarf context. */
                    bp = (const unsigned char *) opd2;
                    esb_append(string_out," contents 0x");
                    for( ; i < print_len; ++i,++bp) {
                        /*  Do not use DW_PR_DUx here,
                            the value  *bp is a const unsigned char. */
                        snprintf(small_buf, sizeof(small_buf), 
                            "%02x", *bp);
                        esb_append(string_out,small_buf);
                    }
                }
            }
            break;

        /* We do not know what the operands, if any, are. */
        case DW_OP_HP_unknown:
        case DW_OP_HP_is_value:
        case DW_OP_HP_fltconst4:
        case DW_OP_HP_fltconst8:
        case DW_OP_HP_mod_range:
        case DW_OP_HP_unmod_range:
        case DW_OP_HP_tls:
        case DW_OP_INTEL_bit_piece:
            break;
        case DW_OP_stack_value:  /* DWARF4 */
            break;
        case DW_OP_GNU_uninit:  /* DW_OP_APPLE_uninit */
            /* No operands. */ 
            break;  
        case DW_OP_GNU_encoded_addr: 
            snprintf(small_buf, sizeof(small_buf), 
                " 0x%" DW_PR_XZEROS  DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            break;  
        case DW_OP_GNU_implicit_pointer:
            snprintf(small_buf, sizeof(small_buf), 
                " 0x%" DW_PR_XZEROS  DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            break;  
        case DW_OP_GNU_entry_value:
            snprintf(small_buf, sizeof(small_buf), 
                " 0x%" DW_PR_XZEROS  DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            break;      
        default:
            {
                snprintf(small_buf, sizeof(small_buf),
                    " dwarf_op unknown 0x%x", (unsigned)op);
                esb_append(string_out,small_buf);
            }
            break;
        }
    }
    return DW_DLV_OK;
}

/*  Fill buffer with location lists 
    Buffer esbp expands as needed.
*/
/*ARGSUSED*/ static void
get_location_list(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Attribute attr,
    struct esb_s *esbp)
{
    Dwarf_Locdesc *llbuf = 0;
    Dwarf_Locdesc **llbufarray = 0;
    Dwarf_Signed no_of_elements;
    Dwarf_Error err;
    int i;
    int lres = 0;
    int llent = 0;
    int skip_locdesc_header = 0;

    /* Base address used to update entries in .debug_loc */
    Dwarf_Addr base_address = CU_base_address;
    Dwarf_Addr lopc = 0;
    Dwarf_Addr hipc = 0;
    Dwarf_Bool bError = FALSE;

    if (use_old_dwarf_loclist) {
        lres = dwarf_loclist(attr, &llbuf, &no_of_elements, &err);
        if (lres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_loclist", lres, err);
        } else if (lres == DW_DLV_NO_ENTRY) {
            return;
        }
        dwarfdump_print_one_locdesc(dbg, llbuf,skip_locdesc_header,esbp);
        dwarf_dealloc(dbg, llbuf->ld_s, DW_DLA_LOC_BLOCK);
        dwarf_dealloc(dbg, llbuf, DW_DLA_LOCDESC);
        return;
    }

    lres = dwarf_loclist_n(attr, &llbufarray, &no_of_elements, &err);
    if (lres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_loclist", lres, err);
    } else if (lres == DW_DLV_NO_ENTRY) {
        return;
    }
    for (llent = 0; llent < no_of_elements; ++llent) {
        char small_buf[100];
        Dwarf_Off offset = 0;

        llbuf = llbufarray[llent];
        /*  If we have a location list refering to the .debug_loc
            Check for specific compiler we are validating. */
        if (check_locations && in_valid_code && 
            llbuf->ld_from_loclist && checking_this_compiler()) {
            /*  To calculate the offset, we use:
                sizeof(Dwarf_Half) -> number of expression list
                2 * address_size -> low_pc and high_pc */
            offset = llbuf->ld_section_offset - 
                llbuf->ld_cents * sizeof(Dwarf_Half) - 
                2 * elf_address_size;

            if (llbuf->ld_lopc == elf_max_address) {
                /*  (0xffffffff,addr), use specific address 
                    (current PU address) */
                base_address = llbuf->ld_hipc;
            } else {
                /* (offset,offset), update using CU address */
                lopc = llbuf->ld_lopc + base_address;
                hipc = llbuf->ld_hipc + base_address;

                DWARF_CHECK_COUNT(locations_result,1);

                /*  Check the low_pc and high_pc are within 
                    a valid range in the .text section */
                if (IsValidInBucketGroup(pRangesInfo,lopc) && 
                    IsValidInBucketGroup(pRangesInfo,hipc)) {
                    /* Valid values; do nothing */
                } else {
                    /*  At this point may be we are dealing with 
                        a linkonce symbol */
                    if (IsValidInLinkonce(pLinkonceInfo,PU_name,
                        lopc,hipc)) {
                        /* Valid values; do nothing */
                    } else {
                        bError = TRUE;
                        DWARF_CHECK_ERROR(locations_result,
                            ".debug_loc: Address outside a "
                            "valid .text range");
                        if (check_verbose_mode) {
                            printf(
                                "Offset = 0x%" DW_PR_XZEROS DW_PR_DUx 
                                ", Base = 0x%"  DW_PR_XZEROS DW_PR_DUx ", "
                                "Low = 0x%"  DW_PR_XZEROS DW_PR_DUx 
                                " (0x%"  DW_PR_XZEROS DW_PR_DUx 
                                "), High = 0x%"  DW_PR_XZEROS DW_PR_DUx 
                                " (0x%"  DW_PR_XZEROS DW_PR_DUx ")\n",
                                offset,base_address,lopc,
                                llbuf->ld_lopc,
                                hipc,
                                llbuf->ld_hipc);
                        }
                    }
                }
            }
        }

        if (!dense && llbuf->ld_from_loclist) {
            if (llent == 0) {
                snprintf(small_buf, sizeof(small_buf),
                    "<loclist with %ld entries follows>",
                    (long) no_of_elements);
                esb_append(esbp, small_buf);
            }
            esb_append(esbp, "\n\t\t\t");
            snprintf(small_buf, sizeof(small_buf), "[%2d]", llent);
            esb_append(esbp, small_buf);
        }
        lres = dwarfdump_print_one_locdesc(dbg,
            llbuf, 
            skip_locdesc_header,
            esbp);
        if (lres == DW_DLV_ERROR) {
            return;
        } else {
            /*  DW_DLV_OK so we add follow-on at end, else is
                DW_DLV_NO_ENTRY (which is impossible, treat like
                DW_DLV_OK). */
        }
    }

    if (bError && check_verbose_mode) {
        printf("\n");
    }

    for (i = 0; i < no_of_elements; ++i) {
        dwarf_dealloc(dbg, llbufarray[i]->ld_s, DW_DLA_LOC_BLOCK);
        dwarf_dealloc(dbg, llbufarray[i], DW_DLA_LOCDESC);
    }
    dwarf_dealloc(dbg, llbufarray, DW_DLA_LIST);
}

static void
formx_unsigned(Dwarf_Unsigned u, struct esb_s *esbp, Dwarf_Bool hex_format)
{
    char small_buf[40];
    if (hex_format) {
        snprintf(small_buf, sizeof(small_buf),"0x%"  DW_PR_XZEROS DW_PR_DUx , u);
    } else {
        snprintf(small_buf, sizeof(small_buf),
            "%" DW_PR_DUu , u);
    }
    esb_append(esbp, small_buf);
}
static void
formx_signed(Dwarf_Signed u, struct esb_s *esbp)
{
    char small_buf[40];
    snprintf(small_buf, sizeof(small_buf),
        "%" DW_PR_DSd , u);
    esb_append(esbp, small_buf);
}
/*  We think this is an integer. Figure out how to print it.
    In case the signedness is ambiguous (such as on 
    DW_FORM_data1 (ie, unknown signedness) print two ways.
*/
static int
formxdata_print_value(Dwarf_Debug dbg,Dwarf_Attribute attrib, 
    struct esb_s *esbp,
    Dwarf_Error * err, Dwarf_Bool hex_format)
{
    Dwarf_Signed tempsd = 0;
    Dwarf_Unsigned tempud = 0;
    int sres = 0;
    int ures = 0;
    Dwarf_Error serr = 0;

    ures = dwarf_formudata(attrib, &tempud, err);
    sres = dwarf_formsdata(attrib, &tempsd, &serr);
    if(ures == DW_DLV_OK) {
        if(sres == DW_DLV_OK) {
            if(tempud == tempsd && tempsd >= 0) {
                /*  Data is the same value and not negative,
                    so makes no difference which
                    we print. */
                formx_unsigned(tempud,esbp,hex_format);
            } else {
                formx_unsigned(tempud,esbp,hex_format);
                esb_append(esbp,"(as signed = ");
                formx_signed(tempsd,esbp);
                esb_append(esbp,")");
            }
        } else if (sres == DW_DLV_NO_ENTRY) {
            formx_unsigned(tempud,esbp,hex_format);
        } else /* DW_DLV_ERROR */{
            formx_unsigned(tempud,esbp,hex_format);
        }
        goto cleanup;
    } else {
        /* ures ==  DW_DLV_ERROR  or DW_DLV_NO_ENTRY*/ 
        if(sres == DW_DLV_OK) {
            formx_signed(tempsd,esbp);
        } else {
            /* Neither worked. */
        }
    } 
    /*  Clean up any unused Dwarf_Error data. 
        DW_DLV_NO_ENTRY cannot really happen,
        so a complete cleanup for that is
        not necessary. */
    cleanup:
    if(sres == DW_DLV_OK || ures == DW_DLV_OK) {
        if(sres == DW_DLV_ERROR) {
            dwarf_dealloc(dbg,serr,DW_DLA_ERROR);
        }
        if(ures == DW_DLV_ERROR) {
            dwarf_dealloc(dbg,*err,DW_DLA_ERROR);
            *err = 0;
        }
        return DW_DLV_OK;
    }
    if(sres == DW_DLV_ERROR || ures == DW_DLV_ERROR) {
        if(sres == DW_DLV_ERROR && ures == DW_DLV_ERROR) {
            dwarf_dealloc(dbg,serr,DW_DLA_ERROR);
            return DW_DLV_ERROR;
        }
        if(sres == DW_DLV_ERROR) {
            *err = serr;
        }
        return DW_DLV_ERROR;
    }
    /* Both are DW_DLV_NO_ENTRY which is crazy, impossible. */
    return DW_DLV_NO_ENTRY;
}

static char *
get_form_number_as_string(int form, char *buf, unsigned bufsize)
{
    snprintf(buf,bufsize," %d",form);
    return buf;
}

static void
print_exprloc_content(Dwarf_Debug dbg,Dwarf_Die die, Dwarf_Attribute attrib,
    int showhextoo, struct esb_s *esbp)
{   
    Dwarf_Ptr x = 0;
    Dwarf_Unsigned tempud = 0;
    char small_buf[80];
    Dwarf_Error err = 0;
    int wres = 0;
    wres = dwarf_formexprloc(attrib,&tempud,&x,&err);
    if(wres == DW_DLV_NO_ENTRY) {
        /* Show nothing?  Impossible. */
    } else if(wres == DW_DLV_ERROR) {
        print_error(dbg, "Cannot get a  DW_FORM_exprbloc....", wres, err);
    } else {
        Dwarf_Half address_size = 0;
        int ares = 0;
        unsigned u = 0;
        snprintf(small_buf, sizeof(small_buf),
            "len 0x%04" DW_PR_DUx ": ",tempud);
        esb_append(esbp, small_buf);
        if(showhextoo) {
            for (u = 0; u < tempud; u++) {
                snprintf(small_buf, sizeof(small_buf), "%02x",
                    *(u + (unsigned char *) x));
                esb_append(esbp, small_buf);
            }
            esb_append(esbp,": ");
        }
        address_size = 0;
        ares = dwarf_get_die_address_size(die,&address_size,&err);
        if(wres == DW_DLV_NO_ENTRY) {
            print_error(dbg,"Cannot get die address size for exprloc",
                ares,err);
        } else if(wres == DW_DLV_ERROR) {
            print_error(dbg,"Cannot Get die address size for exprloc",
                ares,err);
        } else {
            get_string_from_locs(dbg,x,tempud,address_size, esbp);
        }
    }
}




/*  Fill buffer with attribute value.
    We pass in tag so we can try to do the right thing with
    broken compiler DW_TAG_enumerator 

    We append to esbp's buffer.
*/
void
get_attr_value(Dwarf_Debug dbg, Dwarf_Half tag, 
    Dwarf_Die die, Dwarf_Attribute attrib,
    char **srcfiles, Dwarf_Signed cnt, struct esb_s *esbp,
    int show_form,
    int local_verbose)
{
    Dwarf_Half theform = 0;
    char * temps = 0;
    Dwarf_Block *tempb = 0;
    Dwarf_Signed tempsd = 0;
    Dwarf_Unsigned tempud = 0;
    int i = 0;
    Dwarf_Half attr = 0;
    Dwarf_Off off = 0;
    Dwarf_Off goff = 0; /* Global offset */
    Dwarf_Die die_for_check = 0;
    Dwarf_Half tag_for_check = 0;
    Dwarf_Bool tempbool = 0;
    Dwarf_Addr addr = 0;
    int fres = 0;
    int bres = 0;
    int wres = 0;
    int dres = 0;
    Dwarf_Half direct_form = 0;
    char small_buf[COMPILE_UNIT_NAME_LEN];  /* Size to hold a filename */
    Dwarf_Bool is_info = TRUE;


    is_info = dwarf_get_die_infotypes_flag(die);
    /*  Dwarf_whatform gets the real form, DW_FORM_indir is
        never returned: instead the real form following
        DW_FORM_indir is returned. */
    fres = dwarf_whatform(attrib, &theform, &err);
    /*  Depending on the form and the attribute, process the form. */
    if (fres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_whatform cannot Find Attr Form", fres,
            err);
    } else if (fres == DW_DLV_NO_ENTRY) {
        return;
    }

    /*  dwarf_whatform_direct gets the 'direct' form, so if
        the form is DW_FORM_indir that is what is returned. */
    dwarf_whatform_direct(attrib, &direct_form, &err);
    /*  Ignore errors in dwarf_whatform_direct() */


    switch (theform) {
    case DW_FORM_addr:
        bres = dwarf_formaddr(attrib, &addr, &err);
        if (bres == DW_DLV_OK) {
            snprintf(small_buf, sizeof(small_buf), 
                "0x%" DW_PR_XZEROS DW_PR_DUx ,
                (Dwarf_Unsigned) addr);
            esb_append(esbp, small_buf);
        } else {
            print_error(dbg, "addr formwith no addr?!", bres, err);
        }
        break;
    case DW_FORM_ref_addr:
        /*  DW_FORM_ref_addr is not accessed thru formref: ** it is an
            address (global section offset) in ** the .debug_info
            section. */
        bres = dwarf_global_formref(attrib, &off, &err);
        if (bres == DW_DLV_OK) {
            snprintf(small_buf, sizeof(small_buf),
                "<global die offset 0x%" DW_PR_XZEROS DW_PR_DUx 
                ">",
                (Dwarf_Unsigned) off);
            esb_append(esbp, small_buf);
        } else {
            print_error(dbg,
                "DW_FORM_ref_addr form with no reference?!",
                bres, err);
        }
        wres = dwarf_whatattr(attrib, &attr, &err);
        if (wres == DW_DLV_ERROR) {
        } else if (wres == DW_DLV_NO_ENTRY) {
        } else {
            if (attr == DW_AT_sibling) {
                /*  The value had better be inside the current CU
                    else there is a nasty error here, as a sibling
                    has to be in the same CU, it seems. */
                Dwarf_Off cuoff = 0;
                Dwarf_Off culen = 0;
                int res = dwarf_die_CU_offset_range(die,&cuoff,
                    &culen,&err);
                DWARF_CHECK_COUNT(tag_tree_result,1);
                if(res != DW_DLV_OK) {
                } else {
                    Dwarf_Off cuend = cuoff+culen;
                    if(off <  cuoff || off >= cuend) { 
                        DWARF_CHECK_ERROR(tag_tree_result,
                            "DW_AT_sibling DW_FORM_ref_addr offset points "
                            "outside of current CU");
                    }
                }
            }
        }

        break;
    case DW_FORM_ref1:
    case DW_FORM_ref2:
    case DW_FORM_ref4:
    case DW_FORM_ref8:
    case DW_FORM_ref_udata:
        bres = dwarf_formref(attrib, &off, &err);
        if (bres != DW_DLV_OK) {
            /* Report incorrect offset */
            snprintf(small_buf,sizeof(small_buf),
                "%s, offset=<0x%"  DW_PR_XZEROS  DW_PR_DUx 
                ">","reference form with no valid local ref?!",off);
            print_error(dbg, small_buf, bres, err);
        }

        /* Convert the local offset into a relative section offset */
        if (show_global_offsets) {
            bres = dwarf_convert_to_global_offset(attrib, 
                off, &goff, &err);
            if (bres != DW_DLV_OK) {
                /*  Report incorrect offset */
                snprintf(small_buf,sizeof(small_buf),
                    "%s, global die offset=<0x%"  DW_PR_XZEROS  DW_PR_DUx 
                    ">","invalid offset",goff);
                print_error(dbg, small_buf, bres, err);
            }
        }

        /*  Do references inside <> to distinguish them ** from
            constants. In dense form this results in <<>>. Ugly for
            dense form, but better than ambiguous. davea 9/94 */
        if (show_global_offsets) {
            snprintf(small_buf, sizeof(small_buf), 
                "<0x%"  DW_PR_XZEROS  DW_PR_DUx " GOFF=0x%"  DW_PR_XZEROS  DW_PR_DUx ">", 
            (Dwarf_Unsigned)off, goff);
        } else {
            snprintf(small_buf, sizeof(small_buf), 
                "<0x%" DW_PR_XZEROS DW_PR_DUx ">", off);
        }

        esb_append(esbp, small_buf);
        if (check_type_offset) {
            attr = 0;
            wres = dwarf_whatattr(attrib, &attr, &err);
            if (wres == DW_DLV_ERROR) {
                
            } else if (wres == DW_DLV_NO_ENTRY) {
            }
            if (attr == DW_AT_type) {
                dres = dwarf_offdie_b(dbg, cu_offset + off,
                    is_info,
                    &die_for_check, &err);
                DWARF_CHECK_COUNT(type_offset_result,1);
                if (dres != DW_DLV_OK) {
                    snprintf(small_buf,sizeof(small_buf),
                        "DW_AT_type offset does not point to a DIE for global offset 0x%" DW_PR_DUx " cu off 0x%" DW_PR_DUx " local offset 0x%" DW_PR_DUx,
                        cu_offset + off,cu_offset,off);
                    DWARF_CHECK_ERROR(type_offset_result,small_buf);
                } else {
                    int tres2 =
                        dwarf_tag(die_for_check, &tag_for_check, &err);
                    if (tres2 == DW_DLV_OK) {
                        switch (tag_for_check) {
                        case DW_TAG_array_type:
                        case DW_TAG_class_type:
                        case DW_TAG_enumeration_type:
                        case DW_TAG_pointer_type:
                        case DW_TAG_reference_type:
                        case DW_TAG_string_type:
                        case DW_TAG_structure_type:
                        case DW_TAG_subroutine_type:
                        case DW_TAG_typedef:
                        case DW_TAG_union_type:
                        case DW_TAG_ptr_to_member_type:
                        case DW_TAG_set_type:
                        case DW_TAG_subrange_type:
                        case DW_TAG_base_type:
                        case DW_TAG_const_type:
                        case DW_TAG_file_type:
                        case DW_TAG_packed_type:
                        case DW_TAG_thrown_type:
                        case DW_TAG_volatile_type:
                        case DW_TAG_template_type_parameter:
                        case DW_TAG_template_value_parameter:
                        case DW_TAG_unspecified_type:
                            /* OK */
                            break;
                        default:
                            { 
                                snprintf(small_buf,sizeof(small_buf),
                                    "DW_AT_type offset does not point to Type info we got tag 0x%x %s",
                                tag_for_check,
                                get_TAG_name(tag_for_check,
                                    dwarf_names_print_on_error));
                                DWARF_CHECK_ERROR(type_offset_result,small_buf);
                            }
                            break;
                        }
                        dwarf_dealloc(dbg, die_for_check, DW_DLA_DIE);
                        die_for_check = 0;
                    } else {
                        DWARF_CHECK_ERROR(type_offset_result,
                            "DW_AT_type offset does not exist");
                    }
                }
            }
        }
        break;
    case DW_FORM_block:
    case DW_FORM_block1:
    case DW_FORM_block2:
    case DW_FORM_block4:
        fres = dwarf_formblock(attrib, &tempb, &err);
        if (fres == DW_DLV_OK) {
            for (i = 0; i < tempb->bl_len; i++) {
                snprintf(small_buf, sizeof(small_buf), "%02x",
                    *(i + (unsigned char *) tempb->bl_data));
                esb_append(esbp, small_buf);
            }
            dwarf_dealloc(dbg, tempb, DW_DLA_BLOCK);
            tempb = 0;
        } else {
            print_error(dbg, "DW_FORM_blockn cannot get block\n", fres,
                err);
        }
        break;
    case DW_FORM_data1:
    case DW_FORM_data2:
    case DW_FORM_data4:
    case DW_FORM_data8:
        fres = dwarf_whatattr(attrib, &attr, &err);
        if (fres == DW_DLV_ERROR) {
            print_error(dbg, "FORM_datan cannot get attr", fres, err);
        } else if (fres == DW_DLV_NO_ENTRY) {
            print_error(dbg, "FORM_datan cannot get attr", fres, err);
        } else {
            switch (attr) {
            case DW_AT_ordering:
            case DW_AT_byte_size:
            case DW_AT_bit_offset:
            case DW_AT_bit_size:
            case DW_AT_inline:
            case DW_AT_language:
            case DW_AT_visibility:
            case DW_AT_virtuality:
            case DW_AT_accessibility:
            case DW_AT_address_class:
            case DW_AT_calling_convention:
            case DW_AT_discr_list:      /* DWARF3 */
            case DW_AT_encoding:
            case DW_AT_identifier_case:
            case DW_AT_MIPS_loop_unroll_factor:
            case DW_AT_MIPS_software_pipeline_depth:
            case DW_AT_decl_column:
            case DW_AT_decl_file:
            case DW_AT_decl_line:
            case DW_AT_call_column:
            case DW_AT_call_file:
            case DW_AT_call_line:
            case DW_AT_start_scope:
            case DW_AT_byte_stride:
            case DW_AT_bit_stride:
            case DW_AT_count:
            case DW_AT_stmt_list:
            case DW_AT_MIPS_fde:
                {  int show_form_here = 0;
                wres = get_small_encoding_integer_and_name(dbg,
                    attrib,
                    &tempud,
                    /* attrname */ (char *) NULL,
                    /* err_string */ (const char **) NULL,
                    (encoding_type_func) 0,
                    &err,show_form_here);

                if (wres == DW_DLV_OK) {
                    snprintf(small_buf, sizeof(small_buf), 
                        "0x%08" DW_PR_DUx ,
                        tempud);
                    esb_append(esbp, small_buf);
                    if (attr == DW_AT_decl_file || attr == DW_AT_call_file) {
                        if (srcfiles && tempud > 0 && tempud <= cnt) {
                            /*  added by user request */
                            /*  srcfiles is indexed starting at 0, but
                                DW_AT_decl_file defines that 0 means no
                                file, so tempud 1 means the 0th entry in
                                srcfiles, thus tempud-1 is the correct
                                index into srcfiles.  */
                            char *fname = srcfiles[tempud - 1];

                            esb_append(esbp, " ");
                            esb_append(esbp, fname);
                        }

                        /*  Validate integrity of files 
                            referenced in .debug_line */
                        if(check_decl_file) {
                            DWARF_CHECK_COUNT(decl_file_result,1);
                            /*  Zero is always a legal index, it means
                                no source name provided. */
                            if(tempud != 0 && tempud > cnt) {
                                if(!srcfiles) {
                                    snprintf(small_buf,sizeof(small_buf),
                                        "There is a file number=%" DW_PR_DUu
                                        " but no source files "
                                        " are known.",tempud);
                                } else {
                                    snprintf(small_buf, sizeof(small_buf),
                                        "Does not point to valid file info "
                                        " filenum=%"  DW_PR_DUu
                                        " filecount=%" DW_PR_DUu ".",
                                        tempud,cnt);
                                }
                                DWARF_CHECK_ERROR2(decl_file_result,
                                    get_AT_name(attr,
                                        dwarf_names_print_on_error),
                                    small_buf);
                            }
                        }
                    }
                } else {
                    print_error(dbg, "Cannot get encoding attribute ..",
                        wres, err);
                }
                }
                break;
            case DW_AT_const_value:
                /* Do not use hexadecimal format */
                wres = formxdata_print_value(dbg,attrib,esbp, &err, FALSE);
                if(wres == DW_DLV_OK){
                    /* String appended already. */
                } else if (wres == DW_DLV_NO_ENTRY) {
                    /* nothing? */
                } else {
                    print_error(dbg,"Cannot get DW_AT_const_value ",wres,err);
                }
  
                
                break;
            case DW_AT_upper_bound:
            case DW_AT_lower_bound:
            default:
                /* Do not use hexadecimal format except for 
                    DW_AT_ranges. */
                wres = formxdata_print_value(dbg,attrib,esbp, &err, 
                    (DW_AT_ranges == attr));
                if (wres == DW_DLV_OK) {
                    /* String appended already. */
                } else if (wres == DW_DLV_NO_ENTRY) {
                    /* nothing? */
                } else {
                    print_error(dbg, "Cannot get form data..", wres,
                        err);
                }
                break;
            }
        }
        if (cu_name_flag) {
            if (attr == DW_AT_MIPS_fde) {
                if (fde_offset_for_cu_low == DW_DLV_BADOFFSET) {
                    fde_offset_for_cu_low
                        = fde_offset_for_cu_high = tempud;
                } else if (tempud < fde_offset_for_cu_low) {
                    fde_offset_for_cu_low = tempud;
                } else if (tempud > fde_offset_for_cu_high) {
                    fde_offset_for_cu_high = tempud;
                }
            }
        }
        break;
    case DW_FORM_sdata:
        wres = dwarf_formsdata(attrib, &tempsd, &err);
        if (wres == DW_DLV_OK) {
            snprintf(small_buf, sizeof(small_buf), 
                "0x%" DW_PR_XZEROS DW_PR_DUx , tempsd);
            esb_append(esbp, small_buf);
        } else if (wres == DW_DLV_NO_ENTRY) {
            /* nothing? */
        } else {
            print_error(dbg, "Cannot get formsdata..", wres, err);
        }
        break;
    case DW_FORM_udata:
        wres = dwarf_formudata(attrib, &tempud, &err);
        if (wres == DW_DLV_OK) {
            snprintf(small_buf, sizeof(small_buf), "0x%" DW_PR_XZEROS DW_PR_DUx ,
                tempud);
            esb_append(esbp, small_buf);
        } else if (wres == DW_DLV_NO_ENTRY) {
            /* nothing? */
        } else {
            print_error(dbg, "Cannot get formudata....", wres, err);
        }
        break;
    case DW_FORM_string:
    case DW_FORM_strp:
        wres = dwarf_formstring(attrib, &temps, &err);
        if (wres == DW_DLV_OK) {
            /* Print as quoted string for clarity. */
            esb_append(esbp, "\"");
            esb_append(esbp, temps);
            esb_append(esbp, "\"");
        } else if (wres == DW_DLV_NO_ENTRY) {
            /* nothing? */
        } else {
            print_error(dbg, "Cannot get a formstr (or a formstrp)....", 
                wres, err);
        }

        break;
    case DW_FORM_flag:
        wres = dwarf_formflag(attrib, &tempbool, &err);
        if (wres == DW_DLV_OK) {
            if (tempbool) {
                snprintf(small_buf, sizeof(small_buf), "yes(%d)",
                    tempbool);
                esb_append(esbp, small_buf);
            } else {
                snprintf(small_buf, sizeof(small_buf), "no");
                esb_append(esbp, small_buf);
            }
        } else if (wres == DW_DLV_NO_ENTRY) {
            /* nothing? */
        } else {
            print_error(dbg, "Cannot get formflag/p....", wres, err);
        }
        break;
    case DW_FORM_indirect:
        /*  We should not ever get here, since the true form was
            determined and direct_form has the DW_FORM_indirect if it is
            used here in this attr. */
        esb_append(esbp, get_FORM_name(theform,
            dwarf_names_print_on_error));
        break;
    case DW_FORM_exprloc: {    /* DWARF4 */
        int showhextoo = 1;
        print_exprloc_content(dbg,die,attrib,showhextoo,esbp);
        }
        break;
    case DW_FORM_sec_offset: { /* DWARF4 */
        string emptyattrname = 0;
        int show_form_here = 0;
        wres = get_small_encoding_integer_and_name(dbg,
            attrib,
            &tempud,
            emptyattrname,
            /* err_string */ NULL,
            (encoding_type_func) 0,
            &err,show_form_here);
        if(wres == DW_DLV_NO_ENTRY) {
            /* Show nothing? */
        } else if(wres == DW_DLV_ERROR) {
            print_error(dbg,
                "Cannot get a  DW_FORM_sec_offset....",
                wres, err);
        } else {
            snprintf(small_buf, sizeof(small_buf), 
                "0x%" DW_PR_XZEROS DW_PR_DUx, 
                tempud);
            esb_append(esbp,small_buf);
        }
        }

        break;
    case DW_FORM_flag_present: /* DWARF4 */
        esb_append(esbp,"yes(1)");
        break;
    case DW_FORM_ref_sig8: {  /* DWARF4 */
        Dwarf_Sig8 sig8data;
        wres = dwarf_formsig8(attrib,&sig8data,&err);
        if(wres != DW_DLV_OK) {
            /* Show nothing? */
            print_error(dbg,
                "Cannot get a  DW_FORM_ref_sig8 ....",
                wres, err);
        } else {
            struct esb_s sig8str;
            esb_constructor(&sig8str);
            format_sig8_string(&sig8data,&sig8str);
            esb_append(esbp,esb_get_string(&sig8str));
            esb_destructor(&sig8str);
        }
        }
        break;

    default:
        print_error(dbg, "dwarf_whatform unexpected value", DW_DLV_OK,
            err);
    }
    show_form_itself(show_form,local_verbose,theform, direct_form,esbp);
}

void 
format_sig8_string(Dwarf_Sig8*data, struct esb_s *out)
{
    unsigned i = 0;
    char small_buf[40];
    esb_append(out,"0x");
    for( ; i < sizeof(data->signature); ++i) {
        if (i == 4) {
            esb_append(out," 0x");
        }
        snprintf(small_buf,sizeof(small_buf), "%02x",
            (unsigned char)(data->signature[i]));
        esb_append(out,small_buf);
    }
}


/* A cleanup so that when using a memory checker
   we don't show irrelevant leftovers.
*/
void
clean_up_die_esb()
{
   esb_destructor(&esb_base);
}

static int
get_form_values(Dwarf_Attribute attrib,
    Dwarf_Half * theform, Dwarf_Half * directform)
{
    Dwarf_Error err = 0;
    int res = dwarf_whatform(attrib, theform, &err);
    dwarf_whatform_direct(attrib, directform, &err);
    return res;
}
static void
show_form_itself(int local_show_form,
    int local_verbose,
    int theform,
    int directform, struct esb_s *esbp)
{
    char small_buf[100];
    if (local_show_form
        && directform && directform == DW_FORM_indirect) {
        char *form_indir = " (used DW_FORM_indirect";
        char *form_indir2 = ") ";
        esb_append(esbp, form_indir);
        if(local_verbose) {
            esb_append(esbp, get_form_number_as_string(DW_FORM_indirect,
                small_buf,sizeof(small_buf)));
        }
        esb_append(esbp, form_indir2);
    }
    if(local_show_form) {
        esb_append(esbp," <form ");
        esb_append(esbp,get_FORM_name(theform,
            dwarf_names_print_on_error));
        if(local_verbose) {
            esb_append(esbp, get_form_number_as_string(theform,
                small_buf, sizeof(small_buf)));
        }
        esb_append(esbp,">");
    }
}


#include "tmp-ta-table.c"
#include "tmp-ta-ext-table.c"

static int
legal_tag_attr_combination(Dwarf_Half tag, Dwarf_Half attr)
{
    if(tag <= 0) {
        return FALSE;
    }
    if(tag < ATTR_TREE_ROW_COUNT) {
        int index = attr / BITS_PER_WORD;
        if ( index < ATTR_TREE_COLUMN_COUNT) {
            unsigned bitflag = 1 << (attr % BITS_PER_WORD);
            int known = ((tag_attr_combination_table[tag][index]
                & bitflag) > 0 ? TRUE : FALSE);
            if(known) {
                return TRUE;
            }
        }
    }
    /*  DW_AT_MIPS_fde  used to return TRUE as that was
        convenient for SGI/MIPS users. */
    if(!suppress_check_extensions_tables) {
        int r = 0;
        for ( ; r < ATTR_TREE_EXT_ROW_COUNT; ++r ) {
            int c = 1;
            if(tag != tag_attr_combination_ext_table[r][0]) {
                continue;
            }
            for( ; c < ATTR_TREE_EXT_COLUMN_COUNT ; ++c) {
                if (tag_attr_combination_ext_table[r][c] == attr) {
                    return TRUE;
                }
            }
        }
    }
    return (FALSE);
}

#include "tmp-tt-table.c"
#include "tmp-tt-ext-table.c"

/*  Look only at valid table entries
    The check here must match the building-logic in
    tag_tree.c
    And must match the tags defined in dwarf.h
    The tag_tree_combination_table is a table of bit flags.  */
static int
legal_tag_tree_combination(Dwarf_Half tag_parent, Dwarf_Half tag_child)
{
    if(tag_parent <= 0) { 
        return FALSE;
    }    
    if ( tag_parent < TAG_TREE_ROW_COUNT) {
        int index = tag_child / BITS_PER_WORD;
        if ( index < TAG_TREE_COLUMN_COUNT) {
            unsigned bitflag = 1 << (tag_child % BITS_PER_WORD);
            int known = ((tag_tree_combination_table[tag_parent]
                [index] & bitflag) > 0 ? TRUE : FALSE);
            if(known) {
                return TRUE;
            }
        }    
    }    
    if(!suppress_check_extensions_tables) {
        int r = 0; 
        for ( ; r < TAG_TREE_EXT_ROW_COUNT; ++r ) {
            int c = 1; 
            if(tag_parent != tag_tree_combination_ext_table[r][0]) {
                continue;
            }    
            for( ; c < TAG_TREE_EXT_COLUMN_COUNT ; ++c) {
                if (tag_tree_combination_ext_table[r][c] == tag_child) {
                    return TRUE;
                }    
            }    
        }    
    }    
    return (FALSE);
}

