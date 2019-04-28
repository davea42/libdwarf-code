Created 26 April 2019 
Updated 28 April 2019

The configure script scripts/FIX-COMPILE-TIMES does not apply
to cmake. Don't bother to use it if you build with cmake.
The FIX-COMPILE-TIMES script is irrelevant to cmake.

The cmake build has been revised somewhat.  By default the
build builds just libdwarf and dwarfdump.

cmake testing is not enabled (for now). The testing only tests
various internal interfaces, not libdwarf interfaces. Neither
'make test' nor 'make install' have been verified to work
properly with Makefiles created by cmake (27 April 2019)

Lets assume the base directory of the the libdwarf source in a
directory named 'code' inside the directory '/path/to/' Always
arrange to issue the cmake command in an empty directory.
For example:

    mkdir /tmp/bld
    cd /tmp/bld
    cmake /path/to/code
    make


Will build libdwarf (a static library, a libdwarf.a) and
dwarfdump (linking to that static library).  If there is no
libelf.h present during cmake/build then dwarfdump won't read
archives or honor requests to print elf headers.

To show all the available cmake options we'll show the
default build,

The default build is identical to

    cmake -Dlibelf=ON \
        -Dstatic=ON \
        -Dshared=OFF \
        -Ddodwarfgen=OFF \
        -Ddodwarfexample=OFF \
        -Dwall=OFF \
        -Dnonstandardprintf=OFF
    make

The short form, doing the same as the default:

    cmake /path/to/code
    make

The short form, nolibelf, for when you wish to build without
libelf even if libelf.h and libelf are present:

    cmake -Dlibelf=OFF /path/to/code
    make

For this case any attempt to compile dwarfgen will be
overridden: dwarfgen requires libelf.

For dwarfexample:

    cmake -Ddodwarfexample=ON /path/to/code
    make

If libelf is missing -Ddodwarfgen=ON will not be honored
as dwarfgen will not build without libelf.
