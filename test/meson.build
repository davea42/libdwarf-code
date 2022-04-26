
tests = [
  [
   'test_dwarfstring.c',
   '../src/lib/libdwarf/dwarf_string.c'
  ],
  [
   'test_extra_flag_strings.c',
   '../src/lib/libdwarf/dwarf_string.c',
   '../src/lib/libdwarfp/dwarf_pro_log_extra_flag_strings.c'
  ],
  [
   'test_linkedtopath.c',
   '../src/lib/libdwarf/dwarf_string.c',
   '../src/lib/libdwarf/dwarf_safe_strcpy.c',
   '../src/lib/libdwarf/dwarf_debuglink.c',
  ],
  [
   'getopttest.c',
   '../src/bin/dwarfdump/dd_getopt.c'
  ],
  [
   'makename_test.c',
   '../src/bin/dwarfdump/dd_esb.c',
   '../src/bin/dwarfdump/dd_makename.c',
   '../src/bin/dwarfdump/dd_tsearchbal.c'
  ],
  [
   'helpertree_test.c',
   '../src/bin/dwarfdump/dd_helpertree.c',
   '../src/bin/dwarfdump/dd_tsearchbal.c'
  ],
  [
   'testesb.c',
   '../src/bin/dwarfdump/dd_esb.c',
   '../src/bin/dwarfdump/dd_tsearchbal.c'
  ],
  [
   'test_macrocheck.c',
   '../src/bin/dwarfdump/dd_esb.c',
   '../src/bin/dwarfdump/dd_tsearchbal.c'
  ],
  [
   'section_bitmaps_test.c',
   '../src/bin/dwarfdump/dd_section_bitmaps.c'
  ],
  [
   'dwarf_leb_test.c',
   '../src/lib/libdwarf/dwarf_leb.c'
  ],
  [
   'dwarf_tied_test.c',
   '../src/lib/libdwarf/dwarf_tied.c',
   '../src/lib/libdwarf/dwarf_tsearchhash.c'
  ],
  [
   'getnametest.c',
   '../src/lib/libdwarf/dwarf_names.c'
  ],
  [
   'test_canonical.c',
   '../src/bin/dwarfdump/dd_canonical_append.c',
   '../src/bin/dwarfdump/dd_safe_strcpy.c',
   '../src/bin/dwarfdump/dd_esb.c'
  ],
  [
   'test_safe_strcpy.c',
   '../src/lib/libdwarf/dwarf_safe_strcpy.c',
  ],
  [
   'test_regex.c',
   '../src/bin/dwarfdump/dd_regex.c',
  ]
]

incdir = include_directories(
  '../src/lib/libdwarf',
  '../src/lib/libdwarfp',
  '../src/bin/dwarfdump'
)

foreach test_src : tests
  test_name = test_src[0].split('.')[0]
  test(test_name,
    executable(test_name, test_src,
      c_args : [ dev_cflags, libdwarf_args, '-DTESTING', '-DLIBDWARF_BUILD' ],
      include_directories : [ config_dir, incdir ],
      install : false
    )
  )
endforeach


shscripttests = [
  'dwarfdumpLinux.sh',
]

#py3_exe = import('python').find_installation()
#git_exe = find_program('git', required: false)
#pkgconfig_mod = import('pkgconfig')

projectbase = meson.source_root()
buildbase = meson.current_source_dir()

foreach testscr : shscripttests
  test_name = testscr.split('.')[0]
  testscrname = join_paths(projectbase,'test',testscr)
  add_test_setup(test_name,
    exe_wrapper: [ testscrname , projectbase, 'ninja'],
    gdb : false,
    is_default : true,
  )
endforeach

