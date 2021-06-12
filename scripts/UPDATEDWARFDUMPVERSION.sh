#!/bin/bash
# Copyright (C) 2021 David Anderson
# This test script is in the public domain for use
# by anyone for any purpose.

# Do not turn on -x here. it will screw things up!
# This puts the current date/time into 
# libdwarf/libdwarf_version.h
# so libdwarf and dwarfdump can report the date time
# applying currently to the source.
# Sometimes only used. just before a release.
l=`pwd`
echo $l

chkisdir() {
  if [ ! -d $1 ]
  then
    echo "The directory $1 is not found"
    echo "we are in the wrong directory to update version strings"
    exit 1
  fi
}

chkisdir scripts
libdir=src/lib/libdwarf
chkisdir $libdir
chkisdir src/bin/dwarfdump


x=`date --rfc-3339=seconds |tr '\n' ' '`
cat > scripts/UPD.awk <<\EOF
BEGIN {
if (ARGC <=  2)  {
    print "Bogus use of awk file, requires arg"
    exit 1   
} else  {
    v=ARGV[1]
    ARGV[1]=""
}
}
$0 ~  /#define DW_VERSION_DATE_STR/ { print $1, $2, "\"",v,"\"" }
$0 !~ /^#define DW_VERSION_DATE_STR/ { print $0 }
EOF
awk -f scripts/UPD.awk  "$x"  "$libdir/libdwarf_version.h" >t
mv t "$libdir/libdwarf_version.h"
rm -f t
rm -f scripts/UPD.awk
