/* 
  Copyright (C) 2011 David Anderson. All Rights Reserved.

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

*/
/*

   These simple list-processing functions are in support
   of checking DWARF for compiler-errors of various sorts.


*/

#include "globals.h"
#include <assert.h>
using std::cout;
using std::cerr;
using std::endl;


bool 
AddressRangesData::IsAddressInAddressRange(Dwarf_Unsigned pc)
{
    if(pc < low_pc_) {
        return false;
    }
    if(pc > high_pc_) {
        return false;
    }
    for(std::list<AddressRangeEntry>::iterator it =
        address_range_data_.begin(); it != address_range_data_.end();
        ++it) {
        if (it->inThisRange(pc) ) {
            return true;
        }
    }
    return false;

}

void
AddressRangeEntry::printAddressRangeEntry(unsigned ct) 
{
    cout <<"[" << IToDec(ct,6) << "] Low = " <<
        IToHex(range_low_pc_, 10 ) <<
        ", High =  " <<
        IToHex(range_high_pc_, 10) << std::endl;
};

// Leave overall high/low as is, delete the details.
void 
AddressRangesData::ResetRangesList()
{
    address_range_data_.clear();
}

// We might want to sort these by low-address rather than printing
// in random order!
void 
AddressRangesData::PrintRangesData()
{
    unsigned ct  = 0;
    cout << "Begin Traversing, Low = "<<
        IToHex(low_pc_,10) <<
        "  High = " <<
        IToHex(high_pc_,10);
    for(std::list<AddressRangeEntry>::iterator it = 
        address_range_data_.begin(); it != address_range_data_.end();
        ++it,++ct) {

        it->printAddressRangeEntry(ct);
    }
}
void 
AddressRangesData::AddAddressRange(Dwarf_Unsigned low_pc, Dwarf_Unsigned high_pc) 
{
    // Presently we are not checking for duplicates, so some
    // can be present.
    address_range_data_.push_back(AddressRangeEntry(low_pc,high_pc));
};

void 
AddressRangesData::SetLimitsAddressRange(Dwarf_Unsigned low_pc, Dwarf_Unsigned high_pc)
{
    if(low_pc < high_pc) {
        low_pc_ = low_pc;
        high_pc_ = high_pc;
    }
};

void
LinkOnceEntry::printLinkOnceEntry(unsigned ct) 
{
    cout <<"[" << IToDec(ct,6) << "] Low = " <<
        IToHex(lo_section_low_pc_, 10 ) <<
        ", High =  " <<
        IToHex(lo_section_high_pc_, 10) << 
        ",  section index = " <<
        lo_section_index_ <<
        ",  section = " <<
        lo_section_name_ << std::endl;
};

void
LinkOnceData::PrintLinkOnceData()
{
    unsigned ct = 0;
    for(std::list<LinkOnceEntry>::iterator it = 
        link_once_data_.begin(); it != link_once_data_.end();
        ++it,++ct) {

        it->printLinkOnceEntry(ct);
    }
}

bool LinkOnceData::FindLinkOnceEntry(Dwarf_Unsigned pc)
{
    std::list<LinkOnceEntry>::iterator it;
    for( it = link_once_data_.begin(); it != link_once_data_.end();
        ++it) {
        if (it->inThisLinkOnceRange(pc) ) {
            return true;
        }
    }
    return false;
}
bool LinkOnceData::FindLinkOnceEntry(const std::string &secname,Dwarf_Unsigned lopc,Dwarf_Unsigned hipc)
{
    std::list<LinkOnceEntry>::iterator it;
    for( it = link_once_data_.begin(); it != link_once_data_.end();
        ++it) {
        if (it->inThisLinkOnceRange(secname,lopc,hipc) ) {
            return true;
        }
    }
    return false;
}

