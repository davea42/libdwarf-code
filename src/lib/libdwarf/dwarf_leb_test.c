/*
  Copyright (C) 2000,2004 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2011-2020 David Anderson. All Rights Reserved.

  This program is free software; you can redistribute it
  and/or modify it under the terms of version 2.1 of the
  GNU Lesser General Public License as published by the Free
  Software Foundation.

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

  You should have received a copy of the GNU Lesser General
  Public License along with this program; if not, write the
  Free Software Foundation, Inc., 51 Franklin Street - Fifth
  Floor, Boston MA 02110-1301, USA.

*/


#include "config.h"
#include <stdio.h>
#include "dwarf_incl.h"
#include "libdwarf.h"
#include "dwarf_error.h"
#include "dwarf_util.h"
#include "pro_encode_nm.h"


static void
printinteresting(void)
{
    return;
}

static Dwarf_Signed stest[] = {
0,0xff,
0x800000000000002f,
0x800000000000003f,
0x800000000000004f,
0x8000000000000070,
0x800000000000007f,
0x8000000000000080,
0x8000000000000000,
0x800000ffffffffff,
0x80000000ffffffff,
0x800000ffffffffff,
0x8000ffffffffffff,
0xffffffffffffffff,
-1703944 /*18446744073707847672 as signed*/,
562949951588368,
-1,
-127,
-100000,
-2000000000,
-4000000000,
-8000000000,
-800000000000,
};
static Dwarf_Unsigned utest[] = {
0,0xff,0x7f,0x80,
0x800000000000002f,
0x800000000000003f,
0x800000000000004f,
0x8000000000000070,
0x800000000000007f,
0x8000000000000080,
0x800000ffffffffff,
0x80000000ffffffff,
0x800000ffffffffff,
0x8000ffffffffffff,
9223372036854775808ULL,
-1703944 /*18446744073707847672 as signed*/,
562949951588368,
0xffff,
0xffffff,
0xffffffff,
0xffffffffff,
0xffffffffffff,
0xffffffffffffff,
0xffffffffffffffff
};


#if 0 /* FOR DEBUGGING */
static void
dump_encoded(char *space,int len)
{
    int t;

    printf("encode len %d: ",len);
    for ( t = 0; t < len; ++t) {
        printf("%02x",space[t] & 0xff);
    }
    printf("\n");
}
#endif


#define BUFFERLEN 100


static unsigned
signedtest(unsigned len)
{
    unsigned errcnt = 0;
    unsigned t = 0;
    char bufferspace[BUFFERLEN];

    for ( ; t < len; ++t) {
        int res = 0;
        int encodelen = 0;
        Dwarf_Unsigned decodelen = 0;
        Dwarf_Signed decodeval = 0;

        res = _dwarf_pro_encode_signed_leb128_nm(
            stest[t],&encodelen,bufferspace,BUFFERLEN);
        if (res != DW_DLV_OK) {
            printf("FAIL signed encode "
                "DW_DLV_ERROR, index %u expected 0x%llx"
                " line:%d\n",
                t,stest[t],__LINE__);
            ++errcnt;
        }
        res = dwarf_decode_signed_leb128(
            (char *)bufferspace,
            &decodelen,
            &decodeval,
            (char *)(&bufferspace[BUFFERLEN-1]));
        if (res != DW_DLV_OK) {
            printf("FAIL public DW_DLV_ERROR "
                "signed decode index %u "
                "val 0x%llx line:%d\n",
                t,stest[t],__LINE__);
            ++errcnt;
        }
        if (stest[t] != decodeval) {
            printf("FAIL public signed decode val index %u "
                "expected 0x%llx vs got 0x%llx line:%d\n",
                t,stest[t],decodeval,__LINE__);
            ++errcnt;
        }
        if ((Dwarf_Unsigned)encodelen != decodelen) {
            printf("FAIL public signed decodelen val "
                "index %u val 0x%llx "
                " encodelen %u decodelen %u   line:%d\n",
                t,stest[t],(unsigned)encodelen,
                (unsigned)decodelen,__LINE__);
            ++errcnt;
        }
        res = _dwarf_decode_s_leb128_chk(
            (Dwarf_Small *)bufferspace,
            &decodelen,
            &decodeval,
            (Dwarf_Byte_Ptr)(&bufferspace[BUFFERLEN-1]));
        if (res != DW_DLV_OK) {
            printf("FAIL got DW_DLV_ERRROR signed decode"
                " index %u "
                "expected val 0x%llx line:%d\n",
                t,stest[t],__LINE__);
            ++errcnt;
        }
        if (stest[t] != decodeval) {
            printf("FAIL signed decode val index %u "
                "val 0x%llx vs 0x%llx line:%d\n",
                t,stest[t],decodeval,__LINE__);
            ++errcnt;
        }
        if ((Dwarf_Unsigned)encodelen != decodelen) {
            printf("FAIL signed decodelen val index %u val 0x%llx "
                " encodelen %u decodelen %u   line:%d\n",
                t,stest[t],(unsigned)encodelen,
                (unsigned)decodelen,__LINE__);
            ++errcnt;
        }
    }
    return errcnt;
}

static  unsigned
unsignedtest(unsigned len)
{
    unsigned errcnt = 0;
    unsigned t = 0;
    char bufferspace[BUFFERLEN];

    for ( ; t < len; ++t) {
        int res = 0;
        int encodelen = 0;
        Dwarf_Unsigned decodelen = 0;
        Dwarf_Unsigned decodeval = 0;

        res = _dwarf_pro_encode_leb128_nm(
            utest[t],&encodelen,bufferspace,BUFFERLEN);
        if (res != DW_DLV_OK) {
            printf("FAIL signed encode index %u val 0x%llx line:%d\n",
                t,utest[t],__LINE__);
            ++errcnt;
        }
        res = dwarf_decode_leb128(
            (char *)bufferspace,
            &decodelen,
            &decodeval,
            (char *)(&bufferspace[BUFFERLEN-1]));
        if (res != DW_DLV_OK) {
            printf("FAIL public unsigned decode index %u "
                "val 0x%llx line:%d\n", t,utest[t],__LINE__);
            ++errcnt;
        }
        if (utest[t] != decodeval) {
            printf("FAIL public unsigned decode val index %u "
                "expected 0x%llx vs received 0x%llx line:%d\n",
                t,utest[t],decodeval,__LINE__);
            ++errcnt;
        }
        if ((Dwarf_Unsigned)encodelen != decodelen) {
            printf("FAIL public unsigned decodelen val index %u "
                "val 0x%llx line:%d\n", t,utest[t],__LINE__);
            ++errcnt;
        }

        res = _dwarf_decode_u_leb128_chk(
            (Dwarf_Small *)bufferspace,
            &decodelen,
            &decodeval,
            (Dwarf_Byte_Ptr)(&bufferspace[BUFFERLEN-1]));
        if (res != DW_DLV_OK) {
            printf("FAIL unsigned decode index %u "
                "val 0x%llx line:%d\n",
                t,utest[t],__LINE__);
            ++errcnt;
        }
        if (utest[t] != decodeval) {
            printf("FAIL unsigned decode val index %u "
                "val 0x%llx vs 0x%llx line:%d\n",
                t,utest[t],decodeval,__LINE__);
            ++errcnt;
        }
        if ((Dwarf_Unsigned)encodelen != decodelen) {
            printf("FAIL unsigned decodelen val index %u "
                "val 0x%llx line:%d\n",
                t,utest[t],__LINE__);
            ++errcnt;
        }
    }
    return errcnt;
}
static unsigned char v1[] = {
0x90, 0x90, 0x90,
0x90, 0x90, 0x90,
0x90, 0x90, 0x90,
0x90, 0x90, 0x90,
0x90 };

static unsigned char v2[] = {
0xf4,0xff,
0xff,
0xff,
0x0f,
0x4c,
0x00,
0x00,
0x00};

/*   9223372036854775808 == -9223372036854775808 */
static unsigned char v3[] = {
0x80, 0x80, 0x80,
0x80, 0x80, 0x80,
0x80, 0x80, 0x80,
0x41 };


/*  This warning with --enable-sanitize is fixed
    as of November 11, 2016 when decoding test v4.
    dwarf_leb.c: runtime error: negation of
    -9223372036854775808 cannot be
    represented in type 'Dwarf_Signed' (aka 'long long');
    cast to an unsigned type to negate this value to itself.
    The actual value here is
    -4611686018427387904 0xc000000000000000,
    for a 64bit signed int target */
static unsigned char v4[] = {
0x80, 0x80, 0x80,
0x80, 0x80, 0x80,
0x80, 0x80, 0x40 }; /* the 0x40 on bit
    plays the role of incorporating the bit and
    also requesting a sign bit. */

/*  sort of v4 with zero padding on end.
    Here with target signed 64bit twos complement */
static unsigned char v5[] = {
0x80, 0x80, 0x80,
0x80, 0x80, 0x80,
0x80, 0x80, 0xc0, /* the 0xc0 plays the role of
    incorporating a bit and continuing input. */
0x80, 0x80, 0x80,
0x80, 0x80, 0x80,
0x80, 0x80, 0x40  /* The 0x40  cannot be or-d in
    as it is shifted off the end of the target
    64bit int area, but it plays the role of
    requesting sign bit. */
};

/*  If we had a target of 32bit signed int we would, to
    to get 0xc0000000 with trailing padding:
0x80, 0x80, 0x80,
0x80, 0x84, The 4 gets into the int., 8 continues
    the input any higher bits in the bottom
    7 bits get shifted off and vanish,
    so 0xf6 would get the same value.
0x80, 0x80, 0x80,
0x80, 0x80, 0x80,
0x80, 0x80, 0x40     The 0x40  gets the sign set

Simlarly, but without any padding bytes:
0x80, 0x80, 0x80,
0x80, 0x44, The  second 4 (and 2 zero bits) gets into the int,
    first 4 gets the sign bit set
*/



/*  Error, too long  due to the non-zero past any valid
    Dwarf_Signed*/
static unsigned char v6[] = {
0x80, 0x80, 0x80,
0x80, 0x80, 0x80,
0x80, 0x80, 0xc0,
0x80, 0x80, 0x80,
0x80, 0x80, 0x80,
0x80, 0x80, 0x41, /* The 0x40  cannot be or-d in
    as we are off the end of shiftable area,
    but it plays the role of requesting sign bit. */

};

/*  unsigned decode with padding */
static unsigned char v7[] = {
0x81, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80
};
/* padding exceeds our chosen max */
static unsigned char v8[] = {
0x81, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80
};


static unsigned
specialtests(void)
{
    unsigned errcnt = 0;
    unsigned vlen = 0;
    Dwarf_Unsigned decodelen = 0;
    Dwarf_Signed decodeval = 0;
    Dwarf_Unsigned udecodeval = 0;
    int res;

    vlen = sizeof(v1)/sizeof(char);
    res = _dwarf_decode_s_leb128_chk(
        (Dwarf_Small *)v1,
        &decodelen,
        &decodeval,
        (Dwarf_Byte_Ptr)(&v1[vlen]));
    if (res != DW_DLV_ERROR) {
        printf("FAIL unsigned decode special v1  line:%d\n",__LINE__);
        ++errcnt;
    }
    res = _dwarf_decode_u_leb128_chk(
        (Dwarf_Small *)v1,
        &decodelen,
        &udecodeval,
        (Dwarf_Byte_Ptr)(&v1[vlen]));
    if (res != DW_DLV_ERROR) {
        printf("FAIL unsigned decode special v1  line:%d\n",__LINE__);
        ++errcnt;
    }

    vlen = sizeof(v2)/sizeof(char);
    res = _dwarf_decode_s_leb128_chk(
        (Dwarf_Small *)v2,
        &decodelen,
        &decodeval,
        (Dwarf_Byte_Ptr)(&v2[vlen]));
    if (res != DW_DLV_OK) {
        printf("FAIL signed decode special v2  line:%d\n",__LINE__);
        ++errcnt;
    }
    /*  If you just do (byte & 0x7f) << shift
        and byte is (or is promoted to) a signed type
        on the following decode you get the wrong value.
        Undefined effect in C leads to error.  */
    res = _dwarf_decode_u_leb128_chk(
        (Dwarf_Small *)v2,
        &decodelen,
        &udecodeval,
        (Dwarf_Byte_Ptr)(&v2[vlen]));
    if (res != DW_DLV_OK) {
        printf("FAIL unsigned decode special v2  line:%d\n",__LINE__);
        ++errcnt;
    }

    vlen = sizeof(v3)/sizeof(char);
    res = _dwarf_decode_s_leb128_chk(
        (Dwarf_Small *)v3,
        &decodelen,
        &decodeval,
        (Dwarf_Byte_Ptr)(&v3[vlen]));
    if (res != DW_DLV_OK) {
        printf("FAIL signed decode special v3  line:%d\n",__LINE__);
        ++errcnt;
    }
    if ((Dwarf_Unsigned)decodeval !=
        (Dwarf_Unsigned)0x8000000000000000) {
        printf("FAIL signed decode special v3 value check %lld "
            "vs %lld  line:%d\n",
            decodeval,(Dwarf_Signed)0x8000000000000000,__LINE__);
        ++errcnt;
    }

    vlen = sizeof(v4)/sizeof(char);
    res = _dwarf_decode_s_leb128_chk(
        (Dwarf_Small *)v4,
        &decodelen,
        &decodeval,
        (Dwarf_Byte_Ptr)(&v4[vlen]));
    if (res != DW_DLV_OK) {
        printf("FAIL signed decode special v4  line:%d\n",__LINE__);
        ++errcnt;
    }
    if (decodeval != -4611686018427387904) {
        printf("FAIL signed decode special v4 value check %lld "
            "vs %lld  line:%d\n",
            decodeval,-4611686018427387904LL,__LINE__);
        printf("FAIL signed decode special v4 value check 0x%llx "
            "vs 0x%llx  line:%d\n",
            decodeval,-4611686018427387904LL,__LINE__);
        ++errcnt;
    }
    vlen = sizeof(v5)/sizeof(char);
    res = _dwarf_decode_s_leb128_chk(
        (Dwarf_Small *)v5,
        &decodelen,
        &decodeval,
        (Dwarf_Byte_Ptr)(&v5[vlen]));
    if (res != DW_DLV_OK) {
        printf("FAIL signed decode special v5  line:%d\n",__LINE__);
        ++errcnt;
    }
    if (decodeval != -4611686018427387904) {
        printf("FAIL signed decode special v5 value check got %lld "
            "vs expected %lld  line:%d\n",
            decodeval,-4611686018427387904LL,__LINE__);
        printf("FAIL signed decode special v5 value check got %llx "
            "vs expected %llx  line:%d\n",
            decodeval,-4611686018427387904LL,__LINE__);
        ++errcnt;
    }
    if (decodelen != vlen) {
        printf("FAIL signed decode special v5 decode len ck"
            "Expected decode len %u"
            "got decode len  %u  line %d\n",
            (unsigned)vlen,(unsigned)decodelen,__LINE__);
    }
    vlen = sizeof(v6)/sizeof(char);
    res = _dwarf_decode_s_leb128_chk(
        (Dwarf_Small *)v6,
        &decodelen,
        &decodeval,
        (Dwarf_Byte_Ptr)(&v6[vlen]));
    if (res != DW_DLV_ERROR) {
        printf("FAIL signed decode special v6 "
            "did not get expected error output %d\n",
            __LINE__);
    }
    vlen = sizeof(v7)/sizeof(char);
    res = _dwarf_decode_u_leb128_chk(
        (Dwarf_Small *)v7,
        &decodelen,
        &udecodeval,
        (Dwarf_Byte_Ptr)(&v7[vlen]));
    if (res == DW_DLV_ERROR) {
        printf("FAIL unsigned decode special v7 "
            "unexpected error output %d\n",
            __LINE__);
    }
    if (udecodeval != 1) {
        printf("FAIL usigned decode special v7 value check got %llu "
            "vs expected %u  line:%d\n",
            udecodeval,1,__LINE__);
    }
    if (vlen != decodelen) {
        printf("FAIL usigned decode special v7 decode len got %u "
            "vs expected %u  line:%d\n",
            (unsigned)decodelen,vlen,__LINE__);
    }
    vlen = sizeof(v8)/sizeof(char);
    res = _dwarf_decode_u_leb128_chk(
        (Dwarf_Small *)v8,
        &decodelen,
        &udecodeval,
        (Dwarf_Byte_Ptr)(&v8[vlen]));
    if (res != DW_DLV_ERROR) {
        printf("FAIL unsigned decode special v8 "
            "unexpected pass expected DW_DLV_ERROR line:%d\n",
            __LINE__);
    }

    return errcnt;
}

int main(void)
{
    unsigned slen = sizeof(stest)/sizeof(Dwarf_Signed);
    unsigned ulen = sizeof(utest)/sizeof(Dwarf_Unsigned);
    int errs = 0;

    printinteresting();
    errs += signedtest(slen);

    errs += unsignedtest(ulen);

    errs += specialtests();

    if (errs) {
        printf("FAIL. leb encode/decode errors\n");
        return 1;
    }
    printf("PASS leb tests\n");
    return 0;
}
