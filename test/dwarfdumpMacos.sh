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
echo "Run: $dd -a -vvv  $f | head -n $textlim"
$dd $f | head -n $textlim > $t
r=$?
chkres $r "FAIL test/dwarfdumpMacos.sh $dd $f to $t base $b "
if [ $r -ne 0 ]
then
  echo "$dd FAILED"
  exit $r
fi

if [ x$win = "xy" ]
then
  echo "drop two lines"
  droptwoifwin $t
fi
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
  echo "Showing diff $b $t"
  diff $b $t
  echo "to update , mv  $top_blddir/dwarfdump/$t $b"
  exit $r 
fi
rm -f $t
rm -f $t.diff
exit 0
