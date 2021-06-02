/*  David Anderson.  2019-2020. This small program is hereby
    placed into the public domain to be copied or
    used by anyone for any purpose.

    See
    https://sourceware.org/gdb/onlinedocs/\
        gdb/Separate-Debug-Files.html

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
#ifdef HAVE_UNISTD_H
#include <unistd.h> /* for close */
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h> /* for off_t ssize_t */
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_MALLOC_H
/* Useful include for some Windows compilers. */
#include <malloc.h>
#endif /* HAVE_MALLOC_H */
#ifdef HAVE_STRING_H
#include <string.h> /* for memset */
#endif /* HAVE_STRING_H */
#include "dwarf.h"
#include "libdwarf.h"

#define TRUE 1
#define FALSE 0
char trueoutpath[2000];
static const char *dlname = ".gnu_debuglink";
static const char *buildidname = ".note.gnu.buildid";

static int doprintbuildid = 1;
static int doprintdebuglink = 1;

#define Dwarf_Small unsigned char
static void
dump_bytes(const char *prefix,
    char              *msg,
    unsigned char     *start,
    long              len)
{
    Dwarf_Small *end = start + len;
    Dwarf_Small *cur = start;
    if (!start) {
        printf("%s ptr null, ignore. \n",msg);
        return;
    }
    printf("%s%s ",prefix,msg);
    for (; cur < end; cur++) {
        printf("%02x ", *cur);
    }
    printf("\n");
}

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

static void
print_debuglink(const char *prefix,
    char        *debuglinkpath,
    unsigned char *crc,
    char        *debuglinkfullpath,
    unsigned int debuglinkfullpath_strlen)
{
    unsigned char *crcx = 0;
    unsigned char *end = 0;

    printf("%s Section            : %s\n",prefix,dlname);
    printf("%s Debuglink name     : %s\n",prefix,debuglinkpath);
    crcx = crc;
    end = crcx + 4;
    printf("%s crc              0X: ",prefix);
    for (; crcx < end; crcx++) {
        printf("%02x ", *crcx);
    }
    printf("\n");
    if (debuglinkfullpath_strlen) {
        printf("%s Debuglink target   : %s\n",
            prefix,debuglinkfullpath);
    }
}

static void
print_buildid(const char *prefix,
    unsigned int   buildid_type,
    char          *buildidownername,
    unsigned int   buildid_length,
    unsigned char  *buildid)
{
    printf("%s Section            : %s\n",prefix,buildidname);
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

/* Returns TRUE if its a real file of some interest. */
static int
print_ftype_message(const char * prefix,
    unsigned int ftype)
{
    switch(ftype) {
    case DW_FTYPE_ELF:
        printf("%s file above is an Elf object\n",prefix);
        return TRUE;
    case DW_FTYPE_MACH_O:
        printf("%s file above is a Mach-O object\n",prefix);
        return TRUE;
    case DW_FTYPE_PE:
        printf("%s file above is a PE object",prefix);
        return TRUE;
    case DW_FTYPE_CUSTOM_ELF:
        printf("%s file above is a custom elf object",prefix);
        return TRUE;
    case DW_FTYPE_ARCHIVE:
        printf("%s file above is an archive so ignore it.",
            prefix);
        return FALSE;
    default:
        printf("%s file above is not an object type"
            " we recognize\n",prefix);
    }
    return FALSE;
}



/*  The debug version we expect not to have debuglink,
    checking here if buildid matches.
    Never returns DW_DLV_ERROR. */
static int
match_buildid(const char *prefix,
    unsigned char *crc_base,
    unsigned         buildid_length_base,
    unsigned  char  * buildid_base,
    /*  *_base is executable info while
        *_debug is the debug object. */
    unsigned char  * crc_debug,
    unsigned  buildid_length_debug,
    unsigned  char *buildid_debug)
{
    if (crc_debug && crc_base) {
        /* crc available for both */
        if (!blockmatch(crc_debug,crc_base,4)) {
            dump_bytes(prefix," crc base  ",crc_base,4);
            dump_bytes(prefix," crc target",crc_debug,4);
            printf("%s===crc does not match\n",prefix);
            return DW_DLV_NO_ENTRY;
        }
    } else {
    }
    if(buildid_length_base != buildid_length_debug) {
        printf("%s===buildid length does not match",prefix);
        return DW_DLV_NO_ENTRY;
    }
    if (!blockmatch(buildid_base,buildid_debug,
        buildid_length_base)) {
        printf("%s===buildid does not match\n",prefix);
        return DW_DLV_NO_ENTRY;
    }
    return DW_DLV_OK;
}

static int
one_file_debuglink_internal(int is_outer,const char *prefix,
    char          **gl_pathnames,
    unsigned        gl_pathcount,
    int             no_follow_debuglink,
    char           *path_in,
    unsigned char  *crc_in,
    unsigned        buildid_len_in,
    unsigned char  *buildid_in,
    char           *debug_path_in)
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
    Dwarf_Unsigned laccess = DW_DLC_READ;
    unsigned int p = 0;

    /*  Don't let dwarf_init_path find the debuglink,
        we want to do it here so we can show it all. */
    path = basepath = path_in;
    if (!is_outer) {
        path = debug_path_in;
        printf("%s===Referred-path    : %s\n",prefix,debug_path_in);
    } else {
        printf("%s===Exec-path        : %s\n",prefix,basepath);
    }
    res = dwarf_init_path(path,
        0,0,
        laccess, DW_GROUPNUMBER_ANY,
        0,0, &dbg,
        0,0,0,&error);
    if (res == DW_DLV_ERROR) {
        printf("%sError from libdwarf opening \"%s\":  %s\n",
            prefix, path, dwarf_errmsg(error));
        dwarf_dealloc_error(dbg,error);
        error = 0;
        return res;
    }
    if (res == DW_DLV_NO_ENTRY) {
        printf("%sThere is no such file as \"%s\"\n",
            prefix, path);
        return DW_DLV_NO_ENTRY;
    }
    if (is_outer && no_follow_debuglink) {
        printf("%s no follow debuglink: TRUE\n",prefix);
    }
    for (p = 0; p < gl_pathcount; ++p) {
        const char *lpath = 0;

        lpath = (const char *)gl_pathnames[p];
        res = dwarf_add_debuglink_global_path(dbg,
            lpath, &error);
        if (res != DW_DLV_OK){
            printf("Failed add global path. result %d line %d\n",
                res,__LINE__);
            exit(1);
        }
        printf("%s global path        : %s\n",prefix,lpath);
    }
    res = dwarf_gnu_debuglink(dbg,
        &debuglinkpath,
        &crc, &debuglinkfullpath, &debuglinkfullpath_strlen,
        &buildid_type, &buildidownername,
        &buildid, &buildid_length,
        &paths, &paths_count, &error);
    if (res == DW_DLV_ERROR) {
        printf("%sError from libdwarf accessing debuglink "
            "related sections in \"%s\": %s\n",
            prefix, path, dwarf_errmsg(error));
        dwarf_dealloc_error(dbg,error);
        error = 0;
        dwarf_finish(dbg,&error);
        return res;
    } else if (res == DW_DLV_NO_ENTRY) {
        printf("%sThere is no %s or %s section in \"%s\"\n",
            prefix,dlname,buildidname,path);
        res = dwarf_finish(dbg,&error);
        return DW_DLV_NO_ENTRY;
    }
    if (doprintdebuglink && crc) {
        print_debuglink(prefix,debuglinkpath,crc,
            debuglinkfullpath,debuglinkfullpath_strlen);
    }
    if (doprintbuildid && buildid) {
        print_buildid(prefix, buildid_type,
            buildidownername, buildid_length, buildid);
    }
    if (!is_outer) {
        unsigned char lcrc[4];

        /*  dbg might be the correct .debug object */
        memset(&lcrc[0],0,sizeof(lcrc));
        if (crc_in && !crc) {
            res = dwarf_crc32(dbg,lcrc,&error);
            if (res == DW_DLV_ERROR) {
                dwarf_dealloc_error(dbg,error);
                error = 0;
            } else if (res == DW_DLV_OK) {
                crc = &lcrc[0];
            }
        }
        res = match_buildid(prefix,
            /* This is the executable */
            crc_in,buildid_len_in,buildid_in,
            /* pass in dbg so we can calculate the missing crc */
            /* following is the target, ie, debug */
            crc,buildid_length,buildid);
        if (res == DW_DLV_OK) {
            printf("%s===executable and debug buildid match\n",
                prefix);
        }
        return DW_DLV_NO_ENTRY;
    }
    /*  If debug_path_in then this list does not
        mean anything. */

    for (i =0; is_outer && i < paths_count; ++i) {
        char *pa =     paths[i];
        unsigned int   ftype = 0;
        unsigned int   endian = 0;
        unsigned int   offsetsize = 0;
        Dwarf_Unsigned filesize = 0;
        int errcode = 0;
        int realobj = TRUE;
        static char lprefix[50];

        snprintf(lprefix,sizeof(lprefix), "    [%2u]",i);
        printf("%s Path [%2u] %s\n",lprefix,i,pa);
        /*  First, open the file to determine if it exists.
            If not, loop again */
        res = dwarf_object_detector_path(pa,
            0,0,&ftype,&endian,&offsetsize,
            &filesize, &errcode);
        if (res == DW_DLV_NO_ENTRY) {
            printf("%s file above does not match/exist.\n",lprefix);
            continue;
        }
        if (res == DW_DLV_ERROR) {
            printf("%s file above access attempt "
                "lead to error %s\n",
                dwarf_errmsg_by_number(errcode),lprefix);
            continue;
        }
        realobj = print_ftype_message(lprefix, ftype);
        if (!realobj) {
            continue;
        }
        /*  Now see if the debug has buildid matching
            the executable. */
        if (is_outer && !no_follow_debuglink) {
            /*  read the executable, now look to the
                debug (ie pa) to see if it matches.
                Do not pass in globals paths*/
            res = one_file_debuglink_internal(
                FALSE,lprefix,0,0,0,
                pa,crc,buildid_length, buildid,pa);
            if (res == DW_DLV_OK) {
                printf("%s =====File %s is a correct"
                    " .debug object\n\n", lprefix,pa);
            }
        }
    }
    free(paths);
    free(debuglinkfullpath);
    dwarf_finish(dbg,&error);
    return DW_DLV_OK;
}

static void
one_file_debuglink(char *path,char **dlpaths,unsigned int dlcount,
   int no_follow_debuglink)
{
    one_file_debuglink_internal(TRUE,"",dlpaths,dlcount,
        no_follow_debuglink,
        path,0,0,0,0);
}

static char **gl_pathnames;
static unsigned int gl_pathcount;
static void add_a_path(char *path)
{
    unsigned count;
    char ** newpathnames = 0;
    unsigned int newslen = 0;
    unsigned int i = 0;

    if (!path) {
        printf("Null debuglink path error\n");
        exit(1);
    }
    newslen = strlen(path);
    if (!newslen){
        printf("Empty debuglink path ignored\n");
        return;
    }
    count = gl_pathcount + 1;
    newpathnames = (char **)malloc(sizeof(char *) *count);
    if (!newpathnames) {
        printf("Out of malloc space? giving up.\n");
        exit(1);
    }
    for (i = 0; i < gl_pathcount; ++i) {
        newpathnames[i] = gl_pathnames[i];
    }
    newpathnames[i] = strdup(path);
    if (!newpathnames[i]) {
        printf("Out of malloc space? giving up.\n");
        exit(1);
    }
    free(gl_pathnames);
    gl_pathcount = count;
    gl_pathnames = newpathnames;
}
static void free_paths(void)
{
    unsigned i = 0;
    if (!gl_pathcount) {
        return;
    }
    for(i = 0; i < gl_pathcount; ++i) {
        free(gl_pathnames[i]);
        gl_pathnames[i] = 0;
    }
    free(gl_pathnames);
    gl_pathnames = 0;
}

int
main(int argc, char **argv)
{
    int i = 1;
    char              *filenamein = 0;
    int no_follow_debuglink = FALSE;

    for ( ; i < argc; ++i) {
        char *arg = argv[i];
        if (!strncmp(arg,"--no-follow-debuglink",21)) {
            no_follow_debuglink = TRUE;
            continue;
        }
        if (!strncmp(arg,"--add-debuglink-path=",21)){
            add_a_path(arg+21);
            continue;
        }
        filenamein = arg;
        one_file_debuglink(filenamein,gl_pathnames,gl_pathcount,
            no_follow_debuglink);
        printf("=======done with %s\n",filenamein);
    }
    free_paths();
}
