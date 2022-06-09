#!/bin/sh
#
# Intended to be run only on local machine.
# Run only after config.h created in a configure
# in the source directory
#
# Assumes we run the script in the test directory of the build.
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
bname=`basename $blddir`
top_blddir="$blddir"
if [ x$bname = "xtest" ]
then
  top_blddir="$blddir/.."
fi

if [ "x$top_srcdir" = "x.." ]
then
  # This case hopefully eliminates relative path to test dir. 
  top_srcdir=$top_blddir
fi
# bldloc is the executable directories.
bldloc=$top_blddir/src/bin/dwarfexample
#localsrc is the source dir with baseline data
localsrc=$top_srcdir/test
srcdir=$top_srcdir/test
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
o=junk.dlinkb
p=" --no-follow-debuglink --add-debuglink-path=/exam/ple"
p2="--add-debuglink-path=/tmp/phony"
echo "Run: $bldloc/dwdebuglink $p $p2 $testsrc/dummyexecutable "
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
# use @ instead of / or \ or : to avoid tripping over
# normal path characters in Linux/Macos/Windows

sed -e "s@$localsrc@..src..@" <$testbin/$o  >$testbin/${o}a
sed -e "s@$blddir@..bld..@" <$testbin/${o}a  >$testbin/${o}b
${localsrc}/dos2unix.py $testbin/${o}b
chkres $? "FAIL debuglinktest-b.sh dos2unix.py"
diff $testsrc/debuglink2.base  $testbin/${o}b
r=$?
echo "To update debuglinktest-b.sh  baseline:"
echo " mv $testbin/${o}b $testsrc/debuglink2.base"
chkres $r "running debuglinktest-b.sh  diff against baseline"
exit 0
