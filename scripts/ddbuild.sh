# ddbuild.sh
# A primitive build.
# Intended for simple non-elf builds on systems
# with no libelf, no elf.h, no libz.
# This script is by David Anderson and hereby 
# put into the public domain
# for anyone to use in any way.

# Requires a basic config.h at top level

d=`pwd`
db=`basename $d`

if [ x$db != "xdwarfdump" ]
then
   echo FAIL Run this in the dwarfdump directory.
   exit 1
fi
# Following two lines needed for independent tests of
# this script. Done by buildstandardsource.sh normally.
# If added *must* be removed else dwarfdump builds will fail.
#cp ../scripts/baseconfig.h config.h
#cp ../libdwarf/libdwarf.h.in libdwarf.h

set -x
top_builddir=..
top_srcdir=..
CC="gcc -g -Wall  -I.. -I../libdwarf -I../dwarfdump"
EXEXT=.exe

cp $top_builddir/libdwarf/dwarf_names.c .
cp $top_builddir/libdwarf/dwarf_names.h .
$CC -DTRIVIAL_NAMING   dwarf_names.c common.c \
dwarf_tsearchbal.c \
$top_srcdir/libdwarf/dwarf_form_class_names.c \
dwgetopt.c \
esb.c \
makename.c \
naming.c \
sanitized.c \
tag_attr.c \
glflags.c \
tag_common.c -o tag_attr_build$EXEXT
if [ $? -ne 0 ]
then
   echo tag_attr_build compile fail
   exit 1
fi

$CC  -DTRIVIAL_NAMING  dwarf_names.c common.c \
dwarf_tsearchbal.c \
$top_srcdir/libdwarf/dwarf_form_class_names.c \
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
taf=tempaftab
rm $taf
af=dwarfdump-af-table.h
if [ ! -f $af ]
then
  touch $af
fi
$CC  -DTRIVIAL_NAMING  dwarf_names.c common.c \
$top_srcdir/libdwarf/dwarf_form_class_names.c \
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

cp $top_srcdir/dwarfdump/attr_formclass.list tmp-t1.c
ls -l tmp-t1.c
$CC -E tmp-t1.c >tmp-attr-formclass-build1.tmp
ls -l tmp-attr-formclass-build1.tmp

cp $top_srcdir/dwarfdump/attr_formclass_ext.list tmp-t2.c
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

cp $top_srcdir/dwarfdump/tag_tree.list tmp-t1.c
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
cp $top_srcdir/dwarfdump/tag_attr.list tmp-t2.c
$CC -DTRIVIAL_NAMING  -I$top_srcdir/libdwarf -E tmp-t2.c >tmp-tag-attr-build2.tmp
./tag_attr_build$EXEXT -s -i tmp-tag-attr-build2.tmp -o dwarfdump-ta-table.h
if [ $? -ne 0 ]
then
   echo tag_attr_build 2 FAIL
   exit 1
fi
rm -f tmp-tag-attr-build2.tmp 
rm -f tmp-t2.c

rm -f tmp-t3.c
cp $top_srcdir/dwarfdump/tag_attr_ext.list tmp-t3.c
$CC  -I$top_srcdir/libdwarf -DTRIVIAL_NAMING -E tmp-t3.c > tmp-tag-attr-build3.tmp
./tag_attr_build$EXEXT -e -i tmp-tag-attr-build3.tmp -o dwarfdump-ta-ext-table.h
if [ $? -ne 0 ]
then
   echo tag_attr_build 3 FAIL
   exit 1
fi
rm -f tmp-tag-attr-build3.tmp 
rm -f tmp-t3.c

rm -f tmp-t4.c
cp $top_srcdir/dwarfdump/tag_tree_ext.list tmp-t4.c
$CC  -I$top_srcdir/libdwarf  -DTRIVIAL_NAMING -E tmp-t4.c > tmp-tag-tree-build4.tmp
./tag_tree_build$EXEXT -e -i tmp-tag-tree-build4.tmp -o dwarfdump-tt-ext-table.h
if [ $? -ne 0 ]
then
   echo tag_tree_build 4 compile fail
   exit 1
fi

$CC -I $top_srcdir/libdwarf \
  $top_srcdir/dwarfdump/buildopscounttab.c \
  $top_srcdir/dwarfdump/dwarf_names.c -o buildop
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
