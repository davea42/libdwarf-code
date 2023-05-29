*;
  Copyright (C) 2010-2019 David Anderson.  All rights reserved.

  Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that the
  following conditions are met:

  * Redistributions of source code must retain the above
  copyright notice, this list of conditions and the following
  disclaimer.

  * Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the following
  disclaimer in the documentation and/or other materials
  provided with the distribution.

  * Neither the name of the example nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior written
  permission.

  THIS SOFTWARE IS PROVIDED BY David Anderson ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL David
  Anderson BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
// dwarfgen.cc
//
// Using some information source, create a tree of dwarf
// information (speaking of a DIE tree).
// Turn the die tree into dwarfdata using libdwarf producer
// and write the resulting data in an object file.
// It is a bit inconsistent in error handling just to
// demonstrate the various possibilities using the producer
// library.
//
//  dwarfgen [-t def|obj|txt] [-o outpath] [-c cunum]  path

//  where -t means what sort of input to read
//         def means predefined (no input is read, the output
//         is based on some canned setups built into dwarfgen).
//         'path' is ignored in this case. This is the default.
//
//         obj means 'path' is required, it is the object file to
//             read (the dwarf sections are duplicated
//             in the output file)
//
//         txt means 'path' is required, path must contain plain text
//             (in a form rather like output by dwarfdump)
//             that defines the dwarf that is to be output.
//
//  where  -o means specify the pathname of the output object. If not
//         supplied testout.o is used as the default output path.
//  where -c supplies a CU number of the obj input to output
//         because the dwarf producer wants just one CU.
//         Default is -1 which won't match anything.

#include "config.h"

/* Windows specific header files */
#if defined(_WIN32) && defined(HAVE_STDAFX_H)
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h> /* for exit() */
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring> // For memcpy
#include <list>
#include <map>
#include <vector>
#include <string.h> /* for strchr etc */
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>  /* For open() S_IRUSR etc */
#endif /* HAVE_SYS_STAT_H */
#include <fcntl.h> //open
#include "general.h"
#include "dg_getopt.h"
#include "strtabdata.h"
#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarfp.h"
#include "libdwarf_private.h"
#include "dwarf_elf_data.h"
#include "dwarf_elfstructs.h"
#include "dwarf_elf_defines.h"
#include "dwarf_elf_interface.h"
#include "irepresentation.h"
#include "ireptodbg.h"
#include "createirepfrombinary.h"
#ifdef _WIN32
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif /* HAVE_STDINT_H */
#include <io.h>
#endif /* _WIN32 */

#ifdef _WIN32
#ifndef O_RDONLY
/*  This is for a Windows environment */
# define O_RDONLY _O_RDONLY
#endif
#ifdef _O_BINARY
/*  This is for a Windows environment */
#define O_BINARY _O_BINARY
#endif
#else /* !_WIN32 */
# ifndef O_BINARY
# define O_BINARY 0  /* So it does nothing in Linux/Unix */
# endif
#endif /* !_WIN32 */

/*  These are for a Windows environment */
#ifdef _WIN32
#ifdef _O_WRONLY
#define O_WRONLY _O_WRONLY
#endif
#ifdef _O_CREAT
#define O_CREAT _O_CREAT
#endif
#ifdef _O_TRUNC
#define O_TRUNC _O_TRUNC
#endif
#ifdef _S_IREAD
#define S_IREAD _S_IREAD
#endif
#ifdef _S_IWRITE
#define S_IWRITE _S_IWRITE
#endif
/* open modes S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH */
#ifndef S_IRUSR
#define S_IRUSR _S_IREAD
#endif // S_IRUSR
#ifndef S_IWUSR
#define S_IWUSR _S_IWRITE
#endif
#ifndef S_IRGRP
#define S_IRGRP 0
#endif
#ifndef S_IROTH
#define S_IROTH 0
#endif
#endif /* _WIN32 */

static void
bitreplace(char *outbuf, unsigned int buflen,
    Dwarf_Unsigned newval, unsigned int newvalsize);

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
extern "C" {
static int CallbackFunc(
    const char* name,
    int                 size,
    Dwarf_Unsigned      type,
    Dwarf_Unsigned      flags,
    Dwarf_Unsigned      link,
    Dwarf_Unsigned      info,
    Dwarf_Unsigned*     sect_name_symbol_index,
    void *              user_data,
    int*                error);
}
// End extern "C"

static void create_debug_sup_content(Dwarf_P_Debug dbg);
// FIXME. This is incomplete. See FIXME just below here.
#ifdef WORDS_BIGENDIAN
static void
_dwarf_memcpy_swap_bytes(void *s1, const void *s2, unsigned long len)
{
    unsigned char *targ = (unsigned char *) s1;
    const unsigned char *src = (const unsigned char *) s2;

    if (len == 4) {
        targ[3] = src[0];
        targ[2] = src[1];
        targ[1] = src[2];
        targ[0] = src[3];
    } else if (len == 8) {
        targ[7] = src[0];
        targ[6] = src[1];
        targ[5] = src[2];
        targ[4] = src[3];
        targ[3] = src[4];
        targ[2] = src[5];
        targ[1] = src[6];
        targ[0] = src[7];
    } else if (len == 2) {
        targ[1] = src[0];
        targ[0] = src[1];
    }
/* should NOT get below here: is not the intended use */
    else if (len == 1) {
        targ[0] = src[0];
    } else {
        memcpy(s1, s2, (size_t)len);
    }
    return;
}
#endif /* WORDS_BIGENDIAN */

//  There are issues 32/64 here, and endianness.
//  We are asserting here in dwarfgen that the target
//  object created is to be DW_DLC_TARGET_LITTLEENDIAN.
//  FIXME. not general.
//  op and ol aread MUST NOT overlap.
//  ip and il should be 2,4,or 8. Nothing else.
//  These could perfectly well be functions.
#ifdef WORDS_BIGENDIAN
#define ASNX(op,ol,ip,il)           \
    do {                            \
        if (ol > il) {              \
            unsigned sbyte = 0;     \
            memset(op,0,ol);        \
            sbyte = ol - il;        \
            const void * ipi =      \
                (const void *)ip;   \
            _dwarf_memcpy_swap_bytes(((char *)(op))+sbyte,\
                ipi,il);\
        } else {                    \
            unsigned sbyte = 0;     \
            sbyte = il - ol;        \
            const void * ipi =      \
                ((Dwarf_Small*)ip) +sbyte; \
            _dwarf_memcpy_swap_bytes((char *)(op),      \
                (const void *)ipi,  \
                 ol);               \
        }                           \
    } while (0)
#else // LITTLEENDIAN
#define ASNX(op,ol,ip,il)           \
    do {                            \
        if (ol > il) {              \
            const void *ipi =       \
                (const void *)ip;   \
            memset(op,0,ol);        \
            memcpy(((char *)(op)),  \
               ipi,il);             \
        } else {                    \
            const void *ipi =       \
                (const void *)ip;   \
            memcpy((char *)(op),    \
                ipi,ol);            \
        }                           \
    } while (0)
#endif // ENDIANNESS

static void write_object_file(Dwarf_P_Debug dbg,
    IRepresentation &irep,
    unsigned machine,
    unsigned endian,
    unsigned long dwbitflags,
    void * user_data);

static void create_initial_section(Dwarf_Elf * elf);
static void create_text_section(Dwarf_Elf * elf,unsigned elfclass);
static void write_generated_dbg(Dwarf_P_Debug dbg,Dwarf_Elf * elf,
    IRepresentation &irep);

static string outfile("testout.o");
static string infile;
static enum  WhichInputSource { OptNone, OptReadText,
    OptReadBin,OptPredefined}
    whichinput(OptPredefined);

//  Use a generic call to open the file 
//  (issues with Windows)
int open_a_file(const char * name);
void close_a_file(int f);
void calculate_all_offsets(void);
Dwarf_Unsigned write_to_object(void);

// This is a global so thet CallbackFunc can get to it
// If we used the dwarf_producer_init_c() user_data pointer
// creatively we would not need a global.
static IRepresentation Irep;

static Dwarf_Elf * elf = 0;
static strtabdata secstrtab;

CmdOptions cmdoptions = {
    false, //transformHighpcToConst
    DW_FORM_string, // defaultInfoStringForm
    false, //showrelocdetails
    false, //adddata16
    false, //addimplicitconst
    false, //addframeadvanceloc
    false, //addSUNfuncoffsets
    false, //add_debug_sup
    false //addskipbranch
};

// loff_t is signed for some reason (strange)
// but we make offsets unsigned.
#define LOFFTODWUNS(x)  ( (Dwarf_Unsigned)(x))

class Writer {
public:
    int fd_;
    std::string name_;
    Dwarf_Unsigned curoffset_;
    Writer() {
        fd_ = -1;
        curoffset_ =0;
    };

    void write(Dwarf_Unsigned offset,
        Dwarf_Unsigned length,
        const void *bytes) {
        if (fd == -1) {
            cerr << "dwarfgen: write with fd_ -1" << endl;
            exit(1);
        }
        if (curoffset_ != offset) {
            off_t v = lseek(fd_,offset,SEEK_SET);
            if (v == (off_t)-1) {
               cerr << "dwarfgen: fseek to " << offset<<
                   "fails " << endl;
               exit(1);
            }
            curoffset_ = v;
        }
        ssize_t wrote = write(fd_,bytes,length);
        if (wrote == -1) {
               cerr << "dwarfgen: write length " << length<<
                   "fails " << endl;
               exit(1);
        }
        curoffset_ += length;
    }
    void closeFile() {
        if (fd_ != -1) {
            close(fd_);
            fd_ = -1;
        }
        curoffset_ = 0;
    }
    ~Writer() {
        closeFile();
    };
    void openFile(std::string & name) {
        name_ = name;
        fd_ =  open(name, O_RDONLY | O_BINARY);
        if (fd_ ==-1) {
            cerr << "dwarfgen: open " <<<name<< " fails" << endl;
            exit(1);
        }
        curoffset_ =0;
        return DW_DLV_OK;
    }
};

class ByteBlob {
public:
    ByteBlob() {bytes_ = 0;,len_=0;};
    ByteBlob(unsigned char *bytes,size_t len) {
       bytes_ = bytes;
       len_ = len;
    };
    setFileOffset(Dwarf_Unsigned b) {fileoffset_=b;};
    Dwarf_Unsigned getFileOffset() { return fileoffset_;};
    setBlob(char *bytes,size_t len) {
       if (len_) {
           cerr << "dwarfgen: Duplicate ByteBlob setting "
              "for length " len_ " to length " len << endl;
           exit(1); 
       }
       bytes_ = bytes;
       len_ = len;
    };
    unsigned char *bytes_;
    Dwarf_Unsigned len_;
    Dwarf_Unsigned fileoffset_;;
    ~ByteBlob() { bytes_ = 0; len_= 0; };
};

/*  See the Elf ABI for further definitions of these fields. */
//  By definition, this outputs data to file offset zero. 
class ElfHeaderFromDwarf {
public:
    unsigned char e_ident_[EI_NIDENT];
    Dwarf_Unsigned e_type_;
    Dwarf_Unsigned e_machine_;
    Dwarf_Unsigned e_version_;
    Dwarf_Unsigned e_entry_;
    Dwarf_Unsigned e_phoff_;
    Dwarf_Unsigned e_shoff_;
    Dwarf_Unsigned e_flags_;
    Dwarf_Unsigned e_ehsize_;
    Dwarf_Unsigned e_phentsize_;
    Dwarf_Unsigned e_phnum_;
    Dwarf_Unsigned e_shentsize_;
    Dwarf_Unsigned e_shnum_;
    Dwarf_Unsigned e_shstrndx_;
    dw_elf32_header e_32_;
    dw_elf64_header e_64_;
    unsigned  char  e_ptrbytesize_ = 0;
    Dwarf_Unsigned  e_hdrlen_;
    bool is_64bit() { return e_ptrbytesize == 8 };
    bool is_32bit() { return e_ptrbytesize == 4 };
    ElfHeaderFromDwarf() {
        memset(e_ident_,0,sizeof(e_ident_), 
        e_type_= 0; e_machine_= 0;; e_version_= 0;; e_entry_= 0;
        e_phoff_= 0; e_shoff_= 0; e_flags_= 0; e_ehsize_= 0;
        e_phentsize_= 0; e_phnum_= 0; e_shentsize_= 0; e_shnum_= 0;
        e_shstrndx_ = 0; e_content_ = 0 e_ptrbytesize_=0;
        memset(&e_32_,0,sizeof(e_32_);
        memset(&e_64_,0,sizeof(e_64_);
        e_hdrlen_ = 0;
    };
};
/*  See the Elf ABI for further definitions of these fields. */
class SectionFromDwarf {
public:
    std::string    name_;
    Dwarf_Unsigned section_name_itself_;
    ElfSymIndex    section_name_symidx_;
    Dwarf_Unsigned size_;
    Dwarf_Unsigned type_;
    Dwarf_Unsigned flags_;

    /*  type: SHT_REL, RELA: Section header index of the section
        relocation applies to.
        SHT_SYMTAB: Section header index of the
        associated string table. */
    Dwarf_Unsigned link_;

    /*  type: SHT_REL, RELA: Section header index of the section
        relocation applies to.
        SHT_SYMTAB: One greater than index of the
        last local symbol.. */
    Dwarf_Unsigned info_;
private:
    ElfSectIndex elf_sect_index_;
    Dwarf_Unsigned lengthWrittenToElf_;

    struct dw_elf64_shdr shdr64_;
    struct dw_elf32_shdr shdr32_;
    // In output object section endianness
    ElfHeaderFromDwarf   fileheader_;

    vector<ByteBlob> sectioncontent_;
    Dwarf_Unsigned   sectioncontentlen_;
    Dwarf_Unsigned   sectionoutoffset_; //Offset in the output object.
    
    
public:
    Dwarf_Unsigned getNextOffset() { return lengthWrittenToElf_; }
    void setNextOffset(Dwarf_Unsigned v) { lengthWrittenToElf_ = v; }

    void add_section_header(char *data,Dwarf_Unsigned datalen) { 
        section_header.setBlob(data,datalen);
    }
    void add_section_content(char *data,Dwarf_Unsigned datalen) {
        x = new ByteBlob(data,datalen);  
        sectioncontent_.push_back(x);
        sectioncontentlen_ += datalen;
    }

    unsigned getSectionNameSymidx() {
        return section_name_symidx_.getSymIndex(); };
    SectionFromDwarf():section_name_itself_(0),
        section_name_symidx_(0),
        size_(0),type_(0),flags_(0),
        link_(0), info_(0), elf_sect_index_(0),
        lengthWrittenToElf_(0) {} ;
    ~SectionFromDwarf() {};
    void setSectIndex(ElfSectIndex v) { elf_sect_index_ = v;}
    ElfSectIndex getSectIndex() const { return elf_sect_index_;}
    SectionFromDwarf(const std::string&name,
        int size,Dwarf_Unsigned type,Dwarf_Unsigned flags,
        Dwarf_Unsigned link, Dwarf_Unsigned info):
        name_(name),
        size_(size),type_(type),flags_(flags),
        link_(link), info_(info), elf_sect_index_(0),
        lengthWrittenToElf_(0) {
            // Now create section name string section.
            section_name_itself_ = secstrtab.addString(name.c_str());
            ElfSymbols& es = Irep.getElfSymbols();
            // Now creat a symbol for the section name.
            // (which has its own string table)
            section_name_symidx_  = es.addElfSymbol(0,name);
    } ;
}; // SectionFromDwarf

// Data for section header headers is kept in native integers
// as well as target-endianness. 
// Eventually contains the section headers
// and section content, including Elf Section zero.
// The vector is indexable by ElfSectIndex
vector<SectionFromDwarf> dwsectab;

// Contains the elf header, native integers and target-endianness 
ElfHeaderFromDwarf       dwelfheader;

Writer                   dwwriter;

static ElfSectIndex create_dw_elf(SectionFromDwarf  &ds,
    unsigned elfclass);

static SectionFromDwarf & FindMySection(
    const ElfSectIndex & elf_section_index)
{
    for (unsigned i =0; i < dwsectab.size(); ++i) {
        if (elf_section_index.getSectIndex() !=
            dwsectab[i].getSectIndex().getSectIndex()) {
            continue;
        }
        return dwsectab[i];
    }
    cerr << "dwarfgen: Unable to find my dw sec data "
        "for elf section " <<
        elf_section_index.getSectIndex() << endl;
    exit(1);
}

static int FindMySectionNum(const ElfSectIndex & elf_section_index)
{
    for (unsigned i =0; i < dwsectab.size(); ++i) {
        if (elf_section_index.getSectIndex() !=
            dwsectab[i].getSectIndex().getSectIndex()) {
            continue;
        }
        return i;
    }
    cerr << "dwarfgen: Unable to find my dw sec index "
        "for elf section " <<
        elf_section_index.getSectIndex() << endl;
    exit(1);
}

static Dwarf_Unsigned
createnamestr(unsigned strtabstroff,unsigned elfclass)
{
    DW_Elf_Scn * strscn =_dwarf_elf_newscn(elf);
    if (!strscn) {
        cerr << "dwarfgen: Unable to _dwarf_elf_newscn() on "
            << outfile << endl;
        exit(1);
    }
    DW_Elf_Data* shstr =_dwarf_elf_newdata(strscn);
    if (!shstr) {
        cerr << "dwarfgen: Unable to _dwarf_elf_newdata() on "
            << outfile << endl;
        exit(1);
    }
    shstr->d_buf = secstrtab.exposedata();
    shstr->d_type =  DWARF_T_BYTE;
    shstr->d_size = secstrtab.exposelen();
    shstr->d_off = 0;
    shstr->d_align = 1;
    shstr->d_version = EV_CURRENT;
    if (elfclass == ELFCLASS32) {
        dw_elf32_shdr * strshdr = _dwarf_elf32_getshdr(strscn);
        if (!strshdr) {
            cerr << "dwarfgen: Unable to elf32_getshdr() on "
                << outfile << endl;
            exit(1);
        }
        //strshdr->sh_name =  strtabstroff;
        ASNX(strshdr->sh_name,sizeof(strshdr->sh_name),
             &strtabstroff,sizeof(strtabstroff));
        unsigned long type = SHT_STRTAB;
        //strshdr->sh_type= SHT_STRTAB;
        ASNX(strshdr->sh_type,sizeof(strshdr->sh_type),
             &type,sizeof(type));
        //strshdr->sh_flags = SHF_STRINGS;
        unsigned long flags = SHF_STRINGS;
        ASNX(strshdr->sh_flags,sizeof(strshdr->sh_flags),
             &flags,sizeof(flags));
        unsigned long zero = 0;
        //strshdr->sh_addr = 0;
        ASNX(strshdr->sh_addr,sizeof(strshdr->sh_addr),
             &zero,sizeof(zero));
        //strshdr->sh_offset = 0;
        ASNX(strshdr->sh_offset,sizeof(strshdr->sh_offset),
             &zero,sizeof(zero));
        //strshdr->sh_size = 0;
        ASNX(strshdr->sh_size,sizeof(strshdr->sh_size),
             &zero,sizeof(zero));
        //strshdr->sh_link  = 0;
        ASNX(strshdr->sh_link,sizeof(strshdr->sh_link),
             &zero,sizeof(zero));
        //strshdr->sh_info = 0;
        ASNX(strshdr->sh_info,sizeof(strshdr->sh_info),
             &zero,sizeof(zero));
        //strshdr->sh_addralign = 1;
        unsigned long align = 1;
        ASNX(strshdr->sh_addralign,sizeof(strshdr->sh_addralign),
             &align,sizeof(align));
        ASNX(strshdr->sh_entsize,sizeof(strshdr->sh_entsize),
             &zero,sizeof(zero));
        //strshdr->sh_entsize = 0;
    } else {
        dw_elf64_shdr * strshdr = _dwarf_elf64_getshdr(strscn);
        if (!strshdr) {
            cerr << "dwarfgen: Unable to elf64_getshdr() on "
                << outfile << endl;
            exit(1);
        }
        //strshdr->sh_name =  strtabstroff;
        ASNX(strshdr->sh_name,sizeof(strshdr->sh_name),
             &strtabstroff,sizeof(strtabstroff));
        //strshdr->sh_flags = SHF_STRINGS;
        unsigned long flags = SHF_STRINGS;
        ASNX(strshdr->sh_flags,sizeof(strshdr->sh_flags),
             &flags,sizeof(flags));
        unsigned long zero = 0;
        //strshdr->sh_addr = 0;
        ASNX(strshdr->sh_addr,sizeof(strshdr->sh_addr),
             &zero,sizeof(zero));
        //strshdr->sh_offset = 0;
        ASNX(strshdr->sh_offset,sizeof(strshdr->sh_offset),
             &zero,sizeof(zero));
        //strshdr->sh_size = 0;
        ASNX(strshdr->sh_size,sizeof(strshdr->sh_size),
             &zero,sizeof(zero));
        //strshdr->sh_link  = 0;
        ASNX(strshdr->sh_link,sizeof(strshdr->sh_link),
             &zero,sizeof(zero));
        //strshdr->sh_info = 0;
        ASNX(strshdr->sh_info,sizeof(strshdr->sh_info),
             &zero,sizeof(zero));
        //strshdr->sh_addralign = 1;
        unsigned long align = 1;
        ASNX(strshdr->sh_addralign,sizeof(strshdr->sh_addralign),
             &align,sizeof(align));
        ASNX(strshdr->sh_entsize,sizeof(strshdr->sh_entsize),
             &zero,sizeof(zero));
        //strshdr->sh_entsize = 0;
    }

    Dwarf_Unsigned retsecnum = 0;
    int res = _dwarf_elf_ndxscn(strscn,&retsecnum);
    if (res != DW_DLV_OK) {
        cerr << " dwarf_ndxscn failed. " <<
            " line " << __LINE__ << " " <<__FILE__ << endl;
        exit(1);
    }
    return retsecnum;
}

// This functional interface is defined by libdwarf.
// Please see the comments in libdwarf2p.1.pdf
// (libdwarf2p.1.mm)  on this callback interface.
// Returns (to libdwarf) an Elf section number, so
// since 0 is always empty and dwarfgen sets 1 to be a fake
// text section on the first call this returns 2, second 3, etc.
static int CallbackFunc(
    const char* name,
    int                 size,
    Dwarf_Unsigned      type,
    Dwarf_Unsigned      flags,
    Dwarf_Unsigned      link,
    Dwarf_Unsigned      info,
    Dwarf_Unsigned*     sect_name_symbol_index,
    void *              user_data,
    int*                error)
{
    // Create an elf section.
    // If the data is relocations, we suppress the generation
    // of a section when we intend to do the relocations
    // ourself (fine for dwarfgen but would
    // be really surprising for a normal compiler
    // back end using the producer code).

    // The section name appears both in the section strings
    // .shstrtab and
    // in the elf symtab .symtab and its strings .strtab.
    unsigned elfclass = 0;

    (void)error;
    if (user_data) {
        elfclass = *(unsigned *)user_data;
    } else {
        cerr << "We created an internal-to-dwarfgen bug here. " <<
            " line " << __LINE__ << " " <<__FILE__ << endl;
        exit(1);
    }

    if (0 == strncmp(name,".rel",4))  {
        // It is relocation, create no section!
        return 0;
    }
    SectionFromDwarf ds(name,size,type,flags,link,info) ;

    // It is up to you to provide (to libdwarf,
    // to generate relocation records)
    // a symbol index for the section.
    // In Elf, each section gets an elf symbol table entry.
    // So that relocations have an address to refer to.
    // You will create the Elf symbol table, so you have to tell
    // libdwarf the index to put into relocation records for the
    // section newly defined here.
    *sect_name_symbol_index = ds.getSectionNameSymidx();
    ElfSectIndex createdsec = create_dw_elf(ds,elfclass);

    // Do all the data creation before pushing
    // (copying) ds onto dwsectab!
    dwsectab.push_back(ds);
    // The number returned is elf section, not dwsectab[] index

    return createdsec.getSectIndex();
}

// Here we create a new Elf section
// This never happens for relocations in dwarfgen,
// only a few sections are created by dwarfgen.
static ElfSectIndex
create_dw_elf(SectionFromDwarf  &ds,unsigned elfclass)
{
    DW_Elf_Scn * scn =_dwarf_elf_newscn(elf);
    if (!scn) {
        cerr << "dwarfgen: Unable to _dwarf_elf_newscn() on " <<
            ds.name_  << endl;
        exit(1);
    }
    unsigned long zero = 0;
    unsigned long  one = 1;
    if (elfclass == ELFCLASS32) {
        dw_elf32_shdr * shdr = _dwarf_elf32_getshdr(scn);
        if (!shdr) {
            cerr << "dwarfgen: Unable to elf32_getshdr() on "
                << ds.name_ << endl;
            exit(1);
        }
        //shdr->sh_name   = ds.section_name_itself_;
        ASNX(shdr->sh_name,sizeof(shdr->sh_name),
            &ds.section_name_itself_,sizeof(ds.section_name_itself_));
        //shdr->sh_type   = ds.type_;
        ASNX(shdr->sh_type,sizeof(shdr->sh_type),
            &ds.type_,sizeof(ds.type_));
        //shdr->sh_flags  = ds.flags_;
        ASNX(shdr->sh_flags,sizeof(shdr->sh_flags),
            &ds.flags_,sizeof(ds.flags_));
        //shdr->sh_addr   = 0;
        ASNX(shdr->sh_addr,sizeof(shdr->sh_addr),
            &zero,sizeof(zero));
        //shdr->sh_offset = 0;
        ASNX(shdr->sh_offset,sizeof(shdr->sh_offset),
            &zero,sizeof(zero));

        //shdr->sh_size   = ds.size_;
        ASNX(shdr->sh_size,sizeof(shdr->sh_size),
            &ds.size_,sizeof(ds.size_));
        //shdr->sh_link   = ds.link_;
        ASNX(shdr->sh_link,sizeof(shdr->sh_link),
            &ds.link_,sizeof(ds.link_));
        //shdr->sh_info   = ds.info_;
        ASNX(shdr->sh_info,sizeof(shdr->sh_info),
            &ds.info_,sizeof(ds.info_));
        //shdr->sh_addralign = 1;
        ASNX(shdr->sh_addralign,sizeof(shdr->sh_addralign),
            &one,sizeof(one));
        ASNX(shdr->sh_entsize,sizeof(shdr->sh_entsize),
            &zero,sizeof(zero));
    } else {
        dw_elf64_shdr * shdr = _dwarf_elf64_getshdr(scn);
        if (!shdr) {
            cerr << "dwarfgen: Unable to elf64_getshdr() on "
                << ds.name_ << endl;
            exit(1);
        }
        //shdr->sh_name   = ds.section_name_itself_;
        ASNX(shdr->sh_name,sizeof(shdr->sh_name),
            &ds.section_name_itself_,sizeof(ds.section_name_itself_));
        //shdr->sh_type   = ds.type_;
        ASNX(shdr->sh_type,sizeof(shdr->sh_type),
            &ds.type_,sizeof(ds.type_));
        //shdr->sh_flags  = ds.flags_;
        ASNX(shdr->sh_flags,sizeof(shdr->sh_flags),
            &ds.flags_,sizeof(ds.flags_));
        //shdr->sh_addr   = 0;
        ASNX(shdr->sh_addr,sizeof(shdr->sh_addr),
            &zero,sizeof(zero));
        //shdr->sh_offset = 0;
        ASNX(shdr->sh_offset,sizeof(shdr->sh_offset),
            &zero,sizeof(zero));

        //shdr->sh_size   = ds.size_;
        ASNX(shdr->sh_size,sizeof(shdr->sh_size),
            &ds.size_,sizeof(ds.size_));
        //shdr->sh_link   = ds.link_;
        ASNX(shdr->sh_link,sizeof(shdr->sh_link),
            &ds.link_,sizeof(ds.link_));
        //shdr->sh_info   = ds.info_;
        ASNX(shdr->sh_info,sizeof(shdr->sh_info),
            &ds.info_,sizeof(ds.info_));

        //shdr->sh_addralign = 1;
        ASNX(shdr->sh_addralign,sizeof(shdr->sh_addralign),
            &one,sizeof(one));

        //shdr->sh_entsize = 0;
        ASNX(shdr->sh_entsize,sizeof(shdr->sh_entsize),
            &zero,sizeof(zero));
    }
    Dwarf_Unsigned retsecnum = 0;
    int res = _dwarf_elf_ndxscn(scn,&retsecnum);
    if (res != DW_DLV_OK) {
        cerr << "dwarfgen:  dwarf_elf_ndxscn FAILs" << endl;
        exit(1);
    }
    ElfSectIndex si(retsecnum);

    ds.setSectIndex(si);
    cout << "New Elf section: "<< ds.name_ <<
        " Type="<< ds.type_ <<
        " Flags="<< ds.flags_ <<
        " Elf secnum="<< si.getSectIndex() <<
        " link section=" << ds.link_<<
        " info=" << ds.info_ << endl ;
    return  si;
}

static void
setinput(enum  WhichInputSource *src,
    const string &type,
    bool *pathreq)
{
    if (type == "txt") {
        *src = OptReadText;
        *pathreq = true;
        return;
    } else if (type == "obj") {
        *src = OptReadBin;
        *pathreq = true;
        return;
    } else if (type == "def") {
        *src = OptPredefined;
        *pathreq = false;
        return;
    }
    cout << "dwarfgen: Giving up, only txt obj or def "
        "accepted after -t" << endl;
    exit(1);
}

int
main(int argc, char **argv)
{
    try {
        int opt;
        bool pathrequired(false);
        long cu_of_input_we_output = -1;
        bool force_empty_dnames = false;

        // Overriding macro constants from pro_line.h
        // so we can choose at runtime
        // and avoid modifying header files.
        // This is the original set of constants
        // used for testing.
        // see also output_v4_test (--output-v4-test
        // longopt).
        const char *dwarf_extras ="opcode_base=13,"
            "minimum_instruction_length=1,"
            "line_base=-1,"
            "line_range=4";
        int endian =  DW_DLC_TARGET_LITTLEENDIAN;
        const char *dwarf_version = "V2";
        const char * isa_name = "x86";
        unsigned long ptrsizeflagbit = DW_DLC_POINTER32;
        unsigned long dwarfoffsetsizeflagbit = DW_DLC_OFFSET32;
        /*  DW_DLC_ELF_OFFSET_SIZE 32/64 winds up determining
            the ELFCLASS 32/64 in write_object_file() */
        unsigned long elfoffsetsizeflagbit =
            DW_DLC_ELF_OFFSET_SIZE_32;
        unsigned machine = EM_386; /* from elf.h */
        int output_v4_test = 0;

        unsigned global_elfclass = 0;

        int longindex;
        static struct dwoption longopts[] = {
            {"adddata16",dwno_argument,0,1000},
            {"force-empty-dnames",dwno_argument,0,1001},
            {"add-implicit-const",dwno_argument,0,1002},
            {"add-frame-advance-loc",dwno_argument,0,1003},
            {"add-sun-func-offsets",dwno_argument,0,1004},
            {"add-debug-sup",dwno_argument,0,1006},
            {"output-pointer-size",dwrequired_argument,0,'p'},
            {"output-offset-size",dwrequired_argument,0,'f'},
            {"output-dwarf-version",dwrequired_argument,0,'v'},
            {"output-v4-test",dwno_argument,0,1005},
            {"default-form-strp",dwno_argument,0,'s'},
            {"show-reloc-details",dwno_argument,0,'r'},
            {"high-pc-as-const",dwno_argument,0,'h'},
            {"add-skip-branch-ops",dwno_argument,0,1007},
            {0,0,0,0},
        };
        // -p is pointer size
        // -f is offset size, overriding the above
        // -v is version number for dwarf output

        while((opt=dwgetopt_long(argc,argv,
            "o:t:c:hsrv:p:f:",
            longopts,&longindex)) != -1) {
            switch(opt) {
            case 1000:
                if (longindex == 0) {
                    // To test adding the DWARF5
                    // DW_FORM_data16
                    // libdwarf reading is thus testable.
                    cmdoptions.adddata16 = true;
                } else {
                    cerr << "dwarfgen: Invalid lnogoption input " <<
                        longindex << endl;
                    exit(1);
                }
                break;
            case 1001:
                // To test having an empty .debug_dnames
                // section.
                // libdwarf reading is thus testable.
                force_empty_dnames = true;
                break;
            case 1002:
                // To test creating DWARF5
                // DW_FORM_implicit_const.
                // libdwarf reading is thus testable.
                cmdoptions.addimplicitconst = true;
                break;
            case 1003:
                // To test dwarf_add_fde_inst_a().
                // libdwarf reading is thus testable.
                cmdoptions.addframeadvanceloc = true;
                break;
            case 1004:
                //{"add-sun-func-offsets",dwno_argument,0,1004},
                // To test creating DWARF5
                // DW_AT_SUN_func_offsets.
                // libdwarf reading is thus testable.
                cmdoptions.addSUNfuncoffsets = true;
                break;
            case 1005:
                //{"output-v4-test",dwno_argument,0,1005}
                output_v4_test = 1;
                break;
            case 1006:
                //{"add-debug-sup",dwno_argument,0,1006}
                cmdoptions.adddebugsup = true;
                break;
            case 1007:
                //{"add-skip-branch-ops",dwno_argument,0,1007},
                cmdoptions.addskipbranch = true;
                break;
            case 'c':
                // At present we can only create a single
                // cu in the output of the libdwarf producer.
                cu_of_input_we_output = atoi(dwoptarg);
                break;
            case 'r':
                cmdoptions.showrelocdetails=true;
                break;
            case 's': // --default-form-strp
                cmdoptions.defaultInfoStringForm = DW_FORM_strp;
                break;
            case 'p': /* pointer size: value 4 or 8. */
                if (!strcmp("4",dwoptarg)) {
                    ptrsizeflagbit = DW_DLC_POINTER32;
                    elfoffsetsizeflagbit = DW_DLC_ELF_OFFSET_SIZE_32;
                } else if (!strcmp("8",dwoptarg)) {
                    ptrsizeflagbit = DW_DLC_POINTER64;
                    elfoffsetsizeflagbit = DW_DLC_ELF_OFFSET_SIZE_64;
                } else {
                    cerr << "dwarfgen: Invalid p option input " <<
                        dwoptarg << endl;
                    exit(1);
                }
                break;
            case 'f': /*  offset size for DWARF: value 4 or 8. */
                if (!strcmp("4",dwoptarg)) {
                    dwarfoffsetsizeflagbit = DW_DLC_OFFSET32;
                } else if (!strcmp("8",dwoptarg)) {
                    dwarfoffsetsizeflagbit = DW_DLC_OFFSET64;
                } else {
                    cerr << "dwarfgen: Invalid f option input " <<
                        dwoptarg << endl;
                    exit(1);
                }
                break;
            case 'v': /* Version 2 3 4 or 5 */
                if (!strcmp("5",dwoptarg)) {
                    dwarf_version = "V5";
                } else if (!strcmp("4",dwoptarg)) {
                    dwarf_version = "V4";
                } else if (!strcmp("3",dwoptarg)) {
                    dwarf_version = "V3";
                } else if (!strcmp("2",dwoptarg)) {
                    dwarf_version = "V2";
                } else {
                    cerr << "dwarfgen: Invalid v option input " <<
                        dwoptarg << endl;
                    exit(1);
                }
                break;
            case 't':
                setinput(&whichinput,dwoptarg,&pathrequired);
                break;
            case 'h':
                cmdoptions.transformHighpcToConst = true;
                break;
            case 'o':
                outfile = dwoptarg;
                break;
            case '?':
                cerr << "dwarfgen: Invalid quest? option input "
                    << endl;
                exit(1);
            default:
                cerr << "dwarfgen: Invalid option input " << endl;
                exit(1);
            }
        }
        if ( (dwoptind >= argc) && pathrequired) {
            cerr << "dwarfgen: Expected argument after options!"
                " Giving up."
                << endl;
            exit(EXIT_FAILURE);
        }
        if (pathrequired) {
            infile = argv[dwoptind];
        }
        if (output_v4_test) {
            dwarf_extras ="opcode_base=13,"
                "minimum_instruction_length=1,"
                "line_base=-1,"
                "line_range=4";
            endian =  DW_DLC_TARGET_LITTLEENDIAN;
            dwarf_version = "V4";
            isa_name = "x86_64";
            ptrsizeflagbit = DW_DLC_POINTER64;
            dwarfoffsetsizeflagbit = DW_DLC_OFFSET32;
            elfoffsetsizeflagbit = DW_DLC_ELF_OFFSET_SIZE_64;
            machine = EM_X86_64; /* from elf.h */
        }

        if (whichinput == OptReadBin) {
            createIrepFromBinary(infile,Irep);
        } else if (whichinput == OptReadText) {
            cerr << "dwarfgen: dwarfgen: text read not supported yet"
                << endl;
            exit(EXIT_FAILURE);
        } else if (whichinput == OptPredefined) {
            cerr << "dwarfgen: predefined not supported yet" << endl;
            exit(EXIT_FAILURE);
        } else {
            cerr << "dwarfgen: Impossible: unknown input style."
                << endl;
            exit(EXIT_FAILURE);
        }

        // We use the latest calls returning
        // DW_DLV_OK, DW_DLV_NO_ENTRY, or DW_DLV_ERROR
        // as an int.
        Dwarf_Ptr errarg = 0;
        Dwarf_Error err = 0;

        // The point of user_data is, as here, to
        // have crucial data available to the callback
        // function implementation.
        void *user_data = &global_elfclass;

        Dwarf_P_Debug dbg = 0;
        unsigned long dwbitflags =
            endian |
            ptrsizeflagbit|
            elfoffsetsizeflagbit|
            dwarfoffsetsizeflagbit|
            DW_DLC_SYMBOLIC_RELOCATIONS;

        // We use DW_DLC_SYMBOLIC_RELOCATIONS so we can
        // read the relocations and do our own relocating.
        // See calls of dwarf_get_relocation_info().
        int res = dwarf_producer_init(
            dwbitflags,
            CallbackFunc,
            0, // errhand
            errarg,
            user_data,
            isa_name,
            dwarf_version,
            dwarf_extras,
            &dbg,
            &err);
        if (res == DW_DLV_NO_ENTRY) {
            cerr << "dwarfgen: Failed dwarf_producer_init() NO_ENTRY"
                << endl;
            exit(EXIT_FAILURE);
        }
        if (res == DW_DLV_ERROR) {
            cerr << "dwarfgen: Failed dwarf_producer_init() ERROR"
                << endl;
            cerr << "dwarfgen errmsg " << dwarf_errmsg(err)<<endl;
            exit(EXIT_FAILURE);
        }
        res = dwarf_pro_set_default_string_form(dbg,
            cmdoptions.defaultInfoStringForm,&err);
        if (res != DW_DLV_OK) {
            cerr << "dwarfgen: Failed " <<
                "dwarf_pro_set_default_string_form" << endl;
            exit(EXIT_FAILURE);
        }
        if (force_empty_dnames) {
            /*  Fills out a default dnames for testing. */
            res = dwarf_force_dnames(dbg,0,&err);
            if (res != DW_DLV_OK) {
                cerr << "dwarfgen: "
                    "Failed dwarf_force_debug_names"
                    << endl;
                exit(EXIT_FAILURE);
            }
        }
        if (cmdoptions.adddebugsup) {
            create_debug_sup_content(dbg);
        }
        transform_irep_to_dbg(dbg,Irep,cu_of_input_we_output);
        write_object_file(dbg,Irep,machine,endian, dwbitflags,
            user_data);

        Dwarf_Unsigned str_count = 0;
        Dwarf_Unsigned str_len = 0;
        Dwarf_Unsigned debug_str_count = 0;
        Dwarf_Unsigned debug_str_len = 0;
        Dwarf_Unsigned reused_count = 0;
        Dwarf_Unsigned reused_len = 0;
        res = dwarf_pro_get_string_stats(dbg,
            &str_count,&str_len,
            &debug_str_count,
            &debug_str_len,
            &reused_count,
            &reused_len,&err);
        if (res != DW_DLV_OK) {
            cout << "Unable to get string statistics. ERROR."
                << endl;
        } else {
            cout << "Debug_Str: debug_info str count " <<str_count <<
                ", byte total len " <<str_len << endl;
            cout << "Debug_Str: count " <<debug_str_count <<
                ", byte total len " <<debug_str_len << endl;
            cout << "Debug_Str: Reused count " <<reused_count <<
                ", byte total len not emitted " <<reused_len << endl;
        }
        dwarf_producer_finish_a( dbg, 0);
        return 0;
    } // End try
    catch (std::bad_alloc &ba) {
        cout << "dwarfgen FAIL:bad alloc caught " <<
            ba.what() << endl;
        exit(EXIT_FAILURE);
    }
    catch (std::exception &e) {
        cout << "dwarfgen FAIL:std lib exception " <<
            e.what() << endl;
        exit(EXIT_FAILURE);
    }
    catch (...) {
        cout << "dwarfgen FAIL:other failure " << endl;
        exit(EXIT_FAILURE);
    }
    exit(1);
}

static void
create_debug_sup_content(Dwarf_P_Debug dbg)
{
    int res = 0;
    Dwarf_Error err = 0;
    char x_content[] = "fake checksum_content";
    Dwarf_Small * content = (Dwarf_Small *)x_content;

    res = dwarf_add_debug_sup(dbg,
        2,0,
        (char *)"fake-file/fake-file-name",
        sizeof(x_content),
        content,&err);
    if (res != DW_DLV_OK) {
        printf("FAIL creating .debug_sup data in dbg "
            " due to error  %s\n",
            dwarf_errmsg(err));
        exit(1);
    }
    return;
}

// Gets all the data from libdwarfp and writes
// an Elf object to outfile.c_str()
static void
write_object_file(Dwarf_P_Debug dbg,
    IRepresentation &irep,
    unsigned machine,
    unsigned endian,
    unsigned long dwbitflags,
    void *user_data)
{
    unsigned elfclass = (dwbitflags & DW_DLC_ELF_OFFSET_SIZE_64)?
        ELFCLASS64:
        ELFCLASS32;
    unsigned elfendian = (endian & DW_DLC_TARGET_LITTLEENDIAN)?
        ELFDATA2LSB:
        ELFDATA2MSB;
    int fd = 0;

    *(unsigned *)user_data = elfclass;
    dwwriter.openFile(outfile);

    ElfHeaderFromdwarf dwelfheader();
    dwelfheader.e_ident_[EI_MAG0] = ELFMAG0;
    dwelfheader.e_ident_[EI_MAG1] = ELFMAG1;
    dwelfheader.e_ident_[EI_MAG2] = ELFMAG2;
    dwelfheader.e_ident_[EI_MAG3] = ELFMAG3;
    dwelfheader.e_ident_[EI_CLASS] = elfclass;
    dwelfheader.e_ident_[EI_DATA] = elfendian;
    dwelfheader.e_ident_[EI_VERSION] = EV_CURRENT;
    dwelfheader.e_machine_ = machine;
    dwelfheader.e_type_ = ET_REL;
    dwelfheader.e_version_ = EV_CURRENT;

    if (elfclass == ELFCLASS32) {
        dw_elf32 *ehp = &dwelfheadr.e_32_;
        if (!ehp) {
            cerr << "dwarfgen: Unable to elf32_newehdr() on " <<
                outfile << endl;
            exit(1);
        }
        dwelfheader.e_ptrbytesize = 4;
        ehp->e_ident[EI_MAG0] = ELFMAG0;
        ehp->e_ident[EI_MAG1] = ELFMAG1;
        ehp->e_ident[EI_MAG2] = ELFMAG2;
        ehp->e_ident[EI_MAG3] = ELFMAG3;
        ehp->e_ident[EI_CLASS] = elfclass;
        ehp->e_ident[EI_DATA] = elfendian;
        ehp->e_ident[EI_VERSION] = EV_CURRENT;

        ASNX(ehp->e_machine,sizeof(ehp->e_machine),
            &machine,sizeof(machine));
        ASNX(ehp->e_type,sizeof(ehp->e_type),
            &dwelfheader.e_type,sizeof(elfheader.e_type));
        ASNX(ehp->e_version,sizeof(ehp->e_version),
            &dwelfheader.e_version, sizeof(elfheader.e_version));
        unsigned  strtabstroff = secstrtab.addString(".shstrtab");
        datalen = sizeof(dw_elf32_ehdr) 

    } else {
        dw_elf64_ehdr *ehp = &dwelfheadr.e_64_;
        if (!ehp) {
            cerr << "dwarfgen: Unable to elf64_newehdr() on " <<
                outfile << endl;
            exit(1);
        }
        dwelfheader.e_ptrbytesize_ = 8;
        ehp->e_ident[EI_MAG0] = ELFMAG0;
        ehp->e_ident[EI_MAG1] = ELFMAG1;
        ehp->e_ident[EI_MAG2] = ELFMAG2;
        ehp->e_ident[EI_MAG3] = ELFMAG3;
        ehp->e_ident[EI_CLASS] = elfclass;
        ehp->e_ident[EI_DATA] = elfendian;
        ehp->e_ident[EI_VERSION] = EV_CURRENT;
        ASNX(ehp->e_version,sizeof(ehp->e_version),
            &dwelfheader.e_version, sizeof(dwelfheader.e_version));
        unsigned  strtabstroff = secstrtab.addString(".shstrtab");
        // an object section with fake .text data
        // (just as an example).
        datalen = sizeof(dw_elf64_ehdr)
    }
    dwelfheader.e_datalen_ = datalen;
    create_initial_section(elf,elfclass);
    create_text_section(elf,elfclass);
    write_generated_dbg(dbg,elf,irep);
    // Create the section name string section,
    // set e_shstrndx.
    unsigned long shstrindex =
            createnamestr(strtabstroff,elfclass);
    if (dwelfheader.is_32bit(){
        dw_elf32_ehdr *ehp = &dwelfheadr.e_32_;
        ASNX(ehp->e_shstrndx,sizeof(ehp->e_shstrndx),
            &shstrindex,sizeof(shstrindex));
    } else if dwelfheader.is_32bit {
        dw_elf64_ehdr *ehp = &dwelfheadr.e_64_;
        ASNX(ehp->e_shstrndx,sizeof(ehp->e_shstrndx),
            &shstrindex,sizeof(shstrindex));
    } else {
        cerr << "dwarfgen: 32/64 not set in elf header " << endl;
        exit(1);
    }

    // Get it all written to the output file.
    calculate_all_offsets();
    finalsize == write_to_object();
    cout << " output image size in bytes " << finalsize << endl;
    
    dwwriter.closeFile();
}

// an object section with fake .text data (just as an example).
static void
write_text_section(Dwarf_Elf * elf_w,unsigned elfclass)
{
    Dwarf_Unsigned  osecnameoff = secstrtab.addString(".text");

FIXME    

    DW_Elf_Scn * scn1 =_dwarf_elf_newscn(elf_w);
    if (!scn1) {
        cerr << "dwarfgen: Unable to _dwarf_elf_newscn() on " <<
            outfile << endl;
        exit(1);
    }

    DW_Elf_Data* ed1 =_dwarf_elf_newdata(scn1);
    if (!ed1) {
        cerr << "dwarfgen: Unable to _dwarf_elf_newdata() on " << outfile
            << endl;
        exit(1);
    }
    const char *d = "data in section";
    ed1->d_buf = (void *)d;
    ed1->d_type =  DWARF_T_BYTE;
    ed1->d_size = strlen(d) +1;
    ed1->d_off = 0;
    ed1->d_align = 4;
    ed1->d_version = EV_CURRENT;
    unsigned long zero = 0;
    unsigned long one = 1;

    if (elfclass == ELFCLASS32) {
        dw_elf32_shdr * shdr = _dwarf_elf32_getshdr(scn1);
        if (!shdr) {
            cerr << "dwarfgen: Unable to elf32_getshdr() on " <<
                outfile << endl;
            exit(1);
        }
        ASNX(shdr->sh_name,sizeof(shdr->sh_name),
             &osecnameoff,sizeof(osecnameoff));
        unsigned long type = SHT_PROGBITS;
        ASNX(shdr->sh_type,sizeof(shdr->sh_type),
             &type,sizeof(type));
        ASNX(shdr->sh_flags,sizeof(shdr->sh_flags),
             &zero,sizeof(zero));
        ASNX(shdr->sh_addr,sizeof(shdr->sh_addr),
             &zero,sizeof(zero));
        //shdr->sh_offset = 0;
        ASNX(shdr->sh_offset,sizeof(shdr->sh_offset),
             &zero,sizeof(zero));
        //shdr->sh_size = 0;
        ASNX(shdr->sh_size,sizeof(shdr->sh_size),
             &zero,sizeof(zero));
        //shdr->sh_link  = 0;
        ASNX(shdr->sh_link,sizeof(shdr->sh_link),
             &zero,sizeof(zero));
        //shdr->sh_info = 0;
        ASNX(shdr->sh_info,sizeof(shdr->sh_info),
             &zero,sizeof(zero));
        //shdr->sh_addralign = 1;
        unsigned long align = 1;
        ASNX(shdr->sh_addralign,sizeof(shdr->sh_addralign),
             &align,sizeof(align));
        ASNX(shdr->sh_entsize,sizeof(shdr->sh_entsize),
             &zero,sizeof(zero));
        //shdr->sh_entsize = 0;
    } else {
        dw_elf64_shdr * shdr = _dwarf_elf64_getshdr(scn1);
        if (!shdr) {
            cerr << "dwarfgen: Unable to elf32_getshdr() on "
                << outfile << endl;
            exit(1);
        }
        ASNX(shdr->sh_name,sizeof(shdr->sh_name),
            &osecnameoff,sizeof(osecnameoff));
        unsigned long type = SHT_PROGBITS;
        ASNX(shdr->sh_type,sizeof(shdr->sh_type),
            &type,sizeof(type));
        ASNX(shdr->sh_flags,sizeof(shdr->sh_flags),
            &zero,sizeof(zero));
        ASNX(shdr->sh_addr,sizeof(shdr->sh_addr),
            &zero,sizeof(zero));
        //shdr->sh_offset = 0;
        ASNX(shdr->sh_offset,sizeof(shdr->sh_offset),
            &zero,sizeof(zero));
        //shdr->sh_size = 0;
        ASNX(shdr->sh_size,sizeof(shdr->sh_size),
            &zero,sizeof(zero));
        //shdr->sh_link  = 0;
        ASNX(shdr->sh_link,sizeof(shdr->sh_link),
            &zero,sizeof(zero));
        //shdr->sh_info = 0;
        ASNX(shdr->sh_info,sizeof(shdr->sh_info),
            &zero,sizeof(zero));
        //shdr->sh_addralign = 1;
        unsigned long align = 1;
        ASNX(shdr->sh_addralign,sizeof(shdr->sh_addralign),
            &align,sizeof(align));
        ASNX(shdr->sh_entsize,sizeof(shdr->sh_entsize),
            &zero,sizeof(zero));
        //shdr->sh_entsize = 0;
    }
}

// Take the lists of data blocks and build a single
// malloc block to contain it all pointing
// to it with the public struct DW_Elf_Data that
// _dwarf_elf_newdata() creates.
static void
InsertDataIntoElf(Dwarf_Unsigned d,Dwarf_P_Debug dbg,Dwarf_Elf *elf_i)
{
    Dwarf_Unsigned elf_section_index = 0;
    Dwarf_Unsigned length = 0;
    int res = 0;
    Dwarf_Ptr bytes = 0;

    res = dwarf_get_section_bytes_a(dbg,d,
        &elf_section_index,&length,&bytes,0);
    if (res != DW_DLV_OK) {
        cerr << "dwarfgen: get_section_bytes_a problem " << d
            << endl;
        exit(1);
    }

    DW_Elf_Scn *scn =  _dwarf_elf_getscn(elf_i,elf_section_index);
    if (!scn) {
        cerr << "dwarfgen: Unable to _dwarf_elf_getscn on disk transform # "
            << d << endl;
        exit(1);
    }

    ElfSectIndex si(elf_section_index);
    SectionFromDwarf & sfd  = FindMySection(si);

    DW_Elf_Data* ed = _dwarf_elf_newdata(scn);
    if (!ed) {
        cerr << "dwarfgen: _dwarf_elf_newdata died on transformed index "
            << d << endl;
        exit(1);
    }
    ed->d_buf = bytes;
    ed->d_type =  DWARF_T_BYTE;
    ed->d_size = length;
    ed->d_off = sfd.getNextOffset();
    sfd.setNextOffset(ed->d_off + length);
    ed->d_align = 1;
    ed->d_version = EV_CURRENT;
    cout << "Inserted " << length << " bytes into elf section index "
        << elf_section_index << endl;
}

#if 0
static string
printable_rel_type(unsigned char reltype)
{
    enum Dwarf_Rel_Type t = (enum Dwarf_Rel_Type)reltype;
    switch(t) {
    case   dwarf_drt_none:
        return "dwarf_drt_none";
    case   dwarf_drt_data_reloc:
        return "dwarf_drt_data_reloc";
    case   dwarf_drt_segment_rel:
        return "dwarf_drt_segment_rel";
    case   dwarf_drt_first_of_length_pair:
        return "dwarf_drt_first_of_length_pair";
    case   dwarf_drt_second_of_length_pair:
        return "dwarf_drt_second_of_length_pair";
    default:
        break;
    }
    return "drt-unknown (impossible case)";
}
#endif

static Dwarf_Unsigned
FindSymbolValue(ElfSymIndex symi,IRepresentation &irep)
{
    ElfSymbols & syms = irep.getElfSymbols();
    ElfSymbol & es =  syms.getElfSymbol(symi);
    Dwarf_Unsigned symv = es.getSymbolValue();
    return symv;
}

#if 0
static int
dump_bytes(const char *msg,void *val,int len)
{
    char *p = (char *)val;
    int i = 0;
    cout << msg << " ";
    for (; i < len; ++i) {
        char x = *(p+i);
        cout << IToHex(x,2) <<" ";
    }
    cout << endl;
}
#endif

/* Lets not assume that the quantities are aligned. */
static void
bitreplace(char *buf, unsigned int targlen, Dwarf_Unsigned newval,
    size_t newvalsize)
{
    (void)newvalsize;
    if (targlen == 4) {
        Dwarf_Unsigned my4 = newval;
        Dwarf_Unsigned oldval = 0;
        ASNX(&oldval,sizeof(oldval),buf,(unsigned)targlen);
        oldval += my4;
        ASNX(buf,(unsigned)targlen,&oldval,sizeof(oldval));
    } else if (targlen == 8) {
        Dwarf_Unsigned my8 = newval;
        Dwarf_Unsigned oldval = 0;
        memcpy(&oldval,buf,targlen);
        oldval += my8;
        memcpy(buf,&oldval,targlen);
    } else {
        cerr << "dwarfgen:  Relocation is length " << targlen <<
            " which we do not yet handle." << endl;
        exit(1);
    }
}

// This remembers nothing, so is dreadfully slow.
static char *
findelfbuf(Dwarf_Elf *elf_f, DW_Elf_Scn *scn,
    Dwarf_Unsigned offset,
    Dwarf_Unsigned length)
{
    DW_Elf_Data * ed = _dwarf_elf_getdata(scn,0);
    unsigned bct = 0;

    (void)elf_f;
    for ( ; ed ; ed = _dwarf_elf_getdata(scn,ed)) {
        bct++;
        if (offset >= LOFFTODWUNS(ed->d_off + ed->d_size) ) {
            continue;
        }
        if (offset < LOFFTODWUNS(ed->d_off)) {
            cerr << "dwarfgen:  Relocation at offset  " <<
                offset << " cannot be accomplished, no buffer. "
                << endl;
            exit(1);
        }
        Dwarf_Unsigned localoff = offset - ed->d_off;
        if ((localoff + length) > ed->d_size) {
            cerr << "dwarfgen:  Relocation at offset  " <<
                offset << " cannot be accomplished, size mismatch. "
                << endl;
            exit(1);
        }
        char *lclptr = reinterpret_cast<char *>(ed->d_buf) + localoff;
        return lclptr;
    }
    cerr << " Relocation at offset  " << offset  <<
        " cannot be accomplished,  past end of buffers" << endl;
    return 0;
}

static void
write_generated_dbg(Dwarf_P_Debug dbg,Dwarf_Elf * elf_w,
    IRepresentation &irep)
{
    Dwarf_Error err = 0;
    Dwarf_Unsigned sectioncount = 0;

    int res = dwarf_transform_to_disk_form_a(dbg,&sectioncount,&err);
    if (res != DW_DLV_OK) {
        if (res == DW_DLV_ERROR) {
            string msg(dwarf_errmsg(err));
            cerr << "Dwarfgen fails: " << msg << endl;
            exit(1);
        }
        /* ASSERT: rex == DW_DLV_NO_ENTRY */
        cerr << "Dwarfgen fails, some internal error " << endl;
        exit(1);
    }
    Dwarf_Unsigned d = 0;
    for (d = 0; d < sectioncount ; ++d) {
        InsertDataIntoElf(d,dbg,elf_w);
    }

    // Since we are emitting in final form sometimes, we may
    // do relocation processing here or we could (but do not)
    // emit relocation records into the object file.
    // The following is for DW_DLC_SYMBOLIC_RELOCATIONS.

    Dwarf_Unsigned reloc_sections_count = 0;
    int drd_version = 0;
    res = dwarf_get_relocation_info_count(dbg,&reloc_sections_count,
        &drd_version,&err);
    if (res != DW_DLV_OK) {
        cerr << "dwarfgen: Error getting relocation info count."
            << endl;
        exit(1);

    }
    cout << "Relocations sections count= " << reloc_sections_count <<
        " relversion=" << drd_version << endl;
    for (Dwarf_Unsigned ct = 0; ct < reloc_sections_count ; ++ct) {
        // elf_section_index is the elf index of the
        // section to be relocated, and the section number
        // in the object file which we are creating.
        // In dwarfgen we do not use this as we do not create
        // relocation sections. Here it is always zero.
        Dwarf_Unsigned elf_section_index = 0;

        // elf_section_index_link is the elf index of the
        // section  the relocations apply to, such as .debug_info.
        // An elf index, not dwsectab[] index.
        Dwarf_Unsigned elf_section_index_link = 0;

        // relocation_buffer_count is the number of relocations
        // of this section.
        Dwarf_Unsigned relocation_buffer_count = 0;
        Dwarf_Relocation_Data reld;
        res = dwarf_get_relocation_info(dbg,&elf_section_index,
            &elf_section_index_link,
            &relocation_buffer_count,
            &reld,&err);
        // elf_section_index_link
        // refers to the output section numbers, not to dwsectab.
        if (res != DW_DLV_OK) {
            cerr << "dwarfgen: Error getting relocation record " <<
                ct << "."  << endl;
            exit(1);
        }

        int dwseclink =  FindMySectionNum(elf_section_index_link);
        ElfSectIndex sitarg = dwsectab[dwseclink].getSectIndex();
        string linktarg= dwsectab[dwseclink].name_;
        Dwarf_Unsigned targsec = sitarg.getSectIndex();

        cout << "Relocs for sec=" << ct <<
            " ourlinkto="       << elf_section_index_link <<
            " linktoobjsecnum=" << targsec <<
            " name="            << linktarg <<
            " reloc-count="     << relocation_buffer_count << endl;
        DW_Elf_Scn *scn =  _dwarf_elf_getscn(elf_w,
            elf_section_index_link);
        if (!scn) {
            cerr << "dwarfgen: Unable to _dwarf_elf_getscn  # " <<
                elf_section_index_link << endl;
            exit(1);
        }

        for (Dwarf_Unsigned r = 0; r < relocation_buffer_count; ++r) {
            Dwarf_Relocation_Data rec = reld+r;
            ElfSymIndex symi(rec->drd_symbol_index);
            Dwarf_Unsigned newval = FindSymbolValue(symi,irep);
            char *buf_to_update = findelfbuf(elf_w,scn,
                rec->drd_offset,rec->drd_length);

            if (buf_to_update) {
                if (cmdoptions.showrelocdetails) {
                    cout << "Reloc "<< r <<
                        " symindex=" << rec->drd_symbol_index <<
                        " targoffset= " << IToHex(rec->drd_offset) <<
                        " newval = " << IToHex(newval) << endl;
                }
                bitreplace(buf_to_update,rec->drd_length, 
                    newval,sizeof(newval));
            } else {
                if (cmdoptions.showrelocdetails) {
                    cout << "Reloc "<< r << "does nothing"<<endl;
                }
            }
        }
    }
}

// All the data is in dwsectab or dwelfheader.
// So do the offset calculations 
// (including of all necessary offsets) then write the
// data out.
void
calculate_all_offsets(void)
{
}
Dwarf_Unsigned
write_to_object(void)
{
}

int
open_a_file(const char * name)
{
    /* Set to a file number that cannot be legal. */
    int f = -1;

    f = open(name, O_RDONLY | O_BINARY);
    return f;
}

void
close_a_file(int f)
{
    close(f);
}
