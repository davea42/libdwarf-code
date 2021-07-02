###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src/lib/libdwarf src/bin/dwarfdump

if HAVE_DWARFGEN
  SUBDIRS += src/lib/libdwarfp
  SUBDIRS += src/bin/dwarfgen
endif

if HAVE_DWARFEXAMPLE
SUBDIRS += src/bin/dwarfexample
endif

SUBDIRS += test

SUBDIRS += doc

doc:
	make -C doc doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libdwarf.pc
if HAVE_DWARFGEN 
pkgconfig_DATA +=  libdwarfp.pc
endif

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
libdwarfp.pc.in \
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
.travis.yml

# AUTHORS, ChangeLog and COPYING must be present
