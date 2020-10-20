#!/bin/sh
#  A script verifying the distribution gets all needed files
#  for building, including 'make check'
# First, get the current configure.ac version into v:

# if stdint.h does not define uintptr_t and intptr_t
# Then dwarfgen (being c++) will not build
# Use --disable-libelf to disable reliance on libelf
# and dwarfgen.
# To just eliminate dwarfgen build/test/install use --disable-dwarfgen.
genopta="--enable-dwarfgen"
genoptb="-DBUILD_DWARFGEN=ON"
libelfopt=''
wd=`pwd`
nonstdprintf=
while [ $# -ne 0 ]
do
  case $1 in
   --disable-libelf ) genopta='' ; genoptb='' 
        libelfopt=$1 ; shift ;;
   --enable-libelf )  shift  ;;
   --disable-dwarfgen ) genopta='' ; genoptb='' ; shift  ;;
   --enable-nonstandardprintf ) nonstdprintf=$1 ; shift  ;;
   * ) echo "Unknown buildandreleasetest.sh option $1. Error." ; exit 1 ;;
  esac
done

if [ -f ./configure.ac ]
then
  f=./configure.ac
else
  if [ -f ../configure.ac ]
  then 
    f=../configure.ac
  else
    echo "FAIL Running distribution test from the wrong place."
    exit 
  fi
fi
v=`grep -o '20[1-2][0-9][0-9][0-9][0-9][0-9]'< $f | head -n 1`

if [ x$v = "x" ]
then
   echo FAIL did not get configure.ac version
   exit 1
fi
configloc=$wd/configure

mdirs() {
while [ $# -ne 0 ]
do
  f=$1
  rm -rf $f
  mkdir $f
  if [ $? -ne 0 ]
  then
    echo "mkdir $f failed!"
    exit 1
  fi
  shift
done
}

safecd() {
  f=$1
  cd $f
  if [ $? -ne 0 ]
  then
    echo "cd $f failed " $2
    exit 1
  fi
}

abld=/tmp/a-dwbld
ainstall=/tmp/a-install
binstrel=/tmp/a-installrel
binstrelbld=/tmp/b-installrelbld
crelbld=/tmp/c-installrelbld
dbigend=/tmp/d-bigendian
ecmakebld=/tmp/e-cmakebld
mdirs $abld $ainstall $binstrel $binstrelbld $crelbld
mdirs $dbigend $ecmakebld

arelgz=/tmp/a-dwrelease.tar.gz
echo "dirs created empty"

echo cd $abld
safecd $abld "FAIL A cd failed"
echo "now: $configloc --prefix=$ainstall $libelfopt $nonstdprintf"
$configloc --prefix=$ainstall $libelfopt $nonstdprintf
if [ $? -ne 0 ]
then
  echo FAIL A4a configure fail
  exit 1
fi
echo "TEST Section A: initial $ainstall make install"
make install
if [ $? -ne 0 ]
then
  echo FAIL A4b make install 
  exit 1
fi
ls -lR $ainstall
make dist
if [ $? -ne 0 ]
then
  echo FAIL make dist  
  exit 1
fi
ls -1 *tar.gz >/tmp/dwrelset
ct=`wc < /tmp/dwrelset`
echo "count of gz files $ct"
cp *.tar.gz $arelgz
cd /tmp
if [ $? -ne 0 ]
then
  echo FAIL B2  cd /tmp
  exit 1
fi
tar -zxf $arelgz
ls -d *dw*
################ End Section A
################ Start Section B
echo "TEST Section B: now cd libdwarf-$v for second build install"
safecd $binstrelbld "FAIL C cd"
echo "TEST: now second install install, prefix $binstrel"
echo "TEST: Expecting src in /tmp/libdwarf-$v"
/tmp/libdwarf-$v/configure --enable-wall --enable-dwarfgen --enable-dwarfexample --prefix=$binstrel $libelfopt $nonstdprintf
if [ $? -ne 0 ]
then
  echo FAIL C2  configure fail
  exit 1
fi
echo "TEST: In $binstrelbld make install from /tmp/libdwarf-$v/configure"
make install
if [ $? -ne 0 ]
then
  echo FAIL C3  final install fail
  exit 1
fi
ls -lR $binstrel
echo "TEST: Now lets see if make check works"
make check
if [ $? -ne 0 ]
then
  echo FAIL make check C4 
  exit 1
fi
################ End section B

################ Start section C
echo "TEST Section C: now cd libdwarf-$v for big-endian build (not runnable) "

safecd $dbigend "FAIL C be1 "
echo "TEST: now second install install, prefix /tmp/dwinstallrel"
echo "TEST: Expecting src in /tmp/libdwarf-$v"
echo "TEST: /tmp/libdwarf-$v/configure $genopta --enable-wall --enable-dwarfexample --prefix=/tmp/dwinstallrel $libelfopt $nonstdprintf"
/tmp/libdwarf-$v/configure $genopta --enable-wall --enable-dwarfexample --prefix=/tmp/dwinstallrel $libelfopt $nonstdprintf
if [ $? -ne 0 ]
then
  echo FAIL be2  configure fail
  exit 1
fi
echo "#define WORDS_BIGENDIAN 1" >> config.h
echo "TEST: Compile In $dbigend make from /tmp/libdwarf-$v/configure"
make
if [ $? -ne 0 ]
then
  echo FAIL be3  Build failed
  exit 1
fi
################ End section C

################ Start section D
safecd $crelbld "FAIL section D cd "
echo "TEST: Now configure from source dir /tmp/libdwarf-$v/ in build dir $crelbld"
/tmp/libdwarf-$v/configure --enable-wall --enable-dwarfexample $genopta
$nonstdprintf
if [ $? -ne 0 ]
then
  echo FAIL C9  /tmp/libdwarf-$v/configure 
  exit 1
fi
make
if [ $? -ne 0 ]
then
  echo FAIL C9  /tmp/libdwarf-$v/configure  make
  exit 1
fi

################### End Section D
################### Cmake test E
safecd $ecmakebld "FAIL C10 Section E cd"
havecmake=n
which cmake >/dev/null
if [ $? -eq 0 ]
then
  havecmake=y
  echo "We have cmake and can test it."
fi
if [ $havecmake = "y" ]
then
  echo "TEST: Now cmake from source dir /tmp/libdwarf-$v/ in build dir  $ecmakebld"
  cmake $genoptb -DWALL=ON -DBUILD_DWARFEXAMPLE=ON -DDO_TESTING=ON /tmp/libdwarf-$v/
  if [ $? -ne 0 ]
  then
    echo "FAIL C10b  cmake in $ecmakdbld"
    exit 1
  fi
  make
  if [ $? -ne 0 ]
  then
    echo "FAIL C10c  cmake make in $ecmakebld"
    exit 1
  fi
  make test
  if [ $? -ne 0 ]
  then
    echo "FAIL C10d  cmake make test in $ecmakebld"
    exit 1
  fi
  ctest -R self
  if [ $? -ne 0 ]
  then
    echo "FAIL C10e  ctest -R self in $ecmakebld"
    exit 1
  fi
else
  echo "cmake is not installed so not tested."
fi
############ End Section E
exit 0
