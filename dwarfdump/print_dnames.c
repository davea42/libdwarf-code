/*
  Copyright 2017-2017 David Anderson. All rights reserved.

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

#include "globals.h"
#include "naming.h"
#include "dwconf.h"
#include "esb.h"

extern void
print_debug_names(Dwarf_Debug dbg)
{

    if(!dbg) {
        printf("Cannot print .debug_names, no Dwarf_Debug passed in");
        printf("dwarfdump giving up. exit.\n");
        exit(1);
    }
    current_section_id = DEBUG_NAMES;

    /* Do nothing if not printing. */
    if (!glflags.gf_do_print_dwarf) {
        return;
    }
    /*  Only print anything if we know it has debug names
        present. And for now there is none. FIXME. */
}
