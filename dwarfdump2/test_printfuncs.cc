/*
   Copyright (C) 2009-2012 David Anderson. All Rights Reserved.
   Portions Copyright 2012 SN Systems Ltd. All rights reserved.

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

/*  The address of the Free Software Foundation is
    Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
    Boston, MA 02110-1301, USA.
    SGI has moved from the Crittenden Lane address.
*/

#include "globals.h"
#include  <string>
#include <iostream>
#include <sstream> // For IToDec
#include <iomanip> // For setw

using std::cout;
using std::cerr;
using std::endl;
using std::string;

struct testdata {
    long long val;
    int definedlen;
    const char *expected;
} itests[] = {
{1,0,"1"},
{1,3,"  1"},
{-1,3," -1"},
{-1,0,"-1"},
{1003,3,"1003"},
{-1003,3,"-1003"},
{0,0,0}
};
struct testdata itestsx[] = {
{1,0,"0x1"},
{1,6,"   0x1"},
{-1,3,"0xffffffffffffffff"},
{-1,0,"0xffffffffffffffff"},
{1003,3,"0x3eb"},
{-1003,3,"0xfffffffffffffc15"},
{0,0,0}
};
struct utestdata {
    unsigned long long val;
    int definedlen;
    const char *expected;
} utests[] = {
{1,0,"1"},
{0,0,"0"},
{1,3,"  1"},
{1,3,"  1"},
{11245,0,"11245"},
{1003,3,"1003"},
{1003,3,"1003"},
{0,0,0}
};
struct utestdata utestsx[] = {
{1,0,"0x1"},
{1,6,"   0x1"},
{1,3,"0x1"},
{0,3,"0x0"},
{0xaa,0,"0xaa"},
{1003,3,"0x3eb"},
{1003,3,"0x3eb"},
{0,0,0}
};

// defined len ignored in these tests.
struct utestdata utests02x[] = {
{1,0,"01"},
{1,6,"01"},
{0x1d,6,"1d"},
{0xca,0,"ca"},
{0,0,"00"},
{1,3,"01"},
{1003,3,"eb"},
{0,0,0}
};

struct utestdata utests0Nx[] = {
{1,0,"0x1"},
{1,6,"0x0001"},
{0x1da,6,"0x01da"},
{0xca,0,"0xca"},
{1,3,"0x1"},
{0,3,"0x0"},
{0,4,"0x00"},
{1003,3,"0x3eb"},
{0,0,0}
};


int
test_ints()
{
    int errcount = 0;
    for (int i = 0; ; ++i) {
        struct testdata & x = itests[i];
        if (x.val == 0 && x.definedlen == 0 && x.expected == 0) {
            break;
        }
        string res = IToDec(x.val,x.definedlen);
        if (res != string(x.expected) ) {
            cout << "FAIL: test " << i << " got \"" << res <<
                "\" but expected \"" << x.expected <<
                "\"  line "  << __LINE__ << endl;
            errcount++;
        }
    }
    return errcount;
}

int
test_ints_hex()
{
    int errcount = 0;
    for (int i = 0; ; ++i) {
        struct testdata & x = itestsx[i];
        if (x.val == 0 && x.definedlen == 0 && x.expected == 0) {
            break;
        }
        string res = IToHex(x.val,x.definedlen);
        if (res != string(x.expected) ) {
            cout << "FAIL: test " << i << " got \"" << res <<
                "\" but expected \"" << x.expected <<
                "\"  line "  << __LINE__ << endl;
            errcount++;
        }
    }
    return errcount;
}
int
test_uints()
{
    int errcount = 0;
    for (int i = 0; ; ++i) {
        struct utestdata & x = utests[i];
        if (x.val == 0 && x.definedlen == 0 && x.expected == 0) {
            break;
        }
        string res = IToDec(x.val,x.definedlen);
        if (res != string(x.expected) ) {
            cout << "FAIL: test " << i << " got \"" << res <<
                "\" but expected \"" << x.expected <<
                "\"  line "  << __LINE__ << endl;
            errcount++;
        }
    }
    return errcount;
}

int
test_uints_hex()
{
    int errcount = 0;
    for (int i = 0; ; ++i) {
        struct utestdata & x = utestsx[i];
        if (x.val == 0 && x.definedlen == 0 && x.expected == 0) {
            break;
        }
        string res = IToHex(x.val,x.definedlen);
        if (res != string(x.expected) ) {
            cout << "FAIL: test " << i << " got \"" << res <<
                "\" but expected \"" << x.expected <<
                "\"  line "  << __LINE__ << endl;
            errcount++;
        }
    }
    return errcount;
}
int
test_uints_02hex()
{
    int errcount = 0;
    for (int i = 0; ; ++i) {
        struct utestdata & x = utests02x[i];
        if (x.val == 0 && x.definedlen == 0 && x.expected == 0) {
            break;
        }
        string res = IToHex02(x.val);
        if (res != string(x.expected) ) {
            cout << "FAIL: test " << i << " got \"" << res <<
                "\" but expected \"" << x.expected <<
                "\"  line "  << __LINE__ << endl;
            errcount++;
        }
    }
    return errcount;
}

int
test_uints_0Nhex()
{
    int errcount = 0;
    for (int i = 0; ; ++i) {
        struct utestdata & x = utests0Nx[i];
        if (x.val == 0 && x.definedlen == 0 && x.expected == 0) {
            break;
        }
        string res = IToHex0N(x.val,x.definedlen);
        if (res != string(x.expected) ) {
            cout << "FAIL: test " << i << " got \"" << res <<
                "\" but expected \"" << x.expected <<
                "\"  line "  << __LINE__ << endl;
            errcount++;
        }
    }
    return errcount;
}

struct testdata itests0Nd[] = {
{1,0,"1"},
{0,6,"000000"},
{1,6,"000001"},
{1003,6,"001003"},
{-1008,0,"-1008"},
{-1,3,"-01"},
{-1003,8,"-0001003"},
{0,0,0}
};

int
test_ints_0Ndec()
{
    int errcount = 0;
    for (int i = 0; ; ++i) {
        struct testdata & x = itests0Nd[i];
        if (x.val == 0 && x.definedlen == 0 && x.expected == 0) {
            break;
        }
        string res = IToDec0N(x.val,x.definedlen);
        if (res != string(x.expected) ) {
            cout << "FAIL: test " << i << " got \"" << res <<
                "\" but expected \"" << x.expected <<
                "\"  line "  << __LINE__ << endl;
            errcount++;
        }
    }
    return errcount;
}

struct testdatas {
    const char *val;
    int definedlen;
    const char *expected;
}sdata[] = {
{"aaad",3,"aaad"},
{"axae",6,"axae  "},
{"aaaf",24,"aaaf                    "},
{0,0,0}
};

int
test_leftalign()
{
    int errcount = 0;
    for (int i = 0; ; ++i) {
        struct testdatas & x = sdata[i];
        if (x.val == 0 && x.definedlen == 0 && x.expected == 0) {
            break;
        }
        std::string res = LeftAlign(x.definedlen,x.val);
        if (res != std::string(x.expected) ) {
            cout << "FAIL: test " << i << " got \"" << res <<
                "\" length: " << res.size() <<
                "  but expected \"" << x.expected <<
                "\" length: " << strlen(x.expected) <<
                "  line "  << __LINE__ << endl;
            errcount++;
        }
    }
    return errcount;
}


int main()
{
    int errcount = 0;
    errcount += test_ints();
    errcount += test_ints_hex();
    errcount += test_uints();
    errcount += test_uints_hex();
    errcount += test_uints_02hex();
    errcount += test_uints_0Nhex();
    errcount += test_ints_0Ndec();
    errcount += test_leftalign();
    if (errcount) {
        cout << "FAIL " << errcount << " tests" << endl;
        exit(1);
    }
    cout << "PASS "  << endl;
    exit(0);
}
