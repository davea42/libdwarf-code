#!/bin/sh
#
if [ x$DWTOPSRCDIR = "x" ]
then
  t=$top_blddir
else
  t=$DWTOPSRCDIR
fi
. $t/test/dwarfdumpsetup.sh

srcloc=$top_srcdir/src/bin/dwarfexample
bldloc=$top_blddir/src/bin/dwarfdump
testbin=$top_blddir/test
testsrc=$top_srcdir/test
# The following stop after 400 lines to limit the size
# of the data here.  
# It is a sanity check, not a full check.
f=$testsrc/testobjLE32PE.exe
b=$testsrc/testobjLE32PE.base
t=$testbin/junk.testobjLE32PE.base
echo "start  dwarfdumpPE.sh sanity check on pe $f"
# Windows dwarfdump emits a couple prefix lines
#we do not want. 
# So let dwarfdump emit more then trim.
# In addition the zero date for file time in line tables
# prints differently for different time zones.
# Delete what follows 'last time 0x0'
if [ x$win = "xy" ]
then
  textlim=702
else
  textlim=700
fi
dd=$bldloc/dwarfdump
echo "$dd -a -vvv  $f | head -n $textlim > $t "
$dd -a  -vvv $f | head -n $textlim > $t
chkres $? "dwarfdumpPE.sh dwarfdump $f output to $t base $b"
if [ x$win = "xy" ]
then
  echo "drop two lines"
  droptwoifwin $t
  echo did drop two
  wc $t
fi
fixlasttime $t
which dos2unix
if [ $? -eq 0 ]
then
  dos2unix $t
fi
diff  $b $t > $t.diffjunk.testsmallpe.diff
r=$?
chkres $r "FAILdwarfdumpPE.sh diff of $b $t"
if [ $r -ne 0 ]
then
  echo "to update , mv $t $b"
fi
rm -f $t
rm -f $t.diffjunk.testsmallpe.diff
exit 0
