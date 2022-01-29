Created 26 April 2019 
Updated 29 January 2022

Now we are using a new source structure and using semantic
versioning for tar.xz names (earlier we used tar.gz).
The assumption is that you usually download an appropriate
libdwarf-0.3.3.tar.gz (or a later one)
You may find README or README.md useful to read.

============================
For cmake, ignore the autogen.sh
script in the base source directory.
By default the build builds just libdwarf and dwarfdump.

Lets assume the base directory of the the libdwarf source in a
directory named 'code' inside the directory '/path/to/' Always
arrange to issue the cmake command in an empty directory.
For example:

    mkdir /tmp/cmbld
    cd /tmp/cmbld
    cmake /path/to/code
    make

The above will build libdwarf (a static library, a libdwarf.a)
and dwarfdump (linking to that static library).  If there is
no libelf.h present during cmake/build then dwarfdump won't
read archives or honor requests to print elf headers.

To show all the available cmake options we'll show the
default build next:

    cmake -DDWARF_WITH_LIBELF=ON \
        -DBUILD_NON_SHARED=ON \
        -DBUILD_SHARED=OFF \
        -DBUILD_DWARFGEN=OFF \
        -DBUILD_DWARFEXAMPLE=OFF \
        -DWALL=OFF \
        -DDO_TESTING=OFF\
        /path/to/code
    make


The options after -DDO_TESTING should not normally be
used, they are for testing old features and not relevant to
modern usage.

The short form, doing the same as the default:
    cmake /path/to/code
    make

For this case any attempt to compile dwarfgen will be
overridden: dwarfgen requires libelf.

For dwarfexample:

    cmake -DBUILD_DWARFEXAMPLE=ON /path/to/code
    make

If libelf is missing -DBUILD_DWARFGEN=ON will not be honored
as dwarfgen will not build without libelf.

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

cmake make install and make dist do not yet work properly.
Use configure for those.

