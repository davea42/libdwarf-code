#!/bin/sh
#
#Set stuff up.
if [ x$DWTOPSRCDIR = "x" ]
then
  t=$top_blddir
else
  t=$DWTOPSRCDIR
fi
. $t/test/dwarfdumpsetup.sh

f=$srcdir/test-mach-o-32.dSYM
b=$srcdir/test-mach-o-32.base
t=junk.test-mach-o-32.base
echo "start dwarfdumpMacos.sh dwarfdump sanity check on $f"
$dd $f | head -n $textlim > $t
chkres $? "FAIL test/dwarfdumpMacos.sh $dd $f to $t base $b "
if [ x$win = "xy" ]
then
  echo "drop two lines"
  droptwoifwin $t
fi
chkres $? "test/dwarfdumpMacos.sh Running dwarfdump on $f"
echo "if update required, mv $top_blddir/$t $b"
fixlasttime $t
which dos2unix
if [ $? -eq 0 ]
then
  dos2unix $t
fi
diff $b $t > $t.diff
r=$?
chkres $r "FAIL test/dwarfdumpMacos.sh diff of $b $t"
if [ $r -ne 0 ]
then
  echo "to update , mv  $top_blddir/dwarfdump/$t $b"
fi
rm -f $t
rm -f $t.diff
exit 0
