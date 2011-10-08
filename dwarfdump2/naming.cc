
/* 
   Copyright (C) 2000,2002,2004,2005 Silicon Graphics, Inc. All Rights Reserved.
   Portions Copyright (C) 2007-2011 David Anderson. All Rights Reserved.
           
  
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

/*  The address of the Free Software Foundation is
    Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
    Boston, MA 02110-1301, USA.
    SGI has moved from the Crittenden Lane address.
*/

 
#include "globals.h"
#include "dwarf.h"
#include "libdwarf.h"

using std::string;
using std::cerr;
using std::endl;

static const char *
skipunder(const char *v)
{
    const char *cp = v;
    int undercount = 0;
    for(  ; *cp ; ++cp) {
        if( *cp == '_') {
            ++undercount;
            if(undercount == 2) {
                return cp+1;
            }
        }
    }
    return "";
};

static string
ellipname(int res, unsigned int val_in, const char *v,const char *ty,bool printonerr)
{
#ifndef TRIVIAL_NAMING
    if (check_dwarf_constants && checking_this_compiler()) {
        DWARF_CHECK_COUNT(dwarf_constants_result,1);
    }
#endif
    if(res != DW_DLV_OK) {
        if(printonerr) {
#ifndef TRIVIAL_NAMING
        if(printonerr && check_dwarf_constants && checking_this_compiler()) {
            if (check_verbose_mode) {
                std::cerr << ty << " of " << val_in << " (" <<
                    IToHex(val_in) << 
                    ") is unknown to dwarfdump. " <<
                    "Continuing. " << endl;
            }
            DWARF_ERROR_COUNT(dwarf_constants_result,1);
            DWARF_CHECK_ERROR_PRINT_CU();
        }
#else
        /* This is for the tree-generation, not dwarfdump itself. */
        if(printonerr) {
            std::cerr << ty << " of " << val_in << 
                " (" << IToHex(val_in,0) << 
                ") is unknown to dwarfdump. Continuing. " << std::endl;
        }
#endif
        }
        return "<Unknown " + string(ty) + " value " +
            IToHex(val_in,0) + ">";
    }
    if(ellipsis) {
        return skipunder(v);
    }
    return v;
};

std::string get_TAG_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_TAG_name(val_in,&v);
   return ellipname(res,val_in,v,"TAG",printonerr);
}
std::string get_children_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_children_name(val_in,&v);
   return ellipname(res,val_in,v,"children",printonerr);
}
std::string get_FORM_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_FORM_name(val_in,&v);
   return ellipname(res,val_in,v,"FORM",printonerr);
}
std::string get_AT_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_AT_name(val_in,&v);
   return ellipname(res,val_in,v,"AT",printonerr);
}
std::string get_OP_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_OP_name(val_in,&v);
   return ellipname(res,val_in,v,"OP",printonerr);
}
std::string get_ATE_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_ATE_name(val_in,&v);
   return ellipname(res,val_in,v,"ATE",printonerr);
}
std::string get_DS_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_DS_name(val_in,&v);
   return ellipname(res,val_in,v,"DS",printonerr);
}
std::string get_END_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_END_name(val_in,&v);
   return ellipname(res,val_in,v,"END",printonerr);
}
std::string get_ATCF_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_ATCF_name(val_in,&v);
   return ellipname(res,val_in,v,"ATCF",printonerr);
}
std::string get_ACCESS_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_ACCESS_name(val_in,&v);
   return ellipname(res,val_in,v,"ACCESS",printonerr);
}
std::string get_VIS_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_VIS_name(val_in,&v);
   return ellipname(res,val_in,v,"VIS",printonerr);
}
std::string get_VIRTUALITY_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_VIRTUALITY_name(val_in,&v);
   return ellipname(res,val_in,v,"VIRTUALITY",printonerr);
}
std::string get_LANG_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_LANG_name(val_in,&v);
   return ellipname(res,val_in,v,"LANG",printonerr);
}
std::string get_ID_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_ID_name(val_in,&v);
   return ellipname(res,val_in,v,"ID",printonerr);
}
std::string get_CC_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_CC_name(val_in,&v);
   return ellipname(res,val_in,v,"CC",printonerr);
}
std::string get_INL_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_INL_name(val_in,&v);
   return ellipname(res,val_in,v,"INL",printonerr);
}
std::string get_ORD_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_ORD_name(val_in,&v);
   return ellipname(res,val_in,v,"ORD",printonerr);
}
std::string get_DSC_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_DSC_name(val_in,&v);
   return ellipname(res,val_in,v,"DSC",printonerr);
}
std::string get_LNS_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_LNS_name(val_in,&v);
   return ellipname(res,val_in,v,"LNS",printonerr);
}
std::string get_LNE_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_LNE_name(val_in,&v);
   return ellipname(res,val_in,v,"LNE",printonerr);
}
std::string get_MACINFO_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_MACINFO_name(val_in,&v);
   return ellipname(res,val_in,v,"MACINFO",printonerr);
}
std::string get_CFA_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_CFA_name(val_in,&v);
   return ellipname(res,val_in,v,"CFA",printonerr);
}
std::string get_EH_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_EH_name(val_in,&v);
   return ellipname(res,val_in,v,"EH",printonerr);
}
std::string get_FRAME_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_FRAME_name(val_in,&v);
   return ellipname(res,val_in,v,"FRAME",printonerr);
}
std::string get_CHILDREN_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_CHILDREN_name(val_in,&v);
   return ellipname(res,val_in,v,"CHILDREN",printonerr);
}
std::string get_ADDR_name(unsigned int val_in,bool printonerr)
{
   const char *v = 0;
   int res = dwarf_get_ADDR_name(val_in,&v);
   return ellipname(res,val_in,v,"ADDR",printonerr);
}


