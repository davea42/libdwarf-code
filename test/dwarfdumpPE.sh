#!/bin/sh
# Copyright (C) 2021 David Anderson
# This script is hereby placed in the Public Domain
# for anyone to use in any way for any purpose.
#
# configure passes in DWTOPSRCDIR via env var
# cmake passes in DWTOPSRCDIR via argument
# meson passes in DWTOPSRCDIR via argument
# For running out of source tree DWTOPSRCDIR must
# be set on entry.
#
if [ $# -gt 0  ]
then
  t="$1"
  if [ $# -gt 1  ]
  then
    y="$2"
    if [ "$y" = "ninja" ]
    then
       # Needed for meson as test run is not in test/ itself
       minja=y
       top_blddir=`pwd`
       echo "ninja: reset top blddir to $top_blddir"
    fi
    # else ignore  $2
  fi
else
  if [ x$DWTOPSRCDIR = "x" ]
  then
    # Running from the source tree
    t=$top_blddir
  else
    # Running outside of source tree (the usual case)
    t=$DWTOPSRCDIR
  fi
fi
# Using the source base.
. $t/test/dwarfdumpsetup.sh $t

f=$top_srcdir/test/testobjLE32PE.exe
b=$top_srcdir/test/testobjLE32PE.base
testbin=$top_blddir/test
localsrc=$top_srcdir/test
tx=$testbin/junk.testobjLE32PE.base
tx2=$testbin/junk2.testobjLE32PE.base
rm -f $tx
echo "start  dwarfdumpPE.sh sanity check on pe $f"
echo "Run: $dd -vvv -a  $f | head -n $textlim"
$dd -vvv -a $f | head -n $textlim > $tx
r=$?
chkres $r "dwarfdumpPE.sh dwarfdump $f output to $tx xbase $b"
if [ $r -ne 0 ]
then
   echo "$dd FAILED"
   exit $r
fi
echo "if update required, mv $tx $b"
# input and result are $tx, $tx2 is a temp file.
fixlasttime $tx $tx2
# $tx updated if line ends are Windows
${localsrc}/dwdiff.py $b $tx
r=$?
chkres $r "FAIL dwarfdumpPE.sh dwdiff.py"
echo "report file lengths"
wc -l $b  $tx
if [ $r -ne 0 ]
then
  echo "FAIL  dwdiff $b $tx"
  echo "To update , mv $tx $b"
  exit $r
fi
rm -f dwarfdump.conf
rm -f $tx
rm -f $tx.diff
exit 0
