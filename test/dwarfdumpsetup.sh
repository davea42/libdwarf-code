#!/bin/sh
# Copyright (C) 2021 David Anderson
# This script is hereby placed in the Public Domain
# for anyone to use in any way for any purpose.
#
# Assumes running from the build directory: path/test
# which is sometimes 
#    /var/tmp/bld/test 
# Sets variables top_srcdir and top_blddir
# If DWTOPSRCDIR is set on entry, top_srcdir
# is set to the value of $DWTOPSRCDIR
# as in this case we are running the script outside
# of the source tree.
#
top_blddir=`pwd`/..

if [ x$DWTOPSRCDIR = "x" ]
then
  # Running in the source tree
  top_srcdir=$top_blddir
else
  # Running outside the source tree (the normal case)
  top_srcdir=$DWTOPSRCDIR
fi
srcdir=$top_srcdir/test

echo "TOP topsrc $top_srcdir topbld $top_blddir localsrc $srcdir"
chkres() {
r=$1
m=$2
if [ $r -ne 0 ]
then
  echo "FAIL $m.  Exit status for the test $r"
fi
}

# So let dwarfdump emit more then trim.
# In addition the zero date for file time in line tables
# prints differently for different time zones.
# Delete what follows 'last time 0x0'
if [ x$win = "xy" ]
then
  textlim=702
else
  textlim=700
fi
cp "$top_srcdir/src/bin/dwarfdump/dwarfdump.conf" .
dd=$top_blddir/src/bin/dwarfdump/dwarfdump
# Remove the leading two lines for windows
# as windows dwarfdump emits two leading lines
# as compared to non-windows dwarfdump
droptwoifwin() {
i=$1
l=`wc -l < $i`
if [ $l -gt 2 ]
then
  l=`expr $l - 2`
  tail -$l <$i >junk.tmp
  cp junk.tmp $i
  rm -f junk.tmp
fi
}
fixlasttime() {
  i=$1
  sed 's/last time 0x.*/last time 0x0/' <$i >junk.tmp
  cp junk.tmp $i
  rm -f junk.tmp
}

