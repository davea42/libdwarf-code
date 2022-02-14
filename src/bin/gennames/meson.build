gennames_src = [
  '../../lib/libdwarf/dwarf_safe_strcpy.c',
  '../dwarfdump/dd_getopt.c',
  'gennames.c'
]

localinclude = include_directories('../dwarfdump')
executable('gennames', gennames_src,
  c_args : [ dev_cflags, libdwarf_args, '-I.' ],
  dependencies : libdwarf,
  include_directories : [ config_dir, localinclude],
  install : false
)
