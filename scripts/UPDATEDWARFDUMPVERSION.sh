#!/bin/bash
# Do not turn on -x here. it will screw things up!

l=`pwd`
echo $l
lb=`basename $l`
ld=`dirname $l`
#echo " lb: $lb"
#echo " ld: $ld"
cdtarg=.
sloc=$l/scripts
if [ x$lb = "xscripts" ]
then
  #echo "ld: $ld"
  ldb=`basename $ld`
  #echo "ldb: $ldb"
  if [ x$ldb = "xcode" ]
  then
     cdtarg=..
     sloc=$l
  else
     echo "Run from */code, not $l , giving up."
     exit 1
  fi
  cd $cdtarg
  if [ $? -ne 0 ]
  then
     echo "cd $cdtarg failed, giving up."
     exit 1
  fi
else
  if [ x$lb = "xcode" ]
  then
    cdtarg="."
  else
    echo "Run from */code, not $l , giving up."
    exit 1
  fi
  # No need to cd.
fi
l=`pwd`
#echo "Now at $l"
#echo "sloc $sloc"



x=`date --rfc-3339=seconds |tr '\n' ' '`
cat > $sloc/UPD.awk <<\EOF
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
awk -f UPD.awk  "$x"  dwarfdump/dwarfdump.c >t
mv t dwarfdump/dwarfdump.c
awk -f UPD.awk  "$x"  dwarfdump/common.c >t
mv t dwarfdump/common.c
awk -f UPD.awk  "$x"  dwarfdump/tag_attr.c >t
mv t dwarfdump/tag_attr.c
awk -f UPD.awk  "$x"  dwarfdump/tag_tree.c >t
mv t dwarfdump/tag_tree.c
awk -f UPD.awk  "$x"  libdwarf/gennames.c >t
mv t libdwarf/gennames.c
