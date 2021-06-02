/*
  Copyright (C) 2021 David Anderson. All Rights Reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of version 2 of the GNU General
  Public License as published by the Free Software Foundation.

  This program is distributed in the hope that it would be
  useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  Further, this software is distributed without any warranty
  that it is free of the rightful claim of any third person
  regarding infringement or the like.  Any license provided
  herein, whether implied or otherwise, applies only to this
  software file.  Patent licenses, if any, provided herein
  do not apply to combinations of this program with other
  software, or any other product whatsoever.

  You should have received a copy of the GNU General Public
  License along with this program; if not, write the Free
  Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
  Boston MA 02110-1301, USA.

*/

#include "globals.h"
#include <stdio.h>
#include <stdarg.h>   /* For va_start va_arg va_list */
#include <errno.h>              /* For errno declaration. */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_ */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>             /* For exit() declaration etc. */
#endif /* HAVE_STDLIB_H */
#include <dwarf.h>
#include "libdwarf.h"
#include "common.h"
#include "esb.h"
#include "tag_common.h"
#include "dwgetopt.h"
#define DW_TSHASHTYPE long /* we are not using hash tree */
#include "dwarf_tsearch.h"
#include "attr_form.h"
#include "libdwarf_version.h"

Dwarf_Bool ellipsis = FALSE; /* So we can use dwarf_names.c */

/* Expected input format

0xffffffff
DW_AT_something (a number as seen here)
name of a form-class enum entry
...
0xffffffff
DW_AT_something_else (a number as seen here)
name of a form-class enum entry
...
0xffffffff
...

We generate a text list of numbers as a C header file.
The array there is used
#include "dwarf_tsearch.h"
by dwarfdump at runtime if attr/form checking
is requested.
The file is named dwarfump/dwarfdump-af-table.h and is intended
to be included in exactly one place in dwarfdump source.

The list is N entries (N is not limited) of
    attribute#  formclass#   std1/extended2 flag
followed by a space and the # and then names.

For example:
{0x02,10, 1},  DW_AT_location, DW_FORM_CLASS_REFERENCE, Std
with the names in a C comment block (which we do not
show quite right here). See dwarfdump/dwarfdump-af-table.h

The Standard vs Extended table indication is a rough indication.
dwarfdump will know what are extension ATtributes and
extension FORMs by the valuing being at or above
the formal AT DW_AT_lo_user and the FORM being above 0x2c
(ie the highest defined by any DWARF standard, lacking
a DW_FORM_lo_user value)

Lines beggining with a # character are ignored
by the code in dwarfdump reading this output.
Any lines commented with C comments are stripped
by the initial C pre-processor invocation.

*/

#define AF_STANDARD 1
#define AF_EXTENDED 2

static const char *usage[] = {
    "Usage: attr_form_build <options>",
    "    -i input-table-path",
    "    -o output-table-path",
    "    -s (Generate standard attr-formclass table)",
    "    -e (Generate extended attr-formclass table "
        "(common extensions))",
    ""
};

const char *program_name = 0;
char *input_name = 0;
char *output_name = 0;
int standard_flag = FALSE;
int extended_flag = FALSE;

/* process arguments */
static void
process_args(int argc, char *argv[])
{
    int c = 0;
    Dwarf_Bool usage_error = FALSE;

    program_name = argv[0];

    while ((c = dwgetopt(argc, argv, "i:o:se")) != EOF) {
        switch (c) {
        case 'i':
            input_name = dwoptarg;
            break;
        case 'o':
            output_name = dwoptarg;
            break;
        case 'e':
            extended_flag = TRUE;
            break;
        case 's':
            standard_flag = TRUE;
            break;
        default:
            usage_error = TRUE;
            break;
        }
    }

    if (usage_error || 1 == dwoptind || dwoptind != argc) {
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
}

void *attr_check_dups;
void
check_for_dup_attr(unsigned attr)
{
    Three_Key_Entry *e = 0;
    Three_Key_Entry *re = 0;
    int kres = 0;
    void *ret = 0;

    kres = make_3key(attr,0,0,1,1,0,&e);
    if (kres != DW_DLV_OK) {
        printf("FAIL malloc in check_for_dup_attr line %d\n",
            __LINE__);
        exit(1);
    }
#if 0
    ret = dwarf_tfind(e,&attr_check_dups,
        std_compare_3key_entry);
    if (ret) {
        printf("FAIL as attribute 0x%x is duplicated\n",
            attr);
        exit(1);
    }
#endif
    ret = dwarf_tsearch(e,&attr_check_dups,
        std_compare_3key_entry);
    if (!ret) {
        printf("FAIL malloc in check_for_dup_attr line %d\n",
            __LINE__);
        exit(1);
    }
    re = *(Three_Key_Entry **)ret;
    if (re != e) {
        printf("FAIL as attribute 0x%x is duplicated\n",
            attr);
        /* If we did not exit we would free e here */
        exit(1);
    }
}
int
main(int argc, char **argv)
{
    unsigned int num = 0;
    int input_eof = 0;
    unsigned current_row = 0;
    FILE * fileInp = 0;
    FILE * fileOut = 0;

    print_version_details(argv[0],FALSE);
    print_args(argc,argv);
    process_args(argc,argv);

    if (!input_name ) {
        fprintf(stderr,"Input name required, not supplied.\n");
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
    fileInp = fopen(input_name,"r");
    if (!fileInp) {
        fprintf(stderr,"Invalid input filename,"
            " could not open '%s'\n",
            input_name);
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }

    if (!output_name ) {
        fprintf(stderr,"Output name required, not supplied.\n");
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
    fileOut = fopen(output_name,"a");
    if (!fileOut) {
        fprintf(stderr,"Invalid output filename,"
            " could not open: '%s'\n",
            output_name);
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
    if ((standard_flag && extended_flag) ||
        (!standard_flag && !extended_flag)) {
        fprintf(stderr,"Invalid table type\n");
        fprintf(stderr,"Choose -e  or -s .\n");
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }

    input_eof = read_value(&num,fileInp);       /* 0xffffffff */
    if (IS_EOF == input_eof) {
        bad_line_input("Empty input file");
    }
    if (num != MAGIC_TOKEN_VALUE) {
        bad_line_input("Expected 0xffffffff");
    }
    if (standard_flag) {
        fprintf(fileOut,"/* Generated table, do not edit. */\n");
        fprintf(fileOut,"/* Generated sourcedate %s */\n",
            DW_VERSION_DATE_STR );
        fprintf(fileOut,"\n");
        fprintf(fileOut,"%s\n",
            "#ifndef DWARFDUMP_AF_TABLE_H");
        fprintf(fileOut,"%s\n",
            "#define DWARFDUMP_AF_TABLE_H");
        fprintf(fileOut,"\n");

        fprintf(fileOut,"%s\n",
            "#ifdef __cplusplus");
        fprintf(fileOut,"%s\n", "extern \"C\" {");
        fprintf(fileOut,"%s\n",
            "#endif /* __cplusplus */");

        fprintf(fileOut,"struct af_table_s {\n");
        fprintf(fileOut,"    Dwarf_Half attr;\n");
        fprintf(fileOut,"    Dwarf_Half formclass;\n");
        fprintf(fileOut,"    unsigned char section;\n");
        fprintf(fileOut,"}  attr_formclass_table[] = {\n");
    }
    while (!feof(stdin)) {
        unsigned int attr = 0;

        input_eof = read_value(&attr,fileInp);
        if (IS_EOF == input_eof) {
            /* Reached normal eof */
            break;
        }
        check_for_dup_attr(attr);
        input_eof = read_value(&num,fileInp);
        if (IS_EOF == input_eof) {
            bad_line_input("Not terminated correctly..");
        }
        while (num != MAGIC_TOKEN_VALUE) {
            int res = 0;
            const char *name  = 0;

            fprintf(fileOut,"{0x%02x,%2u,%d},",
                attr,num,
                standard_flag? AF_STANDARD:AF_EXTENDED);
            res = dwarf_get_AT_name(attr,&name);
            if (res != DW_DLV_OK) {
                printf("Unknown attribute number of 0x%x,"
                    " Giving up\n",num);
                exit(1);
            }
            fprintf(fileOut,"/*%s ",name);
            res = dwarf_get_FORM_CLASS_name(num,&name);
            if (res != DW_DLV_OK) {
                printf("Unknown form class number of 0x%x,"
                    " Giving up\n",num);
                exit(1);
            }
            fprintf(fileOut,"%s ",name);
            fprintf(fileOut,"%s*/\n",
                standard_flag?"Std":"Ext");
            input_eof = read_value(&num,fileInp);
            if (IS_EOF == input_eof) {
                bad_line_input("Not terminated correctly.");
            }
        }
        ++current_row;
    }
    if (extended_flag) {
        fprintf(fileOut,"{ 0,0,0 }\n");
        fprintf(fileOut,"}; /* end af_table extended */\n");
        fprintf(fileOut,"%s\n",
            "#ifdef __cplusplus");
        fprintf(fileOut,"%s\n",
            "extern \"C\" {");
        fprintf(fileOut,"%s\n",
            "#endif /* __cplusplus */");
        fprintf(fileOut,"%s\n",
            "#endif /* DWARFDUMP_AF_TABLE_H */");
    } else {
        fprintf(fileOut,"/* end af_table standard */\n");
    }
    dwarf_tdestroy(attr_check_dups,free_func_3key_entry);
    attr_check_dups = 0;
    fclose(fileInp);
    fclose(fileOut);
    return (0);
}
/* A fake so we can use dwarf_names.c */
void print_error (UNUSEDARG Dwarf_Debug dbg,
    UNUSEDARG const char * msg,
    UNUSEDARG int res,
    UNUSEDARG Dwarf_Error localerr)
{
}
