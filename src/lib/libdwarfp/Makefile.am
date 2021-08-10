###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in

### libdwarfp

lib_LTLIBRARIES = libdwarfp.la

libdwarfp_la_SOURCES = \
libdwarfp.h \
memcpy_swap.h \
pro_alloc.c \
pro_alloc.h \
pro_arange.c \
pro_arange.h \
pro_debug_sup.c \
pro_die.c \
pro_die.h \
pro_dnames.h \
pro_dnames.c \
pro_error.c \
pro_error.h \
pro_expr.c \
pro_expr.h \
pro_finish.c \
pro_forms.c \
pro_frame.c \
pro_frame.h \
pro_funcs.c \
pro_incl.h \
pro_init.c \
pro_line.c \
pro_line.h \
pro_log_extra_flag_strings.c \
pro_macinfo.c \
pro_macinfo.h \
pro_opaque.h \
pro_pubnames.c \
pro_reloc.c \
pro_reloc.h \
pro_reloc_symbolic.c \
pro_reloc_symbolic.h \
pro_reloc_stream.h \
pro_reloc_stream.c \
pro_section.c \
pro_section.h \
pro_types.c \
pro_types.h \
pro_util.h \
pro_vars.c \
pro_weaks.c


libdwarfp_la_CFLAGS = $(DWARF_CFLAGS_WARN)
libdwarfp_la_CPPFLAGS = -I$(top_srcdir)/src/lib/libdwarf

libdwarfp_la_LIBADD = @DWARF_LIBS@ $(top_builddir)/src/lib/libdwarf/libdwarf.la

libdwarfp_la_LDFLAGS = -fPIC -no-undefined -version-info @version_info@ @release_info@

libdwarfppkgincludedir = $(pkgincludedir)-@VMAJ@
libdwarfppkginclude_DATA = \
$(top_srcdir)/src/lib/libdwarf/dwarf.h  \
$(top_srcdir)/src/lib/libdwarfp/libdwarfp.h \
$(top_srcdir)/src/lib/libdwarf/libdwarf.h

libdwarfpdevdir = $(datadir)/src/lib/libdwarfp/libdwarfp-devel
libdwarfpdev_DATA = 

EXTRA_DIST = \
COPYING \
ChangeLog \
CMakeLists.txt \
cmake/libdwarfp-config.cmake \
NEWS \
README \
$(libdwarfp_DATA) \
$(libdwarfpdev_DATA)
