###Copyright (C) 2021 David Anderson <davea42 at linuxmail dot org>
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in
rebuild: gennames 
	cp $(top_srcdir)/src/lib/libdwarf/dwarf.h .
	./gennames -i . -o .
	cp dwarf_names.h $(top_srcdir)/src/lib/libdwarf/dwarf_names.h
	cp dwarf_names.c $(top_srcdir)/src/lib/libdwarf/dwarf_names.c
	mv dwarf_names.h $(top_srcdir)/src/bin/dwarfdump/dwarf_names.h
	mv dwarf_names.c $(top_srcdir)/src/bin/dwarfdump/dwarf_names.c
	rm dwarf.h

noinst_PROGRAMS = gennames
gennames_SOURCES = gennames.c $(top_srcdir)/src/lib/libdwarf/dwgetopt.c
gennames_CPPFLAGS = -I$(top_srcdir)/src/lib/libdwarf \
  -I$(top_builddir)/src/lib/libdwarf
gennames_CFLAGS = $(DWARF_CFLAGS_WARN)


