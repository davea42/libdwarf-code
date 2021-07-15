#!/bin/sh
# Generates Makefiles and more.

srcdir=`dirname $0`

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have autoconf installed to compile $PROJECT."
    echo "Download the appropriate package for your distribution,"
    echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    exit 1
}



# This is a beginning attempt, FIX. 
set -e -x
autoreconf --warnings=all --install --verbose --force
set +x
x=`./configure --version | head -n 1 | cut -f 3 -d " "`
echo "Version is $x"

t=src/lib/libdwarf/libdwarf_version.h
echo "/*" >$t
echo "    This date string is hereby put into the public domain." >>$t
echo "    Copyrighting this is crazy. It's just a version string" >>$t
echo "    and is modified from time to time." >>$t
echo "*/" >>$t
echo "#define DW_VERSION_DATE_STR \" $x \"" >>$t
