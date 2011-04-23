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
// irepmacro.h
//

class IRMacroRecord {
public:
    IRMacroRecord() {};
    ~IRMacroRecord() {};
    IRMacroRecord(Dwarf_Off cuDieOffset,Dwarf_Off offset,Dwarf_Small type,
        Dwarf_Signed lineno, Dwarf_Signed lineindex,
        const std::string &macro):cuDieOffset_(cuDieOffset),
        offset_(offset),
        type_(type),lineno_(lineno),lineindex_(lineindex),
        macro_(macro) {};
private:
    Dwarf_Off cuDieOffset_; 
    Dwarf_Off offset_; 
    Dwarf_Small type_;
    Dwarf_Signed lineno_;
    Dwarf_Signed lineindex_;
    std::string macro_;
};
class IRMacro {
public:
    IRMacro() {};
    ~IRMacro() {};
    std::vector<IRMacroRecord> &getMacroVec() { return macrorec_; };
private:
    std::vector<IRMacroRecord> macrorec_;
};
