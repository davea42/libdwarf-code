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

