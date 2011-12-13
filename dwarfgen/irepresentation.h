/*
  Copyright (C) 2010-2011 David Anderson.  

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 51
  Franklin Street - Fifth Floor, Boston MA 02110-1301, USA.

*/

// 
// irepresentation.h
// The internal (to dwarfgen) representation of debug information.
// All the various components (info, frame, etc)
// will be stored here in an internal-to-dwarfgen form.
//
//
#include "irepform.h"
#include "irepline.h"
#include "irepdie.h"
#include "irepmacro.h"
#include "irepframe.h"
#include "strtabdata.h"

// The elf symbols are used to tie relocations to values.
// We do relocations ourselves in dwarfgen so the data is not needed
// once the dwarf .debug_* sections created  in elf. 
// We don't write the symbols out as an elf section.
// The position in the vector of symbols is the 'elf symbol index'
// we create.
// Symbol 0 is 'no symbol'.  
// Symbol 1 is .text
class ElfSymbol {
public:
    ElfSymbol():symbolValue_(0),
        nameIndex_(0) {};
    ElfSymbol(Dwarf_Unsigned val, const std::string&name, strtabdata&stab):
        symbolValue_(val),name_(name) {
        nameIndex_ = stab.addString(name);
    };
    ~ElfSymbol() {};
    Dwarf_Unsigned getSymbolValue() const { return symbolValue_;}
private:
    Dwarf_Unsigned symbolValue_;
    std::string    name_;
    // The offset in the string table.
    unsigned   nameIndex_;
};

class ElfSectIndex {
public:
    ElfSectIndex():elfsect_(0) {};
    ~ElfSectIndex() {};
    ElfSectIndex(unsigned v):elfsect_(v) {};
    unsigned getSectIndex() const { return elfsect_; }
    void setSectIndex(unsigned v) { elfsect_ = v; }
private:
    unsigned elfsect_;
};


class ElfSymIndex {
public:
    ElfSymIndex():elfsym_(0) {};
    ~ElfSymIndex() {};
    ElfSymIndex(unsigned v):elfsym_(v) {};
    unsigned getSymIndex() const { return elfsym_; }
    void setSymIndex(unsigned v) { elfsym_ = v; }
private:
    unsigned elfsym_;
};

class ElfSymbols {
public:
    ElfSymbols() { 
        // The initial symbol is 'no symbol'.
        std::string emptyname("");
        elfSymbols_.push_back(ElfSymbol(0,emptyname,symstrtab_));

        // We arbitrarily make this symbol .text now, though
        // not needed yet.
        std::string textname(".text");
        elfSymbols_.push_back(ElfSymbol(0,textname,symstrtab_));
        baseTextAddressSymbol_.setSymIndex(elfSymbols_.size()-1);
        }
    ~ElfSymbols() {};
    ElfSymIndex getBaseTextSymbol() const {return baseTextAddressSymbol_;};
    ElfSymIndex addElfSymbol(Dwarf_Unsigned val, const std::string&name) {
        elfSymbols_.push_back(ElfSymbol(val,name,symstrtab_));
        ElfSymIndex indx(elfSymbols_.size()-1);
        return indx;
        
    };
    ElfSymbol &  getElfSymbol(ElfSymIndex symi) {
        size_t i = symi.getSymIndex();
        if (i >= elfSymbols_.size()) {
            std::cerr << "Error, sym index " << i << "  to big for symtab size " << elfSymbols_.size() << std::endl;
            exit(1);
        }
        return elfSymbols_[i];
    }
private:
    std::vector<ElfSymbol> elfSymbols_;
    strtabdata symstrtab_;
    ElfSymIndex  baseTextAddressSymbol_;
};


class IRepresentation {
public:
    IRepresentation() {};
    ~IRepresentation(){};
    IRFrame &framedata() { return framedata_; };
    IRMacro &macrodata() { return macrodata_; };
    IRDInfo &infodata() { return debuginfodata_; };
    ElfSymbols &getElfSymbols() { return elfSymbols_;};
    unsigned getBaseTextSymbol() {
        return elfSymbols_.getBaseTextSymbol().getSymIndex();};
private:
    // The Elf symbols data to use for relocations 
    ElfSymbols elfSymbols_;

    IRFrame  framedata_;
    IRMacro  macrodata_;

    // Line data is inside IRDInfo.
    IRDInfo  debuginfodata_;
};
