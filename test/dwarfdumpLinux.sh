#!/bin/sh
# Copyright (C) 2021 David Anderson
# This script is hereby placed in the Public Domain
# for anyone to use in any way for any purpose.
#
# To call this:
# Either set arg1 to the top source dir
# or set env var DWTOPSRCDIR to the top source dir.
echo $top_srcdir $top_blddir $DWTOPSRCDIR

if [ $# -gt 0  ]
then
  t="$1"
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
. $t/test/dwarfdumpsetup.sh $t

f=$top_srcdir/test/testuriLE64ELf.obj
b=$top_srcdir/test/testuriLE64ELf.base
testbin=$top_blddir/test
tx=$testbin/junk.testuriLE64ELf.base
tx2=$testbin/junk2.testuriLE64ELf.base
rm -f $tx
echo "start  dwarfdumpLinux.sh sanity check on $f"
echo "Run: $dd -a -vvv  $f | head -n $textlim"
$dd -vvv -a $f | head -n $textlim > $tx
r=$?
chkres $r "dwarfdumpLinux.sh running $dd $f output to $tx base $b "
if [ $r -ne 0 ]
then
  echo "$dd failed"
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
chkres $r "FAIL dwarfdumpLinux.sh diff of $b $tx"
if [ $r -ne 0 ]
then
  echo "Showing diff $b $tx"
  diff $b $tx
  echo "To update , mv  $tx $b"
  exit $r
fi
rm -f dwarfdump.conf
rm -f $tx
rm -f $tx.diff
exit 0
