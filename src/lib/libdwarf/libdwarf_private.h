/*
  Copyright (C) 2000-2010 Silicon Graphics, Inc.  All Rights Reserved.
  Portions Copyright 2007-2010 Sun Microsystems, Inc. All rights reserved.
  Portions Copyright 2008-2018 David Anderson. All rights reserved.
  Portions Copyright 2008-2010 Arxan Technologies, Inc. All rights reserved.
  Portions Copyright 2010-2012 SN Systems Ltd. All rights reserved.

  This program is free software; you can redistribute it
  and/or modify it under the terms of version 2.1 of the
  GNU Lesser General Public License as published by the Free
  Software Foundation.

  This program is distributed in the hope that it would be
  useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  Further, this software is distributed without any warranty
  that it is free of the rightful claim of any third person
  regarding infringement or the like.  Any license provided
  herein, whether implied or otherwise, applies only to this
  software file.  Patent licenses, if any, provided herein
  do not apply to combinations of this program with other
  software, or any other product whatsoever.

  You should have received a copy of the GNU Lesser General
  Public License along with this program; if not, write the
  Free Software Foundation, Inc., 51 Franklin Street - Fifth
  Floor, Boston MA 02110-1301, USA.

*/
#ifndef LIBDWARF_PRIVATE_H
#define LIBDWARF_PRIVATE_H

/* To enable printing with printf regardless of the
   actual underlying data type, we define the DW_PR_xxx macros.
   To ensure uses of DW_PR_DUx or DW_PR_DSx look the way you want
   ensure the right DW_PR_XZEROS define is uncommented.
*/
/*#define DW_PR_XZEROS "" */
#define DW_PR_XZEROS "08"

#if defined(_WIN32) && defined(HAVE_NONSTANDARD_PRINTF_64_FORMAT)
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

#endif /* LIBDWARF_PRIVATE_H */
