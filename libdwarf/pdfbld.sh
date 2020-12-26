#!/bin/sh
# This is meant to be done by hand
# when changes made. Not done during build or install.
# Just use the built pdf to install.
# Run in the libdwarf source directory.

c="n"
p="n"
if [ $# -lt 1 ]
then
  echo "Usage: pdfbld.sh [-a] [-c] [-p]"
  echo "where: -c formats libdwarf2.1.pdf"
  echo "where: -p formats libdwarf2p.1.pdf"
  echo "where: -a formats both"
  exit 1
fi
for i in $*
do
  case $i in
    -a) c="y" ; p="y"
       shift ;;
    -c) c="y"
       shift ;;
    -p) p="y"
       shift ;;
    *)  echo "Giving up: unknown argument use argument -a or -c or -p"
       exit 1 ;;
  esac
done

set -x
TROFF=/usr/bin/groff
#TROFFDEV="-T ps"
PSTOPDF=/usr/bin/ps2pdf
if [ $c = "y" ]
then
  rm -f libdwarf2.1.pdf
  t=junklibdwarfread.pdf
  pr -t -e libdwarf2.1.mm | tbl | $TROFF -n16 -mm >libdwarf2.1.ps
  $PSTOPDF libdwarf2.1.ps $t
  echo "Now create libdwarf2.1.mm by tranforming $t"
  set +x
  sh ../scripts/rebuildpdf.sh $t libdwarf2.1.pdf
  #rm $t
  set -x
fi

if [ $p = "y" ]
then
  rm -f libdwarf2p.1.pdf
  pr -t -e  libdwarf2p.1.mm  | tbl | $TROFF -mm >libdwarf2p.1.ps
  $PSTOPDF libdwarf2p.1.ps libdwarf2p.1.pdf
fi
set +x
echo "Check libdwarf/libdwarf2.1.pdf is correct "
echo "Should start with abstract page then follow with"
echo "libdwarf2.1.pdf table of contents."

rm -f libdwarf2.1.ps
rm -f libdwarf2p.1.ps
