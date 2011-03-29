/* 
   Copyright (C) 2009-2011 David Anderson  All Rights Reserved.  
  
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

#ifndef  SRCFILESHOLDER_H
#define SRCFILESHOLDER_H
// Reference counting eliminates confusion and bugs
// when deciding when to do dealloc.
// It might be nice to store a vector of string here and
// dealloc the dwarf data on construction.
// But for now  we store the libdwarf strings.
class SrcfilesHolder {
public:
    SrcfilesHolder():dbg_(0),srcfiles_(0),cnt_(0),refcount_(new int(1)) { };
    SrcfilesHolder(Dwarf_Debug dbg, char **srcfiles, Dwarf_Signed cnt):
        dbg_(dbg),srcfiles_(srcfiles),cnt_(cnt),refcount_(new int(1)) { };
    ~SrcfilesHolder() {
        (*refcount_)--;
        if( (*refcount_) == 0) {
            delete refcount_; 
            if(srcfiles_) do_delete();
        }
    };
    SrcfilesHolder(const SrcfilesHolder & d):dbg_(d.dbg_),
        srcfiles_(d.srcfiles_),cnt_(d.cnt_),
        refcount_(d.refcount_) {
        (*refcount_)++;
    };
    SrcfilesHolder & operator=(const SrcfilesHolder & d) {
        if(this != &d) {
            (*d.refcount_)++;
            (*refcount_)--;
            if( (*refcount_) == 0) {
                delete refcount_; 
                if(srcfiles_) do_delete();
            }
            refcount_ = d.refcount_;
            srcfiles_ = d.srcfiles_;
            cnt_ = d.cnt_;
            dbg_ = d.dbg_;
        }
        return *this;
    };
    Dwarf_Signed count() { return cnt_; };
    char ** srcfiles() { return srcfiles_; };
    Dwarf_Debug dbg() { return dbg_; };
private:
    void do_delete() {
        for (Dwarf_Signed si = 0; si < cnt_; ++si) {
            dwarf_dealloc(dbg_, srcfiles_[si], DW_DLA_STRING);
        }
        dwarf_dealloc(dbg_, srcfiles_, DW_DLA_LIST);
    };
    Dwarf_Debug dbg_;
    char **srcfiles_;
    Dwarf_Signed  cnt_;
    int *refcount_;
};
#endif  // SRCFILESHOLDER_H
