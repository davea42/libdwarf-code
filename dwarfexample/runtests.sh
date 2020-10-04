#!/bin/sh
#
# Intended to be run only on local machine.
# Run in the dwarfdump directory
# Run only after config.h created in a configure
# in the source directory
# Assumes env vars DWTOPSRCDIR set to the path to source.
# Assumes CFLAGS warning stuff set in env var DWCOMPILERFLAGS
# Assumes we run the script in the dwarfexample directory.

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

echo "dwdebuglink test1"
o=junk.debuglink1
p="--add-debuglink-path=/exam/ple"
p2="--add-debuglink-path=/tmp/phony"
$blddir/dwdebuglink $p $p2 $srcdir/dummyexecutable > $blddir/$o
chkres $? "running dwdebuglink"
# we strip out the actual srcdir and blddir for the obvious
# reason: We want the baseline data to be meaningful no matter
# where one's source/build directories are.
sed "s:$srcdir:..src..:" <$blddir/$o  >$blddir/${o}a
sed "s:$blddir:..bld..:" <$blddir/${o}a  >$blddir/${o}b
diff $srcdir/debuglink.base  $blddir/${o}b
r=$?
if [ $r -ne 0 ]
then
   echo "To update dwdebuglink baseline: mv $blddir/${o}b $srcdir/debuglink.base"
fi
chkres $r "running dwdebuglink test1 diff against baseline"

echo "dwdebuglink test2"
o=junk.debuglink2
p=" --no-follow-debuglink --add-debuglink-path=/exam/ple"
p2="--add-debuglink-path=/tmp/phony"
$blddir/dwdebuglink $p $p2 $srcdir/dummyexecutable > $blddir/$o
chkres $? "running dwdebuglink"
# we strip out the actual srcdir and blddir for the obvious
# reason: We want the baseline data to be meaningful no matter
# where one's source/build directories are.
sed "s:$srcdir:..src..:" <$blddir/$o  >$blddir/${o}a
sed "s:$blddir:..bld..:" <$blddir/${o}a  >$blddir/${o}b
diff $srcdir/debuglink2.base  $blddir/${o}b
r=$?
if [ $r -ne 0 ]
then
   echo "To update dwdebuglink baseline: mv $blddir/${o}b $srcdir/debuglink2.base"
fi
chkres $r "running dwdebuglink test2 diff against baseline"




if [ $failcount -gt 0 ] 
then
   echo "FAIL $failcount dwarfexample/runtests.sh"
   exit 1
fi
exit 0
