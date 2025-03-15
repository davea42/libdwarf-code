# Libdwarf Internals Introduction
The library is organized into a
conceptual LeftHandSide (LHS, DWARF related) and a conceptual
RightHandSide (RHS,object file related).
All the functions in the API
are functions in the LHS, and the LHS
knows nothing about the actual object
file.

The RHS provides a small number
of function pointers and the LHS
uses those functions to access any
object file being read.
The RHS knows nothing about the
functions in the LHS.

There are three RHS code blocks.
An Elf, a PE, and a MacOS block.
The structure of simple Elf objects is 
reflected in the function pointers each
of the three provide.
The PE and MacOS blocks hide the differences from Elf
from the LHS.

# Libdwarf Internals Overview
# Initialization

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

# Reading DWARF via the API.

The API functions work on any object, regardless
of ABI and adjust for endianness.

# dwarf_finish()

Finish by calling dwarf_finish(), releasing all
memory allocations and making any pointers to libdwarf
data in your
application memory stale and unusable.

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

