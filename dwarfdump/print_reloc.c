/* 
  Copyright (C) 2000,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2007 David Anderson. All Rights Reserved.

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

/* The address of the Free Software Foundation is
   Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
   Boston, MA 02110-1301, USA.
   SGI has moved from the Crittenden Lane address.
*/




#include "globals.h"


#define DW_SECTION_REL_DEBUG_INFO    0
#define DW_SECTION_REL_DEBUG_LINE    1
#define DW_SECTION_REL_DEBUG_PUBNAME 2
#define DW_SECTION_REL_DEBUG_ABBREV  3
#define DW_SECTION_REL_DEBUG_ARANGES 4
#define DW_SECTION_REL_DEBUG_FRAME   5
#define DW_SECTION_REL_DEBUG_NUM     6

#define DW_SECTNAME_REL_DEBUG_INFO    ".rel.debug_info"
#define DW_SECTNAME_REL_DEBUG_LINE    ".rel.debug_line"
#define DW_SECTNAME_REL_DEBUG_PUBNAME ".rel.debug_pubname"
#define DW_SECTNAME_REL_DEBUG_ABBREV  ".rel.debug_abbrev"
#define DW_SECTNAME_REL_DEBUG_ARANGES ".rel.debug_aranges"
#define DW_SECTNAME_REL_DEBUG_FRAME   ".rel.debug_frame"

#define STRING_FOR_DUPLICATE " duplicate"
#define STRING_FOR_NULL      " null"

static char *sectnames[] = {
    DW_SECTNAME_REL_DEBUG_INFO,
    DW_SECTNAME_REL_DEBUG_LINE,
    DW_SECTNAME_REL_DEBUG_PUBNAME,
    DW_SECTNAME_REL_DEBUG_ABBREV,
    DW_SECTNAME_REL_DEBUG_ARANGES,
    DW_SECTNAME_REL_DEBUG_FRAME,
};

static char *error_msg_duplicate[] = {
    DW_SECTNAME_REL_DEBUG_INFO STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_LINE STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_PUBNAME STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_ABBREV STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_ARANGES STRING_FOR_DUPLICATE,
    DW_SECTNAME_REL_DEBUG_FRAME STRING_FOR_DUPLICATE,
};

static char *error_msg_null[] = {
    DW_SECTNAME_REL_DEBUG_INFO STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_LINE STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_PUBNAME STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_ABBREV STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_ARANGES STRING_FOR_NULL,
    DW_SECTNAME_REL_DEBUG_FRAME STRING_FOR_NULL,
};

#define SECT_DATA_SET(x) { \
            if (sect_data[(x)].buf != NULL) { \
                print_error(dbg, error_msg_duplicate[(x)],DW_DLV_OK, err); \
            } \
            if ((data = elf_getdata(scn, 0)) == NULL || data->d_size == 0) { \
                print_error(dbg, error_msg_null[(x)],DW_DLV_OK, err); \
            } \
            sect_data[(x)].buf = data -> d_buf; \
            sect_data[(x)].size = data -> d_size; \
            }

static char *reloc_type_names[] = {
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

/*
        return valid reloc type names. 
        if buf is used, it is static, so beware it
        will be overrwritten by the next call.
*/
static char *
get_reloc_type_names(int index)
{
    static char buf[100];
    int arysiz = sizeof(reloc_type_names) / sizeof(char *);
    char *retval;

    if (index < 0 || index >= arysiz) {
        sprintf(buf, "reloc type %d unknown", (int) index);
        retval = buf;
    } else {
        retval = reloc_type_names[index];
    }
    return retval;
}


static struct {
    Dwarf_Small *buf;
    Dwarf_Unsigned size;
} sect_data[DW_SECTION_REL_DEBUG_NUM];

#ifndef HAVE_ELF64_GETEHDR
#define Elf64_Addr  long
#define Elf64_Word  unsigned long
#define Elf64_Xword unsigned long
#define Elf64_Sym   long
#endif

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
                                       Dwarf_Unsigned size);
static void print_reloc_information_32(int section_no,
                                       Dwarf_Small * buf,
                                       Dwarf_Unsigned size);
static SYM *readsyms(Elf32_Sym * data, size_t num, Elf * elf,
                     Elf32_Word link);
static SYM64 *read_64_syms(Elf64_Sym * data, size_t num, Elf * elf,
                           Elf64_Word link);
static void *get_scndata(Elf_Scn * fd_scn, size_t * scn_size);
static void print_relocinfo_64(Dwarf_Debug dbg, Elf * elf);
static void print_relocinfo_32(Dwarf_Debug dbg, Elf * elf);

static SYM   *sym_data;
static SYM64 *sym_data_64;
static long   sym_data_entry_count;
static long   sym_data_64_entry_count;

void
print_relocinfo(Dwarf_Debug dbg)
{
    Elf *elf;
    char *endr_ident;
    int is_64bit;
    int res;
    int i;

    for (i = 0; i < DW_SECTION_REL_DEBUG_NUM; i++) {
        sect_data[i].buf = 0;
        sect_data[i].size = 0;
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
    Elf_Data *data;
    Elf64_Ehdr *ehdr64;
    Elf64_Shdr *shdr64;
    char *scn_name;
    int i;
    Elf64_Sym *sym_64 = 0;

    if ((ehdr64 = elf64_getehdr(elf)) == NULL) {
        print_error(dbg, "DW_ELF_GETEHDR_ERROR", DW_DLV_OK, err);
    }

    while ((scn = elf_nextscn(elf, scn)) != NULL) {

        if ((shdr64 = elf64_getshdr(scn)) == NULL) {
            print_error(dbg, "DW_ELF_GETSHDR_ERROR", DW_DLV_OK, err);
        }
        if ((scn_name =
             elf_strptr(elf, ehdr64->e_shstrndx, shdr64->sh_name))
            == NULL) {
            print_error(dbg, "DW_ELF_STRPTR_ERROR", DW_DLV_OK, err);
        }
        if (shdr64->sh_type == SHT_SYMTAB) {
            size_t sym_size = 0;
            size_t count = 0;

            if ((sym_64 =
                 (Elf64_Sym *) get_scndata(scn, &sym_size)) == NULL) {
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
        } else if (strncmp(scn_name, ".rel.debug_", 11))
            continue;
        else if (strcmp(scn_name, ".rel.debug_info") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_INFO)
        } else if (strcmp(scn_name, ".rel.debug_line") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_LINE)
        } else if (strcmp(scn_name, ".rel.debug_pubname") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_PUBNAME)
        } else if (strcmp(scn_name, ".rel.debug_aranges") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_ARANGES)
        } else if (strcmp(scn_name, ".rel.debug_abbrev") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_ABBREV)
        } else if (strcmp(scn_name, ".rel.debug_frame") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_FRAME)
        }
    }                           /* while */

    for (i = 0; i < DW_SECTION_REL_DEBUG_NUM; i++) {
        if (sect_data[i].buf != NULL && sect_data[i].size > 0) {
            print_reloc_information_64(i, sect_data[i].buf,
                                       sect_data[i].size);
        }
    }
#endif
}

static void
print_relocinfo_32(Dwarf_Debug dbg, Elf * elf)
{
    Elf_Scn *scn = NULL;
    Elf_Data *data;
    Elf32_Ehdr *ehdr32;
    Elf32_Shdr *shdr32;
    char *scn_name;
    int i;
    Elf32_Sym  *sym = 0;

    if ((ehdr32 = elf32_getehdr(elf)) == NULL) {
        print_error(dbg, "DW_ELF_GETEHDR_ERROR", DW_DLV_OK, err);
    }
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        if ((shdr32 = elf32_getshdr(scn)) == NULL) {
            print_error(dbg, "DW_ELF_GETSHDR_ERROR", DW_DLV_OK, err);
        }
        if ((scn_name =
             elf_strptr(elf, ehdr32->e_shstrndx, shdr32->sh_name)
            ) == NULL) {
            print_error(dbg, "DW_ELF_STRPTR_ERROR", DW_DLV_OK, err);
        }
        if (shdr32->sh_type == SHT_SYMTAB) {
            size_t sym_size = 0;
            size_t count = 0;

            if ((sym =
                 (Elf32_Sym *) get_scndata(scn, &sym_size)) == NULL) {
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
        } else if (strncmp(scn_name, ".rel.debug_", 11))
            continue;
        else if (strcmp(scn_name, ".rel.debug_info") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_INFO)
        } else if (strcmp(scn_name, ".rel.debug_line") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_LINE)
        } else if (strcmp(scn_name, ".rel.debug_pubname") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_PUBNAME)
        } else if (strcmp(scn_name, ".rel.debug_aranges") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_ARANGES)
        } else if (strcmp(scn_name, ".rel.debug_abbrev") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_ABBREV)
        } else if (strcmp(scn_name, ".rel.debug_frame") == 0) {
            SECT_DATA_SET(DW_SECTION_REL_DEBUG_FRAME)
        }
    }                           /* while */

    for (i = 0; i < DW_SECTION_REL_DEBUG_NUM; i++) {
        if (sect_data[i].buf != NULL && sect_data[i].size > 0) {
            print_reloc_information_32(i, sect_data[i].buf,
                                       sect_data[i].size);
        }
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
                           Dwarf_Unsigned size)
{
    Dwarf_Unsigned off;

    printf("\n%s:\n", sectnames[section_no]);
#if HAVE_ELF64_GETEHDR
    for (off = 0; off < size; off += sizeof(Elf64_Rel)) {
#if HAVE_ELF64_R_INFO
        /* This works for the Elf64_Rel in linux */
        Elf64_Rel *p = (Elf64_Rel *) (buf + off);
        char *name = "<no name>";
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

        printf("%5lu\t<%3ld> %-34s%s\n",
            (unsigned long int) (p->r_offset),
            (long)ELF64_R_SYM(p->r_info),
            name,
            get_reloc_type_names(ELF64_R_TYPE(p->r_info)));
#else
        /* sgi/mips -64 does not have r_info in the 64bit relocations,
           but seperate fields, with 3 types, actually. Only one of
           which prints here, as only one really used with dwarf */
        Elf64_Rel *p = (Elf64_Rel *) (buf + off);
        char *name = "<no name>";
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

        printf("%5" DW_PR_DUu "\t<%3ld> %-34s%s\n",
            (Dwarf_Unsigned) (p->r_offset),
            (long)p->r_sym,
            name,
            get_reloc_type_names(p->r_type));
#endif
    }
#endif /* HAVE_ELF64_GETEHDR */
}

static void
print_reloc_information_32(int section_no, Dwarf_Small * buf,
                           Dwarf_Unsigned size)
{
    Dwarf_Unsigned off;

    printf("\n%s:\n", sectnames[section_no]);
    for (off = 0; off < size; off += sizeof(Elf32_Rel)) {
        Elf32_Rel *p = (Elf32_Rel *) (buf + off);
        char *name = "<no name>";
        if(sym_data) {
           size_t index = ELF32_R_SYM(p->r_info) - 1;
           if(index < sym_data_entry_count) {
               name = sym_data[index].name;
           }
        }

        printf("%5lu\t<%3d> %-34s%s\n",
               (unsigned long int) (p->r_offset),
               ELF32_R_SYM(p->r_info),
               name,
               get_reloc_type_names(ELF32_R_TYPE(p->r_info)));
    }
}

static SYM *
readsyms(Elf32_Sym * data, size_t num, Elf * elf, Elf32_Word link)
{
    SYM *s, *buf;
    indx_type i;

    if ((buf = (SYM *) calloc(num, sizeof(SYM))) == NULL) {
        return NULL;
    }
    s = buf;                    /* save pointer to head of array */
    for (i = 1; i < num; i++, data++, buf++) {
        buf->indx = i;
        buf->name = (char *) elf_strptr(elf, link, data->st_name);
        buf->value = data->st_value;
        buf->size = data->st_size;
        buf->type = ELF32_ST_TYPE(data->st_info);
        buf->bind = ELF32_ST_BIND(data->st_info);
        buf->other = data->st_other;
        buf->shndx = data->st_shndx;
    }                           /* end for loop */
    return (s);
}

static SYM64 *
read_64_syms(Elf64_Sym * data, size_t num, Elf * elf, Elf64_Word link)
{
#ifdef HAVE_ELF64_GETEHDR

    SYM64 *s, *buf;
    indx_type i;

    if ((buf = (SYM64 *) calloc(num, sizeof(SYM64))) == NULL) {
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
