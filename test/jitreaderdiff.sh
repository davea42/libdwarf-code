#!/bin/sh
#
# Makefile.am   set env var DWTOPSRCDIR, no args
# CMakeLIsts.txt pass in 2 args  1.top source dir 
#    no env vars.

chkres() {
r=$1 
m=$2 
if [ $r -ne 0 ]
then
  echo "FAIL $m.  Exit status for the test $r"
fi 
}

echo "Argument count: $#"
blddir=`pwd`
bname=`basename $blddir`
top_blddir="$blddir"
if [ x$bname = "xtest" ]
then
  top_blddir="$blddir/.."
fi
if [ $# -gt 0  ]
then
  DWTOPSRCDIR="$1"
fi

if [ x$DWTOPSRCDIR = "x" ]
then
  # Assume runing tests in source
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
# bldloc is the executable directories.
bldloc=$top_blddir/src/bin/dwarfexample
#localsrc is the source dir with baseline data
localsrc=$top_srcdir/test
srcdir=$top_srcdir/test
testbin=$top_blddir/test
testsrc=$top_srcdir/test
echo "TOP topsrc  : $top_srcdir"
echo "TOP topbld  : $top_blddir"
echo "TOP localsrc: $localsrc"

b=$top_srcdir/test/jitreader.base
localsrc=$top_srcdir/test
testbin=$top_blddir/test
tx=$testbin/junk.jitreader.new
jr=$top_blddir/src/bin/dwarfexample/jitreader

$jr > $tx
r=$?
if [ $r -ne 0 ]
then
   echo "failing jitreader run."
   cat $tx
fi
chkres $r "$jr printing output to $tx base $b "
if [ $r -ne 0 ]
then
  echo " failed"
  exit $r
fi
echo "if update required, mv $tx $b"
${localsrc}/dwdiff.py $b $tx
if [ $r -ne 0 ]
then
  echo "Diff above."
  echo "To update , mv  $tx $b"
  exit $r
fi
rm -f $tx
rm -f $tx.diff
exit 0
