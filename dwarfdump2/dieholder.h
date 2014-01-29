/*
   Copyright (C) 2009-2012 David Anderson  All Rights Reserved.
   Portions Copyright 2012 SN Systems Ltd. All rights reserved.

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

#ifndef  DIEHOLDER_H
#define DIEHOLDER_H
// Reference counting eliminates confusion and bugs
// when deciding when to do dealloc.
class DieHolder {
public:
    DieHolder():dbg_(0),die_(0),
        sibling_global_offset_(0),
        refcount_(new int(1)),die_printed_(false) { };
    DieHolder(Dwarf_Debug dbg, Dwarf_Die die):
        dbg_(dbg),die_(die),
        sibling_global_offset_(0),
        refcount_(new int(1)),die_printed_(false) { };
    ~DieHolder() {
        (*refcount_)--;
        if ((*refcount_) == 0) {
            delete refcount_;
            if (die_) dwarf_dealloc(dbg_,die_,DW_DLA_DIE);
        }
    };
    // We don't copy a DieHolder sibling_global_offset_
    // as that is local to the dieVec instances.
    DieHolder(const DieHolder & d):dbg_(d.dbg_),die_(d.die_),
        sibling_global_offset_(0),
        refcount_(d.refcount_),die_printed_(d.die_printed_) {
        (*refcount_)++;
    };
    DieHolder & operator=(const DieHolder & d) {
        if (this != &d) {
            (*d.refcount_)++;
            (*refcount_)--;
            if ((*refcount_) == 0) {
                delete refcount_;
                if (die_) dwarf_dealloc(dbg_,die_,DW_DLA_DIE);
            }
            refcount_ = d.refcount_;
            die_ = d.die_;
            dbg_ = d.dbg_;
            sibling_global_offset_ = d.sibling_global_offset_;
            die_printed_ = d.die_printed_;
        }
        return *this;
    };
    Dwarf_Die die() { return die_; };
    Dwarf_Debug dbg() { return dbg_; };
    void setSiblingGlobalOffset(Dwarf_Off o) {sibling_global_offset_ = o;}
    Dwarf_Off getSiblingGlobalOffset() {return sibling_global_offset_; }
    bool die_printed() { return die_printed_; };
    void mark_die_printed() { die_printed_ = true; };
private:
    Dwarf_Debug dbg_;
    Dwarf_Die   die_;
    // sibling_global_offset_ is zero until we encounter a sibling attribute.
    // And it says zero if a die has no DW_AT_sibling.
    // It is used for error-checking sibling attributes.
    Dwarf_Off   sibling_global_offset_;
    int *refcount_;
    bool die_printed_;
};


class DieVec {
public:
    DieVec() {};
    ~DieVec() {};
    // Simpler to handle negatives here than in the
    // print_die.cc code.
    void setSiblingGlobalOffset(int indx,Dwarf_Off off) {
        if (indx >= 0 && indx < dieVec_.size()) {
            dieVec_[indx].setSiblingGlobalOffset(off);
        }
    };
    // Zero is always a safe return.
    // Simpler to handle negatives here than in the
    // print_die.cc code.
    Dwarf_Off getSiblingGlobalOffset(int indx) {
        if (indx >= 0 && indx < dieVec_.size()) {
            return dieVec_[indx].getSiblingGlobalOffset();
        }
        return 0;
    };
    bool getDie(int indx,Dwarf_Die &dieout) {
        if (indx >= 0 && indx < dieVec_.size()) {
            dieout  =dieVec_[indx].die();
            return true;
        }
        // Should never happen!
        return false;
    };
    bool getDieHolder(int indx,DieHolder &dh) {
        if (indx >= 0 && indx < dieVec_.size()) {
            dh= dieVec_[indx];
            return true;
        }
        // Should never happen!
        return false;
    };
    int Size() {
        return dieVec_.size();
    }
    void PushBack(DieHolder &d) {
        dieVec_.push_back(d);
    }
    void PopBack() {
        dieVec_.pop_back();
    }
private:
    std::vector<DieHolder> dieVec_;
};

#endif  // DIEHOLDER_H
