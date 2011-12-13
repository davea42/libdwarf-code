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

// ireptodbg.cc

#include "config.h"
#include <unistd.h> 
#include <stdlib.h> // for exit
#include <iostream>
#include <string>
#include <list>
#include <map>
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
#include "ireptodbg.h"
#include "irepattrtodbg.h"
#include "general.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::map;
using std::list;

static Dwarf_Error error;

typedef std::map<std::string,unsigned> pathToUnsignedType;

static Dwarf_P_Die 
HandleOneDieAndChildren(Dwarf_P_Debug dbg,
    IRepresentation & Irep,
    IRDie &inDie, unsigned level)
{
    list<IRDie>& children = inDie.getChildren();
    // We create our target DIE first so we can link
    // children to it, but add no content yet.
    Dwarf_P_Die ourdie = dwarf_new_die(dbg,inDie.getTag(),NULL,NULL,
        NULL,NULL,&error);
    if (reinterpret_cast<int>(ourdie) == DW_DLV_BADADDR) {
        cerr << "Die creation failure.  "<< endl;
        exit(1);
    }
       
    Dwarf_P_Die lastch = 0;
    for ( list<IRDie>::iterator it = children.begin();
        it != children.end();
        it++) {
        IRDie & ch = *it;
        Dwarf_P_Die chp = HandleOneDieAndChildren(dbg,Irep,ch,level+1);
        Dwarf_P_Die res = 0;
        if(lastch) {
            // Link to right of earlier sibling.
            res = dwarf_die_link(chp,NULL,NULL,lastch,NULL,&error);
        } else {
            // Link as first child.
            res  = dwarf_die_link(chp,ourdie,NULL,NULL, NULL,&error);
        }
        // Bad cast here, FIXME
        if (reinterpret_cast<int>(res) == DW_DLV_BADADDR) {
            cerr << "Die link failure.  "<< endl;
            exit(1);
        }
        lastch = chp;
    }
    list<IRAttr>& attrs = inDie.getAttributes();
    // Now we add attributes (content), if any, to the 'ourdie'.
    for (list<IRAttr>::iterator it = attrs.begin();
        it != attrs.end();
        it++) {
        IRAttr & attr = *it;
        AddAttrToDie(dbg,Irep,ourdie,inDie,attr);
    }
    return ourdie; 
}

static void
HandleLineData(Dwarf_P_Debug dbg,IRepresentation & Irep, IRCUdata&cu)
{
    Dwarf_Error error = 0;
    // We refer to files by fileno, this builds an index.
    pathToUnsignedType pathmap;

    IRCULineData& ld = cu.getCULines();
    std::vector<IRCULine> & cu_lines = ld.get_cu_lines();
    //std::vector<IRCUSrcfile> &cu_srcfiles  = ld.get_cu_srcfiles();
    if(cu_lines.empty()) {
        // No lines data to emit, do nothing.
        return;
    }
    // To start with, we are doing a trivial generation here.
    // To be refined 'soon'.  FIXME
    // Initially we don't worry about dwarf_add_directory_decl().
    
    bool firstline = true;
    bool addrsetincu = false;
    for(unsigned k = 0; k < cu_lines.size(); ++k) {
        IRCULine &li = cu_lines[k];
        const std::string&path = li.getpath();
        unsigned pathindex = 0;
        pathToUnsignedType::const_iterator it = pathmap.find(path);
        if(it == pathmap.end()) {
            Dwarf_Error error = 0;
            Dwarf_Unsigned idx = dwarf_add_file_decl(
                dbg,const_cast<char *>(path.c_str()),
                0,0,0,&error);
            if(idx == DW_DLV_NOCOUNT) {
                cerr << "Error from dwarf_add_file_decl() on " << 
                    path << endl;
                exit(1);
            }
            pathindex = idx;
            pathmap[path] = pathindex;
        } else {
            pathindex = it->second;
        }
        Dwarf_Addr a = li.getaddr();
        bool addrsetinline = li.getaddrset();
        bool endsequence = li.getendsequence();
        if(firstline || !addrsetincu) {
            // We fake an elf sym index here.
            Dwarf_Unsigned elfsymidx = 0;
            if(firstline && !addrsetinline) {
                cerr << "Error building line, first entry not addr set" << 
                    endl; 
                exit(1);
            }
            Dwarf_Unsigned res = dwarf_lne_set_address(dbg,
                a,elfsymidx,&error);
            if(res == DW_DLV_NOCOUNT) {
                cerr << "Error building line, dwarf_lne_set_address" << 
                    endl; 
                exit(1);
            }
            addrsetincu = true;
            firstline = false;
        } else if( endsequence) {
            Dwarf_Unsigned esres = dwarf_lne_end_sequence(dbg,
                a,&error);
            if(esres == DW_DLV_NOCOUNT) {
                cerr << "Error building line, dwarf_lne_end_sequence" << 
                    endl; 
                exit(1);
            }
            addrsetincu = false;
            continue;
        }
        Dwarf_Signed linecol = li.getlinecol();
        // It's really the code address or (when in a proper compiler)
        // a section or function offset.
        // libdwarf subtracts the code_offset from the address passed
        // this way or from dwarf_lne_set_address() and writes a small
        // offset in a DW_LNS_advance_pc instruction.
        Dwarf_Addr code_offset = a;
        Dwarf_Unsigned lineno = li.getlineno();
        Dwarf_Bool isstmt = li.getisstmt()?1:0;
        Dwarf_Bool isblock = li.getisblock()?1:0;
        Dwarf_Bool isepiloguebegin = li.getepiloguebegin()?1:0;
        Dwarf_Bool isprologueend = li.getprologueend()?1:0;
        Dwarf_Unsigned isa = li.getisa();
        Dwarf_Unsigned discriminator = li.getdiscriminator();
        Dwarf_Unsigned lires = dwarf_add_line_entry_b(dbg,
            pathindex, 
            code_offset,
            lineno,
            linecol,
            isstmt,
            isblock,
            isepiloguebegin,
            isprologueend,
            isa,
            discriminator,
            &error);
        if(lires == DW_DLV_NOCOUNT) {
            cerr << "Error building line, dwarf_add_line_entry" << 
                endl; 
            exit(1);
        }
    }
    if(addrsetincu) {
        cerr << "CU Lines did not end in an end_sequence!" << endl;
    }
}
// This emits the DIEs for a single CU and possibly line data
// associated with the CU.
// The DIEs form a graph (which can be created and linked together 
// in any order)  and which is emitted in tree preorder as 
// defined by the DWARF spec.
//
static void
emitOneCU( Dwarf_P_Debug dbg,IRepresentation & Irep, IRCUdata&cu,
    int cu_of_input_we_output)
{
    // We descend the the tree, creating DIEs and linking
    // them in as we return back up the tree of recursing
    // on IRDie children.
    Dwarf_Error error;
    
    IRDie & basedie =  cu.baseDie();
    Dwarf_P_Die cudie = HandleOneDieAndChildren(dbg,Irep,basedie,0);

    // Add base die to debug, this is the CU die.
    // This is not a good design as DWARF3/4 have
    // requirements of multiple CUs in a single creation,
    // which cannot be handled yet.
    Dwarf_Unsigned res = dwarf_add_die_to_debug(dbg,cudie,&error);
    if(res != DW_DLV_OK)  {
        cerr << "Unable to add_die_to_debug " << endl;
        exit(1);
    }

    HandleLineData(dbg,Irep,cu);
}
// .debug_info creation.
// Also creates .debug_line
static void
transform_debug_info(Dwarf_P_Debug dbg,
   IRepresentation & irep,int cu_of_input_we_output)
{
    int cu_number = 0;
    std::list<IRCUdata> &culist = irep.infodata().getCUData();
    // For now,  just one CU we write (as spoken by Yoda).
    
    for ( list<IRCUdata>::iterator it = culist.begin();
        it != culist.end();
        it++,cu_number++) {
        if(cu_number == cu_of_input_we_output) { 
            IRCUdata & primecu = *it;
            emitOneCU(dbg,irep,primecu,cu_of_input_we_output);
            break;
        }
    }
}
static void
transform_cie_fde(Dwarf_P_Debug dbg,
    IRepresentation & Irep,int cu_of_input_we_output)
{
    Dwarf_Error err = 0;
    std::vector<IRCie> &cie_vec = Irep.framedata().get_cie_vec();
    std::vector<IRFde> &fde_vec = Irep.framedata().get_fde_vec();
    
    for(size_t i = 0; i < cie_vec.size(); ++i) { 
        IRCie &ciein = cie_vec[i];
        Dwarf_Unsigned version = 0;
        string aug;
        Dwarf_Unsigned code_align = 0;
        Dwarf_Signed data_align = 0;
        Dwarf_Half ret_addr_reg = -1;
        void * bytes = 0;
        Dwarf_Unsigned bytes_len;
        ciein.get_basic_cie_data(&version, &aug,
            &code_align, &data_align, &ret_addr_reg);
        ciein.get_init_instructions(&bytes_len,&bytes);
        // version implied: FIXME, need to let user code set output
        // frame version.
        char *str = const_cast<char *>(aug.c_str());
        Dwarf_Signed out_cie_index =
            dwarf_add_frame_cie(dbg, str,
            code_align, data_align, ret_addr_reg,
            bytes,bytes_len,
            &err);
        if(out_cie_index == DW_DLV_NOCOUNT) {
            cerr << "Error creating cie from input cie " << i << endl;
            exit(1);
        }
        vector<int> fdeindex;
        // This inner loop is C*F so a bit slow.
        for(size_t j = 0; j < fde_vec.size(); ++j) { 
            IRFde &fdein = fde_vec[j];
            Dwarf_Unsigned code_len = 0;
            Dwarf_Addr code_virt_addr = 0;
            Dwarf_Signed cie_input_index = 0;
            fdein.get_fde_base_data(&code_virt_addr,
                &code_len, &cie_input_index);
            if(cie_input_index != i) {
                // Wrong cie, ignore this fde right now.
                continue;
            }

            
            Dwarf_P_Fde fdeout = dwarf_new_fde(dbg,&err);
            if(reinterpret_cast<Dwarf_Addr>(fdeout) == DW_DLV_BADADDR) {
                cerr << "Error creating new fde " << j << endl;
                exit(1);
            }
            Dwarf_Unsigned ilen = 0;
            void *instrs = 0;
            fdein.get_fde_instructions(&ilen, &instrs);

            int res = dwarf_insert_fde_inst_bytes(dbg,
                fdeout, ilen, instrs,&err);
            if(res != DW_DLV_OK) {
                cerr << "Error inserting frame instr block " << j << endl;
                exit(1);
            }
            
            Dwarf_P_Die irix_die = 0; 
            Dwarf_Signed irix_table_offset = 0;
            Dwarf_Unsigned irix_excep_sym = 0;
            Dwarf_Unsigned code_virt_addr_symidx = 
                Irep.getBaseTextSymbol();
            Dwarf_Unsigned fde_index = dwarf_add_frame_info(
                dbg, fdeout,irix_die,
                out_cie_index, code_virt_addr,
                code_len,code_virt_addr_symidx, 
                irix_table_offset,irix_excep_sym,
                &err);
            if(fde_index == DW_DLV_BADADDR) {
                cerr << "Error creating new fde " << j << endl;
                exit(1);
            }
        }
    }
}

static void 
transform_macro_info(Dwarf_P_Debug dbg,
   IRepresentation & Irep,int cu_of_input_we_output)
{
    IRMacro &macrodata = Irep.macrodata();
    std::vector<IRMacroRecord> &macrov = macrodata.getMacroVec();
    for(size_t m = 0; m < macrov.size() ; m++ ) {
        // FIXME: we need to coordinate with generated
        // CUs .
        cout << "FIXME: macros not really output yet " << 
            m << " " <<
            macrov.size() << endl;
    }
    Dwarf_Unsigned reloc_count = 0;
    int drd_version = 0;
    int res = dwarf_get_relocation_info_count(dbg,&reloc_count,
        &drd_version,&error);
    if( res != DW_DLV_OK) {
        cerr << "Error getting relocation info count." << endl;
        exit(1);

    }
    for( Dwarf_Unsigned ct = 0; ct < reloc_count ; ++ct) {
    }
}
void
transform_irep_to_dbg(Dwarf_P_Debug dbg,
   IRepresentation & Irep,int cu_of_input_we_output)
{
    transform_debug_info(dbg,Irep,cu_of_input_we_output);
    transform_cie_fde(dbg,Irep,cu_of_input_we_output);
    transform_macro_info(dbg,Irep,cu_of_input_we_output);
}
