/*

*/




#ifdef 0
#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif
#ifndef LLONG_MIN
#define LLONG_MIN    (-LLONG_MAX - 1LL)
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX   18446744073709551615ULL
#endif
#endif



/* See:
https://stackoverflow.com/questions/3944505/detecting-signed-overflow-in-c-c
*/
int _dwarf_add_check(Dwarf_Signed l, Dwarf_Signed r, 
    Dwarf_Signed *sum, Dwarf_Debug dbg,
    Dwarf_Error *error);

/*  Thanks to David Grayson/
codereview.stackexchange.com/questions/98791/
safe-multiplication-of-two-64-bit-signed-integers
*/

int _dwarf_int64_mult(Dwarf_Signed x, Dwarf_Signed y, 
    Dwarf_Signed * result, Dwarf_Debug dbg,
    Dwarf_Error*error);

int _dwarf_uint64_mult(Dwarf_Unsigned x, Dwarf_Unsigned y,
    Dwarf_Unsigned * result, Dwarf_Debug dbg,
    Dwarf_Error *error);

