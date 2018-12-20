#!/bin/sh
#  A script verifying the distribution gets all needed files
#  for building, including 'make check'
# First, get the current configure.ac version into v:

if [ -f ./configure.ac ]
then
  f=./configure.ac
else
  if [ -f ../configure.ac ]
  then 
    f=../configure.ac
  else
    echo FAIL Running distribution test from the wrong place.
    exit 
  fi
fi
v=`grep -o '201[0-9][0-9][0-9][0-9][0-9]'< $f | head -n 1`

if [ x$v = "x" ]
then
   echo FAIL did not get configure.ac version
   exit 1
fi
configloc=/home/davea/dwarf/code/configure


rm -rf /tmp/dwbld
rm -rf /tmp/dwinstall
rm -rf /tmp/dwinstallrel
rm -rf /tmp/dwinstallrelbld
rm -rf /tmp/dwinstallrelbldall
rm -f /tmp/dwrelease.tar.gz
rm -rf /tmp/dwreleasebld
rm -rf /tmp/cmakebld
mkdir  /tmp/dwbld
if [ $? -ne 0 ] 
then
   echo FAIL A1 mkdir
   exit 1
fi
mkdir /tmp/dwinstall
if [ $? -ne 0 ] 
then
   echo FAIL A2 mkdir
   exit 1
fi
mkdir /tmp/dwinstallrel
if [ $? -ne 0 ] 
then
   echo FAIL A3 mkdir
   exit 1
fi
mkdir /tmp/dwinstallrelbld
if [ $? -ne 0 ] 
then
   echo FAIL A3b mkdir
   exit 1
fi
mkdir /tmp/dwinstallrelbldall
if [ $? -ne 0 ] 
then
   echo FAIL A3c mkdir /tmp/dwinstallrelbldall
   exit 1
fi
mkdir /tmp/cmakebld
if [ $? -ne 0 ] 
then
   echo FAIL A3d mkdir /tmp/cmakebld
   exit 1
fi

echo "dirs created empty"
echo cd /tmp/dwbld
cd /tmp/dwbld
if [ $? -ne 0 ]
then
  echo FAIL A cd $v
      exit 1
fi
echo "now initial install, prefix /tmp/dwinstall"
$configloc --prefix=/tmp/dwinstall
if [ $? -ne 0 ]
then
  echo FAIL A4a configure fail
      exit 1
fi
echo "initial (dwinstall) make install"
make install
if [ $? -ne 0 ]
then
  echo FAIL A4b make install 
      exit 1
fi
ls -lR /tmp/dwinstall
make dist
ls -1 *tar.gz >/tmp/dwrelset
ct=`wc < /tmp/dwrelset`
echo "count of gz files $ct"
cp *.tar.gz /tmp/dwrelease.tar.gz
cd /tmp
if [ $? -ne 0 ]
then
  echo FAIL B2  cd /tmp
      exit 1
fi
tar -zxf /tmp/dwrelease.tar.gz
ls -d *dw*
echo "now cd libdwarf-$v for second build install"
cd /tmp/dwinstallrelbld
if [ $? -ne 0 ]
then
  echo FAIL C cd /tmp/dwinstallrelbld
      exit 1
fi
echo "now second install install, prefix /tmp/dwinstallrel"
echo "Expecting src in /tmp/libdwarf-$v"
/tmp/libdwarf-$v/configure --prefix=/tmp/dwinstallrel
if [ $? -ne 0 ]
then
  echo FAIL C2  configure fail
      exit 1
fi
echo "In dwinstallrelbld make install from /tmp/libdwarf-$v/configure"
make install
if [ $? -ne 0 ]
then
  echo FAIL C3  final install fail
      exit 1
fi
ls -lR /tmp/dwinstallrel
echo Now lets see if check works
make check
cd /tmp/dwinstallrelbldall
if [ $? -ne 0 ]
then
  echo FAIL Ca cd /dwinstallrelbldall
      exit 1
fi
/tmp/libdwarf-$v/configure --enable-dwarfexample --enable-dwarfgen
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
cd /tmp/cmakebld
if [ $? -ne 0 ]
then
  echo FAIL C10  cd /tmp/cmakebld
      exit 1
fi
cmake /tmp/libdwarf-$v/
if [ $? -ne 0 ]
then
  echo FAIL C10b  cmake in /tmp/cmakebld
      exit 1
fi
make
if [ $? -ne 0 ]
then
  echo FAIL C10c  cmake make in /tmp/cmakebld
      exit 1
fi



