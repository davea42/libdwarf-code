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
#include "esb.h"
#include "uri.h"
#include <stdio.h>
#include <ctype.h>

static char *
xchar(int c, char *buf, int size)
{
     snprintf(buf, size,"%%%02x",c);
     return buf;
}
/* Translate dangerous or hard to see characters to safe ones.
   This is not fast. A table would be a bette approach.
*/
void
translate_to_uri(const char * filename, struct esb_s *out)
{
    const char *cp = filename;
    char buf[8];
    for(  ; *cp; ++cp) {
       char v[2];
       int c = (unsigned char)*cp;
       if(isalnum(c) || c == ' ' ) {
            /* We let the space character print as space since
               lots of files are named that way in Mac and Windows.
            */
            
            v[0] = c;
            v[1] = 0;
            esb_append(out,v);
            continue;
       }
       if(isspace(c) || c == 0x7f) {
           char *b = xchar(c,buf,sizeof(buf));
           esb_append(out,b);
           continue;
       }
       if(c >= 0x01 && c <=  0x20 ) {
           /* ASCII control characters. */
           char *b = xchar(c,buf,sizeof(buf));
           esb_append(out,b);
           continue;
       }
       if(c == '\'' || c == '\"' || c == '%' || c == ';' ) {
           char *b = xchar(c,buf,sizeof(buf));
           esb_append(out,b);
           continue;
       }
       if(c == 0xa0 || c == 0xff ) {
           char *b = xchar(c,buf,sizeof(buf));
           esb_append(out,b);
           continue;
       }
       /* We are allowing other iso 8859 characters through unchanged. */
       v[0] = c;
       v[1] = 0;
       esb_append(out,v);
    }
}

#ifdef TEST

unsigned errcnt = 0;

static void
mytest(char *in,char *expected,int testnum)
{
     struct esb_s out;
     esb_constructor(&out);
     translate_to_uri(in, &out);
     if(strcmp(expected, esb_get_string(&out))) {
         printf(" Fail test %d expected %s got %s\n",testnum,expected,esb_get_string(&out));
         ++errcnt;
     }
     esb_destructor(&out);
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

