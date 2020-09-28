/*  David Anderson. 2019. This small program is hereby
    placed into the public domain to be copied or
    used by anyone for any purpose.

    See
    https://sourceware.org/gdb/onlinedocs/\
        gdb/Separate-Debug-Files.html
    to calculate the crc of a file.
*/



#include "config.h"
#include <stdio.h>

/* Windows specific header files */
#if defined(_WIN32) && defined(HAVE_STDAFX_H)
#include "stdafx.h"
#endif /* HAVE_STDAFX_H */
#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for exit(), C89 malloc */
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_MALLOC_H
/* Useful include for some Windows compilers. */
#include <malloc.h>
#endif /* HAVE_MALLOC_H */
#include "dwarf.h"
#include "libdwarf.h"

char trueoutpath[2000];

static const char *dlname = ".gnu_debuglink";
static const char *buildidname = ".note.gnu.buildid";

static int doprintbuildid = 1;
static int doprintdebuglink = 1;

static void
one_file_debuglink(char *path)
{
    int res = 0;
    Dwarf_Debug dbg = 0;
    unsigned i = 0;
    char *debuglinkpath = 0; /* must be freed */
    unsigned char *crc = 0;
    char *debuglinkfullpath = 0;
    unsigned debuglinkfullpath_strlen = 0;
    unsigned buildid_type = 0;
    char * buildidownername = 0;
    unsigned char *buildid = 0;
    unsigned buildid_length = 0;
    char ** paths = 0; /* must be freed */
    unsigned paths_count = 0;
    Dwarf_Error error = 0;

    res = dwarf_init_path(path,
        trueoutpath,
        sizeof(trueoutpath),
        DW_DLC_READ,
        DW_GROUPNUMBER_ANY,
        0,0,
        &dbg,
        0,0,0,&error);
    if (res == DW_DLV_ERROR) {
        printf("Error from libdwarf opening \"%s\":  %s\n",
            path,
            dwarf_errmsg(error));
        return;
    }
    if (res == DW_DLV_NO_ENTRY) {
        printf("There is no such file as \"%s\"\n",
            path);
        return;
    }
    /*  We could call dwarf_add_debuglink_global_path()
        for each additional global path beside the default.
        Instead of
            &paths,&paths_count,
        pass 0,0 and do the paths construction yourself. */
    res = dwarf_gnu_debuglink(dbg,
        &debuglinkpath,
        &crc,
        &debuglinkfullpath,
        &debuglinkfullpath_strlen,
        &buildid_type,
        &buildidownername,
        &buildid,
        &buildid_length,
        &paths,
        &paths_count,
        &error);
    if (res == DW_DLV_ERROR) {
        printf("Error from libdwarf accessing debuglink "
            "related sections in \"%s\": %s\n",
            path,
            dwarf_errmsg(error));
        res = dwarf_finish(dbg,&error);
        return;
    } else if (res == DW_DLV_NO_ENTRY) {
        printf("There is no %s or %s section in \"%s\"\n",
            dlname,buildidname,path);
        res = dwarf_finish(dbg,&error);
        return;
    }
    if (doprintdebuglink && crc) {
        printf(" Section %s\n",dlname);
        printf(" Debuglink name  : %s",debuglinkpath);
        {
            unsigned char *crcx = 0;
            unsigned char *end = 0;

            crcx = crc;
            end = crcx + 4;
            printf("   crc 0X: ");
            for (; crcx < end; crcx++) {
                printf("%02x ", *crcx);
            }
        }
        printf("\n");
        if (debuglinkfullpath_strlen) {
            printf(" Debuglink target: %s\n",debuglinkfullpath);
        }

    }

    if (doprintbuildid && buildid) {
        printf(" Section %s\n",buildidname);
        printf(" Build-id  type     : %u\n", buildid_type);
        printf(" Build-id  ownername: %s\n",
            buildidownername);
        printf(" Build-id  length   : %u\n",buildid_length);
        printf(" Build-id           : ");
        {
            const unsigned char *cur = 0;
            const unsigned char *end = 0;

            cur = buildid;
            end = cur + buildid_length;
            for (; cur < end; cur++) {
                printf("%02x", (unsigned char)*cur);
            }
        }
        printf("\n");
    }

    /*  We could ignore the paths and paths_count
        from libdwarf and develop a list of possible
        paths ourselves. */
    for (i =0; i < paths_count; ++i) {
        char *pa =     paths[i];
        char           outpath[2000];
        unsigned long  outpathlen = sizeof(outpath);
        unsigned int   ftype = 0;
        unsigned int   endian = 0;
        unsigned int   offsetsize = 0;
        Dwarf_Unsigned filesize = 0;
        int errcode = 0;

        printf("Path [%2u] %s\n",i,pa);
        /*  First, opn the file to determine if it exists.
            If not, loop again */
        res = dwarf_object_detector_path(pa,
            outpath,outpathlen,&ftype,&endian,&offsetsize,
            &filesize, &errcode);
        if (res == DW_DLV_NO_ENTRY) {
            printf(" file above does not exist\n");
            continue;
        }
        if (res == DW_DLV_ERROR) {
            printf(" file above access attempt lead to error %s\n",
                dwarf_errmsg_by_number(errcode));
            continue;
        }
        switch(ftype) {
        case DW_FTYPE_ELF:
            printf(" file above is an Elf object\n");
            break;
        case DW_FTYPE_MACH_O:
            printf(" file above is a Mach-O object\n");
            break;
        case DW_FTYPE_PE:
            printf(" file above is a PE object");
            break;

        case DW_FTYPE_CUSTOM_ELF:
            printf(" file above is a custom elf object");
            break;
        case DW_FTYPE_ARCHIVE:
            printf(" file above is an archive so ignore it.");
            continue;
        default:
            printf(" file above is not an object type"
                " we recognize\n");
            continue;
        }

        /*  if crc is non-null calculate the crc of the
            opened file and compare the 4-byte values.
            If they match, this is the desired object file
            with debug information. Do a dwarf_init_path()
            initializing (for example) Dwarf_Debug dbg2;
            Else continue the loop. */
    }
    free(paths);
    free(debuglinkfullpath);
    dwarf_finish(dbg,&error);
    return;
}


int main(int argc, char **argv)
{
    int i = 1;
    char *filenamein = 0;

    for ( ; i < argc; ++i) {
        filenamein = argv[i];
        one_file_debuglink(filenamein);
    }
}
