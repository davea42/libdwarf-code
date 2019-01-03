
# A helper script to extract defines from header files.
# This trivial script is by David Anderson and is hereby put in to the public domain
# as it is is trivial so might as well be public.

for i in ../libdwarf/dwarf_reloc*h
do
    b=`basename $i`
    grep '^#define' <$i >/tmp/raw_elf_defines$b
done
