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
#include "tag_common.h" 
#include "print_frames.h" 
#include <vector>
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;

static int get_form_values(Dwarf_Attribute attrib,
    Dwarf_Half & theform, Dwarf_Half & directform);
static void show_form_itself(bool show_form,
    int theform, int directform, string *str_out);
static void get_attr_value(Dwarf_Debug dbg, Dwarf_Half tag,
   Dwarf_Die die,
   Dwarf_Attribute attrib, 
   SrcfilesHolder &srcfiles,
   string &str_out,bool show_form);
static bool print_attribute(Dwarf_Debug dbg, Dwarf_Die die,
   Dwarf_Half attr,
   Dwarf_Attribute actual_addr,
   bool print_information, 
   SrcfilesHolder &srcfiles);
static void get_location_list(Dwarf_Debug dbg, 
   Dwarf_Die die,
   Dwarf_Attribute attr, string &str_out );
static void print_exprloc_content(Dwarf_Debug dbg,Dwarf_Die die, 
    Dwarf_Attribute attrib,
    bool showhextoo, string &str_out);
static int legal_tag_attr_combination(Dwarf_Half tag, Dwarf_Half attr);
static bool legal_tag_tree_combination(Dwarf_Half parent_tag, 
    Dwarf_Half child_tag);
static int _dwarf_print_one_expr_op(Dwarf_Debug dbg,Dwarf_Loc* expr,int index, string &string_out);

// This following variable is weird. ???
static bool local_symbols_already_began = false;

typedef string(*encoding_type_func) (unsigned int val,bool doprintingonerr);

Dwarf_Off fde_offset_for_cu_low = DW_DLV_BADOFFSET;
Dwarf_Off fde_offset_for_cu_high = DW_DLV_BADOFFSET;

struct operation_descr_s {
    int op_code;
    int op_count;
    string op_1type;
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

static void
print_die_and_children_internal(DieHolder &die_in,
    vector<DieHolder> &dieVec,
    int &indent_level,
    SrcfilesHolder & srcfiles);

static bool 
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
    Dwarf_Unsigned next_cu_offset = 0;
    int nres = DW_DLV_OK;
    int   cu_count = 0;

    if (print_as_info_or_cu()) {
        cout << endl;
        cout << ".debug_info" << endl;
    }

    /* Loop until it fails.  */
    while ((nres =
            dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp,
                                 &abbrev_offset, &address_size,
                                 &next_cu_offset, &err))
           == DW_DLV_OK) {
        if(cu_count >=  break_after_n_units) {
            cout << "Break at " << cu_count << endl;
            break;
        }
        int sres = 0;
        if (cu_name_flag) {
            Dwarf_Die cudie = 0;
            sres = dwarf_siblingof(dbg, NULL, &cudie, &err);
            if (sres != DW_DLV_OK) {
                print_error(dbg, "siblingof cu header", sres, err);
            }
            DieHolder hcu_die(dbg,cudie);
            if(should_skip_this_cu(hcu_die,err)) {
                  ++cu_count;
                  cu_offset = next_cu_offset;
                  continue;
            }
        }
        if (verbose) {
            if (dense) {
                cout << BracketSurround("cu_header");
                cout << " cu_header_length" << 
                    BracketSurround(IToDec(cu_header_length));
                cout << " version_stamp" << 
                    BracketSurround(IToDec(version_stamp));
                cout << " abbrev_offset" << 
                    BracketSurround(IToDec(abbrev_offset));
                cout << " address_size" << 
                    BracketSurround(IToDec(address_size));
            } else {
                cout << endl;
                cout << "CU_HEADER:" << endl;
                unsigned commonw = 28;
                cout << "\t\t" << LeftAlign(commonw,"cu_header_length") <<
                    cu_header_length << endl;

                cout << "\t\t" << LeftAlign(commonw,"version_stamp") <<
                    version_stamp << endl;
                cout << "\t\t" << LeftAlign(commonw,"abbrev_offset") <<
                    abbrev_offset << endl;
                cout << "\t\t" << LeftAlign(commonw,"address_size") <<
                    address_size;
            }
        }

        /* process a single compilation unit in .debug_info. */
        Dwarf_Die cu_die = 0;
        sres = dwarf_siblingof(dbg, NULL, &cu_die, &err);
        if (sres == DW_DLV_OK) {
            DieHolder hcu_die(dbg,cu_die);
            if (print_as_info_or_cu() || search_is_on) {
                Dwarf_Signed cnt = 0;
                char **srcfiles = 0;
                int srcf = dwarf_srcfiles(cu_die,
                                          &srcfiles, &cnt, &err);

                if (srcf != DW_DLV_OK) {
                    srcfiles = 0;
                    cnt = 0;
                }
                SrcfilesHolder hsrcfiles(dbg,srcfiles,cnt);
                print_die_and_children(hcu_die, hsrcfiles);
            }
            if (line_flag) {
                print_line_numbers_this_cu(hcu_die);
            }
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

        cerr << program_name << " ERROR:  " <<
           "attempting to print .debug_info" <<
                errmsg << " " << myerr << endl;
        cerr << "attempting to continue." << endl;
    }
}


void
print_die_and_children(DieHolder & in_die_in,
    SrcfilesHolder &hsrcfiles)
{
    int indent_level = 0;

    vector<DieHolder> dieVec;
    print_die_and_children_internal(in_die_in,
        dieVec,
        indent_level, hsrcfiles);
    return;
}

static void
print_die_stack(DieHolder &curdie, vector<DieHolder> &dieVec,
    SrcfilesHolder & hsrcfiles)
{
    unsigned lev = 0;

    for(lev = 0; lev < dieVec.size(); ++lev)
    {
        print_one_die(dieVec[lev],true,lev,hsrcfiles,
            /* ignore_die_printed_flag= */false);
    }
}


// Recursively follow the die tree 
static void
print_die_and_children_internal(DieHolder & hin_die_in,
    vector<DieHolder> &dieVec,
    int &indent_level,
    SrcfilesHolder & hsrcfiles)
{
    Dwarf_Die child;
    Dwarf_Error err;
    int tres;
    int cdres;
    DieHolder hin_die(hin_die_in);
    Dwarf_Debug dbg = hin_die_in.dbg();

    for (;;) {
        // We loop on siblings, this is the sibling loop.
        dieVec.push_back(hin_die);
        Dwarf_Die in_die = hin_die.die();
        if (check_tag_tree) {
            tag_tree_result.checks++;
            if (indent_level == 0) {
                Dwarf_Half tag;

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
                string ctagname("<child tag invalid>");
                string ptagname("<parent tag invalid>");

                Dwarf_Die tp = dieVec[indent_level - 1].die();
                int pres = dwarf_tag(tp, &tag_parent, &err);
                int cres = dwarf_tag(in_die, &tag_child, &err);
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

        /* here to pre-descent processing of the die */
        bool retry_print_on_match = 
            print_one_die(hin_die, print_as_info_or_cu(),
                indent_level, hsrcfiles,
            /* ignore_die_printed_flag= */ false);
        if(!print_as_info_or_cu() && retry_print_on_match) {
            print_die_stack(hin_die,dieVec,hsrcfiles);
        }
        cdres = dwarf_child(in_die, &child, &err);
        /* child first: we are doing depth-first walk */
        if (cdres == DW_DLV_OK) {
            DieHolder hchild(dbg,child);
            indent_level++;
            print_die_and_children_internal(hchild, 
                 dieVec,indent_level,hsrcfiles);
            indent_level--;
            if (indent_level == 0) {
                local_symbols_already_began = false;
            }
        } else if (cdres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_child", cdres, err);
        }

        Dwarf_Die sibling = 0;
        cdres = dwarf_siblingof(dbg, in_die, &sibling, &err);
        if (cdres == DW_DLV_OK) {
            /* print_die_and_children_internal(); We
               loop around to actually print this, rather than
               recursing. Recursing is horribly wasteful of stack
               space. */
        } else if (cdres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_siblingof", cdres, err);
        }
        DieHolder hsibling(dbg,sibling);

        /* Here do any post-descent (ie post-dwarf_child) processing of 
           the in_die (just pop stack). */
        dieVec.pop_back();
        if (cdres == DW_DLV_OK) {
            /* Set to process the sibling, loop again. */
            hin_die = hsibling;
        } else {
            /* We are done, no more siblings at this level. */
            break;
        }
    }   /* end for loop on siblings */
    return;
}

bool
print_one_die(DieHolder & hdie, bool print_information,
    int indent_level,
    SrcfilesHolder &hsrcfiles,
    bool ignore_die_printed_flag)
{
    Dwarf_Die die = hdie.die();
    Dwarf_Debug dbg = hdie.dbg();
    int abbrev_code = dwarf_die_abbrev_code(die);
    bool attribute_matched = false;

    if(!ignore_die_printed_flag && hdie.die_printed()) {
        /* Seems arbitrary as a return, but ok. */
        return false;
    }
    Dwarf_Half tag = 0;
    int tres = dwarf_tag(die, &tag, &err);
    if (tres != DW_DLV_OK) {
        print_error(dbg, "accessing tag of die!", tres, err);
    }
    string tagname = get_TAG_name(tag,dwarf_names_print_on_error);

    Dwarf_Off overall_offset = 0;
    int ores = dwarf_dieoffset(die, &overall_offset, &err);
    if (ores != DW_DLV_OK) {
        print_error(dbg, "dwarf_dieoffset", ores, err);
    }
    Dwarf_Off offset = 0; 
    ores = dwarf_die_CU_offset(die, &offset, &err);
    if (ores != DW_DLV_OK) {
        print_error(dbg, "dwarf_die_CU_offset", ores, err);
    }

    if (print_information) {
        if(!ignore_die_printed_flag) {
            hdie.mark_die_printed();
        }
        if (indent_level == 0) {
            cout << endl;
            if (!dense) {
                cout << "COMPILE_UNIT<header overall offset = "
                    << (overall_offset - offset) << ">:" << endl;
            }
        } else if (local_symbols_already_began == false &&
                   indent_level == 1 && !dense) {
            cout << endl;
            // This prints once per top-level DIE.
            cout <<"LOCAL_SYMBOLS:" << endl;
            local_symbols_already_began = true;
        }
        if (dense) {
            if (show_global_offsets) {
                if (indent_level == 0) {
                    cout << BracketSurround(IToDec(indent_level)) <<
                          BracketSurround(
                             IToDec(overall_offset - offset) +
                             string("+") +
                             IToDec(offset) +
                             string(" GOFF=") +
                             IToDec(overall_offset)); 
                } else {
                    cout << BracketSurround(IToDec(indent_level)) <<
                          BracketSurround(
                             IToDec(offset) +
                             string(" GOFF=") +
                             IToDec(overall_offset)); 
                }
            } else {
                if (indent_level == 0) {
                    cout << BracketSurround(IToDec(indent_level)) <<
                        BracketSurround(
                            IToDec(overall_offset - offset) +
                            string("+") +
                            IToDec(offset));
                } else {
                    cout << BracketSurround(IToDec(indent_level)) <<
                        BracketSurround(IToDec(offset));
                }
            }
            cout << BracketSurround(tagname);
            if(verbose) {
                cout << " " << BracketSurround(string("abbrev ") +
                    IToDec(abbrev_code));
            }
        } else {
            if (show_global_offsets) {
                cout << BracketSurround(IToDec(indent_level)) <<
                    BracketSurround(
                        IToDec(offset,5) +
                        string(" GOFF=") +
                        IToDec(overall_offset)) <<
                    "\t"; 
            } else {
                cout << BracketSurround(IToDec(indent_level)) <<
                    BracketSurround(IToDec(offset,5)) <<
                    "\t"; 
            }
            cout << tagname;
            if(verbose) {
                cout << " " << BracketSurround(string("abbrev ") +
                    IToDec(abbrev_code));
            }
            cout << endl;
        }
    }

    Dwarf_Signed atcnt = 0;
    Dwarf_Attribute *atlist = 0;
    int atres = dwarf_attrlist(die, &atlist, &atcnt, &err);
    if (atres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_attrlist", atres, err);
    } else if (atres == DW_DLV_NO_ENTRY) {
        /* indicates there are no attrs.  It is not an error. */
        atcnt = 0;
    }


    for (Dwarf_Signed i = 0; i < atcnt; i++) {
        Dwarf_Half attr;
        int ares;

        ares = dwarf_whatattr(atlist[i], &attr, &err);
        if (ares == DW_DLV_OK) {
            bool attr_match = print_attribute(dbg, die, attr,
                            atlist[i],
                            print_information, hsrcfiles);
            if(print_information == false && attr_match) {
                attribute_matched = true;
            }
        } else {
            print_error(dbg, "dwarf_whatattr entry missing", ares, err);
        }
    }

    for (Dwarf_Signed i = 0; i < atcnt; i++) {
        dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
    }
    if (atres == DW_DLV_OK) {
        dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
    }

    if (dense && print_information) {
        cout << endl ;
        cout << endl ;
    }
    return attribute_matched;
}

/* Encodings have undefined signedness. Accept either
   signedness.  The values are small (they are defined
   in the DWARF specification), so the
   form the compiler uses (as long as it is
   a constant value) is a non-issue.

   If string_out is non-NULL, construct a string output, either
   an error message or the name of the encoding.
   The function pointer passed in is to code generated
   by a script at dwarfdump build time. The code for
   the val_as_string function is generated
   from dwarf.h.  See <build dir>/dwarf_names.c

   If string_out is non-NULL then attr_name and val_as_string
   must also be non-NULL.

*/
static int
get_small_encoding_integer_and_name(Dwarf_Debug dbg,
                                    Dwarf_Attribute attrib,
                                    Dwarf_Unsigned * uval_out,
                                    const string &attr_name,
                                    string * string_out,
                                    encoding_type_func val_as_string,
                                    Dwarf_Error * err,
                                    bool show_form)
{
    Dwarf_Unsigned uval = 0;
    int vres = dwarf_formudata(attrib, &uval, err);
    if (vres != DW_DLV_OK) {
        Dwarf_Signed sval = 0;
        vres = dwarf_formsdata(attrib, &sval, err);
        if (vres != DW_DLV_OK) {
            vres = dwarf_global_formref(attrib,&uval,err);
            if (vres != DW_DLV_OK) {
                if (string_out != 0) {
                    string b = attr_name + " has a bad form.";
                    *string_out = b;
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
        *string_out = val_as_string((unsigned) uval,
             dwarf_names_print_on_error);
         Dwarf_Half theform = 0;
         Dwarf_Half directform = 0;
         get_form_values(attrib,theform,directform);
         show_form_itself(show_form, theform, directform,string_out);
    }
    return DW_DLV_OK;
}




/*
 * We need a 32-bit signed number here, but there's no portable
 * way of getting that.  So use __uint32_t instead.  It's supplied
 * in a reliable way by the autoconf infrastructure.
 */

static string
get_FLAG_BLOCK_string(Dwarf_Debug dbg, Dwarf_Attribute attrib)
{
    int fres = 0;
    Dwarf_Block *tempb = 0;
    __uint32_t * array = 0;
    Dwarf_Unsigned array_len = 0;
    __uint32_t * array_ptr;
    Dwarf_Unsigned array_remain = 0;

    /* first get compressed block data */
    fres = dwarf_formblock (attrib,&tempb, &err);
    if (fres != DW_DLV_OK) {
        string msg("DW_FORM_blockn cannot get block");
        print_error(dbg,msg,fres,err);
        return msg;
    }

    /* uncompress block into int array */
    void *vd = dwarf_uncompress_integer_block(dbg,
        1, /* 'true' (meaning signed ints)*/
        32, /* bits per unit */
        reinterpret_cast<void *>(tempb->bl_data),
        tempb->bl_len,
        &array_len, /* len of out array */
        &err);
    if (vd == reinterpret_cast<void *>(DW_DLV_BADADDR)) {
        string msg("DW_AT_SUN_func_offsets cannot uncompress data");
        print_error(dbg,msg,0,err);
        return msg;
    }
    array = reinterpret_cast<__uint32_t *>(vd);
    if (array_len == 0) {
        string msg("DW_AT_SUN_func_offsets has no data");
        print_error(dbg,msg,0,err);
        return msg;
    }
    
    /* fill in string buffer */
    array_remain = array_len;
    array_ptr = array;
    const unsigned array_lim = 8;
    string blank(" ");
    string out_str;
    while (array_remain > array_lim) {
        out_str.append("\n");
        for(unsigned j = 0; j < array_lim; ++j) {
          out_str.append(blank + IToHex0N(array_ptr[0],10));
        }
        array_ptr += array_lim;
        array_remain -= array_lim;
    }

    /* now do the last line */
    if (array_remain > 0) {
        out_str.append("\n ");
        while (array_remain > 0) {
            out_str.append(blank + IToHex0N(*array_ptr,10));
            array_remain--;
            array_ptr++;
        }
    }
    /* free array buffer */
    dwarf_dealloc_uncompressed_block(dbg, array);
    return out_str;
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


string
print_ranges_list_to_extra(Dwarf_Debug dbg,
    Dwarf_Unsigned off,
    Dwarf_Ranges *rangeset,
    Dwarf_Signed rangecount,
    Dwarf_Unsigned bytecount)
{
    string out;
    if(dense) {
        out.append("< ranges: ");
    } else {
        out.append("\t\tranges: ");
    }
    out.append(IToDec(rangecount));
    if(dense) {
        // This is a goofy difference. Historical.
        out.append(" ranges at .debug_ranges offset ");
    } else {
        out.append(" at .debug_ranges offset ");
    }
    out.append(IToDec(off));
    out.append(" (");
    out.append(IToHex(off));
    out.append(") (");
    out.append(IToDec(bytecount));
    out.append(" bytes)");
    if(dense) {
        out.append(">");
    } else {
        out.append("\n");
    }
    for(Dwarf_Signed i = 0; i < rangecount; ++i) {
        Dwarf_Ranges * r = rangeset +i;
        const char *type = get_rangelist_type_descr(r);
        if(dense) {
            out.append("<[");
        } else {
            out.append("\t\t\t[");
        }
        out.append(IToDec(i,2));
        out.append("] ");
        if(dense) {
            out.append(type);
        } else {
            out.append(LeftAlign(14,type));
        }
        out.append(" ");
        out.append(IToHex0N(r->dwr_addr1,dense?0:10));
        out.append(" ");
        out.append(IToHex0N(r->dwr_addr2,dense?0:10));
        if(dense) {
            out.append(">");
        } else {
            out.append("\n");
        }
    }
    return out;
}

/* This is a slightly simplistic rendering of the FORM
   issue, it is not precise. However it is really only
   here so we can detect and report an error (producing
   incorrect DWARF) by a particular compiler (a quite unusual error, 
   noticed in April 2010).
   So this simplistic form suffices.  See the libdwarf get_loclist_n()
   function source for the precise test.
*/
static bool
is_location_form(int form)
{
    if(form == DW_FORM_block1 ||
       form == DW_FORM_block2 ||
       form == DW_FORM_block4 ||
       form == DW_FORM_block || 
       form == DW_FORM_data4 ||
       form == DW_FORM_data8 ||
       form == DW_FORM_sec_offset) {
       return true;
    }  
    return false;
}

static void
show_attr_form_error(Dwarf_Debug dbg,unsigned attr,unsigned form,string *out)
{
    const char *n = 0;
    int res;
    out->append("ERROR: Attribute ");
    out->append(IToDec(attr));
    out->append(" (");
    res = dwarf_get_AT_name(attr,&n);
    if(res != DW_DLV_OK) {
       n = "UknownAttribute";
    }
    out->append(n);
    out->append(") ");
    out->append(" has form ");
    out->append(IToDec(form));
    out->append(" (");
    res = dwarf_get_FORM_name(form,&n);
    if(res != DW_DLV_OK) {
       n = "UknownForm";
    }
    out->append(n);
    out->append("), a form which is not appropriate");
    print_error_and_continue(dbg, out->c_str(), DW_DLV_OK, err);
}


static bool
print_attribute(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attr,
                Dwarf_Attribute attr_in,
                bool print_information,
                SrcfilesHolder & hsrcfiles)
{
    Dwarf_Attribute attrib = 0;
    Dwarf_Unsigned uval = 0;
    string atname;
    string valname;
    string extra;
    int tres = 0;
    Dwarf_Half tag = 0;
    string extra_string;
    bool found_search_attr = false;

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
        string tagname = "<tag invalid>";
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
    case DW_AT_frame_base: {
        /* value is a location description or location list */
        Dwarf_Half theform = 0;
        Dwarf_Half directform = 0;
        get_form_values(attrib,theform,directform);
        if(is_location_form(theform)) {
           get_location_list(dbg, die, attrib, valname);
           show_form_itself(show_form_used, theform, directform,&valname);
        } else if (theform == DW_FORM_exprloc)  {
           bool showhextoo = true;
           print_exprloc_content(dbg,die,attrib,showhextoo,valname);
        } else {
           show_attr_form_error(dbg,attr,theform,&valname);
        }
        }
        break;
    case DW_AT_SUN_func_offsets: {
        Dwarf_Half theform = 0;
        Dwarf_Half directform = 0;
        get_form_values(attrib,theform,directform);
        valname = get_FLAG_BLOCK_string(dbg, attrib);
        show_form_itself(show_form_used, theform, directform,&valname);
        }
        break;
    case DW_AT_SUN_cf_kind:
        {
            Dwarf_Half kind;
            Dwarf_Unsigned tempud;
            Dwarf_Error err;
            Dwarf_Half theform = 0;
            Dwarf_Half directform = 0;
            get_form_values(attrib,theform,directform);
            int wres;
            wres = dwarf_formudata (attrib,&tempud, &err);
            if(wres == DW_DLV_OK) {
                kind = tempud;
                valname = get_ATCF_name(kind,dwarf_names_print_on_error);
            } else if (wres == DW_DLV_NO_ENTRY) {
                valname = "?";
            } else {
                print_error(dbg,"Cannot get formudata....",wres,err);
                valname = "??";
            }
            show_form_itself(show_form_used, theform, directform,&valname);
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

            switch (theform) {
            case DW_FORM_block1: {
                Dwarf_Half theform = 0;
                Dwarf_Half directform = 0;
                get_form_values(attrib,theform,directform);
                get_location_list(dbg, die, attrib, valname);
                show_form_itself(show_form_used, theform, directform,&valname);
                }
                break;
            default:
                get_attr_value(dbg, tag, die,
                    attrib, hsrcfiles, valname,show_form_used);
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
            if( theform != DW_FORM_addr) {
              /* New in DWARF4: other forms are not an address
                 but are instead offset from pc.
                 One could test for DWARF4 here before adding
                 this string, but that seems unnecessary as this
                 could not happen with DWARF3 or earlier. 
                 A normal consumer would have to add this value to
                 DW_AT_low_pc to get a true pc. */
              valname.append("<offset-from-lowpc>");
            }
            get_attr_value(dbg, tag, die, attrib, hsrcfiles, valname,
                show_form_used);
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

            get_attr_value(dbg, tag,die, attrib, hsrcfiles, valname,
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
                           extra = print_ranges_list_to_extra(dbg,off,
                               rangeset,rangecount,bytecount);
                       }
                       dwarf_ranges_dealloc(dbg,rangeset,rangecount);
                    } else if (rres == DW_DLV_ERROR) {
                        cout << "dwarf_get_ranges() " <<
                            "cannot find DW_AT_ranges at offset " <<
                            off <<
                            " (" << IToHex(off) <<
                            ")."; 
                        DWARF_CHECK_ERROR2(ranges_result,
                            get_AT_name(attr,
                                dwarf_names_print_on_error),
                            " cannot find DW_AT_ranges at offset");
                    } else { /* NO ENTRY */
                        cout << "dwarf_get_ranges() " <<
                            "finds no ranges at offset " <<
                            off <<
                            " (" << IToHex(off) <<
                            ")."; 
                        DWARF_CHECK_ERROR2(ranges_result,
                            get_AT_name(attr,
                                dwarf_names_print_on_error),
                            " fails to find DW_AT_ranges at offset");
                    }
                } else if (ures == DW_DLV_ERROR) {
                    cout << 
                        "dwarf_formudata fails to find DW_AT_ranges offset";
                    DWARF_CHECK_ERROR2(ranges_result,
                        get_AT_name(attr,
                            dwarf_names_print_on_error),
                        " fails to find DW_AT_ranges offset");
                } else { /* NO ENTRY */
                    cout << 
                        "dwarf_formudata cannot find DW_AT_ranges offset.";
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
        }
        break;
    default:
        get_attr_value(dbg, tag,die, attrib, hsrcfiles, valname,show_form_used);
        break;
    }
    if (!print_information) {
        if ( (!search_match_text.empty() && 
              !strcmp(valname.c_str(),search_match_text.c_str())) ||
           (!search_any_text.empty() && 
               is_strstrnocase(valname.c_str(), search_any_text.c_str()))
#ifdef HAVE_REGEX
           || (!search_regex_text.empty() && !regexec(&search_re,
                 valname.c_str(),0,NULL,0))
#endif
           ) {
                found_search_attr = true;
        }
    }

    if (print_information) {
        if (dense) {
            cout << " " << atname << BracketSurround(valname);
            cout << extra;
        } else {
            cout << "\t\t" << LeftAlign(28,atname) << valname << endl;
            cout << extra;
        }
    }
    return found_search_attr;
}


// Appends the locdesc to string_out.
// Does not print.
int
dwarfdump_print_one_locdesc(Dwarf_Debug dbg,
                         Dwarf_Locdesc * llbuf,
                         int skip_locdesc_header,
                         string &string_out)
{


    if (!skip_locdesc_header && (verbose || llbuf->ld_from_loclist)) {
        string_out.append(BracketSurround(
           string("lowpc=") + IToHex(llbuf->ld_lopc)));
        string_out.append(BracketSurround(
           string("highpc=") + IToHex(llbuf->ld_hipc)));
        if (verbose) {
            string s("from ");
            s.append(llbuf->
                     ld_from_loclist ? ".debug_loc" : ".debug_info");
            s.append(" offset ");
            s.append(IToHex(llbuf->ld_section_offset));
            string_out.append(BracketSurround(s));
        }
    }


    Dwarf_Locdesc *locd  = llbuf;
    int no_of_ops = llbuf->ld_cents;
    for (int i = 0; i < no_of_ops; i++) {
        Dwarf_Loc * op = &locd->ld_s[i];

        int res = _dwarf_print_one_expr_op(dbg,op,i,string_out);
        if(res == DW_DLV_ERROR) {
          return res;
        }
    }
    return DW_DLV_OK;
}

static bool
op_has_no_operands(int op)
{
    unsigned i = 0; 
    if(op >= DW_OP_lit0 && op <= DW_OP_reg31) {
         return true;
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
            return true;
        }    
        return false;
    }    
    return false;
}

int
_dwarf_print_one_expr_op(Dwarf_Debug dbg,Dwarf_Loc* expr,int index,
        string &string_out)
{
    if (index > 0) {
        string_out.append(" ");
    }

    Dwarf_Small op = expr->lr_atom;
    string op_name = get_OP_name(op,dwarf_names_print_on_error);
    string_out.append(op_name);

    Dwarf_Unsigned opd1 = expr->lr_number;
    if (op_has_no_operands(op)) {
        /* Nothing to add. */
    } else if (op >= DW_OP_breg0 && op <= DW_OP_breg31) {
            char small_buf[40];
            snprintf(small_buf, sizeof(small_buf),
                     "%+" DW_PR_DSd , (Dwarf_Signed) opd1);
            string_out.append(small_buf);
    } else {
        switch (op) {
        case DW_OP_addr:
            string_out.append(" ");
            string_out.append(IToHex(opd1));
            break;
        case DW_OP_const1s:
        case DW_OP_const2s:
        case DW_OP_const4s:
        case DW_OP_const8s:
        case DW_OP_consts:
        case DW_OP_skip:
        case DW_OP_bra:
        case DW_OP_fbreg:
            {
            Dwarf_Signed si = opd1;
            string_out.append(" ");
            string_out.append(IToDec(si));
            }
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
            string_out.append(" ");
            string_out.append(IToDec(opd1));
            break;
        case DW_OP_bregx:
            {
            string_out.append(IToDec(opd1));

            char small_buf[40];
            Dwarf_Unsigned opd2 = expr->lr_number2;
            snprintf(small_buf, sizeof(small_buf),
                "%+" DW_PR_DSd , (Dwarf_Signed) opd2);
            string_out.append(small_buf);
            }
            break;
        case DW_OP_call2:
            string_out.append(IToHex(opd1));

            break;
        case DW_OP_call4:
            string_out.append(IToHex(opd1));

            break;
        case DW_OP_call_ref:
            string_out.append(IToHex0N(opd1,8));
            break;
        case DW_OP_bit_piece:
            {
            string_out.append(IToHex0N(opd1,8));
            string_out.append(" offset ");
            Dwarf_Unsigned opd2 = expr->lr_number2;
            string_out.append(IToHex0N(opd2,8));
            }
            break;
        case DW_OP_implicit_value:
            {
#define IMPLICIT_VALUE_PRINT_MAX 12
            string_out.append(" ");
            string_out.append(IToHex0N(opd1,10));
            // The other operand is a block of opd1 bytes. 
            // FIXME 
            unsigned int print_len = opd1;
            if(print_len > IMPLICIT_VALUE_PRINT_MAX) {
                    print_len = IMPLICIT_VALUE_PRINT_MAX;
            }
            if(print_len > 0) {
                unsigned int i = 0;
                Dwarf_Unsigned opd2 = expr->lr_number2;
                const unsigned char *bp = 
                    reinterpret_cast<const unsigned char *>(opd2);
                string_out.append(" contents 0x");
                for( ; i < print_len; ++i,++bp) {
                    char small_buf[40];
                    snprintf(small_buf, sizeof(small_buf),
                        "%02x", *bp);
                    string_out.append(small_buf);
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
        case DW_OP_APPLE_uninit:
            break;
        default:
            string_out.append(string(" dwarf_op unknown: ") +
                   IToHex((unsigned)op));
            break;
        }
    }
    return DW_DLV_OK;
}

/* Fill buffer with location lists 
   Return DW_DLV_OK if no errors.
*/
 /*ARGSUSED*/ static void
get_location_list(Dwarf_Debug dbg,
    Dwarf_Die die, Dwarf_Attribute attr,
    string &locstr)
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
        dwarfdump_print_one_locdesc(dbg, llbuf,skip_locdesc_header,locstr);
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
        llbuf = llbufarray[llent];

        if (!dense && llbuf->ld_from_loclist) {
            if (llent == 0) {
                locstr.append("<loclist with ");
                locstr.append(IToDec(no_of_elements));
                locstr.append(" entries follows>");
            }
            locstr.append("\n\t\t\t");
            locstr.append("[");
            locstr.append(IToDec(llent,2));
            locstr.append("]");
        }
        lres = dwarfdump_print_one_locdesc(dbg,
              llbuf, 
              skip_locdesc_header,
              locstr);
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

/* We think this is an integer. Figure out how to print it.
   In case the signedness is ambiguous (such as on 
   DW_FORM_data1 (ie, unknown signedness) print two ways.
*/
static int
formxdata_print_value(Dwarf_Attribute attrib, string &str_out,
        Dwarf_Error * err)
{
    Dwarf_Signed tempsd = 0;
    Dwarf_Unsigned tempud = 0;
    Dwarf_Error serr = 0;
    int ures = dwarf_formudata(attrib, &tempud, err);
    int sres = dwarf_formsdata(attrib, &tempsd, &serr);
    if(ures == DW_DLV_OK) {
        if(sres == DW_DLV_OK) {
            if(tempud == static_cast<Dwarf_Unsigned>(tempsd)) {
               /* Data is the same value, so makes no difference which
                we print. */
               str_out.append(IToDec(tempud));
            } else {
               str_out.append(IToDec(tempud));
               str_out.append("(as signed = ");
               str_out.append(IToDec(tempsd));
               str_out.append(")");
            }
        } else if (sres == DW_DLV_NO_ENTRY) {
            str_out.append(IToDec(tempud));
        } else /* DW_DLV_ERROR */{
            str_out.append(IToDec(tempud));
        }
        return DW_DLV_OK;
    } else  if (ures == DW_DLV_NO_ENTRY) {
        if(sres == DW_DLV_OK) {
            str_out.append(IToDec(tempsd));
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
        str_out.append(IToDec(tempsd));
    } else if (sres == DW_DLV_NO_ENTRY) {
        return ures;
    } 
    /* DW_DLV_ERROR */
    return ures;
}

static void
print_exprloc_content(Dwarf_Debug dbg,Dwarf_Die die, 
    Dwarf_Attribute attrib,
    bool showhextoo, string &str_out)
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
        str_out.append( small_buf);
        if(showhextoo) {
                for (u = 0; u < tempud; u++) {
                    snprintf(small_buf, sizeof(small_buf), "%02x",
                         *(u + (unsigned char *) x));
                    str_out.append(small_buf);
                }
                str_out.append(": ");
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
            string v;
            get_string_from_locs(dbg,x,tempud,address_size, v);
            str_out.append(v);
        }
    }
}


/* Fill buffer with attribute value.
   We pass in tag so we can try to do the right thing with
   broken compiler DW_TAG_enumerator 

   We append to str_out.
*/
static void
get_attr_value(Dwarf_Debug dbg, Dwarf_Half tag, 
               Dwarf_Die die, Dwarf_Attribute attrib,
               SrcfilesHolder &hsrcfiles, string &str_out,
               bool show_form)
{
    Dwarf_Signed tempsd = 0;
    Dwarf_Unsigned tempud = 0;
    Dwarf_Half attr = 0;
    Dwarf_Off off = 0;
    Dwarf_Die die_for_check = 0;
    Dwarf_Half tag_for_check = 0;
    Dwarf_Addr addr = 0;
    int bres  = DW_DLV_ERROR;
    int wres  = DW_DLV_ERROR;
    int dres  = DW_DLV_ERROR;
    Dwarf_Half direct_form = 0;
    Dwarf_Half theform = 0;

    int fres = get_form_values(attrib,theform,direct_form);
    if (fres == DW_DLV_ERROR) {
         print_error(dbg, "dwarf_whatform cannot find attr form", fres,
                     err);
    } else if (fres == DW_DLV_NO_ENTRY) {
         return;
    }

    switch (theform) {
    case DW_FORM_addr:
        bres = dwarf_formaddr(attrib, &addr, &err);
        if (bres == DW_DLV_OK) {
            str_out.append(IToHex(addr));
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
            str_out.append(BracketSurround(
               string("global die offset ") +
               IToDec(off)));
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
            print_error(dbg, "reference form with no valid local ref?!", 
                bres, err);
        }
        /* do references inside <> to distinguish them ** from
           constants. In dense form this results in <<>>. Ugly for
           dense form, but better than ambiguous. davea 9/94 */
        str_out.append(BracketSurround(IToDec(off)));
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
        {
            Dwarf_Block *tempb;
            fres = dwarf_formblock(attrib, &tempb, &err);
            if (fres == DW_DLV_OK) {
                for (unsigned i = 0; i < tempb->bl_len; i++) {
                    str_out.append(IToHex02(
                        *(i + (unsigned char *) tempb->bl_data)));
                }
                dwarf_dealloc(dbg, tempb, DW_DLA_BLOCK);
            } else {
                print_error(dbg, "DW_FORM_blockn cannot get block\n", fres,
                            err);
            }
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
                {
                string emptyattrname;
                bool show_form_here = false;
                wres = get_small_encoding_integer_and_name(dbg,
                             attrib,
                             &tempud,
                             emptyattrname,
                             /* err_string */ NULL,
                             (encoding_type_func) 0,
                             &err,show_form_here);

                if (wres == DW_DLV_OK) {
                    str_out.append(IToDec(tempud));
                    if (attr == DW_AT_decl_file || attr == DW_AT_call_file) {
                        Dwarf_Unsigned srccount =  hsrcfiles.count();
                        char **srcfiles = hsrcfiles.srcfiles();
                        if (srcfiles && tempud > 0 && 
                             tempud <= srccount) {
                            /* added by user request */
                            /* srcfiles is indexed starting at 0, but
                               DW_AT_decl_file defines that 0 means no
                               file, so tempud 1 means the 0th entry in
                               srcfiles, thus tempud-1 is the correct
                               index into srcfiles.  */
                            string fname = srcfiles[tempud - 1];
                            str_out.append(" ");
                            str_out.append(fname);
                       }
                       if(check_decl_file) {
                           decl_file_result.checks++;
                           /* Zero is always a legal index, it means
                              no source name provided. */
                           if(tempud > srccount) {
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
                wres = formxdata_print_value(attrib,str_out, &err);
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
                wres = formxdata_print_value(attrib,str_out, &err);
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
            str_out.append(IToDec(tempsd));
        } else if (wres == DW_DLV_NO_ENTRY) {
            /* nothing? */
        } else {
            print_error(dbg, "Cannot get formsdata..", wres, err);
        }
        break;
    case DW_FORM_udata:
        wres = dwarf_formudata(attrib, &tempud, &err);
        if (wres == DW_DLV_OK) {
            str_out.append(IToDec(tempud));
        } else if (wres == DW_DLV_NO_ENTRY) {
            /* nothing? */
        } else {
            print_error(dbg, "Cannot get formudata....", wres, err);
        }
        break;
    case DW_FORM_string:
    case DW_FORM_strp:
        { char *temps = 0;
        wres = dwarf_formstring(attrib, &temps, &err);
        if (wres == DW_DLV_OK) {
            str_out.append(temps);
        } else if (wres == DW_DLV_NO_ENTRY) {
            /* nothing? */
        } else {
            print_error(dbg, "Cannot get a formstr (or a formstrp)....", 
                wres, err);
        }
        }

        break;
    case DW_FORM_flag:
        {
        Dwarf_Bool tempbool;
        wres = dwarf_formflag(attrib, &tempbool, &err);
        if (wres == DW_DLV_OK) {
            if (tempbool) {
                str_out.append("yes(");
                str_out.append(IToDec(tempbool));
                str_out.append(")");
            } else {
                str_out.append("no");
            }
        } else if (wres == DW_DLV_NO_ENTRY) {
            /* nothing? */
        } else {
            print_error(dbg, "Cannot get formflag/p....", wres, err);
        }
        }
        break;
    case DW_FORM_indirect:
        /* We should not ever get here, since the true form was
           determined and direct_form has the DW_FORM_indirect if it is
           used here in this attr. */
        str_out.append( get_FORM_name(theform,
            dwarf_names_print_on_error));
        break;
    case DW_FORM_exprloc: {    /* DWARF4 */
        int showhextoo = true;
        print_exprloc_content(dbg,die,attrib,showhextoo,str_out);
        }
        break;

    case DW_FORM_sec_offset:{ /* DWARF4 */
        string emptyattrname;
        bool show_form_here = false;
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
            str_out.append(IToHex0N(tempud,10));
        }
        }
        
        break;
    case DW_FORM_flag_present: /* DWARF4 */
        str_out.append("yes(1)");
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
            char small_buf[40];
            str_out.append("0x");
            for( unsigned i = 0; i < sizeof(sig8data.signature); ++i) {
                if (i == 4) {
                    str_out.append(" ");
                }
                snprintf(small_buf,sizeof(small_buf), "%02x",
                          (unsigned)(sig8data.signature[i]));
                str_out.append(small_buf);
            }
        }
        }
        break;
    default:
        print_error(dbg, "dwarf_whatform unexpected value", DW_DLV_OK,
                    err);
    }
    show_form_itself(show_form,theform, direct_form,&str_out);
}

static int
get_form_values(Dwarf_Attribute attrib,
    Dwarf_Half & theform, Dwarf_Half & directform)
{
    Dwarf_Error err = 0;
    int res = dwarf_whatform(attrib, &theform, &err);
    dwarf_whatform_direct(attrib, &directform, &err);
    return res;
}
static void
show_form_itself(bool local_show_form,int theform, 
    int directform, string *str_out)
{
    if ((verbose || local_show_form)
        && directform && directform == DW_FORM_indirect) {
        str_out->append(" (used DW_FORM_indirect");
        if(verbose) {
            str_out->append(" ");
            str_out->append(IToDec(DW_FORM_indirect));

        }
        str_out->append( ") ");
    }
    if(local_show_form) {
        str_out->append(" <form ");
        str_out->append(get_FORM_name(theform,
             dwarf_names_print_on_error));
        if(verbose) {
            str_out->append(" ");
            str_out->append(IToDec(theform));
        }
        str_out->append(">");
    }
}


#include "tmp-ta-table.cc"
#include "tmp-ta-ext-table.cc"

static int
legal_tag_attr_combination(Dwarf_Half tag, Dwarf_Half attr)
{
    if(tag <= 0) {
        return false;
    }
    if(tag < ATTR_TREE_ROW_COUNT) {
        int index = attr / BITS_PER_WORD;
        if ( index < ATTR_TREE_COLUMN_COUNT) {
           unsigned bitflag = 1 << (attr % BITS_PER_WORD);
           int known = ((tag_attr_combination_table[tag][index]
                 & bitflag) > 0 ? true : false);
           if(known) {
               return true;
           }
        }
    }
    /* DW_AT_MIPS_fde  used to return true as that was
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
                    return true;
                }
            }
        }
    }
    return (false);
}
#include "tmp-tt-table.cc"
#include "tmp-tt-ext-table.cc"

/*
 *    Look only at valid table entries
 *       The check here must match the building-logic in
 *          tag_tree.cc
 *             And must match the tags defined in dwarf.h
 *                The tag_tree_combination_table is a table of bit flags.
 *                */
static bool
legal_tag_tree_combination(Dwarf_Half tag_parent, Dwarf_Half tag_child)
{
    if(tag_parent <= 0) { 
        return false;
    }    
    if ( tag_parent < TAG_TREE_ROW_COUNT) {
        int index = tag_child / BITS_PER_WORD;
        if ( index < TAG_TREE_COLUMN_COUNT) {
            unsigned bitflag = 1 << (tag_child % BITS_PER_WORD);
            int known = ((tag_tree_combination_table[tag_parent]
                    [index] & bitflag) > 0 ? true : false);
            if(known) {
                return true;
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
                    return true;
                }    
            }    
        }    
    }    
    return (false);
}

