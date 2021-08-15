###Copyright (C) 2021 David Anderson <davea42 at linuxmail dot org>
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = opscounttab.c
rebuild: buildopstab
	./buildopstab -f $(top_srcdir) >opscounttab.c
	cp opscounttab.c $(top_srcdir)/src/bin/dwarfdump/opscounttab.c

noinst_PROGRAMS = buildopstab
buildopstab_SOURCES = buildopscounttab.c
buildopstab_CPPFLAGS = -I$(top_srcdir)/src/lib/libdwarf \
  -I$(top_srcdir)/src/bin/dwarfdump \
  -I$(top_builddir)/src/lib/libdwarf
buildopstab_CFLAGS = $(DWARF_CFLAGS_WARN)
buildopstab_LDADD = $(top_builddir)/src/lib/libdwarf/libdwarf.la
