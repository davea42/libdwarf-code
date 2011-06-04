/* 
  Copyright (C) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2009-2010 SN Systems Ltd. All rights reserved.
  Portions Copyright 2009-2011 David Anderson. All rights reserved.

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



$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/tag_tree.c,v 1.8 2005/12/01 17:34:59 davea Exp $ */
#include <dwarf.h>
#include <stdio.h>
#include <stdlib.h>             /* For exit() declaration etc. */
#include <errno.h>              /* For errno declaration. */
#include <unistd.h>             /* For getopt. */

#include "globals.h"
#include "libdwarf.h"
#include "common.h"
#include "tag_common.h"

unsigned int tag_tree_combination_table[TAG_TABLE_ROW_MAXIMUM][TAG_TABLE_COLUMN_MAXIMUM];

string program_name;

boolean ellipsis = FALSE; /* So we can use dwarf_names.c */

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
    boolean usage_error = FALSE;

    program_name = argv[0];
  
    while ((c = getopt(argc, argv, "i:o:es")) != EOF) {
        switch (c) {
        case 'i':
            input_name = strdup(optarg);
            break;
        case 'o':
            output_name = strdup(optarg);
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

    if (usage_error || 1 == optind || optind != argc) {
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
}



int
main(int argc, char **argv)
{
    int i = 0;
    unsigned int num = 0;
    int input_eof = 0;
    int table_rows = 0;
    int table_columns = 0;
    int current_row = 0;
    FILE *fileInp = 0;
    FILE *fileOut = 0;


    print_version_details(argv[0],FALSE);
    process_args(argc,argv);
    print_args(argc,argv);

    if (!input_name ) {
        fprintf(stderr,"Input name required, not supplied.\n");
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
    fileInp = fopen(input_name,"r");
    if (!fileInp) {
        fprintf(stderr,"Invalid input filename, could not open '%s'\n",
            input_name);
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }


    if (!output_name ) {
        fprintf(stderr,"Output name required, not supplied.\n");
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
    fileOut = fopen(output_name,"w");
    if (!fileOut) {
        fprintf(stderr,"Invalid output filename, could not open: '%s'\n",
            output_name);
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
    if ((standard_flag && extended_flag) || (!standard_flag && !extended_flag)) {
        fprintf(stderr,"Invalid table type\n");
        fprintf(stderr,"Choose -e  or -s .\n"); 
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
    if(standard_flag) {
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

    while (!feof(stdin)) {
        unsigned int tag = 0;
        int nTagLoc = 0;

        input_eof = read_value(&tag,fileInp);
        if (IS_EOF == input_eof) {
            /* Reached normal eof */
            break;
        }
        if(standard_flag) {
            if (tag >= table_rows ) {
                bad_line_input("tag value exceeds standard table size");
            }
        } else {
            if(current_row >= table_rows) {
                bad_line_input("too many extended table rows.");
            }
            tag_tree_combination_table[current_row][0] = tag;
        }
        input_eof = read_value(&num,fileInp);
        if (IS_EOF == input_eof) {
            bad_line_input("Not terminated correctly..");
        }
        nTagLoc = 1;
        while (num != 0xffffffff) {
            if(standard_flag) {
                int idx = num / BITS_PER_WORD;
                int bit = num % BITS_PER_WORD;
    
                if (idx >= table_columns) {
                    fprintf(stderr,"Want column %d, have only %d\n",
                        idx,table_columns);
                    bad_line_input("too many TAGs: table incomplete.");
                }
                tag_tree_combination_table[tag][idx] |= (1 << bit);
            } else {
                if(nTagLoc >= table_columns) {
                    printf("Attempting to use colum %d, max is %d\n",
                        nTagLoc,table_columns);
                    bad_line_input("too many subTAGs, table incomplete.");
                }
                tag_tree_combination_table[current_row][nTagLoc] = num;
                nTagLoc++;
            }
            input_eof = read_value(&num,fileInp);
            if (IS_EOF == input_eof) {
                bad_line_input("Not terminated correctly.");
            }
        }
        ++current_row; /* for extended table */
    }
    fprintf(fileOut,"/* Generated code, do not edit. */\n");
    fprintf(fileOut,"/* Generated on %s  %s */\n",__DATE__,__TIME__);
    fprintf(fileOut,"\n/* BEGIN FILE */\n\n");
    if (standard_flag) {
        fprintf(fileOut,"#define TAG_TREE_COLUMN_COUNT %d\n\n",table_columns);
        fprintf(fileOut,"#define TAG_TREE_ROW_COUNT %d\n\n",table_rows);
        fprintf(fileOut,
            "static unsigned int tag_tree_combination_table"
            "[TAG_TREE_ROW_COUNT][TAG_TREE_COLUMN_COUNT] = {\n");
    } else {
        fprintf(fileOut,"#define TAG_TREE_EXT_COLUMN_COUNT %d\n\n",
            table_columns);
        fprintf(fileOut,"#define TAG_TREE_EXT_ROW_COUNT %d\n\n",table_rows);
        fprintf(fileOut,"/* Common extensions */\n");
        fprintf(fileOut,
            "static unsigned int tag_tree_combination_ext_table"
            "[TAG_TREE_EXT_ROW_COUNT][TAG_TREE_EXT_COLUMN_COUNT] = {\n");
    }

    for (i = 0; i < table_rows; i++) {
        int j = 0;
        const char *name = 0;
        if (standard_flag) {
            dwarf_get_TAG_name(i,&name);;
            fprintf(fileOut,"/* %d %-37s*/\n",i, name);
        } else {
            int k = tag_tree_combination_table[i][0];
            dwarf_get_TAG_name(i,&name);;
            fprintf(fileOut,"/* %u %-37s*/\n", k, name);
        }
        fprintf(fileOut,"    { ");
        for(j = 0; j < table_columns; ++j ) {
            fprintf(fileOut,"0x%08x,",tag_tree_combination_table[i][j]);
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
void print_error (Dwarf_Debug dbg, string msg,int res, Dwarf_Error err)
{
}

