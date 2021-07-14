# scripts/ddbuild.sh
# Copyright (C) 2021 David Anderson
# This test script is in the public domain for use
# by anyone for any purpose.

# Do not run this by hand.
# This is used by scripts/buildstandardsource.sh
# A primitive build of a little bit of stuff
# from libdwarf and dwarfdump to create
# standard C source for libdwarf and dwarfdump.

# Requires a basic config.h at top level

d=`pwd`
db=`basename $d`
topdir=$d/../../..
libdir=$d/../../lib/libdwarf

if [ x$db != "xdwarfdump" ]
then
   echo FAIL Run this in the dwarfdump directory.
   exit 1
fi
if [ ! -d "$libdir" ]
then
   echo FAIL $libdir  not found. Run this in the dwarfdump directory.
   exit 1
fi
set -x
CC="gcc -g -Wall  -I. -I$libdir "
EXEXT=.exe

#cp $libdir/dwarf_names.c .
#cp $libdir/dwarf_names.h .
#$CC -DTRIVIAL_NAMING   dwarf_names.c common.c \
#dwarf_tsearchbal.c \
#$libdir/dwarf_form_class_names.c \
#dwgetopt.c \
#esb.c \
#makename.c \
#naming.c \
#sanitized.c \
#tag_attr.c \
#glflags.c \
#tag_common.c -o tag_attr_build$EXEXT
#if [ $? -ne 0 ]
#then
#   echo tag_attr_build compile fail
#   exit 1
#fi

$CC  -DTRIVIAL_NAMING  dwarf_names.c common.c \
dwarf_tsearchbal.c \
$libdir/dwarf_form_class_names.c \
dwgetopt.c \
esb.c \
makename.c \
naming.c \
glflags.c \
sanitized.c \
tag_common.c \
tag_tree.c -o tag_tree_build$EXEXT
if [ $? -ne 0 ]
then
   echo tag_tree_build compile fail
   exit 1
fi
rm -f tmp-t1.c

#=======
echo BEGIN attr_form build
af=dwarfdump-af-table.h
if [ ! -f $af ]
then
  touch $af
fi
taf=tempaftab
rm -f $taf
$CC  -DTRIVIAL_NAMING -DSKIP_AF_CHECK  dwarf_names.c common.c \
$libdir/dwarf_form_class_names.c \
attr_form.c \
dwarf_tsearchbal.c \
dwgetopt.c \
esb.c \
makename.c \
naming.c \
glflags.c \
sanitized.c \
tag_common.c \
attr_form_build.c -o attr_form_build$EXEXT
if [ $? -ne 0 ]
then
   echo attr_form_build compile fail
   exit 1
fi
rm -f tmp-t1.c

cp attr_formclass.list tmp-t1.c
ls -l tmp-t1.c
$CC -E tmp-t1.c >tmp-attr-formclass-build1.tmp
ls -l tmp-attr-formclass-build1.tmp

cp attr_formclass_ext.list tmp-t2.c
ls -l tmp-t2.c
$CC -E tmp-t2.c >tmp-attr-formclass-build2.tmp
ls -l tmp-attr-formclass-build2.tmp

# Both of the next two add to the same array used by
# dwarfdump itself.
./attr_form_build$EXEXT -s -i tmp-attr-formclass-build1.tmp -o $taf
if [ $? -ne 0 ]
then
   echo attr_formclass_build 1  FAIL
   exit 1
fi
./attr_form_build$EXEXT -e -i tmp-attr-formclass-build2.tmp -o $taf
if [ $? -ne 0 ]
then
   echo attr_formclass_build 2  FAIL
   exit 1
fi
mv $taf $af
rm -f tmp-attr-formclass-build1.tmp 
rm -f tmp-attr-formclass-build2.tmp 
rm -f ./attr_form_build$EXEXT 

cp tag_tree.list tmp-t1.c
$CC -E tmp-t1.c >tmp-tag-tree-build1.tmp
./tag_tree_build$EXEXT -s -i tmp-tag-tree-build1.tmp -o dwarfdump-tt-table.h
if [ $? -ne 0 ]
then
   echo tag_tree_build 1  FAIL
   exit 1
fi
rm -f tmp-tag-tree-build1.tmp 
rm -f tmp-t1.c
rm -f tmp-t2.c

#cp tag_attr.list tmp-t2.c
#$CC -DTRIVIAL_NAMING  -I$libdir -E tmp-t2.c >tmp-tag-attr-build2.tmp
#./tag_attr_build$EXEXT -s -i tmp-tag-attr-build2.tmp -o dwarfdump-ta-table.h
#if [ $? -ne 0 ]
#then
#   echo tag_attr_build 2 FAIL
#   exit 1
#fi
#rm -f tmp-tag-attr-build2.tmp 
#rm -f tmp-t2.c
#rm -f tmp-t3.c

#cp tag_attr_ext.list tmp-t3.c
#$CC  -I$libdir -DTRIVIAL_NAMING -E tmp-t3.c > tmp-tag-attr-build3.tmp
#./tag_attr_build$EXEXT -e -i tmp-tag-attr-build3.tmp -o dwarfdump-ta-ext-table.h
#if [ $? -ne 0 ]
#then
#   echo tag_attr_build 3 FAIL
#   exit 1
#fi
#rm -f tmp-tag-attr-build3.tmp 
#rm -f tmp-t3.c

rm -f tmp-t4.c
cp tag_tree_ext.list tmp-t4.c
$CC  -I$libdir  -DTRIVIAL_NAMING -E tmp-t4.c > tmp-tag-tree-build4.tmp
./tag_tree_build$EXEXT -e -i tmp-tag-tree-build4.tmp -o dwarfdump-tt-ext-table.h
if [ $? -ne 0 ]
then
   echo tag_tree_build 4 compile fail
   exit 1
fi

$CC -I $libdir \
  buildopscounttab.c \
  dwarf_names.c -o buildop
if [ $? -ne 0 ]
then
    echo "FAIL compiling buildop  and building opstabcount.c source"
    exit 1
fi
./buildop >opscounttab.c
if [ $? -ne 0 ]
then
    echo "FAIL building opstabcount.c source"
    exit 1
fi
rm -f ./buildop

rm -f tmp-tag-tree-build4.tmp 
rm -f tmp-t4.c

rm -f tag_attr_build$EXEXT
rm -f tag_tree_build$EXEXT
rm -f attr_form_build$EXEXT

exit 0
