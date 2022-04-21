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

# In addition the zero date for file time in line tables
# prints differently for different time zones.
textlim=700
cp "$top_srcdir/src/bin/dwarfdump/dwarfdump.conf" .
dd=$top_blddir/src/bin/dwarfdump/dwarfdump

# Delete what follows 'last time 0x0'
fixlasttime() {
  i=$1
  t=$2
  sed 's/last time 0x.*/last time 0x0/' <$i >$t
  mv $t $i
}

