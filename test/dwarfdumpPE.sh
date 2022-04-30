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
else
  if [ x$DWTOPSRCDIR = "x" ]
  then
    # Building in tree
    t=$top_blddir
  else
    #  Setting find the source base.
    # Building out of tree, using env var
    t=$DWTOPSRCDIR
  fi
fi
# Using the source base.
. $t/test/dwarfdumpsetup.sh $t

f=$top_srcdir/test/testobjLE32PE.exe
b=$top_srcdir/test/testobjLE32PE.base
testbin=$top_blddir/test
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
${localsrc}/dos2unix.py $tx
chkres $? "FAILdwarfdumpPE.sh dos2unix.py"
diff $b $tx > $tx.diff 
echo "report file lengths"
wc -l $b  $tx
r=$?
if [ $r -ne 0 ]
then
  echo "Showing diff $b $tx"
  diff $b $tx
  echo "To update , mv $tx $b"
  exit $r
fi
chkres $r "FAILdwarfdumpPE.sh diff of $b $tx"
rm -f dwarfdump.conf
rm -f $tx
rm -f $tx.diff
exit 0
