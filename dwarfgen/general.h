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
// general.h
// The following needed in code using this.
//#include <sstream> 
//#include <iomanip> // iomanip for setw etc

#include <sstream>
#include <iomanip>

template <typename T >
std::string IToHex(T v,unsigned l=0)
{
    if(v == 0) {
        // For a zero value, ostringstream does not insert 0x.
        // So we do zeroes here.
        std::string out = "0x0";
        if(l > 3)  {
            out.append(l-3,'0');
        }
        return out;
    }
    std::ostringstream s;
    s.setf(std::ios::hex,std::ios::basefield);
    s.setf(std::ios::showbase);
    if (l > 0) {
        s << std::setw(l);
    }
    s << v ;
    return s.str();
};

template <typename T>
std::string  BldName(const std::string & prefix, T v)
{
    std::ostringstream s;
    s << prefix;
    s << v;
    return s.str();
}

