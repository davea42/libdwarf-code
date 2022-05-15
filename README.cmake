Created 26 April 2019 
Updated 14 May 2022

Now we are using a new source structure and using semantic
versioning for tar.xz names (earlier we used tar.gz).
The assumption is that you usually download an appropriate
libdwarf-0.4.1.tar.gz (or a later one)
You may find README or README.md useful to read.

===========================
See below for msys2 mingw cmake.

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

===========================
USING MINGW (WINDOWS) CMAKE.

to use mingw64 one must install the right packages
(speaking here of mingw for Windows, not plain Windows)

Do not use the -DWALL option to cmake, that trips
a minor warning in gcc (treated as an error).

At present we do not know how to get the linkages
from the build-tree executables to the libdwarf dll
so make check has several failures.
We suggest you use meson for  msys2 builds.

After following the instructions for the msys2
and minw64 install, ensure the following useful
items are
installed with packman -S> Use  pacman -Q
with no options to see what is installed.

The cmake and meson items below are only 
necessary if using cmake or meson to build.
There are also i686 versions of some of
these if you wish to build 32bit windows applications.

cmake will generate ninja makefiles by default,
add -G "Unix Makefiles" to the cmake command line
to generate makefiles for gnu make.


Use
-DBUILD_SHARED=ON
-DBUILD_STATIC=OFF
on the cmake command
to be consistent with normal Windows use.

mingw-w64-x86_64-binutils
mingw-w64-x86_64-brotli
mingw-w64-x86_64-bzip2
mingw-w64-x86_64-c-ares
mingw-w64-x86_64-ca-certificates
mingw-w64-x86_64-cmake
mingw-w64-x86_64-crt-git
mingw-w64-x86_64-curl
mingw-w64-x86_64-expat
mingw-w64-x86_64-gcc
mingw-w64-x86_64-gcc-ada
mingw-w64-x86_64-gcc-fortran
mingw-w64-x86_64-gcc-libgfortran
mingw-w64-x86_64-gcc-libs
mingw-w64-x86_64-gcc-objc
mingw-w64-x86_64-gdb
mingw-w64-x86_64-gdb-multiarch
mingw-w64-x86_64-gettext
mingw-w64-x86_64-gmp
mingw-w64-x86_64-headers-git
mingw-w64-x86_64-isl
mingw-w64-x86_64-jansson
mingw-w64-x86_64-jemalloc
mingw-w64-x86_64-jsoncpp
mingw-w64-x86_64-libarchive
mingw-w64-x86_64-libb2
mingw-w64-x86_64-libffi
mingw-w64-x86_64-libgccjit
mingw-w64-x86_64-libiconv
mingw-w64-x86_64-libidn2
mingw-w64-x86_64-libmangle-git
mingw-w64-x86_64-libpsl
mingw-w64-x86_64-libssh2
mingw-w64-x86_64-libsystre
mingw-w64-x86_64-libtasn1
mingw-w64-x86_64-libtre-git
mingw-w64-x86_64-libunistring
mingw-w64-x86_64-libuv
mingw-w64-x86_64-libwinpthread-git
mingw-w64-x86_64-libxml2
mingw-w64-x86_64-lz4
mingw-w64-x86_64-make
mingw-w64-x86_64-meson
mingw-w64-x86_64-mpc
mingw-w64-x86_64-mpdecimal
mingw-w64-x86_64-mpfr
mingw-w64-x86_64-ncurses
mingw-w64-x86_64-nghttp2
mingw-w64-x86_64-ninja
mingw-w64-x86_64-openssl
mingw-w64-x86_64-p11-kit
mingw-w64-x86_64-pkgconf
mingw-w64-x86_64-python
mingw-w64-x86_64-readline
mingw-w64-x86_64-rhash
mingw-w64-x86_64-sqlite3
mingw-w64-x86_64-tcl
mingw-w64-x86_64-termcap
mingw-w64-x86_64-tk
mingw-w64-x86_64-tools-git
mingw-w64-x86_64-tzdata
mingw-w64-x86_64-windows-default-manifest
mingw-w64-x86_64-winpthreads-git
mingw-w64-x86_64-winstorecompat-git
mingw-w64-x86_64-xxhash
mingw-w64-x86_64-xz
mingw-w64-x86_64-zlib
mingw-w64-x86_64-zstd

To get a usable set of executables
set a prefix for cmake of, for example,
-DCMAKE_INSTALL_PREFIX=/c/msys64/usr
as the bin is something in your $PATH in msys2.
copy src/bin/dwarfdump/dwarfdump.conf to ~
do 
  make install
  # Run with
  # copy src/bin/dwarfdump/dwarfdump.conf to ~
  # then
  dwarfdump.exe
  # which will give a short message  about
  # No object file provided. In which case 
  # dwarfdump is usable.
  




