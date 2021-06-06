###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = Makefile

### The following is very wrong. FIX
all: libdwarf2.1.pdf  libdwarf2p.1.pdf

libdwarf2.1.pdf: libdwarf2.1.mm
	sh pdfbld.sh -c $(top_srcdir)/libdwarf2.1.mm

libdwarf2p.1.pdf: libdwarf2p.1.pdf
	sh pdfbld.sh -p  $(top_srcdir)/libdwarf2p.1.mm
###above wrong.

man_MANS = dwarfdump.1
man_MANS += dwarfgen.1

EXTRA_DIST = \
mips_extensions.mm \
mips_extensions.pdf \
dwarfdump.1 \
dwarfgen.1  \
libdwarf2.1.mm \
libdwarf2.1.pdf \
libdwarf2p.1.mm \
libdwarf2p.1.pdf \
pdfbld.sh \
$(man_MANS) 



