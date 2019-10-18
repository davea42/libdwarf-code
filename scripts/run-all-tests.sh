#!/bin/sh
start=`date`
echo start at 
date
here=`pwd`

ddsrc=$here/dwarfdump
rosrc=$here/../readelfobj
rtestsrc=$here/../regressiontests

ddbld=/tmp/vaddbld
robld=/tmp/varobld

echo run from $here
if [ ! -d $here/dwarfdump ]
then
   echo "A FAIL: This is not the libdwarf 'code' directory "
   echo "A FAIL: $here/dwarfdump missing. Run from 'code'"
   echo FAIL Giving up.
   exit 1
fi
if [ ! -d $here/libdwarf ]
then
  echo "B FAIL: This is not the libdwarf 'code' directory "
  echo "B FAIL: $here/libdwarf missing"
  echo FAIL Giving up.
  exit 1
fi
if [ ! -d $here/dwarfexample ]
then
  echo "C FAIL: This is not the libdwarf 'code' directory"
  echo "C FAIL: $here/dwarfexample missing"
  echo FAIL Giving up.
  exit 1
fi

# ========
builddwarfdump() {
  oac=$ddbld
  rm -rf $oac
  mkdir $oac
  if [ $? -ne 0  ]
  then
    echo "D FAIL: unable to create $oac, giving up."
    exit 1
  fi
  cd $oac
  if [ $? -ne 0  ]
  then
    echo "E FAIL: unable to cd to $oac, giving up."
    exit 1
  fi
  $here/configure --enable-wall --enable-dwarfgen --enable-dwarfexample
  if [ $? -ne 0  ]
  then
    echo "F FAIL: configure failed in $oac giving up."
    exit 1
  fi
  make check
  if [ $? -ne 0  ]
  then
    echo "G FAIL: make check failed in $oac, giving up."
    exit 1
  fi
  echo "PASS doing make check on $here, build in $oac"
}

rundistcheck()
{
  cd $here
  if [ $? -ne 0 ]
  then
     echo "Q FAIL: scripts/buildandreleasetest.sh FAIL"
  fi
  sh scripts/buildandreleasetest.sh
  if [ $? -ne 0 ]
  then
     echo "R FAIL: scripts/buildandreleasetest.sh FAIL"
  fi
  echo PASS scripts/buildandreleasetest.sh
}

#======= readelfobj, readobjpe, readobjmacho etc tests
# with make check
buildreadelfobj() {
  rodir=$rosrc
  if [ $? -ne 0  ]
  then
    echo "K FAIL: cd  $rodir failed, giving up."
    exit 1
  fi
  rm -rf $robld
  mkdir $robld
  echo "Now run readelfobj in $robld from readelfobj source $rodir"
  if [ $? -ne 0  ]
  then
    echo "L FAIL: $robld mkdir failed, giving up."
    exit 1
  fi
  cd $robld
  if [ $? -ne 0  ]
  then
    echo "M FAIL: cd $robld failed, giving up."
    exit 1
  fi
  $rodir/configure --enable-wall
  if [ $? -ne 0  ]
  then
    echo "N FAIL: configure $rodir/configure failed, giving up."
    exit 1
  fi
  make 
  if [ $? -ne 0  ]
  then
    echo "Oa FAIL: make in $rodir failed, giving up."
    exit 1
  fi
  make check
  if [ $? -ne 0  ]
  then
    echo "Ob FAIL: make check in $rodir failed, giving up."
    exit 1
  fi
}

#=================now run tests, meaning regressiontests

runfullddtest() {
  ddtestdir=$rtestsrc
  echo "now run regressiontests in $ddtestdir"
  cd $ddtestdir
  if [ $? -ne 0  ]
  then
    echo "H FAIL: cd $ddtestdir failed , giving up."
    exit 1
  fi
  # Ensure no leftovers, ok if it fails
  make distclean
  sha=~/SHALIAS.sh
  if [ -f $sha ]
  then
    # so we get any needed local alias settings.
    cp $sha SHALIAS.sh
  fi
  ./configure
  if [ $? -ne 0  ]
  then
    echo "I FAIL: configure in $ddtestdir failed , giving up."
    exit 1
  fi
  make
  if [ $? -ne 0  ]
  then
    echo "J FAIL: tests failed in $ddtestdir , giving up."
    exit 1
  fi
  grep FAIL <$ddtestdir/ALLdd
  grep "FAIL 0" $ddtestdir/ALLdd
  if [ $? -ne 0 ]
  then
    echo "Q FAIL: something failed in $ddtestdir."
    exit 1
  fi
  tail -40 $ddtestdir/ALLdd
  echo "PASS full regressiontests"
}


#========actually run tests
if [ -d $ddsrc ]
then
  builddwarfdump
  rundistcheck
else
  echo "dwarfdump make check etc not run"
fi
if [ -d $rosrc ]
then
  buildreadelfobj
else
  echo "readelfobj make check etc not run"
fi
if [ -d $rtestsrc ]
then
  runfullddtest
else
  echo "dwarfdump regressiontests not run"
fi
echo "Done with all tests"
echo "PASS"
echo "started at $start"
don=`$date`
echo "done at $don"
exit 0

