project('libdwarf', ['c','cpp'],
  version: '1.99.0',
  default_options : ['buildtype=plain', 'warning_level=3', 'werror=true'],
  meson_version : '>=0.53'
)

v_arr = meson.project_version().split('.')
v_maj = v_arr[0]
v_min = v_arr[1]
v_mic = v_arr[2]

# install paths
dir_prefix = get_option('prefix')
dir_include = join_paths(dir_prefix, get_option('includedir'))
dir_pkginclude = join_paths(dir_include, meson.project_name())
dir_bin = join_paths(dir_prefix, get_option('bindir'))
dir_lib = join_paths(dir_prefix, get_option('libdir'))
dir_data = join_paths(dir_prefix, get_option('datadir'))
dir_pkgdata = join_paths(dir_data, meson.project_name())
dir_locale = join_paths(dir_prefix, get_option('localedir'))

# host

windows = import('windows')

host_os = host_machine.system()

windows = ['windows', 'cygwin']
#bsd for meson 0.46 and 0.47
bsd = ['bsd', 'freebsd', 'dragonfly', 'netbsd', 'openbsd']
linux = ['linux']
osx = ['darwin']
sun = ['sunos']

sys_linux = linux.contains(host_machine.system())
sys_bsd = bsd.contains(host_machine.system())
sys_windows = windows.contains(host_machine.system())
sys_osx = osx.contains(host_machine.system())
sys_sun = sun.contains(host_machine.system())

# compiler

cc = meson.get_compiler('c')

dev_cflags = []
dev_cflags_try = [
  '-Wpointer-arith',
  '-Wmissing-declarations',
  '-Wstrict-prototypes',
  '-Wcomment',
  '-Wformat',
  '-Wuninitialized',
  '-Wshadow',
  '-Wno-long-long',
  '-Wfloat-compare',
  '-Wsign-compare',
  '-Wno-missing-field-initializers',
  '-fno-omit-frame-pointer'
]

if sys_windows
  if cc.get_id() != 'msvc'
    dev_cflags_try += '-Wno-pedantic-ms-format'
  endif
else
  dev_cflags_try += '-fvisibility=hidden'
endif

foreach cf: dev_cflags_try
  if cc.has_argument(cf)
    dev_cflags += cf
  endif
endforeach

add_project_arguments(dev_cflags, language: 'c')
add_project_arguments(dev_cflags, language: 'cpp')

libdwarf_args = [ '-D__USE_MINGW_ANSI_STDIO=0' ]

config_dir = [include_directories('.')]

# configuration

header_checks = [
  'fcntl.h',
  'inttypes.h',
  'malloc.h',
  'stdint.h',
]

if sys_windows == false
  header_checks += 'unistd.h'
endif

config_h = configuration_data()
config_h.set_quoted('PACKAGE_NAME', meson.project_name())
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h.set_quoted('PACKAGE_BIN_DIR', dir_bin)
config_h.set_quoted('PACKAGE_LIB_DIR', dir_lib)
config_h.set_quoted('PACKAGE_DATA_DIR', dir_data)
config_h.set_quoted('LOCALEDIR', dir_locale)

if host_machine.endian() == 'big'
  config_h.set10('WORDS_BIGENDIAN', true)
endif

if cc.has_function_attribute('unused') == true
  config_h.set10('HAVE_UNUSED_ATTRIBUTE', true)
endif

foreach header : header_checks
  if cc.has_header(header)
    config_h.set10('HAVE_'+header.underscorify().to_upper(), true)
  endif
endforeach

foreach t : [ 'uint64_t', 'uintptr_t', 'intptr_t' ]
  if not cc.has_type(t, prefix: '#include <stdint.h>')
    error('Sanity check failed: type @0@ not provided via stdint.h'.format(t))
  endif
endforeach

have_libelf = false
if get_option('libelf') == true
  header_elf_checks = [ 'libelf.h', 'libelf/libelf.h' ]
  foreach header : header_elf_checks
    if cc.has_header(header)
      config_h.set10('HAVE_'+header.underscorify().to_upper(), true)
      have_libelf = true
    endif
  endforeach
  if have_libelf == true
    if cc.has_header('elf.h')
      config_h.set10('HAVE_ELF_H', true)
    endif
    if get_option('dwarfgen') == true
      elf_lib = cc.find_library('elf', required : false)
    endif
  endif
endif

subdir('src/lib/libdwarf')
#subdir('src/bin/dwarfdump')

# Use config_h after all subdirs have set values

configure_file(output : 'config.h', configuration : config_h)

# pkg-config files

pkgconf = configuration_data()

pkgconf.set('prefix', get_option('prefix'))
pkgconf.set('exec_prefix', '${prefix}')
pkgconf.set('libdir', '${prefix}/@0@'.format(get_option('libdir')))
pkgconf.set('includedir', '${prefix}/@0@'.format(get_option('includedir')))
pkgconf.set('pkgincludedir', '${prefix}/@0@'.format(get_option('includedir')) + '/libdwarf')
pkgconf.set('VMAJ', v_maj)
pkgconf.set('PACKAGE_VERSION', meson.project_version())
pkgconf.set('requirements_libdwarf_pc', 'zlib')
pkgconf.set('requirements_libdwarf_libs', '')

pkg_install_dir = '@0@/pkgconfig'.format(get_option('libdir'))

configure_file(
  input : join_paths(meson.source_root(), 'libdwarf.pc.in'),
  output : 'libdwarf.pc',
  configuration : pkgconf,
  install_dir : pkg_install_dir
)

# output

summary({'OS': host_os,
         'BuildOS-BigEndian': host_machine.endian() == 'big' ? 'yes' : 'no',
        }, section: 'Configuration Options Summary:')

summary({'zlib': zlib_dep.found() ? 'yes' : 'no',
         'libelf': have_libelf ? 'yes' : 'no',
        }, section: 'libdwarf:')

summary({'prefix': dir_prefix,
         'bindir': dir_bin,
         'libdir': dir_lib,
         'incdir': dir_include,
         'pkgincdir': dir_pkginclude,
         'datadir': dir_data,
         'pkgdatadir': dir_pkgdata,
        }, section: 'Directories:')

summary({'compilation': 'ninja',
         'installation': 'ninja install',
        }, section: 'Compilation')
