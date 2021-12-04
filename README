[![Travis Build
Status](https://travis-ci.com/davea42/libdwarf-code.svg?branch=master)](https://travis-ci.com/github/davea42/libdwarf-code)

# This is libdwarf README.md

Updated 04 December 2021
For release libdwarf-0.3.1

## BUILDING from a libdwarf<name>.tar.xz

This is always recommended as it's not necessary
to have GNU autotools installed.
These examples show doing a build in a directory
different than the source as that is generally
recommended practice. 

### GNU configure build

    rm -rf /tmp/build
    mkdir /tmp/build
    cd /tmp
    tar xf <path to>/libdwarf-0.2.0.tar.xz
    cd  /tmp/build
    /tmp/libdwarf-0.2.0/configure
    make
    make check

### cmake build

README.cmake has details on the available cmake options.

Just like configure, except instead of configure do:

    cmake  /tmp/libdwarf-0.2.0
    make
    ctest -R self

## BUILDING from a git clone of the source tree with configure

This is not recommended as it requires you have
GNU autotools and pkg-config installed.
Here we assume the source is in  a directory named
/path/to/code

For example, on Ubuntu 20.04
```
    sudo apt-get install autoconf libtool pkg-config
```

Using the source/build directories from above as examples,
do:

    cd /path/to/code
    sh autogen.sh
    cd /tmp/build
    /path/to/code/configure
    make
    make check

## BUILDING from a git clone of the source tree with cmake

It's always recommended to do cmake builds in a clean directory.
See also README.cmake
Start just as in the above configure steps by running
autogen.sh, then instead of configure do:

    cmake  /path/to/code
    make
    ctest -R self

### Options to configure on Linux/Unix 

By default configure compiles and uses libdwarf.a.
With `--enable-shared` appended to the configure step,
both libdwarf.a and libdwarf.so
are built and the runtimes built will reference libdwarf.so.

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

### Options to configure on Windows

All libdwarf builds are automatically shared object (dll)
builds. No static libdwarf.a is supported.

### Distributing

When ready to create a new source distribution do
a build and then

    make distcheck

# INCOMPATIBILITIES. Changes to interfaces

Comparing libdwarf-0.2.0 to libdwarf-20210528
(the final non-semantic-version release)
there are significant changes. 

Many functions that only supported
DWARF before DWARF5 have been dropped
in favor of functions that support all
DWARF versions through DWARF5.
The later versions add a 
`_a_` (or `_b_` or `_c_` or `_d_`)
to the end of the function name.
In nearly all cases the libdwarf-0.2.0 interface
was already available in libdwarf-20210528 
along with older interfaces that only
worked with earlier DWARF.

## Specific functions whose argument lists changed
Some arguments were removed from

    dwarf_init_b()
    dwarf_init_path()
    dwarf_init_path_dl()
    dwarf_object_init_b()
    dwarf_finish()
 
the arguments were unused and/or unnecessary.
The argument list to 

    dwarf_bitoffset()

changed to allow use with DWARF5.

libdwarf.h still has

    dwarf_expand_frame_instructions()
    struct Dwarf_Frame_Op_s

but these will be dropped
since they only work with DWARF2.
A usable interface is not yet available.

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

David Anderson.
