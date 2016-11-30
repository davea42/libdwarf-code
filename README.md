[![Travis Build Status](https://travis-ci.org/dvirtz/libdwarf.svg?branch=cmake)](https://travis-ci.org/dvirtz/libdwarf)
[![AppVeyor Build status](https://ci.appveyor.com/api/projects/status/oxh8pg7hsuav2jrl?svg=true)](https://ci.appveyor.com/project/dvirtz/libdwarf)

## BUILDING

To just build libdwarf and dwarfdump, if the source tree is in `/a/b/libdwarf-1`

### Using CMake

To build using CMake one might do
* `cd /a/b/libdwarf-1`
* configure: `cmake . -B_Release -DCMAKE_BUILD_TYPE=Release`
* build: `cmake --build _Release --target dd`
* (optionally install): `sudo cmake --build _Release --target install`

### Using autotools

#### Builing in Source Tree

```bash
cd /a/b/libdwarf-1
./configure
make dd
#You may need to be root to do the following copy commands
cp dwarfdump/dwarfdump      /usr/local/bin
cp dwarfdump/dwarfdump.conf /usr/local/lib
#The following is optional, not needed to run dwarfdump
#when doing the default build.
cp libdwarf/libdwarf.a      /usr/local/lib
```

#### Building Out of Source Tree

Or one could  create a new directory, for example,
```bash
mkdir /var/tmp/dwarfex
cd /var/tmp/dwarfex
/a/b/libdwarf-1/configure
make dd
```
In this case the source directory is not touched and
all objects and files created are under `/var/tmp/dwarfex`

NOTE: When building out of source tree the source tree
must be cleaned of any files created by a build
in the source tree. This is due to the way GNU Make
VPATH works.

### Build All

To build all the tools (including dwarfgen and 
dwarfexample) use `--target all` on CMake or `make all` on autotools. 
There are known small compile-time issues with building dwarfgen on 
MaxOSX and most don't need to build dwarfgen.

### Options

By default configure compiles and uses libdwarf.a.

With `-Dshared=ON` (CMake) or `--enable-shared` (autotools)
appended to the configure step, 
both libdwarf.a and libdwarf.so 
are built. The runtimes built will reference libdwarf.so.

With `-Dnonshared=OFF` (CMake) or `--disable-nonshared` (autotools)
appeded to the configure step, 
libdwarf.so is built and used; libdwarf.a is not built.

Sanity checking:
Recent gcc has some checks that can be done at runtime.
  -fsanitize=address
  -fsanitize=leak
  -fsanitize=undefined
which are turned on here by --enable-sanitize at build time.

### Distributing

When ready to create a new source distribution do
```bash
./CPTOPUBLIC
./BLDLIBDWARF yyyymmdd
# where that could be
./BLDLIBDWARF 20140131
# as an example.
```

David Anderson.  Updated October 31, 2016.
