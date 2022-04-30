#!/usr/bin/env python3

import os
import sys

def writetextout(text, filepath):
    try:
        f = open(filepath, "w")
    except:
        print("Unable to write dwarfdump putput ", filepath, " giving up")
        sys.exit(1)
    for s in txtout:
        # Even on windows writes in Linux text format
        print(s.rstrip("\r\n"), file=f, end="\n")
    f.close()


def readin(srcfile):
    hasdos = False
    try:
        f = open(srcfile, "r")
    except:
        print("Unable to open input conf ", srcfile, " giving up")
        sys.exit(1)
    y = f.readlines()
    for l in y:
        if l.endswith("\r\n"):
            hasdos = True
            break
    f.close()
    return hasdos, y


if __name__ == "__main__":
    filepath = False
    if len(sys.argv) > 1:
        filepath = sys.argv[1]
    hasdos, lines = readin(filepath)
    if hasdos:
        writeoutput(lines, filepath)
