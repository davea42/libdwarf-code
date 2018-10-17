/*
Copyright (c) 2018, David Anderson
All rights reserved.

Redistribution and use in source and binary forms, with
or without modification, are permitted provided that the
following conditions are met:

    Redistributions of source code must retain the above
    copyright notice, this list of conditions and the following
    disclaimer.

    Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#ifndef DWARF_READING_H
#define DWARF_READING_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* uint*_t are for macho-loader.h */
#define uint16_t  unsigned DW_TYPEOF_16BIT
#define uint32_t  unsigned DW_TYPEOF_32BIT
#define uint8_t  unsigned char

#ifndef DW_DLV_OK
/* DW_DLV_OK  must match libdwarf.h */
/* DW_DLV_NO_ENTRY  must match libdwarf.h  */
#define DW_DLV_OK 0
#define DW_DLV_NO_ENTRY -1
#define DW_DLV_ERROR 1
#endif /* DW_DLV_OK */

#if (SIZEOF_UNSIGNED_LONG < 8) && (SIZEOF_UNSIGNED_LONG_LONG == 8)
#define uint64_t  unsigned long long
#else
#define uint64_t  unsigned long
#endif

#define TRUE 1
#define FALSE 0

#define ALIGN4 4
#define ALIGN8 8

#define PREFIX "\t"
#define LUFMT "%lu"
#define UFMT "%u"
#define DFMT "%d"
#define XFMT "0x%x"

/* even if already seen, values must match, so no #ifdef needed. */
#define DW_DLV_NO_ENTRY  -1
#define DW_DLV_OK         0
#define DW_DLV_ERROR      1

#define P printf
#define F fflush(stdout)

#define RRMOA(f,buf,loc,siz,errc) dwarf_object_read_random(f, \
    (char *)buf,loc,siz,errc);

#ifdef WORDS_BIGENDIAN
#define ASSIGN(func,t,s)                             \
    do {                                        \
        unsigned tbyte = sizeof(t) - sizeof(s); \
        t = 0;                                  \
        func(((char *)t)+tbyte ,&s,sizeof(s)); \
    } while (0)
#define SIGN_EXTEND(dest, length)           \
    do {                                    \
        if (*(signed char *)((char *)&dest +\
            sizeof(dest) - length) < 0) {   \
            memcpy((char *)&dest, "\xff\xff\xff\xff\xff\xff\xff\xff", \
                sizeof(dest) - length);     \
        }                                   \
    } while (0)

#else /* LITTLE ENDIAN */
#define ASSIGN(func,t,s)                             \
    do {                                        \
        t = 0;                                  \
        func(&t,&s,sizeof(s));    \
    } while (0)
#define SIGN_EXTEND(dest, length)                               \
    do {                                                        \
        if (*(signed char *)((char *)&dest + (length-1)) < 0) { \
            memcpy((char *)&dest+length,                        \
                "\xff\xff\xff\xff\xff\xff\xff\xff",             \
                sizeof(dest) - length);                         \
        }                                                       \
    } while (0)
#endif /* end LITTLE- BIG-ENDIAN */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DWARF_READING_H */
