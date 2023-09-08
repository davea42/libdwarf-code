
## MacOS builds of libdwarf and dwarfdump 

Tested on MacOS Ventura

## Getting set up
Install Mac Command Line Tools from Apple if
you do not have compilers installed.

Point your browser to macports.org and
install the port command
by downloading a .pkg file, clicking on the downloaded
package, and following the instructions.

Close Terminal (if open).

Open a terminal window (at this point
the port program will be in your $PATH ).

    sudo port install zlib
    sudo port install zstd
    sudo port install meson
    sudo port select --set python3 python311

## Basic validation 

At this point normal configure/cmake/meson
builds should work and 'make check' works too.

From the source directory one can run:

    sh scripts/buildandreleasetest.sh




