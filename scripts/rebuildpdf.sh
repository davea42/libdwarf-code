#!/bin/bash
# This assume you run it from the libdwarf directory
# and the libdwarf2.1.pdf it will alter is in the code/libdwarf
# directory.
#
# The purpose of this script is to rearange what groff
# prepares so the table of contents comes before the content
# hoping this will make the document easier to read.
# See also libdwarf/pdfbld.sh and scripts/find_pdfpages.py

if [ $# -ne 2 ]
then
  echo "Expected rebuildpdf.sh inputpdf outputpdf"
  echo "Got arg count $#"
  exit 1
fi
f=$1
outfile=$2
rm -rf /tmp/dwtoc
mkdir /tmp/dwtoc
tdir=/tmp/dwtoc
abstractpage=0
tocpage=0
endmain=`expr $tocpage - 1 `
endtoc=`expr $abstractpage - 1 `

if [ ! -f $f  ]
then
  echo "We are in the wrong directory. We expected "
  echo "$f as a file in libdwarf."
  exit 1
fi

pdftotext $f $tdir/dw.txt
if [ $? -ne 0 ]
then
  echo "Running pdftotext fails"
  exit 1
fi
python3 ../scripts/find_pdfpages.py $tdir/dw.txt >$tdir/pgnums
if [ $? -ne 0 ]
then
  echo "Running scripts/find_pdfpages.py fails"
  exit 1
fi
rm -f $outfile
abstractpage=`head -1 $tdir/pgnums`
tocpage=`tail -n 1 $tdir/pgnums`
echo " tocpage $tocpage abstractpage $abstractpage"
endmain=`expr $tocpage - 1 `
endtoc=`expr $abstractpage - 1 `
echo "  endmain $endmain endtoc $endtoc"
# pdfseparate separates each page into its own file.
pdfseparate -f 1 -l $endmain        $f $tdir/dwmain-%04d.pdf 
pdfseparate -f $tocpage -l $endtoc  $f $tdir/dwtoc-%02d.pdf 
pdfseparate -f $abstractpage        $f $tdir/dwabs-%d.pdf 
# Now reassemble the document.
pdfunite $tdir/dwabs-*.pdf $tdir/dwtoc-*pdf $tdir/dwmain-*pdf $outfile
#rm -rf $tdir
