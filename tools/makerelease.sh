#!/bin/sh
# This encapsulates the way we release and how we set
# up tags marking a release.
# It uses configure and computes values we put
# in www.prevanders.net/dwarf.html 

d=0.10.1
echo "Assuming release is $d"
sleep 5
chkres() {
r=$1
m=$2
if [ $r -ne 0 ]
then
  echo "FAIL $m.  Exit status $r"
  exit 1
fi
}

bldloc=/tmp/bldrel
rm -rf $bldloc
mkdir $bldloc
chkres $? "mkdir failed"
cd $bldloc
chkres $? "cd failed"
~/dwarf/code/configure
chkres $? "configure failed"
make dist
chkres $? "make dist "
f=libdwarf-$d.tar.xz
echo "Release name: $f"

cp $f /home/davea/web4/gweb/pagedata/
chkres $? "cp to gweb failed"
cp $f /home/davea/Desktop/
chkres $? "cp to Desktop failed"
echo "Now size of release, bytes:"
ls -l $f
echo "Now md5sum:"
md5sum $f
chkres $? "md5sum failed"
echo "Now sha512sum:"
sha512sum $f| fold -w 32
chkres $? "sha512sum pipe failed"

  # To get unforgeable checksums for the tar.gz file
  # md5sum is weak, but the pair should be
  # a strong confirmation.
  # The fold(1) is just to make the web
  # release page easier to work with.

echo "The release is $bldloc/libdwarf-$d.tar.xz"
date
echo "Now do by hand:"
echo "git tag -a v$d  -m Release=$d"
echo "git tag -a libdwarf-$d  -m Release=$d"
echo "now push the tags:"
echo "git push origin libdwarf-$d"  
echo "git push origin v$d"    

echo "Try this  right after making release"
echo "git checkout main"
echo "git log | head -20"
echo "git checkout v$d"
echo "git log | head -20"
echo "git diff main v$d --name-status"
echo "Ensure the  log snippets match and the diff of"
echo "the last command produces no output."
echo "git checkout main"
echo "git log | head -20"
echo "git checkout libdwarf-$d"
echo "git log | head -20"
echo "git diff main libdwarf-$d --name-status"
echo "and check as just above"
echo "done makerelease.sh"


