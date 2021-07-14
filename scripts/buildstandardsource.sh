#!/bin/sh
# Copyright (C) 2021 David Anderson
# This test script is in the public domain for use
# by anyone for any purpose.

# Use to build the .c and .h source based on static information
# in libdwarf.h and dwarf_errmsg_list.h and the
# dwarfdump *.list files.
# If you change any of those you should run this script
# (which, for non-linux non-unix may mean some changes of this script
# or of scripts/libbuild.sh or scripts/ddbuild.sh or baseconfig.h)
# This script is idempotent, so if nothing relevant to the script
# changes the output files will not be any different
# than before.

if [ $? -ne 0 ]
then
    echo "FAIL getting base config.h for .c .h builing.Runningfrom wrong place?"
    exit 1
fi
topdir=`pwd`

if [ ! -d src/lib/libdwarf ]
then
    echo "FAIL src/lib/libdwarf. Running buildstandardsource.sh from the wrong place"
    exit 1
fi
if [ ! -d src/bin/dwarfdump ]
then
    echo "FAIL src/bin/dwarfdump. Running buildstandardsource.sh from the wrong place"
    exit 1
fi

#Now build the source files needed by dwarfdump builds.
cd src/bin/dwarfdump
if [ $? -ne 0 ]
then
    echo "FAIL cd to dwarfdump. Running buildstandardsource.sh from the wrong place"
    exit 1
fi
cp $topdir/scripts/baseconfig.h config.h
sh $topdir/scripts/ddbuild.sh
if [ $? -ne 0 ]
then
    echo "FAIL building dwarfdump .c .h source"
    exit 1
fi
rm -f config.h
cd $topdir
if [ $? -ne 0 ]
then
    echo "FAIL second cd back to top-level"
    exit 1
fi
echo "PASS. The .c and .h files are built"
exit 0
