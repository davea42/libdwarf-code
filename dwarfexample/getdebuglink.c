/*  David Anderson.  2019-2020. This small program is hereby
    placed into the public domain to be copied or
    used by anyone for any purpose.

    See
    https://sourceware.org/gdb/onlinedocs/\
        gdb/Separate-Debug-Files.html
    to calculate the crc of a file.
    It's unclear whether one can legally
    copy the crc code into this library/example
    source, so we do nothing with the crc.

    An emerging GNU pattern seems to be:
        If a path found from debuglink or
        build-id matches an elf object then
        both the objects may bave a build-id
        and if it is the right object the build-ids
        will match.  This pattern does not
        refer to the crc from the executable
        debuglink.
    To build a separate debug file x.debug with
    DWARF and an executable with just debugid
    and debuglink data using the pattern
    seen in Ubuntu 20.04:

    first compile and link, creating x
    then:
    objcopy --only-keep-debug x x.debug
    objcopy --strip-debug x
    objcopy --add-gnu-debuglink=x.debug x

    See 'man objcopy' or
    https://sourceware.org/binutils/docs/binutils/objcopy.html 
    for more information.
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

#ifdef HAVE_UNUSED_ATTRIBUTE
#define  UNUSEDARG __attribute__ ((unused))
#else
#define  UNUSEDARG
#endif

#define TRUE 1
#define FALSE 1
char trueoutpath[2000];
static const char *dlname = ".gnu_debuglink";
static const char *buildidname = ".note.gnu.buildid";

static int doprintbuildid = 1;
static int doprintdebuglink = 1;

static int
blockmatch(unsigned char *l,
    unsigned char* r,
    unsigned length)
{
    unsigned int i = 0;
    for( ; i < length; ++i) {
        if (l[i] != r[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

/*  The debug version we expect not to have debuglink,
    checking here if buildid matches. */
static int
match_buildid(const char *prefix,
    UNUSEDARG  unsigned char *crc_base,
    unsigned         buildid_length_base,
    unsigned  char  * buildid_base,
    UNUSEDARG char *   path_base,

    UNUSEDARG unsigned char  * crc_debug,
    unsigned  buildid_length_debug,
    unsigned  char *buildid_debug,
    UNUSEDARG char *   path_debug)
{
    if(buildid_length_base != buildid_length_debug) {
        return DW_DLV_NO_ENTRY;
    }
    if (!blockmatch(buildid_base,buildid_debug,
        buildid_length_base)) {
        return DW_DLV_NO_ENTRY;
    }
    printf("%s executable and debug buildid match\n",prefix);
    return DW_DLV_OK;
}

static int
one_file_debuglink_internal(const char *prefix,
    char *path_in,
    unsigned char * crc_in,
    unsigned        buildid_len_in,
    unsigned char * buildid_in,
    char          * debug_path_in)
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
    char  * path = 0;
    char  * basepath = 0;
    Dwarf_Error error = 0;


    path = basepath = path_in;

    if (debug_path_in) {
        path = debug_path_in;
        printf("%s==Referred-path: %s\n",prefix,debug_path_in);
    } else {
        printf("%s==Exec-path    : %s\n",prefix,basepath);
    }
    res = dwarf_init_path(path,
        trueoutpath,
        sizeof(trueoutpath),
        DW_DLC_READ,
        DW_GROUPNUMBER_ANY,
        0,0,
        &dbg,
        0,0,0,&error);
    if (res == DW_DLV_ERROR) {
        printf("%sError from libdwarf opening \"%s\":  %s\n",
            prefix,
            path,
            dwarf_errmsg(error));
        return res;
    }
    if (res == DW_DLV_NO_ENTRY) {
        printf("%sThere is no such file as \"%s\"\n",
            prefix,
            path);
        return DW_DLV_NO_ENTRY;
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
        printf("%sError from libdwarf accessing debuglink "
            "related sections in \"%s\": %s\n",
            prefix,
            path,
            dwarf_errmsg(error));
        dwarf_finish(dbg,&error);
        return res;
    } else if (res == DW_DLV_NO_ENTRY) {
        printf("%sThere is no %s or %s section in \"%s\"\n",
            prefix,dlname,buildidname,path);
        res = dwarf_finish(dbg,&error);
        return res;
    }
    if (doprintdebuglink && crc) {
        printf("%s Section %s\n",prefix,dlname);
        printf("%s Debuglink name  : %s",prefix,debuglinkpath);
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
            printf("%s Debuglink target: %s\n",
                prefix,debuglinkfullpath);
        }
    }
    if (doprintbuildid && buildid) {
        printf("%s Section %s\n",prefix,buildidname);
        printf("%s Build-id  type     : %u\n",prefix, buildid_type);
        printf("%s Build-id  ownername: %s\n",prefix,
            buildidownername);
        printf("%s Build-id  length   : %u\n",prefix,buildid_length);
        printf("%s Build-id           : ",prefix);
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
    if (debug_path_in) {
        res = match_buildid(prefix,
            crc_in,buildid_len_in,buildid_in,
            basepath,
            /* following is the target, ie, debug */
            crc,buildid_length,buildid,path);
        if ( res == DW_DLV_OK) {
            free(paths);
            free(debuglinkfullpath);
            dwarf_finish(dbg,&error);
            return res;
        }
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

        printf("%s Path [%2u] %s\n",prefix,i,pa);
        /*  First, open the file to determine if it exists.
            If not, loop again */
        res = dwarf_object_detector_path(pa,
            outpath,outpathlen,&ftype,&endian,&offsetsize,
            &filesize, &errcode);
        if (res == DW_DLV_NO_ENTRY) {
            printf("%s file above does not exist\n",prefix);
            continue;
        }
        if (res == DW_DLV_ERROR) {
            printf("%s file above access attempt lead to error %s\n",
                dwarf_errmsg_by_number(errcode),prefix);
            continue;
        }
        switch(ftype) {
        case DW_FTYPE_ELF:
            printf("%s file above is an Elf object\n",prefix);
            break;
        case DW_FTYPE_MACH_O:
            printf("%s file above is a Mach-O object\n",prefix);
            break;
        case DW_FTYPE_PE:
            printf("%s file above is a PE object",prefix);
            break;

        case DW_FTYPE_CUSTOM_ELF:
            printf("%s file above is a custom elf object",prefix);
            break;
        case DW_FTYPE_ARCHIVE:
            printf("%s file above is an archive so ignore it.",
                prefix);
            continue;
        default:
            printf("%s file above is not an object type"
                " we recognize\n",prefix);
            continue;
        }
        /*  Now see if the debug has buildid matching
            the executable. */
        if (!debug_path_in) {
            res = one_file_debuglink_internal(
                "    ",
                path,crc,buildid_length,
                buildid,pa);
        }
        if (res == DW_DLV_OK) {
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
    return DW_DLV_OK;
}

static void
one_file_debuglink(char *path)
{
   one_file_debuglink_internal("",path,0,0,0,0);
}

int
main(int argc, char **argv)
{
    int i = 1;
    char *filenamein = 0;

    for ( ; i < argc; ++i) {
        filenamein = argv[i];
        one_file_debuglink(filenamein);
    }
}
