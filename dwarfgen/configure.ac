dnl This is input to autoconf, producing a configure script.
AC_INIT(dwarfgen.cc)
AC_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_CXX
AC_C_BIGENDIAN
AC_GCC_TRADITIONAL
AC_PROG_INSTALL
AC_CHECK_TOOL(RANLIB, ranlib, :)
AC_CHECK_TOOL(AR, ar)

AC_CHECK_HEADERS(elf.h libelf.h sgidefs.h sys/types.h)

dnl Attempt to determine if it is really IRIX/SGI or 'other'.
AC_TRY_COMPILE([#include <sgidefs.h>],[  __uint32_t p; p = 27;] ,
  AC_DEFINE(HAVE___UINT32_T_IN_SGIDEFS_H,1,
        [Define 1 if __uint32_t is in sgidefs.h.]))
AC_TRY_COMPILE([#include <sgidefs.h>],[  __uint64_t p; p = 27;] ,
  AC_DEFINE(HAVE___UINT64_T_IN_SGIDEFS_H,1,
        [Define 1 if __uint64_t is in sgidefs.h.]))
AC_TRY_COMPILE([#include <sgidefs.h>],[  __uint64_t p; p = 27;] ,
  AC_DEFINE(HAVE___UINT64_T_IN_SGIDEFS_H,1,
        [Define 1 if  is in sgidefs.h.]))
dnl the existence of sgidefs.h does not prove it's truly SGI, nor
dnl prove that __uint32_t or __uint64_t is defined therein.
AC_TRY_COMPILE([#include <sgidefs.h>],[  __uint32_t p; p = 27;] ,
  AC_DEFINE(HAVE___UINT32_T_IN_SGIDEFS_H,1,
        [Define 1 if __uint32_t is in sgidefs.h.]))
AC_TRY_COMPILE([#include <stdint.h>],[intptr_t p; p = 27;] ,
  AC_DEFINE(HAVE_INTPTR_T,1,
        [Define 1 if intptr_t defined in C99 stdint.h]))
AC_TRY_COMPILE([#include <sgidefs.h>],[  __uint64_t p; p = 27;] ,
  AC_DEFINE(HAVE___UINT64_T_IN_SGIDEFS_H,1,
        [Define 1 if __uint64_t is in sgidefs.h.]))
AC_TRY_COMPILE([#include <sgidefs.h>],[  __uint64_t p; p = 27;] ,
  AC_DEFINE(HAVE___UINT64_T_IN_SGIDEFS_H,1,
        [Define 1 if  is in sgidefs.h.]))
AC_SUBST(dwfzlib,[])
AC_TRY_COMPILE([#include "zlib.h"],[
    Bytef dest[100]; 
    uLongf destlen = 100;
    Bytef *src = 0;
    uLong srclen = 3;
    int res = uncompress(dest,&destlen,src,srclen);
    if (res == Z_OK) {
         /* ALL IS WELL */ 
    }
    ] ,
    [AC_DEFINE(HAVE_ZLIB,1,
        [Define 1 if zlib (decompression library) seems available.])
     AC_SUBST(dwfzlib,[-lz])
    ])

dnl This adds compiler option -fsanitize=address (gcc compiler run-time checks))
AC_SUBST(dwfsanitize,[])
AC_MSG_CHECKING(build -fsanitize-address) 
AC_ARG_ENABLE(sanitize,AC_HELP_STRING([--enable-sanitize],
       [Add -fsanitize (default is not to)]),
       [ AC_SUBST(dwfsanitize,["-fsanitize=address -fsanitize=leak -fsanitize=undefined"])
          AC_MSG_RESULT(yes) ],
       [ AC_SUBST(dwfsanitize,[]) 
          AC_MSG_RESULT(no)
        ]
        )

AC_MSG_CHECKING(do we expect elf_open exists as an open() alternative )
AC_ARG_ENABLE(elf_open,AC_HELP_STRING([--enable-elf_open],
                [use elf_open not open]))
AS_IF([ test "x$enable_elf_open" = "xyes"], [
   AC_DEFINE(HAVE_ELF_OPEN,1,
     [Define to 1 if we use elf_open in preference to open])
     AC_MSG_RESULT(yes) ],
   [ AC_MSG_RESULT(no)])




dnl default-disabled shared
shrd=''
AC_ARG_ENABLE(shared,AC_HELP_STRING([--enable-shared],
                [build shared library libdwarf.so and use it if present]))
AS_IF([ test "x$enable_shared" = "xyes"], [
   shrd='--enable-shared'])

nonshrd=''
dnl default-enabled nonshared
AC_SUBST(build_nonshared,[none.a])
AC_ARG_ENABLE(nonshared,AC_HELP_STRING([--disable-nonshared],
                [do not build archive library libdwarf.a]))
AS_IF([ test "x$enable_nonshared" = "xno"], [
   nonshrd='--disable-shared'
])


AC_OUTPUT(Makefile)
