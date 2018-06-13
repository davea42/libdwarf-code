dnl process with autoheader, autoconf to produce a configure script.
AC_PREREQ([2.50])
AC_INIT(simplereader.c,frame1.c)
AC_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PATH_PROG([CC_FOR_BUILD],[cc])
AC_C_BIGENDIAN
AC_GCC_TRADITIONAL
AC_PROG_INSTALL
AC_CHECK_TOOL(RANLIB, ranlib, :)
AC_CHECK_TOOL(AR, ar)

AC_CHECK_HEADERS([elf.h unistd.h libelf.h sgidefs.h sys/types.h zlib.h])

dnl Attempt to determine if it is really IRIX/SGI or 'other'.
AC_MSG_CHECKING(check if __uint32_t in IRIX sgidefs.h)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <sgidefs.h>
  int main() {
   __uint32_t p; p = 27;
  } ]])],
  [AC_DEFINE(HAVE___UINT32_T_IN_SGIDEFS_H,1,
        [Define 1 if __uint32_t is in sgidefs.h.])
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)])
  
AC_MSG_CHECKING(check if __uint64_t in IRIX sgidefs.h)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <sgidefs.h>
  int main() {
    __uint64_t p; p = 27;
  } ]])],
  [AC_DEFINE(HAVE___UINT64_T_IN_SGIDEFS_H,1,
        [Define 1 if __uint64_t is in sgidefs.h.])
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)])

AC_MSG_CHECKING(is C99 intptr_t in stdint.h)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdint.h>
  int main() {
    intptr_t p; p = 27;
  } ]])],
  [AC_DEFINE(HAVE_INTPTR_T,1,
        [Define 1 if intptr_t defined in C99 stdint.h])
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)])


dnl default-disabled shared
AC_MSG_CHECKING(should we enable shared lib)
shrd=''
AC_ARG_ENABLE(shared,AC_HELP_STRING([--enable-shared],
                [build shared library libdwarf.so and use it if present]))
AS_IF([ test "x$enable_shared" = "xyes"], 
  [ shrd='--enable-shared'
    AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)])

nonshrd=''
AC_MSG_CHECKING(should we disable nonshared)
AC_SUBST(build_nonshared,[none.a])
AC_ARG_ENABLE(nonshared,AC_HELP_STRING([--disable-nonshared],
                [do not build archive library libdwarf.a]))
AS_IF([ test "x$enable_nonshared" = "xno"], 
  [ nonshrd='--disable-shared'
    AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)])

dnl This adds compiler option -Wall (gcc compiler warnings)
AC_MSG_CHECKING(should we use full compiler warnings)
AC_SUBST(dwfwall,[])
AC_ARG_ENABLE(wall,AC_HELP_STRING([--enable-wall],
        [Add -Wall (default is none)]),
        [ AC_SUBST(dwfwall,["-Wall -O0 -Wpointer-arith -Wmissing-declarations -Wmissing-prototypes -Wdeclaration-after-statement -Wextra -Wcomment -Wformat -Wpedantic -Wuninitialized -Wno-long-long -Wshadow" ])
          AC_MSG_RESULT(yes) ],
        [ AC_SUBST(dwfwall,[]) 
          AC_MSG_RESULT(no)]
        )

dnl This adds compiler option -fsanitize=address (gcc compiler run-time checks))
AC_MSG_CHECKING(should we use full runtime sanitization checks)
AC_SUBST(dwfsanitize,[])
AC_MSG_CHECKING(build -fsanitize-address) 
AC_ARG_ENABLE(sanitize,AC_HELP_STRING([--enable-sanitize],
       [Add -fsanitize (default is not to)]),
       [ AC_SUBST(dwfsanitize,["-fsanitize=address -fsanitize=leak -fsanitize=undefined"])
          AC_MSG_RESULT(yes) ],
       [ AC_SUBST(dwfsanitize,[]) 
          AC_MSG_RESULT(no) ]
       )
AC_MSG_CHECKING(do we expect elf_open exists as an open() alternative )
AC_ARG_ENABLE(elf_open,AC_HELP_STRING([--enable-elf_open],
                [use elf_open not open]))
AS_IF([ test "x$enable_elf_open" = "xyes"], [
   AC_DEFINE(HAVE_ELF_OPEN,1,
     [Define to 1 if we use elf_open in preference to open])
     AC_MSG_RESULT(yes) ],
   [ AC_MSG_RESULT(no)])

AC_SUBST(dwfzlib,[])
AC_MSG_CHECKING(libz aka zlib has uncompress function)
AC_SEARCH_LIBS([uncompress],[z],
    [ libzlib=y 
      AC_MSG_RESULT(yes) ],
    [ libzlib=n
      AC_MSG_RESULT(no) ])

AC_MSG_CHECKING(is zlib.h present)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include "zlib.h"
    int main() {
      Bytef dest[100];
      uLongf destlen = 100;
      Bytef *src = 0;
      uLong srclen = 3;
      int res = uncompress(dest,&destlen,src,srclen);
      if (res == Z_OK) {
         /* ALL IS WELL */
      }
    } ]])],
    [ libzheader="y"
     AC_MSG_RESULT(yes)
    ],
    [ libzheader="n" 
     AC_MSG_RESULT(no)])

AC_MSG_CHECKING(are zlib.h and its library present)
AS_IF([ test x$libzlib = "xy" -a x$libzheader = "xy" ],
    [AC_DEFINE(HAVE_ZLIB,1,
        [Define 1 if zlib decompression  available.])
     AC_SUBST(dwfzlib,[-lz])
     AC_MSG_RESULT(yes)
    ],
    [AC_MSG_RESULT(no)])


AC_OUTPUT(Makefile)

