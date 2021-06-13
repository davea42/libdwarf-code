[![Travis Build
Status](https://travis-ci.com/davea42/libdwarf-code.svg?branch=master)](https://travis-ci.com/github/davea42/libdwarf-code)
# This is README.md
#Updated 13 June 2021
## BUILDING from a git clone of the source tree with configure
   in source base dir, do
      x=`pwd`
      sh autogen.sh
      rm -rf /tmp/empty
      mkdir /tmp/empty
      cd /tmp/empty
      # CFLAGS and --enable-wall and
      # --enable-dwarfgen and --enable-dwarfexample are optional.
      CFLAGS='-g -O0' $x/configure \
         --enable-dwarfexample --enable-dwarfgen \
         --enable-wall
      make 
      make check

## BUILDING from a git clone of the source tree with cmake
   # See also README.cmake
   # where code is the directory name for the libdwarf project
   rm -rf /tmp/bld
   mkdir /tmp/bld
   cd /tmp/bld
   cmake -DDO_TESTING=ON -DBUILD_DWARFEXAMPLE=ON \
      -DBUILD_DWARFGEN=ON  /path/to/code
   ctest -R self

## BUILDING from a tar.gz or tar.xz in code (base) dir:
   rm -rf /tmp/bld
   x=`pwd`
   mkdir /tmp/bld
   cd /tmp/bld
   # CFLAGS and --enable-wall and
   # --enable-dwarfgen and --enable-dwarfexample are optional.
   CFLAGS='-g -O0' $x/configure \
       --enable-dwarfexample --enable-dwarfgen \
       --enable-wall

To build using CMake one might do
* `cd /a/b/libdwarf-1`
* configure: `cmake . -B_Release -DCMAKE_BUILD_TYPE=Release`
* build: `cmake --build _Release --target dd`
* (optionally install): `sudo cmake --build _Release --target install`

# for autotools builds, see README

### Using autotools
IGNORE THE FOLLOWING. NEEDS REWRITE

#### Building out of Source Tree
 
 NOTE: When building out of source tree the source tree
 must be cleaned of any files created by a build
 in the source tree. This is due to the way GNU Make
 VPATH works.
### Build All
 
### Options

By default configure compiles and uses libdwarf.a.
With `-Dshared=ON` (CMake) or `--enable-shared` (autotools)
appended to the configure step, 
both libdwarf.a and libdwarf.so 
 are built. The runtimes built will reference libdwarf.so.

With `-Dnonshared=FALSE` (CMake) or `--disable-nonshared` (autotools)
appeded to the configure step, 
 libdwarf.so is built and used; libdwarf.a is not built.

Sanity checking:
 Recent gcc has some checks that can be done at runtime.
   -fsanitize=undefined
 which are turned on here by --enable-sanitize at build time.

### Distributing

When ready to create a new source distribution do
a build and then 
  make dist

David Anderson. 

