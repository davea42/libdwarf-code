/*
Copyright (c) 2020, David Anderson
All rights reserved.

This software file is hereby placed in the public domain.
For use by anyone for any purpose.
*/

#include <stdio.h>  /* FILE fclose() fopen() fprintf() printf() */
#include <stdlib.h> /* exit() getenv() */
#include <string.h> /* strcmp() strcpy() strncmp() strlen() */

#include "dwarf.h"
#include "dwarf_lname_data.h"

#define MAXDEFINELINE 1000
static char *input_name = 0;
static char pathbuf[BUFSIZ];
static char buffer[BUFSIZ];

#define FALSE 0
#define TRUE  1

static unsigned     tcount = sizeof(lnsix) / sizeof(lnsix[0]);
static unsigned int touchcounts[sizeof(lnsix) / sizeof(lnsix[0])];

static void
safe_strcpy(char *targ,char *src,unsigned targlen, unsigned srclen)
{
    if (srclen > targlen) {
        printf("Target name does not fit in buffer.\n"
            "In buildopstabcount.c increase buffer size "
            " from %u \n",(unsigned int)sizeof(buffer));
        exit(EXIT_FAILURE);
    }
    strcpy(targ,src);
}

static void
validatetouchcount(char *curdefname,unsigned long v)
{
     if (v >= tcount) {
         printf("ERROR: For LNAME id 0x%04lx  there is "
             "No count available, limit is %x. Something wrong!\n",
             v,tcount);
         exit(EXIT_FAILURE);
     }
     if (touchcounts[v]) {
         printf("ERROR: For LNAME id 0x%04lx  we are double "
             "counting at count 0x%x . Something wrong!\n",
             v,touchcounts[v]);
         exit(EXIT_FAILURE);
     }
     ++touchcounts[v];
     if (lnsix[v].ln_value !=  v) {
         printf("ERROR: For LNAME id 0x%04lx  not at the "
             "correct entry vs "
             " ln_value 0x%04x\n",
             v,lnsix[v].ln_value);
         exit(EXIT_FAILURE);
     }
     if (strcmp(curdefname,lnsix[v].ln_name)) {
         printf("ERROR: For LNAME id 0x%04lx we find the"
             " wrong string! name %s lnsixname %s \n",
             v, curdefname,lnsix[v].ln_name);
         exit(EXIT_FAILURE);
     }
}
static void
validatefinaltouchcount()
{
     unsigned long i = 1;
     /*  Skip [0], it is not real, just a placeholder.
         0 is not a valid LNAME id */
     for( ; i < tcount; ++i) {
         if (touchcounts[i] != 1) {
             printf(" ERROR: an entry in the LNAMES list "
                 " with DW_LNAME id 0x%04lx has touchcount"
                 " %u which indicates an error comparing "
                 " dwarf.h and the lnsix table here.\n",
                 i,touchcounts[i]);
            exit(EXIT_FAILURE);
         }
     }
}


/*  Writes a complete new function 
    int dwarf_language_version_data(..)
    in
    dwarf_lname_version.c

    Replacing what was in dwarf_query.c for
    that function.

    We avoid anything in the new source file
    representing date or time as we want 
    re-running this (if no changes in dwarf_lname_data.h)
    outputs bit-for-bit identical dwarf_lname_version.c .
*/


static void
write_new_query_function(char *path)
{
    
    printf("Query function not complete. %s\n",path);
}
/*  Issue error message and exit there is a mismatch,
    this should never fail.  
    It verifies that dwarf.h DW_LNAME values match
    exactly the LNAME values in dwarf_lname_data.h*/
static void
check_if_lname_complete(char *path)
{
    FILE         *fin = 0;
    unsigned int  linenum = 1;
    int           loop_done = FALSE;
    const char   *oldop = "#define DW_LNAME_";
    int           oldoplen = strlen(oldop);
    int           langentry = -1;
    unsigned long lastvalue = 0;

    fin = fopen(path,"r");
    if (!fin) {
        printf("Unable to open dwarf.h to read %s\n",path);
        exit(EXIT_FAILURE);
    }
    for ( ;!loop_done;++linenum) {
        char *line = 0;
        char * pastname  = 0;
        unsigned int linelen = 0;
        char *curdefname = 0;
        unsigned int curdefnamelen = 0;
        char *endptr     = 0;
        char *numstart   = 0;
        unsigned long v  = 0;

        line = fgets(buffer,MAXDEFINELINE,fin);
        if (!line) {
            break;
        }
        linelen = strlen(line);
        line[linelen-1] = 0;
        --linelen;
        if (linelen >= (unsigned)(MAXDEFINELINE-1)) {
            printf("define line %u is too long!\n",linenum);
            exit(EXIT_FAILURE);
        }
        if (strncmp(line,oldop,oldoplen)) {
            /* Not ours. */
            continue;
        }
        ++langentry;
        /* ASSERT: line ends with NUL byte. */
        curdefname = line+8; /* now past #define */
        for ( ; ; curdefnamelen++) {
            pastname = curdefname +curdefnamelen;
            if (!*pastname) {
                /* At end of line. Missing value. */
                printf("define line %u of %s: has no number value!\n",
                    linenum,path);
                exit(EXIT_FAILURE);
            }
            if (*pastname == ' ') {
                /* Ok. Now look for value. */
                numstart = pastname + 1;
                break;
            }
        }
        *pastname = 0; /* so curdefname points to only the name */
        /*  Skip spaces. */
        for ( ; *numstart == ' '; ++numstart) { }
        endptr = 0;
        v = strtoul(numstart,&endptr,0);
        if (v == 0 && endptr == numstart) {
            printf("define line %u of %s: number value missing.\n",
                linenum,path);
            printf("Leaving a space as in #define A B 3"
                " in dwarf.h.in will cause this.\n");
            exit(EXIT_FAILURE);
        }
        if (*endptr != ' ' && *endptr != 0) {
            unsigned char e = *endptr;
            printf("define line %u: number value terminates oddly "
                "char: %u 0x%x line %s\n",
                linenum,e,e,line);
            exit(EXIT_FAILURE);
        }
        if (!v) {
            printf("define line %u: DW_LANG number value "
                "zero unreasonable.\n",
                linenum);
            exit(EXIT_FAILURE);
        }
        /*  v is the DW_LNAM value, 0x0001 for DW_LNAME_Ada */
        if (!lastvalue) {
            /* Nothing to check yet. Is first DW_LNAME. */
            lastvalue = v;
            validatetouchcount(curdefname,v);
            continue;
        } 
        if (lastvalue == v) {
            printf("define line %u: DW_LNAME number value "
                " 0x%lx duplicated.\n",
                linenum,lastvalue);
            exit(EXIT_FAILURE);
        }
        if (v != (lastvalue +1)) {
            printf("define line %u: DW_LNAME number "
                " last value  0x%lx after "
                " expected 0x%lx, not 0x%lx",
                linenum,lastvalue,lastvalue+1,v);
            exit(EXIT_FAILURE);
        }
        validatetouchcount(curdefname,v);
        lastvalue = v;
    }
    validatefinaltouchcount();
    fclose(fin);
}

int main(int argc, char**argv)
{
    const char *tailpath = "/src/lib/libdwarf/dwarf.h";
    char  *path  = 0;
    unsigned len = 0;

    if (argc > 1) {
        if (argc != 3) {
            printf("Expected -f <filename> of base code path\n");
            exit(EXIT_FAILURE);
        }
        if (strcmp(argv[1],"-f")) {
            printf("Expected -f\n");
            exit(EXIT_FAILURE);
        }
        path=argv[2];
    } else {
        /* env var should be set with base path of code */
        path = getenv("DWTOPSRCDIR");
        if (!path) {
            printf("Expected environment variable "
                "DWTOPSRCDIR with path of "
                "base directory (usually called 'code')\n");
            exit(EXIT_FAILURE);
        }
    }
    len = strlen(path);
    if (len >= sizeof(pathbuf)) {
        printf(" buildopstab Input path greater length "
            "than makes any sense:"
            " Giving up\n");
        exit(EXIT_FAILURE);

    }
    safe_strcpy(pathbuf,path,sizeof(pathbuf),len);
    {
        size_t remaining =  sizeof(pathbuf) -len -1;
        size_t tailpathlen = strlen(tailpath);
        if (tailpathlen >= remaining) {
            printf(" buildopstab Input tailpath greater "
                "length fits in buf: "
                "Giving up\n");
            exit(EXIT_FAILURE);
        }
        /* Notice tailpath has a leading /  */
        safe_strcpy(pathbuf+len,(char *)tailpath,
            remaining,tailpathlen);
    }
    input_name = pathbuf;

    check_if_lname_complete(pathbuf);
    write_new_query_function(pathbuf);

    return 0;
}
