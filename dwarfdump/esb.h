/*
    Copyright (C) 2005 Silicon Graphics, Inc.  All Rights Reserved.
    Portions Copyright 2011 David Anderson. All Rights Reserved.
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
  
    Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
    Mountain View, CA 94043, or:
  
    http://www.sgi.com
  
    For further information regarding this notice, see:
  
    http://oss.sgi.com/projects/GenInfo/NoticeExplan
  

    $Header: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/dwarfdump/RCS/esb.h,v 1.1 2005/08/04 05:09:37 davea Exp $ */


/*  esb.h
    Extensible string buffer.
    A simple vaguely  object oriented extensible string buffer.

    The struct could be opaque here, but it seems ok to expose
    the contents: simplifies debugging.
*/


struct esb_s {
    string  esb_string; /* pointer to the data itself, or  NULL. */
    size_t  esb_allocated_size; /* Size of allocated data or 0 */
    size_t  esb_used_bytes; /* Amount of space used  or 0 */
};

/* string length taken from string itself. */
void esb_append(struct esb_s *data, const char * in_string);

/* The 'len' is believed. Do not pass in strings < len bytes long. */
void esb_appendn(struct esb_s *data, const char * in_string, size_t len);

/* Always returns an empty string or a non-empty string. Never 0. */
string esb_get_string(struct esb_s *data);


/* Sets esb_used_bytes to zero. The string is not freed and
   esb_allocated_size is unchanged.  */
void esb_empty_string(struct esb_s *data);


/* Return esb_used_bytes. */
size_t esb_string_len(struct esb_s *data);

/* The following are for testing esb, not use by dwarfdump. */

/* *data is presumed to contain garbage, not values, and
   is properly initialized. */
void esb_constructor(struct esb_s *data);

/*  The string is freed, contents of *data set to zeroes. */
void esb_destructor(struct esb_s *data);


/* To get all paths in the code tested, this sets the
   allocation/reallocation to the given value, which can be quite small
   but must not be zero. */
void esb_alloc_size(size_t size);
size_t esb_get_allocated_size(struct esb_s *data);

/* Append a formatted string */
void esb_append_printf(struct esb_s *data,const char *in_string, ...);
