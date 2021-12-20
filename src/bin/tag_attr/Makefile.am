###Copyright (C) 2021 David Anderson <davea42 at linuxmail dot org>
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in
rebuild: tagattr 
	cp $(top_srcdir)/src/bin/dwarfdump/tag_attr.list tmp-t2.c
	$(CC) -DTRIVIAL_NAMING -I$(top_srcdir)/src/lib/libdwarf -E tmp-t2.c >tmp-tag-attr-build2.tmp
	./tagattr -s -i tmp-tag-attr-build2.tmp -o dwarfdump-ta-table.h
	cp dwarfdump-ta-table.h $(top_srcdir)/src/bin/dwarfdump/
	rm -f tmp-tag-attr-build2.tmp
	rm -f tmp-t2.c
	rm -f tmp-t3.c
	cp $(top_srcdir)/src/bin/dwarfdump/tag_attr_ext.list tmp-t3.c
	$(CC) -DTRIVIAL_NAMING -I$(top_srcdir)/src/lib/libdwarf -E tmp-t3.c > tmp-tag-attr-build3.tmp
	./tagattr -e -i tmp-tag-attr-build3.tmp -o dwarfdump-ta-ext-table.h
	cp dwarfdump-ta-ext-table.h $(top_srcdir)/src/bin/dwarfdump/

noinst_PROGRAMS = tagattr
tagattr_SOURCES = tag_attr.c \
$(top_srcdir)/src/bin/dwarfdump/dwarf_getopt.c \
$(top_srcdir)/src/bin/dwarfdump/dwarf_tsearchbal.c \
$(top_srcdir)/src/bin/dwarfdump/dd_common.c \
$(top_srcdir)/src/bin/dwarfdump/dd_esb.c \
$(top_srcdir)/src/bin/dwarfdump/dd_makename.c \
$(top_srcdir)/src/bin/dwarfdump/dd_naming.c \
$(top_srcdir)/src/bin/dwarfdump/dd_sanitized.c \
$(top_srcdir)/src/bin/dwarfdump/glflags.c \
$(top_srcdir)/src/bin/dwarfdump/dd_safe_strcpy.c \
$(top_srcdir)/src/bin/dwarfdump/tag_common.c
tagattr_CPPFLAGS = -DTRIVIAL_NAMING \
-DBUILD_NONLIB_SOURCE \
-I$(top_srcdir)/src/lib/libdwarf \
-I$(top_srcdir)/src/bin/dwarfdump
tagattr_CFLAGS = $(DWARF_CFLAGS_WARN)
tagattr_LDADD = $(top_builddir)/src/lib/libdwarf/libdwarf.la


