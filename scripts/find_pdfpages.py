#/usr/bin/env python3
"""
Run this against the output of pdftotxt
Here we use formfeeds to identify pages and page numbers.
This is invoked via libdwarf/pdfbld.sh -c
to rearrange a pdf so the order is
abstract, table of contents,major document content.
See also scripts/rebuildpdf.sh
"""


import os
import sys

def scanfile(path):
    try:
        fx = open(path,"r")
    except IOError as  message:
        print("File",path,"could not be opened: ", message)
        sys.exit(1)
    iline = 0
    contentpage = 0
    abstractpage = 0
    lastline = ''
    ipage = 0
    while 1:
        iline = int(iline)+1
        try:
            rline = fx.readline()
        except IOError as message:
            print("read fails, line ",iline,message)
            sys.exit(1)
        if rline == '':
            break
        if rline[0] == chr(0xc):
            ipage = int(ipage) + 1
            wds = lastline.split()
            if len(wds) < 3:
                continue
            if wds[0] != "-":
                continue
            if wds[2] != "-":
                continue
            pnum = wds[1]
            if not pnum.isdigit():
                continue
            contentpage = ipage
        else:
            if len(rline) < 2:
                pass
            else:
                lastline = rline.strip()
    # Ends with lone formfeed.
    abstractpage= int(ipage)
    contentpage = int(contentpage) + 1
    return (contentpage,abstractpage)

if __name__ == '__main__':
    i = 1
    ct = 0
    if i < len(sys.argv):
        path = sys.argv[i]
        #print("=======path=",path)
        (a,c) =scanfile(path);
        if int(a) == 0:
            print("Could not find abstract page")
            sys.exit(1)
        if int(c) == 0:
            print("Could not find abstract page")
            sys.exit(1)
        print(c)
        print(a)
    else:
        print("file name argument(s) required");
        sys.exit(1)
