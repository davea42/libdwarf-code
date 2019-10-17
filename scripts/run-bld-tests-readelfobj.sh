#!/bin/sh
start=`date`
echo start at 
date
here=`pwd`
echo run from $here
if [ ! -d dwarfdump ]
then
   echo "A: This is not the libdwarf 'code' directory"
   echo Giving up.
   exit 1
fi
if [ ! -d libdwarf ]
then
  echo "B: This is not the libdwarf 'code' directory "
  echo Giving up.
  exit 1
fi
if [ ! -d dwarfexample ]
then
  echo "C: This is not the libdwarf 'code' directory"
  echo Giving up.
  exit 1
fi

oac=/tmp/overallcheck
rm -rf $oac
mkdir $oac
if [ $? -ne 0  ]
then
  echo "D: unable to create $oac, giving up."
  exit 1
fi
cd $oac
if [ $? -ne 0  ]
then
  echo "E: unable to cd to $oac, giving up."
  exit 1
fi
$here/configure --enable-wall --enable-dwarfgen --enable-dwarfexample
if [ $? -ne 0  ]
then
  echo "F: configure failed in $oac giving up."
  exit 1
fi
make check
if [ $? -ne 0  ]
then
  echo "G: make check failed in $oac, giving up."
  exit 1
fi
echo "PASS doing make check on $here"

#=======now readelfobj
rodir=$here/../readelfobj
if [ $? -ne 0  ]
then
  echo "K: cd  $rodir failed, giving up."
  exit 1
fi
robld=/tmp/robld
rm -rf $robld
mkdir $robld
echo "Now run readelfobj in $robld from readelfobj source $rodir"
if [ $? -ne 0  ]
then
  echo "L: $robld mkdir failed, giving up."
  exit 1
fi
cd $robld
if [ $? -ne 0  ]
then
  echo "M: cd $robld failed, giving up."
  exit 1
fi
$rodir/configure --enable-wall
if [ $? -ne 0  ]
then
  echo "N: configure $rodir/configure failed, giving up."
  exit 1
fi
make check
if [ $? -ne 0  ]
then
  echo "O: make check in $rodir failed, giving up."
  exit 1
fi

#=================now run tests

ddtestdir=$here/../regressiontests
echo "now run regressiontests in $ddtestdir"
cd $ddtestdir
if [ $? -ne 0  ]
then
  echo "H: cd $ddtestdir failed , giving up."
  exit 1
fi
./configure
if [ $? -ne 0  ]
then
  echo "I: configure in $ddtestdir failed , giving up."
  exit 1
fi
make
if [ $? -ne 0  ]
then
  echo "J: tests failed in $ddtestdir , giving up."
  exit 1
fi
echo "Done with all tests"

echo "PASS"
echo "started at $start"
don=`$date`
echo "done at $don"
exit 0

