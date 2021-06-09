###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = Makefile

doc: buildconsumerpdf buildproducerpdf
# These pdf builds are to be undertaken only on request.
buildconsumerpdf:
	cp $(top_srcdir)/doc/libdwarf.mm .
	sh $(top_srcdir)/doc/pdfbld.sh -c
	cp libdwarf.pdf $(top_srcdir)/doc/

buildproducerpdf:
	cp $(top_srcdir)/doc/libdwarfp.mm .
	sh $(top_srcdir)/doc/pdfbld.sh -p
	cp libdwarfp.pdf $(top_srcdir)/doc/

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
libdwarf.mm \
libdwarf.pdf \
libdwarfp.mm \
libdwarfp.pdf \
pdfbld.sh \
$(man_MANS) 



