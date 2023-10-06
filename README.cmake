Created 26 April 2019
Updated 19 June 2023

Consider switching entirely to meson for your build.

There are two parts of this file:
CMAKE on Unix/linux/MacOS/FreeBSD/OpenBSD
and
USING MSYS2 (WINDOWS) CMAKE.

Unless a shared library is specifically requested
cmake builds a static library: libdwarf.a

============================
CMAKE on Unix/linux/MacOS/FreeBSD/OpenBSD
For cmake, ignore the autogen.sh
script in the base source directory.
By default the build builds just libdwarf and dwarfdump.

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

END of cmake on Linux/Unix/Macos/Freebsd/Openbsd
===========================
USING MSYS2 (WINDOWS) CMAKE, MESON, CONFIGURE

Do not use the -DWALL option to cmake, that trips
a minor warning in gcc (treated as an error).

We suggest you use meson for  msys2 builds.

Go to msys2.org

Download a recent .exe from the downloads page.
I downloaded msys2-x86_64-20230718.exe
Execute it and follow the instructions on msys2.org

the following should get you sufficent files for
building and testing all the build mechanisms:

Avoid using --enable-wall. Avoid other
compiler options (as presented in
CFLAGS=<someting) unless you verify they
cause no build issue.

    basics
    pacman -Suy
    pacman -S base-devel git autoconf automake libtool
    pacnam -S mingw-w64-x86_64-python3 
    pacman -S mingw-w64-x86_64-toolchain
    pacman -S mingw-w64-x86_64-zlib 
    pacman -S mingw-w64-x86_64-zstd 
    pacman -S mingw-w64-x86_64-doxygen

    extras for meson/cmake
    pacman -S mingw-w64-x86_64-meson
    pacman -S mingw-w64-x86_64-cmake
    pacnam -S mingw-w64-x86_64-python3-pip

    To create a distribution
    pacman -S mingw-w64-x86_64-xz

    to list packages
    pacman -Q 
    to remove packages
    pacman -R  <packagename>

cmake will generate ninja makefiles by default, add
'-G "Unix Makefiles"' to the cmake command line to
generate makefiles for gnu make, but we suggest you
use "-G Ninja" for speed and clarity..

Use
-DBUILD_SHARED:BOOL=TRUE  \
-DBUILD_NON_SHARED:BOOL=FALSE
on the cmake command
to be consistent with normal Windows use.

To get a usable set of executables
set a prefix (for cmake,
-DCMAKE_INSTALL_PREFIX=/c/msys64/usr),
presuming  the bin directory
is something in your $PATH in msys2.
Set an appropriate prefix whichever
build tool you use.

copy src/bin/dwarfdump/dwarfdump.conf to ~
do 
  ninja install
  # Run with
  # copy src/bin/dwarfdump/dwarfdump.conf to ~
  # then
  dwarfdump.exe
  # which will give a short message  about
  # No object file provided. In which case 
  # dwarfdump is usable.
END USING MSYS2 (WINDOWS) CMAKE.
===========================
