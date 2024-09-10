/*
Copyright (C) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
Portions Copyright 2009-2012 SN Systems Ltd. All rights reserved.
Portions Copyright 2009-2017 David Anderson. All rights reserved.

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

#include <config.h>

#include <ctype.h>  /* isspace() */
#include <stdarg.h> /* va_end() va_list va_start() */
#include <stdio.h>  /* FILE stderr fprintf() vfprintf() */
#include <stdlib.h> /* exit() */
#include <string.h> /* strlen() strncmp() */

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "dd_defined_types.h"
#include "dd_checkutil.h"
#include "dd_glflags.h"
#include "dd_globals.h"
#include "dd_common.h"
#include "dd_tag_common.h"
#include "dd_getopt.h"
#include "dd_safe_strcpy.h"
#include "dd_minimal.h"

void dd_minimal_count_global_error(void) {}
Dwarf_Half tag_tree_combination_table[TAG_TABLE_ROW_MAXIMUM]
    [TAG_TABLE_COLUMN_MAXIMUM];

const char * program_name;

Dwarf_Bool ellipsis = FALSE; /* So we can use dwarf_names.c */

/* Expected input format

0xffffffff
value of a tag
value of a standard tag that may be a child ofthat tag
...
0xffffffff
value of a tag
value of a standard tag that may be a child ofthat tag
...
0xffffffff
...

No commentary allowed, no symbols, just numbers.
Blank lines are allowed and are dropped.

*/

static const char *usage[] = {
    "Usage: tag_tree_build <options>",
    "options:\t-t\tGenerate Tags table",
    "    -i Input-file-path",
    "    -o Output-table-path",
    "    -e   (Want Extended table (common extensions))",
    "    -s   (Want Standard table)",
    ""
};

static char *input_name = 0;
static char *output_name = 0;
int extended_flag = FALSE;
int standard_flag = FALSE;

static void
process_args(int argc, char *argv[])
{
    int c = 0;
    Dwarf_Bool usage_error = FALSE;

    program_name = argv[0];

    while ((c = dwgetopt(argc, argv, "i:o:es")) != EOF) {
        switch (c) {
        case 'i':
            input_name = (char *)strdup(dwoptarg);
            break;
        case 'o':
            output_name = (char *)strdup(dwoptarg);
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
        print_usage_message(usage);
        exit(EXIT_FAILURE);
    }
}
/*  New naming routine May 2016.
    Instead of directly calling dwarf_get_*()
    When bad tag/attr numbers are presented we return
    a warning string through the pointer.
    The thought is that eventually someone will notice the error.
    It might, of course, be better to emit an error message
    and stop. */
static void
ta_get_TAG_name(unsigned int tagnum,const char **nameout)
{
    int res = 0;

    res = dwarf_get_TAG_name(tagnum,nameout);
    if (res == DW_DLV_OK) {
        return;
    }
    if (res == DW_DLV_NO_ENTRY) {
        *nameout = "<no name known for the TAG>";
        return;
    }
    *nameout = "<ERROR so no name known for the TAG>";
    return;
}

/*  these are used in assignments. */
static unsigned maxrowused = 0;
static unsigned maxcolused = 0;

/*  The argument sizes are declared in tag_common.h, so
    the max usable is toprow-1 and topcol-1. */
static void
check_unused_combo(unsigned toprow,unsigned topcol)
{
    if ((toprow-1) !=  maxrowused) {
        printf("Providing for %u rows but used 0-%u\n",
            toprow,maxrowused);
        printf("Giving up\n");
        exit(EXIT_FAILURE);
    }
    if ((topcol-1) != maxcolused) {
        printf("Providing for %u cols but used 0-%u\n",
            topcol,maxcolused);
        printf("Giving up\n");
        exit(EXIT_FAILURE);
    }
}

static void
validate_row_col(const char *position,
    unsigned crow,
    unsigned ccol,
    unsigned maxrow,
    unsigned maxcol)
{
    if (crow >= TAG_TABLE_ROW_MAXIMUM) {
        printf("error generating row in tag-attr array, %s "
            "current row: %u  size of static array decl: %u\n",
            position,crow, TAG_TABLE_ROW_MAXIMUM);
        exit(EXIT_FAILURE);
    }

    if (crow >= maxrow) {
        printf("error generating row in tree tag array, %s "
            "current row: %u  max allowed: %u\n",
            position,crow,maxrow-1);
        exit(EXIT_FAILURE);
    }
    if (ccol >= TAG_TABLE_COLUMN_MAXIMUM) {
        printf("error generating column in tag-attr array, %s "
            "current col: %u  size of static array decl: %u\n",
            position,ccol, TAG_TABLE_COLUMN_MAXIMUM);
        exit(EXIT_FAILURE);
    }

    if (ccol >= maxcol) {
        printf("error generating column in tree tag array, %s "
            "current row: %u  max allowed: %u\n",
            position,ccol,maxcol-1);
        exit(EXIT_FAILURE);
    }
    if (crow > maxrowused) {
        maxrowused = crow;
    }
    if (ccol > maxcolused) {
        maxcolused = ccol;
    }
    return;
}

int
main(int argc, char **argv)
{
    unsigned u = 0;
    unsigned int num = 0;
    int input_eof = 0;
    unsigned table_rows = 0;
    unsigned table_columns = 0;
    unsigned current_row = 0;
    FILE *fileInp = 0;
    FILE *fileOut = 0;

    print_version_details(argv[0]);
    print_args(argc,argv);
    process_args(argc,argv);

    if (!input_name ) {
        fprintf(stderr,"Input name required, not supplied.\n");
        print_usage_message(usage);
        exit(EXIT_FAILURE);
    }
    fileInp = fopen(input_name,"r");
    if (!fileInp) {
        fprintf(stderr,"Invalid input filename,"
            " could not open '%s'\n",
            input_name);
        print_usage_message(usage);
        exit(EXIT_FAILURE);
    }

    if (!output_name ) {
        fprintf(stderr,"Output name required, not supplied.\n");
        print_usage_message(usage);
        exit(EXIT_FAILURE);
    }
printf("dadebug open %s\n",output_name);
    fileOut = fopen(output_name,"w");
    if (!fileOut) {
        fprintf(stderr,"Invalid output filename,"
            " could not open: '%s'\n",
            output_name);
        print_usage_message(usage);
        exit(EXIT_FAILURE);
    }
    if ((standard_flag && extended_flag) ||
        (!standard_flag && !extended_flag)) {
        fprintf(stderr,"Invalid table type\n");
        fprintf(stderr,"Choose -e  or -s .\n");
        print_usage_message(usage);
        exit(EXIT_FAILURE);
    }
    if (standard_flag) {
        table_rows = STD_TAG_TABLE_ROWS;
        table_columns = STD_TAG_TABLE_COLUMNS;
    } else {
        table_rows = EXT_TAG_TABLE_ROWS;
        table_columns = EXT_TAG_TABLE_COLS;
    }

    input_eof = read_value(&num,fileInp);       /* 0xffffffff */
    if (IS_EOF == input_eof) {
        bad_line_input("Empty input file");
    }
    if (num != MAGIC_TOKEN_VALUE) {
        bad_line_input("Expected 0xffffffff");
    }

    /*  Generate main header, regardless of contents */
    fprintf(fileOut,"/* Generated code, do not edit. */\n");
    fprintf(fileOut,"/* Generated for source version %s */\n",
        PACKAGE_VERSION );
    fprintf(fileOut,"\n/* BEGIN FILE */\n\n");

    while (!feof(stdin)) {
        unsigned int tag = 0;
        unsigned nTagLoc = 0;

        input_eof = read_value(&tag,fileInp);
        if (IS_EOF == input_eof) {
            /* Reached normal eof */
            break;
        }
        if (current_row >= table_rows ) {
            bad_line_input(
                "tag value exceeds standard table size");
        }
        validate_row_col("Reading tag",current_row,0,
            table_rows,table_columns);
        tag_tree_combination_table[current_row][0] = tag;
        input_eof = read_value(&num,fileInp);
        if (IS_EOF == input_eof) {
            bad_line_input("Not terminated correctly..");
        }
        nTagLoc = 1;

        while (num != MAGIC_TOKEN_VALUE) {
            {
                if (nTagLoc >= table_columns) {
                    printf("Attempting to use column %d, max is %d\n",
                        nTagLoc,table_columns);
                    bad_line_input(
                        "too many subTAGs, table incomplete.");
                }
                validate_row_col("Update tagloc",current_row,nTagLoc,
                    table_rows,table_columns);
                tag_tree_combination_table[current_row][nTagLoc] =
                    num;
                nTagLoc++;
            }

            input_eof = read_value(&num,fileInp);
            if (IS_EOF == input_eof) {
                bad_line_input("Not terminated correctly.");
            }
        }
        ++current_row; /* for extended table */
    }

    check_unused_combo(table_rows, table_columns);
    if (standard_flag) {
        fprintf(fileOut,"#define TAG_TREE_COLUMN_COUNT %d\n\n",
            table_columns);
        fprintf(fileOut,"#define TAG_TREE_ROW_COUNT %d\n\n",
            table_rows);
        fprintf(fileOut,
            "static Dwarf_Half tag_tree_combination_table\n");
        fprintf(fileOut,
            "    [TAG_TREE_ROW_COUNT][TAG_TREE_COLUMN_COUNT] = {\n");
    } else {
        fprintf(fileOut,
            "#define TAG_TREE_EXT_COLUMN_COUNT %d\n\n",
            table_columns);
        fprintf(fileOut,
            "#define TAG_TREE_EXT_ROW_COUNT %d\n\n",
            table_rows);
        fprintf(fileOut,"/* Common extensions */\n");
        fprintf(fileOut,
            "static Dwarf_Half tag_tree_combination_ext_table\n");
        fprintf(fileOut,
            "    [TAG_TREE_EXT_ROW_COUNT]"
            "[TAG_TREE_EXT_COLUMN_COUNT]"
            " = {\n");
    }
printf("dadebug header printed\n");
    for (u = 0; u < table_rows; u++) {
        unsigned j = 0;
        const char *name = 0;
        {
            unsigned k = tag_tree_combination_table[u][0];
            ta_get_TAG_name(k,&name);
            fprintf(fileOut,"/* 0x%02x - %-37s*/\n", k, name);
        }
        fprintf(fileOut,"    { ");
        for (j = 0; j < table_columns; ++j ) {
            fprintf(fileOut,"0x%08x,",
                tag_tree_combination_table[u][j]);
        }
        fprintf(fileOut,"},\n");

    }
    fprintf(fileOut,"};\n");
    fprintf(fileOut,"\n/* END FILE */\n");
    fclose(fileInp);
    fclose(fileOut);
    return (0);
}

/* A fake so we can use dwarf_names.c */
void print_error (Dwarf_Debug dbg,
    const char *msg,
    int res,
    Dwarf_Error localerr)
{
    (void)dbg;
    (void)msg;
    (void)res;
    (void)localerr;;
}
