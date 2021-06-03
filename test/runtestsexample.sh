#!/bin/sh
#
# Intended to be run only on local machine.
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
srcloc=$top_srcdir/src/bin/dwarfexample
bldloc=$top_blddir/src/bin/dwarfexample
testbin=$top_blddir/test
testsrc=$top_srcdir/test
# So we know the build. Because of debuglink.
echo "DWARF_BIGENDIAN=$DWARF_BIGENDIAN"
if [ x"$DWCOMPILERFLAGS" = 'x' ]
then
  CFLAGS="-g -O2 -I$top_blddir -I$top_srcdir/src/lib/libdwarf  -I$top_blddir/src/lib/libdwarf -Wall -Wextra"
  echo "CFLAGS basic default default  $CFLAGS"
else
  CFLAGS="-g -O2 -I$top_blddir -I$top_srcdir/src/lib/libdwarf  -I$top_blddir/src/lib/libdwarf $DWCOMPILERFLAGS"
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

# echo "y" or "n"
srcrelativepath () {
  p="$1"
  rm -f /tmp/srcrel.$$
  echo "$p" >/tmp/srcrel.$$
  grep '^/' </tmp/srcrel.$$ >/dev/null 2>/dev/null
  if [ $? -eq 0 ]
  then 
    echo "n"
  else
    echo "y"
  fi
  rm /tmp/srcrel.$$
}

#lsiok=y
#ls -i $blddir >/dev/null 2>/dev/null
#r=$?
#if [ $r -ne 0 ]
#then
#  echo "SKIP dwdebuglink tests cannot work when ls -i does not work"
#  lsiok=n
#fi
rel=`srcrelativepath "$srcdir"`
if [  $rel = "y" ]
then
  echo "dwdebuglink tests: $srcdir is a relative path"
  echo "so we are building in the source tree. Use $blddir "
  srcdir="$blddir"
  #This depends on perhaps-incorrect belief that 
  #only srcdir can be a relative path (if
  #the test is run out-of-source-tree srcdir will
  #be a full path too). 
  # For both srcdir and blddir the final directory component will
  # be 'dwarfexample'
fi

if [ x"$DWARF_BIGENDIAN" = "xyes" ]
then
  echo "SKIP dwdebuglink test1, cannot work on bigendian build "
else
  echo "dwdebuglink test1"
  o=junk.debuglink1
  p="--add-debuglink-path=/exam/ple"
  p2="--add-debuglink-path=/tmp/phony"
  $bldloc/dwdebuglink $p $p2 $testsrc/dummyexecutable > $testbin/$o
  chkres $? "running dwdebuglink test1"
  # we strip out the actual srcdir and blddir for the obvious
  # reason: We want the baseline data to be meaningful no matter
  # where one's source/build directories are.
  echo $srcdir | sed "s:[.]:\[.\]:g" >$testbin/${o}sed1
  sedv1=`head -n 1 $testbin/${o}sed1`
  sed "s:$sedv1:..src..:" <$testbin/$o  >$testbin/${o}a
  echo $blddir | sed "s:[.]:\[.\]:g" >$testbin/${o}sed2
  sedv2=`head -n 1 $testbin/${o}sed2`
  sed "s:$sedv2:..bld..:" <$testbin/${o}a  >$testbin/${o}b
  diff $testsrc/debuglink.base  $testbin/${o}b
  r=$?
  if [ $r -ne 0 ]
  then
     echo "To update dwdebuglink baseline:"
     echo "mv $testbin/${o}b $testsrc/debuglink.base"
  fi
  chkres $r "running dwdebuglink test1 diff against baseline"
fi

echo "dwdebuglink test2"
o=junk.debuglink2
p=" --no-follow-debuglink --add-debuglink-path=/exam/ple"
p2="--add-debuglink-path=/tmp/phony"
$bldloc/dwdebuglink $p $p2 $testsrc/dummyexecutable > $testbin/$o
chkres $? "running dwdebuglink test2"
# we strip out the actual srcdir and blddir for the obvious
# reason: We want the baseline data to be meaningful no matter
# where one's source/build directories are.
sed "s:$srcdir:..src..:" <$testbin/$o  >$testbin/${o}a
sed "s:$blddir:..bld..:" <$testbin/${o}a  >$testbin/${o}b
diff $testsrc/debuglink2.base  $testbin/${o}b
r=$?
if [ $r -ne 0 ]
then
   echo "To update dwdebuglink test2 baseline: mv $testbin/${o}b $testsrc/debuglink2.base"
fi
chkres $r "running dwdebuglink test2 diff against baseline"

if [ $failcount -gt 0 ] 
then
   echo "FAIL $failcount dwarfexample/runtestsexample.sh"
   exit 1
fi
exit 0
