###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dwarfgen

dwarfgen_SOURCES = \
createirepformfrombinary.cc \
createirepfrombinary.h \
createirepfrombinary.cc \
dwarfgen.cc \
general.h \
irepattrtodbg.cc \
irepattrtodbg.h \
irepdie.h \
irepform.h \
irepframe.h \
irepline.h \
irepmacro.h \
ireppubnames.h \
irepresentation.h \
ireptodbg.cc \
ireptodbg.h \
strtabdata.h

# dwarfdump here is so we find dwgetopt.h
dwarfgen_CPPFLAGS = -I$(top_srcdir)/src/lib/libdwarf -I$(top_srcdir)/src/lib/dwarfdump -I$(top_builddir)/src/lib/libdwarf

dwarfgen_CXXFLAGS = $(DWARF_CXXFLAGS_WARN)

dwarfgen_LDADD = \
$(top_builddir)/src/bin/dwarfdump/dwarfdump-dwgetopt.o \
$(top_builddir)/src/lib/libdwarf/libdwarf.la \
@DWARF_LIBS@

EXTRA_DIST = \
COPYING \
ChangeLog \
ChangeLog2011 \
ChangeLog2012 \
ChangeLog2013 \
ChangeLog2014 \
ChangeLog2015 \
ChangeLog2016 \
ChangeLog2017 \
ChangeLog2018 \
CMakeLists.txt \
NEWS \
README \
TESTmallocfail \
$(dwarfgen_DATA) \
dwarf-generator.txt \
fakemalloc.in
