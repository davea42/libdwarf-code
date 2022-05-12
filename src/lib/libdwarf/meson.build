
libdwarf_header_src = [ 'libdwarf.h', 'dwarf.h' ]

libdwarf_src = [
  'dwarf_abbrev.c',
  'dwarf_alloc.c',
  'dwarf_arange.c',
  'dwarf_crc.c',
  'dwarf_crc32.c',
  'dwarf_debuglink.c',
  'dwarf_die_deliv.c',
  'dwarf_debugnames.c',
  'dwarf_debug_sup.c',
  'dwarf_dsc.c',
  'dwarf_elf_load_headers.c',
  'dwarf_elfread.c',
  'dwarf_elf_rel_detector.c',
  'dwarf_error.c',
  'dwarf_find_sigref.c',
  'dwarf_fission_to_cu.c',
  'dwarf_form.c',
  'dwarf_form_class_names.c',
  'dwarf_frame.c',
  'dwarf_frame2.c',
  'dwarf_funcs.c',
  'dwarf_gdbindex.c',
  'dwarf_generic_init.c',
  'dwarf_global.c',
  'dwarf_gnu_index.c',
  'dwarf_groups.c',
  'dwarf_harmless.c',
  'dwarf_init_finish.c',
  'dwarf_leb.c',
  'dwarf_line.c',
  'dwarf_loc.c',
  'dwarf_locationop_read.c',
  'dwarf_loclists.c',
  'dwarf_machoread.c',
  'dwarf_macro.c',
  'dwarf_macro5.c',
  'dwarf_memcpy_swap.c',
  'dwarf_names.c',
  'dwarf_object_detector.c',
  'dwarf_object_read_common.c',
  'dwarf_peread.c',
  'dwarf_print_lines.c',
  'dwarf_pubtypes.c',
  'dwarf_query.c',
  'dwarf_safe_strcpy.c',
  'dwarf_ranges.c',
  'dwarf_rnglists.c',
  'dwarf_str_offsets.c',
  'dwarf_string.c',
  'dwarf_stringsection.c',
  'dwarf_tied.c',
  'dwarf_tsearchhash.c',
  'dwarf_types.c',
  'dwarf_util.c',
  'dwarf_vars.c',
  'dwarf_weaks.c',
  'dwarf_xu_index.c',
]

zlib_deps = dependency('zlib', method: 'pkg-config', required: false)

if zlib_deps.found() == true
  config_h.set10('HAVE_ZLIB', true)
  config_h.set10('HAVE_ZLIB_H', true)
endif

libdwarf_lib = library('dwarf', libdwarf_src,
  c_args : [ dev_cflags, libdwarf_args, '-DLIBDWARF_BUILD' ],
  dependencies : zlib_deps,
  gnu_symbol_visibility: 'hidden',
  include_directories : config_dir,
  install : true,
  pic : true,
  version : meson.project_version()
)

libdwarf = declare_dependency(
  include_directories : [ include_directories('.')],
  link_with : libdwarf_lib,
  dependencies : zlib_deps
)

install_headers(libdwarf_header_src,
  install_dir : dir_pkginclude + '-' + v_maj
)
