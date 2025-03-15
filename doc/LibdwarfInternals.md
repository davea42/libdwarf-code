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
# Reading DWARF via the API.
# dwarf_finish()

# The Special Case

If there is no object file to read (such as with
just-in-time DWARF creation)  
an extra initialization call (dwarf_obj_init())
lets one invent RHS code that behaves as if
there was a file and present the DWARF data
to the LHS in the same was as for the
standard object files.

See src/bin/dwarfexample/jitreader.c  

