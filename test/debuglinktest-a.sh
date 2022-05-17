#!/bin/sh
#
# Intended to be run only on local machine.
# Run only after config.h created in a configure
# in the source directory
#
# Either pass in the top source dir as an argument
# or set env var DWTOPSRCDIR to the source directory.

chkres() {
r=$1
m=$2
if [ $r -ne 0 ]
then
  echo "FAIL $m.  Exit status for the test $r"
  exit 1
fi
}

echo "Argument count: $#"
if [ $# -gt 0 ]
then
  top_srcdir="$1"
else
  if [ x$DWTOPSRCDIR = "x" ]
  then
    top_srcdir=$top_blddir
    echo "top_srcdir from top_blddir $top_srcdir"
  else
    top_srcdir=$DWTOPSRCDIR
    echo "top_srcdir from DWTOPSRCDIR $top_srcdir"
  fi
fi
blddir=`pwd`
top_blddir="$blddir/.."

if [ "x$top_srcdir" = "x.." ]
then
  # Should never happen if run by make or equivalent
  # This case hopefully eliminates relative path to test dir. 
  top_srcdir=$top_blddir
fi
# bld loc to find dwdebuglink executable.
bldloc=$top_blddir/src/bin/dwarfexample
#localsrc is the source dir with baseline data
localsrc=$top_srcdir/test

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
  o=junk.dlinka
  p="--add-debuglink-path=/exam/ple"
  p2="--add-debuglink-path=/tmp/phony"
  echo "Run: $bldloc/dwdebuglink $p $p2 $testsrc/dummyexecutable "
  $bldloc/dwdebuglink $p $p2 $testsrc/dummyexecutable > $testbin/$o
  r=$?
  chkres $r "debuglinktest-a.sh running dwdebuglink test1"
  # we strip out the actual localsrc and blddir for the obvious
  # reason: We want the baseline data to be meaningful no matter
  # where one's source/build directories are.
  echo $localsrc | sed -e "s@[.]@\[.\]@g" >$testbin/${o}sed1
  echo "testbin/${o}sed1 = $testbin/${o}sed1"
  sedv1=`head -n 1 $testbin/${o}sed1`
  # Transform source dir name from literal to ..src.. so
  # test diff not dependent on local file path. 
  # use @ instead of / or \ or : to avoid tripping over
  # normal path characters in Linux/Macos/Windows
  sed -e "s@$sedv1@..src..@" <$testbin/$o  >$testbin/${o}a
  cat $testbin/${o}a
  echo " source path now changed to ..src.."
  echo "testbin/${o}a  = $testbin/${o}a"
  echo $blddir | sed -e "s@[.]@\[.\]@g" >$testbin/${o}sed2
  echo "testbin/${o}sed2  = $testbin/${o}sed2"
  sedv2=`head -n 1 $testbin/${o}sed2`
  # Transform build dir name from literal to ..bld.. so
  # test diff not dependent on local file path. 
  sed -e "s@$sedv2@..bld..@" <$testbin/${o}a  >$testbin/${o}b
  echo "bin path now changed to ..bld.. "
  echo "testbin/${o}b  = $testbin/${o}b"
  cat $testbin/${o}b
  ls -l   $testsrc/debuglink.base $testbin/${o}b 
  echo " now diff "$testsrc/debuglink.base $testbin/${o}b
  ${localsrc}/dos2unix.py $testbin/${o}b
  chkres $? "FAIL debuglinktest-a.sh dos2unix.py"
  diff $testsrc/debuglink.base  $testbin/${o}b
  r=$?
  echo "To update debuglinktest-a.sh baseline:"
  echo "mv $testbin/${o}b $testsrc/debuglink.base"
  chkres $r "running debuglinktest-a.sh test1 diff against baseline"
fi
exit 0
