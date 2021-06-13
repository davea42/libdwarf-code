###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src/lib/libdwarf src/bin/dwarfdump test


if HAVE_DWARFGEN
SUBDIRS += src/bin/dwarfgen
SUBDIRS += src/lib/libdwarfp
endif

if HAVE_DWARFEXAMPLE
SUBDIRS += src/bin/dwarfexample
endif

SUBDIRS += doc

doc:
	make -C doc doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libdwarf.pc

DISTCLEANFILES = 

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
ar-lib \
compile \
config.guess \
config.h.in \
config.h.in~ \
config.sub \
configure \
depcomp \
install-sh \
ltmain.sh \
m4/libtool.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4 \
m4/lt~obsolete.m4 \
missing  \
test-driver \
$(distdir).tar.gz  \
$(distdir).tar.xz 

EXTRA_DIST = \
appveyor.yml \
autogen.sh \
bugxml \
ChangeLog \
ChangeLog2018 \
ChangeLog2019 \
ChangeLog2020 \
cmake \
CMakeLists.txt \
codingstyle.txt \
cmake/config.h.cmake \
README \
README.md \
README.cmake \
libdwarf.pc.in \
scripts/baseconfig.h  \
scripts/buildandreleasetest.sh \
scripts/buildstandardsource.sh \
scripts/run-all-tests.sh \
scripts/ChangeLog \
scripts/ChangeLog2018 \
scripts/ChangeLog2019 \
scripts/ChangeLog2020 \
scripts/CLEANUP \
scripts/ddbuild.sh  \
scripts/libbuild.sh \
scripts/UPDATEDWARFDUMPVERSION.sh \
tsearch/ChangeLog \
tsearch/ChangeLog2014 \
tsearch/ChangeLog2015 \
tsearch/ChangeLog2016 \
tsearch/ChangeLog2017 \
tsearch/ChangeLog2018 \
tsearch/ChangeLog2019 \
tsearch/ChangeLog2020 \
tsearch/config.h \
tsearch/dwarf_incl.h \
tsearch/dwarf_tsearchbal.c \
tsearch/dwarf_tsearchbin.c \
tsearch/dwarf_tsearch.c \
tsearch/dwarf_tsearchepp.c \
tsearch/dwarf_tsearch.h \
tsearch/dwarf_tsearchhash.c \
tsearch/dwarf_tsearchred.c \
tsearch/ESSAY.txt \
tsearch/Makefile \
tsearch/README \
tsearch/RUNTEST \
tsearch/scripts \
tsearch/tsearch.c \
tsearch/tsearchlibtimes.csv \
tsearch/tsearchlibtimes.ods \
tsearch/tsearch_tester.c \
.travis.yml

# AUTHORS, ChangeLog and COPYING must be present
