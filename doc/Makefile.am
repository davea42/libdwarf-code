###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in

doc: buildconsumerpdf buildproducerpdf
# These pdf builds are to be undertaken only on request.
buildconsumerpdf:
	doxygen $(top_srcdir)/doc/Doxyfile
	cd latex ; make
	cp latex/refman.pdf $(top_srcdir)/doc/libdwarf.pdf 

buildproducerpdf:
	cp $(top_srcdir)/doc/libdwarfp.mm .
	sh $(top_srcdir)/doc/pdfbld.sh -p
	cp libdwarfp.pdf $(top_srcdir)/doc/

man_MANS = dwarfdump.1
if HAVE_DWARFGEN
man_MANS += dwarfgen.1
endif

dox_verbose = $(dox_verbose_@AM_V@)
dox_verbose_ = $(dox_verbose_@AM_DEFAULT_V@)
dox_verbose_0 = @echo "  DOX      $@";

if HAVE_DOC
all-local: doxygen

doxygen:
	$(dox_verbose)$(DOXYGEN)

install-data-local:
	$(MKDIR_P) $(docdir)
	cp -r html $(docdir)
	cp -r latex $(docdir)
endif

clean-local:
	rm -rf html latex

uninstall-local:
	rm -rf $(docdir)/html $(docdir)/latex

EXTRA_DIST = \
Doxyfile \
mips_extensions.mm \
mips_extensions.pdf \
checkexamples.c \
dwarfdump.1 \
dwarfgen.1  \
libdwarf.pdf \
libdwarfp.mm \
libdwarfp.pdf \
pdfbld.sh
