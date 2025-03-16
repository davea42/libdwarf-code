# Libdwarf Internals Introduction

Updated: 16 March 2025

The library is organized into a
conceptual LeftHandSide (LHS, DWARF related) and a conceptual
RightHandSide (RHS,object file related).
All the functions in the API
are functions in the LHS, and the LHS
knows nothing about the actual object
file.

All the source files for LHS and RHS  are in src/lib/libdwarf.

RHS source to read Elf:dwarf_elf_access.h dwarf_elfread.h
dwarf_elf_defines.h dwarf_elf_rel_detector.c
dwarf_elf_load_headers.c dwarf_elf_rel_detector.h
dwarf_elfread.c dwarf_elfstructs.h

RHS source to read PE: dwarf_pe_descr.h dwarf_peread.c
dwarf_peread.h

RHS source to read MacOS objects: dwarf_macho_loader.h 
dwarf_machoread.c dwarf_machoread.h

The RHS provides a small number
of function pointers and the LHS
uses those functions to access any
object file being read.
The RHS knows nothing about the
functions in the LHS.

There are three RHS code blocks.
An Elf, a PE, and a MacOS block
(reflected in header and source names in libdwarf).
The structure of simple Elf objects is 
reflected in the function pointers each
of the three provide.
The PE and MacOS blocks hide the differences from Elf
from the LHS.

All the code blocks are available at runtime, there
is no selecting at library-build time. 

# Libdwarf Internals Overview
## Initialization

Any one of the functions

  dwarf_init_path(),
  dwarf_init_path_a(),
  dwarf_init_path_dl(),
  dwarf_init_path_dl_a(), or
  dwarf_init_b() may be used.

dwarf_init_path_dl_a() is the most general, but
for most objects dwarf_init_path()
suffices. Other flavors allow more control
in looking to find an object file containing
actual DWARF when given a path to an object
without DWARF data (compilers provide hints
to libdwarf to help find the DWARF-containing object).

When initialization is complete both the LHS DWARF
data and the RHS object-specific data have arrays
of sections.  These are defined as, and must be,
one-to-one identical in terms of numbering so
index B in one and index B in the other refer to
the same actual object section. This is crucial
as section index is what connects LHS data
and RHS data.

## Reading DWARF via the API.

The API functions work on any object, regardless
of ABI and all functions account for endianness.

## dwarf_finish()

Finish by calling dwarf_finish(), releasing all
memory allocations and making any pointers to libdwarf
data in your
application memory stale and unusable.

Before release 0.12.0 the functions

  _dwarf_destruct_elf_nlaccess(dbg->de_obj_file),
  _dwarf_destruct_macho_access(dbg->de_obj_file),
  _dwarf_destruct_pe_access(dbg->de_obj_file).
 
were visible to the entire library, but as of 0.12.0
there is a new function pointer in Dwarf_Obj_Access_Methods_a:
om_finish(), and those three are declared static, invisible
to the LHS.

# The Special Case

If there is no object file to read (such as with
just-in-time DWARF creation)  
an extra initialization call (dwarf_obj_init())
lets one invent RHS code that behaves as if
there was a file and present the DWARF data
to the LHS in the same was as for the
standard object files.

Initialize by calling dwarf_object_init_b().

Finish by calling dwarf_object_finish().

See src/bin/dwarfexample/jitreader.c  

