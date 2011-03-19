/* 
  Copyright 2011 David Anderson. All rights reserved.

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

*/

/* The address of the Free Software Foundation is
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA 02110-1301, USA.  
 * SGI has moved from the Crittenden Lane address.
 */

#include "globals.h"
#include <string>
#include "uri.h"
#include <stdio.h>
#include <ctype.h>
using std::string;

static std::string
xchar(int c)
{
     char buf[10];
     snprintf(buf, sizeof(buf),"%%%02x",c);
     return buf;
}
/* Translate dangerous or hard to see characters to safe ones.
   Do not empty the 'out' string.
   This is not fast.  A table would be a better approach.
*/
void
translate_to_uri(const char * filename, string &out)
{
    const char *cp = filename;
    char buf[8];
    for(  ; *cp; ++cp) {
       
       char c = (unsigned char)*cp;
       if(isalnum(c)  || c == ' ') {
            /* We let the space character print as space since
               lots of files are named that way in Mac and Windows.
            */
            out.push_back(c);
            continue;
       }
       if(isspace(c) || c == 0x7f) {
           string b = xchar(c);
           out.append(b);
           continue;
       }
       if(c >= 0x01 && c <=  0x20 ) {
           /* ASCII control characters. */
           string b = xchar(c);
           out.append(b);
           continue;
       }
       if(c == '\'' || c == '\"' || c == '%' || c == ';' ) {
           string b = xchar(c);
           out.append(b);
           continue;
       }
       if(c == 0xa0 || c == 0xff ) {
           string b = xchar(c);
           out.append(b);
           continue;
       }
       /* We are allowing other iso 8859 characters through unchanged. */
       out.push_back(c);
    }
}

#ifdef TEST

unsigned errcnt = 0;

static void
mytest(const std::string & in,const std::string & expected,int testnum)
{
     string out;
     translate_to_uri(in.c_str(), out);
     if(expected !=  out) {
         printf(" Fail test %d expected %s got %s\n",testnum,expected.c_str(),out.c_str());
         ++errcnt;
     }
}


int 
main()
{
    mytest("aaa","aaa",1);
    mytest(" bc","%20bc",2);
    mytest(" bc\n","%20bc%0a",2);
    mytest(" bc\r","%20bc%0d",2);
    mytest(" \x01","%20%01",2);
    if(errcnt) {
        printf("uri errcount ",errcnt);
    }
    return errcnt? 1:0;
}
#endif

