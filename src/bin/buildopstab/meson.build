
executable('buildopstab', 'buildopscounttab.c',
  c_args : [ dev_cflags, libdwarf_args, '-I../src/bin/dwarfdump' ],
  dependencies : libdwarf,
  include_directories : config_dir,
  install : false
)
