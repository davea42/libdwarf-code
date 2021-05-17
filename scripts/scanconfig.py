#!/usr/bin/env python3
"""
Modify config.h.in config.h.in.cmake
so the undef/cmakedefine lines get 
extracted and simplified to see if there
are errors or omissions between them.
"""

import os
import sys


cmdef = "#cmakedefine"
undef = "#undef"


def lineisofinterest(line):
    if line.startswith(cmdef):
        return cmdef
    if line.startswith(undef):
        return undef
    return False



def scanfile(path):
    try:
        fx = open(path, "r")
    except IOError as message:
        print("File", path, "could not be opened: ", message)
        sys.exit(1)

    ifdefline = 0
    insideifdef = False
    ifdefdepth = 0
    ifdeftag = ""
    includetag = ""
    includeline = 0
    endifline = 0
    iline = 0
    while 1:
        iline = int(iline) + 1
        try:
            rline = fx.readline()
        except IOError as message:
            print("read fails, line ", iline, message)
            sys.exit(1)
        if rline == "":
            break
        tline = rline.strip()
        x = lineisofinterest(tline)
        if not x:
            continue
        wds = tline.split()
        print("config",wds[1])

if __name__ == "__main__":
    i = 1
    ct = 0
    while i < len(sys.argv):
        path = sys.argv[i]
        print("=======path=", path)
        scanfile(path)
        ct = int(ct) + 1
        i = int(i) + 1
    if int(ct) < 1:
        print("file name argument(s) required")
        sys.exit(1)
