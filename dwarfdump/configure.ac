dnl Process this file with autoconf to produce a configure script.
AC_INIT(dwarfdump.c)
AC_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PATH_PROG([CC_FOR_BUILD],[cc])
AC_GCC_TRADITIONAL
AC_PROG_INSTALL
AC_CHECK_TOOL(RANLIB, ranlib, :)
AC_CHECK_TOOL(AR, ar)
dnl AC_ARFLAGS

AC_CHECK_HEADERS([elf.h unistd.h libelf.h libelf/libelf.h sgidefs.h sys/types.h])

dnl Windows seems to have libelf.h with the normal elf.h
dnl data embedded. No literal elf.h  is present.
dnl Get a best estimate of elf header data location.
dnl The name HAVE_LOCATION_OF_LIBELFHEADER is just wrong, 
dnl this is looking for the location of the Elf header data.
AC_MSG_CHECKING(where is basic elf data)
if test "$ac_cv_header_elf_h" = yes; then
 AC_DEFINE(HAVE_LOCATION_OF_LIBELFHEADER,[<elf.h>], [Define to header that first defines elf])
 AC_MSG_RESULT(<elf.h>)
elif test "$ac_cv_header_libelf_h" = yes; then
 AC_DEFINE(HAVE_LOCATION_OF_LIBELFHEADER, [<libelf.h>],
	[Define to header that first defines elf.])
 AC_MSG_RESULT(<libelf.h>)
elif test "$ac_cv_header_libelf_libelf_h" = yes; then
 AC_DEFINE(HAVE_LOCATION_OF_LIBELFHEADER,[<libelf/libelf.h>],
	[Define to header that first defines elf.])
 AC_MSG_RESULT(<libelf/libelf.h>)
else
 AC_MSG_RESULT(NO standard ELF HEADERS found)
fi

AC_MSG_CHECKING(libelf has elf_getehdr function)
AC_SEARCH_LIBS([elf64_getehdr],[elf],
    [ AC_DEFINE(HAVE_ELF64_GETEHDR,1,
        [Define to 1 if the elf64_getehdr function is in libelf.a.])
      AC_MSG_RESULT(yes) ],
    [ AC_MSG_RESULT(no) ])

dnl default-disabled shared
dnl we allow the argument but the result is ignored.
shrd=''
AC_MSG_CHECKING(do we build to use shared lib)
AC_ARG_ENABLE(shared,AC_HELP_STRING([--enable-shared],
                [build shared library libdwarf.so and use it if present]))
AS_IF([ test "x$enable_shared" = "xyes"], [
   AC_MSG_RESULT(yes)
   shrd='--enable-shared'],
   [ AC_MSG_RESULT(no)])

nonshrd=''
dnl default-enabled nonshared
dnl we allow the argument but the result is ignored.
AC_SUBST(build_nonshared,[none.a])
AC_ARG_ENABLE(nonshared,AC_HELP_STRING([--disable-nonshared],
                [do not build archive library libdwarf.a]))
AS_IF([ test "x$enable_nonshared" = "xno"], [
   nonshrd='--disable-shared'
])



dnl This adds compiler options for gcc to get as complete
dnl diagnostics as seems to make sense (subject to change...).
AC_MSG_CHECKING(compile with -Wall)
AC_SUBST(dwfwall,[])
AC_ARG_ENABLE(wall,AC_HELP_STRING([--enable-wall],
        [Add -Wall (default is none)]),
        [ AC_MSG_RESULT(yes)
          AC_SUBST(dwfwall,["-Wall -O0 -Wpointer-arith -Wmissing-declarations -Wmissing-prototypes -Wdeclaration-after-statement -Wextra -Wcomment -Wformat -Wpedantic -Wuninitialized -Wno-long-long -Wshadow"]) ],
        [ AC_SUBST(dwfwall,[])
          AC_MSG_RESULT(no) ])

dnl This adds compiler option -fsanitize=address etc (gcc compiler run-time checks))
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
AC_MSG_CHECKING(checking for windows stdafx.h)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include "stdafx.h"
  int main() {
     Elf64_Rel *p; int i; i = p->r_info;
  } ]])],
  [AC_DEFINE(HAVE_STDAFX_H,1,
        [Define 1 if we have the Windows specific header stdafx.h])
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)])

dnl Using following does not work
dnl AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include HAVE_LOCATION_OF_LIBELFHEADER
AC_MSG_CHECKING(do we have Elf64_Rel struct)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <elf.h>
   int main() {
      Elf64_Rel *p; int i; i = p->r_info; 
   } ]])],
   [AC_DEFINE(HAVE_ELF64_R_INFO,1,
	[Define to 1 if the Elf64_Rel structure has r_info field.])
    AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)])

AC_CHECK_FUNCS(snprintf)
AC_CHECK_FUNCS(vsnprintf)

AC_MSG_CHECKING(is __uint32_t predefined in the compiler)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
   int main() {
        __uint32_t p; p = 2;
   } ]])],
   [AC_DEFINE(HAVE___UINT32_T,1,
        [See if __uint32_t is predefined in the compiler.])
    AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)])

AC_MSG_CHECKING(is __uint64_t predefined in the compiler)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
   int main() {
       __uint64_t p; p = 3;
   } ]])],
   [AC_DEFINE(HAVE___UINT64_T,1,
        [See if __uint64_t is predefined in the compiler.])
    AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)])

AC_MSG_CHECKING(is __uint32_t defined in sys/types.h)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <sys/types.h>
  int main() {
     __uint32_t p; p = 4;
  } ]])],
  [AC_DEFINE(HAVE___UINT32_T_IN_SYS_TYPES_H,1,
        [Define 1 if sys/types.h defines __uint32_t.])
    AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)])

AC_MSG_CHECKING(is __uint64_t defined in sys/types.h)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <sys/types.h>
   int main() {
     __uint64_t p; p = 5;
   } ]])],
  [AC_DEFINE(HAVE___UINT64_T_IN_SYS_TYPES_H,1,
        [Define 1 if sys/types.h defines __uint64_t.])
    AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)])



AC_MSG_CHECKING(is regex defined)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <sys/types.h>
#include <regex.h>
   int main() {
        int i;
        regex_t r;
        int cflags = REG_EXTENDED;
        const char *s = "abc";
        i = regcomp(&r,s,cflags);
        regfree(&r);
   } ]])],
   [AC_DEFINE(HAVE_REGEX,1,
		[Define 1 if regex seems to be defined])
    AC_MSG_RESULT(yes)],
   [AC_MSG_RESULT(no)])


AC_MSG_CHECKING(checking if __attribute__ unused compiles ok)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
  static unsigned foo( unsigned x, __attribute__ ((unused)) int y)
  {  unsigned x2 = x + 1;
     return x2;
  }
  int goo() {
     unsigned y = 0;
     y = foo(12,y);
  }
  ]])],
  [AC_DEFINE(HAVE_UNUSED_ATTRIBUTE,1,
      [Define 1 if __attribute__ ((unused)) compiles ok.])
    AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)])


AC_SUBST(dwelflib,[])
AC_MSG_CHECKING(libelf has elf_begin function)
AC_SEARCH_LIBS([elf_begin],[elf],
    [ libelflib=y
      AC_MSG_RESULT(yes) ],
    [ libelflib=n
      AC_MSG_RESULT(no) ])
dnl using following does not work
dnl AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include "HAVE_LOCATION_OF_LIBELF_HEADER"
AC_MSG_CHECKING(is libelf header present and library linkable)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <libelf.h>
    int main() {
      int fd = 1;
      Elf_Cmd cmd = 0;
      Elf *elf = 0;

      elf = elf_begin(fd,cmd,&elf);
      if (res) {
         /* ALL IS WELL */
      }
    } ]])],
    [ libelfheader="y"
     AC_MSG_RESULT(yes)
    ],
    [ libelfheader="n"
     AC_MSG_RESULT(no)])
AC_MSG_CHECKING(are libelf.h and its library present and linkable)
AS_IF([ test x$libelflib = "xy" -a x$libelfheader = "xy" ],
    [ AC_MSG_RESULT(yes) ],
    [AC_MSG_RESULT(no)])

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


dnl this is ignored in dwarfdump.
AC_MSG_CHECKING(enable nonstandardprintf)
AC_ARG_ENABLE(nonstandardprintf,AS_HELP_STRING([--enable-nonstandardprintf],
        [Use a special printf format for 64bit (default is NO)]),
        [ AC_DEFINE([HAVE_NONSTANDARD_PRINTF_64_FORMAT],[1],
           [Define 1 if need nonstandard printf format for 64bit] )
          AC_MSG_RESULT(yes)],
        [AC_MSG_RESULT(no)])


AC_MSG_CHECKING(libelf defines off64_t if _GNU_SOURCE defined)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
#define _GNU_SOURCE
#include <libelf.h>
   int main() {
     off64_t  p; p = 0;
   } ]])],
  [AC_DEFINE(HAVE_LIBELF_OFF64_OK,1,
      [Define 1 if  off64 is defined via libelf with GNU_SOURCE.])
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)])



AC_OUTPUT(Makefile)
