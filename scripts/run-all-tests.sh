#!/bin/sh
start=`date`
echo "start run-all-tests.sh at $start"
here=`pwd`
ddsrc=$here/dwarfdump
rosrc=$here/../readelfobj
rtestsrc=$here/../regressiontests

ddbld=/tmp/vaddbld
robld=/tmp/varobld

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

echo run from $here
if [ ! -d $here/dwarfdump ]
then
   chkres 1 "A FAIL: This is not the libdwarf 'code' directory "
   echo "A FAIL: $here/dwarfdump missing. Run from 'code'"
   
fi
if [ ! -d $here/libdwarf ]
then
  chkres 1 "B FAIL: This is not the libdwarf 'code' directory "
  echo "B FAIL: $here/libdwarf missing"
fi
if [ ! -d $here/dwarfexample ]
then
  chkres 1 "C FAIL: This is not the libdwarf 'code' directory"
  echo "C FAIL: $here/dwarfexample missing"
fi


# ========
builddwarfdump() {
  echo "Build dwarfdump source: $here builddir: $ddbld"
  oac=$ddbld
  rm -rf $oac
  mkdir $oac
  chkres $? "D FAIL: unable to create $oac, giving up."
  cd $oac
  chkres $? "E FAIL: unable to cd to $oac, giving up."
  $here/configure --enable-wall --enable-dwarfgen --enable-dwarfexample
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
  cd $here
  chkres $? "Q FAIL: scripts/buildandreleasetest.sh FAIL"
  sh scripts/buildandreleasetest.sh
  chkres $? "R FAIL: scripts/buildandreleasetest.sh FAIL"
  if [ $failcount -eq 0 ]
  then
      echo "PASS run-all-tests.sh rundistcheck"
  fi
}

#======= readelfobj, readobjpe, readobjmacho etc tests
# with make check
buildreadelfobj() {
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
  $rodir/configure --enable-wall
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
  ddtestdir=$rtestsrc
  echo "Run regressiontests in $ddtestdir"
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
  ./configure
  chkres $? "I FAIL: configure in $ddtestdir failed , giving up."
  make
  chkres $? "J FAIL: tests failed in $ddtestdir , giving up."
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
  fi
}


#========actually run tests
if [ -d $ddsrc ]
then
  builddwarfdump
  rundistcheck
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
  runfullddtest
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
   exit 1
fi
exit 0

