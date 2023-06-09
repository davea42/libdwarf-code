/*

*/




#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif
#ifndef LLONG_MIN
#define LLONG_MIN    (-LLONG_MAX - 1LL)
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX   18446744073709551615ULL
#endif



/* See:
https://stackoverflow.com/questions/3944505/detecting-signed-overflow-in-c-c
*/
static
int _dwarf_add_check(Dwarf_Signed l, Dwarf_Signed r, 
    Dwarf_Signed *sum, Dwarf_Debug dbg,
    Dwarf_Error *error)
{
     if (l >= 0) {
         if ((0x7fffffffffffffffLL - l) < r) {
             _dwarf_error_string(dbg,error,
                DW_DLE_ARITHMETIC_OVERFLOW,
                "DW_DLE_ARITHMETIC_OVERFLOW: "
                "Adding integers l+r (l >= 0) overflows");
             return DW_DLV_ERROR;
         }
     } else {
         if (r < (0x7ffffffffffffffeLL -l))
         {
             _dwarf_error_string(dbg,error,
                DW_DLE_ARITHMETIC_OVERFLOW,
                "DW_DLE_ARITHMETIC_OVERFLOW: "
                "Adding integers l+r (l < 0) overflows");
             return DW_DLV_ERROR;
         }
     }
     if (sum) {
         *sum = l + r;
     } 
     return DW_DLV_OK;
}

/*  Thanks to David Grayson/
codereview.stackexchange.com/questions/98791/
safe-multiplication-of-two-64-bit-signed-integers
*/

static int 
_dwarf_int64_mult(Dwarf_Signed x, Dwarf_Signed y, 
    Dwarf_Signed * result, Dwarf_Debug dbg,
    Dwarf_Error*error)
{
    *result = 0;
    if (sizeof(Dwarf_Signed) != 8) {
        _dwarf_error_string(dbg,error,
            DW_DLE_ARITHMETIC_OVERFLOW,
            "DW_DLE_ARITHMETIC_OVERFLOW "
            "Signed 64bit multiply overflow(a)");
        return DW_DLV_ERROR;
    }
    if (x > 0 && y > 0 && x > INT64_MAX / y)  {
        _dwarf_error_string(dbg,error,
            DW_DLE_ARITHMETIC_OVERFLOW,
            "DW_DLE_ARITHMETIC_OVERFLOW "
            "Signed 64bit multiply overflow(b)");
        return DW_DLV_ERROR;
    }
    if (x < 0 && y > 0 && x < LLONG_MIN / y) {
        _dwarf_error_string(dbg,error,
            DW_DLE_ARITHMETIC_OVERFLOW,
            "DW_DLE_ARITHMETIC_OVERFLOW "
            "Signed 64bit multiply overflow(c)");
        return DW_DLV_ERROR;
    }
    if (x > 0 && y < 0 && y < LLONG_MIN / x) {
        _dwarf_error_string(dbg,error,
            DW_DLE_ARITHMETIC_OVERFLOW,
            "DW_DLE_ARITHMETIC_OVERFLOW "
            "Signed 64bit multiply overflow(d)");
        return DW_DLV_ERROR;
    }
    if (x < 0 && y < 0 && 
        (x <= LLONG_MIN || 
        y <= LLONG_MIN || 
        -x > LLONG_MAX / -y)) {
        _dwarf_error_string(dbg,error,
            DW_DLE_ARITHMETIC_OVERFLOW,
            "DW_DLE_ARITHMETIC_OVERFLOW "
            "Signed 64bit multiply overflow(e)");
        return DW_DLV_ERROR;
    }
    if (result) {
        *result = x * y;
    }
    return DW_DLV_OK;
}

static int
_dwarf_uint64_mult(Dwarf_Unsigned x, Dwarf_Unsigned y,
    Dwarf_Unsigned * result, Dwarf_Debug dbg,
    Dwarf_Error *error)
{
    Dwarf_Unsigned a = (x >> 32U) * (y & 0xFFFFFFFFU);
    Dwarf_Unsigned b = (x & 0xFFFFFFFFU) * (y >> 32U);
    int overflow = ((x >> 32U) * (y >> 32U)) +
        (a >> 32U) + (b >> 32U);
    if (overflow) {
        _dwarf_error_string(dbg,error,
            DW_DLE_ARITHMETIC_OVERFLOW,
            "DW_DLE_ARITHMETIC_OVERFLOW "
            "unsigned 64bit multiply overflow(e)");
        return DW_DLV_ERROR;
    }
    if (result) {
        *result = x*y;
    }
    return DW_DLV_OK;
}

