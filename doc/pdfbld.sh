#!/bin/sh
# This is meant to be done by hand
# when changes made. Not done during build or install.
#   see doc/Makefile.am
# mips_extensions.mm is a historical document
# and should never change, so we do not build or
# install mips_extensions.pdf

c="n"
p="n"
if [ $# -lt 1 ]
then
  echo "Usage: pdfbld.sh [-c] [-p]"
  echo "where: -c formats libdwarf.pdf"
  echo "where: -p formats libdwarfp.pdf"
  exit 1
fi
for i in $*
do
  case $i in
    -c) c="y"
       echo "Build libdwarf consumer pdf for $src"
       fin=libdwarf.mm
       fout=libdwarf.pdf
       shift ;;
    -p) p="y"
       fin=libdwarfp.mm
       fout=libdwarfp.pdf
       echo "Build libdwarf consumer pdf for $src"
       shift ;;
    *)  echo "Giving up: unknown argument use argument -a or -c or -p"
       exit 1 ;;
  esac
done
src=$1
echo "Build pdf for $src"

set -x
TROFF=/usr/bin/groff
#TROFFDEV="-T ps"
PSTOPDF=/usr/bin/ps2pdf
rm -f $fout

pr -t -e $fin | tbl | $TROFF -n16 -mm >temp.ps
if [  $? -ne 0 ]
then
  echo "Building $fout FAILED in the pr/tbl/$TROFF step"
  exit 1
fi
$PSTOPDF temp.ps $fout
if [  $? -ne 0 ]
then
  echo "Building $fout FAILED in the $PSTOPDF step"
  exit 1
fi

set +x
rm -f temp.ps
exit 0
