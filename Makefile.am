###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = libdwarf dwarfdump
if HAVE_DWARFGEN
SUBDIRS += dwarfgen
endif
if HAVE_DWARFEXAMPLE
SUBDIRS += dwarfexample
endif

DISTCLEANFILES = \
libdwarf/libdwarf.h

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
missing \
$(distdir).tar.gz \
$(distdir).tar.xz

EXTRA_DIST = \
CMakeLists.txt \
README.md \
appveyor.yml \
dwarfdump/runtests.sh \
libdwarf/runtests.sh \
bugxml \
tsearch \
cmake \
configure.cmake \
codingstyle.txt \
scripts/BLDLIBDWARF \
scripts/BLDLIBDWARFTAR \
scripts/BLDTESTDIR \
scripts/CLEANUP \
scripts/CPTARTOWEBDIR \
scripts/CPTOPUBLIC \
scripts/CREATINGARELEASE \
scripts/GETTOTOP \
scripts/fixlibdwarfelf.sh \
scripts/REBLDLIBDWARF \
scripts/SETUP_MASTER_TREE \
scripts/UPD.awk \
scripts/UPDATEDWARFDUMPVERSION.sh

# AUTHORS, ChangeLog and COPYING must be present
