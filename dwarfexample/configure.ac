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

AC_CHECK_HEADERS([unistd.h libelf.h sys/types.h])

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
    [ AC_SUBST(dwfzlib,[-lz])
      AC_MSG_RESULT(yes) ],
    [ AC_SUBST(dwfzlib,[])
      AC_MSG_RESULT(no) ])

AC_OUTPUT(Makefile)

