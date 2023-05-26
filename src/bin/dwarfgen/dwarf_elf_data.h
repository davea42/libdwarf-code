/*
Copyright (c) 2023, David Anderson All rights reserved.
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
#ifndef DWARF_ELF_DATA_H
#define DWARF_ELF_DATA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct DW_Elf_Data_s {
    void *         d_buf;
    Dwarf_Unsigned d_size;
    Dwarf_Off      d_off;
    Dwarf_Unsigned d_align;
    unsigned int   d_type;
    unsigned int   d_version;
} DW_Elf_Data;

typedef struct DW_Elf_Scn_s {
    /* FIXME internal only data */
} DW_Elf_Scn;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DWARF_ELF_DATA_H */

