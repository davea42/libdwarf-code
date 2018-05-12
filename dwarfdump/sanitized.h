/*
  Copyright 2011-2018 David Anderson. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 51
  Franklin Street - Fifth Floor, Boston MA 02110-1301, USA.
*/

#ifndef SANITIZED_H
#define SANITIZED_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*  Eliminate control characters from the input,
    leaving the input unchanged. Return pointer to
    an ephemeral location (only callfor printf,
    and only once per printf! */
const char * sanitized(const char *s);
void sanitized_string_destructor(void);

/* Set TRUE if you want to simply assume strings to be
   printed are safe to print. Leave FALSE if you want
   dangerous or unprintable characters to be switched to the
   character '?'. */
extern boolean no_sanitize_string_garbage;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* SANITIZED_H */
