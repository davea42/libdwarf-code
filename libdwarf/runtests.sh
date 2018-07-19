#!/bin/sh
#
# Intended to be run only on local machine.
# Run in the libdwarf directory
# Run only after config.h created in a configure
# in the source directory

srcdir=`pwd`
top_srcdir=$srcdir/..
cd $top_srcdir/libdwarf

chkres() {
r=$1
m=$2
if [ $r -ne 0 ]
then
   echo "FAIL $m.  Exit status $r"
   exit 1
fi
}

CC=cc
CFLAGS="-g -O2 -I$top_srcdir -I$top_srcdir/libdwarf"
$CC $CFLAGS -DTESTING  dwarf_leb.c pro_encode_nm.c -o dwarfleb
chkres $? "compiling dwarfleb test"
./dwarfleb
chkres $? "Running dwarfleb test"
rm ./dwarfleb

$CC $CFLAGS -DTESTING dwarf_tied.c dwarf_tsearchhash.c -o dwarftied
chkres $? "compiling dwarftied test"
./dwarftied
chkres $? "Running dwarftiedtest test"
rm ./dwarftied
exit 0


