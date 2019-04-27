Created 26 April 2019 

The configure script scripts/FIX-COMPILE-TIMES does not apply
to cmake. Don't bother to use it if you build with cmake.
The FIX-COMPILE-TIMES script is irrelevant to cmake.

The cmake build has been revised somewhat.  By default the
build builds just libdwarf and dwarfdump.

cmake testing is not enabled (for now). The testing
only tests various internal interfaces, not
libdwarf interfaces.

Lets assume the base directory of the the
libdwarf package in a directory named code at /path/to/code
     cmake /path/to/code
Will build libdwarf (a static library, a libdwarf.a) and
dwarfdump (linking to that static library).  And, in this
case there is no libelf so dwarfdump won't read archives or
honor requests to print elf headers.

To show all the available cmake options we'll show the
default build,

The default build is identical to
    cmake -Dlibelf:BOOL=yes \
        -Ddodwarfgen:BOOL=NO \
        -Ddodwarfexample:BOOL=NO \
        -Dstatic:BOOL=YES \
        -Dshared:BOOL=NO \
        -Dwall:BOOL=NO \
        -Dnonstandardprintf:BOOL=NO
    make

The short form, doing the same as the default:
    cmake /path/to/code
    make

The short form, nolibelf:
    cmake -Dlibelf:BOOL=NO /path/to/code
For this case any attempt to compile dwarfgen will be
overridden: dwarfgen requires libelf.

For dwarfexample:
    cmake -Ddodwarfexample:BOOL=YES /path/to/code
    make

If libelf is missing -Ddodwarfgen:BOOL=YES will not be honored
as dwarfgen will not build without libelf.

    
