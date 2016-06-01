# Do not turn on -x here. it will screw things up!
x=`date --rfc-3339=seconds |tr '\n' ' '`
cat > UPD.awk <<\EOF
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
