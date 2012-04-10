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

// irepattrtodbg.cc

#include "config.h"
#include <unistd.h> 
#include <stdlib.h> // for exit
#include <iostream>
#include <sstream> // For BldName
#include <iomanip> // iomanp for setw etc
#include <string>
#include <list>
#include <vector>
#include <string.h> // For memset etc
#include <sys/stat.h> //open
#include <fcntl.h> //open
#include "general.h"
#include "elf.h"
#include "gelf.h"
#include "strtabdata.h"
#include "dwarf.h"
#include "libdwarf.h"
#include "irepresentation.h"
#include "ireptodbg.h"
#include "irepattrtodbg.h"

#ifdef HAVE_INTPTR_T  
#include <stdint.h>
typedef intptr_t myintfromp; // intptr_t is from C99.
#else
// We want an integer that is big enough for a pointer so the
// pointer return value from the libdwarf producer can be
// tested for -1.  Ugly overloading of integer and pointer in libdwarf.
// We just hope it will compile for you.
typedef long myintfromp;
#endif

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::list;

static Dwarf_Error error;
static unsigned fakeaddrnum;

// We are not going to 'validate' the FORM for this Attribute.
// or this Die.  We just assume that what we are handed is
// what we are to produce.  We do test the attribute
// at times, partly to ensure we use as many of the dwarf_add_AT* 
// functions as possible.

// Correctness/appropriateness must be evaluated elsewhere.

void
AddAttrToDie(Dwarf_P_Debug dbg,
    IRepresentation & Irep,
    Dwarf_P_Die outdie,IRDie & irdie,IRAttr &irattr)
{
    int attrnum = irattr.getAttrNum();
    enum Dwarf_Form_Class formclass = irattr.getFormClass();
    // IRForm is an abstract base class.
    IRForm *form = irattr.getFormData();

    switch(formclass) {
    case DW_FORM_CLASS_UNKNOWN:   
        cerr << "ERROR Impossible DW_FORM_CLASS_UNKNOWN, attrnum "
            <<attrnum << cerr;
        break;
    case DW_FORM_CLASS_ADDRESS:
        {
        IRFormAddress *f = dynamic_cast<IRFormAddress *>(form);
        if (!f) {
            cerr << "ERROR Impossible DW_FORM_CLASS_ADDRESS cast fails, attrnum "
                <<attrnum << cerr;
            break;
        }
        // FIXME: do better creating a symbol:  try to match original 
        // or specified input.
        Dwarf_Addr addr = f->getAddress();
       
        string symname = BldName("addrsym",fakeaddrnum++);
        Dwarf_Addr pcval = addr;

        ElfSymbols& es = Irep.getElfSymbols();
        ElfSymIndex esi = es.addElfSymbol(pcval,symname);
        Dwarf_Unsigned sym_index =  esi.getSymIndex();

        // FIXME: we should  allow for DW_FORM_indirect here.
        // Relocation later will fix value.
        Dwarf_P_Attribute a = dwarf_add_AT_targ_address(dbg,
            outdie,attrnum,0,sym_index,&error);
        if( reinterpret_cast<myintfromp>(a) == DW_DLV_BADADDR) {
            cerr << "ERROR dwarf_add_AT_targ_address fails, attrnum "
                <<attrnum << cerr;
           
        }
        }
        break;
    case DW_FORM_CLASS_BLOCK:   
        {
        //FIXME
        }
        break;
    case DW_FORM_CLASS_CONSTANT:
        {
        //FIXME
        }
        break;
    case DW_FORM_CLASS_EXPRLOC:   
        {
        //FIXME
        }
        break;
    case DW_FORM_CLASS_FLAG:
        {
        IRFormFlag *f = dynamic_cast<IRFormFlag *>(form);
        if (!f) {
            cerr << "ERROR Impossible DW_FORM_CLASS_FLAG cast fails, attrnum "
                <<attrnum << cerr;
            break;
        }
        // FIXME: handle indirect form (libdwarf needs feature).
        // FIXME: handle implicit flag (libdwarf needs feature).
        // FIXME: rel type ok?
        Dwarf_P_Attribute a = 
            dwarf_add_AT_flag(dbg,outdie,attrnum,f->getFlagVal(),&error);
        if( reinterpret_cast<myintfromp>(a) == DW_DLV_BADADDR) {
            cerr << "ERROR dwarf_add_AT_flag fails, attrnum "
                <<attrnum << cerr;
        }
        }
        break;
    case DW_FORM_CLASS_LINEPTR:   
        {
        //FIXME
        }
        break;
    case DW_FORM_CLASS_LOCLISTPTR:
        {
        //FIXME
        }
        break;
    case DW_FORM_CLASS_MACPTR:    
        {
        //FIXME
        }
        break;
    case DW_FORM_CLASS_RANGELISTPTR:
        {
        //FIXME
        }
        break;
    case DW_FORM_CLASS_REFERENCE: 
        break;
    case DW_FORM_CLASS_STRING:
        {
        IRFormString *f = dynamic_cast<IRFormString *>(form);
        if (!f) {
            cerr << "ERROR Impossible DW_FORM_CLASS_STRING cast fails, attrnum "
                <<attrnum << cerr;
            break;
        }
        Dwarf_P_Attribute a = 0;
        // We know libdwarf does not change the string. Historical mistake
        // not making it a const char * argument.
        // Ugly cast.
        // FIXME: handle indirect form (libdwarf needs feature).
        // FIXME: rel type ok?
        char *mystr = const_cast<char *>(f->getString().c_str());
        switch(attrnum) {
        case DW_AT_name:
            a = dwarf_add_AT_name(outdie,mystr,&error);
            break;
        case DW_AT_producer:
            a = dwarf_add_AT_producer(outdie,mystr,&error);
            break;
        case DW_AT_comp_dir:
            a = dwarf_add_AT_comp_dir(outdie,mystr,&error);
            break;
        default:
            a = dwarf_add_AT_string(dbg,outdie,attrnum,mystr,
                &error);
            break;
        }
        if( reinterpret_cast<myintfromp>(a) == DW_DLV_BADADDR) {
            cerr << "ERROR dwarf_add_AT_string fails, attrnum "
                <<attrnum << cerr;
        }
        }
        break;
    case DW_FORM_CLASS_FRAMEPTR: // SGI/MIPS/IRIX only.
        {
        //FIXME
        }
        break;
    default:
        cerr << "ERROR Impossible DW_FORM_CLASS  "<<
            static_cast<int>(formclass)
            <<attrnum << cerr;
        //FIXME
    }
    return;
}
  
