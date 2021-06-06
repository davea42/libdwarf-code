/*  Copyright 2021 David Anderson 
    This trivial set of defines is hereby placed in the public domain
    for all to use.
*/
/* To enable printing with printf regardless of the
   actual underlying data type, we define the DW_PR_xxx macros.
   To ensure uses of DW_PR_DUx or DW_PR_DSx look the way you want
   ensure the DW_PR_XZEROS define as you want it.
*/
#ifndef LIBDWARF_PRIVATE_H
#define LIBDWARF_PRIVATE_H
#define DW_PR_XZEROS "08"

#ifdef _WIN32
#define DW_PR_DUx "I64x"
#define DW_PR_DSx "I64x"
#define DW_PR_DUu "I64u"
#define DW_PR_DSd "I64d"
#else
#define DW_PR_DUx "llx"
#define DW_PR_DSx "llx"
#define DW_PR_DUu "llu"
#define DW_PR_DSd "lld"
#endif /* DW_PR defines */

#ifdef HAVE_UNUSED_ATTRIBUTE
#define  UNUSEDARG __attribute__ ((unused))
#else
#define  UNUSEDARG
#endif

#ifndef TRUE
#define TRUE 1
#endif  /* TRUE */
#ifndef FALSE
#define FALSE 0
#endif  /* FALSE */

#define DWARF_HALF_SIZE 2

#endif /* LIBDWARF_PRIVATE_H */

