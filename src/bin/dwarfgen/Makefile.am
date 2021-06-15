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
dwgetopt.c \
dwgetopt.h \
strtabdata.h

# dwarfdump here is so we find dwgetopt.h
dwarfgen_CPPFLAGS = -I$(top_srcdir)/src/lib/libdwarfp \
-I$(top_srcdir)/src/lib/libdwarf \
-I$(top_builddir)/src/lib/libdwarf \
-I$(top_builddir)/src/lib/libdwarfp

dwarfgen_CXXFLAGS = $(DWARF_CXXFLAGS_WARN) "-DCONFPREFIX=@prefix@/share/dwarfgen"

dwarfgen_LDADD = \
$(top_builddir)/src/lib/libdwarfp/libdwarfp.la \
$(top_builddir)/src/lib/libdwarf/libdwarf.la \
@DWARF_LIBS@ @DWARFGEN_LIBS@

EXTRA_DIST = \
COPYING \
ChangeLog \
CMakeLists.txt \
NEWS \
README \
$(dwarfgen_DATA) \
dwarf-generator.txt 
