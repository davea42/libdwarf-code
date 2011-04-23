#ifndef IREPATTRTODBG_H
#define IREPATTRTODBG_H
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

// irepattrtodbg.h

void AddAttrToDie(Dwarf_P_Debug dbg,
   IRepresentation & Irep,
   Dwarf_P_Die outdie,IRDie & irdie,IRAttr &irattr);

#endif /* IREPATTRTODBG_H */
