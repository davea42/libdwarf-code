#!/bin/sh
#
if [ x$DWTOPSRCDIR = "x" ]
then
  t=$top_blddir
else
  t=$DWTOPSRCDIR
fi
. $t/test/dwarfdumpsetup.sh

f=$top_srcdir/test/testuriLE64ELf.obj
b=$top_srcdir/test/testuriLE64ELf.base
testbin=$top_blddir/test
tx=$testbin/junk.testuriLE64ELf.base
echo "start  dwarfdumpLinux.sh sanity check on $f"
$dd -vvv -a $f | head -n $textlim > $tx
chkres $? "dwarfdumpLinux.sh running $dd $f output to $tx base $b "
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
chkres $r "FAIL dwarfdumpLinux.sh diff of $b $tx"
if [ $r -ne 0 ]
then
  echo "to update , mv  $tx $b"
fi
rm -f $tx
rm -f $tx.diff
exit 0
