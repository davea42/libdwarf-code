#!/bin/sh
#
# Intended to be run only on local machine.
# Run in the dwarfdump directory
# Run only after config.h created in a configure
# in the source directory
# Assumes env vars DWTOPSRCDIR set to the path to source.
# Assumes CFLAGS warning stuff set in env var DWCOMPILERFLAGS
# Assumes we run the script in the dwarfdump directory.

blddir=`pwd`
top_blddir=`pwd`/..
if [ x$DWTOPSRCDIR = "x" ]
then
  top_srcdir=$top_blddir
else
  top_srcdir=$DWTOPSRCDIR
fi
srcdir=$top_srcdir/dwarfexample
if [ x"$DWCOMPILERFLAGS" = 'x' ]
then
  CFLAGS="-g -O2 -I$top_blddir -I$top_srcdir/libdwarf  -I$top_blddir/libdwarf -Wall -Wextra"
  echo "CFLAGS basic default default  $CFLAGS"
else
  CFLAGS="-g -O2 -I$top_blddir -I$top_srcdir/libdwarf  -I$top_blddir/libdwarf $DWCOMPILERFLAGS"
  echo "CFLAGS via configure $CFLAGS"
fi

goodcount=0
failcount=0

echo "TOP topsrc $top_srcdir topbld $top_blddir localsrc $srcdir"
chkres() {
r=$1
m=$2
if [ $r -ne 0 ]
then
  echo "FAIL $m.  Exit status for the test $r"
  failcount=`expr $failcount + 1`
else 
  goodcount=`expr $goodcount + 1`
fi
}

which cc
if [ $? -eq 0 ]
then
  CC=cc
else
  which gcc
  if [ $? -eq 0 ]
  then
    CC=gcc
  else
    # we will fail
    CC=cc
  fi
fi

echo "getdebuglink test"
o=junk.debuglink
$blddir/getdebuglink a b $srcdir/dummyexecutable > $blddir/$o
chkres $? "running getdebuglink"
diff $srcdir/debuglink.base  $blddir/$o
r=$?
if [ $r -ne 0 ]
then
   echo "To update getdebuglink baseline: mv $blddir/$o $srcdir/debuglink.base"
fi
chkres $r "running getdebuglink diff against baseline"

if [ $failcount -gt 0 ] 
then
   echo "FAIL $failcount dwarfexample/runtests.sh"
   exit 1
fi
exit 0
