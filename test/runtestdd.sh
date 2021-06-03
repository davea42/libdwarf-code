#!/bin/sh
#

top_blddir=`pwd`/..
if [ x$DWTOPSRCDIR = "x" ]
then
  top_srcdir=$top_blddir
else
  top_srcdir=$DWTOPSRCDIR
fi
srcdir=$top_srcdir/test
if [ x"$DWCOMPILERFLAGS" = 'x' ]
then
  CFLAGS="-g -O2 -I$top_blddir -I$top_srcdir/src/lib/libdwarf  -I$top_blddir/src/lib/libdwarf -Wall -Wextra"
  echo "CFLAGS basic default default  $CFLAGS"
else
  CFLAGS="-g -O2 -I$top_blddir -I$top_srcdir/src/lib/libdwarf  -I$top_blddir/src/lib/libdwarf $DWCOMPILERFLAGS"
  echo "CFLAGS via configure $CFLAGS"
fi

goodcount=0
failcount=0

echo "TOP topsrc $top_srcdir topbld $top_blddir localsrc $srcdir"
chkres() {
r=$1
m=$2
if [ $r -ne 0 ]
then
  echo "FAIL $m.  Exit status for the test $r"
  failcount=`expr $failcount + 1`
else 
  goodcount=`expr $goodcount + 1`
fi
}

which cc
if [ $? -eq 0 ]
then
  CC=cc
else
  which gcc
  if [ $? -eq 0 ]
  then
    CC=gcc
  else
    # we will fail
    CC=cc
  fi
fi
#echo "cflags before runtests.sh sets it $CFLAGS"
#CFLAGS="-g -O2 -I$top_blddir -I$top_srcdir/libdwarf  -I$top_blddir/libdwarf -Wall -Wextra -Wpointer-arith -Wmissing-declarations -Wcomment -Wformat -Wpedantic -Wuninitialized -Wshadow -Wno-long-long -Werror"


# Remove the leading two lines for windows
# as windows dwarfdump emits two leading lines
# as compared to non-windows dwarfdump
droptwoifwin() {
i=$1
l=`wc -l < $i`
if [ $l -gt 2 ]
then
  l=`expr $l - 2`
  tail -$l <$i >junk.tmp
  cp junk.tmp $i
  rm -f junk.tmp
fi
}
fixlasttime() {
  i=$1
  sed 's/last time 0x.*/last time 0x0/' <$i >junk.tmp
  cp junk.tmp $i
  rm -f junk.tmp
}

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
echo "start  dwarfdump sanity check on pe $f"
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
chkres $? "Running dwarfdump $f output to $t base $b"
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
chkres $r "FAIL diff of $b $t"
if [ $r -ne 0 ]
then
  echo "to update , mv $t $b"
fi
rm -f $t
rm -f $t.diffjunk.testsmallpe.diff

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

f=$srcdir/test-mach-o-32.dSYM
b=$srcdir/test-mach-o-32.base
t=junk.test-mach-o-32.base
echo "start  dwarfdump sanity check on $f"
$dd $f | head -n $textlim > $t
chkres $? "FAIL dwarfdump/runtests.sh $dd $f to $t base $b "
if [ x$win = "xy" ]
then
  echo "drop two lines"
  droptwoifwin $t
fi
chkres $? "Running dwarfdump on $f"
echo "if update required, mv $top_blddir/$t $b"
fixlasttime $t
which dos2unix
if [ $? -eq 0 ]
then
  dos2unix $t
fi
diff $b $t > $t.diff
r=$?
chkres $r "FAIL dwarfdump/runtests.sh diff of $b $t"
if [ $r -ne 0 ]
then
  echo "to update , mv  $top_blddir/dwarfdump/$t $b"
fi
if [ $failcount -ne 0 ]
then
   echo "FAIL $failcount dwarfdump/runtests.sh"
   exit 1
fi
rm -f $t
rm -f $t.diff
exit 0
