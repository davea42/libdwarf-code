dnl Process this file with autoconf to produce a configure script.
AC_INIT()
AC_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_C_BIGENDIAN
AC_GCC_TRADITIONAL
AC_PROG_INSTALL
AC_CHECK_TOOL(RANLIB, ranlib, :)
AC_CHECK_TOOL(AR, ar)

dnl libdwarf default-disabled shared
shrd=''
AC_ARG_ENABLE(shared,AC_HELP_STRING([--enable-shared],
                [build shared library libdwarf.so and use it if present]))
AS_IF([ test "x$enable_shared" = "xyes"], [
   shrd='--enable-shared'])

AC_SUBST(dwfpic,[-fPIC])
nonshrd=''
dnl default-enabled nonshared
AC_SUBST(build_nonshared,[none.a])
AC_ARG_ENABLE(nonshared,AC_HELP_STRING([--disable-nonshared],
                [do not build archive library libdwarf.a]))
AS_IF([ test "x$enable_nonshared" = "xno"], [
   nonshrd='--disable-shared'
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

chckres() {
if test $1 != 0
then
  echo "Configure Error exit:  $2"
  exit 2
fi
}

AC_CONFIG_SUBDIRS([libdwarf dwarfdump dwarfgen dwarfexample])

AC_OUTPUT(Makefile)
