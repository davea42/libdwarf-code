gennames_src = [
  '../../lib/libdwarf/dwarf_safe_strcpy.c',
  '../dwarfdump/dd_getopt.c',
  'gennames.c'
]

executable('gennames', gennames_src,
  c_args : [ dev_cflags, libdwarf_args, '-I../src/bin/dwarfdump' ],
  dependencies : libdwarf,
  include_directories : config_dir,
  install : false
)
