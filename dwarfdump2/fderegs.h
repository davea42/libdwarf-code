/*
  Copyright (C) 2009-2011 David Anderson.  All Rights Reserved.
 
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
 
*/
 

// Abstracts out calling dwarf to get fde register values
// at a pc value.

// Assumes libdwarf.h etc already included.
//
// Should speed up printing  a frame by nearly N times
// where N is the number of register rules (columns).

class FdeRegs {
public:
    FdeRegs(Dwarf_Fde fde,struct dwconf_s *config_data):
        fde_(fde),interfaceNumber_(config_data->cf_interface_number),
        confData_(*config_data),pcAddr_(0),rowPc_(0) {
        // regTable_ is a C struct from libdwarf.h
        memset(&regTable_,0,sizeof(regTable_));
        unsigned count = confData_.cf_table_entry_count;
        regTable_.rt3_reg_table_size = count;
        regTable_.rt3_rules = new Dwarf_Regtable_Entry3_s[count];
        tableByteCount_ = sizeof(Dwarf_Regtable_Entry3_s) *count;
        zeroRegTab();
    };
    ~FdeRegs() { delete [] regTable_.rt3_rules;};

    int getInterfaceNumber() { return interfaceNumber_; };

    void setPc(Dwarf_Addr pcval) { pcAddr_ = pcval;};
    int preliminaryRead(Dwarf_Error *err) {
        zeroRegTab();
        if(interfaceNumber_ == 2) {
            // Interface 2 is deprecated. Ok to use for testing.
            static Dwarf_Regtable t;
            Dwarf_Regtable regtab2;
            regtab2 = t;
            int res = dwarf_get_fde_info_for_all_regs(fde_,
                pcAddr_, &regtab2  ,&rowPc_,err);
            if(res == DW_DLV_OK) {
                // Transform to form 3.
                for(unsigned i = 0; i < confData_.cf_table_entry_count; ++i) {
                    Dwarf_Regtable_Entry3 *out = 
                        regTable_.rt3_rules+i;
                    Dwarf_Regtable_Entry *in =
                        &regtab2.rules[i];
                    out->dw_offset_relevant = in->dw_offset_relevant;
                    out->dw_value_type = in->dw_value_type;
                    out->dw_regnum = in->dw_regnum;
                    out->dw_offset_or_block_len = in->dw_offset;
                    out->dw_block_ptr = 0;
                }
            }
            return res;
        } else if(interfaceNumber_ == 3) {
            //int rulecount = confData_.cf_table_entry_count;
            int res = dwarf_get_fde_info_for_all_regs3(fde_,
                pcAddr_, &regTable_,&rowPc_,err);
            return res;
        } else {
            return DW_DLV_ERROR;
        }
    };

    // Interface number 3 only. 
    int getCfaRegdata( Dwarf_Regtable_Entry3 * entry_out, 
        Dwarf_Addr * rowpc_out, 
        Dwarf_Error *err) {
        if (interfaceNumber_ != 3) {
            // Really a programmer botch here.
            return DW_DLV_NO_ENTRY;
        };
        *rowpc_out = rowPc_;
        *entry_out = regTable_.rt3_cfa_rule;
        return DW_DLV_OK;
    };
    // Interfaces 2 and 3.
    int getRegdata(unsigned table_col,
        Dwarf_Regtable_Entry3 * entry_out, Dwarf_Addr * rowpc_out, 
        Dwarf_Error *err) {
        if ( table_col >= confData_.cf_table_entry_count) {
            return DW_DLV_ERROR;
        }  
        *rowpc_out = rowPc_;
        *entry_out = regTable_.rt3_rules[table_col];
        return DW_DLV_OK;
    };

private:
    Dwarf_Fde fde_;
    int interfaceNumber_;
    struct dwconf_s confData_;
    Dwarf_Addr pcAddr_;
    Dwarf_Addr rowPc_;
    Dwarf_Regtable3_s regTable_;
    unsigned tableByteCount_;

    void zeroRegTab() {
        memset( regTable_.rt3_rules,0, tableByteCount_);
        memset( &regTable_.rt3_cfa_rule,0, sizeof(Dwarf_Regtable_Entry3_s));
        // Do not set rt3_reg_table_size  here. Set already.
    }

    // Unimplemented.
    FdeRegs();
};
