/* 
  Copyright (C) 2008-2010 SN Systems.  All Rights Reserved.
  Portions Copyright (C) 2008-2010 David Anderson.  All Rights Reserved.

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

#include <string>
#include <iostream>
#include "common.h"
#define DWARFDUMP_VERSION " Tue Apr 10 11:43:32 PDT 2012  "
using std::string;
using std::cout;
using std::cerr;
using std::endl;

void
print_version_details(const std::string & name,bool alwaysprint)
{
#ifdef WIN32
#   ifdef _DEBUG
    char *acType = "Debug";
#   else
    char *acType = "Release";
#   endif /* _DEBUG */
    static char acVersion[32];
    snprintf(acVersion,sizeof(acVersion),
        "[%s %s %s]",__DATE__,__TIME__,acType);
    cout << name << " " << acVersion << endl;
#else  /* !WIN32 */
    if(alwaysprint) {
        cout << DWARFDUMP_VERSION << endl;
    }
#endif /* WIN32 */
}


void
print_args(int argc, char *argv[])
{
#ifdef WIN32
    int nIndex;
    cout << "Arguments:";
    for (nIndex = 1; nIndex < argc; ++nIndex) {
        cout << " " << argv[nIndex] ;
    }
    cout << endl;
#endif
}

void
print_usage_message(const std::string &progname,
    const char **text)
{
#ifndef WIN32
    cerr <<"Usage:  " << progname<<"  <options> <object file>" << endl;
#endif
    for (unsigned i = 0; *text[i]; ++i) {
        cerr <<  text[i] << endl;
    }
}
