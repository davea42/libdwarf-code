#!/bin/sh
#
if [ x$DWTOPSRCDIR = "x" ]
then
  t=$top_blddir
else
  t=$DWTOPSRCDIR
fi

chkres() {
r=$1 
m=$2 
if [ $r -ne 0 ]
then
  echo "FAIL $m.  Exit status for the test $r"
fi 
}
echo "Argument count: $#"
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
diff $b $tx > $tx.diff
r=$?
chkres $r "FAIL jitreader.sh diff of $b $tx"
if [ $r -ne 0 ]
then
  echo "Showing diff $b $tx"
  diff $b $tx
  echo "To update , mv  $tx $b"
  exit $r
fi
rm -f $tx
rm -f $tx.diff
exit 0
