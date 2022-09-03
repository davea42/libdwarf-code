# This is libdwarf README[.md]

Updated 3 September 2022

ci runs builds on Linux, Freebsd, msys2, and MacOS
using configure,cmake, and meson.

[![ci](https://github.com/davea42/libdwarf-code/actions/workflows/test.yml/badge.svg)](https://github.com/davea42/libdwarf-code/actions/workflows/test.yml)

Versions before 0.4.2 had problems
in the test suite so
make check or ninja test might fail
on some tests (sorry).

## REQUIREMENTS from a libdwarf<name>.tar.xz

Mentioning some that might not be automatically
in your base OS release. Restricting attention
here to just building libdwarf and dwarfdump. 

If the objects you work with do not have
compressed-elf-section content zlib/libz
are not required for building/using 
libdwarf/dwarfdump.

    Ubuntu: 
    sudo apt install pkgconf zlib1g zlib1g-dev
    optional add: cmake meson ninja doxygen 

    FreeBSD:
    pkg install bash python3 gmake binutils pkgconf lzlib
    optional add: cmake meson ninja doxygen

    Ensure that all the needed programs are in $PATH,
    including python3.  

## BUILDING from a libdwarf<name>.tar.xz

This is always recommended as it's not necessary
to have GNU autotools installed.
These examples show doing a build in a directory
different than the source as that is generally
recommended practice. 

### GNU configure/autotools build

Note: if you get a build failure that mentions
something about test/ and missing .Po object files
add --disable-dependency-tracking to the configure
command.
    
    rm -rf /tmp/build
    mkdir /tmp/build
    cd /tmp
    tar xf <path to>/libdwarf-0.4.2.tar.xz
    cd  /tmp/build
    /tmp/libdwarf-0.4.2/configure
    make
    make check

### cmake build

README.cmake has details on the available cmake options.

We suggest that you will find meson a more satisfactory
tool.

### meson build

    meson 0.45.1  on Ubuntu 18.04 fails.
    meson 0.55.2  on Ubuntu 20.04 works.
    meson 0.60.3  on Freebsd 12.2 and Freebsd 13.0 works.

See README.cmake for the mingw64 msys2 packages to install
and the command(s) to do that in msys2.
The tools listed there are also for msys2 meson and
autotools/configure.

The meson ninja install not only installs libdwarf-0.dll
and dwarfdump.exe it updates the executables in
the build tree linking to that dll so all such
executables in the build tree work too.

For example:

    meson /tmp/libdwarf-0.4.2
    ninja
    ninja install
    ninja test

For a faster build, adding additional checks:

    export CFLAGS="-g -pipe"
    export CXXFLAGS="-g -pipe"
    meson /tmp/libdwarf-0.4.2 -Ddwarfexample=true 
    ninja -j8
    ninja install
    ninja test

## BUILDING on linux from a git clone with configure/autotools

Ignore this section if using meson (or cmake).

This is not recommended as it requires you have
GNU autotools and pkg-config installed.
Here we assume the source is in  a directory named
/path/to/code

For example, on Ubuntu 20.04
```
    sudo apt-get install autoconf libtool pkg-config
```

Note: if you get a build failure that mentions
something about test/ and missing .Po object files
add --disable-dependency-tracking to the configure
command.

Using the source/build directories from above as examples,
do :

    # Standard Linux Build
    cd /path/to/code
    sh autogen.sh
    cd /tmp/build
    /path/to/code/configure
    make
    make check

## BUILDING on MacOS from a git clone configure
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    brew install autoconf automake libtool
    # Then use the  Standard Linux Build lines above.

### Options to meson on Linux/Unix 

For the basic configuration options list , do:
    meson configure /path/to/code

To set options and show the resulting actual options:

    # Here  just setting one option.
    meson setup  -Ddwarfexample=true  .  /home/davea/dwarf/code 
    meson configure .

The meson configure output is very wide (just letting you know).

### Options to configure/autotools on Linux/Unix 

For the full options list , do:

    /path/to/code/configure --help

By default configure compiles and uses libdwarf.a.
With `--enable-shared` appended to the configure step,
both libdwarf.a and libdwarf.so
are built and the runtimes built will reference libdwarf.so.

If you get a build failure that mentions
something about test/ and missing .Po object files
add --disable-dependency-tracking to the configure
command. With that option do not assume you can
alter source files and have make rebuild all
necessary.
See:
https://www.gnu.org/savannah-checkouts/gnu/automake/history/automake-history.html#Dependency-Tracking-Evolution

With `--enable-shared --disable-static`
appended to the configure step,
 libdwarf.so is built and used; libdwarf.a is not built.

Other options of possible interest:

    --enable-wall to turn on compiler diagnostics 
    --enable-dwarfexample to compile the example programs.

    configure -h     shows the options available.  

Sanity checking:
 gcc has some checks that can be done at runtime.
 -fsanitize=undefined is turned on by --enable-sanitize

### Options to meson on Windows

All libdwarf builds are automatically shared object (dll)
builds. No static library libdwarf.a for installation
is supported.

Has the same meson setup reporting as on Linux (above).

### Options to configure on Windows

All libdwarf builds are automatically shared object (dll)
builds. No static libdwarf.a can be installed.

Has the same meson setup reporting as on Linux (above).

### Distributing via configure/autotools

When ready to create a new source distribution do
a build and then

    make distcheck

# INCOMPATIBILITIES. Changes to interfaces

Comparing libdwarf-0.4.2 to libdwarf-0.4.1
No incompatibilities. 0.4.2 not released.

Comparing libdwarf-0.4.1 to libdwarf-0.4.0
Added a new function dwarf_suppress_debuglink_crc()
which speeds up gnu debuglink (only use it if
you are sure the debuglink name-check alone is sufficent).

Comparing libdwarf-0.4.0 to libdwarf-0.3.4
A few  dealloc() functions changed name to have
a consistent pattern for all such.
Access to the DWARF5 .debug_names section
is now fully implemented. 

See the <strong>Recent Changes</strong> section in
libdwarf.pdf (in the release).

[dwhtml]: https://www.prevanders.net/libdwarfdoc/index.html
[dwpdf]: https://www.prevanders.net/libdwarf.pdf

Or see the latest online html version [dwhtml] for the details..
Or see (via download) the latest pdf html version [dwpdf].

Notice the table of contents at the right edge of the html page.

## Reading DWARF from memory 

If one has DWARF bytes in memory or in a
kind of file system libdwarf cannot understand
one should use 

    dwarf_object_init_b()
    ...call libdwarf functions...
    dwarf_finish()

and create source to provide
functions and data for the three struct
types:

    struct Dwarf_Obj_Access_Interface_a_s
    struct Dwarf_Obj_Access_Methods_a_s
    struct Dwarf_Obj_Access_Section_a_s

These functions and structs now seem complete
(unlike the earlier libdwarf versions), hence
the name and content changes.

For a worked out example of reading DWARF direct from memory
with no file system involved
see

    src/bin/dwarfexample/jitreader.c

and see the html [dwhtml] (www.prevanders.net/libdwarfdoc/index.html).

The latest pdf is [dwpdf] (www.prevanders.net/libdwarf.pdf) 

David Anderson.
