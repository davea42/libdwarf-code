#!/bin/sh
# Copyright (C) 2021 David Anderson
# This script is hereby placed in the Public Domain
# for anyone to use in any way for any purpose.
#
# For running out of source tree DWTOPSRCDIR must
# be set on entry.
#
if [ x$DWTOPSRCDIR = "x" ]
then
  t=$top_blddir
else
  t=$DWTOPSRCDIR
fi
. $t/test/dwarfdumpsetup.sh

f=$top_srcdir/test/testobjLE32PE.exe
b=$top_srcdir/test/testobjLE32PE.base
testbin=$top_blddir/test
tx=$testbin/junk.testobjLE32PE.base
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
if [ x$win = "xy" ]
then
  echo "drop two lines"
  droptwoifwin $tx
fi
echo "if update required, mv $tx $b"
fixlasttime $tx
which dos2unix
if [ $? -eq 0 ]
then
  dos2unix $tx
fi
diff $b $tx > $tx.diff 
r=$?
chkres $r "FAILdwarfdumpPE.sh diff of $b $tx"
if [ $r -ne 0 ]
then
  echo "Showing diff $b $tx"
  diff $b $tx
  echo "To update , mv $tx $b"
  exit $r
fi
rm -f $tx
rm -f $tx.diff
exit 0
