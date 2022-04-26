###Copyright (C) 2021 David Anderson <davea42 at linuxmail dot org>
###This code is public domain and can be freely used or copied.

AUTOMAKE_OPTIONS=subdir-objects
MAINTAINERCLEANFILES = Makefile.in
rebuild: attrform
	rm -f tempaftab
	cp $(top_srcdir)/src/bin/attr_form/attr_formclass.list af-t1.c
	$(CC) -E -I$(top_srcdir)/src/lib/libdwarf af-t1.c >tmp-af-build1.tmp
	cp $(top_srcdir)/src/bin/attr_form/attr_formclass_ext.list af-t2.c
	$(CC) -E -I$(top_srcdir)/src/lib/libdwarf af-t2.c >tmp-af-build2.tmp
	./attrform -s -i tmp-af-build1.tmp -o tempaftab
	./attrform -e -i tmp-af-build2.tmp -o tempaftab
	mv tempaftab $(top_srcdir)/src/bin/dwarfdump/dwarfdump-af-table.h
	rm -f af-t1.c
	rm -f af-t2.c
	rm -f tmp-af-build1.tmp
	rm -f tmp-af-build2.tmp

noinst_PROGRAMS = attrform
attrform_SOURCES = attr_form_build.c \
$(top_srcdir)/src/bin/dwarfdump/dd_common.c \
$(top_srcdir)/src/bin/dwarfdump/dd_attr_form.c \
$(top_srcdir)/src/bin/dwarfdump/dd_tsearchbal.c \
$(top_srcdir)/src/bin/dwarfdump/dd_getopt.c \
$(top_srcdir)/src/bin/dwarfdump/dd_esb.c \
$(top_srcdir)/src/bin/dwarfdump/dd_makename.c \
$(top_srcdir)/src/bin/dwarfdump/dd_naming.c \
$(top_srcdir)/src/bin/dwarfdump/dd_glflags.c \
$(top_srcdir)/src/bin/dwarfdump/dd_sanitized.c \
$(top_srcdir)/src/bin/dwarfdump/dd_safe_strcpy.c \
$(top_srcdir)/src/bin/tag_tree/tag_common.c
### In the following dwarfdump before libdwarf
### is essential to avoid _dwarf_tsearch link error.
attrform_CPPFLAGS = -DBUILD_NONLIB_SOURCE -DSKIP_AF_CHECK -DTRIVIAL_NAMING \
 -I$(top_builddir) \
 -I$(top_srcdir)/src/bin/dwarfdump  \
 -I$(top_srcdir)/src/lib/libdwarf
attrform_CFLAGS = $(DWARF_CFLAGS_WARN)
attrform_LDADD = $(top_builddir)/src/lib/libdwarf/libdwarf.la

EXTRA_DIST= \
attr_formclass.list \
attr_formclass_ext.list \
meson.build
