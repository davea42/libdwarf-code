
attr_form_src = [
  'attr_form_build.c',
  '../tag_tree/tag_common.c',
  '../dwarfdump/dd_attr_form.c',
  '../dwarfdump/dd_common.c',
  '../dwarfdump/dd_esb.c',
  '../dwarfdump/dd_getopt.c',
  '../dwarfdump/dd_glflags.c',
  '../dwarfdump/dd_makename.c',
  '../dwarfdump/dd_naming.c',
  '../dwarfdump/dd_safe_strcpy.c',
  '../dwarfdump/dd_sanitized.c',
  '../dwarfdump/dd_tsearchbal.c'
]

attr_form_args = [
  '-DTRIVIAL_NAMING',
  '-DBUILD_NONLIB_SOURCE',
  '-DSKIP_AF_CHECK',
]
localinclude = include_directories('../dwarfdump')

executable('attr_form', attr_form_src,
  c_args : [ dev_cflags, libdwarf_args, attr_form_args ],
  dependencies : libdwarf,
  include_directories : [config_dir, localinclude],
  install : false
)
