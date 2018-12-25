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
scripts/baseconfig.h           scripts/CPTARTOWEBDIR       scripts/funcfindersrcs.py \
scripts/BLDLIBDWARF            scripts/CPTOPUBLIC          scripts/GETTOTOP \
scripts/BLDLIBDWARFTAR         scripts/CREATINGARELEASE    scripts/libbuild.sh \
scripts/BLDTESTDIR             scripts/dateorder           scripts/REBLDLIBDWARF \
scripts/buildandreleasetest.sh scripts/ddbuild.sh          scripts/RUNPY \
scripts/buildstandardsource.sh scripts/FIX-CONFIGURE-TIMES scripts/SETUP_MASTER_TREE \
scripts/ChangeLog              scripts/fixlibdwarfelf.sh   scripts/UPDATEDWARFDUMPVERSION.sh \
scripts/CLEANUP	               scripts/funcfinderhdr.py    scripts/UPD.awk \
scripts/conddef.py             scripts/funcfindermm.py

# AUTHORS, ChangeLog and COPYING must be present
