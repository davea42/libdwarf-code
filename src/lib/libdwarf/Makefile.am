###Copyright (C) 2018 Vincent Torri <vincent dot torri at gmail dot com
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = junk.ltp

### libdwarf

lib_LTLIBRARIES = libdwarf.la

libdwarf_la_SOURCES = \
dwarf.h \
dwarf_abbrev.c \
dwarf_abbrev.h \
dwarf_alloc.c \
dwarf_alloc.h \
dwarf_arange.c \
dwarf_arange.h \
dwarf_base_types.h \
dwarf_crc32.c \
dwarf_debuglink.c \
dwarf_debuglink.h \
dwarf_die_deliv.c \
dwarf_die_deliv.h \
dwarf_debug_names.c \
dwarf_debug_names.h \
dwarf_debug_sup.c \
dwarf_dsc.c \
dwarf_dsc.h \
dwarf_elf_access.h \
dwarf_elf_defines.h \
dwarf_elf_load_headers.c \
dwarf_elfread.c \
dwarf_elfread.h \
dwarf_elf_rel_detector.c \
dwarf_elf_rel_detector.h \
dwarf_elf_reloc_386.h \
dwarf_elf_reloc_aarch64.h \
dwarf_elf_reloc_arm.h \
dwarf_elf_reloc_mips.h \
dwarf_elf_reloc_ppc64.h \
dwarf_elf_reloc_ppc.h \
dwarf_elf_reloc_sparc.h \
dwarf_elf_reloc_x86_64.h \
dwarf_elfstructs.h \
dwarf_errmsg_list.h \
dwarf_error.c \
dwarf_error.h \
dwarf_find_sigref.c \
dwarf_fission_to_cu.c \
dwarf_form.c \
dwarf_form_class_names.c \
dwarf_frame.c \
dwarf_frame.h \
dwarf_frame2.c \
dwarf_funcs.c \
dwarf_funcs.h \
dwarf_gdbindex.c \
dwarf_gdbindex.h \
dwarf_generic_init.c \
dwarf_global.c \
dwarf_global.h \
dwarf_gnu_index.h \
dwarf_gnu_index.c \
dwarf_groups.c \
dwarf_harmless.c \
dwarf_harmless.h \
dwarf_incl.h \
dwarf_init_finish.c \
dwarf_leb.c \
dwarf_line.c \
dwarf_line.h \
dwarf_line_table_reader_common.h \
dwarf_loc.c \
dwarf_loc.h \
dwarf_locationop_read.c \
dwarf_loclists.c \
dwarf_macho_loader.h \
dwarf_machoread.c \
dwarf_machoread.h \
dwarf_macro.c \
dwarf_macro.h \
dwarf_macro5.c \
dwarf_macro5.h \
dwarf_names.c \
dwarf_names.h \
dwarf_object_detector.c \
dwarf_object_detector.h \
dwarf_object_read_common.c \
dwarf_object_read_common.h \
dwarf_opaque.h \
dwarf_original_elf_init.c \
dwarf_pe_descr.h \
dwarf_peread.c \
dwarf_peread.h \
dwarf_print_lines.c \
dwarf_pubtypes.c \
dwarf_query.c \
dwarf_ranges.c \
dwarf_rnglists.c \
dwarf_rnglists.h \
dwarf_reading.h \
dwarf_reloc_386.h \
dwarf_reloc_arm.h \
dwarf_reloc_mips.h \
dwarf_reloc_ppc.h \
dwarf_reloc_ppc64.h \
dwarf_reloc_x86_64.h \
dwarf_str_offsets.c \
dwarf_str_offsets.h \
dwarfstring.c       \
dwarfstring.h       \
dwarf_stringsection.c \
dwarf_tied.c \
dwarf_tied_decls.h \
dwarf_tsearchhash.c \
dwarf_tsearch.h \
dwarf_types.c \
dwarf_types.h \
dwarf_util.c \
dwarf_util.h \
dwarf_vars.c \
dwarf_vars.h \
dwarf_weaks.c \
dwarf_weaks.h \
dwarf_xu_index.c \
dwarf_xu_index.h \
crc32.c \
libdwarf.h \
libdwarf_private.h \
libdwarf_version.h \
malloc_check.c \
malloc_check.h \
memcpy_swap.h \
pro_alloc.c \
pro_alloc.h \
pro_arange.c \
pro_arange.h \
pro_debug_sup.c \
pro_die.c \
pro_die.h \
pro_dnames.h \
pro_dnames.c \
pro_encode_nm.c \
pro_encode_nm.h \
pro_error.c \
pro_error.h \
pro_expr.c \
pro_expr.h \
pro_finish.c \
pro_forms.c \
pro_frame.c \
pro_frame.h \
pro_funcs.c \
pro_incl.h \
pro_init.c \
pro_line.c \
pro_line.h \
pro_log_extra_flag_strings.c \
pro_macinfo.c \
pro_macinfo.h \
pro_opaque.h \
pro_pubnames.c \
pro_reloc.c \
pro_reloc.h \
pro_reloc_symbolic.c \
pro_reloc_symbolic.h \
pro_section.c \
pro_section.h \
pro_types.c \
pro_types.h \
pro_util.h \
pro_vars.c \
pro_weaks.c

libdwarf_la_CFLAGS = $(DWARF_CFLAGS_WARN) 

libdwarf_la_LIBADD = @DWARF_LIBS@

libdwarf_la_LDFLAGS = -fPIC -no-undefined -version-info @version_info@ @release_info@

libdwarfpkgincludedir = $(pkgincludedir)-@VMAJ@
libdwarfpkginclude_DATA = \
$(top_srcdir)/src/lib/libdwarf/dwarf.h  \
$(top_srcdir)/src/lib/libdwarf/libdwarf.h

libdwarfdevdir = $(datadir)/src/lib/libdwarf/libdwarf-devel
libdwarfdev_DATA = 

EXTRA_DIST = \
COPYING \
LGPL.txt \
LIBDWARFCOPYRIGHT \
CHANGES \
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
checkexamples.c \
CODINGSTYLE \
baseline.ltp  \
dw_elfstructs.h \
dw-linetableheader.txt \
dwarf_test_errmsg_list.c \
dwarf_names_new.h \
dwarf_names_enum.h \
pro_reloc_stream.h \
pro_reloc_stream.c \
gennames.c \
dwgetopt.h \
dwgetopt.c \
CMakeLists.txt \
cmake/libdwarf-config.cmake \
NEWS \
README \
CODINGSTYLE \
$(libdwarf_DATA) \
$(libdwarfdev_DATA)

