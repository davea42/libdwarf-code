###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in

AUTOMAKE_OPTIONS = 

### The following is very wrong. FIX
all: libdwarf2.1.pdf  libdwarf2p.1.pdf

libdwarf2.1.pdf: libdwarf2.1.mm
	sh pdfbld.sh -c $(top_srcdir)/libdwarf2.1.mm

libdwarf2p.1.pdf: libdwarf2p.1.pdf
	sh pdfbld.sh -p  $(top_srcdir)/libdwarf2p.1.mm
###above wrong.

man_MANS = dwarfdump.1

dwarfdumpdevdir = $(datadir)/src/dwarfdump

EXTRA_DIST = \
$(man_MANS) 


CLEANFILES = 

