
examples = [
  'dwdebuglink.c',
  'findfuncbypc.c',
  'frame1.c',
  'jitreader.c',
  'showsectiongroups.c',
  'simplereader.c'
]

libdwarf_dir = include_directories('../../lib/libdwarf')

foreach example_src : examples
  example_name = example_src.split('.')[0]
  executable(example_name, example_src,
    c_args : [ dev_cflags, libdwarf_args ],
    dependencies : libdwarf,
    include_directories : [ config_dir, libdwarf_dir ],
    install : false
  )
endforeach
