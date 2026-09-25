/*
University of Illinois/NCSA
Open Source License

Copyright (c) 2003-2017 University of Illinois at Urbana-Champaign.
All rights reserved.

Developed by:

LLVM Team

University of Illinois at Urbana-Champaign

http://llvm.org

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal with the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following
conditions:

* Redistributions of source code must retain the above
copyright notice,
this list of conditions and the following disclaimers.

* Redistributions in binary form must reproduce the above
copyright notice,
this list of conditions and the following disclaimers
in the documentation and/or other materials provided
with the distribution.

* Neither the names of the LLVM Team, University of
Illinois at
Urbana-Champaign, nor the names of its contributors
may be used to endorse or promote products derived from
this Software without specific prior written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
AND NONINFRINGEMENT.  IN NO EVENT SHALL THE CONTRIBUTORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include "dwarf.h"
#include "libdwarf.h"
#ifdef HAVE_ELF_H
#include "elf.h"
#endif /* HAVE_ELF_H */
#include "dd_safe_strcpy.h"
#include "dd_esb.h"
#include "dd_elf_naming.h"

static void
standard_bitmap_table_name(struct em_values *em,
    Dwarf_Unsigned value,
    struct esb_s*out)
{
    struct em_values *ev = em;
    unsigned set_leading_space = 0;

    esb_append(out,"(");
    for (;  ev->em_name; ev++) {
        Dwarf_Unsigned mval = value&ev->em_number;
        if (mval != ev->em_number) {
            continue;
        }
        if (set_leading_space) {
            esb_append(out," ");
        } else {
            set_leading_space = 1;
        }
        esb_append(out,ev->em_name);
    }

    if (!set_leading_space) {
        esb_append(out,"(Unknown)");
    } else {
        esb_append(out,")");
    }
}

static void
standard_table_name(struct em_values *em,
    Dwarf_Unsigned value,
    struct esb_s *out)
{
    struct em_values *ev = em;
    unsigned set_leading_space = 0;

    esb_append(out,"(");
    for (;  ev->em_name; ev++) {
        if (value != ev->em_number) {
            continue;
        }
        if (set_leading_space) {
            esb_append(out," ");
        } else {
            set_leading_space = 1;
        }
        esb_append(out,ev->em_name);
    }
    if (!set_leading_space) {
        esb_append(out,"(Unknown)");
    } else {
        esb_append(out,")");
    }
}

static struct em_values emvals[] = {
    {"EM_NONE",0}, /* No machine */
    {"EM_M32",1}, /* AT&T WE 32100 */
    {"EM_SPARC",2}, /* SPARC */
    {"EM_386",3}, /* Intel 386 */
    {"EM_68K",4}, /* Motorola 68000 */
    {"EM_88K",5}, /* Motorola 88000 */
    {"EM_IAMCU",6}, /* Intel MCU */
    {"EM_860",7}, /* Intel 80860 */
    {"EM_MIPS",8}, /* MIPS R3000 */
    {"EM_S370",9}, /* IBM System/370 */
    {"EM_MIPS_RS3_LE",10}, /* MIPS RS3000 Little-endian */
    {"EM_PARISC",15}, /* Hewlett-Packard PA-RISC */
    {"EM_VPP500",17}, /* Fujitsu VPP500 */
    {"EM_SPARC32PLUS",18}, /* Enhanced instruction set SPARC */
    {"EM_960",19}, /* Intel 80960 */
    {"EM_PPC",20}, /* PowerPC */
    {"EM_PPC64",21}, /* PowerPC64 */
    {"EM_S390",22}, /* IBM System/390 */
    {"EM_SPU",23}, /* IBM SPU/SPC */
    {"EM_V800",36}, /* NEC V800 */
    {"EM_FR20",37}, /* Fujitsu FR20 */
    {"EM_RH32",38}, /* TRW RH-32 */
    {"EM_RCE",39}, /* Motorola RCE */
    {"EM_ARM",40}, /* ARM */
    {"EM_ALPHA",41}, /* DEC Alpha */
    {"EM_SH",42}, /* Hitachi SH */
    {"EM_SPARCV9",43}, /* SPARC V9 */
    {"EM_TRICORE",44}, /* Siemens TriCore */
    {"EM_ARC",45}, /* Argonaut RISC Core */
    {"EM_H8_300",46}, /* Hitachi H8/300 */
    {"EM_H8_300H",47}, /* Hitachi H8/300H */
    {"EM_H8S",48}, /* Hitachi H8S */
    {"EM_H8_500",49}, /* Hitachi H8/500 */
    {"EM_IA_64",50}, /* Intel IA-64 processor architecture */
    {"EM_MIPS_X",51}, /* Stanford MIPS-X */
    {"EM_COLDFIRE",52}, /* Motorola ColdFire */
    {"EM_68HC12",53}, /* Motorola M68HC12 */
    {"EM_MMA",54}, /* Fujitsu MMA Multimedia Accelerator */
    {"EM_PCP",55}, /* Siemens PCP */
    {"EM_NCPU",56}, /* Sony nCPU embedded RISC processor */
    {"EM_NDR1",57}, /* Denso NDR1 microprocessor */
    {"EM_STARCORE",58}, /* Motorola Star*Core processor */
    {"EM_ME16",59}, /* Toyota ME16 processor */
    {"EM_ST100",60}, /* STMicroelectronics ST100 processor */
    {"EM_TINYJ",61},
    /* Advanced Logic Corp. TinyJ embedded processor family */

    {"EM_X86_64",62}, /* AMD x86-64 architecture */
    {"EM_PDSP",63}, /* Sony DSP Processor */
    {"EM_PDP10",64}, /* Digital Equipment Corp. PDP-10 */
    {"EM_PDP11",65}, /* Digital Equipment Corp. PDP-11 */
    {"EM_FX66",66}, /* Siemens FX66 microcontroller */
    {"EM_ST9PLUS",67},
    /* STMicroelectronics ST9+ 8/16 bit microcontroller */

    {"EM_ST7",68},
    /* STMicroelectronics ST7 8-bit microcontroller */

    {"EM_68HC16",69}, /* Motorola MC68HC16 Microcontroller */
    {"EM_68HC11",70}, /* Motorola MC68HC11 Microcontroller */
    {"EM_68HC08",71}, /* Motorola MC68HC08 Microcontroller */
    {"EM_68HC05",72}, /* Motorola MC68HC05 Microcontroller */
    {"EM_SVX",73}, /* Silicon Graphics SVx */
    {"EM_ST19",74},
    /* STMicroelectronics ST19 8-bit microcontroller */

    {"EM_VAX",75}, /* Digital VAX */
    {"EM_CRIS",76},
    /* Axis Communications 32-bit embedded processor */

    {"EM_JAVELIN",77},
    /* Infineon Technologies 32-bit embedded processor */

    {"EM_FIREPATH",78}, /* Element 14 64-bit DSP Processor */
    {"EM_ZSP",79}, /* LSI Logic 16-bit DSP Processor */
    {"EM_MMIX",80}, /* Donald Knuth's educational 64-bit processor */
    {"EM_HUANY",81},
    /* Harvard University machine-independent object files */

    {"EM_PRISM",82}, /* SiTera Prism */
    {"EM_AVR",83}, /* Atmel AVR 8-bit microcontroller */
    {"EM_FR30",84}, /* Fujitsu FR30 */
    {"EM_D10V",85}, /* Mitsubishi D10V */
    {"EM_D30V",86}, /* Mitsubishi D30V */
    {"EM_V850",87}, /* NEC v850 */
    {"EM_M32R",88}, /* Mitsubishi M32R */
    {"EM_MN10300",89}, /* Matsushita MN10300 */
    {"EM_MN10200",90}, /* Matsushita MN10200 */
    {"EM_PJ",91}, /* picoJava */
    {"EM_OPENRISC",92}, /* OpenRISC 32-bit embedded processor */
    {"EM_ARC_COMPACT",93}, /* ARC International ARCompact processor */
    {"EM_XTENSA",94}, /* Tensilica Xtensa Architecture */
    {"EM_VIDEOCORE",95}, /* Alphamosaic VideoCore processor */
    {"EM_TMM_GPP",96},
    /* Thompson Multimedia General Purpose Processor */

    {"EM_NS32K",97}, /* National Semiconductor 32000 series */
    {"EM_TPC",98}, /* Tenor Network TPC processor */
    {"EM_SNP1K",99}, /* Trebia SNP 1000 processor */
    {"EM_ST200",100}, /* STMicroelectronics (www.st.com) ST200 */
    {"EM_IP2K",101}, /* Ubicom IP2xxx microcontroller family */
    {"EM_MAX",102}, /* MAX Processor */
    {"EM_CR",103},
    /* National Semiconductor CompactRISC microprocessor */

    {"EM_F2MC16",104}, /* Fujitsu F2MC16 */
    {"EM_MSP430",105},
    /* Texas Instruments embedded microcontroller msp430 */

    {"EM_BLACKFIN",106}, /* Analog Devices Blackfin (DSP) processor */
    {"EM_SE_C33",107}, /* S1C33 Family of Seiko Epson processors */
    {"EM_SEP",108}, /* Sharp embedded microprocessor */
    {"EM_ARCA",109}, /* Arca RISC Microprocessor */
    {"EM_UNICORE",110},
    /* Microprocessor series from PKU-Unity Ltd. and MPRC */

    {"EM_EXCESS",111},
    /* eXcess: 16/32/64-bit configurable embedded CPU */

    {"EM_DXP",112},
    /* Icera Semiconductor Inc. Deep Execution Processor */

    {"EM_ALTERA_NIOS2",113}, /* Altera Nios II soft-core processor */
    {"EM_CRX",114}, /* National Semiconductor CompactRISC CRX */
    {"EM_XGATE",115}, /* Motorola XGATE embedded processor */
    {"EM_C166",116}, /* Infineon C16x/XC16x processor */
    {"EM_M16C",117}, /* Renesas M16C series microprocessors */
    {"EM_DSPIC30F",118},
    /* Microchip Technology dsPIC30F Digital Signal */

    {"EM_CE",119}, /* Freescale Communication Engine RISC core */
    {"EM_M32C",120}, /* Renesas M32C series microprocessors */
    {"EM_TSK3000",131}, /* Altium TSK3000 core */
    {"EM_RS08",132}, /* Freescale RS08 embedded processor */
    {"EM_SHARC",133}, /* Analog Devices SHARC family of 32-bit DSP */
    {"EM_ECOG2",134}, /* Cyan Technology eCOG2 microprocessor */
    {"EM_SCORE7",135}, /* Sunplus S+core7 RISC processor */
    {"EM_DSP24",136}, /* New Japan Radio (NJR) 24-bit DSP Processor */
    {"EM_VIDEOCORE3",137}, /* Broadcom VideoCore III processor */
    {"EM_LATTICEMICO32",138},
    /* RISC processor for Lattice FPGA architecture */

    {"EM_SE_C17",139}, /* Seiko Epson C17 family */
    {"EM_TI_C6000",140},
    /* The Texas Instruments TMS320C6000 DSP family */

    {"EM_TI_C2000",141},
    /* The Texas Instruments TMS320C2000 DSP family */

    {"EM_TI_C5500",142},
    /* The Texas Instruments TMS320C55x DSP family */

    {"EM_MMDSP_PLUS",160},
    /* STMicroelectronics 64bit VLIW Data Signal Processor */

    {"EM_CYPRESS_M8C",161}, /* Cypress M8C microprocessor */
    {"EM_R32C",162}, /* Renesas R32C series microprocessors */
    {"EM_TRIMEDIA",163},
    /* NXP Semiconductors TriMedia architecture family */

    {"EM_HEXAGON",164}, /* Qualcomm Hexagon processor */
    {"EM_8051",165}, /* Intel 8051 and variants */
    {"EM_STXP7X",166},
    /* STMicroelectronics STxP7x family of configurable */

    {"EM_NDS32",167},
    /* Andes Technology compact code size embedded RISC */

    {"EM_ECOG1",168}, /* Cyan Technology eCOG1X family */
    {"EM_ECOG1X",168}, /* Cyan Technology eCOG1X family */
    {"EM_MAXQ30",169},
    /* Dallas Semiconductor MAXQ30 Core Micro-controllers */

    {"EM_XIMO16",170},
    /* New Japan Radio (NJR) 16-bit DSP Processor */

    {"EM_MANIK",171}, /* M2000 Reconfigurable RISC Microprocessor */
    {"EM_CRAYNV2",172}, /* Cray Inc. NV2 vector architecture */
    {"EM_RX",173}, /* Renesas RX family */
    {"EM_METAG",174}, /* Imagination Technologies META processor */
    {"EM_MCST_ELBRUS",175},
    /* MCST Elbrus general purpose hardware architecture */

    {"EM_ECOG16",176}, /* Cyan Technology eCOG16 family */
    {"EM_CR16",177},
    /* National Semiconductor CompactRISC CR16 16-bit */

    {"EM_ETPU",178}, /* Freescale Extended Time Processing Unit */
    {"EM_SLE9X",179}, /* Infineon Technologies SLE9X core */
    {"EM_L10M",180}, /* Intel L10M */
    {"EM_K10M",181}, /* Intel K10M */
    {"EM_AARCH64",183}, /* ARM AArch64 */
    {"EM_AVR32",185},
    /* Atmel Corporation 32-bit microprocessor family */

    {"EM_STM8",186},
    /* STMicroeletronics STM8 8-bit microcontroller */

    {"EM_TILE64",187},
    /* Tilera TILE64 multicore architecture family */

    {"EM_TILEPRO",188},
    /* Tilera TILEPro multicore architecture family */

    {"EM_CUDA",190}, /* NVIDIA CUDA architecture */
    {"EM_TILEGX",191},
    /* Tilera TILE-Gx multicore architecture family */

    {"EM_CLOUDSHIELD",192}, /* CloudShield architecture family */
    {"EM_COREA_1ST",193},
    /* KIPO-KAIST Core-A 1st generation processor family */

    {"EM_COREA_2ND",194},
    /* KIPO-KAIST Core-A 2nd generation processor family */

    {"EM_ARC_COMPACT2",195}, /* Synopsys ARCompact V2 */
    {"EM_OPEN8",196}, /* Open8 8-bit RISC soft processor core */
    {"EM_RL78",197}, /* Renesas RL78 family */
    {"EM_VIDEOCORE5",198}, /* Broadcom VideoCore V processor */
    {"EM_78KOR",199}, /* Renesas 78KOR family */
    {"EM_56800EX",200},
    /* Freescale 56800EX Digital Signal Controller (DSC) */

    {"EM_BA1",201}, /* Beyond BA1 CPU architecture */
    {"EM_BA2",202}, /* Beyond BA2 CPU architecture */
    {"EM_XCORE",203}, /* XMOS xCORE processor family */
    {"EM_MCHP_PIC",204}, /* Microchip 8-bit PIC(r) family */
    {"EM_INTEL205",205}, /* Reserved by Intel */
    {"EM_INTEL206",206}, /* Reserved by Intel */
    {"EM_INTEL207",207}, /* Reserved by Intel */
    {"EM_INTEL208",208}, /* Reserved by Intel */
    {"EM_INTEL209",209}, /* Reserved by Intel */
    {"EM_KM32",210}, /* KM211 KM32 32-bit processor */
    {"EM_KMX32",211}, /* KM211 KMX32 32-bit processor */
    {"EM_KMX16",212}, /* KM211 KMX16 16-bit processor */
    {"EM_KMX8",213}, /* KM211 KMX8 8-bit processor */
    {"EM_KVARC",214}, /* KM211 KVARC processor */
    {"EM_CDP",215}, /* Paneve CDP architecture family */
    {"EM_COGE",216}, /* Cognitive Smart Memory Processor */
    {"EM_COOL",217}, /* iCelero CoolEngine */
    {"EM_NORC",218}, /* Nanoradio Optimized RISC */
    {"EM_CSR_KALIMBA",219}, /* CSR Kalimba architecture family */
    {"EM_AMDGPU",224}, /* AMD GPU architecture */
    {"EM_RISCV",243}, /* RISC-V */
    {"EM_LANAI",244}, /* Lanai 32-bit processor */
    {"EM_CSKY",252}, /* C-SKY */
    {"EM_LOONGARCH",258}, /* LoongArch */
    {"EM_WEBASSEMBLY",0x4157}, /* WebAssembly architecture */
    {0,0}
};

void
dd_get_elf_machine_name(Dwarf_Unsigned value,
    struct esb_s *out)
{
    struct em_values *ev = &emvals[0];

    for (;  ev->em_name; ev++) {
        if (value > ev->em_number) {
            continue;
        }
        if (value < ev->em_number) {
            break;
        }
        esb_append(out,ev->em_name);
        return;
    }
    esb_append(out,"Unknown em_machine");
}

static struct em_values dtvals[] = {
    {"DT_NULL",0},         /* Marks end of dynamic array.*/
    {"DT_NEEDED",1},       /* String table offset of needed library.*/
    {"DT_PLTRELSZ",2},     /* Size of relocation entries in PLT.*/
    {"DT_PLTGOT",3},       /* Address associated with linkage table.*/
    {"DT_HASH",4},         /* Address of symbolic hash table.*/
    {"DT_STRTAB",5},       /* Address of dynamic string table.*/
    {"DT_SYMTAB",6},       /* Address of dynamic symbol table.*/
    {"DT_RELA",7},
    /* Address of relocation table (Rela entries).*/

    {"DT_RELASZ",8},       /* Size of Rela relocation table.*/
    {"DT_RELAENT",9},      /* Size of a Rela relocation entry.*/
    {"DT_STRSZ",10},       /* Total size of the string table.*/
    {"DT_SYMENT",11},      /* Size of a symbol table entry.*/
    {"DT_INIT",12},        /* Address of initialization function.*/
    {"DT_FINI",13},        /* Address of termination function.*/
    {"DT_SONAME",14},
    /* String table offset of a shared objects name.*/

    {"DT_RPATH",15},
    /* String table offset of library search path.*/

    {"DT_SYMBOLIC",16},
    /* Changes symbol resolution algorithm.*/

    {"DT_REL",17},
    /* Address of relocation table (Rel entries).*/

    {"DT_RELSZ",18},       /* Size of Rel relocation table.*/
    {"DT_RELENT",19},      /* Size of a Rel relocation entry.*/
    {"DT_PLTREL",20},
    /* Type of relocation entry used for linking.*/

    {"DT_DEBUG",21},       /* Reserved for debugger.*/
    {"DT_TEXTREL",22},
    /* Relocations exist for non-writable segments.*/

    {"DT_JMPREL",23},
    /* Address of relocations associated with PLT.*/

    {"DT_BIND_NOW",24},
    /* Process all relocations before execution.*/

    {"DT_INIT_ARRAY",25},
    /* Pointer to array of initialization functions.*/

    {"DT_FINI_ARRAY",26},
    /* Pointer to array of termination functions.*/

    {"DT_INIT_ARRAYSZ",27},/* Size of DT_INIT_ARRAY.*/
    {"DT_FINI_ARRAYSZ",28},/* Size of DT_FINI_ARRAY.*/

    {"DT_RUNPATH",29},
    /* String table offset of lib search path.*/

    {"DT_FLAGS",30},       /* Flags.*/
    {"DT_ENCODING",32},
    /* Values from here to DT_LOOS follow the rules*/

    {"DT_PREINIT_ARRAY",32},
    /* Pointer to array of preinit functions.*/

    {"DT_PREINIT_ARRAYSZ",33},/* Size of the DT_PREINIT_ARRAY array.*/
    {"DT_LOOS",0x60000000},  /* Start of environment specific tags.*/
    {"DT_HIOS",0x6FFFFFFF},  /* End of environment specific tags.*/
    {"DT_LOPROC",0x70000000},/* Start of processor specific tags.*/
    {"DT_HIPROC",0x7FFFFFFF},/* End of processor specific tags.*/
    {"DT_ANDROID_REL",0x6000000F},
    {"DT_ANDROID_RELSZ",0x60000010},
    {"DT_ANDROID_RELA",0x60000011},
    {"DT_ANDROID_RELASZ",0x60000012},
    {"DT_GNU_HASH",0x6FFFFEF5},/* Reference to the GNU hash table.*/
    {"DT_RELCOUNT",0x6FFFFFFA},   /* ELF32_Rel count.*/
    {"DT_FLAGS_1",0X6FFFFFFB},   /* Flags_1.*/
    {"DT_VERSYM",0x6FFFFFF0},
    /* The address of .gnu.version section.*/

    {"DT_VERDEF",0X6FFFFFFC},
    /* The address of the version definition table.*/

    {"DT_VERDEFNUM",0X6FFFFFFD},
    {"DT_VERNEEDNUM",0X6FFFFFFF},
    {"DT_HEXAGON_SYMSZ",0x70000000},
    {"DT_HEXAGON_VER",0x70000001},
    {"DT_HEXAGON_PLT",0x70000002},
    {"DT_MIPS_RLD_VERSION",0x70000001},
    /* 32 bit version number for runtime*/

    {"DT_MIPS_TIME_STAMP",0x70000002},    /* Time stamp.*/
    {"DT_MIPS_ICHECKSUM",0x70000003},
    /* Checksum of external strings*/

    {"DT_MIPS_IVERSION",0x70000004},
    /* Index of version string*/

    {"DT_MIPS_FLAGS",0x70000005},         /* 32 bits of flags.*/
    {"DT_MIPS_BASE_ADDRESS",0x70000006},
    /* Base address of the segment.*/

    {"DT_MIPS_MSYM",0x70000007},    /* Address of .msym section.*/
    {"DT_MIPS_CONFLICT",0x70000008},
    /* Address of .conflict section.*/

    {"DT_MIPS_LIBLIST",0x70000009}, /* Address of .liblist section.*/
    {"DT_MIPS_LOCAL_GOTNO",0x7000000a},
    /* Number of local global offset*/

    {"DT_MIPS_CONFLICTNO",0x7000000b},    /* Number of entries*/
    {"DT_MIPS_LIBLISTNO",0x70000010},     /* Number of entries*/
    {"DT_MIPS_SYMTABNO",0x70000011},      /* Number of entries*/
    {"DT_MIPS_UNREFEXTNO",0x70000012},
    /* Index of first external dynamic symbol*/

    {"DT_MIPS_GOTSYM",0x70000013}, /* Index of first dynamic symbol*/
    {"DT_MIPS_HIPAGENO",0x70000014}, /* Number of page table entries*/
    {"DT_MIPS_RLD_MAP",0x70000016},
    /* Address of run time loader map,*/

    {"DT_MIPS_DELTA_CLASS",0x70000017},
    /* Delta C++ class definition.*/

    {"DT_MIPS_DELTA_CLASS_NO",0x70000018},/* Number of entries*/
    {"DT_MIPS_DELTA_INSTANCE",0x70000019},
    /* Delta C++ class instances.*/

    {"DT_MIPS_DELTA_INSTANCE_NO",0x7000001A}, /* Number of entries*/
    {"DT_MIPS_DELTA_RELOC",0x7000001B},       /* Delta relocations.*/
    {"DT_MIPS_DELTA_RELOC_NO",0x7000001C},    /* Number of entries*/
    {"DT_MIPS_DELTA_SYM",0x7000001D}, /* Delta symbols that Delta*/
    {"DT_MIPS_DELTA_SYM_NO",0x7000001E},   /* Number of entries*/
    {"DT_MIPS_DELTA_CLASSSYM",0x70000020},
    /* Delta symbols that hold*/

    {"DT_MIPS_DELTA_CLASSSYM_NO",0x70000021}, /* Number of entries*/
    {"DT_MIPS_CXX_FLAGS",0x70000022},
    /* Flags indicating information*/

    {"DT_MIPS_PIXIE_INIT",0x70000023}, /* Pixie information.*/
    {"DT_MIPS_SYMBOL_LIB",0x70000024}, /* Address of .MIPS.symlib*/
    {"DT_MIPS_LOCALPAGE_GOTIDX",0x70000025},
    /* The GOT index of the first PTE*/

    {"DT_MIPS_LOCAL_GOTIDX",0x70000026},
    /* The GOT index of the first PTE*/

    {"DT_MIPS_HIDDEN_GOTIDX",0x70000027},
    /* The GOT index of the first PTE*/

    {"DT_MIPS_PROTECTED_GOTIDX",0x70000028},
    /* The GOT index of the first PTE*/

    {"DT_MIPS_OPTIONS",0x70000029},   /* Address of `.MIPS.options'.*/
    {"DT_MIPS_INTERFACE",0x7000002A}, /* Address of `.interface'.*/
    {"DT_MIPS_DYNSTR_ALIGN",0x7000002B},         /* Unknown.*/
    {"DT_MIPS_INTERFACE_SIZE",0x7000002C},
    /* Size of the .interface section.*/

    {"DT_MIPS_RLD_TEXT_RESOLVE_ADDR",0x7000002D},
    /* Size of rld_text_resolve*/

    {"DT_MIPS_PERF_SUFFIX",0x7000002E},
    /* Default suffix of DSO to be added*/

    {"DT_MIPS_COMPACT_SIZE",0x7000002F},
    /* Size of compact relocation*/

    {"DT_MIPS_GP_VALUE",0x70000030}, /* GP value for auxiliary GOTs.*/
    {"DT_MIPS_AUX_DYNAMIC",0x70000031},
    /* Address of auxiliary .dynamic.*/

    {"DT_MIPS_PLTGOT",0x70000032},
    /* Address of the base of the PLTGOT.*/

    {"DT_MIPS_RWPLT",0x70000034},       /* Points to the base*/
    {"DT_MIPS_RLD_MAP_REL",0x70000035},
    /* Relative offset of run time loader*/

    {"DT_AUXILIARY",0x7FFFFFFD},
    /* Shared object to load before self*/

    {"DT_USED",0x7ffffffe},
    {"DT_FILTER",0x7FFFFFFF },
    /* Shared object to get values from*/

    {"DT_VALRNGLO",0x6ffffd00},
    {"DT_GNU_PRELINKED",0x6ffffdf5},
    {"DT_GNU_CONFLICTSZ",0x6ffffdf6},
    {"DT_GNU_LIBLISTSZ",0x6ffffdf7},
    {"DT_CHECKSUM",0x6ffffdf8},
    {"DT_PLTPADSZ",0x6ffffdf9},
    {"DT_MOVEENT",0x6ffffdfa},
    {"DT_MOVESZ",0x6ffffdfb},
    {"DT_FEATURE",0x6ffffdfc},
    {"DT_POSFLAG_1",0x6ffffdfd},
    {"DT_SYMINSZ",0x6ffffdfe},
    {"DT_SYMINENT",0x6ffffdff},
    {"DT_VALRNGHI",0x6ffffdff},

    {"DT_ADDRRNGLO",0x6ffffe00},
    {"DT_TLSDESC_PLT",0x6ffffef6},
    {"DT_TLSDESC_GOT",0x6ffffef7},
    {"DT_GNU_CONFLICT",0x6ffffef8},
    {"DT_GNU_LIBLIST",0x6ffffef9},
    {"DT_CONFIG",0x6ffffefa},
    {"DT_DEPAUDIT",0x6ffffefb},
    {"DT_AUDIT",0x6ffffefc},
    {"DT_PLTPAD",0x6ffffefd},
    {"DT_MOVETAB",0x6ffffefe},
    {"DT_SYMINFO",0x6ffffeff},
    {"DT_ADDRRNGHI",0x6ffffeff},
    {"DT_RELACOUNT",0x6ffffff9},
    {"DT_VERNEED",0x6ffffffe},
    {"DT_PPC_GOT",0x70000000},
    {"DT_PPC_OPT",0x70000001},
    {"DT_PPC64_GLINK",0x70000000},

    {"DT_PPC64_OPD",0x70000001},
    {"DT_PPC64_OPDSZ",0x70000002},

    {"DT_PPC64_OPT",0x70000003},

    {"DT_SPARC_REGISTER",0x70000001},
    {0,0}
};

/*  Table values not sorted by em_number,
    will have dups */
void
dd_get_elf_dynamic_table_name(Dwarf_Unsigned value,
    struct esb_s *out)
{
    struct em_values *ev = &dtvals[0];

    standard_table_name(ev, value,out);
    return;
}

#if 0
static struct em_values pt_vals[] = {
    {"PT_NULL",0},      /* Unused segment. */
    {"PT_LOAD",1},      /* Loadable segment. */
    {"PT_DYNAMIC",2},   /* Dynamic linking information. */
    {"PT_INTERP",3},    /* Interpreter pathname. */
    {"PT_NOTE",4},      /* Auxiliary information. */
    {"PT_SHLIB",5},     /* Reserved. */
    {"PT_PHDR",6},      /* The program header table itself. */
    {"PT_TLS",7},       /* The thread-local storage template. */
    {"PT_LOOS",0x60000000},
    /* Lowest operating system-specific pt entry type. */

    {"PT_HIOS",0x6fffffff},
    /* Highest operating system-specific pt entry type. */

    {"PT_LOPROC",0x70000000},
    /* Lowest processor-specific program hdr entry type. */

    {"PT_HIPROC",0x7fffffff},
    /* Highest processor-specific program hdr entry type. */

    /*  x86-64 program header types.
        These all contain stack unwind tables. */
    {"PT_GNU_EH_FRAME",0x6474e550},
    {"PT_SUNW_EH_FRAME",0x6474e550},
    {"PT_SUNW_UNWIND",0x6464e550},

    {"PT_GNU_STACK",0x6474e551}, /* Indicates stack executability. */
    {"PT_GNU_RELRO",0x6474e552}, /* Read-only after relocation. */

    {"PT_OPENBSD_RANDOMIZE",0x65a3dbe6}, /* Fill with random data. */
    {"PT_OPENBSD_WXNEEDED",0x65a3dbe7},
    /* Program does W^X violations. */
    {"PT_PAX_FLAGS",0x65041580 }, /* To be replaced by XATTR_PAX?*/

    {"PT_OPENBSD_BOOTDATA",0x65a41be6},
    /* Section for boot arguments. */

    /*   ARM program header types. */
    {"PT_ARM_ARCHEXT",0x70000000},
    /* Platform architecture compatibility info */
    /* These all contain stack unwind tables. */

    {"PT_ARM_EXIDX",  0x70000001},
    {"PT_ARM_UNWIND", 0x70000001},

    /* MIPS program header types. */
    {"PT_MIPS_REGINFO", 0x70000000}, /* Register usage information. */
    {"PT_MIPS_RTPROC",  0x70000001},  /* Runtime procedure table. */
    {"PT_MIPS_OPTIONS", 0x70000002}, /* Options segment. */
    {"PT_MIPS_ABIFLAGS",0x70000003}, /* Abiflags segment. */
    {"PT_WEBASSEMBLY_FUNCTIONS",PT_LOPROC + 0},
    /* Function definitions. */
    {0,0}
};

/*  There are duplicates here and not all in order.  */
void
dd_get_elf_program_header_type_name(Dwarf_Unsigned value,
    struct esb_s *out)
{
    struct em_values *ev = &pt_vals[0];

    standard_table_name(ev, value,out);
}
#endif /* 0 */

static struct em_values shf_vals[] = {
    /* Section data should be writable during execution. */
    {"SHF_WRITE",0x1},

    /* Section occupies memory during program execution. */
    {"SHF_ALLOC",0x2},

    /* Section contains executable machine instructions. */
    {"SHF_EXECINSTR",0x4},

    /* The data in this section may be merged. */
    {"SHF_MERGE",0x10},

    /* The data in this section is null-terminated strings. */
    {"SHF_STRINGS",0x20},

    /* A field in this section holds a section header table index. */
    {"SHF_INFO_LINK",0x40U},

    /* Adds special ordering requirements for link editors. */
    {"SHF_LINK_ORDER",0x80U},

    /*  This section requires special OS-specific
        processing to avoid incorrect
        behavior. */
    {"SHF_OS_NONCONFORMING",0x100U},

    /* This section is a member of a section group. */
    {"SHF_GROUP",0x200U},

    /* This section holds Thread-Local Storage. */
    {"SHF_TLS",0x400U},

    /* Identifies a section containing compressed data. */
    {"SHF_COMPRESSED",0x800U},

    /*  This section is excluded from the final
        executable or shared library. */
    {"SHF_EXCLUDE",0x80000000U},

    /* Start of target-specific flags. */

    {"SHF_MASKOS",0x0ff00000},

    /* Bits indicating processor-specific flags. */
    {"SHF_MASKPROC",0xf0000000},

    /*  All sections with the "d" flag are grouped together
        by the linker to form the data section and the dp
        register is set to the start of the section by
        the boot code. */
    {"XCORE_SHF_DP_SECTION",0x10000000},

    /*  All sections with the "c" flag are grouped together
        by the linker to form the constant pool and the cp
        register is set to the start of the constant pool by
        the boot code. */
    {"XCORE_SHF_CP_SECTION",0x20000000},

    /*  If an object file section does not have this flag
        set, then it may not hold more than 2GB and can be
        freely referred to in objects using smaller code j
        models. Otherwise, only objects using larger code models
        can refer to them.  For example, a medium code model object
        can refer to data in a section that sets this flag besides
        being able to refer to data in a section that does not
        set it; likewise, a small code model object can refer
        only to code in a section that does not set this flag. */
    {"SHF_X86_64_LARGE", 0x10000000},

    /*  All sections with the GPREL flag are grouped into a
        global data area for faster accesses. */
    {"SHF_HEX_GPREL",0x10000000},

    /* Section contains text/data which may be replicated in
    other sections.  Linker must retain only one copy. */
    {"SHF_MIPS_NODUPES",0x01000000},

    /* Linker must generate implicit hidden weak names. */
    {"SHF_MIPS_NAMES",0x02000000},

    /* Section data local to process. */
    {"SHF_MIPS_LOCAL",0x04000000},

    /* Do not strip this section.  */
    {"SHF_MIPS_NOSTRIP",0x08000000},

    /* Section must be part of global data area. */
    {"SHF_MIPS_GPREL",0x10000000},

    /* This section should be merged. */
    {"SHF_MIPS_MERGE",0x20000000},

    /* Address size to be inferred from section entry size. */
    {"SHF_MIPS_ADDR",0x40000000},

    /* Section data is string data by default. */
    {"SHF_MIPS_STRING",0x80000000},

    /* Make code section unreadable when in execute-only mode.  */
    {"SHF_ARM_PURECODE",0x20000000},
    {0,0}
};
/*  There are duplicates here and not all in order.  */
void
dd_get_elf_section_header_flag_names(Dwarf_Unsigned value,
    struct esb_s *out)
{
    struct em_values *ev = &shf_vals[0];

    standard_bitmap_table_name(ev, value,out);
}

static struct em_values sht_vals[] = {
    {"SHT_NULL",0},  /* No associated section (inactive entry). */
    {"SHT_PROGBITS",1},     /* Program-defined contents. */
    {"SHT_SYMTAB",2},       /* Symbol table. */
    {"SHT_STRTAB",3},       /* String table. */
    {"SHT_RELA",4},  /* Relocation entries; explicit addends. */
    {"SHT_HASH",5},         /* Symbol hash table. */
    {"SHT_DYNAMIC",6},      /* Information for dynamic linking. */
    {"SHT_NOTE",7},         /* Information about the file. */
    {"SHT_NOBITS",8},       /* Data occupies no space in the file. */
    {"SHT_REL",9},    /* Relocation entries; no explicit addends. */
    {"SHT_SHLIB",10},       /* Reserved. */
    {"SHT_DYNSYM",11},      /* Symbol table. */
    {"SHT_INIT_ARRAY",14}, /* Pointers to initialization functions. */
    {"SHT_FINI_ARRAY",15},  /* Pointers to termination functions. */
    {"SHT_PREINIT_ARRAY",16},/* Pointers to pre-init functions. */
    {"SHT_GROUP",17},        /* Section group. */
    {"SHT_SYMTAB_SHNDX",18}, /* Indices for SHN_XINDEX entries. */
    {"SHT_LOOS",0x60000000},
    /* Lowest operating system-specific type. */

    /*  Android packed relocation section types.
        https://android.googlesource.com/platform/
        bionic/+/6f12bfece5dcc01325e0abba56a46b1bcf991c69/tools/
        relocation_packer/src/elf_file.cc#37
    */
    {"SHT_ANDROID_REL",0x60000001},
    {"SHT_ANDROID_RELA",0x60000002},
    {"SHT_LLVM_ODRTAB",0x6fff4c00},    /* LLVM ODR table. */
    {"SHT_GNU_ATTRIBUTES",0x6ffffff5}, /* Object attributes. */
    {"SHT_GNU_HASH",0x6ffffff6},       /* GNU-style hash table. */
    {"SHT_GNU_verdef",0x6ffffffd},     /* GNU version definitions. */
    {"SHT_GNU_verneed",0x6ffffffe},    /* GNU version references. */
    {"SHT_GNU_versym",0x6fffffff},   /* GNU symbol versions table. */
    {"SHT_HIOS",0x6fffffff},
    /* Highest operating system-specific type. */

    {"SHT_LOPROC",0x70000000},
    /* Lowest processor arch-specific type. */

    /* Fixme: All this is duplicated in MCSectionELF. Why?? */
    /* Exception Index table */
    {"SHT_ARM_EXIDX",0x70000001U},
    /* BPABI DLL dynamic linking pre-emption map */
    {"SHT_ARM_PREEMPTMAP",0x70000002U},
    /*  Object file compatibility attributes */
    {"SHT_ARM_ATTRIBUTES",0x70000003U},
    {"SHT_ARM_DEBUGOVERLAY",0x70000004U},
    {"SHT_ARM_OVERLAYSECTION",0x70000005U},
    {"SHT_HEX_ORDERED",0x70000000},
        /* Link editor is to sort the entries in
        this section based on their sizes */

    {"SHT_X86_64_UNWIND",0x70000001}, /* Unwind information */

    {"SHT_MIPS_REGINFO",0x70000006}, /* Register usage information */
    {"SHT_MIPS_OPTIONS",0x7000000d},  /* General options */
    {"SHT_MIPS_DWARF",0x7000001e},    /* DWARF debugging section. */
    {"SHT_MIPS_ABIFLAGS",0x7000002a}, /* ABI information. */

    {"SHT_HIPROC",0x7fffffff},
    /* Highest processor arch-specific type. */

    {"SHT_LOUSER",0x80000000},
    /* Lowest type reserved for applications. */

    {"SHT_HIUSER",0xffffffff},
    /* Highest type reserved for applications. */

    {0,0}
};

/* Not fully in order. Few duplicates. */

void
dd_get_elf_section_header_st_type(Dwarf_Unsigned value,
    struct esb_s *out)
{
    struct em_values *ev = &sht_vals[0];

    standard_table_name(ev, value,out);
    return;
}

static struct em_values sto_vals[] = {
    {"STV_DEFAULT",0},
    {"STV_INTERNAL",1},
    {"STV_HIDDEN",2},
    {"STV_PROTECTED",3},

    /* Symbol whose definition is optional */
    {"STO_MIPS_OPTIONAL",0x04},
    {"STO_MIPS_PLT",0x08},/* PLT entry related dynamic table record */

    /* PIC func in an object mixes PIC/non-PIC */
    {"STO_MIPS_PIC",0x20},
    {"STO_MIPS_MICROMIPS",0x80}, /* MIPS Specific ISA for MicroMips */
    {"STO_MIPS_MIPS16",0xf0},    /* MIPS Specific ISA for Mips16 */
    {"STO_PPC64_LOCAL_BIT",5},
    {"STO_PPC64_LOCAL_MASK",(7 << 5)},
    {0,0}

};
void
dd_get_elf_symbol_sto_type(Dwarf_Unsigned value, 
    struct esb_s *out)
{
    struct em_values *ev = &sto_vals[0];
    standard_table_name(ev, value,out);
}
static struct em_values shn_vals[] = {
    /* Undefined, missing, irrelevant, or meaningless */
    {"SHN_UNDEF",0},

    {"SHN_LORESERVE",0xff00},
    /* Lowest reserved index */

    {"SHN_LOPROC",0xff00},
    /* Lowest processor-specific index */

    {"SHN_HIPROC",0xff1f},
    /* Highest processor-specific index */

    {"SHN_LOOS",0xff20},
    /* Lowest operating system-specific index */

    {"SHN_HIOS",0xff3f},
    /* Highest operating system-specific index */

    {"SHN_ABS",0xfff1},
    /* Symbol has absolute value; does not need relocation */

    {"SHN_COMMON",0xfff2},
    /* FORTRAN COMMON or C external global variables */

    {"SHN_XINDEX",0xffff},
    /*  The actual index is too high, the actual value
        is in section with section type SHT_SYTMTAB_SHNDX */

    {"SHN_HIRESERVE",0xffff},  /* Highest reserved index */
    {"SHN_HEXAGON_SCOMMON",0xff00},   /* Other access sizes */
    {"SHN_HEXAGON_SCOMMON_1",0xff01}, /* Byte-sized access */
    {"SHN_HEXAGON_SCOMMON_2",0xff02}, /* Half-word-sized access */
    {"SHN_HEXAGON_SCOMMON_4",0xff03}, /* Word-sized access */
    {"SHN_HEXAGON_SCOMMON_8",0xff04}, /* Double-word-size access */
    {0,0}
};

void
dd_get_elf_symbol_shn_type(Dwarf_Unsigned value,
    struct esb_s *out)
{
    struct em_values *ev = &shn_vals[0];

    standard_table_name(ev, value,out);
}

static struct em_values stb_vals[] = {
    {"STB_LOCAL",0},
    /* Local symbol, not visible outside obj file containing def */

    {"STB_GLOBAL",1},
    /* Global symbol, visible to all object files being combined */

    {"STB_WEAK",2},
    /* Weak symbol, like global but lower-precedence */

    {"STB_GNU_UNIQUE",10},
    {"STB_LOOS",10},
    /* Lowest operating system-specific binding type */

    {"STB_HIOS",12},
    /* Highest operating system-specific binding type */

    {"STB_LOPROC",13},
    /* Lowest processor-specific binding type */

    {"STB_HIPROC",15},
    {0,0}
};

void
dd_get_elf_symbol_stb_string(Dwarf_Unsigned value,
    struct esb_s *out)
{
    struct em_values *ev = &stb_vals[0];

    standard_table_name(ev, value,out);
}

static struct em_values stt_vals[] = {
    {"STT_NOTYPE",0},     /* Symbol's type is not specified */
    {"STT_OBJECT",1},
    /* Symbol is a data object (variable, array, etc.) */

    {"STT_FUNC",2},
    /* Symbol is executable code (function, etc.) */

    {"STT_SECTION",3},    /* Symbol refers to a section */
    {"STT_FILE",4},
    /* Local, absolute symbol that refers to a file */

    {"STT_COMMON",5},     /* An uninitialized common block */
    {"STT_TLS",6},        /* Thread local data object */
    {"STT_GNU_IFUNC",10}, /* GNU indirect function */
    {"STT_LOOS",10},
    /* Lowest operating system-specific symbol type */

    {"STT_HIOS",12},
    /* Highest operating system-specific symbol type */

    {"STT_LOPROC",13},    /* Lowest processor-specific symbol type */
    {"STT_HIPROC",15},    /* Highest processor-specific symbol type */

    /* AMDGPU symbol types */
    {"STT_AMDGPU_HSA_KERNEL",10 },
    {0,0}
};

void
dd_get_elf_symbol_stt_type(Dwarf_Unsigned value, 
    struct esb_s *out)
{
    struct em_values *ev = &stt_vals[0];

    standard_table_name(ev, value,out);
}

static struct em_values osabi_vals[] = {
    {"ELFOSABI_NONE",0},   /* UNIX System V ABI */
    {"ELFOSABI_HPUX",1},   /* HP-UX operating system */
    {"ELFOSABI_NETBSD",2}, /* NetBSD */
    {"ELFOSABI_GNU",3},    /* GNU/Linux */
    {"ELFOSABI_LINUX",3},  /* Historical alias for ELFOSABI_GNU. */
    {"ELFOSABI_HURD",4},   /* GNU/Hurd */
    {"ELFOSABI_SOLARIS",6},/* Solaris */
    {"ELFOSABI_AIX",7},    /* AIX */
    {"ELFOSABI_IRIX",8},   /* IRIX */
    {"ELFOSABI_FREEBSD",9},/* FreeBSD */
    {"ELFOSABI_TRU64",10}, /* TRU64 UNIX */
    {"ELFOSABI_MODESTO",11},/* Novell Modesto */
    {"ELFOSABI_OPENBSD",12},/* OpenBSD */
    {"ELFOSABI_OPENVMS",13},/* OpenVMS */
    {"ELFOSABI_NSK",14},    /* Hewlett-Packard Non-Stop Kernel */
    {"ELFOSABI_AROS",15},      /* AROS */
    {"ELFOSABI_FENIXOS",16},   /* FenixOS */
    {"ELFOSABI_CLOUDABI",17},  /* Nuxi CloudABI */

    /* First architecture-specific OS ABI */
    {"ELFOSABI_FIRST_ARCH",64},
    {"ELFOSABI_AMDGPU_HSA",64},    /* AMD HSA runtime */
    {"ELFOSABI_AMDGPU_PAL",65},    /* AMD PAL runtime */
    /* AMD GCN GPUs (GFX6+) for MESA runtime */
    {"ELFOSABI_AMDGPU_MESA3D",66},
    {"ELFOSABI_ARM",97},          /* ARM */
    {"ELFOSABI_C6000_ELFABI",64}, /* Bare-metal TMS320C6000 */
    {"ELFOSABI_C6000_LINUX",65},  /* Linux TMS320C6000 */

    /* Standalone (embedded) application */
    {"ELFOSABI_STANDALONE",255},

    /* Last Architecture-specific OS ABI */
    {"ELFOSABI_LAST_ARCH",255},
    {0,0}
};

void
dd_get_elf_osabi_name(Dwarf_Unsigned value,
    struct esb_s *out)
{
    struct em_values *ev = &osabi_vals[0];

    standard_table_name(ev, value,out);
}
