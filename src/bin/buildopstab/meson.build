
localinclude = include_directories('../dwarfdump')

executable('buildopstab', 'buildopscounttab.c',
  c_args : [ dev_cflags, libdwarf_args, '-I.' ],
  dependencies : libdwarf,
  include_directories : [config_dir,localinclude],
  install : false
)
