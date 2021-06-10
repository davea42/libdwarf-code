/*  baseconfig.h.  Maintained by hand for buildstandardsource.sh
    This avoids the necessity for running configure but
    makes assumptions (such as the existence of libz)
    that are not tested.  So you may need to
    alter this to make buildstandardsource.sh work
    if your environment is sufficiently different.
    Assuming you have a reason to run
    scripts/buildstandardsource.sh  (you most likely
    have no reason).
*/


/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/*  Define to one of `_getb67', `GETB67', `getb67' for
    Cray-2 and Cray-YMP
    systems. This function is required for `alloca.c'
    support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define to 1 if using `alloca.c'. */
/* #undef C_ALLOCA */

/* Define to 1 if you have `alloca', as a function or macro. */
#undef HAVE_ALLOCA

/* Only defined here, done for the benefit if
   buildstandardsource.sh */
#define BUILD_STANDARD_SOURCE 1

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#undef HAVE_ALLOCA_H 

/* Define to 1 if you have the <dlfcn.h> header file. */
#undef HAVE_DLFCN_H 

/*  Fake. For buildstandardsource.sh */
#define PACKAGE_VERSION "20190101"

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Set to 1 if old frame columns are enabled. */
/* #undef HAVE_OLD_FRAME_CFA_COL */

/* Define to 1 if you have the <sgidefs.h> header file. */
/* #undef HAVE_SGIDEFS_H */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Set to 1 if __attribute__ ((unused)) is available. */
#define HAVE_UNUSED_ATTRIBUTE 1

/* Set to 1 if zlib decompression is available. */
#undef HAVE_ZLIB 

/* Define to 1 if you have the <zlib.h> header file. */
#undef HAVE_ZLIB_H

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */
