###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = Makefile

doc: buildconsumerpdf buildproducerpdf
# These pdf builds are to be undertaken only on request.
buildconsumerpdf:
	cp $(top_srcdir)/doc/libdwarf2.1.mm .
	sh $(top_srcdir)/doc/pdfbld.sh -c
	cp libdwarf2.1.pdf $(top_srcdir)/doc/

buildproducerpdf:
	cp $(top_srcdir)/doc/libdwarf2p.1.mm .
	sh $(top_srcdir)/doc/pdfbld.sh -p
	cp libdwarf2p.1.pdf $(top_srcdir)/doc/

libdwarfdevdir = $(datadir)/libdwarf/libdwarf-devel
libdwarfdev_DATA = \
libdwarf2.1.pdf \
libdwarf2p.1.pdf

man_MANS = dwarfdump.1
man_MANS += @DWARFGEN_MANPAGE@

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



