###Copyright (C) 2021 David Anderson <davea42 at linuxmail dot org>
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = 
rebuild: buildopstab 
	./buildopstab >opscounttab.c
	mv opscounttab.c $(top_srcdir)/src/bin/dwarfdump/opscounttab.c

noinst_PROGRAMS = buildopstab 
buildopstab_SOURCES = buildopscounttab.c \
  $(top_srcdir)/src/lib/libdwarf/dwarf_names.c
buildopstab_CPPFLAGS = -I$(top_srcdir)/src/lib/libdwarf \
  -I$(top_srcdir)/src/bin/dwarfdump \
  -I$(top_builddir)/src/lib/libdwarf
buildopstab_CFLAGS = $(DWARF_CFLAGS_WARN)


