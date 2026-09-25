/*
University of Illinois/NCSA
Open Source License

Copyright (c) 2003-2017 University of Illinois at Urbana-Champaign.
All rights reserved.

Developed by:

LLVM Team

University of Illinois at Urbana-Champaign

http://llvm.org

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal with the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following
conditions:

* Redistributions of source code must retain the above
copyright notice,
this list of conditions and the following disclaimers.

* Redistributions in binary form must reproduce the above
copyright notice,
this list of conditions and the following disclaimers
in the documentation and/or other materials provided
with the distribution.

* Neither the names of the LLVM Team, University of
Illinois at
Urbana-Champaign, nor the names of its contributors
may be used to endorse or promote products derived from
this Software without specific prior written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
AND NONINFRINGEMENT.  IN NO EVENT SHALL THE CONTRIBUTORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
*/


/*  Used for all sorts of tables, not just e_machine */
struct em_values {
    const char *em_name;
    Dwarf_Unsigned em_number;
};

/* ASSERT: table values sorted by em_number */
void
dd_get_elf_machine_name(Dwarf_Unsigned value,struct esb_s *out);

/*  Table values not sorted by em_number,
    will have dups */
void
dd_get_elf_dynamic_table_name(Dwarf_Unsigned value,
    struct esb_s *out);

/*  There are duplicates here and not all in order.  */
void dd_get_elf_section_header_flag_names(
    Dwarf_Unsigned value,
    struct esb_s *out);

void dd_get_elf_section_header_st_type(
    Dwarf_Unsigned value,
    struct esb_s *out);

void dd_get_elf_symbol_sto_type(
    Dwarf_Unsigned value,
    struct esb_s *out);

void dd_get_elf_symbol_shn_type(
    Dwarf_Unsigned value,
    struct esb_s *out);

void dd_get_elf_symbol_stb_string(Dwarf_Unsigned value,
    struct esb_s *out);

void  dd_get_elf_symbol_stt_type(
    Dwarf_Unsigned value,
    struct esb_s *out);

void dd_get_elf_osabi_name(Dwarf_Unsigned value,
        struct esb_s *out);
