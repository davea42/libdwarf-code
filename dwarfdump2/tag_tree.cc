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
#include <stdlib.h>   /* For exit() declaration etc. */
#include <errno.h>    /* For errno declaration. */
#include <unistd.h>   /* For getopt. */

#include "globals.h"
#include "naming.h"
#include "common.h"
#include "tag_common.h"
using std::cout;
using std::cerr;
using std::endl;
using std::string;

unsigned int tag_tree_combination_table[TAG_TABLE_ROW_MAXIMUM][TAG_TABLE_COLUMN_MAXIMUM];

bool ellipsis = false; /* So we can use dwarf_names.c */

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
    "Usage: tag_tree_build <options>\n",
    "options:\t-t\tGenerate Tags table\n",
    "    -i Input-file-path\n",
    "    -o Output-table-path\n",
    "    -e   (Want Extended table (common extensions))\n",
    "    -s   (Want Standard table)\n",
    ""
};

static std::string input_name;
static std::string output_name;
bool extended_flag = false;
bool standard_flag = false;

static void
process_args(int argc, char *argv[])
{
    int c = 0;
    bool usage_error = false;

    while ((c = getopt(argc, argv, "i:o:es")) != EOF) {
        switch (c) {
        case 'i':
            input_name = optarg;
            break;
        case 'o':
            output_name = optarg;
            break;
        case 'e':
            extended_flag = true;
            break;
        case 's':
            standard_flag = true;
            break;
        default:
            usage_error = true;
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


    print_version_details(argv[0],false);
    process_args(argc,argv);
    print_args(argc,argv);

    if (input_name.empty() ) {
        cerr << "Input name required, not supplied." << endl;
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
    FILE *fileInp = fopen(input_name.c_str(),"r");
    if (!fileInp) {
        cerr << "Invalid input filename, could not open '" << 
            input_name << "'" << endl;
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }


    if (output_name.empty() ) {
        cerr << "Output name required, not supplied." << endl;
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
    FILE *fileOut = fopen(output_name.c_str(),"w");
    if (!fileOut) {
        cerr << "Invalid output filename, could not open: '" <<
            output_name <<  "'" << endl;
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
    if ((standard_flag && extended_flag) || (!standard_flag && !extended_flag)) {
        cerr <<"Invalid table type" << endl;
        cerr << "Choose -e  or -s ." << endl;
        print_usage_message(argv[0],usage);
        exit(FAILED);
    }
    unsigned int  table_rows = 0;
    unsigned int table_columns = 0;
    if(standard_flag) {
        table_rows = STD_TAG_TABLE_ROWS;
        table_columns = STD_TAG_TABLE_COLUMNS;
    } else {
        table_rows = EXT_TAG_TABLE_ROWS;
        table_columns = EXT_TAG_TABLE_COLS;
    }

    unsigned int num = 0;
    int input_eof = read_value(&num,fileInp);       /* 0xffffffff */
    if (IS_EOF == input_eof) {
        bad_line_input("Empty input file");
    }
    if (num != MAGIC_TOKEN_VALUE) {
        bad_line_input("Expected 0xffffffff");
    }

    unsigned int current_row = 0;
    while (!feof(stdin)) {
        unsigned int tag = 0;
        unsigned int nTagLoc = 0;

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
                unsigned idx = num / BITS_PER_WORD;
                unsigned bit = num % BITS_PER_WORD;
    
                if (idx >= table_columns) {
                    cout << "Want column " << idx << ", have only " <<
                        table_columns << endl;
                    bad_line_input("too many TAGs: table incomplete.");
                }
                tag_tree_combination_table[tag][idx] |= (1 << bit);
            } else {
                if(nTagLoc >= table_columns) {
                    cout << "Attempting to use column " << nTagLoc <<
                        ", max is " << table_columns << endl;
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

    for (unsigned i = 0; i < table_rows; i++) {
        bool printonerr=false;
        if (standard_flag) {
            fprintf(fileOut,"/* %d %-37s*/\n",i, 
                get_TAG_name(i,printonerr).c_str());
        } else {
            fprintf(fileOut,"/* %u %-37s*/\n",
                tag_tree_combination_table[i][0],
                get_TAG_name(
                    tag_tree_combination_table[i][0],printonerr).c_str());
        }
        fprintf(fileOut,"    { ");
        for(unsigned j = 0; j < table_columns; ++j ) {
            fprintf(fileOut,"0x%08x,",tag_tree_combination_table[i][j]);
        }
        fprintf(fileOut,"},\n");

    }
    fprintf(fileOut,"\n#define MAX_CHECKED_TAG_ID 0x%2x\n",STD_TAG_TABLE_ROWS);
    fprintf(fileOut,"};\n");
    fclose(fileInp);
    fclose(fileOut);
    return (0);
}

/* A fake so we can use dwarf_names.c */
void print_error (Dwarf_Debug dbg, const string& msg,int res, Dwarf_Error err)
{
}

