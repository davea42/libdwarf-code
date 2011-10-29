/* 
  Copyright (C) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007-2010 David Anderson. All Rights Reserved.

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



$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/print_reloc.c,v 1.11 2005/08/04 05:09:37 davea Exp $ */

/*  The address of the Free Software Foundation is
    Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
    Boston, MA 02110-1301, USA.
    SGI has moved from the Crittenden Lane address.
*/




#include "globals.h"
using std::string;
using std::cout;
using std::cerr;
using std::endl;

#define DW_SECTION_REL_DEBUG_NUM      9  /* Number of sections */
/* .debug_abbrev should never have a relocation applied to it as it
   never refers to another section! (other sections refer to 
   .debug_abbrev) */

#define DW_SECTNAME_RELA_DEBUG          ".rela.debug_"
#define DW_SECTNAME_RELA_DEBUG_INFO     ".rela.debug_info"
#define DW_SECTNAME_RELA_DEBUG_LINE     ".rela.debug_line"
#define DW_SECTNAME_RELA_DEBUG_PUBNAMES ".rela.debug_pubnames"
#define DW_SECTNAME_RELA_DEBUG_ABBREV   ".rela.debug_abbrev"
#define DW_SECTNAME_RELA_DEBUG_ARANGES  ".rela.debug_aranges"
#define DW_SECTNAME_RELA_DEBUG_FRAME    ".rela.debug_frame"
#define DW_SECTNAME_RELA_DEBUG_LOC      ".rela.debug_loc"
#define DW_SECTNAME_RELA_DEBUG_RANGES   ".rela.debug_ranges"
#define DW_SECTNAME_RELA_DEBUG_TYPES    ".rela.debug_types"

#define DW_SECTNAME_REL_DEBUG          ".rel.debug_"
#define DW_SECTNAME_REL_DEBUG_INFO     ".rel.debug_info"
#define DW_SECTNAME_REL_DEBUG_LINE     ".rel.debug_line"
#define DW_SECTNAME_REL_DEBUG_PUBNAMES ".rel.debug_pubnames"
#define DW_SECTNAME_REL_DEBUG_ABBREV   ".rel.debug_abbrev"
#define DW_SECTNAME_REL_DEBUG_ARANGES  ".rel.debug_aranges"
#define DW_SECTNAME_REL_DEBUG_FRAME    ".rel.debug_frame"
#define DW_SECTNAME_REL_DEBUG_LOC      ".rel.debug_loc"
#define DW_SECTNAME_REL_DEBUG_RANGES   ".rel.debug_ranges"
#define DW_SECTNAME_REL_DEBUG_TYPES    ".rel.debug_types"


#define STRING_FOR_DUPLICATE " duplicate"
#define STRING_FOR_NULL      " null"

static const char *sectnames[] = {
    DW_SECTNAME_REL_DEBUG_INFO,
    DW_SECTNAME_REL_DEBUG_LINE,
    DW_SECTNAME_REL_DEBUG_PUBNAMES,
    DW_SECTNAME_REL_DEBUG_ABBREV,
    DW_SECTNAME_REL_DEBUG_ARANGES,
    DW_SECTNAME_REL_DEBUG_FRAME,
    DW_SECTNAME_REL_DEBUG_LOC,
    DW_SECTNAME_REL_DEBUG_RANGES,
    DW_SECTNAME_REL_DEBUG_TYPES,
};
static const char *sectnamesa[] = {
    DW_SECTNAME_RELA_DEBUG_INFO,
    DW_SECTNAME_RELA_DEBUG_LINE,
    DW_SECTNAME_RELA_DEBUG_PUBNAMES,
    DW_SECTNAME_RELA_DEBUG_ABBREV,
    DW_SECTNAME_RELA_DEBUG_ARANGES,
    DW_SECTNAME_RELA_DEBUG_FRAME,
    DW_SECTNAME_RELA_DEBUG_LOC,
    DW_SECTNAME_RELA_DEBUG_RANGES,
    DW_SECTNAME_RELA_DEBUG_TYPES,
};

static const char *error_msg_duplicate[] = {
    DW_SECTNAME_REL_DEBUG_INFO STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_LINE STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_PUBNAMES STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_ABBREV STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_ARANGES STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_FRAME STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_LOC STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_RANGES STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_TYPES STRING_FOR_DUPLICATE,
};

static const char *error_msg_null[] = {
    DW_SECTNAME_REL_DEBUG_INFO STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_LINE STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_PUBNAMES STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_ABBREV STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_ARANGES STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_FRAME STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_LOC STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_RANGES STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_TYPES STRING_FOR_NULL,
};

/*  Include Section type, to be able to deal with all the
    Elf32_Rel, Elf32_Rela, Elf64_Rel, Elf64_Rela relocation types */
#define SECT_DATA_SET(x,t,n) { \
    if (sect_data[(x)].buf != NULL) {                                \
        print_error(dbg, error_msg_duplicate[(x)],DW_DLV_OK, err);   \
    }                                                                \
    if ((data = elf_getdata(scn, 0)) == NULL || data->d_size == 0) { \
        print_error(dbg, error_msg_null[(x)],DW_DLV_OK, err);        \
    }                                                                \
        sect_data[(x)].buf = reinterpret_cast<Dwarf_Small *>(data -> d_buf); \
        sect_data[(x)].size = data -> d_size;                        \
        sect_data[(x)].type = t;                                     \
        sect_data[(x)].name = n;                                     \
    }


/* PowerPC relocations defined by the ABIs */
static  const char *reloc_type_names_PPC[] = {
    "R_PPC_NONE",                 /*  00 */
    "R_PPC_ADDR32",               /*  01 */
    "R_PPC_ADDR24",               /*  02 */
    "R_PPC_ADDR16",               /*  03 */
    "R_PPC_ADDR16_LO",            /*  04 */
    "R_PPC_ADDR16_HI",            /*  05 */
    "R_PPC_ADDR16_HA",            /*  06 */
    "R_PPC_ADDR14",               /*  07 */
    "R_PPC_ADDR14_BRTAKEN",       /*  08 */
    "R_PPC_ADDR14_BRNTAKEN",      /*  09 */
    "R_PPC_REL24",                /*  10 */
    "R_PPC_REL14",                /*  11 */
    "R_PPC_REL14_BRTAKEN",        /*  12 */
    "R_PPC_REL14_BRNTAKEN",       /*  13 */
    "R_PPC_GOT16",                /*  14 */
    "R_PPC_GOT16_LO",             /*  15 */
    "R_PPC_GOT16_HI",             /*  16 */
    "R_PPC_GOT16_HA",             /*  17 */
    "R_PPC_PLTREL24",             /*  18 */
    "R_PPC_COPY",                 /*  19 */
    "R_PPC_GLOB_DAT",             /*  20 */
    "R_PPC_JMP_SLOT",             /*  21 */
    "R_PPC_RELATIVE",             /*  22 */
    "R_PPC_LOCAL24PC",            /*  23 */
    "R_PPC_UADDR32",              /*  24 */
    "R_PPC_UADDR16",              /*  25 */
    "R_PPC_REL32",                /*  26 */
    "R_PPC_PLT32",                /*  27 */
    "R_PPC_PLTREL32",             /*  28 */
    "R_PPC_PLT16_LO",             /*  29 */
    "R_PPC_PLT16_HI",             /*  30 */
    "R_PPC_PLT16_HA",             /*  31 */
    "R_PPC_SDAREL16",             /*  32 */
    "R_PPC_SECTOFF",              /*  33 */
    "R_PPC_SECTOFF_LO",           /*  34 */
    "R_PPC_SECTOFF_HI",           /*  35 */
    "R_PPC_SECTOFF_HA",           /*  36 */
    "R_PPC_37",                   /*  37 */
    "R_PPC_38",                   /*  38 */
    "R_PPC_39",                   /*  39 */
    "R_PPC_40",                   /*  40 */
    "R_PPC_41",                   /*  41 */
    "R_PPC_42",                   /*  42 */
    "R_PPC_43",                   /*  43 */
    "R_PPC_44",                   /*  44 */
    "R_PPC_45",                   /*  45 */
    "R_PPC_46",                   /*  46 */
    "R_PPC_47",                   /*  47 */
    "R_PPC_48",                   /*  48 */
    "R_PPC_49",                   /*  49 */
    "R_PPC_50",                   /*  50 */
    "R_PPC_51",                   /*  51 */
    "R_PPC_52",                   /*  52 */
    "R_PPC_53",                   /*  53 */
    "R_PPC_54",                   /*  54 */
    "R_PPC_55",                   /*  55 */
    "R_PPC_56",                   /*  56 */
    "R_PPC_57",                   /*  57 */
    "R_PPC_58",                   /*  58 */
    "R_PPC_59",                   /*  59 */
    "R_PPC_60",                   /*  60 */
    "R_PPC_61",                   /*  61 */
    "R_PPC_62",                   /*  62 */
    "R_PPC_63",                   /*  63 */
    "R_PPC_64",                   /*  64 */
    "R_PPC_65",                   /*  65 */
    "R_PPC_66",                   /*  66 */
    "R_PPC_TLS",                  /*  67 */
    "R_PPC_DTPMOD32",             /*  68 */
    "R_PPC_TPREL16",              /*  69 */
    "R_PPC_TPREL16_LO",           /*  70 */
    "R_PPC_TPREL16_HI",           /*  71 */
    "R_PPC_TPREL16_HA",           /*  72 */
    "R_PPC_TPREL32",              /*  73 */
    "R_PPC_DTPREL16",             /*  74 */
    "R_PPC_DTPREL16_LO",          /*  75 */
    "R_PPC_DTPREL16_HI",          /*  76 */
    "R_PPC_DTPREL16_HA",          /*  77 */
    "R_PPC_DTPREL64",             /*  78 */
    "R_PPC_GOT_TLSGD16",          /*  79 */
    "R_PPC_GOT_TLSGD16_LO",       /*  80 */
    "R_PPC_GOT_TLSGD16_HI",       /*  81 */
    "R_PPC_GOT_TLSGD16_HA",       /*  82 */
    "R_PPC_GOT_TLSLD16",          /*  83 */
    "R_PPC_GOT_TLSLD16_LO",       /*  84 */
    "R_PPC_GOT_TLSLD16_HI",       /*  85 */
    "R_PPC_GOT_TLSLD16_HA",       /*  86 */
    "R_PPC_GOT_TPREL16_DS",       /*  87 */
    "R_PPC_GOT_TPREL16_LO",       /*  88 */
    "R_PPC_GOT_TPREL16_HI",       /*  89 */
    "R_PPC_GOT_TPREL16_HA",       /*  90 */
    "R_PPC_GOT_DTPREL16",         /*  91 */
    "R_PPC_GOT_DTPREL16_LO",      /*  92 */
    "R_PPC_GOT_DTPREL16_HI",      /*  93 */
    "R_PPC_GOT_DTPREL16_HA",      /*  94 */
};

/* PowerPC64 relocations defined by the ABIs */
static const char *reloc_type_names_PPC64[] = {
    "R_PPC64_NONE",                 /*  00 */
    "R_PPC64_ADDR32",               /*  01 */
    "R_PPC64_ADDR24",               /*  02 */
    "R_PPC64_ADDR16",               /*  03 */
    "R_PPC64_ADDR16_LO",            /*  04 */
    "R_PPC64_ADDR16_HI",            /*  05 */
    "R_PPC64_ADDR16_HA",            /*  06 */
    "R_PPC64_ADDR14",               /*  07 */
    "R_PPC64_ADDR14_BRTAKEN",       /*  08 */
    "R_PPC64_ADDR14_BRNTAKEN",      /*  09 */
    "R_PPC64_REL24",                /*  10 */
    "R_PPC64_REL14",                /*  11 */
    "R_PPC64_REL14_BRTAKEN",        /*  12 */
    "R_PPC64_REL14_BRNTAKEN",       /*  13 */
    "R_PPC64_GOT16",                /*  14 */
    "R_PPC64_GOT16_LO",             /*  15 */
    "R_PPC64_GOT16_HI",             /*  16 */
    "R_PPC64_GOT16_HA",             /*  17 */
    "R_PPC64_PLTREL24",             /*  18 */
    "R_PPC64_COPY",                 /*  19 */
    "R_PPC64_GLOB_DAT",             /*  20 */
    "R_PPC64_JMP_SLOT",             /*  21 */
    "R_PPC64_RELATIVE",             /*  22 */
    "R_PPC64_LOCAL24PC",            /*  23 */
    "R_PPC64_UADDR32",              /*  24 */
    "R_PPC64_UADDR16",              /*  25 */
    "R_PPC64_REL32",                /*  26 */
    "R_PPC64_PLT32",                /*  27 */
    "R_PPC64_PLTREL32",             /*  28 */
    "R_PPC64_PLT16_LO",             /*  29 */
    "R_PPC64_PLT16_HI",             /*  30 */
    "R_PPC64_PLT16_HA",             /*  31 */
    "R_PPC64_SDAREL16",             /*  32 */
    "R_PPC64_SECTOFF",              /*  33 */
    "R_PPC64_SECTOFF_LO",           /*  34 */
    "R_PPC64_SECTOFF_HI",           /*  35 */
    "R_PPC64_SECTOFF_HA",           /*  36 */
    "R_PPC64_REL30",                /*  37 */
    "R_PPC64_ADDR64",               /*  38 */
    "R_PPC64_ADDR16_HIGHER",        /*  39 */
    "R_PPC64_ADDR16_HIGHERA",       /*  40 */
    "R_PPC64_ADDR16_HIGHEST",       /*  41 */
    "R_PPC64_ADDR16_HIGHESTA",      /*  42 */
    "R_PPC64_UADDR64",              /*  43 */
    "R_PPC64_REL64",                /*  44 */
    "R_PPC64_PLT64",                /*  45 */
    "R_PPC64_PLTREL64",             /*  46 */
    "R_PPC64_TOC16",                /*  47 */
    "R_PPC64_TOC16_LO",             /*  48 */
    "R_PPC64_TOC16_HI",             /*  49 */
    "R_PPC64_TOC16_HA",             /*  50 */
    "R_PPC64_TOC",                  /*  51 */
    "R_PPC64_PLTGOT16",             /*  52 */
    "R_PPC64_PLTGOT16_LO",          /*  53 */
    "R_PPC64_PLTGOT16_HI",          /*  54 */
    "R_PPC64_PLTGOT16_HA",          /*  55 */
    "R_PPC64_ADDR16_DS",            /*  56 */
    "R_PPC64_ADDR16_LO_DS",         /*  57 */
    "R_PPC64_GOT16_DS",             /*  58 */
    "R_PPC64_GOT16_LO_DS",          /*  59 */
    "R_PPC64_PLT16_LO_DS",          /*  60 */
    "R_PPC64_SECTOFF_DS",           /*  61 */
    "R_PPC64_SECTOFF_LO_DS",        /*  62 */
    "R_PPC64_TOC16_DS",             /*  63 */
    "R_PPC64_TOC16_LO_DS",          /*  64 */
    "R_PPC64_PLTGOT16_DS",          /*  65 */
    "R_PPC64_PLTGOT16_LO_DS",       /*  66 */
    "R_PPC64_TLS",                  /*  67 */
    "R_PPC64_DTPMOD32",             /*  68 */
    "R_PPC64_TPREL16",              /*  69 */
    "R_PPC64_TPREL16_LO",           /*  70 */
    "R_PPC64_TPREL16_HI",           /*  71 */
    "R_PPC64_TPREL16_HA",           /*  72 */
    "R_PPC64_TPREL32",              /*  73 */
    "R_PPC64_DTPREL16",             /*  74 */
    "R_PPC64_DTPREL16_LO",          /*  75 */
    "R_PPC64_DTPREL16_HI",          /*  76 */
    "R_PPC64_DTPREL16_HA",          /*  77 */
    "R_PPC64_DTPREL64",             /*  78 */
    "R_PPC64_GOT_TLSGD16",          /*  79 */
    "R_PPC64_GOT_TLSGD16_LO",       /*  80 */
    "R_PPC64_GOT_TLSGD16_HI",       /*  81 */
    "R_PPC64_GOT_TLSGD16_HA",       /*  82 */
    "R_PPC64_GOT_TLSLD16",          /*  83 */
    "R_PPC64_GOT_TLSLD16_LO",       /*  84 */
    "R_PPC64_GOT_TLSLD16_HI",       /*  85 */
    "R_PPC64_GOT_TLSLD16_HA",       /*  86 */
    "R_PPC64_GOT_TPREL16_DS",       /*  87 */
    "R_PPC64_GOT_TPREL16_LO",       /*  88 */
    "R_PPC64_GOT_TPREL16_HI",       /*  89 */
    "R_PPC64_GOT_TPREL16_HA",       /*  90 */
    "R_PPC64_GOT_DTPREL16",         /*  91 */
    "R_PPC64_GOT_DTPREL16_LO",      /*  92 */
    "R_PPC64_GOT_DTPREL16_HI",      /*  93 */
    "R_PPC64_GOT_DTPREL16_HA",      /*  94 */
    "R_PPC64_TPREL16_DS",           /*  95 */
    "R_PPC64_TPREL16_LO_DS",        /*  96 */
    "R_PPC64_TPREL16_HIGHER",       /*  97 */
    "R_PPC64_TPREL16_HIGHERA",      /*  98 */
    "R_PPC64_TPREL16_HIGHEST",      /*  99 */
    "R_PPC64_TPREL16_HIGHESTA",     /* 100 */
    "R_PPC64_DTPREL16_DS",          /* 101 */
    "R_PPC64_DTPREL16_LO_DS",       /* 102 */
    "R_PPC64_DTPREL16_HIGHER",      /* 103 */
    "R_PPC64_DTPREL16_HIGHERA",     /* 104 */
    "R_PPC64_DTPREL16_HIGHEST",     /* 105 */
    "R_PPC64_DTPREL16_HIGHESTA",    /* 106 */
    "R_PPC64_TOC32",                /* 107 */
    "R_PPC64_DTPMOD32",             /* 108 */
    "R_PPC64_TPREL32",              /* 109 */
    "R_PPC64_DTPREL32",             /* 110 */
};

/* Relocation types for MIPS */
static const char *reloc_type_names_MIPS[] = {
    "R_MIPS_NONE", "R_MIPS_16", "R_MIPS_32", "R_MIPS_REL32",
    "R_MIPS_26", "R_MIPS_HI16", "R_MIPS_LO16", "R_MIPS_GPREL16",
    "R_MIPS_LITERAL", "R_MIPS_GOT16", "R_MIPS_PC16", "R_MIPS_CALL16",
    "R_MIPS_GPREL32",           /* 12 */
    "reloc type 13?", "reloc type 14?", "reloc type 15?",
    "R_MIPS_SHIFT5",            /* 16 */
    "R_MIPS_SHIFT6",            /* 17 */
    "R_MIPS_64",                /* 18 */
    "R_MIPS_GOT_DISP",          /* 19 */
    "R_MIPS_GOT_PAGE",          /* 20 */
    "R_MIPS_GOT_OFST",          /* 21 */
    "R_MIPS_GOT_HI16",          /* 22 */
    "R_MIPS_GOT_LO16",          /* 23 */
    "R_MIPS_SUB",               /* 24 */
    "R_MIPS_INSERT_A",          /* 25 */
    "R_MIPS_INSERT_B",          /* 26 */
    "R_MIPS_DELETE",            /* 27 */
    "R_MIPS_HIGHER",            /* 28 */
    "R_MIPS_HIGHEST",           /* 29 */
    "R_MIPS_CALL_HI16",         /* 30 */
    "R_MIPS_CALL_LO16",         /* 31 */
    "R_MIPS_SCN_DISP",          /* 32 */
    "R_MIPS_REL16",             /* 33 */
    "R_MIPS_ADD_IMMEDIATE",     /* 34 */
};

/* ARM relocations defined by the ABIs */
static const char *reloc_type_names_ARM[] = {
    "R_ARM_NONE",                 /*  00 */
    "R_ARM_PC24",                 /*  01 */
    "R_ARM_ABS32",                /*  02 */
    "R_ARM_REL32",                /*  03 */
    "R_ARM_LDR_PC_G0",            /*  04 */
    "R_ARM_ABS16",                /*  05 */
    "R_ARM_ABS12",                /*  06 */
    "R_ARM_THM_ABS5",             /*  07 */
    "R_ARM_ABS8",                 /*  08 */
    "R_ARM_SBREL32",              /*  09 */
    "R_ARM_THM_CALL",             /*  10 */
    "R_ARM_THM_PC8",              /*  11 */
    "R_ARM_BREL_ADJ",             /*  12 */
    "R_ARM_TLS_DESC",             /*  13 */
    "R_ARM_THM_SWI8",             /*  14 */
    "R_ARM_XPC25",                /*  15 */
    "R_ARM_THM_XPC22",            /*  16 */
    "R_ARM_TLS_DTPMOD32",         /*  17 */
    "R_ARM_TLS_DTPOFF32",         /*  18 */
    "R_ARM_TLS_TPOFF32",          /*  19 */
    "R_ARM_COPY",                 /*  20 */
    "R_ARM_GLOB_DAT",             /*  21 */
    "R_ARM_JUMP_SLOT",            /*  22 */
    "R_ARM_RELATIVE",             /*  23 */
    "R_ARM_GOTOFF32",             /*  24 */
    "R_ARM_BASE_PREL",            /*  25 */
    "R_ARM_GOT_BREL",             /*  26 */
    "R_ARM_PLT32",                /*  27 */
    "R_ARM_CALL",                 /*  28 */
    "R_ARM_JUMP24",               /*  29 */
    "R_ARM_THM_JUMP24",           /*  30 */
    "R_ARM_BASE_ABS",             /*  31 */
    "R_ARM_ALU_PCREL_7_0",        /*  32 */
    "R_ARM_ALU_PCREL_15_8",       /*  33 */
    "R_ARM_ALU_PCREL_23_15",      /*  34 */
    "R_ARM_LDR_SBREL_11_0_NC",    /*  35 */
    "R_ARM_ALU_SBREL_19_12_NC",   /*  36 */
    "R_ARM_ALU_SBREL_27_20_CK",   /*  37 */
    "R_ARM_TARGET1",              /*  38 */
    "R_ARM_SBREL31",              /*  39 */
    "R_ARM_V4BX",                 /*  40 */
    "R_ARM_TARGET2",              /*  41 */
    "R_ARM_PREL31",               /*  42 */
    "R_ARM_MOVW_ABS_NC",          /*  43 */
    "R_ARM_MOVT_ABS",             /*  44 */
    "R_ARM_MOVW_PREL_NC",         /*  45 */
    "R_ARM_MOVT_PREL",            /*  46 */
    "R_ARM_THM_MOVW_ABS_NC",      /*  47 */
    "R_ARM_THM_MOVT_ABS",         /*  48 */
    "R_ARM_THM_MOVW_PREL_NC",     /*  49 */
    "R_ARM_THM_MOVT_PREL",        /*  50 */
    "R_ARM_THM_JUMP19",           /*  51 */
    "R_ARM_THM_JUMP6",            /*  52 */
    "R_ARM_THM_ALU_PREL_11_0",    /*  53 */
    "R_ARM_THM_PC12",             /*  54 */
    "R_ARM_ABS32_NOI",            /*  55 */
    "R_ARM_REL32_NOI",            /*  56 */
    "R_ARM_ALU_PC_G0_NC",         /*  57 */
    "R_ARM_ALU_PC_G0",            /*  58 */
    "R_ARM_ALU_PC_G1_NC",         /*  59 */
    "R_ARM_ALU_PC_G1",            /*  60 */
    "R_ARM_ALU_PC_G2",            /*  61 */
    "R_ARM_LDR_PC_G1",            /*  62 */
    "R_ARM_LDR_PC_G2",            /*  63 */
    "R_ARM_LDRS_PC_G0",           /*  64 */
    "R_ARM_LDRS_PC_G1",           /*  65 */
    "R_ARM_LDRS_PC_G2",           /*  66 */
    "R_ARM_LDC_PC_G0",            /*  67 */
    "R_ARM_LDC_PC_G1",            /*  68 */
    "R_ARM_LDC_PC_G2",            /*  69 */
    "R_ARM_ALU_SB_G0_NC",         /*  70 */
    "R_ARM_ALU_SB_G0",            /*  71 */
    "R_ARM_ALU_SB_G1_NC",         /*  72 */
    "R_ARM_ALU_SB_G1",            /*  73 */
    "R_ARM_ALU_SB_G2",            /*  74 */
    "R_ARM_LDR_SB_G0",            /*  75 */
    "R_ARM_LDR_SB_G1",            /*  76 */
    "R_ARM_LDR_SB_G2",            /*  77 */
    "R_ARM_LDRS_SB_G0",           /*  78 */
    "R_ARM_LDRS_SB_G1",           /*  79 */
    "R_ARM_LDRS_SB_G2",           /*  80 */
    "R_ARM_LDC_SB_G0",            /*  81 */
    "R_ARM_LDC_SB_G1",            /*  82 */
    "R_ARM_LDC_SB_G2",            /*  83 */
    "R_ARM_MOVW_BREL_NC",         /*  84 */
    "R_ARM_MOVT_BREL",            /*  85 */
    "R_ARM_MOVW_BREL",            /*  86 */
    "R_ARM_THM_MOVW_BREL_NC",     /*  87 */
    "R_ARM_THM_MOVT_BREL",        /*  88 */
    "R_ARM_THM_MOVW_BREL",        /*  89 */
    "R_ARM_TLS_GOTDESC",          /*  90 */
    "R_ARM_TLS_CALL",             /*  91 */
    "R_ARM_TLS_DESCSEQ",          /*  92 */
    "R_ARM_THM_TLS_CALL",         /*  93 */
    "R_ARM_PLT32_ABS",            /*  94 */
    "R_ARM_GOT_ABS",              /*  95 */
    "R_ARM_GOT_PREL",             /*  96 */
    "R_ARM_GOT_BREL12",           /*  97 */
    "R_ARM_GOTOFF12",             /*  98 */
    "R_ARM_GOTRELAX",             /*  99 */
    "R_ARM_GNU_VTENTRY",          /* 100 */
    "R_ARM_GNU_VTINHERIT",        /* 101 */
    "R_ARM_THM_JUMP11",           /* 102 */
    "R_ARM_THM_JUMP8",            /* 103 */
    "R_ARM_TLS_GD32",             /* 104 */
    "R_ARM_TLS_LDM32",            /* 105 */
    "R_ARM_TLS_LDO32",            /* 106 */
    "R_ARM_TLS_IE32",             /* 107 */
    "R_ARM_TLS_LE32",             /* 108 */
    "R_ARM_TLS_LDO12",            /* 109 */
    "R_ARM_TLS_LE12",             /* 110 */
    "R_ARM_TLS_IE12GP",           /* 111 */
    "R_ARM_TLS_MOVT_TPOFF32",     /* 112 */   /* "R_ARM_PRIVATE_0" */
    "R_ARM_TLS_MOVW_TPOFF32",     /* 113 */   /* "R_ARM_PRIVATE_1" */
    "R_ARM_THM_TLS_MOVT_TPOFF32", /* 114 */   /* "R_ARM_PRIVATE_2" */
    "R_ARM_THM_TLS_MOVT_TPOFF32", /* 115 */   /* "R_ARM_PRIVATE_3" */
    "R_ARM_PRIVATE_4",            /* 116 */
    "R_ARM_PRIVATE_5",            /* 117 */
    "R_ARM_PRIVATE_6",            /* 118 */
    "R_ARM_PRIVATE_7",            /* 119 */
    "R_ARM_PRIVATE_8",            /* 120 */
    "R_ARM_PRIVATE_9",            /* 121 */
    "R_ARM_PRIVATE_10",           /* 122 */
    "R_ARM_PRIVATE_11",           /* 123 */
    "R_ARM_PRIVATE_12",           /* 124 */
    "R_ARM_PRIVATE_13",           /* 125 */
    "R_ARM_PRIVATE_14",           /* 126 */
    "R_ARM_PRIVATE_15",           /* 127 */
    "R_ARM_ME_TOO",               /* 128 */
    "R_ARM_THM_TLS_DESCSEQ16",    /* 129 */
    "R_ARM_THM_TLS_DESCSEQ32",    /* 130 */
};

/* Record the relocation table name information */
static const char **reloc_type_names = NULL;
static Dwarf_Small number_of_reloc_type_names = 0;

/* Set the relocation names based on the machine type */
static void 
set_relocation_table_names(Dwarf_Small machine_type)
{
  switch (machine_type) {
    case EM_MIPS:
        reloc_type_names = reloc_type_names_MIPS;
        number_of_reloc_type_names = 
            sizeof(reloc_type_names_MIPS) / sizeof(char *);
        break;
    case EM_PPC:
        reloc_type_names = reloc_type_names_PPC;
        number_of_reloc_type_names = 
            sizeof(reloc_type_names_PPC) / sizeof(char *);
        break;
    case EM_PPC64:
        reloc_type_names = reloc_type_names_PPC64;
        number_of_reloc_type_names = 
            sizeof(reloc_type_names_PPC64) / sizeof(char *);
        break;
    case EM_ARM:
        reloc_type_names = reloc_type_names_ARM;
        number_of_reloc_type_names = 
            sizeof(reloc_type_names_ARM) / sizeof(char *);
        break;
    default:
        /* We don't have others covered. */
        reloc_type_names = 0; 
        number_of_reloc_type_names = 0; 
        break;
  }
}



/*  return valid reloc type names. 
    if buf is used, it is static, so beware it
    will be overrwritten by the next call.
*/
static string
get_reloc_type_names(int index)
{
    string retval;

    if (index < 0 || index >= number_of_reloc_type_names) {
        retval = "reloc type ";
        retval.append(IToDec(index));
        retval.append(" unknown");
    } else {
        retval = reloc_type_names[index];
    }
    return retval;
}

#ifndef HAVE_ELF64_GETEHDR
#define Elf64_Addr  long
#define Elf64_Word  unsigned long
#define Elf64_Xword unsigned long
#define Elf64_Sym   long
#endif

static struct {
    Dwarf_Small *buf;
    Dwarf_Unsigned size;
    Dwarf_Bool display; // Display reloc if TRUE
    const char *name;   // Section name.
    Elf64_Xword type;   // To cover 32 and 64 records types.
} sect_data[DW_SECTION_REL_DEBUG_NUM];


typedef size_t indx_type;

typedef struct {
    indx_type indx;
    char *name;
    Elf32_Addr value;
    Elf32_Word size;
    int type;
    int bind;
    unsigned char other;
    Elf32_Half shndx;
} SYM;


typedef struct {
    indx_type indx;
    char *name;
    Elf64_Addr value;
    Elf64_Xword size;
    int type;
    int bind;
    unsigned char other;
    unsigned short shndx;
} SYM64;

static void print_reloc_information_64(int section_no,
    Dwarf_Small * buf,
    Dwarf_Unsigned size, 
    Elf64_Xword type,
    const char **scn_names, int scn_names_cnt);
static void print_reloc_information_32(int section_no,
    Dwarf_Small * buf,
    Dwarf_Unsigned size, 
    Elf64_Xword type,
    const char **scn_names, int scn_names_cnt);
static SYM *readsyms(Elf32_Sym * data, size_t num, Elf * elf,
    Elf32_Word link);
static SYM64 *read_64_syms(Elf64_Sym * data, size_t num, Elf * elf,
    Elf64_Word link);
static void *get_scndata(Elf_Scn * fd_scn, size_t * scn_size);
static void print_relocinfo_64(Dwarf_Debug dbg, Elf * elf);
static void print_relocinfo_32(Dwarf_Debug dbg, Elf * elf);

static SYM   *sym_data;
static SYM64 *sym_data_64;
static unsigned long   sym_data_entry_count;
static unsigned long   sym_data_64_entry_count;


typedef struct {
    indx_type index;
    const char *name_rel;     /* .rel.debug_* names  */
    const char *name_rela;    /* .rela.debug_* names */
} REL_INFO;


/*  If the incoming scn_name is known, record the name
    in our reloc section names table. 
    For a given (debug) section there can be a .rel or a .rela,
    not both. 
    The name-to-index in this table is fixed, invariant.
    It has to match other tables like
*/
static bool 
get_reloc_section(Dwarf_Debug dbg,
    Elf_Scn *scn,
    const char *scn_name,
    Elf64_Word sh_type)
{
    static REL_INFO rel_info[DW_SECTION_REL_DEBUG_NUM] = {
    {/*0*/ DW_SECTION_REL_DEBUG_INFO,
    DW_SECTNAME_REL_DEBUG_INFO,
    DW_SECTNAME_RELA_DEBUG_INFO},

    {/*1*/ DW_SECTION_REL_DEBUG_LINE,
    DW_SECTNAME_REL_DEBUG_LINE,
    DW_SECTNAME_RELA_DEBUG_LINE},

    {/*2*/ DW_SECTION_REL_DEBUG_PUBNAMES,
    DW_SECTNAME_REL_DEBUG_PUBNAMES,
    DW_SECTNAME_RELA_DEBUG_PUBNAMES},

    {/*3*/ DW_SECTION_REL_DEBUG_ABBREV,
    DW_SECTNAME_REL_DEBUG_ABBREV,
    DW_SECTNAME_RELA_DEBUG_ABBREV},

    {/*4*/ DW_SECTION_REL_DEBUG_ARANGES,
    DW_SECTNAME_REL_DEBUG_ARANGES,
    DW_SECTNAME_RELA_DEBUG_ARANGES},

    {/*5*/ DW_SECTION_REL_DEBUG_FRAME,
    DW_SECTNAME_REL_DEBUG_FRAME,
    DW_SECTNAME_RELA_DEBUG_FRAME},

    {/*6*/ DW_SECTION_REL_DEBUG_LOC,
    DW_SECTNAME_REL_DEBUG_LOC,
    DW_SECTNAME_RELA_DEBUG_LOC},

    {/*7*/ DW_SECTION_REL_DEBUG_RANGES,
    DW_SECTNAME_REL_DEBUG_RANGES,
    DW_SECTNAME_RELA_DEBUG_RANGES},

    {/*8*/ DW_SECTION_REL_DEBUG_TYPES,
    DW_SECTNAME_REL_DEBUG_TYPES,
    DW_SECTNAME_RELA_DEBUG_TYPES},
    };

    Elf_Data *data;
    int index;
    for (index = 0; index < DW_SECTION_REL_DEBUG_NUM; ++index) {
        const char *n = rel_info[index].name_rel;
        const char *na = rel_info[index].name_rela;
        if (strcmp(scn_name, n) == 0) {
            SECT_DATA_SET(rel_info[index].index,sh_type,n)
            return true;
        }
        if (strcmp(scn_name, na) == 0) {
            SECT_DATA_SET(rel_info[index].index,sh_type,na)
            return true;
        }
    }
    return false;
}



void
print_relocinfo(Dwarf_Debug dbg, unsigned reloc_map)
{
    Elf *elf;
    char *endr_ident;
    int is_64bit;
    int res;
    int i;

    for (i = 0; i < DW_SECTION_REL_DEBUG_NUM; i++) {
        sect_data[i].display = reloc_map & (1 << i);
        sect_data[i].buf = 0;
        sect_data[i].size = 0;
        sect_data[i].type = SHT_NULL;
    }
    res = dwarf_get_elf(dbg, &elf, &err);
    if (res != DW_DLV_OK) {
        print_error(dbg, "dwarf_get_elf error", res, err);
    }
    if ((endr_ident = elf_getident(elf, NULL)) == NULL) {
        print_error(dbg, "DW_ELF_GETIDENT_ERROR", res, err);
    }
    is_64bit = (endr_ident[EI_CLASS] == ELFCLASS64);
    if (is_64bit) {
        print_relocinfo_64(dbg, elf);
    } else {
        print_relocinfo_32(dbg, elf);
    }
}

static void
print_relocinfo_64(Dwarf_Debug dbg, Elf * elf)
{
#ifdef HAVE_ELF64_GETEHDR
    Elf_Scn *scn = NULL;
    Elf64_Ehdr *ehdr64 = 0;
    Elf64_Shdr *shdr64  = 0;
    const char *scn_name = 0;
    const char **scn_names = 0;
    int i = 0;
    Elf64_Sym *sym_64 = 0;
    int scn_names_cnt = 0;

    ehdr64 = elf64_getehdr(elf);
    if (ehdr64 == NULL) {
        print_error(dbg, "DW_ELF_GETEHDR_ERROR", DW_DLV_OK, err);
    }
    scn_names = (const char **)calloc(ehdr64->e_shnum + 1, sizeof(char *));

    while ((scn = elf_nextscn(elf, scn)) != NULL) {

        shdr64 = elf64_getshdr(scn);
        if (shdr64  == NULL) {
            print_error(dbg, "DW_ELF_GETSHDR_ERROR", DW_DLV_OK, err);
        }
        scn_name = elf_strptr(elf, ehdr64->e_shstrndx, shdr64->sh_name);
        if (scn_name == NULL) {
            print_error(dbg, "DW_ELF_STRPTR_ERROR", DW_DLV_OK, err);
        }
        if(scn_names) {
            /* elf_nextscn() skips section with index '0' */
            scn_names[++scn_names_cnt] = scn_name;
        }
        if (shdr64->sh_type == SHT_SYMTAB) {
            size_t sym_size = 0;
            size_t count = 0;

            sym_64 = (Elf64_Sym *) get_scndata(scn, &sym_size);
            if (sym_64 == NULL) {
                print_error(dbg, "no symbol table data", DW_DLV_OK,
                    err);
            }
            count = sym_size / sizeof(Elf64_Sym);
            sym_64++;
            free(sym_data_64);
            sym_data_64 = read_64_syms(sym_64, count, elf, shdr64->sh_link);
            sym_data_64_entry_count = count;
            if (sym_data_64  == NULL) {
                print_error(dbg, "problem reading symbol table data",
                    DW_DLV_OK, err);
            }
        } else if (!get_reloc_section(dbg,scn,scn_name,shdr64->sh_type)) {
            continue;
        }
    } /* while */
    /* Set the relocation names based on the machine type */
    set_relocation_table_names(ehdr64->e_machine);
    for (i = 0; i < DW_SECTION_REL_DEBUG_NUM; i++) {
        if (sect_data[i].display && 
            sect_data[i].buf != NULL && 
            sect_data[i].size > 0) { 
            print_reloc_information_64(i, sect_data[i].buf,
                sect_data[i].size, sect_data[i].type,scn_names,
                scn_names_cnt);
        }            
    }        

    if (scn_names) {
        free(scn_names);
        scn_names = 0;
        scn_names_cnt = 0;
    }        
#endif
}

static void
print_relocinfo_32(Dwarf_Debug dbg, Elf * elf)
{
    Elf_Scn *scn = NULL;
    Elf32_Ehdr *ehdr32 = 0;
    Elf32_Shdr *shdr32 = 0;
    char *scn_name = 0;
    int i = 0;
    Elf32_Sym  *sym = 0;
    const char **scn_names = 0;
    int scn_names_cnt = 0;


    ehdr32 = elf32_getehdr(elf);
    if (ehdr32 == NULL) {
        print_error(dbg, "DW_ELF_GETEHDR_ERROR", DW_DLV_OK, err);
    }
    /*  Make the section name array big enough 
        that we don't need to check for overrun in the loop. */
    scn_names = (const char **)calloc(ehdr32->e_shnum + 1, sizeof(char *));
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        shdr32 = elf32_getshdr(scn);
        if (shdr32 == NULL) {
            print_error(dbg, "DW_ELF_GETSHDR_ERROR", DW_DLV_OK, err);
        }
        scn_name = elf_strptr(elf, ehdr32->e_shstrndx, shdr32->sh_name);
        if (scn_name == NULL) {
            print_error(dbg, "DW_ELF_STRPTR_ERROR", DW_DLV_OK, err);
        }
        if (scn_names) {
            /* elf_nextscn() skips section with index '0' */
            scn_names[++scn_names_cnt] = scn_name;
        }
        if (shdr32->sh_type == SHT_SYMTAB) {
            size_t sym_size = 0;
            size_t count = 0;

            sym = (Elf32_Sym *) get_scndata(scn, &sym_size);
            if (sym == NULL) {
                print_error(dbg, "no symbol table data", DW_DLV_OK,
                    err);
            }
            sym = (Elf32_Sym *) get_scndata(scn, &sym_size);
            count = sym_size / sizeof(Elf32_Sym);
            sym++;
            free(sym_data);
            sym_data = readsyms(sym, count, elf, shdr32->sh_link);
            sym_data_entry_count = count;
            if (sym_data  == NULL) {
                print_error(dbg, "problem reading symbol table data",
                    DW_DLV_OK, err);
            }
        } else if (!get_reloc_section(dbg,scn,scn_name,shdr32->sh_type)) {
            continue;
        }
    }   /* while */
    /* Set the relocation names based on the machine type */
    set_relocation_table_names(ehdr32->e_machine);

    for (i = 0; i < DW_SECTION_REL_DEBUG_NUM; i++) {
        if (sect_data[i].display &&
            sect_data[i].buf != NULL &&
            sect_data[i].size > 0) {
            print_reloc_information_32(i, sect_data[i].buf,
                sect_data[i].size,sect_data[i].type,scn_names,
                scn_names_cnt);
        }
    }

    if (scn_names) {
        free(scn_names);
        scn_names = 0;
        scn_names_cnt = 0;
    }

}

#if HAVE_ELF64_R_INFO
#ifndef ELF64_R_TYPE
#define ELF64_R_TYPE(x) 0       /* FIXME */
#endif
#ifndef ELF64_R_SYM
#define ELF64_R_SYM(x) 0        /* FIXME */
#endif
#ifndef ELF64_ST_TYPE
#define ELF64_ST_TYPE(x) 0      /* FIXME */
#endif
#ifndef ELF64_ST_BIND
#define ELF64_ST_BIND(x) 0      /* FIXME */
#endif
#endif /* HAVE_ELF64_R_INFO */


static void
print_reloc_information_64(int section_no, Dwarf_Small * buf,
    Dwarf_Unsigned size, Elf64_Xword type,
    const char **scn_names, int scn_names_count)
{
    /* Include support for Elf64_Rel and Elf64_Rela */
    Dwarf_Unsigned add = 0;
    Dwarf_Half rel_size = SHT_RELA == type ?
        sizeof(Elf64_Rela) : sizeof(Elf64_Rel);
    Dwarf_Unsigned off = 0;

    cout << endl;
    cout << ((type == SHT_RELA)?sectnamesa[section_no]:
        sectnames[section_no]) << ":" << endl;;
    /* Print some headers and change the order for better reading */
    printf("Offset     Addend     %-26s Index   Symbol Name\n","Reloc Type");
#if HAVE_ELF64_GETEHDR
    for (off = 0; off < size; off += rel_size) {
#if HAVE_ELF64_R_INFO
        /* This works for the Elf64_Rel in linux */
        Elf64_Rel *p = (Elf64_Rel *) (buf + off);
        string name;
        if(sym_data ) {
            size_t index = ELF64_R_SYM(p->r_info) - 1;
            if(index < sym_data_entry_count) {
                name = sym_data[index].name;
            }
        } else if (sym_data_64) {
            size_t index = ELF64_R_SYM(p->r_info) - 1;
            if(index < sym_data_64_entry_count) {
                name = sym_data_64[index].name;
            }
        }
        /*  When the name is not available, use the
            section name as a reference for the name.*/
        if (name.empty()) {
            size_t index = ELF64_R_SYM(p->r_info) - 1;
            if (index <  sym_data_64_entry_count) {
                SYM64 *sym_64 = &sym_data_64[index];
                if (sym_64->type == STT_SECTION &&
                    sym_64->shndx < scn_names_count) {
                    name = scn_names[sym_64->shndx];
                }
            }
        }
        if (name.empty()) {
            name = "<no name>";
        }

        if (SHT_RELA == type) {
            Elf64_Rela *pa = (Elf64_Rela *)p;
            add = pa->r_addend;
        }


        cout << IToHex0N(p->r_offset,10);
        cout << " " << IToHex0N(add,10);
        cout << " " <<LeftAlign(26,get_reloc_type_names(ELF64_R_TYPE(p->r_info)));
        cout << " " << BracketSurround(IToDec(ELF64_R_SYM(p->r_info),5));
        cout << " " << name;
        cout << endl;
#else
        /*  sgi/mips -64 does not have r_info in the 64bit relocations,
            but seperate fields, with 3 types, actually. Only one of
            which prints here, as only one really used with dwarf */
        Elf64_Rel *p = (Elf64_Rel *) (buf + off);
        string name;
        if(sym_data ) {
            size_t index = p->r_sym - 1;
            if(index < sym_data_entry_count) {
                name = sym_data[index].name;
            }
        } else if (sym_data_64) {
            size_t index = p->r_sym - 1;
            if(index < sym_data_64_entry_count) {
                name = sym_data_64[index].name;
            }
        }
        if (name.empty()) {
            name = "<no name>";
        }
        cout << IToHex0N(p->r_offset,10);
        cout << " " << IToHex0N(add,10);
        cout << " " << LeftAlign(26,get_reloc_type_names(ELF64_R_TYPE(p->r_info)));
        cout << " " << BracketSurround(IToDec(ELF64_R_SYM(p->r_info),3))
        cout << " " << name;
        cout << endl;
#endif
    }
#endif /* HAVE_ELF64_GETEHDR */
}

static void
print_reloc_information_32(int section_no, Dwarf_Small * buf,
    Dwarf_Unsigned size, Elf64_Xword type,
    const char **scn_names, int scn_names_count)
{
/*  Include support for Elf32_Rel and Elf32_Rela */
    Dwarf_Unsigned add = 0;
    Dwarf_Half rel_size = SHT_RELA == type ?
        sizeof(Elf32_Rela) : sizeof(Elf32_Rel);
    Dwarf_Unsigned off = 0;

    cout << endl;
    cout <<  ((type == SHT_RELA)?sectnamesa[section_no]:
        sectnames[section_no]) << ":" << endl;
    printf("Offset     Addend     %-26s Index   Symbol Name\n","Reloc Type");
    for (off = 0; off < size; off += rel_size) {
        Elf32_Rel *p = (Elf32_Rel *) (buf + off);
        string name;
        if(sym_data) {
            size_t index = ELF32_R_SYM(p->r_info) - 1;
            if(index < sym_data_entry_count) {
                name = sym_data[index].name;
            }
        }
        /*  When the name is not available, use the
            section name as a reference for the name.*/
        if (name.empty()) {
            size_t index = ELF32_R_SYM(p->r_info) - 1;
            if(index < sym_data_entry_count) {
                SYM *sym_32 = &sym_data[index];
                if (sym_32->type == STT_SECTION  &&
                    sym_32->shndx < scn_names_count) {
                    name = scn_names[sym_32->shndx];
                }
            }
        }
        if (name.empty()) {
            name = "<no name>";
        }

        if (SHT_RELA == type) {
            Elf32_Rela *pa = (Elf32_Rela *)p;
            add = pa->r_addend;
        }
        cout << IToHex0N(p->r_offset,10);
        cout << " " << IToHex0N(add,10);
        cout << " " << LeftAlign(26,
            get_reloc_type_names(ELF32_R_TYPE(p->r_info)));
        cout << " " << BracketSurround(IToDec(ELF32_R_SYM(p->r_info),5));
        cout << " " << name;
        cout << endl;
    }
}

static SYM *
readsyms(Elf32_Sym * data, size_t num, Elf * elf, Elf32_Word link)
{
    SYM *s, *buf;
    indx_type i;

    buf = (SYM *) calloc(num, sizeof(SYM));
    if (buf == NULL) {
        return NULL;
    }
    s = buf;  /* save pointer to head of array */
    for (i = 1; i < num; i++, data++, buf++) {
        buf->indx = i;
        buf->name = (char *) elf_strptr(elf, link, data->st_name);
        buf->value = data->st_value;
        buf->size = data->st_size;
        buf->type = ELF32_ST_TYPE(data->st_info);
        buf->bind = ELF32_ST_BIND(data->st_info);
        buf->other = data->st_other;
        buf->shndx = data->st_shndx;
    }   /* end for loop */
    return (s);
}

static SYM64 *
read_64_syms(Elf64_Sym * data, size_t num, Elf * elf, Elf64_Word link)
{
#ifdef HAVE_ELF64_GETEHDR

    SYM64 *s, *buf;
    indx_type i;

    buf = (SYM64 *) calloc(num, sizeof(SYM64));
    if (buf == NULL) {
        return NULL;
    }
    s = buf;                    /* save pointer to head of array */
    for (i = 1; i < num; i++, data++, buf++) {
        buf->indx = i;
        buf->name = (char *) elf_strptr(elf, link, data->st_name);
        buf->value = data->st_value;
        buf->size = data->st_size;
        buf->type = ELF64_ST_TYPE(data->st_info);
        buf->bind = ELF64_ST_BIND(data->st_info);
        buf->other = data->st_other;
        buf->shndx = data->st_shndx;
    }                           /* end for loop */
    return (s);
#else
    return 0;
#endif /* HAVE_ELF64_GETEHDR */
}

static void *
get_scndata(Elf_Scn * fd_scn, size_t * scn_size)
{
    Elf_Data *p_data;

    p_data = 0;
    if ((p_data = elf_getdata(fd_scn, p_data)) == 0 ||
        p_data->d_size == 0) {
        return NULL;
    }
    *scn_size = p_data->d_size;
    return (p_data->d_buf);
}

/* Cleanup of malloc space (some of the pointers will be 0 here)
   so dwarfdump looks 'clean' to a malloc checker.
*/
void
clean_up_syms_malloc_data()
{
    free(sym_data);
    sym_data = 0;
    free(sym_data_64);
    sym_data_64 = 0;
    sym_data_64_entry_count = 0;
    sym_data_entry_count = 0;
}
