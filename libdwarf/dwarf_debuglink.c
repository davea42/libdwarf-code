/*
Copyright (c) 2019, David Anderson
All rights reserved.

Redistribution and use in source and binary forms, with
or without modification, are permitted provided that the
following conditions are met:

    Redistributions of source code must retain the above
    copyright notice, this list of conditions and the following
    disclaimer.

    Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  */

#include "config.h"
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* For free() and emergency abort() */
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#elif defined(_WIN32) && defined(_MSC_VER)
#include <io.h>
#endif /* HAVE_UNISTD_H */
#include <sys/types.h> /* for open() */
#include <sys/stat.h> /* for open() */
#include <fcntl.h> /* for open() */
#include "dwarf_incl.h"
#include "dwarf_error.h"
#include "dwarf_util.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif /* O_BINARY */

#define MINBUFLEN 1000
#define TRUE  1
#define FALSE 0

#if _WIN32
#define NULL_DEVICE_NAME "NUL"
#else
#define NULL_DEVICE_NAME "/dev/null"
#endif /* _WIN32 */


static int
does_file_exist(char *f)
{
    int fd = 0;

    fd = open(f,O_RDONLY|O_BINARY);
    if (fd < 0) {
        return DW_DLV_NO_ENTRY;
    }
    /* Here we could derive the crc to validate the file. */
    close(fd);
    return DW_DLV_OK;
}

struct joins_s {
    char * js_fullpath;
    char * js_dirname;
    size_t js_dirnamelen;
    char * js_wd;
    size_t js_wdlen;
    char * js_wd2;
    size_t js_wd2len;
    char * js_tname;
    char * js_originalfullpath;
    size_t js_originalfullpathlen;
};

static void
construct_js(struct joins_s * js)
{
    memset(js,0,sizeof(struct joins_s));
}
static void
destruct_js(struct joins_s * js)
{
    free(js->js_fullpath);
    js->js_fullpath = 0;

    free(js->js_dirname);
    js->js_dirname = 0;
    js->js_dirnamelen = 0;

    free(js->js_wd);
    js->js_wd = 0;
    js->js_wdlen = 0;

    free(js->js_wd2);
    js->js_wd2 = 0;
    js->js_wd2len = 0;

    free(js->js_tname);
    js->js_tname = 0;

    free(js->js_originalfullpath);
    js->js_originalfullpathlen = 0;
}

static char joinchar = '/';
static int
pathjoinl(char *target, size_t tsize,char *input)
{
    size_t targused = strlen(target);
    size_t inputsize = strlen(input);
    if (!input) {
        /* Nothing to do. Ok. */
        return DW_DLV_OK;
    }
    if ((targused+inputsize+3) > tsize) {
        return DW_DLV_ERROR;
    }

    if (!*target) {
        if (*input != joinchar) {
            target[0] = joinchar;
            strcpy(target+1,input);
        } else {
            strcpy(target,input);
        }
    }
    if (target[targused-1] != joinchar) {
        if (*input != joinchar) {
            target[targused] = joinchar;
            strcpy(target+targused+1,input);
        } else {
            strcpy(target+targused,input);
        }
    } else {
        if (*input != joinchar) {
            strcpy(target+targused,input);
        } else {
            strcpy(target+targused,input+1);
        }
    }
    return DW_DLV_OK;
}
/*  ASSERT: the last character in s is not a /  */
static size_t
mydirlen(char *s)
{
    char *cp = 0;
    char *lastjoinchar = 0;
    size_t count =0;

    for(cp = s ; *cp ; ++cp,++count)  {
        if (*cp == joinchar) {
            lastjoinchar = cp;
        }
    }
    if (lastjoinchar) {
        /* we know diff is postive in all cases */
        ptrdiff_t diff =  lastjoinchar - s;
        /* count the last join charn mydirlen. */
        return (size_t)(diff+1);
    }
    return 0;
}


/*  New September 2019.  Access to the GNU section named
    .gnu_debuglink
    See
    https://sourceware.org/gdb/onlinedocs/gdb/Separate-Debug-Files.html
*/
static void
construct_linkedto_path(Dwarf_Debug dbg,
   char * ptr,
   char ** debuglink_out,
   unsigned * debuglink_length)
{
    char * depath = (char *)dbg->de_path;
    size_t depathlen = strlen(depath)+1;
    size_t joinbaselen = 0;
    char * basename = 0;
    size_t basenamelen = 0;
    size_t wdretlen = 0;
    size_t buflen =2000;
    size_t maxlen = 0;
    char * tname = 0;
    int res = 0;
    struct joins_s joind;

    construct_js(&joind);
    joind.js_dirnamelen = mydirlen(depath);
    if (joind.js_dirnamelen) {
        joind.js_dirname = malloc(depathlen);
        if(!joind.js_dirname) {
            *debuglink_length = 0;
            destruct_js(&joind);
            return;
        }
        joind.js_dirname[0] = 0;
        strncpy(joind.js_dirname,depath,joind.js_dirnamelen);
        joind.js_dirname[joind.js_dirnamelen] = 0;
    }
    basename = ptr;
    basenamelen = strlen(basename);

    joind.js_wdlen = buflen + depathlen + basenamelen +100;
    joind.js_wd = malloc(joind.js_wdlen);
    if(!joind.js_wd) {
        *debuglink_length = 0;
        destruct_js(&joind);
        return;
    }
    joind.js_wd[0] = 0;
    if (depath[0] != joinchar) {
        char *wdret = 0;
        wdret = getcwd(joind.js_wd,joind.js_wdlen);
        if (!wdret) {
            destruct_js(&joind);
            *debuglink_length = 0;
            return;
        }
        wdretlen = strlen(joind.js_wd);
    }
    /* We know the default places plus slashes won't add 100 */
    maxlen = wdretlen + depathlen +100 + basenamelen;
    if ((maxlen > joind.js_wdlen) && wdretlen) {
        char *newwd = 0;

        newwd = malloc(maxlen);
        if(!newwd) {
            destruct_js(&joind);
            *debuglink_length = 0;
            return;
        }
        newwd[0] = 0;
        strcpy(newwd,joind.js_wd);
        free(joind.js_wd);
        joind.js_wd = newwd;
        joind.js_wdlen = maxlen;
    }
    {
        char *opath = 0;

        opath = malloc(maxlen);
        if(!opath) {
            destruct_js(&joind);
            *debuglink_length = 0;
            return;
        }
        opath[0] = 0;
        joind.js_originalfullpath = opath;
        joind.js_originalfullpathlen = maxlen;
        opath = 0;
        strcpy(joind.js_originalfullpath,joind.js_wd);
        res =  pathjoinl(joind.js_originalfullpath,joind.js_originalfullpathlen,
            depath);
        if (res != DW_DLV_OK) {
            destruct_js(&joind);
            *debuglink_length = 0;
            return;
        }
    }
    /*  We need to be sure there is no accidental match with the file we opened. */
    /* wd suffices to build strings. */
    if (joind.js_dirname) {
        res = pathjoinl(joind.js_wd,joind.js_wdlen,
            joind.js_dirname);
    } else {
        res = DW_DLV_OK;
    }
    /* Now js_wd is a leading / directory name. */
    joinbaselen = strlen(joind.js_wd);
    if (res == DW_DLV_OK) {
        /* If we add basename do we find what we look for? */
        res = pathjoinl(joind.js_wd,joind.js_wdlen,basename);
        if (!strcmp(joind.js_originalfullpath,joind.js_wd)) {
            /* duplicated name. spurious match. */
        } else if (res == DW_DLV_OK) {
            res = does_file_exist(joind.js_wd);
            if (res == DW_DLV_OK) {
                *debuglink_out = joind.js_wd;
                *debuglink_length = strlen(joind.js_wd);
                /* ownership passed to caller, then free the rest */
                joind.js_wd = 0;
                joind.js_wdlen = 0;
                destruct_js(&joind);
                return;
            }
        }
    }
    /* No, so remove the basename */
    joind.js_wd[joinbaselen] = 0;
    tname = ".debug";
    res = pathjoinl(joind.js_wd,joind.js_wdlen,tname);
    if (res == DW_DLV_OK) {
        res = pathjoinl(joind.js_wd,joind.js_wdlen,basename);
        if (!strcmp(joind.js_originalfullpath,joind.js_wd)) {
            /* duplicated name. spurious match. */
        } else if(res == DW_DLV_OK) {
            res = does_file_exist(joind.js_wd);
            if (res == DW_DLV_OK) {
                *debuglink_out = joind.js_wd;
                *debuglink_length = strlen(joind.js_wd);
                /* ownership passed to caller, then free the rest */
                joind.js_wd = 0;
                joind.js_wdlen = 0;
                destruct_js(&joind);
                return;
            }
        }
    }
    /* Not found, so remove the .debug etc */
    joind.js_wd[joinbaselen] = 0;

    tname = "/usr/lib/debug";
    joind.js_wd2 = malloc(maxlen);
    if(!joind.js_wd2) {
        *debuglink_length = 0;
        destruct_js(&joind);
        return;
    }
    joind.js_wd2len = maxlen;
    joind.js_wd2[0] = 0;
    strcpy(joind.js_wd2,tname);
    res = pathjoinl(joind.js_wd2,joind.js_wd2len,joind.js_wd);
    if (res == DW_DLV_OK) {
        res = pathjoinl(joind.js_wd2,joind.js_wd2len,basename);
        if (!strcmp(joind.js_originalfullpath,joind.js_wd2)) {
            /* duplicated name. spurious match. */
        } else if (res == DW_DLV_OK) {
            res = does_file_exist(joind.js_wd2);
            if (res == DW_DLV_OK) {
                *debuglink_out = joind.js_wd2;
                *debuglink_length = strlen(joind.js_wd2);
                /* ownership passed to caller, then free the rest */
                joind.js_wd2 = 0;
                joind.js_wd2len = 0;
                destruct_js(&joind);
                return;
            }
        }
    }
    *debuglink_length = 0;
    destruct_js(&joind);
    return;
}

int
dwarf_gnu_debuglink(Dwarf_Debug dbg,
    char ** name_returned,  /* static storage, do not free */
    char ** crc_returned, /* 32bit crc , do not free */
    char **  debuglink_path_returned, /* caller must free returned pointer */
    unsigned *debuglink_path_size_returned,/* Size of the debuglink path.
        zero returned if no path known/found. */
    Dwarf_Error*   error)
{
    char *ptr = 0;
    char *endptr = 0;
    unsigned namelen = 0;
    unsigned m = 0;
    unsigned incr = 0;
    char *crcptr = 0;
    int res = DW_DLV_ERROR;

    if (!dbg->de_gnu_debuglink.dss_data) {
        res = _dwarf_load_section(dbg,
            &dbg->de_gnu_debuglink,error);
        if (res != DW_DLV_OK) {
            return res;
        }
    }
    ptr = (char *)dbg->de_gnu_debuglink.dss_data;
    endptr = ptr + dbg->de_gnu_debuglink.dss_size;
    res = _dwarf_check_string_valid(dbg,ptr,
        ptr,
        endptr,
        DW_DLE_FORM_STRING_BAD_STRING,
        error);
    if (res != DW_DLV_OK) {
        return res;
    }
    namelen = (unsigned)strlen((const char*)ptr);
    m = (namelen+1) %4;
    if (m) {
        incr = 4 - m;
    }
    crcptr = ptr +namelen +1 +incr;
    if ((crcptr +4) != endptr) {
        _dwarf_error(dbg,error,DW_DLE_CORRUPT_GNU_DEBUGLINK);
        return DW_DLV_ERROR;
    }
    if (dbg->de_path) {
        construct_linkedto_path(dbg,ptr,debuglink_path_returned,
            debuglink_path_size_returned);
    } else {
        *debuglink_path_size_returned  = 0;
    }
    *name_returned = ptr;
    *crc_returned = crcptr;
    return DW_DLV_OK;
}

/* New September 2019. */
int  dwarf_add_file_path(
    UNUSEDARG Dwarf_Debug dbg,
    const char *          file_name,
    UNUSEDARG Dwarf_Error* error)
{
    if (!dbg->de_path) {
        dbg->de_path = strdup(file_name);
    }
    return DW_DLV_OK;
}


/*  The definition of .note.gnu.buildid contents (also
    used for other GNU .note.gnu.  sections too. */
struct buildid_s {
    char bu_ownernamesize[4];
    char bu_buildidsize[4];
    char bu_type[4];
    char bu_owner[1];
};

int
dwarf_gnu_buildid(Dwarf_Debug dbg,
    Dwarf_Unsigned * type_returned,
    const char     **owner_name_returned,
    Dwarf_Unsigned * build_id_length_returned,
    const unsigned char  **build_id_returned,
    Dwarf_Error*   error)
{
    Dwarf_Byte_Ptr ptr = 0;
    Dwarf_Byte_Ptr endptr = 0;
    int res = DW_DLV_ERROR;
    struct buildid_s *bu = 0;
    Dwarf_Unsigned namesize = 0;
    Dwarf_Unsigned descrsize = 0;
    Dwarf_Unsigned type = 0;

    if (!dbg->de_note_gnu_buildid.dss_data) {
        res = _dwarf_load_section(dbg,
            &dbg->de_note_gnu_buildid,error);
        if (res != DW_DLV_OK) {
            return res;
        }
    }
    ptr = (Dwarf_Byte_Ptr)dbg->de_note_gnu_buildid.dss_data;
    endptr = ptr + dbg->de_note_gnu_buildid.dss_size;

    if (dbg->de_note_gnu_buildid.dss_size <
        sizeof(struct buildid_s)) {
        _dwarf_error(dbg,error,
            DW_DLE_CORRUPT_NOTE_GNU_DEBUGID);
        return DW_DLV_ERROR;
    }

    bu = (struct buildid_s *)ptr;
    READ_UNALIGNED_CK(dbg,namesize,Dwarf_Unsigned,
        (Dwarf_Byte_Ptr)&bu->bu_ownernamesize[0], 4,
        error,endptr);
    READ_UNALIGNED_CK(dbg,descrsize,Dwarf_Unsigned,
        (Dwarf_Byte_Ptr)&bu->bu_buildidsize[0], 4,
        error,endptr);
    READ_UNALIGNED_CK(dbg,type,Dwarf_Unsigned,
        (Dwarf_Byte_Ptr)&bu->bu_type[0], 4,
        error,endptr);

    if (descrsize != 20) {
        _dwarf_error(dbg,error,DW_DLE_CORRUPT_NOTE_GNU_DEBUGID);
        return DW_DLV_ERROR;
    }
    res = _dwarf_check_string_valid(dbg,&bu->bu_owner[0],
        &bu->bu_owner[0],
        endptr,
        DW_DLE_CORRUPT_GNU_DEBUGID_STRING,
        error);
    if ( res != DW_DLV_OK) {
        return res;
    }
    if ((strlen(bu->bu_owner) +1) != namesize) {
        _dwarf_error(dbg,error,DW_DLE_CORRUPT_GNU_DEBUGID_STRING);
        return DW_DLV_ERROR;
    }

    if ((sizeof(struct buildid_s)-1 + namesize + descrsize) >
        dbg->de_note_gnu_buildid.dss_size) {
        _dwarf_error(dbg,error,DW_DLE_CORRUPT_GNU_DEBUGID_SIZE);
        return DW_DLV_ERROR;
    }
    *type_returned = type;
    *owner_name_returned = &bu->bu_owner[0];
    *build_id_length_returned = descrsize;
    *build_id_returned = ptr + sizeof(struct buildid_s)-1 + namesize;
    return DW_DLV_OK;
}
