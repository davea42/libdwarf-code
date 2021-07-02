#!/bin/sh
#scripts/libbuild.sh
# Copyright (C) 2021 David Anderson
# This test script is in the public domain for use
# by anyone for any purpose.

# Do not run this by hand.
# This is used by scripts/buildstandardsource.sh
# Builds various names .c and .h needed for libdwarf.
# For DW_DLE checks 'make check' tests that.
# Expects a trivial config.h (see scripts/baseconfig.h)

d=`pwd`
db=`basename $d`
topdir=$d/../../..

if [ x$db != "xlibdwarf" ]
then
   echo FAIL Run this in the libdwarf directory.
   exit 1
fi

set -x
CC="gcc -g  -I.."
$CC gennames.c dwgetopt.c -o gennames
rm -f dwarf_names.h dwarf_names.c  
rm -f dwarf_names_enum.h dwarf_names_new.h
./gennames -i . -o .
if [ $? -ne 0 ]
then
   echo gennames fail
   exit 1
fi
rm -f gennames 
exit 0
