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
// createirepfrombinary.cc
// Reads an object and inserts its dwarf data into 
// an object intended to hold all the dwarf data.

#include "config.h"
#include <unistd.h> 
#include <stdlib.h> // for exit
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <string.h> // For memset etc
#include <sys/stat.h> //open
#include <fcntl.h> //open
#include "elf.h"
#include "gelf.h"
#include "strtabdata.h"
#include "dwarf.h"
#include "libdwarf.h"
#include "irepresentation.h"
#include "createirepfrombinary.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::list;

// THis should instantiated only locally, not with 'new'.
// It should not bt copied.
class IRFormInterface {
public:
    IRFormInterface(Dwarf_Debug dbg,
        Dwarf_Attribute attr,IRCUdata &cudata,IRAttr & irattr):
        dbg_(dbg),attr_(attr),cudata_(cudata),irattr_(irattr) {};
    // Do not free anything we did not create here.
    ~IRFormInterface() {};
    Dwarf_Debug dbg_;
    Dwarf_Attribute attr_;
    IRCUdata &cudata_;
    IRAttr &irattr_;
private:
    
    // Do not instantiate.
    IRFormInterface();
    IRFormInterface& operator= (const IRFormInterface &ir);

};


IRForm *formFactory(Dwarf_Debug dbg,
    Dwarf_Attribute attr,IRCUdata &cudata,IRAttr & irattr)
{
    Dwarf_Error err = 0;
    IRFormInterface interface(dbg,attr,cudata,irattr);
    enum Dwarf_Form_Class cl = irattr.getFormClass();
    switch(cl) {
    case DW_FORM_CLASS_UNKNOWN:   
        break;
    case DW_FORM_CLASS_ADDRESS:
        {
            IRFormAddress *f = new IRFormAddress(&interface);
            // FIXME: do we need to do anything about the symbol here?
            return f;
        }
        break;
    case DW_FORM_CLASS_BLOCK:     
        {
            IRFormBlock *f = new IRFormBlock(&interface);
            //FIXME
            return f;
        }
        break;
    case DW_FORM_CLASS_CONSTANT:
        {
            IRFormConstant *f = new IRFormConstant(&interface);
            //FIXME
            return f;
        }
        break;
    case DW_FORM_CLASS_EXPRLOC:   
        {
            IRFormExprloc *f = new IRFormExprloc(&interface);
            //FIXME
            return f;
        }
        break;
    case DW_FORM_CLASS_FLAG:
        {
            IRFormFlag *f = new IRFormFlag(&interface);
            //FIXME
            return f;
        }
        break;
    case DW_FORM_CLASS_LINEPTR:   
        {
            IRFormLinePtr *f = new IRFormLinePtr(&interface);
            //FIXME
            return f;
        }
        break;
    case DW_FORM_CLASS_LOCLISTPTR:
        {
            IRFormLoclistPtr *f = new IRFormLoclistPtr(&interface);
            //FIXME
            return f;
        }
        break;
    case DW_FORM_CLASS_MACPTR:    
        {
            IRFormMacPtr *f = new IRFormMacPtr(&interface);
            //FIXME
            return f;
        }
        break;
    case DW_FORM_CLASS_RANGELISTPTR:
        {
            IRFormRangelistPtr *f = new IRFormRangelistPtr(&interface);
            //FIXME
            return f;
        }
        break;
    case DW_FORM_CLASS_REFERENCE: 
        {
            IRFormReference *f  = new IRFormReference(&interface);
            //FIXME
            return f;
        }
        break;
    case DW_FORM_CLASS_STRING:
        {
            IRFormString *f = new IRFormString(&interface);
            return f;
        }
        break;
    case DW_FORM_CLASS_FRAMEPTR: /* SGI/IRIX extension. */
        {
            IRFormFramePtr *f = new IRFormFramePtr(&interface);
            //FIXME
            return f;
        }
        break;
    default:
        break;
    }
    return new IRFormUnknown();
}

IRFormAddress::IRFormAddress(IRFormInterface * interface)
{
    Dwarf_Addr val = 0;
    Dwarf_Error error = 0;
    int res = dwarf_formaddr(interface->attr_,&val, &error);
    if(res != DW_DLV_OK) {
        cerr << "Unable to read flag value. Impossible error.\n" << endl;
        exit(1);
    }
    // FIXME do we need to do anything about the symbol here?
    address_ = val;
}
IRFormBlock::IRFormBlock(IRFormInterface * interface)
{
    Dwarf_Block *blockptr = 0;
    Dwarf_Error error = 0;
    int res = dwarf_formblock(interface->attr_,&blockptr, &error);
    if(res != DW_DLV_OK) {
        cerr << "Unable to read flag value. Impossible error.\n" << endl;
        exit(1);
    }
    insertBlock(blockptr);
    dwarf_dealloc(interface->dbg_,blockptr,DW_DLA_BLOCK);
}
IRFormConstant::IRFormConstant(IRFormInterface * interface)
{
    enum Signedness oursign = SIGN_NOT_SET;
    Dwarf_Unsigned uval = 0;
    Dwarf_Signed sval = 0;
    Dwarf_Error error = 0;
    int ress = dwarf_formsdata(interface->attr_, &sval,&error);
    int resu = dwarf_formudata(interface->attr_, &uval,&error);
    if(resu == DW_DLV_OK ) {
        if(ress == DW_DLV_OK) {
            oursign = SIGN_UNKNOWN;
        } else {
            oursign = UNSIGNED;
            sval = static_cast<Dwarf_Signed>(uval);
        }
    } else {
        if(ress == DW_DLV_OK) {
            oursign = SIGNED;
            uval = static_cast<Dwarf_Unsigned>(sval);
        } else {
            cerr << "Unable to read constant value. Impossible error.\n" 
                << endl;
            exit(1);
        }
    }
    setValues(sval, uval, oursign);
}
IRFormExprloc::IRFormExprloc(IRFormInterface * interface)
{
    Dwarf_Unsigned len = 0;
    Dwarf_Ptr data = 0;
    Dwarf_Error error = 0;
    int res = dwarf_formexprloc(interface->attr_,&len, &data, &error);
    if(res != DW_DLV_OK) {
        cerr << "Unable to read flag value. Impossible error.\n" << endl;
        exit(1);
    }
    insertBlock(len,data);
}
IRFormFlag::IRFormFlag(IRFormInterface * interface)
{
    Dwarf_Bool flagval = 0;
    Dwarf_Error error = 0;
    int res = dwarf_formflag(interface->attr_,&flagval, &error);
    if(res != DW_DLV_OK) {
        cerr << "Unable to read flag value. Impossible error.\n" << endl;
        exit(1);
    }
}

// We are simply assuming (here) that the value is a global offset
// to some section or other.
// Calling code ensures that is true.
// 
static Dwarf_Unsigned 
get_section_offset(IRFormInterface * interface)
{
    Dwarf_Unsigned uval = 0;
    Dwarf_Signed sval = 0;
    Dwarf_Error error = 0;
    Dwarf_Off sectionoffset = 0;
    int resu = 0;
    // The following allows more sorts of value than
    // we really want to allow here, but that is
    // harmless, we believe.
    int resgf = dwarf_global_formref(interface->attr_,
        &sectionoffset, &error);
    if(resgf == DW_DLV_OK ) {
        return sectionoffset;
    }
    resu = dwarf_formudata(interface->attr_, &uval,&error);
    if(resu != DW_DLV_OK ) {
        int ress = dwarf_formsdata(interface->attr_, &sval,&error);
        if(ress == DW_DLV_OK) {
            uval = static_cast<Dwarf_Unsigned>(sval);
        } else {
            cerr << "Unable to read constant offset value. Impossible error.\n" 
                << endl;
            exit(1);
        }
    }
    return uval;
}
IRFormLoclistPtr::IRFormLoclistPtr(IRFormInterface * interface)
{
    Dwarf_Unsigned uval = get_section_offset(interface);
    setOffset(uval);
}
IRFormLinePtr::IRFormLinePtr(IRFormInterface * interface)
{
    Dwarf_Unsigned uval = get_section_offset(interface);
    setOffset(uval);
}
IRFormMacPtr::IRFormMacPtr(IRFormInterface * interface)
{
    Dwarf_Unsigned uval = get_section_offset(interface);
    setOffset(uval);
}
IRFormRangelistPtr::IRFormRangelistPtr(IRFormInterface * interface)
{
    Dwarf_Unsigned uval = get_section_offset(interface);
    setOffset(uval);
}
IRFormFramePtr::IRFormFramePtr(IRFormInterface * interface)
{
    Dwarf_Unsigned uval = get_section_offset(interface);
    setOffset(uval);
}

// This is a .debug_info to .debug_info (or .debug_types to .debug_types)
// reference.  
IRFormReference::IRFormReference(IRFormInterface * interface)
{
    Dwarf_Off val = 0;
    Dwarf_Error error = 0;
    Dwarf_Half form = interface->irattr_.getFinalForm();
    if(form == DW_FORM_ref_sig8) {
        Dwarf_Sig8 val8;
        int res = dwarf_formsig8(interface->attr_,&val8, &error);
        if(res != DW_DLV_OK) {
            cerr << "Unable to read sig8 reference. Impossible error.\n" << endl;
            exit(1);
        }
        setSignature(&val8);
        return;
    }
    if(form == DW_FORM_ref_addr) {
        int res = dwarf_global_formref(interface->attr_,&val, &error);
        if(res != DW_DLV_OK) {
            cerr << "Unable to read reference. Impossible error.\n" << endl;
            exit(1);
        }
        setOffset(val);
        return;
    }
    // Otherwise it is (if a correct FORM for a .debug_info reference)
    // a local CU offset, and we record it as such..
    int res = dwarf_formref(interface->attr_,&val, &error);
    if(res != DW_DLV_OK) {
        cerr << "Unable to read reference. Impossible error.\n" << endl;
        exit(1);
    }
    setCUOffset(val);
}

// Global static data used to initialized a sig8 reliably.
static Dwarf_Sig8  zero_sig8;
// Initialization helper function.
void
IRFormReference::initSig8() 
{
    typeSig8_ = zero_sig8;
}

IRFormString::IRFormString(IRFormInterface * interface)
{
    char *str = 0;
    Dwarf_Error error = 0;
    int res = dwarf_formstring(interface->attr_,&str, &error);
    if(res != DW_DLV_OK) {
        cerr << "Unable to read string. Impossible error.\n" << endl;
        exit(1);
    }
    setString(str);
}



