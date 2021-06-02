#!/bin/sh
#

top_blddir=`pwd`/..
if [ x$DWTOPSRCDIR = "x" ]
then
  top_srcdir=$top_blddir
else
  top_srcdir=$DWTOPSRCDIR
fi
srcdir=$top_srcdir/test
libsrc=$top_srcdir/src/lib/libdwarf

goodcount=0
failcount=0

echo "TOP topsrc $top_srcdir libsrc $libsrc topbld $top_blddir localsrc $srcdir"
chkres() {
r=$1
m=$2
if [ $r -ne 0 ]
then
   echo "FAIL $m.  Exit status was $r"
   failcount=`expr $failcount + 1`
else
   goodcount=`expr $goodcount + 1`
fi
}

CC=cc
CFLAGS="-g -O2 -I$top_blddir -I$libsrc"
$CC $CFLAGS $srcdir/dwarf_leb_test.c $libsrc/dwarf_leb.c $libsrc/pro_encode_nm.c -o dwarfleb
chkres $? "compiling dwarfleb test"
./dwarfleb
chkres $? "Running dwarfleb test"
rm ./dwarfleb

$CC $CFLAGS $srcdir/dwarf_tied_test.c $libsrc/dwarf_tied.c $libsrc/dwarf_tsearchhash.c -o dwarftied
chkres $? "compiling dwarftied test"
./dwarftied
chkres $? "Running dwarftiedtest test"
rm ./dwarftied

$CC $CFLAGS $srcdir/test_headersok.c -o dwarfheadersok
chkres $? "compiling test_headersok.c"
rm ./test_headersok

if [ $failcount -ne 0 ]
then
   echo "FAIL $failcount in test/runtests.sh"
   exit 1
fi
exit 0
