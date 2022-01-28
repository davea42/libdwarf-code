
libdwarfp_header_src = [ 'libdwarfp.h' ]

libdwarfp_src = [
  '../libdwarf/dwarf_memcpy_swap.c',
  '../libdwarf/dwarf_string.c',
  '../libdwarf/dwarf_tsearchhash.c',
  'dwarf_pro_alloc.c',
  'dwarf_pro_arange.c',
  'dwarf_pro_debug_sup.c',
  'dwarf_pro_die.c',
  'dwarf_pro_dnames.c',
  'dwarf_pro_error.c',
  'dwarf_pro_expr.c',
  'dwarf_pro_finish.c',
  'dwarf_pro_forms.c',
  'dwarf_pro_frame.c',
  'dwarf_pro_funcs.c',
  'dwarf_pro_init.c',
  'dwarf_pro_line.c',
  'dwarf_pro_log_extra_flag_strings.c',
  'dwarf_pro_macinfo.c',
  'dwarf_pro_pubnames.c',
  'dwarf_pro_reloc.c',
  'dwarf_pro_reloc_stream.c',
  'dwarf_pro_reloc_symbolic.c',
  'dwarf_pro_section.c',
  'dwarf_pro_types.c',
  'dwarf_pro_vars.c',
  'dwarf_pro_weaks.c'
]

elf_deps = cc.find_library('elf', required : false)

have_getehdr = false
if cc.has_function('elf64_getehdr', dependencies: elf_deps) == true
  config_h.set10('HAVE_ELF64_GETEHDR', true)
  have_getehdr = true
endif

have_getshdr = false
if cc.has_function('elf64_getshdr', dependencies: elf_deps) == true
  config_h.set10('HAVE_ELF64_GETSHDR', true)
  have_getshdr = true
endif

libdwarf_dir = include_directories('../libdwarf')

libdwarfp_lib = library('libdwarfp', libdwarfp_src,
  c_args : [ dev_cflags, libdwarf_args, '-DLIBDWARFP_BUILD' ],
  dependencies : [ elf_deps, libdwarf ],
  gnu_symbol_visibility: 'hidden',
  include_directories : [ config_dir, libdwarf_dir ],
  install : true,
  pic : true,
  version : meson.project_version()
)

libdwarfp = declare_dependency(
  include_directories : [ include_directories('.')],
  link_with : libdwarfp_lib,
  dependencies : elf_deps
)

install_headers(libdwarfp_header_src,
  install_dir : dir_pkginclude + '-' + v_maj
)
