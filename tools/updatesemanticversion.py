#!/usr/bin/env python3
# Update current package semantic version.

import os
import sys


def usage():
    print("Example of use: updatesemanticversion 0.2.0")
    print("Then git push as appropriate")
    sys.exit(0)


def deriveversions(sver):
    nums = sver.split(".")
    if len(nums) != 3:
        return (False, 0, 0, 0)
    ma = nums[0]
    min = nums[1]
    mic = nums[2]
    if not ma.isdecimal():
        return (False, 0, 0, 0)
    if not min.isdecimal():
        return (False, 0, 0, 0)
    if not mic.isdecimal():
        return (False, 0, 0, 0)
    return (True, ma, min, mic)


# set(VERSION 0.2.0)
# m4_define([v_maj], [0])
# m4_define([v_min], [2])
# m4_define([v_mic], [0])


def extractvercm(l):
    wds = l.rstrip().split(" ")
    if not len(wds) == 2:
        print("Something wrong, cmake version not findable", l)
        sys.exit(1)
    v = wds[1]
    vwds = v.split(")")
    ver = vwds[0]
    return ver


def bracketval(s):
    s2 = "".join(["[", s, "]"])
    return s2


def updatecmversion(l, ver):
    curver = extractvercm(l)
    if curver != ver:
        return (curver, ver)
    return False, False


def extractverac(l, prefix):
    vend = l.replace(prefix, "", 1)
    vwds = vend.split("]")
    ver = vwds[0]
    return ver


def updateacversion(l, prefix, maj, min, mic):
    curver = extractverac(l, prefix)
    n = "mic"
    if maj:
        n = "maj"
        ver = maj
    elif min:
        n = "min"
        ver = min
    else:
        ver = mic
    if curver != ver:
        return (bracketval(curver), bracketval(ver))
    return False, False


#  type is "ac" or "cm"
def updatefile(fname, type, sver, maj, min, mic):
    begincm = "set(VERSION "
    ma = "m4_define([v_maj], ["
    mn = "m4_define([v_min], ["
    mc = "m4_define([v_mic], ["
    foundcm = 0
    foundac = 0
    print("Processing", fname, " type", type, " newver", sver)
    try:
        fin = open(fname, "r")
    except IOError as message:
        print("File could not be opened read: ", message, file=sys.stderr)
        sys.exit(1)
    content = fin.readlines()
    fin.close()
    outdata = []
    for i, l in enumerate(content):
        if type == "cm":
            if l.startswith(begincm):
                foundcm += 1
                print("Found CM version line", i, "current line", l)
                (cur, newv) = updatecmversion(l, sver)
                print("dadebug CM cur,newv", cur, newv)
                if cur:
                    s = l.replace(cur, newv, 1)
                    outdata += [s]
                else:
                    outdata += [l]
                continue
            outdata += [l]
            continue
        else:
            if l.startswith(ma):
                foundac += 1
                print("Found major in line", i, "current line", l[0:25])
                (cur, newv) = updateacversion(l, ma, maj, False, False)
                if cur:
                    s = l.replace(cur, newv, 1)
                    print("updated major in line", i, "current line", l[0:25])
                    outdata += [s]
                else:
                    outdata += [l]
                continue
            if l.startswith(mn):
                foundac += 1
                print("Found minor in line", i, "current line", l[0:25])
                (cur, newv) = updateacversion(l, mn, False, min, False)
                if cur:
                    s = l.replace(cur, newv, 1)
                    print("updated minor in line", i, "current line", l[0:25])
                    outdata += [s]
                else:
                    outdata += [l]
                continue
            if l.startswith(mc):
                foundac += 1
                print("Found micro in line", i, "current line", l[0:25])
                (cur, newv) = updateacversion(l, mc, False, False, mic)
                if cur:
                    s = l.replace(cur, newv, 1)
                    print("updated micro in line", i, "current line", l[0:25])
                    outdata += [s]
                else:
                    outdata += [l]
                continue
            outdata += [l]
    if type == "cm" and not foundcm:
        print("Something wrong, did not find", fname, "  version line")
        sys.exit(1)
    if type == "ac" and not foundac == 3:
        print("Something wrong, did not find configure.ac", "version lines")
        sys.exit(1)
    try:
        fo = open(fname, "w")
    except IOError as message:
        print("File could not be opened write: ", message, file=sys.stderr)
        sys.exit(1)
    for i, l in enumerate(outdata):
        print(l, end="", file=fo)
    fo.close()


# set(VERSION 0.2.0)
# m4_define([v_maj], [0])
# m4_define([v_min], [2])
# m4_define([v_mic], [0])
expecteddirs = ["bugxml", "cmake", "doc", "m4", "scripts", "src", "test"]


if __name__ == "__main__":
    if len(sys.argv) < 2:
        usage()
    if len(sys.argv) > 2:
        usage()
    sver = sys.argv[1]

    ok, maj, min, mic = deriveversions(sver)
    if not ok:
        print("the new semantic version", sver, "is not in the properformat")
        sys.exit(1)
    dirnames = os.listdir(".")
    for d in expecteddirs:
        if d not in dirnames:
            print(
                "Run in base code directory.",
                d,
                "is missing so we are in the wrong directory",
            )
            sys.exit(1)
    updatefile("configure.ac", "ac", sver, maj, min, mic)
    updatefile("CMakeLists.txt", "cm", sver, maj, min, mic)
    print("Updated: done.")
