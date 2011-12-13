/*
  Copyright (C) 2011 David Anderson.  

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
// irepline.h
//

class IRCULine {
public:
    IRCULine() {};
    ~IRCULine() {};
private:

    // Names taken from the DWARF4 std. document, sec 6.2.2.
    Dwarf_Unsigned address_;
    bool isaddrset_;
    Dwarf_Unsigned srcfileno_;
    Dwarf_Unsigned lineno_;
    Dwarf_Signed   linecol_; // aka lineoff
    std::string    linesrc_; // Name for the file, constructed by libdwarf.
    bool           is_stmt_;
    bool           basic_block_;
    bool           end_sequence_;
    bool           prologue_end_;
    bool           epilogue_begin_;
    int            isa_;
    int            discriminator_;
};
class IRCUSrcfile {
public:
    IRCUSrcfile() {};
    ~IRCUSrcfile() {};
private:
    std::string cusrcfile_;
};

class IRCULines {
public:
    IRLine() {};
    ~IRLine() {};
    std::vector<IRCULine> &get_cu_lines() { return culinedata_; };
    std::vector<IRCUSrcfile> &get_cu_srcfiles() { return cusrcfiledata_; };
private:
    std::vector<IRCUSrcfile> cusrcfiledata_;
    std::vector<IRCULine> culinedata_;
};
