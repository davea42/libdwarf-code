#!/bin/sh
#
# Intended to be run only on local machine.
# Run only after config.h created in a configure
# in the source directory
# Assumes env vars DWTOPSRCDIR set to the path to source.
# Assumes CFLAGS warning stuff set in env var DWCOMPILERFLAGS
# Assumes we run the script in the test directory.
# srcdir is from the environment and is, here, the 
# place of the top director itself (may be a relative
# path).

blddir=`pwd`
top_blddir=`dirname $blddir`
if [ x$DWTOPSRCDIR = "x" ]
then
  top_srcdir=$top_blddir
else
  top_srcdir=$DWTOPSRCDIR
fi
if [ $top_srcdir = ".." ]
then
  # This case hopefully eliminates relative path to test dir. 
  top_srcdir=$top_blddir
fi
# srcloc and bldloc are the executable directories.
srcloc=$top_srcdir/src/bin/dwarfexample
bldloc=$top_blddir/src/bin/dwarfexample
#localsrc is the build directory of the test
localsrc=$srcdir
if [ $localsrc = "." ]
then
  localsrc=$top_srcdir/test
fi

testbin=$top_blddir/test
testsrc=$top_srcdir/test
# So we know the build. Because of debuglink.

echo "TOP topsrc  : $top_srcdir"
echo "TOP topbld  : $top_blddir"
echo "TOP localsrc: $localsrc"
chkres() {
r=$1
m=$2
if [ $r -ne 0 ]
then
  echo "FAIL $m.  Exit status for the test $r"
  exit 1
fi
}

echo "debuglinktest-b.sh test2"
o=junk.debuglink2
p=" --no-follow-debuglink --add-debuglink-path=/exam/ple"
p2="--add-debuglink-path=/tmp/phony"
$bldloc/dwdebuglink $p $p2 $testsrc/dummyexecutable > $testbin/$o
r=$?
chkres $r "running dwdebuglink test2"
if [ $r -ne 0 ]
then
  echo "Error debuglinktest-a.sh"
  exit $r
fi
# we strip out the actual localsrc and blddir for the obvious
# reason: We want the baseline data to be meaningful no matter
# where one's source/build directories are.
sed "s:$localsrc:..src..:" <$testbin/$o  >$testbin/${o}a
sed "s:$blddir:..bld..:" <$testbin/${o}a  >$testbin/${o}b
diff $testsrc/debuglink2.base  $testbin/${o}b
r=$?
chkres $r "running debuglinktest-b.sh  diff against baseline"
if [ $r -ne 0 ]
then
   echo "To update debuglinktest-b.sh  baseline: mv $testbin/${o}b $testsrc/debuglink2.base"
   exit $r
fi

exit 0
