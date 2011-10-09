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

#ifndef CHECKUTIL_H
#define CHECKUTIL_H

// A list of section address ranges with identifying
// information.  Provides the ability to check whether
// some line data that does not match .text does match
// some linkonce section data.
struct LinkOnceEntry { 
public:
    LinkOnceEntry(Dwarf_Unsigned section, Dwarf_Unsigned lowpc,
        Dwarf_Unsigned high_pc, const std::string name):
        lo_section_index_(section),lo_section_low_pc_(lowpc),
        lo_section_high_pc_(high_pc),lo_section_name_(name) {};
    ~LinkOnceEntry () {};
    bool inThisLinkOnceRange(Dwarf_Unsigned pc) {
        if(pc >= lo_section_low_pc_ && pc <= lo_section_high_pc_) {
            return true;
        }
        return false;
    }
    bool inThisLinkOnceRange(const std::string &sname,Dwarf_Unsigned lopc,
        Dwarf_Unsigned hipc) {
            if(sname == lo_section_name_ ) {
                if((lopc >= lo_section_low_pc_ && lopc <=
                    lo_section_high_pc_) &&
                    (hipc >= lo_section_low_pc_ && hipc <=
                    lo_section_high_pc_))  {
                    return true;
                }
            }
            return false;
        };
    void printLinkOnceEntry(unsigned index);
private:
    Dwarf_Unsigned lo_section_index_;
    Dwarf_Unsigned lo_section_low_pc_;
    Dwarf_Unsigned lo_section_high_pc_;
    // There are normally relatively few sections (not thousands
    // or millions).
    std::string lo_section_name_;
};

// In C dwarfdump see pLinkonceInfo.
class LinkOnceData {
public:
    LinkOnceData() {};
    ~LinkOnceData() {};
    void AddLinkOnceEntry(const LinkOnceEntry &e) {
        link_once_data_.push_back(e);
    };
    bool FindLinkOnceEntry(Dwarf_Unsigned pc);
    bool FindLinkOnceEntry(const std::string &secname,Dwarf_Unsigned lopc,
        Dwarf_Unsigned hipc);
    void PrintLinkOnceData();
private:
    std::list<LinkOnceEntry> link_once_data_;
};

extern LinkOnceData *pLinkOnceData;

struct AddressRangeEntry {
public:
    AddressRangeEntry(Dwarf_Unsigned lowpc,
        Dwarf_Unsigned high_pc):
        range_low_pc_(lowpc),
        range_high_pc_(high_pc){};
    ~AddressRangeEntry () {};
    bool inThisRange(Dwarf_Unsigned pc) {
        if(pc < range_low_pc_ || pc > range_high_pc_) {
            return false;
        }
        return true;
    };
    void printAddressRangeEntry(unsigned index);
private:
    Dwarf_Unsigned range_low_pc_;
    Dwarf_Unsigned range_high_pc_;
};  


// In C dwarfdump see pRangesInfo.
// These address ranges are within the text section, 
// and though rather like LinkOnceEntry data, we can
// rely on an overall valid range (before we check
// for the specific range) as a qualifier.  So
// data that must fail the search is noted as such quickly.
class AddressRangesData {
public:
    AddressRangesData():low_pc_(0xffffffffffffffff),high_pc_(0) {};
    ~AddressRangesData() {};
    void AddAddressRange(Dwarf_Unsigned low_pc, Dwarf_Unsigned high_pc);
    void SetLimitsAddressRange(Dwarf_Unsigned low_pc, Dwarf_Unsigned high_pc);
    bool IsAddressInAddressRange(Dwarf_Unsigned pc);
    void PrintRangesData();
    void ResetRangesList();
private:
    Dwarf_Unsigned low_pc_;
    Dwarf_Unsigned high_pc_;
    std::list<AddressRangeEntry> address_range_data_; 
};

extern AddressRangesData *pAddressRangesData;

// In C dwarfdump see pVisitedInfo.
// VisitedOffsetData is  used to track offsets so
// recursion and invalid references can be noted.
class VisitedOffsetData {
public:
    typedef std::set<Dwarf_Unsigned,std::less<Dwarf_Unsigned> > VODtype;
    VisitedOffsetData () { offset_ = new VODtype; };
    ~VisitedOffsetData () { delete offset_;};
    void reset() {
        delete offset_;
        offset_ = new VODtype;
    }
    void AddVisitedOffset(Dwarf_Unsigned off) {
        offset_->insert(off);
    };
    void DeleteVisitedOffset(Dwarf_Unsigned off) {
        offset_->erase(off);
    };
    bool IsKnownOffset(Dwarf_Unsigned off) {
        VODtype::size_type v = offset_->count(off);
        if( v) {
            return true;
        }
        return false;
    };
private:
    VODtype *offset_;
};

extern VisitedOffsetData *pVisitedOffsetData;
#endif /* CHECKUTIL_H */
