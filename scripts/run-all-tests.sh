#!/bin/sh
# Copyright (C) 2021 David Anderson
# This test script is in the public domain for use
# by anyone for any purpose.

start=`date`
echo "start run-all-tests.sh at $start"
echo "This removes and recreates /tmp/dw-regression"
# Use --disable-libelf to turn off all reference to
# libelf and to also eliminate reliance on dwarfgen.
# Use --enable-nonstandardprintf to use Windows specific long long
# printf formats.
# Removes and recreates /tmp/dwtestalldd directory
# for the regression tests.

chkisdir() {
  if [ ! -d $1 ]
  then
    echo "The directory $1 is not found"
    echo "we are in the wrong directory to run-all-tests.sh"
    exit 1
  fi
}
here=`pwd`
ddsrc=$here/src/bin/dwarfdump
rosrc=$here/../readelfobj/code
rtestsrc=$here/../regressiontests
# Sanity checks ensuring we are in the right place.
chkisdir scripts
chkisdir $ddsrc
chkisdir $rtestsrc
chkisdir $here/src/lib/libdwarf
chkisdir $here/src/bin/dwarfexample
chkisdir $here/src/bin/dwarfgen
chkisdir $here/test
chkisdir $here/doc

# We run the regression tests here.
ddtestdir=/tmp/dw-regression
rm -rf $ddtestdir
mkdir $ddtestdir

ddbld=/tmp/vaddbld
robld=/tmp/varobld
argval=''
nonstdprintf=

goodcount=0
failcount=0
stsecs=`date '+%s'`
chkres () {
  if [ $1 = 0 ]
  then
    goodcount=`expr $goodcount + 1`
  else
    echo FAIL  $2
    failcount=`expr $failcount + 1`
  fi
}
while [ $# -ne 0 ]
do
  case $1 in
   --enable-libelf ) argval=$1 ; shift ;;
   --disable-libelf ) argval=$1 ; shift ;;
   --enable-nonstandardprintf ) nonstdprintf=$1 ; shift ;;
   * ) echo "Only --enable-libelf or --disable-libelf "
       echo "or --enable-nonstandardprintf allowed."
       echo "No action taken. Exit"
       exit 1 ;;
  esac
done
if [ x$nonstdprintf != "x" ]
then
  echo "Use nonstandardprintf. $nonstdprintf"
fi

echo "Starting run-all-tests.sh now"
echo run from $here
if [ ! -d $here/src/bin/dwarfdump ]
then
   chkres 1 "A FAIL: This is not the libdwarf 'code' directory "
   echo "A FAIL: $here/src/bin/dwarfdump missing. Run from 'code'"
   exit 1
fi
if [ ! -d $here/src/lib/libdwarf ]
then
  chkres 1 "B FAIL: This is not the libdwarf 'code' directory "
  echo "B FAIL: $here/src/lib/libdwarf missing"
fi
if [ ! -d $here/src/bin/dwarfexample ]
then
  chkres 1 "C FAIL: This is not the libdwarf 'code' directory"
  echo "C FAIL: $here/src/bin/dwarfexample missing"
  exit 1
fi


# If we have no libelf we must not attempt to build dwarfgen.
# ========
builddwarfdump() {
  echo "Build dwarfdump source: $here builddir: $ddbld nonstdprintf $nonstdprintf"
  oac=$ddbld
  rm -rf $oac
  mkdir $oac
  chkres $? "D FAIL: unable to create $oac, giving up."
  cd $oac
  chkres $? "E FAIL: unable to cd to $oac, giving up."
  if [ x$1 = "x" ]
  then
    $here/configure --enable-wall $nonstdprintf --enable-dwarfgen --enable-dwarfexample
  else
    $here/configure --enable-wall $1  $nonstdprintf --enable-dwarfexample
  fi
  chkres $? "F FAIL: configure failed in $oac giving up."
  make check
  chkres $? "G FAIL: make check failed in $oac, giving up."
  if [ $failcount -eq 0 ]
  then
      echo "PASS Build dwarfdump"
  fi
}

rundistcheck()
{
  echo "Now rundistcheck"
  cd $here
  chkres $? "Q FAIL: scripts/buildandreleasetest.sh FAIL"
  if  [ x$1 = "--disable-libelf" ]
  then
      sh scripts/buildandreleasetest.sh $1 --disable-dwarfgen $nonstdprintf
  else
      sh scripts/buildandreleasetest.sh $1 $nonstdprintf
  fi
  chkres $? "R FAIL: scripts/buildandreleasetest.sh FAIL"
  if [ $failcount -eq 0 ]
  then
      echo "PASS run-all-tests.sh rundistcheck"
  fi
}

#======= readelfobj, readobjpe, readobjmacho etc tests
# with make check
buildreadelfobj() {
  echo "Now buildreadelfobj"
  rm -rf $robld
  mkdir $robld
  rodir=$rosrc
  echo "Build readelfobj source: $rosrc builddir: $robld"
  if [ ! -d $rodir  ]
  then
    echo "K FAIL: cd  $rodir not a directory, giving up."
    exit 1
  fi
  echo "Now run readelfobj in $robld from readelfobj source $rodir"
  chkres $? "L FAIL: $robld mkdir failed, giving up."
  cd $robld
  chkres $? "M FAIL: cd $robld failed, giving up."
  # Just safety, we do not care if distclean fails
  make distclean
  #
  $rodir/configure --enable-wall $nonstdprintf
  chkres $? "N FAIL: configure $rodir/configure failed, giving up."
  make 
  chkres $? "Oa FAIL: make in $rodir failed, giving up."
  make check
  chkres $? "Ob FAIL: make check in $rodir failed, giving up."
  if [ $failcount -eq 0 ]
  then
      echo "PASS run-all-tests.sh buildreadelfobj"
  fi
}

#=================now run tests, meaning regressiontests

runfullddtest() {
  echo "Run full dd test: run regressiontests in $ddtestdir"
  cd $ddtestdir
  chkres $? "H FAIL: cd $ddtestdir failed , giving up."
  # Ensure no leftovers, ok if it fails
  make distclean
  sha=~/SHALIAS.sh
  if [ -f $sha ]
  then
    # so we get any needed local alias settings.
    cp $sha SHALIAS.sh
  fi
  echo " Now configure regressiontests $ddtestdir/configure $1 $nonstdprintf"
  $rtestsrc/configure $1 $nonstdprintf
  chkres $? "I FAIL: configure in $ddtestdir failed , giving up."
  make
  chkres $? "J FAIL make: tests failed in $ddtestdir. giving up."
  grep FAIL <$ddtestdir/ALLdd
  grep "FAIL 0" $ddtestdir/ALLdd
  chkres $? "Q FAIL: something failed in $ddtestdir."
  tail -40 $ddtestdir/ALLdd
  if [ $failcount -eq 0 ]
  then
      head -n 15 ALLdd
      echo "..."
      head -n 30 ALLdd
      echo "PASS full run-all-tests.sh regressiontests"
  else
      echo "FAIL count $failcount full run-all-tests.sh regressiontests"
  fi
}


#========actually run tests
if [ -d $ddsrc ]
then
  builddwarfdump $argval $nonstdprintf
  rundistcheck  $argval $nonstdprintf
  chkres $? "FAIL rundistcheck" 
else
  echo "dwarfdump make check etc not run"
fi
if [ -d $rosrc ]
then
  buildreadelfobj
  chkres $? "FAIL buildreadelfobj" 
else
  echo "readelfobj make check etc not run"
fi
if [ -d $rtestsrc ]
then
  runfullddtest $argval $nonstdprintf
  chkres $? "FAIL runddtest" 
else
  echo "dwarfdump regressiontests not run"
fi
echo "Done with all tests"
echo "FAIL $failcount in run-all-tests.sh"
if [ $failcount -ne 0 ]
then
   echo "FAIL $failcount as known to run-all-tests.sh"
else
   echo "PASS run-all-tests.sh"
fi
echo "run-all-tests.sh started at $start"
don=`date`
echo "run-all-tests.sh done    at $don"
ndsecs=`date '+%s'`
showminutes() {
   t=`expr  \( $2 \- $1 \+ 29  \) \/ 60`
   echo "Run time in minutes: $t"
}
showminutes $stsecs $ndsecs
if [ $failcount -ne 0 ]
then
   echo "run-all-tests.sh FAIL count $failcount"
   exit 1
else
   echo "run-all-tests.sh all PASS"
fi
exit 0

