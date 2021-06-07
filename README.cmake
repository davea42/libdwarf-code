Created 26 April 2019 
Updated 6 June 2021

Now we are using
a new source structure and using semantic versioning
for tar.gz names.
The assumption is that you usually download an appropriate
libdwarf-0.1.0.tar.gz (or a later one)

============================
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
        -DHAVE_CUSTOM_LIBELF=OFF \
        -DHAVE_WINDOWS_PATH=OFF \
        -DHAVE_OLD_FRAME_CFA_COL=OFF \
        -DHAVE_SGI_IRIX_OFFSETS=OFF \
        -DHAVE_STRICT_DWARF2_32BIT_OFFSET=OFF \
        /path/to/code
    make

Ignore the -DHAVE_CUSTOM_LIBELF line, that option is not
intended for you :-) .

The options after -DHAVE_WINDOWS_PATH should not normally be
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

=============ctest not working 6 June 2021

If you wish to run the selftests (both internal tests and
runs of dwarfdump on a couple selected object files):

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

On Unix/Linux cmake 'make install' will install to
"/usr/local".  The cmake 'make install' here is not
guaranteed to do everything one might hope with libdwarf.
To set another install target set
CMAKE_INSTALL_PREFIX.  Example:

   mkdir /tmp/cmitest
   cmake -DCMAKE_INSTALL_PREFIX=/tmp/cmitest
   make install
