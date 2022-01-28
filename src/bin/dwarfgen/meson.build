
dwarfgen_src = [
  'createirepformfrombinary.cc',
  'createirepfrombinary.cc',
  'dg_getopt.c',
  'dwarfgen.cc',
  'irepattrtodbg.cc',
  'ireptodbg.cc'
]

libdwarfp_dir = include_directories('../../lib/libdwarfp')

executable('dwarfgen', dwarfgen_src,
  cpp_args : [ dev_cppflags, libdwarf_args ],
  dependencies : [ libdwarf, libdwarfp ],
  include_directories : [ config_dir, libdwarfp_dir ],
  install : true
)
