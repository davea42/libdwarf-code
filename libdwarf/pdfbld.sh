
# This is meant to be done by hand
# when changes made. Not done during build or install.
# Just use the built pdf to install.
# Run in the libdwarf source directory.


set -x
TROFF=/usr/bin/groff
#TROFFDEV="-T ps"
PSTOPDF=/usr/bin/ps2pdf
pr -t -e libdwarf2.1.mm | tbl | $TROFF -mm >libdwarf2.1.ps
$PSTOPDF libdwarf2.1.ps libdwarf2.1.pdf

pr -t -e  libdwarf2p.1.mm  | tbl | $TROFF -mm >libdwarf2p.1.ps
$PSTOPDF libdwarf2p.1.ps libdwarf2p.1.pdf

rm libdwarf2.1.ps
rm libdwarf2p.1.ps


