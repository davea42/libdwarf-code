# Copyright 2021 David Anderson
# This Makefile.am is hereby placed in the public domain
### Any error in thie file will lead to:
### error: Something went wrong bootstrapping makefile fragments
### for automatic dependency tracking
###Entering directory '/var/tmp/bld/src/lib/libdwarf'
###CC       libdwarf_la-dwarf_abbrev.lo
###/bin/bash: ../../../libtool: No such file or directory
### with no hint as to what the problem really is..

AUTOMAKE_OPTIONS=subdir-objects
MAINTAINERCLEANFILES = Makefile.in\
  dwarfdump.conf \
  getnametest.log \
  getnametest.trs \
  junk.debuglink1 \
  junk.debuglink1a \
  junk.debuglink1b \
  junk.debuglink1sed1 \
  junk.debuglink1sed2 \
  junk.debuglink2 \
  junk.debuglink2a \
  junk.debuglink2b \
  junk.jitreader.new \
  test_dwarfstring.log \
  test_dwarfstring.trs \
  test_dwgetopt.log \
  test_dwgetopt.trs \
  test_errmsglist.log \
  test_errmsglist.trs \
  test_extra_flag_strings.log \
  test_extra_flag_strings.trs \
  test_helpertree.log  \
  test_helpertree.trs \
  test_linkedtopath.log \
  test_linkedtopath.trs \
  test_macrocheck.log \
  test_macrocheck.trs \
  test_makenametest.log \
  test_makenametest.trs \
  test_objectaccess.log \
  test_objectaccess.trs \
  test_safestrcpy.log \
  test_safestrcpy.trs \
  test_sectionbitmaps.log \
  test_sectionbitmaps.trs \
  test_testesb.log \
  test_testesb.trs 

clean-local:
	rm -f junk.*
	rm -f dwarfdump.conf

TESTS = test_canonical  \
  test_dwarflebtest \
  test_dwarfstring \
  test_dwgetopt \
  test_errmsglist \
  test_extra_flag_strings \
  test_getnametest \
  test_helpertree \
  test_linkedtopath \
  test_macrocheck \
  test_makenametest \
  test_regex \
  test_safestrcpy \
  test_sectionbitmaps \
  test_testesb \
  test_tied

check_PROGRAMS = test_canonical \
  test_dwarflebtest  \
  test_dwarfstring \
  test_dwgetopt \
  test_errmsglist \
  test_extra_flag_strings \
  test_getnametest \
  test_helpertree \
  test_linkedtopath \
  test_macrocheck \
  test_makenametest \
  test_regex \
  test_safestrcpy \
  test_sectionbitmaps \
  test_testesb \
  test_tied

test_canonical_SOURCES = test_canonical.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_canonical_append.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_safe_strcpy.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_safe_strcpy.h \
    $(top_srcdir)/src/bin/dwarfdump/dd_esb.c
test_canonical_CFLAGS = $(DWARF_CFLAGS_WARN)
test_canonical_CPPFLAGS = \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/bin/dwarfdump \
-I$(top_srcdir)/src/lib/libdwarf

test_dwarflebtest_SOURCES = dwarf_leb_test.c \
    $(top_srcdir)/src/lib/libdwarf/dwarf_leb.c
test_dwarflebtest_CFLAGS = $(DWARF_CFLAGS_WARN)
test_dwarflebtest_CPPFLAGS = -DTESTING \
-DLIBDWARF_BUILD \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/bin/dwarfdump \
-I$(top_srcdir)/src/lib/libdwarf

test_dwarfstring_SOURCES = test_dwarfstring.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_string.h \
   $(top_srcdir)/src/lib/libdwarf/dwarf_string.c
test_dwarfstring_CFLAGS = $(CFLAGS_WARN) $(DWARF_CFLAGS_WARN)
test_dwarfstring_CPPFLAGS = \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/lib/libdwarf

test_dwgetopt_SOURCES = getopttest.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_getopt.h \
    $(top_srcdir)/src/bin/dwarfdump/dd_getopt.c
test_dwgetopt_CFLAGS = $(DWARF_CFLAGS_WARN)
test_dwgetopt_CPPFLAGS =  -DTESTING \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/bin/dwarfdump
 
test_errmsglist_SOURCES = test_errmsglist.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_safe_strcpy.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_safe_strcpy.h 
test_errmsglist_CFLAGS = $(DWARF_CFLAGS_WARN)
test_errmsglist_CPPFLAGS = -DTESTING \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/lib/libdwarf

test_extra_flag_strings_SOURCES = test_extra_flag_strings.c \
   $(top_srcdir)/src/lib/libdwarfp/dwarf_pro_log_extra_flag_strings.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_string.h \
   $(top_srcdir)/src/lib/libdwarf/dwarf_string.c
test_extra_flag_strings_CFLAGS = $(CFLAGS_WARN) $(DWARF_CFLAGS_WARN)
test_extra_flag_strings_CPPFLAGS =  -DTESTING \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/lib/libdwarfp \
-I$(top_srcdir)/src/lib/libdwarf

test_getnametest_SOURCES = getnametest.c \
    $(top_srcdir)/src/lib/libdwarf/dwarf_names.c
test_getnametest_CFLAGS = $(DWARF_CFLAGS_WARN)
test_getnametest_CPPFLAGS = -DTESTING \
-DLIBDWARF_BUILD \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/lib/libdwarf

test_helpertree_SOURCES = helpertree_test.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_helpertree.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_tsearchbal.c
test_helpertree_CFLAGS = $(DWARF_CFLAGS_WARN)
test_helpertree_CPPFLAGS =  -DTESTING \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/bin/dwarfdump \
-I$(top_srcdir)/src/lib/libdwarf

test_linkedtopath_SOURCES = test_linkedtopath.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_string.h \
   $(top_srcdir)/src/lib/libdwarf/dwarf_string.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_safe_strcpy.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_safe_strcpy.h \
   $(top_srcdir)/src/lib/libdwarf/dwarf_debuglink.h \
   $(top_srcdir)/src/lib/libdwarf/dwarf_debuglink.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_error.h
test_linkedtopath_CFLAGS =  $(DWARF_CFLAGS_WARN) -DTESTING
test_linkedtopath_CPPFLAGS = -DTESTING \
-DLIBDWARF_BUILD \
-I$(top_srcdir) \
-I$(top_builddir) \
-I$(top_srcdir)/src/lib/libdwarf

test_macrocheck_SOURCES = test_macrocheck.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_esb.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_tsearchbal.c
test_macrocheck_CFLAGS = $(DWARF_CFLAGS_WARN)
test_macrocheck_CPPFLAGS =  -DTESTING \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/bin/dwarfdump \
-I$(top_srcdir)/src/lib/libdwarf

test_makenametest_SOURCES = makename_test.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_esb.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_makename.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_tsearchbal.c
test_makenametest_CFLAGS = $(DWARF_CFLAGS_WARN)
test_makenametest_CPPFLAGS =  -DTESTING \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/bin/dwarfdump \
-I$(top_srcdir)/src/lib/libdwarf

test_regex_SOURCES = test_regex.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_regex.c 
test_regex_CFLAGS = $(DWARF_CFLAGS_WARN)
test_regex_CPPFLAGS = \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/bin/dwarfdump \
-I$(top_srcdir)/src/lib/libdwarf

test_safestrcpy_SOURCES = test_safe_strcpy.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_safe_strcpy.c \
   $(top_srcdir)/src/lib/libdwarf/dwarf_safe_strcpy.h 
test_safestrcpyCFLAGS = $(DWARF_CFLAGS_WARN)
test_safestrcpy_CPPFLAGS = \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/lib/libdwarf

test_sectionbitmaps_SOURCES = section_bitmaps_test.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_section_bitmaps.c
test_sectionbitmaps_CFLAGS = $(DWARF_CFLAGS_WARN)
test_sectionbitmaps_CPPFLAGS =  -DTESTING \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/bin/dwarfdump \
-I$(top_srcdir)/src/lib/libdwarf

test_testesb_SOURCES = testesb.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_esb.c \
    $(top_srcdir)/src/bin/dwarfdump/dd_tsearchbal.c
test_testesb_CFLAGS = $(DWARF_CFLAGS_WARN)
test_testesb_CPPFLAGS = -DTESTING \
-I$(top_srcdir) -I$(top_builddir) \
-I$(top_srcdir)/src/bin/dwarfdump \
-I$(top_srcdir)/src/lib/libdwarf

test_tied_SOURCES = dwarf_tied_test.c \
    $(top_srcdir)/src/lib/libdwarf/dwarf_tied.c \
    $(top_srcdir)/src/lib/libdwarf/dwarf_tsearchhash.c
test_tied_CFLAGS = $(DWARF_CFLAGS_WARN)
test_tied_CPPFLAGS = -DTESTING \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib/libdwarf

### debuglink tests are difficult to support in Windows/mingw
if HAVE_DEBUGLINK 
if HAVE_DWARFEXAMPLE
  TESTS += debuglinktest-a.sh debuglinktest-b.sh
endif
endif
TESTS += dwarfdumpLinux.sh  dwarfdumpPE.sh dwarfdumpMacos.sh 
if HAVE_DWARFEXAMPLE
TESTS += jitreaderdiff.sh
endif

### no dwarfgen tests available.

AM_TESTS_ENVIRONMENT = DWTOPSRCDIR='$(top_srcdir)'; \
    export DWTOPSRCDIR ; \
    DWCOMPILERFLAGS='$(DWARF_CFLAGS_WARN)';\
    export DWCOMPILERFLAGS ;

### dummysource ignore is to be kept, but not used. 
### See buildingdummy.sh which is also not to be used.
EXTRA_DIST= \
buildingdummy.sh \
CMakeLists.txt \
debuglink2.base \
debuglink.base \
debuglinktest-a.sh \
debuglinktest-b.sh \
dos2unix.py \
dummyexecutable \
dummyexecutable.debug \
dummysourceignore \
dwarfdumpLinux.sh  dwarfdumpMacos.sh  dwarfdumpPE.sh  dwarfdumpsetup.sh \
dwarfdumptest.py \
dwarf_leb_test.c \
dwarf_tied_test.c \
dwdiff.py \
getnametest.c \
getopttest.c \
helpertree_test.c \
jitreader.base \
jitreaderdiff.sh \
makename_test.c \
meson.build \
README.testcases \
section_bitmaps_test.c \
test_dwarfstring.c \
test_errmsglist.c \
testesb.c \
test_extra_flag_strings.c \
test_linkedtopath.c \
test-mach-o-32.base \
test-mach-o-32.dSYM \
testobjLE32PE.base \
testobjLE32PE.exe \
testobjLE32PE.test.c \
testuriLE64ELf.base \
testuriLE64ELf.obj \
transformpath.py

