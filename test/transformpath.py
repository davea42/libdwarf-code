#!/usr/bin/env python3

# Run aas:
# transformpath.py inpath outpath
# Does the transformations of paths

import os
import sys

def writeout(lines,path):
    try:
        f = open(path, "w")
    except:
        print("Unable to open output ", path, " giving up")
        sys.exit(1)
    for l in lines:
        print(l,file=f)
    f.close()

def readin(srcfile):
    try:
        f = open(srcfile, "r")
    except:
        print("Unable to open input file ", srcfile, " giving up")
        sys.exit(1)
    y = f.readlines()
    out = []
    for l in y:
        x=l.rstrip()
        out += [x]
    f.close()
    return out

#  for Windows msys2, the default prefix of msys2 /
globalroot = 'C:/msys64'

def transform(ilines,srcpath,binpath):
    out = []
    for n,l1 in enumerate(ilines):
        l2 = l1
        if l1.find(srcpath) != -1:
            l2 = l1.replace(srcpath,"..src..")
        l3 = l2
        if l2.find(binpath) != -1:
            l3 = l2.replace(binpath,"..bld..")
        l4 = l3
        if l3.find(globalroot) != -1:
            l4 = l3.replace(globalroot,"")
        out += [l4]
    return out

if __name__ == "__main__":
    origfile = False
    newfile = False
    if len(sys.argv) > 4:
        localsrcpath = sys.argv[1]
        localbinpath = sys.argv[2]
        infile = sys.argv[3]
        outfile  = sys.argv[4]
    else:
        print("transformpath. args required:", \
            "localsrcpath localbinpath  inputpath outpath")
        exit(1)
    ilines = readin(infile)
    outlines = transform(ilines,localsrcpath,localbinpath)
    writeout(outlines,outfile)
    sys.exit(0)
