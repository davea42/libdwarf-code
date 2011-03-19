/* 
  Copyright (C) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright (C) 2009-2010 SN Systems Ltd. All Rights Reserved.
  Portions Copyright (C) 2009-2010 David Anderson. All Rights Reserved.

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

$Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/tag_common.c,v 1.8 2008/01/23 09:47:59 davea Exp $ */

#include <dwarf.h>
#include <stdio.h>
#include <stdlib.h>  /* For exit() declaration etc. */
#include <errno.h>   /* For errno declaration. */
#include <ctype.h>   /*  For isspace() declaration */
#include <string>
#include <iostream>

#include "globals.h"
#include "naming.h"
#include "tag_common.h" 
using std::string;  
using std::cout;
using std::cerr;
using std::endl;

static int linecount = 0;
static char line_in[MAX_LINE_SIZE];


void
bad_line_input(const std::string &msg)
{
    cerr << "tag_(tree,attr) table build failed " <<
        msg << ", line "<< linecount << ": \"" << line_in << "\"" <<
        endl;
    exit(FAILED);
}

void
trim_newline(char *line, int max)
{
    char *end = line + max - 1;
    for (; *line && (line < end); ++line) {
        if (*line == '\n') {
            /* Found newline, drop it */
            *line = 0;
            return;
        }
    }
    return;
}

/*  Detect empty lines (and other lines we do not want to read) */
bool
is_skippable_line(char *pLine)
{
    bool empty = true;

    if(pLine[0] == '#') {
        // Preprocessor lines are of no interest.
        return true;
    }
    if (pLine[0] == '}') {
        // The }  is from an 'extern "C"' and not interesting.
        return true;
    }
    if(strncmp(&pLine[0],"extern \"C\"",10) == 0) {
        // It is an 'extern "C"' and not interesting.
        return true;
    }
    for (; *pLine && empty; ++pLine) {
        empty = isspace(*pLine);
    }
    return empty;
}

/* Reads a value from the text table. 
   Exits  with non-zero status 
   if the table is erroneous in some way. 
*/
int
read_value(unsigned int *outval, FILE*file)
{
    char *res = 0;
    unsigned long lval;
    char *strout = 0;
    bool bBlankLine = true; 

    ++linecount;
    *outval = 0;

    while (bBlankLine) {
        res = fgets(line_in, sizeof(line_in), file);
        if (res == 0) {
            if (ferror(file)) {
                cerr << "tag_attr: Error reading table, " <<
                    linecount << " lines read" << endl;
                exit(FAILED);
            }
            if (feof(file)) {
                return IS_EOF;
            }
            /* impossible */
            cerr  << "tag_attr: Impossible error reading table, " <<
                linecount << " lines read" << endl;
            exit(FAILED);
        }
        bBlankLine = is_skippable_line(line_in);
    }
    trim_newline(line_in, sizeof(line_in));
    errno = 0;
    lval = strtoul(line_in, &strout, 0);

    if (strout == line_in) {
        bad_line_input("bad number input!");
    }
    if (errno != 0) {
        int myerr = errno;
        cerr << "tag_attr errno " << myerr << endl;
        bad_line_input("invalid number on line");
    }
    *outval = (int) lval;
    return NOT_EOF;
}
