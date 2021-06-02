
###A placeholder to run tests

AUTOMAKE_OPTIONS = subdir-objects

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES =
EXTRA_DIST =


check_PROGRAMS = test_dwarfstring test_extra_flag_strings \
  test_linkedtopath test_headersok

test_dwarfstring_SOURCES = test_dwarfstring.c \
   $(top_srcdir)/src/lib/libdwarf/dwarfstring.h \
   $(top_srcdir)/src/lib/libdwarf/dwarfstring.c
test_dwarfstring_CFLAGS = $(CFLAGS_WARN)
test_dwarfstring_CPPFLAGS = \
-I$(top_srcdir) -I$(top_builddir) -I$(top_srcdir)/src/lib/libdwarf

test_extra_flag_strings_SOURCES = test_extra_flag_strings.c \
   $(top_srcdir)/src/lib/libdwarf/pro_log_extra_flag_strings.c \
   $(top_srcdir)/src/lib/libdwarf/dwarfstring.h \
   $(top_srcdir)/src/lib/libdwarf/dwarfstring.c
test_extra_flag_strings_CFLAGS = $(CFLAGS_WARN)
test_extra_flag_strings_CPPFLAGS =  -DTESTING \
-I$(top_srcdir) -I$(top_builddir) -I$(top_srcdir)/src/lib/libdwarf

test_linkedtopath_SOURCES = test_linkedtopath.c \
   $(top_srcdir)/src/lib/libdwarf/dwarfstring.h \
   $(top_srcdir)/src/lib/libdwarf/dwarfstring.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_debuglink.h \
   $(top_srcdir)/src/lib/libdwarf/dwarf_debuglink.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_error.h
test_linkedtopath_CFLAGS = $(RO_CFLAGS_WARN) -DTESTING
test_linkedtopath_CPPFLAGS = -DTESTING \
-I$(top_srcdir) -I$(top_builddir) -I$(top_srcdir)/src/lib/libdwarf

# Just ensures the headers usable, no C source errors.
test_headersok_SOURCES = test_headersok.c \
    $(top_srcdir)/src/lib/libdwarf/dwarf_reloc_386.h  \
    $(top_srcdir)/src/lib/libdwarf/dwarf_reloc_mips.h  \
    $(top_srcdir)/src/lib/libdwarf/dwarf_reloc_ppc.h   \
    $(top_srcdir)/src/lib/libdwarf/dwarf_reloc_arm.h    \
    $(top_srcdir)/src/lib/libdwarf/dwarf_reloc_ppc64.h  \
    $(top_srcdir)/src/lib/libdwarf/dwarf_reloc_x86_64.h
test_headersok_CFLAGS = $(RO_CFLAGS_WARN)
test_headersok_CPPFLAGS = -DTESTING \
-I$(top_srcdir) -I$(top_builddir) -I$(top_srcdir)/src/lib/libdwarf

###heck_TESTS = testdebuglink.sh runtests.sh
