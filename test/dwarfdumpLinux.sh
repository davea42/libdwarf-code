#!/bin/sh
#
if [ x$DWTOPSRCDIR = "x" ]
then
  t=$top_blddir
else
  t=$DWTOPSRCDIR
fi
. $t/test/dwarfdumpsetup.sh

f=$testsrc/testuriLE64ELf.obj
b=$testsrc/testuriLE64ELf.base
t=$testbin/junk.testuriLE64ELf.base
echo "start  dwarfdump sanity check on $f"
$dd -vvv -a $f | head -n $textlim > $t
chkres $? "running $dd $f output to $t base $b "
if [ x$win = "xy" ]
then
  echo "drop two lines"
  droptwoifwin $t
fi
echo "if update required, mv $t $b"
fixlasttime $t
which dos2unix
if [ $? -eq 0 ]
then
  dos2unix $t
fi
diff $b $t > $t.diff
r=$?
chkres $r "FAIL diff of $b $t"
if [ $r -ne 0 ]
then
  echo "to update , mv  $t $b"
fi
rm -f $t
rm -f $t.diff
exit 0
