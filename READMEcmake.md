# Cmake on Unix/linux/MacOS/FreeBSD/OpenBSD
Created 26 April 2019
Updated 6 October 2023

Consider switching entirely to meson for your build.

There are two parts of this file:
CMAKE on Unix/linux/MacOS/FreeBSD/OpenBSD

Unless a shared library is specifically requested
cmake builds a static library: libdwarf.a

For cmake, ignore the autogen.sh
script in the base source directory.

By default cmake builds just libdwarf and dwarfdump.

Lets assume the base directory of the the libdwarf source in a
directory named 'code' inside the directory '/path/to/' Always
arrange to issue the cmake command in an empty directory.
For example:

    # build the fast way
    mkdir /tmp/cmbld
    cd /tmp/cmbld
    cmake -G Ninja -DDO_TESTING:BOOL=TRUE /path/to/code
    ninja
    ninja test

    # much slower build
    mkdir /tmp/cmbld
    cd /tmp/cmbld
    cmake -DDO_TESTING:BOOL=TRUE  /path/to/code
    make
    ctest -R self

The above will build libdwarf.dll or libdwarf-0.dll 
and dwarfdump (linking to that dll). 

To show all the available cmake options for 'code':

    cmake -L /path/to/code

For dwarfexample:

    cmake -G Ni  -DBUILD_DWARFEXAMPLE=ON /path/to/code
    make
    cmake -DDO_TESTING=ON /path/to/code
    make
    # To list the tests
    ctest -N
    # To run all the tests (their names start with 
    # the letters 'self').
    ctest -R self

By default ctest just shows success or failure with no details.
To debug a cmake test, for example if test 22 fails and you
want to know what the test output is, use the following:

    ctest --verbose -I 22

In case one wishes to see the exact compilation/linking options
passed at compile time use
    make VERBOSE=1
instead of plain
    make

cmake make install works somewhat usefully, but
generates two libdwarf.pc files, only one of which
is usable. It is unclear why two such.

We suggest you use meson or configure for install

