/* 
  Copyright (C) 2006 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007-2011 David Anderson. All Rights Reserved.

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



$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/print_frames.c,v 1.5 2006/06/14 20:34:02 davea Exp $ */

/*  The address of the Free Software Foundation is
    Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
    Boston, MA 02110-1301, USA.
    SGI has moved from the Crittenden Lane address.
*/



#include "globals.h"
#include <vector>
#include <map>
#include <set>
#include "print_frames.h"
#include "dwconf.h"
#include "fderegs.h"



using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::set;

static void
print_one_frame_reg_col(Dwarf_Debug dbg,
    Dwarf_Unsigned rule_id,
    Dwarf_Small value_type,
    Dwarf_Unsigned reg_used,
    Dwarf_Half address_size,
    struct dwconf_s *config_data,
    Dwarf_Signed offset_relevant,
    Dwarf_Signed offset, Dwarf_Ptr block_ptr);

typedef map<Dwarf_Addr,string> LowpcToNameMaptype;
typedef set<Dwarf_Addr> LowpcUsedSettype;



/* For inlined or class mem functions, try to find name */
static int
get_abstract_origin_or_spec_funcname(Dwarf_Debug dbg,
    Dwarf_Attribute attr,
    string &name_out)
{
    Dwarf_Off off = 0;
    Dwarf_Die origin_die = 0;
    Dwarf_Attribute *atlist = NULL;
    Dwarf_Signed atcnt = 0;
    bool name_found = false;
    int res = dwarf_global_formref(attr,&off,&err);
    if(res != DW_DLV_OK) {
        return DW_DLV_NO_ENTRY;
    }
    int dres = dwarf_offdie(dbg,off,&origin_die,&err);
    if(dres != DW_DLV_OK) {
        return DW_DLV_NO_ENTRY;
    }
    int atres = dwarf_attrlist(origin_die, &atlist, &atcnt, &err);
    if (atres != DW_DLV_OK) {
        dwarf_dealloc(dbg,origin_die,DW_DLA_DIE);
        return DW_DLV_NO_ENTRY;
    }
    for (Dwarf_Signed i = 0; i < atcnt; i++) {
        Dwarf_Half lattr;
        int ares;
        ares = dwarf_whatattr(atlist[i], &lattr, &err);
        if (ares == DW_DLV_ERROR) {
            break;
        } else if (ares == DW_DLV_OK) {
            if(lattr == DW_AT_name) {
                int sres = 0;
                char* temps = 0;
                sres = dwarf_formstring(atlist[i], &temps, &err);
                if (sres == DW_DLV_OK) {
                    name_out = temps;
                    name_found = true;
                    break;
                }
            }
        }
    }
    for (Dwarf_Signed i = 0; i < atcnt; i++) {
        dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
    }
    dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
    dwarf_dealloc(dbg,origin_die,DW_DLA_DIE);
    if(!name_found) {
        return DW_DLV_NO_ENTRY;
    }
    return DW_DLV_OK;
}


/*  Returns true  this is a procedure with a name, and sets
    the name in proc_name and the low pc in low_pc_out.
    Else returns false.  */
bool
get_proc_name(Dwarf_Debug dbg, Dwarf_Die die, 
    string & proc_name, Dwarf_Addr & low_pc_out)
{
    Dwarf_Signed atcnt = 0;
    Dwarf_Signed i = 0;
    Dwarf_Attribute *atlist = NULL;
    bool funcpcfound = false;
    bool funcnamefound = false;

    int atres = dwarf_attrlist(die, &atlist, &atcnt, &err);
    if (atres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_attrlist", atres, err);
        return false;
    }
    if (atres == DW_DLV_NO_ENTRY) {
        return false;
    }
    for (i = 0; i < atcnt; i++) {
        Dwarf_Half attr;
        int ares;
        char *temps;
        int sres;
        int dres;

        if (funcnamefound == true && funcpcfound == true) {
            /* stop as soon as both found */
            break;
        }
        ares = dwarf_whatattr(atlist[i], &attr, &err);
        if (ares == DW_DLV_ERROR) {
            print_error(dbg, "get_proc_name whatattr error", ares, err);
        } else if (ares == DW_DLV_OK) {
            switch (attr) {
            case DW_AT_specification:
            case DW_AT_abstract_origin:
                {
                    if(!funcnamefound) {
                        string aotemp;
                        /*  Only use this if we have not seen DW_AT_name
                            yet .*/
                        int aores = get_abstract_origin_or_spec_funcname(dbg,
                            atlist[i], aotemp);
                        if(aores == DW_DLV_OK) {
                            /* FOUND THE NAME */
                            proc_name = aotemp;
                            funcnamefound = 1;
                        }
                    }
                }
                break;
            case DW_AT_name:
                sres = dwarf_formstring(atlist[i], &temps, &err);
                if (sres == DW_DLV_ERROR) {
                    print_error(dbg,
                        "formstring in get_proc_name failed",
                        sres, err);
                    /*  50 is safe wrong length since is bigger than the 
                        actual string */
                    proc_name = "ERROR in dwarf_formstring!";
                } else if (sres == DW_DLV_NO_ENTRY) {
                    /*  50 is safe wrong length since is bigger than the 
                        actual string */
                    proc_name = "NO ENTRY on dwarf_formstring?!";
                } else {
                    proc_name = temps;
                }
                funcnamefound = 1;      /* FOUND THE NAME */
                break;
            case DW_AT_low_pc:
                dres = dwarf_formaddr(atlist[i], &low_pc_out, &err);
                if (dres == DW_DLV_ERROR) {
                    print_error(dbg, "formaddr in get_proc_name failed",
                        dres, err);
                } else if (dres == DW_DLV_OK) {
                    funcpcfound = true;
                }
                break;
            default:
                break;
            }
        }
    }
    for (i = 0; i < atcnt; i++) {
        dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
    }
    dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
    if (funcnamefound == 0 || funcpcfound == 0 ) {
        return false;
    }
    return true;
}

/*  Nested search since some languages, including SGI MP Fortran,
    have nested functions.
 
    Loads all the subprogram names it can find in the current
    sibling/child chain into the pcMap.
    Do not stop except on error.  */
static void
load_nested_proc_names(Dwarf_Debug dbg, Dwarf_Die die, 
    string &proc_name, LowpcToNameMaptype & pcMap)
{
    Dwarf_Die curdie = die;
    int die_locally_gotten = 0;
    Dwarf_Half tag;
    Dwarf_Error err = 0;
    int chres = DW_DLV_OK;
    while (chres == DW_DLV_OK) {
        int tres = dwarf_tag(curdie, &tag, &err);
        err = 0;
        if (tres == DW_DLV_OK) {
            int lchres;

            if (tag == DW_TAG_subprogram) {
                Dwarf_Addr proc_low_pc = 0;
                bool proc_name_v = get_proc_name(dbg, curdie, proc_name,
                    proc_low_pc);
                if (proc_name_v) {
                    pcMap[proc_low_pc] = proc_name;
                }
                /*  Check children of subprograms recursively. Should
                    this really be checking  children of anything,
                    or just children of subprograms? */
                Dwarf_Die newchild = 0;
                lchres = dwarf_child(curdie, &newchild, &err);
                if (lchres == DW_DLV_OK) {
                    /* Look for inner subprogram. */
                    load_nested_proc_names(dbg, newchild, 
                        proc_name, pcMap);
                    dwarf_dealloc(dbg, newchild, DW_DLA_DIE);
                } else if (lchres == DW_DLV_NO_ENTRY) {
                    /* nothing to do */
                } else {
                    print_error(dbg,
                        "load_nested_proc_names dwarf_child() failed ",
                        chres, err);
                }
            }                   /* end if TAG_subprogram */
        } else {
            print_error(dbg, "no tag on child read ", tres, err);
            break;
        }
        /* Try next sibling */
        Dwarf_Die newsibling = 0;
        chres = dwarf_siblingof(dbg, curdie, &newsibling, &err);
        if (chres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf_cu_header On Child read ", chres,
                err);
            break;
        } else if (chres == DW_DLV_NO_ENTRY) {
            // At the end of sibling chain of this nesting level.
            break;
        } else {                /* DW_DLV_OK */
            if (die_locally_gotten) {
                /*  If we got this die from the parent, we do not want
                    to dealloc here! */
                dwarf_dealloc(dbg, curdie, DW_DLA_DIE);
            }
            curdie = newsibling;
            die_locally_gotten = 1;
        }

    }
    if (die_locally_gotten) {
        /*  If we got this die from the parent, we do not want to
            dealloc here! */
        dwarf_dealloc(dbg, curdie, DW_DLA_DIE);
    }
    return;
}

/*  For SGI MP Fortran and other languages, functions 
    nest!  As a result, we must dig thru all functions, 
    not just the top level.

    This remembers the CU die and restarts each search at the start
    of  the current cu.
    If we find nothing we return an empty string.  */
static string
get_fde_proc_name(Dwarf_Debug dbg, Dwarf_Addr low_pc,
    LowpcToNameMaptype & pcMap,
    bool & all_cus_seen)
{
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Unsigned     abbrev_offset = 0;
    Dwarf_Half version_stamp = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Unsigned next_cu_offset = 0;
    int cures = DW_DLV_ERROR;
    int chres = DW_DLV_ERROR;
    string proc_name;

    LowpcToNameMaptype::const_iterator it = pcMap.find(low_pc); 
    if(it != pcMap.end()) {
        string s = it->second;
        return s;
    }
    if(all_cus_seen) {
        return "";
    }

    // Loop through the CUs 
    for (;;) {
        cures = dwarf_next_cu_header(dbg, &cu_header_length,
            &version_stamp, &abbrev_offset,
            &address_size, &next_cu_offset,
            &err);

        if (cures == DW_DLV_NO_ENTRY) {
            all_cus_seen = true;
            break;
        } else if (cures == DW_DLV_ERROR) {
            // Nothing much we can do here.
            all_cus_seen = true;
            break;
        }

        Dwarf_Die current_cu_die_for_print_frames(0);
        int dres = dwarf_siblingof(dbg, NULL, 
            &current_cu_die_for_print_frames, &err);
        if (dres == DW_DLV_ERROR) {
            print_error(dbg,
                "dwarf_cu_header Child Read finding proc name for .debug_frame",
                chres, err);
            continue;
        } else if (dres == DW_DLV_NO_ENTRY) {
            continue;
        }
        /* DW_DLV_OK */
        Dwarf_Die child = 0;
        int chres = dwarf_child(current_cu_die_for_print_frames, &child,
            &err);
        if (chres == DW_DLV_ERROR) {
            print_error(dbg, "dwarf Child Read ", chres, err);
        } else if (chres == DW_DLV_NO_ENTRY) {

            ;  /* do nothing, loop on cu */
        } else { 
            /* DW_DLV_OK */
            // find All the subprograms for this CU and use
            // pcMap to associate each name with its low_pc!
            load_nested_proc_names(dbg, child, proc_name,
                pcMap);
            dwarf_dealloc(dbg, child, DW_DLA_DIE);
        } 
        dwarf_dealloc(dbg, current_cu_die_for_print_frames, DW_DLA_DIE);
        LowpcToNameMaptype::const_iterator it = pcMap.find(low_pc); 
        if(it != pcMap.end()) {
            // If we need more CUs later we will process
            // them as needed (later), but we have done enough
            // CUs to satisfy this low_pc.
            string s = it->second;
            return s;
        }
    }
    return "";
}


/*
    Gather the fde print logic here so the control logic
    determining what FDE to print is clearer.
*/
int
print_one_fde(Dwarf_Debug dbg, Dwarf_Fde fde,
    Dwarf_Unsigned fde_index,
    Dwarf_Cie * cie_data,
    Dwarf_Signed cie_element_count,
    Dwarf_Half address_size, int is_eh,
    struct dwconf_s *config_data,
    LowpcToNameMaptype & pcMap,
    LowpcUsedSettype &lowpcSet,
    bool &all_cus_seen)
{
    Dwarf_Addr low_pc = 0;
    Dwarf_Unsigned func_length = 0;
    Dwarf_Ptr fde_bytes = NULL;
    Dwarf_Unsigned fde_bytes_length = 0;
    Dwarf_Off cie_offset = 0;
    Dwarf_Signed cie_index = 0;
    Dwarf_Off fde_offset = 0;
    Dwarf_Signed eh_table_offset = 0;
    Dwarf_Error err = 0;
    bool printed_intro_addr = false;

    int fres = dwarf_get_fde_range(fde,
        &low_pc, &func_length,
        &fde_bytes,
        &fde_bytes_length,
        &cie_offset, &cie_index,
        &fde_offset, &err);
    if (fres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_get_fde_range", fres, err);
    }
    if (fres == DW_DLV_NO_ENTRY) {
        return DW_DLV_NO_ENTRY;
    }
    if (cu_name_flag &&
        fde_offset_for_cu_low != DW_DLV_BADOFFSET &&
        (fde_offset < fde_offset_for_cu_low ||
        fde_offset > fde_offset_for_cu_high)) {
        return DW_DLV_NO_ENTRY;
    }
    /* eh_table_offset is IRIX ONLY. */
    fres = dwarf_get_fde_exception_info(fde, &eh_table_offset, &err);
    if (fres == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_get_fde_exception_info", fres, err);
    }
    string temps;
    if(!suppress_nested_name_search) {
        temps = get_fde_proc_name(dbg, low_pc,
            pcMap,all_cus_seen);
        LowpcUsedSettype::const_iterator it = lowpcSet.find(low_pc);
        if(check_frames || check_frames_extended) {
            DWARF_CHECK_COUNT(fde_duplication,1);
        }
        if (it != lowpcSet.end()) {
            if(check_frames || check_frames_extended ) {
                string msg = string("An fde low pc of ") + IToHex(low_pc) +
                    string(" is not the first fde with that pc. ");
                if(temps.empty()) {
                    msg.append("The first is not named."); 
                } else {
                    msg.append(string("The first is named \"")+
                    temps + string("\"") );
                }
                DWARF_CHECK_ERROR(fde_duplication,msg);
            }
        } else {
            lowpcSet.insert(low_pc);
        }
    }
    if(!check_frames_extended) {
        cout << BracketSurround(IToDec(cie_index,5));
        cout << BracketSurround(IToHex0N(low_pc,10) + string(":")+
            IToHex0N(low_pc + func_length,10));
        cout << BracketSurround(temps);
        cout << BracketSurround(string("fde offset ") +
            IToHex0N(fde_offset,10) + string(" length: ") +
            IToHex0N(fde_bytes_length,10));
    }

    if (!is_eh) {
        /* IRIX uses eh_table_offset. */
        if(!check_frames_extended) {
            if (eh_table_offset == DW_DLX_NO_EH_OFFSET) {
                cout << BracketSurround(
                    string("eh offset none")) << endl;
            } else if (eh_table_offset == DW_DLX_EH_OFFSET_UNAVAILABLE) {
                cout << BracketSurround(
                    string("eh offset unknown")) << endl;
            } else {
                cout << BracketSurround(
                    string("eh offset ") + IToHex(eh_table_offset))   << endl;
            }
        }
    } else {
        int ares = 0;
        Dwarf_Small *data = 0;
        Dwarf_Unsigned len = 0;

        ares = dwarf_get_fde_augmentation_data(fde, &data, &len, &err);
        if (ares == DW_DLV_NO_ENTRY) {
            /* do nothing. */
        } else if (ares == DW_DLV_OK) {
            if(!check_frames_extended) {
                cout << "<eh aug data len " << IToHex(len);
                for (unsigned k2 = 0; k2 < len; ++k2) {
                    if (k2 == 0) {
                        cout <<" bytes 0x";
                    }
                    cout << IToHex02(data[k2])<< " ";
                }
                cout << ">";
            }
        }                       /* else DW_DLV_ERROR, do nothing */
        if(!check_frames_extended) {
            cout << endl;
        }
    }

    for (Dwarf_Addr j = low_pc; j < low_pc + func_length; j++) {
        FdeRegs fder(fde,config_data);
        fder.setPc(j);
        int fires = fder.preliminaryRead(&err);
        if (fires == DW_DLV_ERROR) {
            print_error(dbg,
                "dwarf_get_fde_info_for_reg", fires, err);
        }
        if (fires == DW_DLV_NO_ENTRY) {
            continue;
        }
        if (config_data->cf_interface_number == 3) {
            Dwarf_Addr row_pc = 0;
            Dwarf_Regtable_Entry3 cfadata;
            // cfdata is a plain-C struct from libdwarf.
            memset(&cfadata,0,sizeof(cfadata));
            int fires2 = fder.getCfaRegdata(&cfadata,&row_pc,&err);
            if (fires2 == DW_DLV_ERROR) {
                print_error(dbg,
                    "dwarf_get_fde_info_for_reg", fires, err);
            }
            if (fires2 == DW_DLV_NO_ENTRY) {
                continue;
            }
            if (row_pc != j) {
                /* duplicate row */
                continue;
            }
            if (!printed_intro_addr & !check_frames_extended) {
                cout <<"        ";
                cout << IToHex0N(j,10);
                cout <<": ";
                printed_intro_addr = true;
            }
            print_one_frame_reg_col(dbg, config_data->cf_cfa_reg,
                cfadata.dw_value_type,
                cfadata.dw_regnum,
                address_size,
                config_data,
                cfadata.dw_offset_relevant, 
                cfadata.dw_offset_or_block_len,
                cfadata.dw_block_ptr);
        }
        for (unsigned k = 0; k < config_data->cf_table_entry_count; k++) {
            Dwarf_Addr row_pc = 0;
            Dwarf_Regtable_Entry3 cfadata;
            memset(&cfadata,0, sizeof(cfadata));

            int fires3 = fder.getRegdata(k,&cfadata,&row_pc,&err);

            if (fires3 == DW_DLV_ERROR) {
                cout << endl;
                print_error(dbg,
                    "dwarf_get_fde_info_for_reg", fires, err);
            }
            if (fires == DW_DLV_NO_ENTRY) {
                continue;
            }
            if (row_pc != j) {
                /* duplicate row */
                break;
            }
            if (!printed_intro_addr && !check_frames_extended) {
                cout << "        " << IToHex0N(j,10) << ": ";
                printed_intro_addr = true;
            }
            print_one_frame_reg_col(dbg,k,
                cfadata.dw_value_type,
                cfadata.dw_regnum,
                address_size,
                config_data,
                cfadata.dw_offset_relevant, 
                cfadata.dw_offset_or_block_len, 
                cfadata.dw_block_ptr);
        }
        if (printed_intro_addr) {
            cout << endl;
            printed_intro_addr = false;
        }
    }
    if (verbose > 1) {
        Dwarf_Off fde_off;
        Dwarf_Off cie_off;

        /*  Get the fde instructions and print them in raw form, just
            like cie instructions */
        Dwarf_Ptr instrs;
        Dwarf_Unsigned ilen;
        int res;

        res = dwarf_get_fde_instr_bytes(fde, &instrs, &ilen, &err);
        int offres =
            dwarf_fde_section_offset(dbg, fde, &fde_off, &cie_off,
                &err);
        if (offres == DW_DLV_OK) {
            if(!check_frames_extended) {
                cout << " fde section offset " << IToDec(fde_off) <<
                    " " <<
                    IToHex0N(fde_off,10);
                cout << " cie offset for fde: " << IToDec(cie_off) <<
                    " " <<
                    IToHex0N(cie_off,10);
                cout << endl;
            }
        }

        if (res == DW_DLV_OK) {
            int cires = 0;
            Dwarf_Unsigned cie_length = 0;
            Dwarf_Small version = 0;
            Dwarf_Unsigned code_alignment_factor = 0;
            Dwarf_Signed data_alignment_factor = 0;
            Dwarf_Half return_address_register_rule = 0;
            Dwarf_Ptr initial_instructions = 0;
            Dwarf_Unsigned initial_instructions_length = 0;

            if (cie_index >= cie_element_count) {
                cout << "Bad cie index " << IToDec(cie_index);
                cout << " with fde index " << IToDec(fde_index);
                cout << "! (table entry max " << IToDec(cie_element_count);
                cout << ")" << endl;
                exit(1);
            }

            char *augmenter_arg = 0;
            cires = dwarf_get_cie_info(cie_data[cie_index],
                &cie_length,
                &version,
                &augmenter_arg,
                &code_alignment_factor,
                &data_alignment_factor,
                &return_address_register_rule,
                &initial_instructions,
                &initial_instructions_length,
                &err);
            if (cires == DW_DLV_ERROR) {
                cout << "Bad cie index " << IToDec(cie_index);
                cout << " with fde index " << IToDec(fde_index);
                cout << "!" << endl;
                print_error(dbg, "dwarf_get_cie_info", cires, err);
            }
            if (cires == DW_DLV_NO_ENTRY) {
                ;               /* ? */
            } else {
                if(!check_frames_extended) {
                    print_frame_inst_bytes(dbg, instrs,
                        (Dwarf_Signed) ilen,
                        data_alignment_factor,
                        (int) code_alignment_factor,
                        address_size, config_data);
                }
            }
        } else if (res == DW_DLV_NO_ENTRY) {
            cout <<"Impossible: no instr bytes for fde index " <<
                IToDec(fde_index) << endl;
        } else {
            /* DW_DLV_ERROR */
            cout << "Error: on gettinginstr bytes for fde index " <<
                IToDec(fde_index) << endl;
            print_error(dbg, "dwarf_get_fde_instr_bytes", res, err);
        }

    }
    return DW_DLV_OK;
}


/* Print a cie.  Gather the print logic here so the
   control logic deciding what to print
   is clearer.
*/
int
print_one_cie(Dwarf_Debug dbg, Dwarf_Cie cie,
    Dwarf_Unsigned cie_index, Dwarf_Half address_size,
    struct dwconf_s *config_data)
{
    int cires = 0;
    Dwarf_Unsigned cie_length = 0;
    Dwarf_Small version = 0;
    Dwarf_Unsigned code_alignment_factor = 0;
    Dwarf_Signed data_alignment_factor = 0;
    Dwarf_Half return_address_register_rule = 0;
    Dwarf_Ptr initial_instructions = 0;
    Dwarf_Unsigned initial_instructions_length = 0;
    Dwarf_Off cie_off = 0;
    Dwarf_Error err = 0;

    char *augmenter_arg = 0;
    cires = dwarf_get_cie_info(cie,
        &cie_length,
        &version,
        &augmenter_arg,
        &code_alignment_factor,
        &data_alignment_factor,
        &return_address_register_rule,
        &initial_instructions,
        &initial_instructions_length, &err);
    if (cires == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_get_cie_info", cires, err);
    }
    if (cires == DW_DLV_NO_ENTRY) {
        cout << "Impossible DW_DLV_NO_ENTRY on cie " <<
            IToDec(cie_index) << endl;
        return DW_DLV_NO_ENTRY;
    }
    {
        if(!check_frames_extended) {
            string augmenter = augmenter_arg;
            cout << BracketSurround(IToDec(cie_index,5));
            cout << "\tversion\t\t\t\t" << static_cast<int>(version) << endl;
            cires = dwarf_cie_section_offset(dbg, cie, &cie_off, &err);
            if (cires == DW_DLV_OK) {
                cout << "\tcie section offset\t\t" << IToDec(cie_off);
                cout << " " << IToHex0N(cie_off,10) << endl;
            }
    
            cout << "\taugmentation\t\t\t" << augmenter << endl;
            cout << "\tcode_alignment_factor\t\t" <<
                code_alignment_factor << endl;
            cout << "\tdata_alignment_factor\t\t" <<
                data_alignment_factor << endl;
            cout << "\treturn_address_register\t\t" <<
                return_address_register_rule << endl;
        }
        {
            int ares = 0;
            Dwarf_Small *data = 0;
            Dwarf_Unsigned len = 0;

            ares =
                dwarf_get_cie_augmentation_data(cie, &data, &len, &err);
            if (ares == DW_DLV_NO_ENTRY) {
                /* do nothing. */
            } else if (ares == DW_DLV_OK && len > 0) {
                if(!check_frames_extended) {
                    cout << " eh aug data len " <<
                        IToHex(len);
                    for (unsigned k2 = 0; data && k2 < len; ++k2) {
                        if (k2 == 0) {
                            cout <<" bytes 0x";
                        }
                        cout << IToHex02(data[k2]) << " ";
                    }
                    cout << endl;
                }
            }  /* else DW_DLV_ERROR or no data, do nothing */
        }

        if(!check_frames_extended) {
            cout <<
                "\tbytes of initial instructions\t" <<
                IToDec(initial_instructions_length) << endl;
            cout <<"\tcie length\t\t\t" <<IToDec(cie_length) << endl;
            cout << "\tinitial instructions" << endl;
            print_frame_inst_bytes(dbg, initial_instructions, 
                (Dwarf_Signed) initial_instructions_length,
                data_alignment_factor,
                (int) code_alignment_factor,
                address_size, config_data);
        }
    }
    return DW_DLV_OK;
}

void
get_string_from_locs(Dwarf_Debug dbg,
    Dwarf_Ptr bytes_in, 
    Dwarf_Unsigned block_len,
    Dwarf_Half addr_size, 
    string &out_string)
{

    Dwarf_Locdesc *locdescarray = 0;
    Dwarf_Signed listlen = 0;
    Dwarf_Error err2 =0;
    int skip_locdesc_header=1;
    int res = 0;
    int res2 = dwarf_loclist_from_expr_a(dbg,
        bytes_in,block_len,
        addr_size,
        &locdescarray,
        &listlen,&err2);
    if (res2 == DW_DLV_ERROR) {
        print_error(dbg, "dwarf_get_loclist_from_expr_a",
            res2, err2);
    }
    if(res2==DW_DLV_NO_ENTRY) {
        return;
    }
    /* lcnt is always 1 */

    /* Use locdescarray  here.*/
    res = dwarfdump_print_one_locdesc(dbg,
        locdescarray,
        skip_locdesc_header,
        out_string);
    if(res != DW_DLV_OK) {
        cout <<"Bad status from _dwarf_print_one_locdesc " << 
            res << endl;
        exit(1);
    }
    dwarf_dealloc(dbg, locdescarray->ld_s, DW_DLA_LOC_BLOCK);
    dwarf_dealloc(dbg, locdescarray, DW_DLA_LOCDESC);
    return ;
}

/*  Print the frame instructions in detail for a glob of instructions.
*/

/*ARGSUSED*/ void
print_frame_inst_bytes(Dwarf_Debug dbg,
    Dwarf_Ptr cie_init_inst, Dwarf_Signed len,
    Dwarf_Signed data_alignment_factor,
    int code_alignment_factor, Dwarf_Half addr_size,
    struct dwconf_s *config_data)
{
    unsigned char *instp = (unsigned char *) cie_init_inst;
    Dwarf_Unsigned uval = 0;
    Dwarf_Unsigned uval2 = 0;
    unsigned int uleblen = 0;
    unsigned int off = 0;
    unsigned int loff = 0;
    unsigned short u16 = 0;
    unsigned int u32 = 0;
    unsigned long long u64 = 0;

    for (; len > 0;) {
        unsigned char ibyte = *instp;
        int top = ibyte & 0xc0;
        int bottom = ibyte & 0x3f;
        int delta = 0;
        int reg = 0;
        const char *cfa_name_x = 0;
        string cfa_name("Unknown-frame-operator");
        int res = dwarf_get_CFA_name(top,&cfa_name_x);
        // The odd character of DFA symbols because of
        // the packing means DW_CFA_extended and DW_CFA_nop
        // conflict.
        if(res == DW_DLV_OK) {
            cfa_name = cfa_name_x;
        }
        switch (top) {
        case DW_CFA_advance_loc:
            delta = ibyte & 0x3f;
            cout << "\t" << IToDec(off,2);
            cout << " DW_CFA_advance_loc " << 
                (delta * code_alignment_factor);
            if (verbose) {
                cout <<"  (" << delta << " * " <<
                    code_alignment_factor << ")";
            }
            cout << endl;
            break;
        case DW_CFA_offset:
            loff = off;
            reg = ibyte & 0x3f;
            uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
            instp += uleblen;
            len -= uleblen;
            off += uleblen;
            cout << "\t" << IToDec(loff,2);
            cout << " DW_CFA_offset " ;
            printreg((Dwarf_Signed) reg, config_data);
            cout << " " << (((Dwarf_Signed) uval) * data_alignment_factor);
            if (verbose) {
                cout << "  (" << uval << " * " << data_alignment_factor <<
                    ")";
            }
            cout << endl;
            break;

        case DW_CFA_restore:
            reg = ibyte & 0x3f;
            cout << "\t" << IToDec(off,2) << SpaceSurround(cfa_name);
            printreg((Dwarf_Signed) reg, config_data);
            cout << endl;
            break;

        default:
            res = dwarf_get_CFA_name(bottom,&cfa_name_x);
            if(res == DW_DLV_OK) {
                cfa_name = cfa_name_x;
            }
            loff = off;
            switch (bottom) {
            case DW_CFA_set_loc:
                /* operand is address, so need address size */
                /* which will be 4 or 8. */
                switch (addr_size) {
                case 4:
                    {
                        __uint32_t v32 = 0;
                        memcpy(&v32, instp + 1, addr_size);
                        uval = v32;
                    }
                    break;
                case 8:
                    {
                        __uint64_t v64 = 0;
                        memcpy(&v64, instp + 1, addr_size);
                        uval = v64;
                    }
                    break;
                default:
                    cout <<
                        "Error: Unexpected address size " <<
                        addr_size << " in DW_CFA_set_loc!" << endl;
                    uval = 0;
                }

                instp += addr_size;
                len -= (Dwarf_Signed) addr_size;
                off += addr_size;
                cout << "\t" << IToDec(loff,2);
                cout << " DW_CFA_set_loc " << uval << endl;
                break;
            case DW_CFA_advance_loc1:
                delta = (unsigned char) *(instp + 1);
                uval2 = delta;
                instp += 1;
                len -= 1;
                off += 1;
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name) << uval2 << endl;
                break;
            case DW_CFA_advance_loc2:
                memcpy(&u16, instp + 1, 2);
                uval2 = u16;
                instp += 2;
                len -= 2;
                off += 2;
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name) << uval2 << endl;
                break;
            case DW_CFA_advance_loc4:
                memcpy(&u32, instp + 1, 4);
                uval2 = u32;
                instp += 4;
                len -= 4;
                off += 4;
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name) << uval2 << endl;
                break;
            case DW_CFA_MIPS_advance_loc8:
                memcpy(&u64, instp + 1, 8);
                uval2 = u64;
                instp += 8;
                len -= 8;
                off += 8;
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name) << uval2 << endl;
                break;
            case DW_CFA_offset_extended:
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                uval2 =
                    local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name);
                printreg((Dwarf_Signed) uval, config_data);
                cout << " " << (Dwarf_Signed) (((Dwarf_Signed) uval2) *
                    data_alignment_factor);
                if (verbose) {
                    cout << "  (" << uval2 << " * " << 
                        data_alignment_factor << ")";
                }
                cout << endl;
                break;

            case DW_CFA_restore_extended:
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name);
                printreg((Dwarf_Signed) uval, config_data);
                cout << endl;
                break;
            case DW_CFA_undefined:
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name);
                printreg((Dwarf_Signed) uval, config_data);
                cout << endl;
                break;
            case DW_CFA_same_value:
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                cout << "\t" << IToDec(loff,2);
                cout <<  SpaceSurround(cfa_name);
                printreg((Dwarf_Signed) uval, config_data);
                cout << endl;
                break;
            case DW_CFA_register:
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                uval2 =
                    local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name);
                printreg((Dwarf_Signed) uval, config_data);
                cout <<" = ";
                printreg((Dwarf_Signed) uval2, config_data);
                cout << endl;
                break;
            case DW_CFA_remember_state:
                cout << "\t" << IToDec(loff,2);
                cout << " " << cfa_name;
                cout << endl;
                break;
            case DW_CFA_restore_state:
                cout << "\t" << IToDec(loff,2);
                cout << " " << cfa_name;
                cout << endl;
                break;
            case DW_CFA_def_cfa:
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                uval2 =
                    local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name);
                printreg((Dwarf_Signed) uval, config_data);
                cout << " " << uval2;
                cout << endl;
                break;
            case DW_CFA_def_cfa_register:
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name);
                printreg((Dwarf_Signed) uval, config_data);
                cout << endl;
                break;
            case DW_CFA_def_cfa_offset:
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name) << uval;
                cout << endl;
                break;

            case DW_CFA_nop:
                cout << "\t" << IToDec(loff,2);
                // cfa name is wrong here due to
                // cfa operation value conflict
                cout << " " << "DW_CFA_nop";
                cout << endl;
                break;

            case DW_CFA_def_cfa_expression:     /* DWARF3 */
                {
                    Dwarf_Unsigned block_len =
                        local_dwarf_decode_u_leb128(instp + 1,
                            &uleblen);

                    instp += uleblen;
                    len -= uleblen;
                    off += uleblen;
                    cout << "\t" << IToDec(loff,2);
                    cout <<
                        " " << cfa_name << " expr block len " <<
                        block_len << endl;
                    dump_block("\t\t", (char *) instp+1,
                        (Dwarf_Signed) block_len);
                    cout << endl;
                    if(verbose) {
                        string exprstring;
                        get_string_from_locs(dbg,
                            instp+1,block_len, addr_size,exprstring);
                        cout << "\t\t" << exprstring << endl;
                    }
                    instp += block_len;
                    len -= block_len;
                    off += block_len;
                }
                break;
            case DW_CFA_expression:     /* DWARF3 */
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                {
                    /*  instp is always 1 byte back, so we need +1
                        when we use it. See the final increment
                        of this for loop. */
                    Dwarf_Unsigned block_len =
                        local_dwarf_decode_u_leb128(instp + 1,
                            &uleblen);

                    instp += uleblen;
                    len -= uleblen;
                    off += uleblen;
                    cout << "\t" << IToDec(loff,2);
                    cout << SpaceSurround(cfa_name) << uval ;
                    cout << " expr block len " << block_len << endl;
                    dump_block("\t\t", (char *) instp+1,
                        (Dwarf_Signed) block_len);
                    cout << endl;
                    if(verbose) {
                        string exprstring;
                        get_string_from_locs(dbg,
                            instp+1,block_len, addr_size,exprstring);
                        cout<< "\t\t" <<exprstring << endl;
                    }
                    instp += block_len;
                    len -= block_len;
                    off += block_len;
                }

                break;
            case DW_CFA_offset_extended_sf: /* DWARF3 */
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                {
                    /*  instp is always 1 byte back, so we need +1
                        when we use it. See the final increment
                        of this for loop. */
                    Dwarf_Signed sval2 =
                        local_dwarf_decode_s_leb128(instp + 1,
                            &uleblen);

                    instp += uleblen;
                    len -= uleblen;
                    off += uleblen;
                    cout << "\t" << IToDec(loff,2);
                    cout << SpaceSurround(cfa_name);
                    printreg((Dwarf_Signed) uval, config_data);
                    cout << " " << ((Dwarf_Signed)
                        ((sval2) * data_alignment_factor));
                    if (verbose) {
                        cout << "  (" << sval2 << " * "<<
                            data_alignment_factor << ")";
                    }
                }
                cout << endl;
                break;
            case DW_CFA_def_cfa_sf:     /* DWARF3 */
                /*  instp is always 1 byte back, so we need +1
                    when we use it. See the final increment
                    of this for loop. */
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                {
                    Dwarf_Signed sval2 =
                        local_dwarf_decode_s_leb128(instp + 1,
                            &uleblen);

                    instp += uleblen;
                    len -= uleblen;
                    off += uleblen;
                    cout << "\t" << IToDec(loff,2);
                    cout << SpaceSurround(cfa_name);
                    printreg((Dwarf_Signed) uval, config_data);
                    cout << " "<< sval2 ;
                    cout << " (*data alignment factor=>" <<
                        ((Dwarf_Signed)(sval2*data_alignment_factor)) <<
                        ")";
                }
                cout << endl;
                break;
            case DW_CFA_def_cfa_offset_sf:      /* DWARF3 */
                {
                    /* instp is always 1 byte back, so we need +1
                        when we use it. See the final increment
                        of this for loop. */
                    Dwarf_Signed sval =
                        local_dwarf_decode_s_leb128(instp + 1,
                            &uleblen);

                    instp += uleblen;
                    len -= uleblen;
                    off += uleblen;
                    cout << "\t" << IToDec(loff,2);
                    cout << SpaceSurround(cfa_name) << sval;
                    cout << " (*data alignment factor=> "<< 
                        ((Dwarf_Signed)(sval*data_alignment_factor)) <<
                        ")" << endl;
                }
                break;
            case DW_CFA_val_offset:     /* DWARF3 */
                /*  instp is always 1 byte back, so we need +1
                    when we use it. See the final increment
                    of this for loop. */
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                {
                    Dwarf_Signed sval2 =
                        local_dwarf_decode_s_leb128(instp + 1,
                            &uleblen);
                    instp += uleblen;
                    len -= uleblen;
                    off += uleblen;
                    cout << "\t" << IToDec(loff,2);
                    cout << SpaceSurround(cfa_name);
                    printreg((Dwarf_Signed)uval, config_data);
                    cout << " " <<
                        ((Dwarf_Signed) (sval2 *
                            data_alignment_factor));
                    if (verbose) {
                        cout <<"  ("<< sval2 <<
                            " * " << data_alignment_factor;
                    }
                }
                cout << endl;
                break;
            case DW_CFA_val_offset_sf:  /* DWARF3 */
                /*  instp is always 1 byte back, so we need +1
                    when we use it. See the final increment
                    of this for loop. */
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                {
                    Dwarf_Signed sval2 =
                        local_dwarf_decode_s_leb128(instp + 1,
                            &uleblen);

                    instp += uleblen;
                    len -= uleblen;
                    off += uleblen;
                    cout << "\t" << IToDec(loff,2);
                    cout << SpaceSurround(cfa_name);
                    printreg((Dwarf_Signed) uval, config_data);
                    cout << " " << ((sval2) * data_alignment_factor);
                    if (verbose) {
                        cout << "  (" << sval2<< " * " <<
                            data_alignment_factor << ")";
                    }
                }
                cout << endl;
                break;
            case DW_CFA_val_expression: /* DWARF3 */
                /*  instp is always 1 byte back, so we need +1
                    when we use it. See the final increment
                    of this for loop. */
                uval = local_dwarf_decode_u_leb128(instp + 1, &uleblen);
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                {
                    Dwarf_Unsigned block_len = 
                        local_dwarf_decode_u_leb128(instp + 1,
                            &uleblen);

                    instp += uleblen;
                    len -= uleblen;
                    off += uleblen;
                    cout << "\t" << IToDec(loff,2);
                    cout << SpaceSurround(cfa_name) << uval;
                    cout << " expr block len " << block_len << endl;
                    dump_block("\t\t", (char *) instp+1,
                        (Dwarf_Signed) block_len);
                    cout << endl;
                    if(verbose) {
                        string exprstring;
                        get_string_from_locs(dbg,
                            instp+1,block_len, addr_size,exprstring);
                        cout<< "\t\t" <<exprstring << endl;
                    }
                    instp += block_len;
                    len -= block_len;
                    off += block_len;
                }


                break;


#ifdef DW_CFA_GNU_window_save
            case DW_CFA_GNU_window_save:{
                /*  No information: this just tells unwinder to
                    the window registers from the previous
                    frame's window save area */
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name) << endl;
                }
                break;
#endif
#ifdef DW_CFA_GNU_negative_offset_extended
            case DW_CFA_GNU_negative_offset_extended:{
                cout << "\t" << IToDec(loff,2);
                cout << SpaceSurround(cfa_name) <<
                    endl;
                }
                break;
#endif
#ifdef  DW_CFA_GNU_args_size
                /*  Single uleb128 is the current arg area size in
                    bytes. no register exists yet to save this in */
            case DW_CFA_GNU_args_size:{
                Dwarf_Unsigned lreg;

                /*  instp is always 1 byte back, so we need +1
                    when we use it. See the final increment
                    of this for loop. */
                lreg = local_dwarf_decode_u_leb128(instp + 1,
                    &uleblen);
                cout << "\t" << IToDec(loff,2);
                cout << " " << cfa_name << " arg size: "  <<
                    lreg << endl;
                instp += uleblen;
                len -= uleblen;
                off += uleblen;
                }
                break;
#endif

            default:
                cout << "\t" << IToDec(loff,2);
                cout << " Unexpected op " <<
                    IToHex(bottom) << ":" << endl;
                len = 0;
                break;
            }
        }
        instp++;
        len--;
        off++;
    }
}

/*  Print our register names for the cases we have a name.
    Delegate to the configure code to actually do the print.
*/
void
printreg(Dwarf_Signed reg, struct dwconf_s *config_data)
{
    print_reg_from_config_data(reg, config_data);
}


/*  Actually does the printing of a rule in the table.
    This may print something or may print nothing!
*/

static void
print_one_frame_reg_col(Dwarf_Debug dbg,
    Dwarf_Unsigned rule_id,
    Dwarf_Small value_type,
    Dwarf_Unsigned reg_used,
    Dwarf_Half address_size,
    struct dwconf_s *config_data,
    Dwarf_Signed offset_relevant,
    Dwarf_Signed offset, 
    Dwarf_Ptr block_ptr)
{
    string type_title = "";
    int print_type_title = 1;
    if (check_frames_extended) {
        return;
    }

    if (config_data->cf_interface_number == 2)
        print_type_title = 0;

    switch (value_type) {
    case DW_EXPR_OFFSET:
        type_title = "off";
        goto preg2;
    case DW_EXPR_VAL_OFFSET:
        type_title = "valoff";

        preg2:
        if (reg_used == config_data->cf_initial_rule_value) {
            break;
        }
        if (print_type_title)
            cout << "<" << type_title << " ";
        printreg((Dwarf_Signed) rule_id, config_data);
        cout << "=";
        if (offset_relevant == 0) {
            printreg((Dwarf_Signed) reg_used, config_data);
            cout << " ";
        } else {
            cout << IToDec0N(offset,2);
            cout << "(";
            printreg((Dwarf_Signed) reg_used, config_data);
            cout << ") ";
        }
        if (print_type_title)
            cout <<  "> ";
        break;
    case DW_EXPR_EXPRESSION:
        type_title = "expr";
        goto pexp2;
    case DW_EXPR_VAL_EXPRESSION:
        type_title = "valexpr";

        pexp2:
        if (print_type_title)
            cout << "<" <<  type_title << " ";
        printreg((Dwarf_Signed) rule_id, config_data);
        cout << "=";
        cout << "expr-block-len=" << offset;
        if (print_type_title)
            cout << "> ";
        if (verbose) {
            if(block_ptr == 0) {
                // Wrong (old) register access used.
                // -R being just one way to request the 'reg3'
                // register interfaces.
                cout << "<Use -R to see content>";
            } else  {
                string pref("<");
                pref.append(type_title);
                pref.append("bytes:");
                dump_block(pref, reinterpret_cast<char *>(block_ptr), offset);
                cout << "> ";
                if(verbose) {
                    string exprstring;
                    get_string_from_locs(dbg,
                        block_ptr,offset, address_size,exprstring);
                    cout<< BracketSurround(string("expr:") +
                        exprstring);
                }
            }
        }
        break;
    default:
        cout <<"Internal error in libdwarf, value type " <<
            value_type << endl;
        exit(1);
    }
    return;

}

/*  get all the data in .debug_frame (or .eh_frame). 
    The '3' versions mean print using the dwarf3 new interfaces.
    The non-3 mean use the old interfaces.
    All combinations of requests are possible.  */
extern void
print_frames(Dwarf_Debug dbg, int print_debug_frame, int print_eh_frame,
    struct dwconf_s *config_data)
{
    Dwarf_Half address_size = 0;
    LowpcToNameMaptype map_lowpc_to_name;


    error_message_data.current_section_id = DEBUG_FRAME;

    // The address size here will not be right for all frames.
    // Only in DWARF4 is there a real address size known
    // in the frame data itself.  If any DIE
    // is known then a real address size can be gotten from
    // dwarf_get_die_address_size(). 
    int fres = dwarf_get_address_size(dbg, &address_size, &err);
    if (fres != DW_DLV_OK) {
        print_error(dbg, "dwarf_get_address_size", fres, err);
    }
    for (int framed = 0; framed < 2; ++framed) {
        Dwarf_Cie *cie_data = NULL;
        Dwarf_Signed cie_element_count = 0;
        Dwarf_Fde *fde_data = NULL;
        Dwarf_Signed fde_element_count = 0;
        Dwarf_Signed i;
        int frame_count = 0;
        int cie_count = 0;
        bool all_cus_seen(false);
        LowpcUsedSettype  lowpcSet;
        string framename;
        int silent_if_missing = 0;
        int is_eh = 0;

        if (framed == 0) {
            if (!print_debug_frame) {
                continue;
            }
            framename = ".debug_frame";
            /*  Big question here is how to print all the info?
                Can print the logical matrix, but that is huge,
                though could skip lines that don't change.
                Either that, or print the instruction statement program
                that describes the changes.  */
            fres = dwarf_get_fde_list(dbg, &cie_data, &cie_element_count,
                &fde_data, &fde_element_count, &err);
            if(check_harmless) {
                print_any_harmless_errors(dbg);
            }
        } else {
            if (!print_eh_frame) {
                continue;
            }
            is_eh = 1;
            /*  This is gnu g++ exceptions in a .eh_frame section. Which 
                is just like .debug_frame except that the empty, or
                'special' CIE_id is 0, not -1 (to distinguish fde from
                cie). And the augmentation is "eh". As of egcs-1.1.2
                anyway. A non-zero cie_id is in a fde and is the
                difference between the fde address and the beginning of
                the cie it belongs to. This makes sense as this is
                intended to be referenced at run time, and is part of
                the running image. For more on augmentation strings, see 
                libdwarf/dwarf_frame.c.  */

            /*  Big question here is how to print all the info?
                Can print the logical matrix, but that is huge,
                though could skip lines that don't change.
                Either that, or print the instruction statement program
                that describes the changes.  */
            silent_if_missing = 1;
            framename = ".eh_frame";
            fres = dwarf_get_fde_list_eh(dbg, &cie_data,
                &cie_element_count, &fde_data,
                &fde_element_count, &err);
            if(check_harmless) {
                print_any_harmless_errors(dbg);
            }
        }
        /* Do not print any frame info if in check mode */
        if (check_frames) {
            continue;
        }

        if (fres == DW_DLV_ERROR) {
            cout << endl;
            cout << framename;
            cout << endl;
            print_error(dbg, "dwarf_get_fde_list", fres, err);
        } else if (fres == DW_DLV_NO_ENTRY) {
            if (!silent_if_missing) {
                cout << endl;
                cout << framename;
                cout << endl;
            }
            /* no frame information */
        } else {                /* DW_DLV_OK */

            if( !check_frames_extended) {
                cout << endl;
                cout << framename;
                cout << endl;
                cout << endl;
                cout << "fde:";
                cout << endl;
            }

            for (i = 0; i < fde_element_count; i++) {
                print_one_fde(dbg, fde_data[i],
                    i, cie_data, cie_element_count,
                    address_size, is_eh, config_data,
                    map_lowpc_to_name,
                    lowpcSet,
                    all_cus_seen);
                ++frame_count;
                if(frame_count >= break_after_n_units) {
                    break;
                }
            }
            /* Print the cie set. */
            if (verbose) {
                /* Do not print if in check mode */
                if (!check_frames_extended) {
                    cout << endl;
                    cout << "cie:";
                    cout << endl;
                }
                for (i = 0; i < cie_element_count; i++) {
                    print_one_cie(dbg, cie_data[i], i, address_size,
                        config_data);
                    ++cie_count;
                    if(cie_count >= break_after_n_units) {
                        break;
                    }
                }
            }
            dwarf_fde_cie_list_dealloc(dbg, cie_data, cie_element_count,
                fde_data, fde_element_count);
        }
    }
}

