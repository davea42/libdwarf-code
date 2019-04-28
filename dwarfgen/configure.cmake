include(AutoconfHelper)

ac_init()

ac_c_bigendian()

ac_check_headers(elf.h libelf.h sgidefs.h sys/types.h)

ac_try_compile([=[
#include <stdint.h>
int main()
{
    intptr_t p; 
    p = 27;
    return 0;
}]=]
HAVE_INTPTR_T)

set(dwfzlib)
ac_try_compile([=[
#include "zlib.h"
int main()
{
    Bytef dest[100];
    uLongf destlen = 100;
    Bytef *src = 0;
    uLong srclen = 3;
    int res = uncompress(dest,&destlen,src,srclen);
    if (res == Z_OK) {
         /* ALL IS WELL */
    }
    return 0;
}]=]
HAVE_ZLIB)
if(HAVE_ZLIB)
    set(dwfzlib "z")
endif()

configure_file(config.h.in.cmake config.h)
