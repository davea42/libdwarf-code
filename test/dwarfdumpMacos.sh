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

f=$top_srcdir/test/test-mach-o-32.dSYM
b=$top_srcdir/test/test-mach-o-32.base
testbin=$top_blddir/test
tx=$testbin/junk.test-mach-o-32.base
tx2=$testbin/junk2.test-mach-o-32.base
rm -f $tx
echo "start dwarfdumpMacos.sh dwarfdump sanity check on $f"
echo "Run: $dd -a -vvv  $f | head -n $textlim"
$dd -a -vvv $f | head -n $textlim > $tx
r=$?
chkres $r "FAIL test/dwarfdumpMacos.sh $dd $f to $tx base $b "
if [ $r -ne 0 ]
then
  echo "$dd FAILED"
  exit $r
fi
echo "if update required, mv $tx $b"
fixlasttime $tx $tx2
which dos2unix
if [ $? -eq 0 ]
then
  dos2unix $tx
fi
diff $b $tx > $tx.diff
r=$?
chkres $r "FAIL test/dwarfdumpMacos.sh diff of $b $tx"
if [ $r -ne 0 ]
then
  echo "Showing diff $b $tx"
  diff $b $tx
  echo "to update , mv $tx $b"
  exit $r 
fi
rm -f dwarfdump.conf
rm -f $tx
rm -f $tx.diff
exit 0
