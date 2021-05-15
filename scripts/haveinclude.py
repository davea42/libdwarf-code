#!/usr/bin/env python3
"""
Ensure for every incude <stdlib.h> that we
have the ifdef HAVE_STDLIB_H  and endif
bracketing it.  And so on.
"""

import os
import sys

includelist = [
    ("HAVE_ALLOCA_H", "alloca.h"),
    ("HAVE_DLFCN_H", "dlfcn.h"),
    ("HAVE_ELFACCESS_H", "elfaccess.h"),
    ("HAVE_ELF_H", "elf.h"),
    ("HAVE_INTTYPES_H", "inttypes.h"),
    ("HAVE_LIBELF_H", "libelf.h"),
    ("HAVE_LIBELF_LIBELF_H", "libelf/libelf.h"),
    ("HAVE_MALLOC_H", "malloc.h"),
    ("HAVE_MEMORY_H", "memory.h"),
    ("HAVE_REGEX_H", "regex.h"),
    ("HAVE_SGIDEFS_H", "sgidefs.h"),
    ("HAVE_STDINT_H", "stdint.h"),
    ("HAVE_STDLIB_H", "stdlib.h"),
    ("HAVE_STRINGS_H", "strings.h"),
    ("HAVE_STRING_H", "string.h"),
    ("HAVE_SYS_ELF_386_H", "sys/elf_386.h"),
    ("HAVE_SYS_ELF_AMD64_H", "sys/elf/amd64_h"),
    ("HAVE_SYS_ELF_SPARC_H", "sys/elf_sparc_h"),
    ("HAVE_SYS_IA64_ELF_H", "sys/ia64_elf.h"),
    ("HAVE_SYS_STAT_H", "sys/stat.h"),
    ("HAVE_SYS_TYPES_H", "sys/types.h"),
    ("HAVE_UNISTD_H", "unistd.h"),
    ("HAVE_WINDOWS_H", "windows.h"),
    ("HAVE_ZLIB_H", "zlib.h"),
]

havedict = {}
incldict = {}
for x in includelist:
    (h, i) = x
    havedict[h] = i
    incldict[i] = h

ifdef = "#ifdef"
endif = "#endif"
include = "#include"


def lineisofinterest(line):
    if line.startswith(ifdef):
        return ifdef
    if line.startswith(include):
        return include
    if line.startswith(endif):
        return endif
    return False


def isolatetag(t, iline):
    if t[0] == "<":
        x = t.replace("<", "")
        y = x.replace(">", "")
        return y
    elif t[0] == '"':
        x = t.replace('"', "")
        return x
    else:
        print("Something very wrong with include on line ", iline)
        sys.exit(1)


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
        if x == ifdef:
            if ifdefdepth > 0:
                print("nested include of ", wds[1], "line", iline)
            else:
                ifdeftag = wds[1]
            ifdefdepth = int(ifdefdepth) + 1
            # print("ifdef depth",ifdefdepth, " at ",\
            # wds[1], "line",iline)
        elif x == include:
            includetag = isolatetag(wds[1], iline)
            if ifdefdepth == 0:
                if includetag in incldict:
                    print("Found include ", includetag, "not in ifdef/endif")
            elif ifdefdepth > 1:
                pass
            else:
                if ifdeftag in havedict:
                    if includetag in incldict:
                        # normal
                        pass
                    else:
                        if ifdefdepth == 0:
                            print("Found include ", includetag, "not in ifdef/endif")
                        elif ifdefdepth == 1:
                            print(
                                "Found have tag ",
                                ifdeftag,
                                "with unk include ",
                                includetag,
                            )
                        else:
                            pass
                else:
                    if includetag in incldict:
                        print(
                            "Found unk have tag ", ifdeftag, "with include ", includetag
                        )
        elif x == endif:
            if ifdefdepth > 1:
                print("end nested include line ", iline)
            else:
                pass
            ifdefdepth = int(ifdefdepth) - 1
            # print("ifdef depth",ifdefdepth, " at ",\
            # wds[1],"line",iline)
        else:
            print("Fail at line ", iline)
            sys.exit(1)
    if ifdefdepth > 0:
        print("ends improperly, not nested ifdef ")


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
