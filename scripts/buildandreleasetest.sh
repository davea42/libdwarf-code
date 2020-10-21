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

chkres() {
  if [ $1 -ne 0 ]
  then
    echo "$2"
    exit 1
  fi
}

mdirs() {
while [ $# -ne 0 ]
do
  f=$1
  rm -rf $f
  mkdir $f
  chkres $? "mkdir $f failed!"
  shift
done
}

safecd() {
  f=$1
  cd $f
  chkres $? "cd $f failed $2" 
}
safemv() {
  s=$1
  t=$2 
  echo "mv $s $t"
  mv $s $t
  chkres $?  "mv $f $t failed  $3"
}

bart=/tmp/bart
abld=$bart/a-dwbld
ainstall=$bart/a-install
binstrelp=$bart/a-installrelp
binstrelbld=$bart/b-installrelbld
blibsrc=$bart/b-libsrc
crelbld=$bart/c-installrelbld
cinstrelp=$bart/c-installrelp
dbigend=$bart/d-bigendian
ecmakebld=$bart/e-cmakebld
mdirs $bart $abld $ainstall $binstrelp $binstrelbld $crelbld
mdirs $cinstrelp $dbigend $ecmakebld 
relset=$bart/a-gzfilelist

arelgz=$bart/a-dwrelease.tar.gz
rm -rf $bart/a-dwrelease
rm -rf $blibsrc
rm -rf $arelgz
echo "dirs created empty"

echo cd $abld
safecd $abld "FAIL A cd failed"
echo "now: $configloc --prefix=$ainstall $libelfopt $nonstdprintf"
$configloc --prefix=$ainstall $libelfopt $nonstdprintf
chkres $? "FAIL A4a configure fail"
echo "TEST Section A: initial $ainstall make install"
make install
chkres $? "FAIL A4b make install"
ls -lR $ainstall
make dist
chkres $? "FAIL make dist " 
# We know there is just one tar.gz in $abld, that we just created
ls -1 ./*tar.gz 
chkres $? "FAIL B2z  ls ./*tar.gz"
safemv ./*.tar.gz $arelgz "FAIL B2  moving gz"
ls -l $arelgz
tar -zxf $arelgz
chkres $? "FAIL B2tar tar -zxf $arelgz"
safemv  libdwarf-$v $blibsrc "FAIL moving libdwarf srcdir"
echo "ls at end Section A  $bart"
ls  $bart
################ End Section A
################ Start Section B
echo "TEST Section B: now cd $binstrelbld for second build install"
safecd $binstrelbld "FAIL C cd"
echo "TEST: now second install install, prefix $binstrelp"
echo "TEST: Expecting src in $blibsrc"
$blibsrc/configure --enable-wall --enable-dwarfgen --enable-dwarfexample --prefix=$binstrelp $libelfopt $nonstdprintf
chkres $? "FAIL C2  configure fail"
echo "TEST: In $binstrelbld make install from $blibsrc/configure"
make install
chkres $? "FAIL C3  final install fail"
ls -lR $binstrelp
echo "TEST: Now lets see if make check works"
make check
chkres $? "FAIL make check C4"
echo "ls at end Section B  $bart"
ls  $bart
################ End section B

################ Start section C
echo "TEST Section C: now cd $dbigend for big-endian build (not runnable) "

safecd $dbigend "FAIL C be1 "
echo "TEST: now second install install, prefix $crelbld"
echo "TEST: Expecting src in $blibsrc"
echo "TEST: $blibsrc/configure $genopta --enable-wall --enable-dwarfexample --prefix=$crelbld $libelfopt $nonstdprintf"
$blibsrc/configure $genopta --enable-wall --enable-dwarfexample --prefix=$cinstrelp $libelfopt $nonstdprintf
chkres $? "FAIL be2  configure fail"
echo "#define WORDS_BIGENDIAN 1" >> config.h
echo "TEST: Compile In $dbigend make from $blibsrc/configure"
make
chkres $? "FAIL be3  Build failed"
echo "ls at end Section C  $bart"
ls  $bart
################ End section C

################ Start section D
safecd $crelbld "FAIL section D cd "
echo "TEST: Now configure from source dir $blibsrc/ in build dir $crelbld"
$blibsrc/configure --enable-wall --enable-dwarfexample $genopta
$nonstdprintf
chkres $? "FAIL C9  $blibsrc/configure"
make
chkres $? "FAIL C9  $blibsrc/configure  make"
echo "ls at end Section D  $bart"
ls  $bart
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
  echo "TEST: Now cmake from source dir $blibsrc/ in build dir  $ecmakebld"
  cmake $genoptb -DWALL=ON -DBUILD_DWARFEXAMPLE=ON -DDO_TESTING=ON $blibsrc
  chkres $? "FAIL C10b  cmake in $ecmakdbld"
  make
  chkres $? "FAIL C10c  cmake make in $ecmakebld"
  make test
  chkres $? "FAIL C10d  cmake make test in $ecmakebld"
  ctest -R self
  chkres $? "FAIL C10e  ctest -R self in $ecmakebld"
else
  echo "cmake is not installed so not tested."
fi
echo "ls at end Section E  $bart"
ls  $bart
############ End Section E
echo "PASS scripts/buildandreleasetest.sh"
rm -rf $bart
exit 0
