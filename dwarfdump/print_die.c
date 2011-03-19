/* 
  Copyright (C) 2000-2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2009-2010 SN Systems Ltd. All rights reserved.
  Portions Copyright 2007-2010 David Anderson. All rights reserved.

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
/* The address of the Free Software Foundation is
 *    Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
 *    Boston, MA 02110-1301, USA.  
 *    SGI has moved from the Crittenden Lane address.
 */


#include "globals.h"
#include "naming.h"
#include "esb.h"                /* For flexible string buffer. */
#include "makename.h"           /* Non-duplicating string table. */
#include "print_frames.h"       /* for get_string_from_locs() . */
#include "tag_common.h" 

static int get_form_values(Dwarf_Attribute attrib,
    Dwarf_Half * theform, Dwarf_Half * directform);
static void show_form_itself(int show_form,
    int theform, int directform, struct esb_s * str_out);
static void get_attr_value(Dwarf_Debug dbg, Dwarf_Half tag,
                           Dwarf_Die die,
                           Dwarf_Attribute attrib, 
                           char **srcfiles,
                           Dwarf_Signed cnt, struct esb_s *esbp,
                           int show_form);
static void print_exprloc_content(Dwarf_Debug dbg,Dwarf_Die die, Dwarf_Attribute attrib,
    int showhextoo, struct esb_s *esbp);
static boolean print_attribute(Dwarf_Debug dbg, Dwarf_Die die,
                            Dwarf_Half attr,
                            Dwarf_Attribute actual_addr,
                            boolean print_information, char **srcfiles,
                            Dwarf_Signed cnt);
static void get_location_list(Dwarf_Debug dbg, Dwarf_Die die,
                              Dwarf_Attribute attr, struct esb_s *esbp);
static int legal_tag_attr_combination(Dwarf_Half tag, Dwarf_Half attr);
static int legal_tag_tree_combination(Dwarf_Half parent_tag, Dwarf_Half child_tag);
static int _dwarf_print_one_expr_op(Dwarf_Debug dbg,Dwarf_Loc* expr,int index, struct esb_s *string_out);

/* esb_base is static so gets initialized to zeros.  
   It is not thread-safe or
   safe for multiple open producer instances for
   but that does not matter here in dwarfdump.

   The memory used by esb_base  and esb_extra is never freed.
*/
static struct esb_s esb_base;   
static struct esb_s esb_extra;   

static int dwarf_names_print_on_error = 1;

static int indent_level = 0;
static boolean local_symbols_already_began = FALSE;

typedef const char *(*encoding_type_func) (unsigned,int doprintingonerr);

Dwarf_Off fde_offset_for_cu_low = DW_DLV_BADOFFSET;
Dwarf_Off fde_offset_for_cu_high = DW_DLV_BADOFFSET;

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
    /* terminator */
    {0,0,""} 
};

struct die_stack_data_s {
    Dwarf_Die die_;
    boolean already_printed_;
};
struct die_stack_data_s empty_stack_entry;

#define DIE_STACK_SIZE 300
static struct die_stack_data_s die_stack[DIE_STACK_SIZE];

#define SET_DIE_STACK_ENTRY(i,x) { die_stack[i].die_ = x; \
       die_stack[indent_level].already_printed_ = FALSE; }
#define EMPTY_DIE_STACK_ENTRY(i) { die_stack[i] = empty_stack_entry; }

static int
print_as_info_or_cu()
{
   return (info_flag || cu_name_flag);
}



/* process each compilation unit in .debug_info */
void
print_infos(Dwarf_Debug dbg)
{
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Unsigned abbrev_offset = 0;
    Dwarf_Half version_stamp = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Die cu_die = 0;
    Dwarf_Unsigned next_cu_offset = 0;
    int nres = DW_DLV_OK;
    int   cu_count = 0;

    if (print_as_info_or_cu())
        printf("\n.debug_info\n");

    /* Loop until it fails.  */
    while ((nres =
            dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp,
                                 &abbrev_offset, &address_size,
                                 &next_cu_offset, &err))
           == DW_DLV_OK) {
        int sres = 0;
        if(cu_count >=  break_after_n_units) {
            printf("Break at %d\n",cu_count);
            break;
        }
        if (cu_name_flag) {

            sres = dwarf_siblingof(dbg, NULL, &cu_die, &err);
            if (sres != DW_DLV_OK) {
                print_error(dbg, "siblingof cu header", sres, err);
            }
            if(should_skip_this_cu(dbg,cu_die,err)) {
                dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
                ++cu_count;
                cu_offset = next_cu_offset;
                continue;
            }
            dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
        }
        if (verbose) {
            if (dense) {
                printf("<%s>", "cu_header");
                printf(" %s<%" DW_PR_DUu ">", "cu_header_length",
                       cu_header_length);
                printf(" %s<%d>", "version_stamp", version_stamp);
                printf(" %s<%" DW_PR_DUu ">", "abbrev_offset", abbrev_offset);
                printf(" %s<%d>\n", "address_size", address_size);

            } else {
                printf("\nCU_HEADER:\n");
                printf("\t\t%-28s%" DW_PR_DUu "\n", "cu_header_length",
                       cu_header_length);
                printf("\t\t%-28s%d\n", "version_stamp", version_stamp);
                printf("\t\t%-28s%" DW_PR_DUu "\n", "abbrev_offset",
                       abbrev_offset);
                printf("\t\t%-28s%d", "address_size", address_size);
            }
        }

        /* process a single compilation unit in .debug_info. */
        sres = dwarf_siblingof(dbg, NULL, &cu_die, &err);
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

                print_die_and_children(dbg, cu_die, srcfiles, cnt);
                if (srcf == DW_DLV_OK) {
                    int si;

                    for (si = 0; si < cnt; ++si) {
                        dwarf_dealloc(dbg, srcfiles[si], DW_DLA_STRING);
                    }
                    dwarf_dealloc(dbg, srcfiles, DW_DLA_LIST);
                }
            }
            if (line_flag)
                print_line_numbers_this_cu(dbg, cu_die);
            dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
        } else if (sres == DW_DLV_NO_ENTRY) {
            /* do nothing I guess. */
        } else {
            print_error(dbg, "Regetting cu_die", sres, err);
        }
        ++cu_count;
        cu_offset = next_cu_offset;
    }
    if (nres == DW_DLV_ERROR) {
        string errmsg = dwarf_errmsg(err);
        Dwarf_Unsigned myerr = dwarf_errno(err);

        fprintf(stderr, "%s ERROR:  %s:  %s (%lu)\n",
                program_name, "attempting to print .debug_info",
                errmsg, (unsigned long) myerr);
        fprintf(stderr, "attempting to continue.\n");
    }
}

static void
print_die_stack(Dwarf_Debug dbg,char **srcfiles,Dwarf_Signed cnt)
{
    int lev = 0;
    boolean ignore_die_stack = FALSE;

    for(lev = 0; lev <= indent_level; ++lev)
    {
        print_one_die(dbg,die_stack[lev].die_,TRUE,lev,srcfiles,cnt,
            ignore_die_stack);
    }
}

/* recursively follow the die tree */
extern void
print_die_and_children(Dwarf_Debug dbg, Dwarf_Die in_die_in,
                       char **srcfiles, Dwarf_Signed cnt)
{
    Dwarf_Die child = 0;
    Dwarf_Die sibling = 0;
    Dwarf_Error err = 0;
    int tres = 0;
    int cdres = 0;
    Dwarf_Die in_die = in_die_in;

    for (;;) {
        SET_DIE_STACK_ENTRY(indent_level,in_die);

        if (check_tag_tree) {
            tag_tree_result.checks++;
            if (indent_level == 0) {
                Dwarf_Half tag = 0;

                tres = dwarf_tag(in_die, &tag, &err);
                if (tres != DW_DLV_OK) {
                    DWARF_CHECK_ERROR(tag_tree_result,
                        "Tag-tree root is not DW_TAG_compile_unit")
                } else if (tag == DW_TAG_compile_unit) {
                    /* OK */
                } else {
                    DWARF_CHECK_ERROR(tag_tree_result,
                        "tag-tree root is not DW_TAG_compile_unit")
                }
            } else {
                Dwarf_Half tag_parent = 0; 
                Dwarf_Half tag_child = 0;
                int pres = 0;
                int cres = 0;
                const char *ctagname = "<child tag invalid>";
                const char *ptagname = "<parent tag invalid>";

                pres = dwarf_tag(die_stack[indent_level - 1].die_, 
                    &tag_parent, &err);
                cres = dwarf_tag(in_die, &tag_child, &err);
                if (pres != DW_DLV_OK)
                    tag_parent = 0;
                if (cres != DW_DLV_OK)
                    tag_child = 0;
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
                } else if (legal_tag_tree_combination(tag_parent, tag_child)) {
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

        /* Here do pre-descent processing of the die. */
        {
            boolean retry_print_on_match = FALSE;
            boolean ignore_die_stack = FALSE;
            retry_print_on_match = print_one_die(dbg, in_die, 
                print_as_info_or_cu(), 
                indent_level, srcfiles, cnt,ignore_die_stack);
            if(!print_as_info_or_cu() && retry_print_on_match) {
                print_die_stack(dbg,srcfiles,cnt);
            }
        }

        cdres = dwarf_child(in_die, &child, &err);
        /* child first: we are doing depth-first walk */
        if (cdres == DW_DLV_OK) {
            indent_level++;
            SET_DIE_STACK_ENTRY(indent_level,0);
            if(indent_level >= DIE_STACK_SIZE ) {
                print_error(dbg,
                  "compiled in DIE_STACK_SIZE limit exceeded",
                  DW_DLV_OK,err);
            }
            print_die_and_children(dbg, child, srcfiles, cnt);
            EMPTY_DIE_STACK_ENTRY(indent_level);
            indent_level--;
            if (indent_level == 0)
                local_symbols_already_began = FALSE;
            dwarf_dealloc(dbg, child, DW_DLA_DIE);
        } else if (cdres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_child", cdres, err);
        }

        cdres = dwarf_siblingof(dbg, in_die, &sibling, &err);
        if (cdres == DW_DLV_OK) {
            /* print_die_and_children(dbg, sibling, srcfiles, cnt); We
               loop around to actually print this, rather than
               recursing. Recursing is horribly wasteful of stack
               space. */
        } else if (cdres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_siblingof", cdres, err);
        }

        /* Here do any post-descent (ie post-dwarf_child) processing of 
           the in_die. */

        EMPTY_DIE_STACK_ENTRY(indent_level);
        if (in_die != in_die_in) {
            /* Dealloc our in_die, but not the argument die, it belongs 
               to our caller. Whether the siblingof call worked or not. 
             */
            dwarf_dealloc(dbg, in_die, DW_DLA_DIE);
        }
        if (cdres == DW_DLV_OK) {
            /* Set to process the sibling, loop again. */
            in_die = sibling;
        } else {
            /* We are done, no more siblings at this level. */

            break;
        }
    }                           /* end for loop on siblings */
    return;
}


/*  If print_information is FALSE, check the TAG and if it is a CU die
 *  print the information anyway. */
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

    if(!ignore_die_stack && die_stack[die_indent_level].already_printed_) {
         /* FALSE seems like a safe return. */
         return FALSE;
    }

    tres = dwarf_tag(die, &tag, &err);
    if (tres != DW_DLV_OK) {
        print_error(dbg, "accessing tag of die!", tres, err);
    }
    tagname = get_TAG_name(tag,dwarf_names_print_on_error);
    ores = dwarf_dieoffset(die, &overall_offset, &err);
    if (ores != DW_DLV_OK) {
        print_error(dbg, "dwarf_dieoffset", ores, err);
    }
    ores = dwarf_die_CU_offset(die, &offset, &err);
    if (ores != DW_DLV_OK) {
        print_error(dbg, "dwarf_die_CU_offset", ores, err);
    }

    if (print_information) {
        if (!ignore_die_stack) {
             die_stack[die_indent_level].already_printed_ = TRUE;
        }
        if (die_indent_level == 0) {
            if (dense)
                printf("\n");
            else {
                printf
                    ("\nCOMPILE_UNIT<header overall offset = %" DW_PR_DUu ">:\n",
                    (Dwarf_Unsigned)(overall_offset - offset));
            }
        } else if (local_symbols_already_began == FALSE &&
                   die_indent_level == 1 && !dense) {
            printf("\nLOCAL_SYMBOLS:\n");
            local_symbols_already_began = TRUE;
        }
        if (dense) {
            if (show_global_offsets) {
                if (die_indent_level == 0) {
                    printf("<%d><%" DW_PR_DUu "+%" DW_PR_DUu " GOFF=%" 
                        DW_PR_DUu ">", die_indent_level,
                       (Dwarf_Unsigned)(overall_offset - offset), 
                       (Dwarf_Unsigned)offset,
                       (Dwarf_Unsigned)overall_offset);
                } else {
                    printf("<%d><%" DW_PR_DUu " GOFF=%" DW_PR_DUu ">", 
                        die_indent_level, 
                        (Dwarf_Unsigned)offset, 
                        (Dwarf_Unsigned)overall_offset);
                }
            } else {
                if (die_indent_level == 0) {
                    printf("<%d><%" DW_PR_DUu "+%" DW_PR_DUu ">", 
                       die_indent_level,
                       (Dwarf_Unsigned)(overall_offset - offset), 
                       (Dwarf_Unsigned)offset);
                } else {
                    printf("<%d><%" DW_PR_DUu ">", die_indent_level, 
                       (Dwarf_Unsigned)offset);
                }
            }
            printf("<%s>",tagname);
            if(verbose) {
                printf(" <abbrev %d>",abbrev_code);
            }
        } else {
            if (show_global_offsets) {
                printf("<%d><%5" DW_PR_DUu " GOFF=%" DW_PR_DUu ">\t", 
                    die_indent_level, (Dwarf_Unsigned)offset,
                    (Dwarf_Unsigned)overall_offset);
            } else {
                printf("<%d><%5" DW_PR_DUu ">\t", die_indent_level, 
                     (Dwarf_Unsigned)offset);
            }
            printf("%s",tagname);
            if(verbose) {
                printf(" <abbrev %d>",abbrev_code);
            }
            fputs("\n",stdout);
        }
    }

    atres = dwarf_attrlist(die, &atlist, &atcnt, &err);
    if (atres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_attrlist", atres, err);
    } else if (atres == DW_DLV_NO_ENTRY) {
        /* indicates there are no attrs.  It is not an error. */
        atcnt = 0;
    }


    for (i = 0; i < atcnt; i++) {
        Dwarf_Half attr;
        int ares;

        ares = dwarf_whatattr(atlist[i], &attr, &err);
        if (ares == DW_DLV_OK) {
            boolean attr_match = print_attribute(dbg, die, attr,
                            atlist[i],
                            print_information, srcfiles, cnt);
            if(print_information == FALSE && attr_match) {
                attribute_matched = TRUE;
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

    if (dense && print_information) {
        printf("\n\n");
    }
    return attribute_matched;
}

/* Encodings have undefined signedness. Accept either
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

   If string_out is non-NULL then attr_name and val_as_string
   must also be non-NULL.

*/
int
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
            *uval_out = (Dwarf_Unsigned) sval;
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
        show_form_itself(show_form, theform, directform,&esb_base);
        *string_out = makename(esb_get_string(&esb_base));
    }
    return DW_DLV_OK;
}




/*
 * We need a 32-bit signed number here, but there's no portable
 * way of getting that.  So use __uint32_t instead.  It's supplied
 * in a reliable way by the autoconf infrastructure.
 */

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
        /* Print a full line */
        /* If you touch this string, update the magic number 8 in
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
            DW_PR_DUu " (0x%" DW_PR_DUx ") "
            "(%" DW_PR_DUu " bytes)>",
            rangecount,
            off,
            off,
            bytecount);
        esb_append(stringbuf,tmp);
    } else {
        snprintf(tmp,sizeof(tmp),
            "\t\tranges: %" DW_PR_DSd " at .debug_ranges offset %" 
            DW_PR_DUu " (0x%" DW_PR_DUx ") "
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
              "<[%2" DW_PR_DSd "] %s 0x%" DW_PR_DUx " 0x%" DW_PR_DUx ">",
              (Dwarf_Signed)i,
              type, 
              (Dwarf_Unsigned) r->dwr_addr1,
              (Dwarf_Unsigned)r->dwr_addr2);
      } else {
          snprintf(tmp,sizeof(tmp),
              "\t\t\t[%2" DW_PR_DSd "] %-14s 0x%08" DW_PR_DUx " 0x%08" 
              DW_PR_DUx "\n",
              (Dwarf_Signed)i,
              type, 
              (Dwarf_Unsigned) r->dwr_addr1,
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

static boolean
print_attribute(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attr,
                Dwarf_Attribute attr_in,
                boolean print_information,
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

    atname = get_AT_name(attr,dwarf_names_print_on_error);

    /* The following gets the real attribute, even in the face of an 
       incorrect doubling, or worse, of attributes. */
    attrib = attr_in;
    /* Do not get attr via dwarf_attr: if there are (erroneously) 
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
    if (check_attr_tag) {
        const char *tagname = "<tag invalid>";

        attr_tag_result.checks++;
        if (tres == DW_DLV_ERROR) {
            DWARF_CHECK_ERROR3(attr_tag_result,tagname,
                get_AT_name(attr,dwarf_names_print_on_error),
                "check the tag-attr combination.");
        } else if (tres == DW_DLV_NO_ENTRY) {
            DWARF_CHECK_ERROR3(attr_tag_result,tagname,
                get_AT_name(attr,dwarf_names_print_on_error),
                "check the tag-attr combination..")
        } else if (legal_tag_attr_combination(tag, attr)) {
            /* OK */
        } else {
            tagname = get_TAG_name(tag,dwarf_names_print_on_error);
            DWARF_CHECK_ERROR3(attr_tag_result,tagname,
                get_AT_name(attr,dwarf_names_print_on_error),
                "check the tag-attr combination")
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
    case DW_AT_location:
    case DW_AT_data_member_location:
    case DW_AT_vtable_elem_location:
    case DW_AT_string_length:
    case DW_AT_return_addr:
    case DW_AT_use_location:
    case DW_AT_static_link:
    case DW_AT_frame_base:
        {
            /* value is a location description or location list */
            Dwarf_Half theform = 0;
            Dwarf_Half directform = 0;
            get_form_values(attrib,&theform,&directform);
            esb_empty_string(&esb_base);
            if(is_location_form(theform)) {
                get_location_list(dbg, die, attrib, &esb_base);
                show_form_itself(show_form_used, theform, directform,&esb_base);
            } else if (theform = DW_FORM_exprloc)  {
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
            show_form_itself(show_form_used, theform, directform,&esb_base);
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
            show_form_itself(show_form_used, theform, directform,&esb_base);
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
                print_error(dbg, "dwarf_whatform cannot find attr form",
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
                show_form_itself(show_form_used, theform, 
                    directform,&esb_base);
                valname = esb_get_string(&esb_base);
                }
                break;
            default:
                get_attr_value(dbg, tag, die,
                    attrib, srcfiles, cnt, &esb_base, show_form_used);
                valname = esb_get_string(&esb_base);
                break;
            }
            break;
        }
    case DW_AT_high_pc:
        {
            Dwarf_Half theform;
            int rv;
            rv = dwarf_whatform(attrib,&theform,&err);
            /* depending on the form and the attribute, process the form */
            if(rv == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_whatform cannot find attr form",
                            rv, err);
            } else if (rv == DW_DLV_NO_ENTRY) {
                break;
            }
            esb_empty_string(&esb_base);
            if( theform != DW_FORM_addr) {
              /* New in DWARF4: other forms are not an address
                 but are instead offset from pc.
                 One could test for DWARF4 here before adding
                 this string, but that seems unnecessary as this
                 could not happen with DWARF3 or earlier. 
                 A normal consumer would have to add this value to
                 DW_AT_low_pc to get a true pc. */
              esb_append(&esb_base,"<offset-from-lowpc>");
            }
            get_attr_value(dbg, tag, die, attrib, srcfiles, cnt, 
                &esb_base,show_form_used);
            valname = esb_get_string(&esb_base);
        }
        break;
    case DW_AT_ranges:
        {
            Dwarf_Half theform = 0;
            int rv;

            rv = dwarf_whatform(attrib,&theform,&err);
            if(rv == DW_DLV_ERROR) {
                print_error(dbg, "dwarf_whatform cannot find attr form",
                            rv, err);
            } else if (rv == DW_DLV_NO_ENTRY) {
                break;
            }


            esb_empty_string(&esb_base);
            get_attr_value(dbg, tag,die, attrib, srcfiles, cnt, &esb_base,
                show_form_used);
            if( theform == DW_FORM_data4 || theform == DW_FORM_data8) {
                int ures = DW_DLV_NO_ENTRY;
                Dwarf_Unsigned off = 0;
                Dwarf_Ranges *rangeset = 0;
                Dwarf_Signed rangecount = 0;

                ures = dwarf_formudata(attrib, &off, &err);
                if(ures ==DW_DLV_OK) {
                    Dwarf_Unsigned bytecount = 0;
                    int rres = dwarf_get_ranges_a(dbg,off,
                        die,
                        &rangeset, 
                        &rangecount,&bytecount,&err);
                    if(rres == DW_DLV_OK) {
                       if(print_information) {
                           append_extra_string = 1;
                           esb_empty_string(&esb_extra);
                           print_ranges_list_to_extra(dbg,off,
                               rangeset,rangecount,bytecount,
                               &esb_extra);
                       }
                       dwarf_ranges_dealloc(dbg,rangeset,rangecount);
                    } else if (rres == DW_DLV_ERROR) {
                        printf("dwarf_get_ranges() "
                            "cannot find DW_AT_ranges at offset %" 
                            DW_PR_DUu " (0x%" DW_PR_DUx ").",
                            off,
                            off);
                        DWARF_CHECK_ERROR2(ranges_result,
                            get_AT_name(attr,
                                dwarf_names_print_on_error),
                            " cannot find DW_AT_ranges at offset");
                    } else { /* NO ENTRY */
                        printf("dwarf_get_ranges() "
                            "finds no DW_AT_ranges at offset %" 
                            DW_PR_DUu " (0x%" DW_PR_DUx ").",
                            off,
                            off);
                        DWARF_CHECK_ERROR2(ranges_result,
                            get_AT_name(attr,
                                dwarf_names_print_on_error),
                            " fails to find DW_AT_ranges at offset");
                    }
                } else if (ures == DW_DLV_ERROR) {
                    printf("dwarf_formudata fails to find DW_AT_ranges offset");
                    DWARF_CHECK_ERROR2(ranges_result,
                        get_AT_name(attr,
                            dwarf_names_print_on_error),
                        " fails to find DW_AT_ranges offset");
                } else { /* NO ENTRY */
                    printf("dwarf_formudata cannot find DW_AT_ranges offset.");
                    DWARF_CHECK_ERROR2(ranges_result,
                        get_AT_name(attr,
                            dwarf_names_print_on_error),
                        " cannot find DW_AT_ranges offset");
                }
            } else {
                DWARF_CHECK_ERROR2(ranges_result,get_AT_name(attr,
                    dwarf_names_print_on_error),
                    "is wrong form, must be data4 or data8");
            }
            valname = esb_get_string(&esb_base);

        }
        break;
    default:
        esb_empty_string(&esb_base);
        get_attr_value(dbg, tag,die, attrib, srcfiles, cnt, &esb_base,
            show_form_used);
        valname = esb_get_string(&esb_base);
        break;
    }
    if (!print_information) {
        if ( (search_match_text && !strcmp(valname,search_match_text)) ||
           (search_match_text && !strcmp(atname,search_match_text)) ||
           (search_any_text && is_strstrnocase(valname,search_any_text)) || 
           (search_any_text && is_strstrnocase(atname,search_any_text)) 
#ifdef HAVE_REGEX
           || (search_regex_text && !regexec(&search_re,valname,0,NULL,0))
           || (search_regex_text && !regexec(&search_re,atname,0,NULL,0))
#endif
           ) {
                found_search_attr = TRUE;
        }
    }

    if (print_information ) {
        if (dense) {
            printf(" %s<%s>", atname, valname);
            if(append_extra_string) {
                char *v = esb_get_string(&esb_extra);
                printf("%s", v);
            }
        } else {
            printf("\t\t%-28s%s\n", atname, valname);
            if( append_extra_string) {
                char *v = esb_get_string(&esb_extra);
                printf("%s", v);
            }
        }
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
        snprintf(small_buf, sizeof(small_buf), "<lowpc=0x%" DW_PR_DUx ">",
                 (Dwarf_Unsigned) llbuf->ld_lopc);
        esb_append(string_out, small_buf);


        snprintf(small_buf, sizeof(small_buf), "<highpc=0x%" DW_PR_DUx ">",
                 (Dwarf_Unsigned) llbuf->ld_hipc);
        esb_append(string_out, small_buf);
        if (verbose) {
            snprintf(small_buf, sizeof(small_buf),
                     "<from %s offset 0x%" DW_PR_DUx ">",
                     llbuf->
                     ld_from_loclist ? ".debug_loc" : ".debug_info",
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
     /* local_space_needed is intended to be 'more than big enough'
       for a short group of loclist entries.  */
    char small_buf[100];
    Dwarf_Small op;
    Dwarf_Unsigned opd1;  
    Dwarf_Unsigned opd2;
    const char * op_name;


    if (index > 0)
        esb_append(string_out, " ");

    op = expr->lr_atom;
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
                snprintf(small_buf, sizeof(small_buf), " 0x%" DW_PR_DUx , 
                    opd1);
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
                         " %" DW_PR_DSd , (Dwarf_Signed) opd1);
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
            snprintf(small_buf, sizeof(small_buf), " %" DW_PR_DUu , opd1);
            esb_append(string_out, small_buf);
                break;
        case DW_OP_bregx:
            snprintf(small_buf, sizeof(small_buf), "%" DW_PR_DUu , opd1);
            esb_append(string_out, small_buf);

            opd2 = expr->lr_number2;
            snprintf(small_buf, sizeof(small_buf),
                "%+" DW_PR_DSd , (Dwarf_Signed) opd2);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_call2:
            snprintf(small_buf, sizeof(small_buf), " 0x%" DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_call4:
            snprintf(small_buf, sizeof(small_buf), " 0x%" DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_call_ref:
            snprintf(small_buf, sizeof(small_buf), " 0x08%" DW_PR_DUx , opd1);
            esb_append(string_out, small_buf);
            break;
        case DW_OP_bit_piece:
            snprintf(small_buf, sizeof(small_buf), " 0x08%" DW_PR_DUx , opd1);
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
                  " 0x%08" DW_PR_DUx , opd1);
                esb_append(string_out, small_buf);
                /* The other operand is a block of opd1 bytes. */
                /* FIXME */
                print_len = opd1;
                if(print_len > IMPLICIT_VALUE_PRINT_MAX) {
                    print_len = IMPLICIT_VALUE_PRINT_MAX;
                }
                if(print_len > 0) {
                    const unsigned char *bp = 0;
                    unsigned int i = 0;
                    opd2 = expr->lr_number2;
                    /* This is a really ugly cast, a way
                       to implement DW_OP_implicit value in
                       this libdwarf context. */
                    bp = (const unsigned char *) opd2;
                    esb_append(string_out," contents 0x");
                    for( ; i < print_len; ++i,++bp) {
                        snprintf(small_buf, sizeof(small_buf), 
                          "%02x", *bp);
                        esb_append(string_out,small_buf);
                    }
                }
#undef IMPLICIT_VALUE_PRINT_MAX
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
        case DW_OP_APPLE_uninit:
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

/* Fill buffer with location lists 
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


    if (use_old_dwarf_loclist) {

        lres = dwarf_loclist(attr, &llbuf, &no_of_elements, &err);
        if (lres == DW_DLV_ERROR)
            print_error(dbg, "dwarf_loclist", lres, err);
        if (lres == DW_DLV_NO_ENTRY)
            return;

        dwarfdump_print_one_locdesc(dbg, llbuf,skip_locdesc_header,esbp);
        dwarf_dealloc(dbg, llbuf->ld_s, DW_DLA_LOC_BLOCK);
        dwarf_dealloc(dbg, llbuf, DW_DLA_LOCDESC);
        return;
    }

    lres = dwarf_loclist_n(attr, &llbufarray, &no_of_elements, &err);
    if (lres == DW_DLV_ERROR)
        print_error(dbg, "dwarf_loclist", lres, err);
    if (lres == DW_DLV_NO_ENTRY)
        return;

    for (llent = 0; llent < no_of_elements; ++llent) {
        char small_buf[100];

        llbuf = llbufarray[llent];

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
            /* DW_DLV_OK so we add follow-on at end, else is
               DW_DLV_NO_ENTRY (which is impossible, treat like
               DW_DLV_OK). */
        }
    }
    for (i = 0; i < no_of_elements; ++i) {
        dwarf_dealloc(dbg, llbufarray[i]->ld_s, DW_DLA_LOC_BLOCK);
        dwarf_dealloc(dbg, llbufarray[i], DW_DLA_LOCDESC);
    }
    dwarf_dealloc(dbg, llbufarray, DW_DLA_LIST);
}

static void
formx_unsigned(Dwarf_Unsigned u, struct esb_s *esbp)
{
     char small_buf[40];
     snprintf(small_buf, sizeof(small_buf),
      "%" DW_PR_DUu , u);
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
/* We think this is an integer. Figure out how to print it.
   In case the signedness is ambiguous (such as on 
   DW_FORM_data1 (ie, unknown signedness) print two ways.
*/
static int
formxdata_print_value(Dwarf_Attribute attrib, struct esb_s *esbp,
        Dwarf_Error * err)
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
        if(tempud == tempsd) {
           /* Data is the same value, so makes no difference which
                we print. */
           formx_unsigned(tempud,esbp);
        } else {
           formx_unsigned(tempud,esbp);
           esb_append(esbp,"(as signed = ");
           formx_signed(tempsd,esbp);
           esb_append(esbp,")");
        }
      } else if (sres == DW_DLV_NO_ENTRY) {
        formx_unsigned(tempud,esbp);
      } else /* DW_DLV_ERROR */{
        formx_unsigned(tempud,esbp);
      }
      return DW_DLV_OK;
    } else  if (ures == DW_DLV_NO_ENTRY) {
      if(sres == DW_DLV_OK) {
        formx_signed(tempsd,esbp);
        return sres;
      } else if (sres == DW_DLV_NO_ENTRY) {
        return sres;
      } else /* DW_DLV_ERROR */{
        *err = serr;
        return sres;
      }
    } 
    /* else ures ==  DW_DLV_ERROR */ 
    if(sres == DW_DLV_OK) {
        formx_signed(tempsd,esbp);
    } else if (sres == DW_DLV_NO_ENTRY) {
        return ures;
    } 
    /* DW_DLV_ERROR */
    return ures;
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
        Dwarf_Half address_size = 0;
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




/* Fill buffer with attribute value.
   We pass in tag so we can try to do the right thing with
   broken compiler DW_TAG_enumerator 

   We pass in die so  

   We append to esbp's buffer.

*/
static void
get_attr_value(Dwarf_Debug dbg, Dwarf_Half tag, 
               Dwarf_Die die, Dwarf_Attribute attrib,
               char **srcfiles, Dwarf_Signed cnt, struct esb_s *esbp,
               int show_form)
{
    Dwarf_Half theform;
    string temps;
    Dwarf_Block *tempb;
    Dwarf_Signed tempsd = 0;
    Dwarf_Unsigned tempud = 0;
    int i;
    Dwarf_Half attr;
    Dwarf_Off off;
    Dwarf_Die die_for_check;
    Dwarf_Half tag_for_check;
    Dwarf_Bool tempbool;
    Dwarf_Addr addr = 0;
    int fres;
    int bres;
    int wres;
    int dres;
    Dwarf_Half direct_form = 0;
    char small_buf[100];


    /* Dwarf_whatform gets the real form, DW_FORM_indir is
       never returned: instead the real form following
       DW_FORM_indir is returned. */
    fres = dwarf_whatform(attrib, &theform, &err);
    /* Depending on the form and the attribute, process the form. */
    if (fres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_whatform cannot find attr form", fres,
                    err);
    } else if (fres == DW_DLV_NO_ENTRY) {
        return;
    }

    /* dwarf_whatform_direct gets the 'direct' form, so if
       the form is DW_FORM_indir that is what is returned. */
    dwarf_whatform_direct(attrib, &direct_form, &err);
    /* Ignore errors in dwarf_whatform_direct() */


    switch (theform) {
    case DW_FORM_addr:
        bres = dwarf_formaddr(attrib, &addr, &err);
        if (bres == DW_DLV_OK) {
            snprintf(small_buf, sizeof(small_buf), "0x%" DW_PR_DUx ,
                     (Dwarf_Unsigned) addr);
            esb_append(esbp, small_buf);
        } else {
            print_error(dbg, "addr formwith no addr?!", bres, err);
        }
        break;
    case DW_FORM_ref_addr:
        /* DW_FORM_ref_addr is not accessed thru formref: ** it is an
           address (global section offset) in ** the .debug_info
           section. */
        bres = dwarf_global_formref(attrib, &off, &err);
        if (bres == DW_DLV_OK) {
            snprintf(small_buf, sizeof(small_buf),
                     "<global die offset %" DW_PR_DUu ">",
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
                /* The value had better be inside the current CU
                   else there is a nasty error here, as a sibling
                   has to be in the same CU, it seems. */
                Dwarf_Off cuoff = 0;
                Dwarf_Off culen = 0;
                tag_tree_result.checks++;
                int res = dwarf_die_CU_offset_range(die,&cuoff,
                    &culen,&err);
                if(res != DW_DLV_OK) {
                } else {
                    Dwarf_Off cuend = cuoff+culen;
                    if(off <  cuoff || off >= cuend) { 
                        DWARF_CHECK_ERROR(tag_tree_result,
                            "DW_AT_sibling DW_FORM_ref_addr offset points "
                            "outside of current CU")
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
            print_error(dbg, "reference form with no valid local ref?!", bres, err);
        }
        /* do references inside <> to distinguish them ** from
           constants. In dense form this results in <<>>. Ugly for
           dense form, but better than ambiguous. davea 9/94 */
        snprintf(small_buf, sizeof(small_buf), "<%" DW_PR_DUu ">", 
            (Dwarf_Unsigned)off);
        esb_append(esbp, small_buf);
        if (check_type_offset) {
            wres = dwarf_whatattr(attrib, &attr, &err);
            if (wres == DW_DLV_ERROR) {

            } else if (wres == DW_DLV_NO_ENTRY) {
            }
            if (attr == DW_AT_type) {
                dres = dwarf_offdie(dbg, cu_offset + off,
                                    &die_for_check, &err);
                type_offset_result.checks++;
                if (dres != DW_DLV_OK) {
                    DWARF_CHECK_ERROR(type_offset_result,
                        "DW_AT_type offset does not point to type info")
                } else {
                    int tres2;

                    tres2 =
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
                            /* OK */
                            break;
                        default:
                            DWARF_CHECK_ERROR(type_offset_result,
                                "DW_AT_type offset does not point to Type info")
                                break;
                        }
                        dwarf_dealloc(dbg, die_for_check, DW_DLA_DIE);
                    } else {
                        DWARF_CHECK_ERROR(type_offset_result,
                            "DW_AT_type offset does not exist")
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
                    snprintf(small_buf, sizeof(small_buf), "%" DW_PR_DUu ,
                             tempud);
                    esb_append(esbp, small_buf);
                    if (attr == DW_AT_decl_file || attr == DW_AT_call_file) {
                        if (srcfiles && tempud > 0 && tempud <= cnt) {
                            /* added by user request */
                            /* srcfiles is indexed starting at 0, but
                               DW_AT_decl_file defines that 0 means no
                               file, so tempud 1 means the 0th entry in
                               srcfiles, thus tempud-1 is the correct
                               index into srcfiles.  */
                            char *fname = srcfiles[tempud - 1];

                            esb_append(esbp, " ");
                            esb_append(esbp, fname);
                       }
                       if(check_decl_file) {
                           decl_file_result.checks++;
                           /* Zero is always a legal index, it means
                              no source name provided. */
                           if(tempud > cnt) {
                               DWARF_CHECK_ERROR2(decl_file_result,
                                   get_AT_name(attr,
                                       dwarf_names_print_on_error),
                                   "does not point to valid file info");
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
                wres = formxdata_print_value(attrib,esbp, &err);
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
                wres = formxdata_print_value(attrib,esbp, &err);
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
            snprintf(small_buf, sizeof(small_buf), "%" DW_PR_DSd , tempsd);
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
            snprintf(small_buf, sizeof(small_buf), "%" DW_PR_DUu , tempud);
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
            esb_append(esbp, temps);
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
        /* We should not ever get here, since the true form was
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
            snprintf(small_buf, sizeof(small_buf), "0x%08" DW_PR_DUx, 
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
            unsigned i = 0;
            esb_append(esbp,"0x");
            for( ; i < sizeof(sig8data.signature); ++i) {
               if (i == 4) {
                    esb_append(esbp," ");
                }
                snprintf(small_buf,sizeof(small_buf), "%02x",
                          (unsigned)(sig8data.signature[i]));
                esb_append(esbp,small_buf);
            }
        }
        }
        break;

    default:
        print_error(dbg, "dwarf_whatform unexpected value", DW_DLV_OK,
                    err);
    }
    show_form_itself(show_form,theform, direct_form,esbp);
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
show_form_itself(int local_show_form,int theform,
    int directform, struct esb_s *esbp)
{
    char small_buf[100];
    if ((verbose || show_form_used)
        && directform && directform == DW_FORM_indirect) {
        char *form_indir = " (used DW_FORM_indirect";
        char *form_indir2 = ") ";
        esb_append(esbp, form_indir);
        if(verbose) {
            esb_append(esbp, get_form_number_as_string(DW_FORM_indirect,
                small_buf,sizeof(small_buf)));
        }
        esb_append(esbp, form_indir2);
    }
    if(show_form_used) {
        esb_append(esbp," <form ");
        esb_append(esbp,get_FORM_name(theform,
             dwarf_names_print_on_error));
        if(verbose) {
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
    /* DW_AT_MIPS_fde  used to return TRUE as that was
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

/*
 *    Look only at valid table entries
 *       The check here must match the building-logic in
 *          tag_tree.c
 *             And must match the tags defined in dwarf.h
 *                The tag_tree_combination_table is a table of bit flags.
 *                */
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

