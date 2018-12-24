#
# Use to build the .c and .h source based on static information
# in libdwarf.h.in and the dwarfdump *.list files.
# Perhaps cp scripts/baseconfig.h config.h
# This script is by David Anderson and hereby put into the public domain
# for anyone to use in any way.
#
cd libdwarf
if [ $? -ne 0 ]
then
    echo "FAIL cd to libdwarf. Running buildstandardsource.sh from the wrong place"
    exit 1
fi
sh ../scripts/libbuild.sh
if [ $? -ne 0 ]
then
    echo "FAIL libbuild.sh. "
    exit 1
fi
cd ..
if [ $? -ne 0 ]
then
    echo "FAIL cd back to top-level"
    exit 1
fi
cd dwarfdump
if [ $? -ne 0 ]
then
    echo "FAIL cd to dwarfdump. Running buildstandardsource.sh from the wrong place"
    exit 1
fi
sh ../scripts/ddbuild.sh
if [ $? -ne 0 ]
then
    echo "FAIL building dwarfdump .c .h source"
    exit 1
fi
echo "PASS. The .c and .h files are built"
exit 0
