#!/bin/sh
#
# Intended to be run only on local machine.
# Run only after config.h created in a configure
# in the source directory
# Assumes env vars DWTOPSRCDIR set to the path to source.
# Assumes we run the script in the test directory.
# place of the top director itself (may be a relative
# path).

chkres() {
r=$1
m=$2
if [ $r -ne 0 ]
then
  echo "FAIL $m.  Exit status for the test $r"
  exit 1
fi
}

blddir=`pwd`
if [ $# -eq 2 ]
then
  DWTOPSRCDIR="$1"
  blddir="$2"
  top_blddir=$blddir
else
  # DWTOPSRCDIR an env var.
  blddir=`pwd`
  top_blddir=`dirname $blddir`
fi
if [ x$DWTOPSRCDIR = "x" ]
then
  top_srcdir=$top_blddir
  echo "top_srcdir from top_blddir $top_srcdir"
else
  top_srcdir=$DWTOPSRCDIR
  echo "top_srcdir from DWTOPSRCDIR $top_srcdir"
fi
if [ "x$top_srcdir" = "x.." ]
then
  # This case hopefully eliminates relative path to test dir. 
  top_srcdir=$top_blddir
fi
# bld loc to find dwdebuglink executable.
bldloc=$top_blddir/src/bin/dwarfexample
#localsrc is the build directory
localsrc=$top_srcdir/test
if [ $localsrc = "." ]
then
  localsrc=$top_srcdir/test
fi

testbin=$top_blddir/test
testsrc=$top_srcdir/test
# So we know the build. Because of debuglink.
echo "DWARF_BIGENDIAN=$DWARF_BIGENDIAN"

echo "TOP topsrc  : $top_srcdir"
echo "TOP topbld  : $top_blddir"
echo "TOP localsrc: $localsrc"

if [ x"$DWARF_BIGENDIAN" = "xyes" ]
then
  echo "SKIP debuglinktest-a.sh , cannot work on bigendian build "
else
  echo "debuglinktest-a.sh "
  o=junk.debuglink1
  p="--add-debuglink-path=/exam/ple"
  p2="--add-debuglink-path=/tmp/phony"
  echo "Run: $bldloc/dwdebuglink $p $p2 $testsrc/dummyexecutable "
  $bldloc/dwdebuglink $p $p2 $testsrc/dummyexecutable > $testbin/$o
  r=$?
  chkres $r "debuglinktest-a.sh running dwdebuglink test1"
  if [ $r -ne 0 ]
  then
    echo "Error debuglinktest-a.sh"
    exit $r
  fi
  # we strip out the actual localsrc and blddir for the obvious
  # reason: We want the baseline data to be meaningful no matter
  # where one's source/build directories are.
  echo $localsrc | sed "s:[.]:\[.\]:g" >$testbin/${o}sed1
  sedv1=`head -n 1 $testbin/${o}sed1`
  sed "s:$sedv1:..src..:" <$testbin/$o  >$testbin/${o}a
  echo $blddir | sed "s:[.]:\[.\]:g" >$testbin/${o}sed2
  sedv2=`head -n 1 $testbin/${o}sed2`
  sed "s:$sedv2:..bld..:" <$testbin/${o}a  >$testbin/${o}b
  diff $testsrc/debuglink.base  $testbin/${o}b
  r=$?
  chkres $r "running debuglinktest-a.sh test1 diff against baseline"
  if [ $r -ne 0 ]
  then
     echo "To update debuglinktest-a.sh baseline:"
     echo "mv $testbin/${o}b $testsrc/debuglink.base"
     exit $r
  fi
fi
exit 0
