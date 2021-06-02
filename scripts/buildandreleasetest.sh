#!/bin/sh
#  A script verifying the distribution gets all needed files
#  for building, including "make check"
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
# If passes, remove the /tmp/bart working directory.
# Useful to consider if all intended files actually present,
# including any possibly not used.
savebart=n
while [ $# -ne 0 ]
do
  case $1 in
   --savebart ) savebart=y ; shift  ;;
   --disable-libelf ) genopta='' ; genoptb='' 
        libelfopt=$1 ; shift ;;
   --enable-libelf )  shift  ;;
   --disable-dwarfgen ) genopta='' ; genoptb='' ; shift  ;;
   --enable-nonstandardprintf ) nonstdprintf=$1 ; shift  ;;
   * ) echo "Unknown buildandreleasetest.sh option $1. Error." ; exit 1 ;;
  esac
done
echo "savebart flag about temp files:...: $savebart"
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
#v=`grep -o '20[1-2][0-9][0-9][0-9][0-9][0-9]'< $f | head -n 1`
v=`grep -o '[1-9][0-9]*[.][0-9][0-9]*[.][0-9][0-9]*'< $f | head -n 1`
echo "configure.ac version is v=$v"

if [ x$v = "x" ]
then
   echo FAIL did not get configure.ac version
   exit 1
fi

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

configloc=$wd/configure
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
fcmakebld=$bart/f-cmakebld
gcmakebld=$bart/g-cmakebld
hcmakebld=$bart/h-cmakebld
mdirs $bart $abld $ainstall $binstrelp $binstrelbld $crelbld
mdirs $cinstrelp $dbigend $ecmakebld $fcmakebld $gcmakebld
mdirs $hcmakebld
relset=$bart/a-gzfilelist
atfout=$bart/a-tarftout
btfout=$bart/b-tarftout

arelgz=$bart/a-dwrelease.tar.gz
brelgz=$bart/b-dwrelease.tar.gz
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
chkres $? "FAIL Secton A 4b make install"
ls -lR $ainstall
make dist
chkres $? "FAIL make dist Section A" 
# We know there is just one tar.gz in $abld, that we just created
ls -1 ./*tar.gz 
chkres $? "FAIL Section A  ls ./*tar.gz"
safemv ./*.tar.gz $arelgz "FAIL Section A moving gz"
ls -l $arelgz
tar -zxf $arelgz
chkres $? "FAIL B2tar tar -zxf $arelgz"
safemv  libdwarf-$v $blibsrc "FAIL moving libdwarf srcdir"
echo "  End Section A  $bart"
################ End Section A
################ Start Section B
echo "TEST Section B: now cd $binstrelbld for second build install"
safecd $binstrelbld "FAIL C cd"
echo "TEST: now second install install, prefix $binstrelp"
echo "TEST: Expecting src in $blibsrc"
$blibsrc/configure --enable-wall --enable-dwarfgen --enable-dwarfexample --prefix=$binstrelp $libelfopt $nonstdprintf
chkres $? "FAIL configure fail in Section B"
echo "TEST: In $binstrelbld make install from $blibsrc/configure"
make install
chkres $? "FAIL Section B install fail"
ls -lR $binstrelp
echo "TEST: Now lets see if make check works"
make check
chkres $? "FAIL make check in Section B"
make dist
chkres $? "FAIL make dist  Section B"
# We know there is just one tar.gz in $abld, that we just created
ls -1 ./*tar.gz
safemv ./*.tar.gz $brelgz "FAIL Section B moving gz"
ls -l $arelgz
ls -l $brelgz
# gzip does not build diffs quite identically to the byte.
# Lots of diffs, So we do tar tf to get the file name list. 
echo "Now tar -tf on $arelgz and $brelgz "
tar -tf $arelgz > $atfout
tar -tf $brelgz > $btfout
echo "Now diff the respective -tf output file lists"
diff $atfout $btfout
chkres $? "FAIL second gen tar gz file list does not match first gen"
echo "  End Section B  $bart"
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
echo "  End Section C  $bart"
################ End section C

################ Start section D
safecd $crelbld "FAIL section D cd "
echo "TEST: Now configure from source dir $blibsrc/ in build dir $crelbld"
$blibsrc/configure --enable-wall --enable-dwarfexample $genopta
$nonstdprintf
chkres $? "FAIL C9  $blibsrc/configure"
make
chkres $? "FAIL C9  $blibsrc/configure  make"
echo "  End Section D  $bart"
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
  echo "cmake not installed so Test section E not tested."
fi
echo " End Section E  $bart (ls output follows)"
ls  $bart
############ End Section E
################### Cmake test F
safecd $fcmakebld "FAIL C11 Section F cd"
havecmake=n
which cmake >/dev/null
if [ $? -eq 0 ]
then
  havecmake=y
  echo "We have cmake and can test it."
fi
if [ $havecmake = "y" ]
then
  echo "TEST: Now cmake from source dir $blibsrc/ in build dir  $fcmakebld"
  cmake $genoptb -DWALL=ON -DDWARF_WITH_LIBELF=OFF -DBUILD_DWARFEXAMPLE=ON -DDO_TESTING=ON $blibsrc
  chkres $? "FAIL Sec F C11b  cmake in $ecmakdbld"
  make
  chkres $? "FAIL Sec F C11c  cmake make in $fcmakebld"
  make test
  chkres $? "FAIL Sec F C11d  cmake make test in $fcmakebld"
  ctest -R self
  chkres $? "FAIL Sec F C11e  ctest -R self in $fcmakebld"
else
  echo "cmake not installed so -DDWARF_WITH_LIBELF=OFF (sec. F) not tested."
fi
echo " End Section F  $bart (ls output follows)"
ls  $bart
############ End Section F
################### Cmake test G
safecd $gcmakebld "FAIL C11 Section G cd"
havecmake=n
which cmake >/dev/null
if [ $? -eq 0 ]
then
  havecmake=y
  echo "We have cmake and can test it."
fi
if [ $havecmake = "y" ]
then
  echo "TEST: Now cmake from source dir $blibsrc/ in build dir  $gcmakebld"
  cmake $genoptb  -DWALL=ON -DBUILD_NON_SHARED=OFF -DDO_TESTING=ON -DBUILD_SHARED=ON -DBUILD_DWARFGEN=ON -DBUILD_DWARFEXAMPLE=ON $blibsrc
  chkres $? "FAIL Sec F C11b  cmake in $gcmakdbld"
  make
  chkres $? "FAIL Sec F C11d cmake  make in $gcmakebld"
  ctest -R self
  chkres $? "FAIL Sec F C11e  ctest -R self in $gcmakebld"
else
  echo "cmake not installed so Section G not tested."
fi
echo " End Section G  $bart (ls output follows)"
ls  $bart
############ End Section G

################### Cmake test H
safecd $hcmakebld "FAIL C12 Section H cd"
havecmake=n
which cmake >/dev/null
if [ $? -eq 0 ]
then
  havecmake=y
  echo "We have cmake and can test it."
else
  echo "We do NOT have cmake, cannot test it."
fi
if [ $havecmake = "y" ]
then
  echo "TEST: Now cmake from source dir $blibsrc/ in build dir  $gcmakebld"
  cmake -DDWARF_WITH_LIBELF=OFF -DWALL=ON -DBUILD_NON_SHARED=ON -DDO_TESTING=ON -DBUILD_SHARED=OFF -DBUILD_DWARFEXAMPLE=ON $blibsrc
  chkres $? "FAIL Sec H C12b  cmake in $hcmakdbld"
  make
  chkres $? "FAIL Sec H C12d  cmake make test in $hcmakebld"
  ctest -R self
  chkres $? "FAIL Sec H C12e  ctest -R self in $hcmakebld"
else
  echo "cmake not installed so Section H not tested."
fi
echo " End Section H  $bart (ls output follows)"
ls  $bart
############ End Section H


echo "PASS scripts/buildandreleasetest.sh"
if [ "$savebart" = "n" ]
then
  rm -rf $bart
fi
exit 0
