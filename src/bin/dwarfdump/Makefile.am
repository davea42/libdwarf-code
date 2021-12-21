###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dwarfdump

dwarfdump_SOURCES = \
dd_addrmap.c \
dd_addrmap.h \
dd_attr_form.h \
dd_attr_form.c \
dd_checkutil.c \
dd_checkutil.h \
dd_command_options.c \
dd_command_options.h \
dd_common.c \
dd_common.h \
dd_compiler_info.c \
dd_compiler_info.h \
dd_defined_types.h \
dd_regex.c \
dd_regex.h \
dd_safe_strcpy.h \
dd_safe_strcpy.c \
dwarfdump.c \
dwarfdump-af-table.h\
dwarfdump-tt-table.h\
dwarfdump-ta-table.h\
dwarfdump-tt-ext-table.h\
dwarfdump-ta-ext-table.h\
dd_tsearch.h \
dd_tsearchbal.c \
dd_dwconf.c \
dd_dwconf.h \
dd_dwconf_using_functions.h \
dd_getopt.c \
dd_getopt.h \
dd_esb.c \
dd_esb.h \
dd_esb_using_functions.h \
glflags.c \
glflags.h \
globals.h \
helpertree.c \
helpertree.h \
macrocheck.c \
macrocheck.h \
dd_makename.c \
dd_makename.h \
dd_naming.c \
dd_naming.h \
opscounttab.c \
opscounttab.h \
print_abbrevs.c \
print_aranges.c \
print_debugfission.c \
print_die.c \
print_debug_gnu.c \
print_debug_gnu.h \
print_debug_names.c \
print_debug_sup.c \
print_frames.c \
print_frames.h \
print_gdbindex.c \
print_hipc_lopc_attr.c \
print_lines.c \
print_llex_codes.c \
print_origloclist_codes.c \
print_loclists.c \
print_loclists_codes.c \
print_macinfo.c \
print_macro.c \
print_pubnames.c \
print_ranges.c \
print_rnglists.c \
print_section_groups.c \
print_sections.c \
print_sections.h \
print_static_funcs.c \
print_static_vars.c \
print_str_offsets.c \
print_strings.c \
print_tag_attributes_usage.c \
print_types.c \
print_weaknames.c \
dd_sanitized.c \
dd_sanitized.h \
dd_section_bitmaps.c \
dd_section_bitmaps.h \
dd_strstrnocase.c \
dd_true_section_name.c \
dd_tag_common.h \
dd_uri.c \
dd_uri.h

dwarfdump_CPPFLAGS = \
-I$(top_srcdir)/src/lib/libdwarf \
-I$(top_srcdir)/src/bin/dwarfdump

dwarfdump_CFLAGS = $(DWARF_CFLAGS_WARN) "-DCONFPREFIX=@prefix@/share/dwarfdump" 

dwarfdump_LDADD = \
$(top_builddir)/src/lib/libdwarf/libdwarf.la \
@DWARF_LIBS@

dwarfdumpconfdir = $(datadir)/dwarfdump
dwarfdumpconf_DATA = dwarfdump.conf

### The tests go elsewhere.
EXTRA_DIST = \
dd_tag_common.c \
dd_uritablebuild.c \
$(man_MANS) \
COPYING \
DWARFDUMPCOPYRIGHT \
GPL.txt \
ChangeLog \
ChangeLog2006 \
ChangeLog2007 \
ChangeLog2008 \
ChangeLog2009 \
ChangeLog2010 \
ChangeLog2011 \
ChangeLog2012 \
ChangeLog2013 \
ChangeLog2014 \
ChangeLog2015 \
ChangeLog2016 \
ChangeLog2017 \
ChangeLog2018 \
ChangeLog2019 \
ChangeLog2020 \
CMakeLists.txt \
NEWS \
README \
CODINGSTYLE \
testdwarfdump.conf \
$(dwarfdumpdev_DATA) \
$(dwarfdumpconf_DATA) \
attr_formclass.list \
attr_formclass_ext.list \
tag_attr.list \
tag_attr_ext.list \
tag_tree.list \
tag_tree_ext.list 
