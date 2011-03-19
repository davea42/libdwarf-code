'\"#ident	"%W%"
'\" $Source: /plroot/cmplrs.src/v7.4.5m/.RCS/PL/libdwarf/RCS/dwarf.v2.mm,v $
'\"
'\" $Revision: 1.2 $
'\"
'\" DESCRIPTION
'\"
'\"	Requirements for 
'\"
'\" COMPILATION
'\"
'\"	pic file.mm | tbl | troff -mm
'\"
'\"	local mileage may vary
'\"
'\" AUTHOR
'\"
'\"	UNIX International Programming Languages SIG
'\"
'\" COPYRIGHT
'\"
'\"	Copyright (c) 1992,1993, UNIX International
'\"
'\"	Permission to use, copy, modify, and distribute this documentation for
'\"	any purpose and without fee is hereby granted, provided that the above
'\"	copyright notice appears in all copies and that both that copyright
'\"	notice and this permission notice appear in supporting documentation,
'\"	and that the name UNIX International not be used in advertising or
'\"	publicity pertaining to distribution of the software without specific,
'\"	written prior permission.  UNIX International makes no representations
'\"	about the suitability of this documentation for any purpose.  It is
'\"	provided "as is" without express or implied warranty.
'\"	
'\"	UNIX INTERNATIONAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
'\"	DOCUMENTATION, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
'\"	FITNESS.  IN NO EVENT SHALL UNIX INTERNATIONAL BE LIABLE FOR ANY
'\"	SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
'\"	RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
'\"	CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
'\"	CONNECTION WITH THE USE OR PERFORMANCE OF THIS DOCUMENTATION.
'\"	
'\"	NOTICE:
'\"	
'\"	UNIX International is making this documentation available as a
'\"	reference point for the industry.  While UNIX International believes
'\"	that this specification is well defined in this first release of the
'\"	document, minor changes may be made prior to products meeting this
'\"	specification being made available from UNIX System Laboratories or 
'\"	UNIX International members.
'\"
'\" $Log$
'\" Revision 1.1  1994/05/18 18:50:42  davea
'\" Initial revision
'\"
'\"
'\"     Abbrevs for funny typeset words
.pl-0.25i
.ds aX U\s-2NIX\s+2
.ds iX \*(aX International
.ds uL \s-2AT&T\ USL\s+2
'\"
'\"  uI should be set to 1 if the publication and copyright page is needed.
.nr uI 1
'\"
'\"     Make the appropriate replacements in this section!
'\"
'\"     Set the ND date to the current date.
'\"     tT is the formal document title
'\"     tP is the name of the Project (if appropriate)
'\"     tD is the short document title
'\"     tE is the work group name (may be the same as the project name)
.ds tT DWARF Debugging Information Format 
.ds tP 
'\"             Document name (i.e., without project name)
.ds tD DWARF Debugging Information Format
.ds tE Programming Languages SIG
'\"
'\"     Define headers and footers macro
'\"
.ds fA Revision: 2.0.0
'\"
'\"     fB null to remove page numbers on cover page
.ds fB
.ds fC July 27, 1993
.ds fE Industry Review Draft
.ds fF \*(tD
.PH "''''"
.PF "''\*(fE''"
.tr ~
.SA 1
.S 10
.nr Ej 1
.nr Hs 5
.nr Hu 1
.nr Hb 5
.ds HP +2 +2 +1 +0 +0 +0 +0
.ds HF 3 3 3 3 3 1 1
.if n .ds HF 1 1 1 1 1 1 1 1
'\"
'\"     First page, print title and authors
'\"
.S +4
.DS C






\fB\*(tT

\s-2\*(tP\s+2\fP

.DE
.S
.sp 3i
\*(iX
.br
\*(tE
.br
\*(fA (\*(fC)
.SK
.if \n(uI\{ 
.DS C
.in -.25i
.B "Published by:"
.R

\*(iX
Waterview Corporate Center
20 Waterview Boulevard
Parsippany, NJ  07054

for further information, contact:
Vice President of Marketing

Phone:	+1 201-263-8400
Fax:	+1 201-263-8401
.DE
.P
Copyright \(co 1992, 1993 \*(iX, Inc.
.P
Permission to use, copy, modify, and distribute this
documentation for any purpose and without fee is hereby granted, provided
that the above copyright notice appears in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name \*(iX not be used in 
advertising or publicity pertaining to distribution of the software 
without specific, written prior permission.  \*(iX makes
no representations about the suitability of this documentation for any 
purpose.  It is provided "as is" without express or implied warranty.
.P
UNIX INTERNATIONAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS DOCUMENTATION, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO 
EVENT SHALL UNIX INTERNATIONAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR 
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF 
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
PERFORMANCE OF THIS DOCUMENTATION.
.sp 2l
.if \n(uI\{
NOTICE:
.P
\*(iX is making this documentation available as a
reference point for the industry.  
While \*(iX believes that this specification is well
defined in this first release of the document,
minor changes may be made prior to products meeting this specification
being made available from \*(aX System Laboratories or \*(iX members.
.sp 1l \}
Trademarks:
.P
Intel386 is a trademark of Intel Corporation.
.br
\*(aX\(rg is a registered trademark of \*(aX System Laboratories 
in the United States and other countries.
.br
.OH "'''\s10\\\\*(tE\s0'"
.EH "'\s10\\\\*(tD\s0'''"
.SK
'\".VM 0 2
.PF "''\s10\\\\*(fE\s0''"
.OF "'\s10\\\\*(fA'\\\\*(fB'\\\\*(fC\s0'"
.EF "'\s10\\\\*(fA'\\\\*(fB'\\\\*(fC\s0'"
'\" -----------------------------------------------------------------------
'\".
'\"     Reset page numbers
'\"
.nr P 1
.nr % 1
'\"
'\"     Define headers and footers
'\"
.FH
'\"     Turn on the page numbering in the footers
.ds fB Page %
'\"
'\"     MACROEND
'\"
.if n .fp 2 R
.if n .fp 3 R
.tr ~  
\fR
.S 11
.SA 1
.tr ~
.OP
.ds | |
.ds ~ ~
.ds ' '
.if t .ds Cw \&\f(CW
.if n .ds Cw \fB
.de Cf		\" Place every other arg in Cw font, beginning with first
.if \\n(.$=1 \&\*(Cw\\$1\fP
.if \\n(.$=2 \&\*(Cw\\$1\fP\\$2
.if \\n(.$=3 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP
.if \\n(.$=4 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4
.if \\n(.$=5 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4\*(Cw\\$5\fP
.if \\n(.$=6 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4\*(Cw\\$5\fP\\$6
.if \\n(.$=7 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4\*(Cw\\$5\fP\\$6\*(Cw\\$7\fP
.if \\n(.$=8 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4\*(Cw\\$5\fP\\$6\*(Cw\\$7\fP\\$8
.if \\n(.$=9 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4\*(Cw\\$5\fP\\$6\*(Cw\\$7\fP\\$8\*(Cw
..
'\" macros used by index generating tool
.deIX
.ie '\\n(.z'' .tm .Index: \\$1 \\$2 \\$3 \\$4 \\$5 \\$6 \\$7 \\$8 \\$9	\\n%
.el \\!.ix \\$1 \\$2 \\$3 \\$4 \\$5 \\$6 \\$7 \\$8 \\$9
..
.deix
.ie '\\n(.z'' .tm .Index: \\$1 \\$2 \\$3 \\$4 \\$5 \\$6 \\$7 \\$8 \\$9	\\n%
.el \\!.ix \\$1 \\$2 \\$3 \\$4 \\$5 \\$6 \\$7 \\$8 \\$9
..
.ta .5i +.5i +.5i +.5i +.5i +.5i +.5i +.5i
.HU "FOREWORD"
This document specifies the second generation of symbolic debugging
information based on the DWARF format that 
has been developed by the \*(iX
Programming Languages Special Interest Group (SIG).
It is being circulated for industry review.  
The first version of the DWARF specification was published
by \*(iX in January, 1992.  The current version adds significant
new functionality, but its main thrust is to achieve a much
denser encoding of the DWARF information.  Because of the new
encoding, DWARF Version 2 is not binary compatible with
DWARF Version 1.
.P 
At this point, the SIG believes that this document sufficiently
supports the debugging needs of C, C++, FORTRAN 77, 
Fortran90, Modula2 and Pascal, and we have
released it for public comment.  We will accept comments on this
document until September 30, 1994.  Comments may be directed via email
to the SIG mailing list (plsig@ui.org).  If you are unable
to send email, paper mail, FAX, or machine readable copy 
on \*(aX, MS-DOS, or Macintosh compatible media can be 
sent to \*(iX at the address listed below, 
and will be forwarded to the SIG.
.SP
.SP
.SP
.in +20
UNIX International
.br
Waterview Corporate Center
.br
20 Waterview Boulevard
.br
Parsippany, NJ 07054
.br
Phone:	+1 201-263-8400
.br
Fax:	+1 201-263-8401
.br
.in -20
.nr H1 0
.OP
.H 1 "INTRODUCTION"
\fR
This document defines the format for the information generated by
compilers, assemblers and linkage editors that is necessary for symbolic,
source-level debugging.  The debugging information format does not favor the
design of any compiler or debugger.  Instead, the goal is to create a method of
communicating an accurate picture of the source program to any debugger in a
form that is economically extensible to different languages while retaining
backward compatibility.
.P
The design of the debugging information format is open-ended, allowing for the
addition of new debugging information to accommodate new languages or
debugger capabilities while remaining compatible with other languages or
different debuggers.
.H 2 "Purpose and Scope"
The debugging information format described in this document is designed to
meet the symbolic, source-level debugging needs of 
different languages in a unified fashion by
requiring language independent debugging information whenever possible.
.IX C++ %caa
.IX virtual functions
.IX Fortran
Individual needs, such as C++ virtual functions or Fortran common blocks are
accommodated by creating attributes that are used only for those
languages.  The \*(iX \*(tE believes 
that this document sufficiently covers the 
.IX languages
debugging information needs of C, C++, FORTRAN77, Fortran90, 
Modula2 and Pascal.
.IX C %c
.IX Modula2
.IX Pascal
.IX FORTRAN77
.IX Fortran90
.P
This document describes DWARF Version 2, the second generation of debugging
.IX Version 2
information based on the DWARF format.  While DWARF Version 2 provides
new debugging information not available in Version 1, the primary focus
of the changes for Version 2 is the representation of the information,
rather than the information content itself.  The basic structure of 
the Version 2 format remains as in Version 1: the debugging information
is represented as a series of debugging information entries, each containing
one or more attributes (name/value pairs). 
.IX debugging information entries
.IX attributes
The Version 2 representation, however,
is much more compact than the Version 1 representation.
.IX Version 1
In some cases, this greater density has been achieved at the expense
of additional complexity or greater difficulty in producing and processing
the DWARF information.  We believe that the reduction in I/O and in
memory paging should more than make up for any increase in processing time.
.P
Because the representation of information has changed from Version 1 to
Version 2, Version 2 DWARF information is not binary compatible
.IX compatibility
with Version 1 information.  To make it easier for consumers to
support both Version 1 and Version 2 DWARF information, the Version
2 information has been moved to a different object file section,
.Cf .debug_info .
.IX \f(CW.debug_info\fP %debugai
.P
The intended audience for this document are the developers of 
both producers and consumers of debugging information, typically
language compilers, debuggers and other tools that need to interpret
a binary program in terms of its original source.
.H 2 "Overview"
There are two major pieces to the description of the DWARF format in
this document.  The first piece is the informational content
of the debugging entries.  The second piece
is the way the debugging information is encoded and 
represented in an object file.
.P
The informational content is described in sections two
through six.
Section two describes the overall structure of
the information and attributes that are common to many or all of
the different debugging information entries.  
Sections three, four and five describe the specific debugging
information entries and how they communicate the
necessary information about the source program to a debugger.
Section six describes debugging information contained
outside of the debugging information entries, themselves.
The encoding of the DWARF information is
presented in section seven.
.P
Section eight describes some future directions for the DWARF
specification.
.P
In the following sections, text in normal font describes required aspects
of the DWARF format.  Text in \fIitalics\fP is explanatory or supplementary 
material, and not part of the format definition itself.
.H 2 "Vendor Extensibility"
.IX vendor extensions
This document does not attempt to cover all interesting languages or even
to cover all of the interesting debugging information needs for its primary
target languages (C, C++, FORTRAN77, Fortran90, Modula2, Pascal). 
Therefore the document provides
vendors a way to define their own debugging information tags, attributes,
base type encodings, location operations, language names,
calling conventions and call frame instructions
by reserving a portion of the name space and valid values 
for these constructs for vendor specific additions.  Future versions
of this document will not use names or values reserved for vendor specific
additions.  All names and values not reserved for vendor additions, however,
are reserved for future versions of this document.  See section 7 for
details.
.H 2 "Changes from Version 1"
The following is a list of the major changes made to the DWARF Debugging
Information Format since Version 1 of the format was published (January
.IX Version 1
20, 1992).  The list is not meant to be exhaustive.
.BL
.LI
Debugging information entries have been moved from the 
.Cf .debug
.IX \f(CW.debug\fP %debugaaa
to the 
.Cf .debug_info
.IX \f(CW.debug_info\fP %debugai
section of an object file.
.LI
.IX tags
.IX attributes, names
.IX attributes, forms
The tag, attribute names and attribute forms encodings have been moved
out of the debugging information itself to a separate abbreviations table.
.IX abbreviations table
.LI
Explicit sibling pointers have been made optional.  Each
.IX debugging information entries, siblings
entry now specifies (through the abbreviations table) whether
or not it has children.
.IX debugging information entries, child entries
.LI
New more compact attribute forms have been added, including a variable
length constant data form.  Attribute values may now have any 
.IX variable length data
.IX attributes, forms
.IX attributes, values
form within a given class of forms.
.LI
Location descriptions have been replaced by a new, more compact
and more expressive format.
.IX locations, descriptions
There is now a way of expressing multiple locations for an object
whose location changes during its lifetime.
.IX locations, lists
.LI
There is a new format for line number information
that provides information
for code contributed to a compilation unit from an included file.
Line number information is now in the 
.IX line number information
.Cf .debug_line
.IX \f(CW.debug_line\fP %debugali
section of an object file.
.LI
The representation of the type of a declaration has been
reworked.
.IX declarations, types of
.LI
A new section provides an encoding for pre-processor macro information.
.IX macro information
.IX pre-processor
.LI
Debugging information entries now provide for the representation
of non-defining declarations of objects, functions or types.
.IX declarations, non-defining
.LI
More complete support for Modula2 and Pascal has been added.
.LI
There is now a way of describing locations for segmented address spaces.
.IX segmented address space
.IX address space, segmented
.LI
A new section provides an encoding for information about call
frame activations.
.IX call frame information
.IX activations
.LI
The representation of enumeration and array types has been
.IX enumerations
.IX arrays
reworked so that DWARF presents only a single way of
representing lists of items.
.LI
Support has been added for C++ templates and exceptions.
.IX C++ %caa
.IX templates
.IX exceptions
.LE
.OP
.H 1 "GENERAL DESCRIPTION"
.H 2   "The Debugging Information Entry"
DWARF uses a series of debugging information entries to define a
.IX debugging information entries
low-level representation of a source program. Each debugging
information entry is described by an identifying tag and
contains a series of attributes. 
The tag specifies the class to which an entry
belongs, and the attributes define the specific characteristics
of the entry.
.P
.nr aX \n(Fg+1
The set of required tag names is listed in Figure \n(aX.
.IX tags
The debugging information entries they identify are described in sections three, four and five.
.P
The debugging information entries in DWARF Version 2 are intended
to exist in the 
.Cf .debug_info
section of an object file.
.IX \f(CW.debug_info\fP %debugai
.DF
.TS
center box;
lf(CW) lf(CW)
. 
DW_TAG_access_declaration	DW_TAG_array_type
DW_TAG_base_type	DW_TAG_catch_block
DW_TAG_class_type	DW_TAG_common_block
DW_TAG_common_inclusion	DW_TAG_compile_unit
DW_TAG_const_type	DW_TAG_constant
DW_TAG_entry_point	DW_TAG_enumeration_type
DW_TAG_enumerator	DW_TAG_file_type
DW_TAG_formal_parameter	DW_TAG_friend
DW_TAG_imported_declaration	DW_TAG_inheritance
DW_TAG_inlined_subroutine	DW_TAG_label
DW_TAG_lexical_block	DW_TAG_member
DW_TAG_module	DW_TAG_namelist
DW_TAG_namelist_item	DW_TAG_packed_type
DW_TAG_pointer_type	DW_TAG_ptr_to_member_type
DW_TAG_reference_type	DW_TAG_set_type
DW_TAG_string_type	DW_TAG_structure_type
DW_TAG_subprogram	DW_TAG_subrange_type
DW_TAG_subroutine_type	DW_TAG_template_type_param
DW_TAG_template_value_param	DW_TAG_thrown_type
DW_TAG_try_block	DW_TAG_typedef
DW_TAG_union_type	DW_TAG_unspecified_parameters
DW_TAG_variable	DW_TAG_variant
DW_TAG_variant_part	DW_TAG_volatile_type
DW_TAG_with_stmt
.TE
.FG "Tag names"
.DE
.H 2 "Attribute Types"
Each attribute value is characterized by an attribute name.
.IX attributes
.IX attributes, names
The set of attribute names is 
.nr aX \n(Fg+1
listed in Figure \n(aX.
.DF
.TS
center box;
lf(CW) lf(CW)
. 
DW_AT_abstract_origin	DW_AT_accessibility
DW_AT_address_class	DW_AT_artificial
DW_AT_base_types	DW_AT_bit_offset
DW_AT_bit_size	DW_AT_byte_size
DW_AT_calling_convention	DW_AT_common_reference
DW_AT_comp_dir	DW_AT_const_value
DW_AT_containing_type	DW_AT_count
DW_AT_data_member_location	DW_AT_decl_column
DW_AT_decl_file	DW_AT_decl_line
DW_AT_declaration	DW_AT_default_value
DW_AT_discr	DW_AT_discr_list
DW_AT_discr_value	DW_AT_encoding
DW_AT_external	DW_AT_frame_base
DW_AT_friend	DW_AT_high_pc
DW_AT_identifier_case	DW_AT_import
DW_AT_inline	DW_AT_is_optional
DW_AT_language	DW_AT_location
DW_AT_low_pc	DW_AT_lower_bound
DW_AT_macro_info	DW_AT_name
DW_AT_namelist_item	DW_AT_ordering
DW_AT_priority	DW_AT_producer
DW_AT_prototyped	DW_AT_return_addr
DW_AT_segment	DW_AT_sibling
DW_AT_specification	DW_AT_start_scope
DW_AT_static_link	DW_AT_stmt_list
DW_AT_stride_size	DW_AT_string_length
DW_AT_type	DW_AT_upper_bound
DW_AT_use_location	DW_AT_variable_parameter
DW_AT_virtuality	DW_AT_visibility
DW_AT_vtable_elem_location
.TE	
.FG "Attribute names"
.DE
.P
The permissible values for an attribute belong to one or more classes
.IX attributes, values
.IX attributes, forms
of attribute value forms.  Each form class may be represented in one or more
ways.  For instance, some attribute values consist of a single piece
of constant data.  ``Constant data'' is the class of attribute value
that those attributes may have.  There are several representations
of constant data, however (one, two, four, eight bytes and variable
length data).  The particular representation for any given instance
of an attribute is encoded along with the attribute name as part
of the information that guides the interpretation of a debugging
information entry.   Attribute value forms may belong
to one of the following classes.
.VL 18
.LI address
.IX attributes, addresses
Refers to some location in the address space of the described program.
.LI block
.IX attributes, blocks
An arbitrary number of uninterpreted bytes of data.
.LI constant
.IX attributes, constants
One, two, four or eight bytes of uninterpreted data, or data encoded
in the variable length format known as LEB128 (see section 7.6).
.IX variable length data
.IX LEB128
.LI flag
.IX attributes, flags
A small constant that indicates the presence or absence of an attribute.
.LI reference
.IX attributes, references
Refers to some member of the set of debugging information entries that describe
the program.  There are two types of reference.  The first is an
offset relative to the beginning of the compilation unit in
which the reference occurs and must refer to an entry within
that same compilation unit.  The second type of reference
is the address of any debugging information entry within
the same executable or shared object; it may refer to an entry
in a different compilation unit from the unit containing the
reference.
.LI string
.IX attributes, strings
A null-terminated sequence of zero or more (non-null) bytes.
Data in this form are generally printable strings.  Strings
may be represented directly in the debugging information entry
or as an offset in a separate string table.
.LE
.P
There are no limitations on the ordering of attributes within a debugging
.IX attributes, ordering
information entry, but to prevent ambiguity,
no more than one attribute with a given name may appear in any debugging
information entry.
.H 2 "Relationship of Debugging Information Entries"
.I
A variety of needs can be met by permitting a single debugging
information entry to ``own'' an arbitrary number of other debugging
entries and by permitting the same debugging information entry to be
one of many owned by another debugging information entry.
This makes it possible to describe, for example,
the static block structure within
a source file, show the members of a structure, union, or class, and associate
declarations with source files or source files with shared objects.
.P
.R
The ownership relation
of debugging information entries is achieved naturally
.IX debugging information entries
because the debugging information is represented as a tree.
The nodes of the tree are the debugging information entries
themselves.  The child entries of any node are exactly those
.IX debugging information entries, child entries
debugging information entries owned by that node.\*F
.FS
While the ownership relation of the debugging information
entries is represented as a tree, other relations among
the entries exist, for example, a pointer from an entry
representing a variable to another entry representing
the type of that variable.  If all such relations are
taken into account, the debugging entries form a graph,
not a tree.
.FE
.P
The tree itself is represented by flattening it in prefix
order.  Each debugging information entry
is defined either to have child entries or not to have child entries
(see section 7.5.3).
If an entry is defined not to have children, the next physically
succeeding entry is the sibling of the prior entry.  If an entry
.IX debugging information entries, siblings
is defined to have children, the next physically succeeding entry
is the first child of the prior entry.  Additional children of the parent
entry are represented as siblings of the first child.  A chain
of sibling entries is terminated by a null entry.
.IX debugging information entries, null entries
.P
In cases where a producer of debugging information
feels that it will be important for consumers of that information
to quickly scan chains of sibling entries, ignoring the children
of individual siblings, that producer may attach an
.Cf AT_sibling
attribute to any debugging information entry.  The value of
this attribute is a reference to the sibling entry of the
entry to which the attribute is attached.
.H 2 "Location Descriptions"
.I
The debugging information must provide consumers a way to find the location
of program variables, determine the bounds of dynamic arrays and strings
and possibly to find the base address of a subroutine's stack frame or
the return address of a subroutine.  Furthermore, to meet the needs
of recent computer architectures and optimization techniques, the debugging
information must be able to describe the location of an object
whose location changes over the object's lifetime.
.P
.R
Information about the location of program objects is provided by
location descriptions.  Location
.IX locations, descriptions
descriptions can be either of two forms: 
.AL
.LI
\fILocation expressions\fP which are a language independent representation of 
addressing rules
.IX locations, expressions
of arbitrary complexity built from a few basic
building blocks, or \fIoperations\fP.  They are sufficient for describing
the location of any object as long as its lifetime is either static
or the same as the lexical block that owns it, and it does not move throughout 
its lifetime.  
.LI
\fILocation lists\fP which are used to describe objects that 
.IX locations, lists
have a limited lifetime or change their location throughout their
lifetime.  Location lists are more completely described below.
.LE
.P
The two forms are distinguished in a context sensitive manner.  As the value
of an attribute, a location expression is 
encoded as a block and a location list is encoded as a constant offset into
a location list table.
.P
.I
Note: The Version 1 concept of "location descriptions" was replaced in Version 2 
with this new abstraction because it is denser and more descriptive.  
.IX Version 1
.IX Version 2
.R
.H 3 "Location Expressions"
A location expression consists of zero or more location operations.
.IX locations, expressions
An expression with zero operations 
is used to denote an object that is
present in the source code but not present in the object code
(perhaps because of optimization).  
.IX optimized code
The location operations fall into two categories, register names and
addressing operations.  Register names always appear alone and indicate
that the referred object is contained inside a particular 
register.  Addressing operations are memory address computation 
rules.  All location operations are encoded as a stream of opcodes that
are each followed by zero or more literal operands.  The number of operands
is determined by the opcode.
.H 3 "Register Name Operators"
.IX locations, register name operators
The following operations can be used to name a register.
.P
.I
Note that the 
register number represents a DWARF specific mapping of numbers onto
the actual registers of a given architecture.
The mapping should be chosen to gain optimal density and 
should be shared by all users of a given architecture. 
The \*(tE recommends
that this mapping be defined by the ABI\*F
.IX ABI
.FS
\fISystem V Application Binary Interface\fP, consisting of the generic
interface and processor supplements for each target architecture.
.FE
authoring committee for each
architecture.
.R
.AL
.LI
.Cf DW_OP_reg0 , " DW_OP_reg1" ", ..., " DW_OP_reg31
.br
The
\f(CWDW_OP_reg\fP\fIn\fP
operations encode the names of up to 32 registers, numbered from
0 through 31, inclusive.  The object addressed is in register \fIn\fP.
.LI
.Cf DW_OP_regx
.br
The
.Cf DW_OP_regx
operation has a single unsigned LEB128 literal operand that encodes the 
name of a register.
.LE
.H 3 "Addressing Operations"
.IX locations, stack
Each addressing operation represents a postfix operation on a simple stack 
machine.  Each element of the stack is the size of an
address on the target machine.
The value on the top of the stack after
``executing'' the location expression is taken to be the result (the address
of the object, or the value of the array bound, or the length of a
dynamic string).  In the case of locations used for structure members, 
.IX members, locations
the computation assumes that the base address of the containing structure
has been pushed on the stack before evaluation of the addressing operation.
.R
.H 4 "Literal Encodings"
.IX locations, literal encodings
The following operations all push a value onto the addressing stack.
.AL
.LI
.Cf DW_OP_lit0 , " DW_OP_lit1" ", ..., " DW_OP_lit31
.br
The
\f(CWDW_OP_lit\fP\fIn\fP operations encode the unsigned 
literal values from 0 through 31, inclusive.
.LI
.Cf DW_OP_addr
.br
The
.Cf DW_OP_addr
operation has a single operand that encodes a
machine address and whose size is the size of an address on the
target machine.
.LI
.Cf DW_OP_const1u
.br
The single operand of the
.Cf DW_OP_const1u
operation provides a 1-byte unsigned integer constant.
.LI
.Cf DW_OP_const1s
.br
The single operand of the
.Cf DW_OP_const1s
operation provides a
1-byte signed integer constant.
.LI
.Cf DW_OP_const2u
.br
The single operand of the
.Cf DW_OP_const2u
operation provides a
2-byte unsigned integer constant.
.LI
.Cf DW_OP_const2s
.br
The single operand of the
.Cf DW_OP_const2s
operation provides a
2-byte signed integer constant.
.LI
.Cf DW_OP_const4u
.br
The single operand of the
.Cf DW_OP_const4u
operation provides a
4-byte unsigned integer constant.
.LI
.Cf DW_OP_const4s
.br
The single operand of the
.Cf DW_OP_const4s
operation provides a
4-byte signed integer constant.
.LI
.Cf DW_OP_const8u
.br
The single operand of the
.Cf DW_OP_const8u
operation provides an
8-byte unsigned integer constant.
.LI
.Cf DW_OP_const8s
.br
The single operand of the
.Cf DW_OP_const8s
operation provides an
8-byte signed integer constant.
.LI
.Cf DW_OP_constu
.br
The single operand of the
.Cf DW_OP_constu
operation provides an
unsigned LEB128 integer constant.
.LI
.Cf DW_OP_consts
.br
The single operand of the
.Cf DW_OP_consts
operation provides a
signed LEB128 integer constant.
.LE
.H 4 "Register Based Addressing"
.IX locations, register based addressing
The following operations push a value onto the stack that 
is the result of adding the contents of a register with 
a given signed offset.  
.AL
.LI
.Cf DW_OP_fbreg
.br
The
\f(CWDW_OP_fbreg\fP
operation provides a signed LEB128 offset from the address specified 
by the location descriptor in the 
.Cf DW_AT_frame_base 
attribute of the current 
.IX subroutines, frame base
function.  \fI(This is typically a "stack pointer" register 
plus or minus some
offset.  On more sophisticated systems it might be a location list that
adjusts the offset according to changes in the stack pointer as
the PC changes.)\fP
.LI
.Cf DW_OP_breg0 , " DW_OP_breg1" ", ..., " DW_OP_breg31
.br
The single operand of the
\f(CWDW_OP_breg\fP\fIn\fP
operations provides a signed LEB128 offset from the specified register.
.LI
.Cf DW_OP_bregx
.br
The
.Cf DW_OP_bregx
operation has two operands:  a signed LEB128 offset from the specified register
which is defined with an unsigned LEB128 number.
.LE
.H 4 "Stack Operations"
.IX locations, stack
The following operations 
manipulate the ``location stack.''
Location operations that index the location stack assume that
the top of the stack (most recently added entry) has index 0.
.AL
.LI
.Cf DW_OP_dup
.br
The
.Cf DW_OP_dup 
operation duplicates the value at the top of the location stack.
.LI
.Cf DW_OP_drop
.br
The
.Cf DW_OP_drop 
operation pops the value at the top of the stack.
.LI
.Cf DW_OP_pick
.br
The single operand of the
.Cf DW_OP_pick
operation provides a 1-byte index.  The stack entry with the specified index
(0 through 255, inclusive) is pushed on the stack.
.LI
.Cf DW_OP_over
.br
The
.Cf DW_OP_over
operation duplicates the entry currently second in the stack
at the top of the stack.  This is equivalent to an
.Cf DW_OP_pick
operation, with index 1.
.LI
.Cf DW_OP_swap
.br
The
.Cf DW_OP_swap
operation swaps the top two stack entries.   The entry at
the top of the stack becomes the second stack entry, and
the second entry becomes the top of the stack.
.LI
.Cf DW_OP_rot
.br
The
.Cf DW_OP_rot
operation rotates the first three stack entries.   The entry at
the top of the stack becomes the third stack entry, the second entry
becomes the top of the stack, and the third entry becomes the second
entry.
.LI
.Cf DW_OP_deref
.br
The
.Cf DW_OP_deref
operation pops the top stack entry and treats it as an address.
The value retrieved from that address is pushed.  The size of the
data retrieved from the dereferenced address is the size of an address
on the target machine.
.LI
.Cf DW_OP_deref_size
.br
The
.Cf DW_OP_deref_size
operation behaves like the 
.Cf DW_OP_deref
operation: it 
pops the top stack entry and treats it as an address.
The value retrieved from that address is pushed.  
In the 
.Cf DW_OP_deref_size
operation, however,
the size in bytes of the
data retrieved from the dereferenced address is specified by the
single operand.  This operand is a 1-byte unsigned integral constant
whose value may not be larger than the size of an address on
the target machine.  The data retrieved is zero extended to the size
of an address on the target machine before being pushed on
the expression stack.
.LI
.Cf DW_OP_xderef
.br
The
.Cf DW_OP_xderef
.IX address space, multiple
operation provides an extended dereference mechanism.  The entry at the
top of the stack is treated as an address.  The second stack entry
is treated as an ``address space identifier'' for those architectures
that support multiple address spaces.  The top two stack elements
are popped, a data item is retrieved through an implementation-defined
address calculation and pushed as the new stack top.  The size of the
data retrieved from the dereferenced address is the size of an address
on the target machine.
.LI
.Cf DW_OP_xderef_size
.br
The
.Cf DW_OP_xderef_size
operation behaves like the 
.Cf DW_OP_xderef
operation: the entry at the
top of the stack is treated as an address.  The second stack entry
is treated as an ``address space identifier'' for those architectures
that support multiple address spaces.  The top two stack elements
are popped, a data item is retrieved through an implementation-defined
address calculation and pushed as the new stack top.  
In the 
.Cf DW_OP_xderef_size
operation, however,
the size in bytes of the
data retrieved from the dereferenced address is specified by the
single operand.  This operand is a 1-byte unsigned integral constant
whose value may not be larger than the size of an address on
the target machine.  The data retrieved is zero extended to the size
of an address on the target machine before being pushed on
the expression stack.
.LE
.H 4 "Arithmetic and Logical Operations"
.IX locations, arithmetic operations
.IX locations, logical operations
The following provide arithmetic and logical operations.
The arithmetic operations perform ``addressing arithmetic,''
that is, unsigned arithmetic that wraps on an address-sized 
boundary.  The operations do not cause an exception on overflow.
.AL
.LI
.Cf DW_OP_abs
.br
The
.Cf DW_OP_abs
operation pops the top stack entry and pushes its absolute value.
.LI
.Cf DW_OP_and
.br
The
.Cf DW_OP_and
operation pops the top two stack values, performs a bitwise \fIand\fP 
operation on the two, and pushes the result.
.LI
.Cf DW_OP_div
.br
The
.Cf DW_OP_div
operation pops the top two stack values, divides the former second entry
by the former top of the stack 
using signed division, 
and pushes the result.
.LI
.Cf DW_OP_minus
.br
The
.Cf DW_OP_minus
operation pops the top two stack values, subtracts the former top of the stack
from the former second entry, and pushes the result.
.LI
.Cf DW_OP_mod
.br
The
.Cf DW_OP_mod
operation pops the top two stack values and pushes the result of the 
calculation: former second stack entry modulo the former top of the
stack.
.LI
.Cf DW_OP_mul
.br
The
.Cf DW_OP_mul
operation pops the top two stack entries, multiplies them together,
and pushes the result.
.LI
.Cf DW_OP_neg
.br
The
.Cf DW_OP_neg
operation pops the top stack entry, and pushes its negation.
.LI
.Cf DW_OP_not
.br
The
.Cf DW_OP_not
operation pops the top stack entry, and pushes its bitwise complement.
.LI
.Cf DW_OP_or
.br
The
.Cf DW_OP_or
operation pops the top two stack entries, performs a bitwise \fIor\fP 
operation on the two, and pushes the result.
.LI
.Cf DW_OP_plus
.br
The
.Cf DW_OP_plus
operation pops the top two stack entries, adds them together,
and pushes the result.
.LI
.Cf DW_OP_plus_uconst
.br
The
.Cf DW_OP_plus_uconst
operation pops the top stack entry, adds it to the unsigned LEB128
constant operand and pushes the result.
.I
This operation is supplied specifically to be able to encode more field
offsets in two bytes than can be done with "\f(CWDW_OP_lit\fP\fIn\fP\f(CW DW_OP_add\fP".
.R
.LI
.Cf DW_OP_shl
.br
The
.Cf DW_OP_shl
operation pops the top two stack entries, shifts the former second
entry left by the number of bits specified by the former top of
the stack, and pushes the result.
.LI
.Cf DW_OP_shr
.br
The
.Cf DW_OP_shr
operation pops the top two stack entries, shifts the former second
entry right (logically) by the number of bits specified by the former top of
the stack, and pushes the result.
.LI
.Cf DW_OP_shra
.br
The
.Cf DW_OP_shra
operation pops the top two stack entries, shifts the former second
entry right (arithmetically) by the number of bits specified by the former top of
the stack, and pushes the result.
.LI
.Cf DW_OP_xor
.br
The
.Cf DW_OP_xor
operation pops the top two stack entries, performs the logical 
\fIexclusive-or\fP operation on the two, and pushes the result.
.LE
.H 4 "Control Flow Operations"
.IX locations, control flow operations
The following operations provide simple control of the flow of a location
expression.
.AL
.LI 
Relational operators
.br
The six relational operators each pops the top two stack values,
compares the former top of the stack with the former second entry,
and pushes the constant value 1 onto the stack if the result of the
operation is true or the constant value 0 if the result of the operation
is false.  The comparisons are done as signed operations.  The six
operators are 
.Cf DW_OP_le
(less than or equal to),
.Cf DW_OP_ge
(greater than or equal to),
.Cf DW_OP_eq
(equal to),
.Cf DW_OP_lt
(less than),
.Cf DW_OP_gt
(greater than) and
.Cf DW_OP_ne
(not equal to).
.LI
.Cf DW_OP_skip
.br
.Cf DW_OP_skip
is an unconditional branch.  Its
single operand is a 2-byte signed integer constant.
The 2-byte constant is the number of bytes of the location
expression to skip from the current operation, beginning after the
2-byte constant.
.LI
.Cf DW_OP_bra
.br
.Cf DW_OP_bra
is a conditional branch.  Its
single operand is a 2-byte signed integer constant.
This operation pops the top of stack.  If the value
popped is not the constant 0, the 2-byte constant operand is the number
of bytes of the location 
expression to skip from the current operation, beginning after the
2-byte constant.
.LE
.H 4 "Special Operations"
.IX locations, special operations
There are two special operations currently defined:
.AL
.LI
.Cf DW_OP_piece
.br
.I
Many compilers store a single variable in sets of registers, or store
a variable partially in memory and partially in registers.  
.Cf DW_OP_piece
provides a way of describing how large a part of a variable
a particular addressing expression refers to.
.R
.P
.Cf DW_OP_piece
takes a single argument which is an unsigned LEB128 number.  The number
describes the size in bytes of the piece of the object referenced
by the addressing expression whose result is at the top of
the stack.
.LI
.Cf DW_OP_nop
.br
The 
.Cf DW_OP_nop
operation is a place holder.  It has no effect on the location stack or
any of its values.
.LE
.H 3 "Sample Stack Operations"
.IX locations, examples
.I
The stack operations defined in section 2.4.3.3 are fairly
.IX locations, stack
conventional, but the following examples illustrate their behavior
graphically.
.R
.DS
.TS
box expand center tab(;);
l s l l s
lf(CW) lf(CW) lf(CW) lf(CW) lf(CW)
.
Before;Operation;After;
_
0;17;DW_OP_dup;0;17
1;29;;1;17
2;1000;;2;29
;;;3;1000
_
0;17;DW_OP_drop;0;29
1;29;;1;1000
2;1000;;;;
_
0;17;DW_OP_pick 2;0;1000
1;29;;1;17
2;1000;;2;29
;;;3;1000
_
0;17;DW_OP_over;0;29
1;29;;1;17
2;1000;;2;29
;;;3;1000
_
0;17;DW_OP_swap;0;29
1;29;;1;17
2;1000;;2;1000
_
0;17;DW_OP_rot;0;29
1;29;;1;1000
2;1000;;2;17
.TE
.DE
.H 3 "Example Location Expressions"
.I
.IX locations, examples
The addressing expression represented by a location expression, 
if evaluated, generates the
runtime address of the value of a symbol except where the
.Cf DW_OP_reg n,
or
.Cf DW_OP_regx
operations are used.
.P
Here are some examples of how location operations are used to form location
expressions:
.R
.DS
\f(CWDW_OP_reg3\fI
	The value is in register 3.

\f(CWDW_OP_regx 54\fI
	The value is in register 54.

\f(CWDW_OP_addr 0x80d0045c\fI
	The value of a static variable is
	at machine address 0x80d0045c.

\f(CWDW_OP_breg11 44\fI
	Add 44 to the value in
	register 11 to get the address of an
	automatic variable instance.

\f(CWDW_OP_fbreg -50\fI
	Given an \f(CWDW_AT_frame_base\fI value of
	"\f(CWOPBREG31 64\fI," this example 
	computes the address of a local variable
	that is -50 bytes from a logical frame 
	pointer that is computed by adding
	64 to the current stack pointer (register 31).

\f(CWDW_OP_bregx 54 32 DW_OP_deref\fI
	A call-by-reference parameter
	whose address is in the
	word 32 bytes from where register
	54 points.  

\f(CWDW_OP_plus_uconst 4\fI
	A structure member is four bytes
	from the start of the structure
	instance.  The base address is
	assumed to be already on the stack.

\f(CWDW_OP_reg3 DW_OP_piece 4 DW_OP_reg10 DW_OP_piece 2\fI
	A variable whose first four bytes reside
	in register 3 and whose next two bytes
	reside in register 10.\fR
.DE
.H 3 "Location Lists"
.IX locations, lists
Location lists are used in place of location expressions whenever
the object whose location is being described can change location
during its lifetime.  Location lists are contained in a separate
object file section called
.Cf .debug_loc.
.IX \f(CW.debug_loc\fP %debugalo
A location list is indicated by a location
attribute whose value is represented as a
constant offset from the beginning of the 
.Cf .debug_loc
section to the first byte of the list for the object in question.
.P
Each entry in a location list consists of:
.AL
.LI
A beginning address.  This address is relative to the base address
of the compilation unit referencing this location list.  It marks
the beginning of the address range over which the location is valid.
.LI
An ending address, again relative to the base address
of the compilation unit referencing this location list.  It marks
the first address past the end of the address range over 
which the location is valid.
.LI
A location expression describing the location of the object over the
range specified by the beginning and end addresses.
.LE
.P
Address ranges may overlap.  When they do, they describe a situation
in which an object exists simultaneously in more than one place.
If all of the address ranges 
in a given location list do not collectively cover the entire
range over which the object in question is defined, it is assumed
that the object is not available for the portion of the range that is not
covered.
.IX optimized code
.P
The end of any given location list is marked by a 0 for the beginning
address and a 0 for the end address; no location description is present.
A location list containing
only such a 0 entry describes an object that exists in the source
code but not in the executable program. 
.H 2 "Types of Declarations"
.IX declarations, types of
Any debugging information entry describing a declaration that
has a type has a
.Cf DW_AT_type
attribute, whose value is a reference to another debugging
information entry.  The entry referenced may describe
.IX base types
.IX types, base
a base type, that is, a type that is not defined in terms
.IX user-defined types
.IX types, user-defined
of other data types, or it may describe a user-defined type,
such as an array, structure or enumeration.  Alternatively,
the entry referenced may describe a type modifier: constant,
packed, pointer, reference or volatile, which in turn will reference
another entry describing a type or type modifier (using a
.IX type modifiers
.IX types, modifiers
.IX types, packed
.IX types, constant
.IX types, pointer
.IX types, reference
.IX types, volatile
.Cf DW_AT_type
attribute of its own).  See section 5 for descriptions of
the entries describing base types, user-defined types and
type modifiers.
.H 2 "Accessibility of Declarations"
.I
.IX accessibility
.IX declarations, accessibility
Some languages, notably C++ and Ada, have the concept of
.IX C++ %caa
the accessibility of an object or of some other program entity.
The accessibility specifies which classes of other program objects
are permitted access to the object in question.
.R
.P
The accessibility of a declaration is represented by a
.Cf DW_AT_accessibility
attribute, whose value is a constant drawn from the set of codes
.nr aX \n(Fg+1
listed in Figure \n(aX.
.DF
.TS
box center;
lf(CW)
. 
DW_ACCESS_public
DW_ACCESS_private
DW_ACCESS_protected
.TE
.FG "Accessibility codes"
.DE
.H 2 "Visibility of Declarations"
.I
.IX Modula2
.IX visibility
.IX declarations, visibility
Modula2 has the concept of the visibility of a declaration.
The visibility specifies which declarations are to be visible outside
of the module in which they are declared.
.R
.P
The visibility of a declaration is represented by a
.Cf DW_AT_visibility
attribute, whose value is a constant drawn from the set of codes
.nr aX \n(Fg+1
listed in Figure \n(aX.
.DF
.TS
box center;
lf(CW)
. 
DW_VIS_local
DW_VIS_exported
DW_VIS_qualified
.TE
.FG "Visibility codes"
.DE
.H 2 "Virtuality of Declarations"
.I
.IX C++ %caa
.IX virtuality
.IX virtual functions
C++ provides for virtual and pure virtual structure or class
member functions and for virtual base classes.
.P
.R
The virtuality of a declaration is represented by a
.Cf DW_AT_virtuality
attribute, whose value is a constant drawn from the set of codes
.nr aX \n(Fg+1
listed in Figure \n(aX.
.DF
.TS
box center;
lf(CW)
. 
DW_VIRTUALITY_none
DW_VIRTUALITY_virtual
DW_VIRTUALITY_pure_virtual
.TE
.FG "Virtuality codes"
.DE
.H 2 "Artificial Entries"
.I
.IX artificial entries
A compiler may wish to generate debugging information entries
for objects or types that were not actually declared
in the source of the application.  An example is a formal parameter
entry to represent the hidden 
.Cf this
parameter that most C++ implementations pass as the first argument
to non-static member functions.
.R
.P
Any debugging information entry representing the declaration of an
object or type artificially generated by a compiler and 
not explicitly declared by the source program may have a
.Cf DW_AT_artificial 
attribute.  The value of this attribute is a flag.
.H 2 "Target-Specific Addressing Information"
.I
.IX segmented address space
.IX address space, segmented
In some systems, addresses are specified as offsets within a given
segment rather than as locations within a single flat address space.
.R
.P
Any debugging information entry that contains a description of the
location of an object or subroutine may have a
.Cf DW_AT_segment
attribute, whose value is a location description.  The description
evaluates to the segment value of the item being described.  If
the entry containing the 
.Cf DW_AT_segment
attribute has a
.Cf DW_AT_low_pc
or 
.Cf DW_AT_high_pc
attribute, or a location description that evaluates to an address,
.IX locations, descriptions
.IX addresses, offset portion
then those values represent the offset portion of the address
within the segment specified by
.Cf DW_AT_segment .
.P
If an entry has no
.Cf DW_AT_segment
attribute, it inherits the segment value from its parent entry.
If none of the entries in the chain of parents for this entry
back to its containing compilation unit entry have 
.Cf DW_AT_segment
attributes, then the entry is assumed to exist within a flat
address space.  Similarly, if the entry has a
.IX flat address space
.IX address space, flat
.Cf DW_AT_segment
attribute containing an empty location description, that entry
is assumed to exist within a flat address space.
.P
.I
Some systems support different classes of addresses.  The address
class may affect the way a pointer is dereferenced or the way
a subroutine is called.
.P
.R
Any debugging information entry representing a pointer or reference
type or a subroutine or subroutine type may have a
.IX types, pointer
.IX types, reference
.IX subroutines
.IX subroutines, types
.Cf DW_AT_address_class
.IX addresses, class
attribute, whose value is a constant.  The set of permissible
values is specific to each target architecture.  The value
.Cf DW_ADDR_none ,
however, is common to all encodings, and means that no address class
has been specified.
.P
.I
For example, the Intel386\(tm processor might use the following
values:
.R
.DF
.TS
box center;
l l l
lf(CW) lf(CW) l
. 
Name	Value	Meaning
_
DW_ADDR_none	0	no class specified
DW_ADDR_near16	1	16-bit offset, no segment
DW_ADDR_far16	2	16-bit offset, 16-bit segment
DW_ADDR_huge16	3	16-bit offset, 16-bit segment
DW_ADDR_near32	4	32-bit offset, no segment
DW_ADDR_far32	5	32-bit offset, 16-bit segment
.TE
.FG "Example address class codes"
.DE
.H 2 "Non-Defining Declarations"
.IX declarations, non-defining
.IX declarations, defining
A debugging information entry representing a program object or type
typically represents the defining declaration of that object or type.  In
certain contexts, however, a debugger might need information about a
declaration of a subroutine, object or type that is not also a 
definition to evaluate an expression correctly. 
.P
.I
As an example, consider the following fragment of C code:
.DS
\f(CWvoid myfunc()
{
        int     x;
        {
                extern float x;
                g(x);
        }
}\fP
.DE
.P
ANSI-C scoping rules require that the value of the variable \f(CWx\fP
passed to the function \f(CWg\fP is the value of the global variable
\f(CWx\fP rather than of the local version.
.R
.P
Debugging information entries that represent non-defining declarations
of a program object or type have a
.Cf DW_AT_declaration
attribute, whose value is a flag.
.H 2 "Declaration Coordinates"
.I
It is sometimes useful in a debugger to be able to associate a declaration
with its occurrence in the program source.  
.P
.R
.IX declarations, coordinates
Any debugging information entry representing the declaration of 
an object, module, subprogram or type may have 
.Cf DW_AT_decl_file ,
.Cf DW_AT_decl_line 
and
.Cf DW_AT_decl_column
attributes, each of whose value is a constant.
.P
The value of the 
.Cf DW_AT_decl_file 
attribute corresponds
to a file number from the statement information table for the compilation
.IX line number information
unit containing this debugging information entry and represents the
source file in which the declaration appeared (see section 6.2).
.IX source, files
The value 0 indicates that no source file has been specified.
.P
The value of the
.Cf DW_AT_decl_line
attribute represents the source line number at which the first
.IX source, lines
character of the identifier of the declared object appears.
The value 0 indicates that no source line has been specified.
.P
The value of the
.Cf DW_AT_decl_column
attribute represents the source column number at which the first
.IX source, columns
character of the identifier of the declared object appears.
The value 0 indicates that no column has been specified.
.H 2 "Identifier Names"
.IX identifiers, names
Any debugging information entry representing a program entity that
has been given a name may have a
.Cf DW_AT_name
attribute, whose value is a string representing the name as it appears
in the source program.  A debugging information entry containing
no name attribute, or containing a name attribute whose value consists
of a name containing a single null byte,
represents a program entity for which no name was given in the source.
.I
.P
Note that since the names of program objects
described by DWARF are the names as they appear in the source program,
implementations of language translators that use some form of mangled
name (as do many implementations of C++) should use the unmangled
.IX C++ %caa
form of the name in the DWARF 
.Cf DW_AT_name
attribute, including the keyword
.Cf operator ,
if present.  Sequences of multiple whitespace characters may be compressed.
.R
.OP
.H 1 "PROGRAM SCOPE ENTRIES"
This section describes debugging information entries that relate
to different levels of program scope: compilation unit, module,
subprogram, and so on.  These entries may be thought of as
bounded by ranges of text addresses within the program.
.H 2   "Compilation Unit Entries"
An object file may be derived from one or more compilation units.  Each
such compilation unit will be described by a debugging information 
entry with the tag \f(CWDW_TAG_compile_unit\fP.
.I
.P
A compilation unit typically represents the text and data contributed
.IX compilation units
to an executable by a single relocatable object file.  It may
be derived from several source files, including pre-processed ``include
files.''
.R
.P
The compilation unit entry may have the following attributes:
.AL
.LI
A 
.Cf DW_AT_low_pc
attribute whose value is the
relocated address of the first machine instruction generated for that 
compilation unit.
.LI
A
.Cf DW_AT_high_pc
attribute whose value is the
relocated address of the first location
past the last machine instruction generated for that compilation unit.
.P
.I
The address may be beyond the last valid instruction in the executable,
of course, for this and other similar attributes.
.R
.P
The presence of low and high pc attributes in a compilation unit entry
imply that the code generated for that compilation unit is
contiguous and exists totally within the boundaries specified
by those two attributes.  If that is not the case, no low
and high pc attributes should be produced.
.IX address space, contiguous
.LI
A
.Cf DW_AT_name
attribute whose value is a
null-terminated string containing the full or relative path name of
the primary source file from which the compilation unit was derived.
.IX source, files
.LI
A 
.Cf DW_AT_language
attribute whose constant value is
.IX languages
a code indicating the source language of the compilation unit.
.nr aX \n(Fg+1
The set of language names and their meanings are 
given in Figure \n(aX.
.DF
.TS
box center;
lf(CW) lf(R)
. 
DW_LANG_C	Non-ANSI C, such as K&R
DW_LANG_C89	ISO/ANSI C
DW_LANG_C_plus_plus	C++
DW_LANG_Fortran77	FORTRAN77
DW_LANG_Fortran90	Fortran90
DW_LANG_Modula2	Modula2
DW_LANG_Pascal83	ISO/ANSI Pascal
.TE
.FG "Language names"
.DE
.LI
A
.Cf DW_AT_stmt_list
attribute whose value is a reference to
line number information for this compilation unit.
.IX line number information
.P
This information is placed in a separate object file section from the debugging
information entries themselves.  The value of the statement list attribute
is the offset in the \f(CW.debug_line\fP section of the first byte of the 
line number information for this compilation unit.  See section 6.2.
.LI
A
.Cf DW_AT_macro_info
attribute whose value is a reference to the macro information for this
compilation unit.
.IX macro information
.P
This information is placed in a separate object file section from the debugging
information entries themselves.  The value of the macro information attribute
is the offset in the \f(CW.debug_macinfo\fP section of the first byte of the 
macro information for this compilation unit.  See section 6.3.
.LI
A
.Cf DW_AT_comp_dir
attribute whose value is a null-terminated string containing
the current working directory of the compilation command that
produced this compilation unit in whatever form makes sense
for the host system.
.P
.I
The suggested form for the value of the \f(CWDW_AT_comp_dir\fP
attribute on \*(aX systems is ``hostname\f(CW:\fPpathname''.  If no
hostname is available, the suggested form is ``\f(CW:\fPpathname''.
.R
.LI
A
.Cf DW_AT_producer
attribute whose value is a null-terminated string containing information
about the compiler that produced the compilation unit.  The
actual contents of the string will be specific to each producer,
but should begin with the name of the compiler vendor or some
other identifying character sequence that should avoid
confusion with other producer values.
.LI
A
.Cf DW_AT_identifier_case
.IX identifiers, case
attribute whose constant value is a code describing the treatment of
identifiers within this compilation unit.  The set of identifier case
.nr aX \n(Fg+1
codes is given in Figure \n(aX.
.DF
.TS
box center;
lf(CW)
. 
DW_ID_case_sensitive
DW_ID_up_case
DW_ID_down_case
DW_ID_case_insensitive
.TE
.FG "Identifier case codes"
.DE
.P
.Cf DW_ID_case_sensitive 
is the default for all compilation units that do not have this attribute.
It indicates that names given as the values of 
.Cf DW_AT_name
attributes in debugging information entries for the compilation unit
reflect the names as they appear in the source program.
The debugger should be sensitive to the case of identifier names
when doing identifier lookups.
.P
.Cf DW_ID_up_case
means that the producer of the debugging information for this compilation
unit converted all source names to upper case.  The values of the
name attributes may not reflect the names as they appear in the source
program.  The debugger should convert all names to upper case
when doing lookups.
.P
.Cf DW_ID_down_case
means that the producer of the debugging information for this compilation
unit converted all source names to lower case.  The values of the
name attributes may not reflect the names as they appear in the source
program.  The debugger should convert all names to lower case when
doing lookups.
.P
.Cf DW_ID_case_insensitive 
means that the values of the name attributes reflect the names
as they appear in the source program but that a case insensitive
lookup should be used to access those names.
.LI
A
.Cf DW_AT_base_types
.IX base types
.IX types, base
attribute whose value is a reference.  This attribute points to
a debugging information entry representing another compilation
unit.  It may be used to specify the compilation unit containing
the base type entries used by entries in the current compilation
unit (see section 5.1).
.P
.I
This attribute provides a consumer a way to find the definition
of base types for a compilation unit that does not itself
contain such definitions.  This allows a consumer, for example,
to interpret a type conversion to a base type correctly.
.R
.LE
.R
.P
A compilation unit entry 
owns debugging information entries that represent the declarations made in
the corresponding compilation unit.
.H 2 "Module Entries"
.I
Several languages have the concept of a ``module.''  
.IX modules
.P
.R
A module is
represented by a debugging information entry with the tag
.Cf DW_TAG_module .
Module entries may own other debugging information entries describing
program entities whose declaration scopes end at the end of the module
itself.
.P
If the module has a name, the module entry has a 
.Cf DW_AT_name 
attribute whose
value is a null-terminated string containing the module name as it appears
in the source program.
.P
If the module contains initialization code, the module entry
has a 
.Cf DW_AT_low_pc
attribute whose value is the
relocated address of the first machine instruction generated for that 
initialization code.  It also has a 
.Cf DW_AT_high_pc
attribute whose value is
the relocated address of the first location past the last machine
instruction generated for the initialization code.
.P
If the module has been assigned a priority, it may have a
.Cf DW_AT_priority
attribute.  The value of this attribute is a reference to another
.IX modules, priority
debugging information entry describing a variable with a constant
value.  The value of this variable is the actual constant
value of the module's priority, represented as it would be on the
target architecture.
.P
.I
.IX Modula2
.IX modules, definition
A Modula2 definition module may be represented by a module entry
containing a
.Cf DW_AT_declaration
attribute.
.R
.H 2   "Subroutine and Entry Point Entries"
.IX subroutines
.IX entry points
The following tags exist to describe debugging information
entries for subroutines and entry points:
.VL 30
.LI \f(CWDW_TAG_subprogram\fP
A global or file static subroutine or function.
.LI \f(CWDW_TAG_inlined_subroutine\fP
A particular inlined instance of a subroutine or function.
.LI \f(CWDW_TAG_entry_point\fP
A Fortran entry point.
.LE
.H 3 "General Subroutine and Entry Point Information"
The subroutine or entry point entry has a 
.Cf DW_AT_name 
attribute
whose value is a null-terminated string containing the subroutine or entry
point name as it appears in the source program.
.P
If the name of the subroutine described by an entry with the tag
.Cf DW_TAG_subprogram
is visible outside of its containing compilation unit, that
entry has a
.Cf DW_AT_external
attribute, whose value is a flag.
.IX declarations, external
.I
.P
.IX members, functions
.IX subroutines, members
Additional attributes for functions that are members of a class or
structure are described in section 5.5.5.
.P
A common debugger feature is to allow the debugger user to call a
subroutine within the subject program.  In certain cases, however,
the generated code for a subroutine will not obey the standard calling
conventions for the target architecture and will therefore not
.IX calling conventions
be safe to call from within a debugger.
.R
.P
A subroutine entry may contain a
.Cf DW_AT_calling_convention
attribute, whose value is a constant.  If this attribute is not
present, or its value is the constant
.Cf DW_CC_normal ,
then the subroutine may be safely called by obeying the ``standard''
calling conventions of the target architecture.  If the value of
the calling convention attribute is the constant
.Cf DW_CC_nocall ,
the subroutine does not obey standard calling conventions, and it
may not be safe for the debugger to call this subroutine.
.P
If the semantics of the language of the compilation unit 
containing the subroutine entry distinguishes between ordinary subroutines
.IX main programs
and subroutines that can serve as the ``main program,'' that is, subroutines
that cannot be called directly following the ordinary calling conventions,
then the debugging information entry for such a subroutine may have a
calling convention attribute whose value is the constant
.Cf DW_CC_program .
.P
.I
The 
.Cf DW_CC_program 
value is intended to support Fortran main programs.
It is not intended as a way of finding the entry address for the program.
.R
.H 3 "Subroutine and Entry Point Return Types"
.IX subroutines, return types
.IX entry points, return types
If the subroutine or entry point is a function that returns a value, then
its debugging information entry has a
.Cf DW_AT_type
attribute to denote the type returned by that function.
.P
.I
Debugging information entries for C 
.Cf void
.IX C %c
functions should not have an attribute for the return type.
.P
In ANSI-C there is a difference between the types of functions
declared using function prototype style declarations and those
declared using non-prototype declarations.  
.IX subroutines, prototypes
.P
.R
A subroutine entry
declared with a function prototype style declaration may have a
.Cf DW_AT_prototyped
attribute, whose value is a flag.
.H 3 "Subroutine and Entry Point Locations"
.IX subroutines, locations
.IX entry points, locations
A subroutine entry has a
.Cf DW_AT_low_pc
attribute whose value is the relocated address of the first machine instruction
generated for the subroutine.
It also has a
.Cf DW_AT_high_pc
attribute whose value is the relocated address of the
first location past the last machine instruction generated
for the subroutine.  
.P
.I
Note that for the low and high pc attributes to have meaning, DWARF
makes the assumption that the code for a single subroutine is allocated
in a single contiguous block of memory.
.IX address space, contiguous
.R
.P
An entry point has a
.Cf DW_AT_low_pc
attribute whose value is the relocated address of the first machine instruction
generated for the entry point.
.P
Subroutines and entry points may also have 
.Cf DW_AT_segment 
and 
.Cf DW_AT_address_class
.IX segmented address space
.IX address space, segmented
.IX addresses, class
attributes, as appropriate, to specify which segments the code
for the subroutine resides in and the addressing mode to be used
in calling that subroutine.
.P
A subroutine entry representing a subroutine declaration
that is not also a definition does not have low and high pc attributes.
.IX declarations, non-defining
.H 3 "Declarations Owned by Subroutines and Entry Points"
.IX subroutines, declarations owned by
.IX entry points, declarations owned by
The declarations enclosed by a subroutine or entry point
are represented by debugging information entries that are
owned by the subroutine or entry point entry.
Entries representing the formal parameters of the subroutine or
entry point appear in
the same order as the corresponding declarations in the source program.
.IX attributes, ordering
.IX parameters, formal
.P
.I
There is no ordering requirement on entries for declarations that are
children of subroutine or entry point entries but that do not represent
formal parameters.  The formal parameter entries may be interspersed
with other entries used by formal parameter entries, such as type entries.
.R
.P
The unspecified parameters of a variable parameter list
.IX parameters, unspecified
are represented by a debugging information entry with the tag 
.Cf DW_TAG_unspecified_parameters .
.P
The entry for a subroutine or entry point that includes a Fortran 
.IX Fortran
.IX common blocks
common block has a child entry with the tag
.Cf DW_TAG_common_inclusion .
The common inclusion entry has a
.Cf DW_AT_common_reference
attribute whose value is a reference to the debugging entry for
the common block being included (see section 4.2).
.H 3 "Low-Level Information"
A subroutine or entry point entry may have a
.Cf DW_AT_return_addr
.IX subroutines, return addresses
attribute, whose value is a location description.
The location calculated is the place where the return address for 
the subroutine or entry point is stored.
.P
A subroutine or entry point entry may also have a
.Cf DW_AT_frame_base
.IX subroutines, frame base
attribute, whose value is a location description that
computes the ``frame base'' for the subroutine or entry point.
.P
.I
The frame base for a procedure is typically an address fixed
relative to the first unit of storage allocated for the procedure's
stack frame.  The 
.Cf DW_AT_frame_base
attribute can be used in several ways:
.AL
.LI
In procedures that need location lists to locate local variables, the
.Cf DW_AT_frame_base
can hold the needed location list, while all variables'
location descriptions can be simpler location expressions involving the frame 
base.
.LI
It can be used as a key in resolving "up-level" addressing with nested
routines.  (See 
.Cf DW_AT_static_link ,
below)
.LE
.P
Some languages support nested subroutines.  In such languages, it is possible
.IX subroutines, nested
to reference the local variables of an outer subroutine from within
an inner subroutine.  The 
.Cf DW_AT_static_link 
and 
.Cf DW_AT_frame_base 
attributes allow debuggers to support this same kind of referencing.
.R
.P
If a subroutine or entry point is nested, it may have a
.Cf DW_AT_static_link
attribute, whose value is a location description that
computes the frame base of the relevant instance of the subroutine
that immediately encloses the subroutine or entry point.
.P
In the context of supporting nested subroutines, the 
.Cf DW_AT_frame_base
attribute value should obey the following constraints:
.AL
.LI
It should compute a value that does not change during the life of the procedure,
and
.LI
The computed value should be unique among instances of the same subroutine.
(For typical 
.Cf DW_AT_frame_base 
use, this means that a recursive
subroutine's stack frame must have non-zero size.)
.LE
.P
.I
If a debugger is attempting to resolve an up-level reference to a variable, it
uses the nesting structure of DWARF to determine which subroutine is the lexical
parent and the 
.Cf DW_AT_static_link
value to identify the appropriate active frame
of the parent.  It can then attempt to find the reference within the context
of the parent. 
.R
.H 3 "Types Thrown by Exceptions"
.I
In C++ a subroutine may declare a set of types for which
.IX C++ %caa
.IX exceptions
that subroutine may generate or ``throw'' an exception.
.P
.R
If a subroutine explicitly declares that it may throw an
exception for one or more types, each such type is
represented by a debugging information entry with the tag
.Cf DW_TAG_thrown_type .
Each such entry is a child of the entry representing the
subroutine that may throw this type.  All thrown type entries
should follow all entries representing the formal parameters
of the subroutine and precede all entries representing the
local variables or lexical blocks contained in the subroutine.
Each thrown type entry contains a
.Cf DW_AT_type
attribute, whose value is a reference to an entry describing
the type of the exception that may be thrown.
.H 3 "Function Template Instantiations"
.I
.IX C++ %caa
.IX templates
In C++ a function template is a generic
definition of a function that
is instantiated differently when called with values
of different types.  DWARF does not represent the generic
template definition, but does represent each instantiation.
.R
.P
A template instantiation is represented by a debugging information
entry with the tag
.Cf DW_TAG_subprogram .
With three exceptions,
such an entry will contain the same attributes and have the same
types of child entries as would an entry for a subroutine 
defined explicitly
using the instantiation types.  The exceptions are:
.AL
.LI 
Each formal parameterized type declaration appearing in the
template definition is represented by a debugging information entry
with the tag 
.Cf DW_TAG_template_type_parameter .
Each such entry has a 
.Cf DW_AT_name
attribute, whose value is a null-terminated
string containing the name of the formal type parameter as it
appears in the source program.  The template type parameter
entry also has a 
.Cf DW_AT_type
attribute describing the actual type by
which the formal is replaced for this instantiation.
All template type parameter entries should appear before
the entries describing the instantiated formal parameters
to the function.
.LI
.IX compilation units
If the compiler has generated a special compilation unit
to hold the template instantiation and that compilation unit
has a different name
from the compilation unit containing the template definition,
the name attribute for the debugging entry representing
that compilation unit should be empty or omitted.
.LI
.IX declarations, coordinates
If the subprogram entry representing the template instantiation
or any of its child entries
contain declaration coordinate attributes, those attributes
should refer to the source for the template definition, not
to any source generated artificially by the compiler for this
instantiation.
.LE
.H 3 "Inline Subroutines"
.IX subroutines, inline
A declaration or a definition of an inlinable subroutine
is represented by a debugging information entry with the tag
.Cf DW_TAG_subprogram .
The entry for a subroutine that is explicitly declared
to be available for inline expansion or that was expanded inline
implicitly by the compiler has a
.Cf DW_AT_inline 
attribute whose value is a constant.  The set of values 
for the 
.Cf DW_AT_inline
.nr aX \n(Fg+1
attribute is given in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) l
. 
Name	Meaning
_
DW_INL_not_inlined	Not declared inline nor inlined by the compiler
DW_INL_inlined	Not declared inline but inlined by the compiler
DW_INL_declared_not_inlined	Declared inline but not inlined by the compiler
DW_INL_declared_inlined	Declared inline and inlined by the compiler
.TE
.FG "Inline codes"
.DE
.H 4 "Abstract Instances"
For the remainder of this discussion,
any debugging information entry that is owned (either directly or
indirectly) by a debugging information entry that contains the 
.Cf DW_AT_inline
attribute will be referred to as an ``abstract instance entry.''
Any subroutine entry that contains a
.Cf DW_AT_inline 
attribute will be known as an ``abstract instance root.''
Any set of abstract instance entries that are all children (either directly
or indirectly) of some abstract instance root, together with the root itself,
will be known as an ``abstract instance tree.''
.P
A debugging information entry that is a member of an abstract instance
tree should not contain a
.Cf DW_AT_high_pc , 
.Cf DW_AT_low_pc , 
.Cf DW_AT_location ,
.Cf DW_AT_return_addr , 
.Cf DW_AT_start_scope ,
or
.Cf DW_AT_segment 
attribute.
.P
.I
It would not make sense to put these attributes
into abstract instance entries since
such entries do not represent actual (concrete) instances and thus
do not actually exist at run-time.
.P
.R
The rules for the relative location of entries belonging to abstract instance 
trees are exactly
the same as for other similar types of entries that are not abstract.
Specifically, the rule that requires that an entry representing a
declaration be a direct child of the entry representing the scope of
the declaration applies equally to both abstract and
non-abstract entries.  Also, the ordering rules for formal parameter entries,
member entries, and so on, all apply regardless of whether or not a given entry
is abstract.
.H 4 "Concrete Inlined Instances"
.IX subroutines, inlined
Each inline expansion of an inlinable subroutine is represented
by a debugging information entry with the tag
.Cf DW_TAG_inlined_subroutine .
Each such entry should be a direct child of the entry that represents the
scope within which the inlining occurs.
.P
Each inlined subroutine entry contains a 
.Cf DW_AT_low_pc
attribute, representing the address of the first
instruction associated with the given inline
expansion.   Each inlined subroutine entry also contains a
.Cf DW_AT_high_pc
attribute, representing the
address of the first location past the last instruction associated with
the inline expansion.
.P
For the remainder of this discussion,
any debugging information entry that is owned (either directly or indirectly)
by a debugging information entry with the tag 
.Cf DW_TAG_inlined_subroutine 
will be referred to as a ``concrete inlined instance entry.''
Any entry that has the tag 
.Cf DW_TAG_inlined_subroutine
will be known as
a ``concrete inlined instance root.''
Any set of concrete inlined instance entries that are all children (either
directly or indirectly) of some concrete inlined instance root, together
with the root itself, will be known as a ``concrete inlined instance
tree.''
.P
Each concrete inlined instance tree is uniquely associated with one (and
only one) abstract instance tree.
.P
.I
Note, however, that the reverse is not true.  Any given abstract instance
tree may be associated with several different concrete inlined instance
trees, or may even be associated with zero concrete inlined instance
trees.
.P
.R
Also, each separate entry within a given concrete inlined instance tree is
uniquely associated with one particular entry in the associated abstract
instance tree.  In other words, there is a one-to-one mapping from entries
in a given concrete inlined instance tree to the entries in the associated
abstract instance tree.
.P
.I
Note, however, that the reverse is not true.  A given abstract instance
tree that is associated with a given concrete inlined instance tree
may (and quite probably will) contain more entries than the associated
concrete inlined instance tree (see below).
.R
.P
Concrete inlined instance entries do not have most of the attributes (except
for 
.Cf DW_AT_low_pc , 
.Cf DW_AT_high_pc ,
.Cf DW_AT_location ,
.Cf DW_AT_return_addr ,
.Cf DW_AT_start_scope
and
.Cf DW_AT_segment )
that such entries
would otherwise normally have.  In place of these omitted attributes,
each concrete inlined instance entry has a
.Cf DW_AT_abstract_origin 
attribute that 
may be used to obtain the missing information (indirectly) from
the associated abstract instance entry.  The value of the abstract
origin attribute is a reference to the associated abstract instance entry.
.P
For each pair of entries that are associated via a
.Cf DW_AT_abstract_origin
attribute, both members of the pair will have the same tag.  So, for
example, an entry with the tag 
.Cf DW_TAG_local_variable 
can only be associated
with another entry that also has the tag 
.Cf DW_TAG_local_variable.
The only exception to this rule is that the root of a concrete
instance tree (which must always have the tag 
.Cf DW_TAG_inlined_subroutine )
can only be associated with the root of its associated abstract
instance tree (which must have the tag
.Cf DW_TAG_subprogram ).
.P
In general, the structure and content of any given concrete
instance tree will be directly analogous to the structure and content
of its associated abstract instance tree.
There are two exceptions to this general rule however.
.AL
.LI
.IX anonymous types
No entries representing anonymous types are ever made a part
of any concrete instance inlined tree.
.LI
.IX members
No entries representing members of structure, union or class
types are ever made a part of any concrete inlined instance tree.
.LE
.P
.I
Entries that represent members and anonymous types are omitted from concrete
inlined instance trees because they would simply be redundant duplicates of
the corresponding entries in the associated abstract instance trees.  If
any entry within a concrete inlined instance tree needs to refer to an
anonymous type that was declared within the scope of the
relevant inline function, the reference should simply refer to the abstract
instance entry for the given anonymous type.
.R
.P
.IX declarations, coordinates
If an entry within a concrete inlined instance tree contains
attributes describing the declaration coordinates of
that entry,
then those attributes should refer to the file, line and column
of the original declaration of the subroutine, not to the
point at which it was inlined.
.H 4 "Out-of-Line Instances of Inline Subroutines"
.IX subroutines, out-of-line
Under some conditions, compilers may need to generate concrete executable
instances of inline subroutines other than at points where those subroutines
are actually called.  For the remainder of this discussion,
such concrete instances of inline subroutines will
be referred to as ``concrete out-of-line instances.'' 
.P
.I
In C++, for example, taking the address of a function declared to be inline
can necessitate the generation of a concrete out-of-line
instance of the given function.
.P
.R
The DWARF representation of a concrete out-of-line instance of an inline
subroutine is essentially the same as for a concrete inlined instance of
that subroutine (as described in the preceding section).  The representation
of such a concrete out-of-line instance makes use of 
.Cf DW_AT_abstract_origin
attributes in exactly the same way as they are used for a concrete inlined
instance (that is, as references to corresponding entries 
within the associated
abstract instance tree) and, as for concrete instance trees, the
entries for anonymous types and for all members are omitted.
.P
The differences between the DWARF representation of a concrete out-of-line
instance of a given subroutine and the representation of a concrete inlined
instance of that same subroutine are as follows:
.AL
.LI
The root entry for a concrete out-of-line instance of a given
inline subroutine has the same tag as does its associated
(abstract) inline subroutine entry (that is, it does not have the
tag 
.Cf DW_TAG_inlined_subroutine ).
.LI
The root entry for a concrete out-of-line instance tree is
always directly owned by the same parent entry that
also owns the root entry of the associated abstract instance.
.LE
.H 2   "Lexical Block Entries"
.I
.IX lexical blocks
A lexical block is a bracketed sequence of source statements that may
contain any number of declarations.  In some languages (C and C++)
blocks can be nested within other blocks to any depth.  
.P
.R
A lexical block is represented by a debugging information entry
with the tag
.Cf DW_TAG_lexical_block .
.P
The lexical block entry has a 
.Cf DW_AT_low_pc
attribute whose value is the
relocated address of the first machine instruction generated for the lexical
block.
The lexical block entry also has a 
.Cf DW_AT_high_pc
attribute whose value is the
relocated address of the first location
past the last machine instruction generated for the lexical block.
.P
If a name has been given to the lexical block in the source program,
then the corresponding lexical block entry has a 
.Cf DW_AT_name 
attribute
whose value is a null-terminated string containing the name of the
lexical block as it appears in the source program.  
.P
.I
This is not the
same as a C or C++ label (see below).
.R
.P
The lexical block entry owns debugging information entries that
describe the declarations within that lexical block.
There is one such debugging information entry for each local declaration
of an identifier or inner lexical block.
.H 2   "Label Entries"
.I
.IX labels
A label is a way of identifying a source statement.  A labeled statement
is usually the target of one or more ``go to'' statements.
.P
.R
A label is represented by a debugging information entry
with the tag 
.Cf DW_TAG_label .
The entry for a label should be owned by
the debugging information entry representing the scope within which the name
of the label could be legally referenced within the source program.
.P
The label entry has a 
.Cf DW_AT_low_pc
attribute whose value is the
relocated address of the first machine instruction generated for the
statement identified by the label in the source program.
The label entry also has a 
.Cf DW_AT_name 
attribute
whose value is a null-terminated string containing the name of the
label as it appears in the source program.
.H 2 "With Statement Entries"
.I
.IX with statements
.IX Pascal
.IX Modula2
Both Pascal and Modula support the concept of a ``with'' statement. 
The with statement specifies a sequence of executable statements
within which the fields of a record variable may be referenced, unqualified
by the name of the record variable.
.P
.R
A with statement is represented by a debugging information entry with
the tag
.Cf DW_TAG_with_stmt .
A with statement entry has a 
.Cf DW_AT_low_pc
attribute whose value is the relocated
address of the first machine instruction generated for the body of
the with statement.  A with statement entry also has a 
.Cf DW_AT_high_pc
attribute whose value is the relocated
address of the first location after the last machine instruction generated for the body of
the statement.
.P
The with statement entry has a 
.Cf DW_AT_type
attribute, denoting
the type of record whose fields may be referenced without full qualification
within the body of the statement.  It also has a
.Cf DW_AT_location
attribute, describing how to find the base address
of the record object referenced within the body of the with statement.
.H 2 "Try and Catch Block Entries"
.I
.IX C++ %caa
.IX exceptions
.IX try blocks
.IX catch blocks
In C++ a lexical block may be designated as a ``catch block.''
A catch block is an exception handler that handles exceptions
thrown by an immediately preceding ``try block.''  A catch block
designates the type of the exception that it can handle.
.R
.P
A try block is represented by a debugging information entry
with the tag
.Cf DW_TAG_try_block .
A catch block is represented by a debugging information entry
with the tag
.Cf DW_TAG_catch_block .
Both try and catch block entries contain a
.Cf DW_AT_low_pc
attribute whose value is the
relocated address of the first machine instruction generated for that 
block.  These entries also contain a
.Cf DW_AT_high_pc
attribute whose value is the
relocated address of the first location
past the last machine instruction generated for that block.
.P
Catch block entries have at least one child entry,
an entry representing the type of exception accepted
by that catch block.  This child entry will have one of the tags
.Cf DW_TAG_formal_parameter
or
.Cf DW_TAG_unspecified_parameters ,
.IX parameters, formal
.IX parameters, unspecified
and will have the same form as other parameter entries.
.P
The first sibling of each try block entry will be a catch block
entry.
.OP
.H 1 "DATA OBJECT AND OBJECT LIST ENTRIES"
This section presents the debugging information entries that
describe individual data objects: variables, parameters and
constants, and lists of those objects that may be grouped
in a single declaration, such as a common block.
.H 2   "Data Object Entries"
.IX variables
.IX parameters, formal
.IX constants
Program variables, formal parameters and constants are represented
by debugging information entries with the tags
.Cf DW_TAG_variable ,
.Cf DW_TAG_formal_parameter
and
.Cf DW_TAG_constant ,
respectively. 
.P
.I
The tag
.Cf DW_TAG_constant
is used for languages that distinguish between variables
that may have constant value and true named constants.
.R
.P
The debugging information entry for a program variable, formal
parameter or constant may have the following attributes:
.AL
.LI
A 
.Cf DW_AT_name 
attribute whose value is a null-terminated
string containing the data object name as it appears in the source program.
.P
.IX anonymous unions
.IX unions, anonymous
.IX C++ %caa
If a variable entry describes a C++ anonymous union, the name
attribute is omitted or consists of a single zero byte.
.LI
If the name of a variable is visible outside of its enclosing 
compilation unit, the variable entry has a
.Cf DW_AT_external
.IX declarations, external
attribute, whose value is a flag.
.I
.P
.IX members, static data
The definitions of C++ static data members
of structures or classes are represented by variable entries flagged
as external.
.IX C %c
.IX C++ %caa
Both file static and local variables in C and C++ are represented 
by non-external variable entries.
.R
.LI
A
.Cf DW_AT_location
attribute, whose value describes the location of a variable or parameter
at run-time.
.P
.IX declarations, non-defining
A data object entry representing a non-defining declaration of the object
will not have a location attribute, and will have the
.Cf DW_AT_declaration
attribute.
.P
In a variable entry representing the definition of the variable
(that is, with no
.Cf DW_AT_declaration
attribute)
if no location attribute is present, or if
the location attribute is present but describes
a null entry (as described in section 2.4), the variable
is assumed to exist in the source code but not in the executable
program (but see number 9, below).
.IX optimized code
.P
The location of a variable may be further specified with a
.Cf DW_AT_segment
attribute, if appropriate.
.IX segmented address space
.IX address space, segmented
.LI
A 
.Cf DW_AT_type
attribute describing the type of the variable, constant or formal
parameter.
.LI
.IX members, static data
.IX declarations, defining
If the variable entry represents the defining declaration for a C++ static
data member of a structure, class or union, the entry has a
.Cf DW_AT_specification
attribute, whose value is a reference to the debugging information
entry representing the declaration of this data member.  The
referenced entry will be a child of some class, structure or
union type entry.
.IX classes
.IX structures
.IX unions
.P
Variable entries containing the 
.Cf DW_AT_specification
attribute do not need to duplicate information provided by the
declaration entry referenced by the specification attribute.
In particular, such variable entries do not need to contain
attributes for the name or type of the data member whose
definition they represent.
.LI
.I
Some languages distinguish between parameters whose value in the
calling function can be modified by the callee (variable parameters), 
and parameters whose value in the calling function cannot be modified
by the callee (constant parameters).
.P
.R
If a formal parameter entry represents a parameter whose value
in the calling function may be modified by the callee, that entry
may have a
.Cf DW_AT_variable_parameter
attribute, whose value is a flag.  The absence of this attribute
implies that the parameter's value in the calling function cannot
be modified by the callee.
.IX parameters, variable
.LI
.I
Fortran90 has the concept of an optional parameter.
.IX Fortran90
.P
.R
.IX parameters, optional
If a parameter entry represents an optional parameter, it has a
.Cf DW_AT_is_optional
attribute, whose value is a flag.
.LI
.IX parameters, default value
A formal parameter entry describing a formal parameter that has a default
value may have a
.Cf DW_AT_default_value
attribute.  The value of this attribute is a reference to the
debugging information entry for a variable or subroutine.  The
default value of the parameter is the value of the variable (which
may be constant) or the value returned by the subroutine.  If the
value of the
.Cf DW_AT_default_value
attribute is 0, it means that no default value has been specified.
.LI
.IX constants
An entry describing a variable whose value is constant
and not represented by an object in the address space of the program,
or an entry describing a named constant,
does not have a location attribute.  Such entries have a
.Cf DW_AT_const_value
attribute, whose value may be a string or any of the constant
data or data block forms, as appropriate for the representation
of the variable's value.  The value of this attribute is the actual
constant value of the variable, represented as it would be
on the target architecture.
.LI
.IX scope
.IX declarations, scope
If the scope of an object begins sometime after the low pc value
for the scope most closely enclosing the object, the
object entry may have a
.Cf DW_AT_start_scope
attribute.  The value of this attribute is the offset in bytes of the beginning
of the scope for the object from the low pc value of the debugging
information entry that defines its scope.
.P
.I
The scope of a variable may begin somewhere in the middle of a lexical
block in a language that allows executable code in a 
block before a variable declaration, or where one declaration
containing initialization code may change the scope of a subsequent
declaration.  For example, in the following C code:
.DS
\f(CWfloat x = 99.99;

int myfunc()
{
	float f = x;
	float x = 88.99;

	return 0;
}\fP
.DE
.P
ANSI-C scoping rules require that the value of the variable \f(CWx\fP
assigned to the variable \f(CWf\fP in the initialization sequence
is the value of the global variable \f(CWx\fP, rather than the local \f(CWx\fP,
because the scope of the local variable \f(CWx\fP only starts after the full
declarator for the local \f(CWx\fP.
.R
.LE
.P
.H 2 "Common Block Entries"
.IX common blocks
.IX Fortran
A Fortran common block may be described by a debugging information
entry with the tag
.Cf DW_TAG_common_block .
The common block entry has a
.Cf DW_AT_name 
attribute whose value is a null-terminated
string containing the common block name as it appears in the source program.
It also has a
.Cf DW_AT_location
attribute whose value describes the location of the beginning of the
common block.  The common block entry owns debugging information
entries describing the variables contained within the common block.
.H 2 "Imported Declaration Entries"
.I 
.IX declarations, imported
.IX imports
Some languages support the concept of importing into a given
module declarations made in a different module.
.R
.P
An imported declaration is represented by a debugging information
entry with the tag
.Cf DW_TAG_imported_declaration .
The entry for the imported declaration has a 
.Cf DW_AT_name 
attribute whose value
is a null-terminated string containing the name of the entity
whose declaration is being imported as it appears in the source
program.  The imported declaration entry also has a
.Cf DW_AT_import
attribute, whose value is a reference to the debugging information
entry representing the declaration that is being imported.
.H 2 "Namelist Entries"
.I
.IX namelists
.IX Fortran90
At least one language, Fortran90, has the concept of a namelist.
A namelist is an ordered list of the names of some set of declared objects.
The namelist object itself may be used as a replacement for the 
list of names in various contexts.
.R
.P
A namelist is represented by a debugging information entry with
the tag
.Cf DW_TAG_namelist .
If the namelist itself has a name, the namelist entry has a
.Cf DW_AT_name
attribute, whose value is a null-terminated string containing the namelist's
name as it appears in the source program.
.P
Each name that is part of the namelist is represented by a debugging
information entry with the tag 
.Cf DW_TAG_namelist_item .
Each such entry is a child of the namelist entry, and all of
the namelist item entries for a given namelist are ordered as were
the list of names they correspond to in the source program.
.P
Each namelist item entry contains a
.Cf DW_AT_namelist_item
attribute whose value is a reference to the debugging information
entry representing the declaration of the item whose name
appears in the namelist.
.OP
.H 1 "TYPE ENTRIES"
This section presents the debugging information entries
that describe program types: base types, modified types
and user-defined types.
.P
If the scope of the declaration of a named type begins sometime after 
.IX scope
.IX declarations, scope
the low pc value
for the scope most closely enclosing the declaration, the
declaration may have a
.Cf DW_AT_start_scope
attribute.  The value of this attribute is the offset in bytes of the beginning
of the scope for the declaration from the low pc value of the debugging
information entry that defines its scope.
.H 2 "Base Type Entries"
.I
.IX base types
.IX types, base
A base type is a data type that is not defined in terms of
other data types.  Each programming language has a set of
base types that are considered to be built into that language.
.R
.P
A base type is represented by a debugging information entry
with the tag
.Cf DW_TAG_base_type .
A base type entry has a 
.Cf DW_AT_name 
attribute whose value is a null-terminated
string describing the name of the base type as recognized by
the programming language of the compilation unit containing
the base type entry.
.P
A base type entry also has a
.Cf DW_AT_encoding
attribute describing how the base type is encoded and is
to be interpreted.  The value of this attribute is a constant.
The set of values and their meanings for the 
.Cf DW_AT_encoding
.nr aX \n(Fg+1
attribute is given in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) l
. 
Name	Meaning
_
DW_ATE_address	linear machine address
DW_ATE_boolean	true or false
DW_ATE_complex_float	complex floating-point number
DW_ATE_float	floating-point number
DW_ATE_signed	signed binary integer
DW_ATE_signed_char	signed character
DW_ATE_unsigned	unsigned binary integer
DW_ATE_unsigned_char	unsigned character
.TE
.FG "Encoding attribute values"
.DE
.P
All encodings assume the representation that is ``normal'' for
the target architecture.
.P
A base type entry has a
.Cf DW_AT_byte_size
attribute, whose value is a constant,
describing the size in bytes of the storage
unit used to represent an object of the given type.
.P
If the value of an object of the given type does not
fully occupy the storage unit described by the byte size attribute,
the base type entry may have a
.Cf DW_AT_bit_size
attribute and a
.Cf DW_AT_bit_offset
attribute, both of whose values are constants.
The bit size attribute describes the actual size in bits used
to represent a value of the given type.  The bit offset
attribute describes the offset in bits of the high order
bit of a value of the given type from the high order bit
of the storage unit used to contain that value.
.I
.P
For example, the C type 
.Cf int
on a machine that uses 32-bit integers would be
represented by a base type entry with a name
attribute whose value was ``\f(CWint\fP,'' an
encoding attribute whose value was 
.Cf DW_ATE_signed
and a byte size attribute whose value was
.Cf 4 .
.R
.H 2 "Type Modifier Entries"
.IX type modifiers
.IX types, modifiers
A base or user-defined type may be modified in different
ways in different languages.  A type modifier is represented
in DWARF by a debugging information entry with one of the
.nr aX \n(Fg+1
tags given in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) l
. 
Tag	Meaning
_
DW_TAG_const_type	C or C++ const qualified type
DW_TAG_packed_type	Pascal packed type
DW_TAG_pointer_type	The address of the object whose type is being modified
DW_TAG_reference_type	A C++ reference to the object whose type is being modified
DW_TAG_volatile_type	C or C++ volatile qualified type
.TE
.FG "Type modifier tags"
.DE
.P
.IX types, constant
.IX types, packed
.IX types, volatile
.IX types, pointer
.IX types, reference
Each of the type modifier entries has a
.Cf DW_AT_type
attribute, whose value is a reference to a debugging information
entry describing a base type, a user-defined type or another type 
modifier.
.P
A modified type entry describing a pointer or reference type
may have a 
.IX addresses, class
.Cf DW_AT_address_class 
attribute
to describe how objects having the given pointer or reference type
ought to be dereferenced.
.P
When multiple type modifiers are chained together to modify
a base or user-defined type, they are ordered as if part of
a right-associative expression involving the base or user-defined
type.
.I
.P
As examples of how type modifiers are ordered, take the following
C declarations:
.R
.DS
.ta .5i +.5i +.5i +.5i +.5i +.5i +.5i +.5i
\f(CWconst char * volatile p;\fP
        \fIwhich represents a volatile pointer to a constant character.\fP
        \fIThis is encoded in DWARF as:\fP
        \f(CWDW_TAG_volatile_type \(-> 
		DW_TAG_pointer_type \(->
			DW_TAG_const_type \(->
				DW_TAG_base_type\fP

\f(CWvolatile char * const p;\fP
        \fIon the other hand, represents a constant pointer
        to a volatile character.\fP
        \fIThis is encoded as:\fP
        \f(CWDW_TAG_const_type \(-> 
		DW_TAG_pointer_type \(->
			DW_TAG_volatile_type \(->
				DW_TAG_base_type\fP

.DE
.R
.H 2 "Typedef Entries"
.IX typedefs
Any arbitrary type named via a typedef is represented
by a debugging information entry with the tag 
.Cf DW_TAG_typedef .
The typedef entry has a 
.Cf DW_AT_name 
attribute whose value is a null-terminated
string containing the name of the typedef as it appears in the
source program.
The typedef entry also contains a 
.Cf DW_AT_type
attribute.
.P
If the debugging information entry for a typedef represents a 
declaration of the type that is not also a definition,
it does not contain a type attribute.
.IX declarations, non-defining
.H 2     "Array Type Entries"
.I
.IX arrays
Many languages share the concept of an ``array,'' which is a
table of components of identical type.
.P
.R
An array type is represented by a debugging information entry with
the tag 
.Cf DW_TAG_array_type .
.P
If a name has been given to the array type in the source program, then the
corresponding array type entry has a 
.Cf DW_AT_name 
attribute whose value is a
null-terminated string containing the array type name as it appears in the
source program.
.P
.IX arrays, ordering
The array type entry describing a multidimensional array may have a
.Cf DW_AT_ordering
attribute whose constant value is interpreted to mean either
row-major or column-major ordering of array elements.
The set of values and their meanings for the ordering attribute
.nr aX \n(Fg+1
are listed in Figure \n(aX.
If no ordering attribute is present, the default ordering for
the source language (which is indicated by the
.Cf DW_AT_language
attribute of the enclosing compilation unit entry)
is assumed.
.DF
.TS
box center;
lf(CW)
. 
DW_ORD_col_major
DW_ORD_row_major
.TE
.FG "Array ordering"
.DE
.P
The ordering attribute may optionally appear on one-dimensional arrays; it
will be ignored.
.P
An array type entry has a 
.Cf DW_AT_type
attribute describing the type
of each element of the array.
.P
.IX arrays, stride
If the amount of storage allocated to hold each element of an object of
the given array type is different from the amount of storage that is normally
allocated to hold an individual object of the indicated element type, then
the array type entry has a
.Cf DW_AT_stride_size 
attribute, whose constant value
represents the size in bits of each element of the array.
.P
If the size of the entire array can be determined statically at compile
time, the array type entry may have a
.Cf DW_AT_byte_size 
attribute, whose constant value represents the total size in bytes of an
instance of the array type.
.P
.I
Note that if the size of the array can be determined statically at
compile time, this value can usually be computed by multiplying
the number of array elements by the size of each element.
.P
.R
Each array dimension is described by a debugging information
entry with either the tag
.IX subranges
.IX enumerations
.IX arrays, dimensions
.Cf DW_TAG_subrange_type
or the tag
.Cf DW_TAG_enumeration_type .
These entries are children of the array type entry and are
ordered to reflect the appearance of the dimensions in the source
program (i.e. leftmost dimension first, next to leftmost second,
and so on).
.P
.I
.IX C %c
In languages, such as ANSI-C, in which there is no concept of a
``multidimensional array,'' 
an array of arrays may be represented by a debugging information entry
for a multidimensional array.
.R
.H 2     "Structure, Union, and Class Type Entries"
.I
The languages C, C++, and Pascal, among others, 
allow the programmer to define types that
are collections of related components.  In C and C++, these collections are
called ``structures.''  In Pascal, they are called ``records.''  The components
may be of different types.  The components are called ``members'' in C and
C++, and ``fields'' in Pascal.
.P
.IX structures
.IX classes
.IX unions
.IX records
.IX C %c
.IX C++ %caa
.IX Pascal
The components of these collections each exist in their own space in
computer memory.  The components of a C or C++ ``union'' all coexist in
the same memory.
.P
Pascal and other languages have a ``discriminated union,'' also called a
.IX variants
.IX discriminated unions
``variant record.''  Here, selection of a number of alternative substructures
(``variants'') is based on the value of a component that is not part of any of
those substructures (the ``discriminant'').
.P
Among the languages discussed in this document,
the ``class'' concept is unique to C++.  A class is similar to a structure.
A C++ class or structure may have ``member functions'' which are subroutines
that are within the scope of a class or structure.
.R
.H 3 "General Structure Description"
Structure, union, and class types are represented by 
debugging information entries with the tags
.Cf DW_TAG_structure_type ,
.Cf DW_TAG_union_type 
and 
.Cf DW_TAG_class_type ,
respectively.
If a name has been given to the structure, union, or class in the source
program, then the corresponding structure type, union type, or class type
entry has a 
.Cf DW_AT_name 
attribute whose value is a null-terminated string
containing the type name as it appears in the source program.
.P
If the size of an instance of the
structure type, union type, or class type entry can be determined
statically at compile time, the entry has a
.Cf DW_AT_byte_size 
attribute whose constant value is the number of bytes required to
hold an instance of the structure, union, or class, and any padding bytes.
.I
.P
.IX structures, incomplete
.IX classes, incomplete
.IX unions, incomplete
For C and C++, an incomplete structure, union or class type is represented
by a structure, union or class entry that does not have
a byte size attribute and that has a
.Cf DW_AT_declaration
attribute.
.R
.P
The members of a structure, union, or class are represented by
debugging information entries that are owned by the corresponding
structure type, union type, or class type entry and appear in the same
order as the corresponding declarations in the source program.
.P
.I
.IX declarations, defining
.IX members, static data
.IX members, data
.IX members, functions
Data member declarations occurring within the declaration of a structure,
union or class type are considered to be ``definitions'' of those members,
with the exception of C++ ``static'' data members, whose definitions
appear outside of the declaration of the enclosing structure, union
or class type.  Function member declarations appearing within a structure,
union or class type declaration are definitions only if the body
of the function also appears within the type declaration.
.R
.P
.IX declarations, non-defining
If the definition for a given member of the structure, union or class
does not appear within the body of the declaration, that member
also has a debugging information entry describing its definition.
That entry will have a
.Cf DW_AT_specification
attribute referencing 
the debugging entry owned by the
body of the structure, union or class debugging entry and representing
a non-defining declaration of the data or function member.  The 
referenced entry will
not have information about the location of that member (low and high
pc attributes for function members, location descriptions for data
members) and will have a
.Cf DW_AT_declaration
attribute.
.H 3 "Derived Classes and Structures"
.IX classes, derived
.IX structures, derived
.IX inheritance
The class type or structure type entry that describes a derived class 
or structure owns debugging information entries describing each of
the classes or structures it is derived from, ordered as they were
in the source program.  Each such entry has the tag
.Cf DW_TAG_inheritance .
.P
An inheritance entry has a 
.Cf DW_AT_type
attribute whose
value is a reference to the debugging information entry describing
the structure or class from which the parent structure or class
of the inheritance entry is derived.  It also has a
.Cf DW_AT_data_member_location
attribute, whose value is a location description describing
the location of the beginning of
the data members contributed to the entire class by this
subobject relative to the beginning address of the data members of the
entire class.
.P
.IX accessibility
.IX virtuality
.IX classes, virtual base
An inheritance entry may have a
.Cf DW_AT_accessibility
attribute.
If no accessibility attribute is present,
private access is assumed.
If the structure or class referenced by the inheritance entry serves
as a virtual base class, the inheritance entry has a
.Cf DW_AT_virtuality
attribute.
.P 
.I
In C++, a derived class may contain access declarations that
change the accessibility of individual class members from
the overall accessibility specified by the inheritance declaration.
A single access declaration may refer to a set of overloaded
names.
.R
.P
If a derived class or structure contains access declarations,
.IX access declarations
.IX C++ %caa
each such declaration may be represented by a debugging information
entry with the tag
.Cf DW_TAG_access_declaration .
Each such entry is a child of the structure or class type entry.
.P
An access declaration entry has a 
.Cf DW_AT_name 
attribute, whose value
is a null-terminated string representing the name used in the
declaration in the source program, including any class or structure
qualifiers.
.P
An access declaration entry also has a 
.Cf DW_AT_accessibility
attribute
describing the declared accessibility of the named entities.
.H 3 "Friends"
.IX friends
.IX classes, friends
Each ``friend'' declared by
a structure, union or class type may be represented by
a debugging information entry that is a child of the structure,
union or class type entry; the friend entry has the tag
.Cf DW_TAG_friend.
.P
A friend entry has a
.Cf DW_AT_friend 
attribute, whose value is a reference to the debugging information
entry describing the declaration of the friend.
.H 3 "Structure Data Member Entries"
.IX members, data
A data member (as opposed to a member function) is represented by
a debugging information entry with the tag
.Cf DW_TAG_member .
The member entry for a named member has a 
.Cf DW_AT_name 
attribute
whose value is a null-terminated string containing the member name
as it appears in the source program.  If the member entry describes
a C++ anonymous union, the name attribute is omitted or consists
of a single zero byte.
.IX unions, anonymous
.IX anonymous unions
.P
The structure data member entry has a 
.Cf DW_AT_type
attribute
to denote the type of that member.
.P
If the member entry is defined in the structure or class body, it has a
.Cf DW_AT_data_member_location
attribute whose value is a location
description that describes the location of that
member relative to the base address of the structure, union, or class that
most closely encloses the corresponding member declaration.
.I
.P
.IX locations, expressions
.IX locations, descriptions
The addressing expression represented by the location 
description for a structure data member expects the base address
of the structure data member to be on the expression stack
before being evaluated.
.P
.IX unions
The location description for a data member of a union may be omitted,
since all data members of a union begin at the same address.
.R
.P
.IX bit fields
.IX members, bit fields
If the member entry describes a bit field, then that entry has the following
attributes:
.AL
.LI
A
.Cf DW_AT_byte_size
attribute whose constant value is the number of bytes that
contain an instance of the bit field and any padding bits.
.P
.I
The byte size attribute may be omitted if the size of the object containing
the bit field can be inferred from the type attribute of the data
member containing the bit field.
.R
.LI
A
.Cf DW_AT_bit_offset
attribute whose constant value is the number of bits
to the left of the leftmost (most significant) bit of the bit field value.
.LI
A
.Cf DW_AT_bit_size
attribute whose constant value is the number of bits occupied
by the bit field value.
.LE
.P
The location description for a bit field calculates the address of
an anonymous object containing the bit field.  The address is
relative to the structure, union, or class that
most closely encloses the bit field declaration.  The number
of bytes in this anonymous object is the value of the byte
size attribute of the bit field.  The offset (in bits)
from the most significant bit of the
anonymous object to the most significant bit of the bit field is the
value of the bit offset attribute.
.I
.P
For example, take one possible representation of the following
structure definition in both big and little endian byte orders:
.DS
\f(CW
struct S {
	int   j:5;
	int   k:6;
	int   m:5;
	int   n:8;
};\fP
.DE
.P
In both cases, the location descriptions for the debugging information
entries for \f(CWj\fP, \f(CWk\fP, \f(CWm\fP and \f(CWn\fP
describe the address of
the same 32-bit word that contains all three members.  
(In the big-endian case,
the location description addresses the most significant byte, in
the little-endian case, the least significant). 
The following diagram shows the structure layout and lists the bit
offsets for each case.  The offsets
are from the most significant bit of the object addressed by the location
description. 
.PS
bitht = .3
boxht = bitht
bitwid = .11
nibwid = .75 * bitwid
bytewid = 8 * bitwid
boxwid = bytewid
define nibble X	# nibble(len, "label", hi-left, hi-right, lo-left, lo-right, any)
N:	box width $1*nibwid $2 $7
	{ if $3 >= 0 then % "\s-4\|$3\s0" at N.w + (0,bitht/3) ljust %
	} # curly on separate line for pic bug
	{ if $4 >= 0 then % "\s-4\|$4\s0" at N.e + (0,bitht/3) rjust %
	} 
	{ if $5 >= 0 then % "\s-4\|$5\s0" at N.w - (0,bitht/3) ljust %
	}
	{ if $6 >= 0 then % "\s-4$6\|\s0" at N.e - (0,bitht/3) rjust %
	}
X
define tbox X # tbox(width,"label", any)
T:	box width $1*nibwid ht 1/6 $3 invis
	{ $2 at T.w ljust
	}
X
.PE
.DS
.PS
	down
H:	tbox(20,"Bit Offsets:")
	tbox(20,"\f(CW    j:0\fP")
	tbox(20,"\f(CW    k:5\fP")
	tbox(20,"\f(CW    m:11\fP")
	tbox(20,"\f(CW    n:16\fP")
	right
H:	tbox(32, "Big-Endian", with .w at H.e)
H:	nibble(5,"\f(CWj\fP",0,-1,31,-1,with .nw at H.sw)
H:	nibble(6,"\f(CWk\fP",-1,-1,26,-1)
H:	nibble(5,"\f(CWm\fP",-1,-1,20,-1)
H:	nibble(8,"\f(CWn\fP",-1,-1,15,-1)
H:	nibble(8,"\fIpad\fP",-1,-1,7,0)
.PE
.DE
.DS
.PS
	down
H:	tbox(20,"Bit Offsets:")
	tbox(20,"\f(CW    j:27\fP")
	tbox(20,"\f(CW    k:21\fP")
	tbox(20,"\f(CW    m:16\fP")
	tbox(20,"\f(CW    n:8\fP")
	right
H:	tbox(32, "Little-Endian", with .w at H.e)
H:	nibble(8,"\f2pad\fP",-1,-1,31,-1, with .nw at H.sw)
H:	nibble(8,"\f(CWn\fP",-1,-1,23,-1)
H:	nibble(5,"\f(CWm\fP",-1,-1,15,-1)
H:	nibble(6,"\f(CWk\fP",-1,-1,10,-1)
H:	nibble(5,"\f(CWj\fP",-1,0,4,0)
.PE
.DE
.R
.H 3 "Structure Member Function Entries"
.IX subroutines, members
.IX members, functions
.IX members, locations
A member function is represented in the debugging information by a
debugging information entry with the tag 
.Cf DW_TAG_subprogram .
The member function entry may contain the same attributes and follows
the same rules as non-member global subroutine entries (see section 3.3).
.P
.IX virtuality
.IX virtual functions
If the member function entry describes a virtual function, then that entry
has a 
.Cf DW_AT_virtuality 
attribute.
.P
An entry for a virtual function also has a
.Cf DW_AT_vtable_elem_location
attribute whose value contains a location
description yielding the address of the slot for the function
within the virtual function table for the enclosing class or structure.
.P
.IX declarations, defining
If a subroutine entry represents the defining declaration
of a member function and that definition appears outside
of the body of the enclosing class or structure declaration,
the subroutine entry has a
.Cf DW_AT_specification
attribute, whose value is a reference to the debugging information
entry representing the declaration of this function member.  The
referenced entry will be a child of some class or structure
type entry.
.P
Subroutine entries containing the 
.Cf DW_AT_specification
attribute do not need to duplicate information provided by the
declaration entry referenced by the specification attribute.
In particular, such entries do not need to contain
attributes for the name or return type of the function member whose
definition they represent.
.H 3 "Class Template Instantiations"
.I
.IX C++ %caa
.IX templates
In C++ a class template is a generic
definition of a class type that
is instantiated differently when an instance of the class
is declared or defined.  The generic description of the class
may include both parameterized types and parameterized constant
values.  DWARF does not represent the generic
template definition, but does represent each instantiation.
.R
.P
A class template instantiation is represented by a debugging information
with the tag
.Cf DW_TAG_class_type .
With four exceptions,
such an entry will contain the same attributes and have the same
types of child entries as would an entry for a class type defined 
explicitly using the instantiation types and values.
The exceptions are:
.AL
.LI 
Each formal parameterized type declaration appearing in the
template definition is represented by a debugging information entry
with the tag 
.Cf DW_TAG_template_type_parameter .
Each such entry has a 
.Cf DW_AT_name 
attribute, whose value is a null-terminated
string containing the name of the formal type parameter as it
appears in the source program.  The template type parameter
entry also has a 
.Cf DW_AT_type
attribute describing the actual type by
which the formal is replaced for this instantiation.
.LI
Each formal parameterized value declaration appearing
in the templated definition is represented by a debugging information
entry with the tag
.Cf DW_TAG_template_value_parameter .
Each such entry has a 
.Cf DW_AT_name 
attribute, whose value is a null-terminated
string containing the name of the formal value parameter as it
appears in the source program.  The template value parameter
entry also has a 
.Cf DW_AT_type
attribute describing the type of the parameterized
value.  Finally, the template value parameter entry has a
.Cf DW_AT_const_value
attribute, whose value is the actual constant value of the value
parameter for this instantiation as represented on the target
architecture.
.LI
.IX compilation units
If the compiler has generated a special compilation unit
to hold the template instantiation and that compilation unit
has a different name
from the compilation unit containing the template definition,
the name attribute for the debugging entry representing
that compilation unit should be empty or omitted.
.LI
.IX declarations, coordinates
If the class type entry representing the template instantiation
or any of its child entries
contain declaration coordinate attributes, those attributes
should refer to the source for the template definition, not
to any source generated artificially by the compiler.
.LE
.H 3 "Variant Entries"
.IX variants
.IX discriminated unions
A variant part of a structure is represented by a debugging
information entry with the tag
.Cf DW_TAG_variant_part
and is owned by the corresponding structure type
entry. 
.P
.IX discriminants
If the variant part has a discriminant, the discriminant is represented
by a separate debugging information entry which is a child of 
the variant part entry.  This entry has the form of a structure data member
entry.
The variant part entry will have a
.Cf DW_AT_discr
attribute whose value is a
reference to the member entry for the discriminant.  
.P
If the variant part
does not have a discriminant (tag field), the variant part entry has a 
.Cf DW_AT_type
attribute to represent the tag type.
.P
Each variant of a particular variant part is represented by a debugging
information entry with the tag
.Cf DW_TAG_variant 
and is a child of the variant part entry.  The value that selects a 
given variant may be represented in one of three ways.  The
variant entry may have a
.Cf DW_AT_discr_value
attribute whose value represents a single case label.  
The value of this attribute
is encoded as an LEB128 number.  The number is signed if the tag
type for the variant part containing this variant is
a signed type.   The number is unsigned if the tag type is an unsigned type.
.P
Alternatively, the variant entry may contain a
.Cf DW_AT_discr_list
attribute, whose value represents a list of discriminant values.
This list is represented by any of the block forms and may contain
a mixture of case labels and label ranges.  Each item on the list
is prefixed with a discriminant value descriptor that determines whether
the list item represents a single label or a label range.
A single case label is represented as an LEB128 
number as defined above
for the
.Cf DW_AT_discr_value
attribute.  A label range is represented by two LEB128 numbers,
the low value of the range followed by the high value.  Both values
follow the rules for signedness just described.  
The discriminant value descriptor is a constant that may have
.nr aX \n(Fg+1
one of the values given in Figure \n(aX.
.DF
.TS
center box;
lf(CW)
.
DW_DSC_label
DW_DSC_range
.TE
.FG "Discriminant descriptor values"
.DE
.P
If a variant entry has neither a
.Cf DW_AT_discr_value
attribute nor a
.Cf DW_AT_discr_list
attribute, or if it has a
.Cf DW_AT_discr_list
attribute with 0 size, the variant is a default variant.
.P
The components selected by a particular variant are represented
by debugging information entries owned by the corresponding variant
entry and appear in the same order as the corresponding declarations in
the source program.
.H 2     "Enumeration Type Entries"
.I
.IX enumerations
An ``enumeration type'' is a scalar that can assume one of a fixed number of
symbolic values.
.P
.R
An enumeration type is represented by a debugging information entry
with the tag
.Cf DW_TAG_enumeration_type .
.P
If a name has been given to the enumeration type in the source program,
then the corresponding enumeration type entry has a 
.Cf DW_AT_name 
attribute
whose value is a null-terminated string containing the enumeration type
name as it appears in the source program.
These entries also have a 
.Cf DW_AT_byte_size 
attribute whose
constant value is the number of bytes required to hold an
instance of the enumeration.
.P
Each enumeration literal is represented by a debugging information
entry with the tag
.Cf DW_TAG_enumerator .
Each such entry is a child of the enumeration type entry, and
the enumerator entries appear in the same order as the declarations of
the enumeration literals in the source program.
.P
Each enumerator entry has a 
.Cf DW_AT_name 
attribute, whose value is
a null-terminated string containing the name of the enumeration
literal as it appears in the source program.  Each enumerator
entry also has a
.Cf DW_AT_const_value
attribute, whose value is the actual numeric value of the enumerator
as represented on the target system.
.H 2     "Subroutine Type Entries"
.I
.IX subroutines, types
It is possible in C to declare pointers to subroutines that return a value
of a specific type.  In both ANSI C and C++, it is possible to declare
pointers to subroutines that not only return a value of a specific type,
but accept only arguments of specific types.  The type of such pointers
would be described with a ``pointer to'' modifier applied to a user-defined
type.  
.R
.P
A subroutine type is represented by a debugging information entry
with the tag
.Cf DW_TAG_subroutine_type .
If a name has been given to the subroutine type in the source program,
then the corresponding subroutine type entry has a 
.Cf DW_AT_name 
attribute
whose value is a null-terminated string containing the subroutine type
name as it appears in the source program.
.P
.IX subroutines, return types
If the subroutine type describes a function that returns a value, then
the subroutine type entry has a 
.Cf DW_AT_type
attribute
to denote the type returned by the subroutine.
If the types of the arguments are necessary to describe the subroutine type,
then the corresponding subroutine type entry owns debugging
information entries that describe the arguments.
These debugging information entries appear in the order
that the corresponding argument types appear in the source program.
.P
.I
.IX C %c
.IX subroutines, prototypes
In ANSI-C there is a difference between the types of functions
declared using function prototype style declarations and those
declared using non-prototype declarations.  
.P
.R
A subroutine entry
declared with a function prototype style declaration may have a
.Cf DW_AT_prototyped
attribute, whose value is a flag.
.P
Each debugging information entry
owned by a subroutine type entry has a tag whose value has one of
two possible interpretations.  
.AL
.LI
.IX parameters, formal
Each debugging information entry that is owned by a subroutine type entry and
that defines a single argument of a specific type has the tag
.Cf DW_TAG_formal_parameter .
.P
The formal parameter entry has a type attribute
to denote the type of the corresponding formal parameter.
.LI
The unspecified parameters of a variable parameter list are represented by a
debugging information entry owned by the subroutine type entry with the tag
.Cf DW_TAG_unspecified_parameters .
.IX parameters, unspecified
.LE
.H 2     "String Type Entries"
.I
.IX string types
.IX Fortran
A ``string'' is a sequence of characters that have specific semantics and
operations that separate them from arrays of characters.  
Fortran is one of
the languages that has a string type.
.R
.P
A string type is represented by a debugging information entry
with the tag 
.Cf DW_TAG_string_type .
If a name has been given to the string type in the source program,
then the corresponding string type entry has a 
.Cf DW_AT_name 
attribute
whose value is a null-terminated string containing the string type
name as it appears in the source program.
.P
The string type entry may have a
.Cf DW_AT_string_length
attribute whose value is a location description
yielding the location where the length of the string
is stored in the program.  The string type entry may also have a
.Cf DW_AT_byte_size 
attribute, whose constant value is the size in bytes of the data
to be retrieved from the location referenced by the string length
attribute.  If no byte size attribute is present, the size of the
data to be retrieved is the same as the size of an address on
the target machine.
.P
If no string length attribute is present, the string type entry may have
a 
.Cf DW_AT_byte_size 
attribute, whose constant value is the length in bytes of
the string.
.H 2 "Set Entries"
.I
Pascal provides the concept of a ``set,'' which represents a group of
values of ordinal type.
.P
.R
.IX Pascal
.IX set types
A set is represented by a debugging information entry
with the tag
.Cf DW_TAG_set_type .
If a name has been given to the set type,
then the set type entry has a 
.Cf DW_AT_name 
attribute
whose value is a null-terminated string containing the set type name
as it appears in the source program.
.P
The set type entry has a 
.Cf DW_AT_type
attribute to denote the type 
of an element of the set.  
.P
If the amount of storage allocated to hold each element of an object of
the given set type is different from the amount of storage that is normally
allocated to hold an individual object of the indicated element type, then
the set type entry has a 
.Cf DW_AT_byte_size 
attribute, whose constant value
represents the size in bytes of an instance of the set type.
.H 2 "Subrange Type Entries"
.I
Several languages support the concept of a ``subrange'' type object.
These objects can represent a subset of the values that an
object of the basis type for the subrange can represent.
Subrange type entries may also be used to represent the bounds
of array dimensions.
.R
.P
.IX subranges
A subrange type is represented by a debugging information entry
with the tag
.Cf DW_TAG_subrange_type .
If a name has been given to the subrange type,
then the subrange type entry has a 
.Cf DW_AT_name 
attribute
whose value is a null-terminated string containing the subrange type name
as it appears in the source program.
.P
The subrange entry may have a 
.Cf DW_AT_type
attribute to describe
the type of object of whose values this subrange is a subset.
.P
If the amount of storage allocated to hold each element of an object of
the given subrange type is different from the amount of storage that is normally
allocated to hold an individual object of the indicated element type, then
the subrange type entry has a
.Cf DW_AT_byte_size 
attribute, whose constant value
represents the size in bytes of each element of the subrange type.
.P
The subrange entry may have the attributes
.Cf DW_AT_lower_bound
and
.Cf DW_AT_upper_bound
to describe, respectively, the lower and upper bound values
of the subrange.
The 
.Cf DW_AT_upper_bound
attribute may be replaced by a
.Cf DW_AT_count
attribute, whose value describes the number of elements in
the subrange rather than the value of the last element.
If a bound or count value is described by a constant
not represented in the program's address space and can
be represented by one of the constant attribute forms, then the value
of the lower or upper bound or count attribute may be one of the constant
types.  Otherwise, the value of the lower or upper bound or count
attribute is a reference to a debugging information entry describing
an object containing the bound value or itself describing a constant
value. 
.P
If either the lower or upper bound or count values are missing, the
bound value is assumed to be a language-dependent default
constant.
.P
.I
.IX C %c
.IX C++ %caa
.IX Fortran
The default lower bound value for C or C++ is 0.  For Fortran,
it is 1.  No other default values are currently defined by DWARF.
.R
.P
If the subrange entry has no type attribute describing the basis
type, the basis type is assumed to be the same as the object
described by the lower bound attribute (if it references an object).
If there is no lower bound attribute, or it does not reference
an object, the basis type is the type of the upper bound or count
attribute
(if it references an object).  If there is no upper bound or count attribute
or it does not reference an object, the type is assumed to be
the same type, in the source language
of the compilation unit containing the subrange entry,
as a signed integer with the same size
as an address on the target machine.
.H 2 "Pointer to Member Type Entries"
.I
In C++, a pointer to a data or function member of a class or
structure is a unique type.  
.P
.R
.IX C++ %caa
.IX members, pointers to
.IX pointers to members
A debugging information entry
representing the type of an object that is a pointer to a structure
or class member has the tag
.Cf DW_TAG_ptr_to_member_type .
.P
If the pointer to member type has a name, the pointer to member entry
has a 
.Cf DW_AT_name 
attribute, whose value is a null-terminated string
containing the type name as it appears in the source program.
.P
The pointer to member entry has a
.Cf DW_AT_type
attribute to describe
the type of the class or structure member to which objects
of this type may point.
.P
The pointer to member entry also has a
.Cf DW_AT_containing_type
attribute, whose value is a reference to a debugging information
entry for the class or structure to whose members objects of
this type may point.
.P
Finally, the pointer to member entry has a
.Cf DW_AT_use_location
attribute whose value is a location description that computes
the address of the member of the class or structure to which the
pointer to member type entry can point.
.P
.I
The method used to find the address of a given member
of a class or structure is common to any instance of that
class or structure and to any instance of the pointer or
member type.  The method is thus associated
with the type entry, rather than with each instance of the type.
.P
The 
.Cf DW_AT_use_location
expression, however, cannot be used on its own, but must
be used in conjunction with the location expressions for
a particular object of the given pointer to member type
and for a particular structure or class instance.  The 
.Cf DW_AT_use_location
attribute expects two values to be pushed onto the location expression
stack before the
.Cf DW_AT_use_location
expression is evaluated.  The first value pushed should be
the value of the pointer to member object itself.
The second value pushed should be the base address of the entire
structure or union instance containing the member whose
address is being calculated.
.P
So, for an expression like
.DS
	\f(CWobject.*mbr_ptr\fP
.DE
where \f(CWmbr_ptr\fP has some pointer to member type,
a debugger should:
.AL
.LI
Push the value of 
.Cf mbr_ptr
onto the location expression stack.
.LI
Push the base address of
.Cf object 
onto the location expression stack.
.LI
Evaluate the 
.Cf DW_AT_use_location
expression for the type of 
.Cf mbr_ptr .
.LE
.R
.H 2 "File Type Entries"
.I
Some languages, such as Pascal, provide a first class data type
to represent files.
.R
.P
.IX Pascal
.IX file types
A file type is represented by a debugging information entry
with the tag
.Cf DW_TAG_file_type.
If the file type has a name, the file type entry
has a 
.Cf DW_AT_name 
attribute, whose value is a null-terminated string
containing the type name as it appears in the source program.
.P
The file type entry has a 
.Cf DW_AT_type
attribute describing the type
of the objects contained in the file.
.P
The file type entry also has a 
.Cf DW_AT_byte_size 
attribute, whose value
is a constant representing the size in bytes of an instance
of this file type. 
.OP
.H 1 "OTHER DEBUGGING INFORMATION"
This section describes debugging information that
is not represented in the form of debugging information
entries and is not contained within the 
.Cf .debug_info
section.
.H 2 "Accelerated Access"
.I
.IX accelerated access
A debugger frequently needs to find the debugging information for
a program object defined outside of the compilation unit
where the debugged program is currently stopped.  Sometimes
it will know only the name of the object; sometimes only the address.
To find the debugging information
associated with a global object by name, using the DWARF debugging information
entries alone, a debugger would need
to run through all entries at the highest scope within each
compilation unit.  For lookup by address, for a subroutine,
a debugger can use the low and high pc attributes
of the compilation unit entries to quickly narrow down the search,
but these attributes only cover 
the range of addresses for the text associated with a compilation
unit entry.  To find the debugging information associated with a
data object, an exhaustive search would be needed.
Furthermore, any search through debugging information entries for
different compilation units within a large program
would potentially require the access of many memory pages,
probably hurting debugger performance.
.R
.P
To make lookups of program objects by name or by address faster,
a producer of DWARF information may provide two different types
of tables containing information about the debugging information
entries owned by a particular compilation unit entry in a more condensed
format.
.H 3 "Lookup by Name"
.IX lookup, by name
For lookup by name, a table is maintained in a separate
object file section called
.Cf .debug_pubnames .
.IX \f(CW.debug_pubnames\fP %debugap
The table consists of sets of variable length entries, each
set describing the names of global objects whose definitions
or declarations are represented by debugging information entries
owned by a single compilation unit.  Each set begins
with a header containing four values: the total length of the entries
for that set, not including the length field itself, a version number,
the offset from the beginning of the
.Cf .debug_info
.IX \f(CW.debug_info\fP %debugai
section of the compilation unit entry referenced by the set and
the size in bytes of the contents of the
.Cf .debug_info
section generated to represent that compilation unit. This
header is followed by a variable number of offset/name pairs.
Each pair consists of the offset from the beginning of the compilation
unit entry corresponding to the current set to the 
debugging information entry for
the given object, followed by a null-terminated character
string representing the name of the object as given by
the
.Cf DW_AT_name
attribute of the referenced debugging entry.
Each set of names is terminated by zero.
.P
.IX C++ %caa
.IX members, static data
In the case of the name of a static data member or function member
of a C++ structure, class or union, the name presented
in the 
.Cf .debug_pubnames
section is not the simple name given by the
.Cf DW_AT_name 
attribute of the referenced debugging entry, but rather
the fully class qualified name of the data or function member.
.IX identifiers, names
.H 3 "Lookup by Address"
.IX lookup, by address
For lookup by address, a table is maintained in a separate
object file section called
.Cf .debug_aranges .
.IX \f(CW.debug_aranges\fP %debugaar
The table consists of sets of variable length entries, each
set describing the portion of the program's address space that
is covered by a single compilation unit.  Each set begins
with a header containing five values: 
.AL
.LI
The total length of the entries
for that set, not including the length field itself.
.LI
A version number.
.LI
The offset from the beginning of the
.Cf .debug_info
.IX \f(CW.debug_info\fP %debugai
section of the compilation unit entry referenced by the set. 
.LI
The size in bytes of an address on the target architecture.  For
segmented addressing, this is the size of the offset portion of the
.IX addresses, offset portion
.IX addresses, size of
address.
.LI
.IX address space, segmented
.IX segmented address space
The size in bytes of a segment descriptor on the target architecture.
If the target system uses a flat address space, this value is 0.
.LE
.P
This
header is followed by a variable number of address
range descriptors.  Each descriptor is a pair consisting of 
the beginning address
of a range of text or data covered by some entry owned
by the corresponding compilation unit entry, followed by the length
of that range.  A particular set is terminated by an entry consisting
of two zeroes.  By scanning the table, a debugger can quickly
decide which compilation unit to look in to find the debugging information
for an object that has a given address.
.H 2 "Line Number Information"
.I
.IX line number information
A source-level debugger will need to know how to associate statements in
the source files with the corresponding machine instruction addresses in
the executable object or the shared objects used by that executable
object.  Such an association would make it possible for the debugger user
to specify machine instruction addresses in terms of source statements.
This would be done by specifying the line number and the source file
containing the statement.  The debugger can also use this information to
display locations in terms of the source files and to single step from
statement to statement.
.R
.P
As mentioned in section 3.1, above, 
the line number information generated for a compilation unit
is represented in the \f(CW.debug_line\fP section of an object file and is
referenced by a corresponding compilation unit debugging information entry
in the \f(CW.debug_info\fP section.
.IX \f(CW.debug_info\fP %debugai
.IX \f(CW.debug_line\fP %debugali
.I
.P
If space were not a consideration, the information
provided in the 
.Cf .debug_line
section could be represented as a large matrix,
with one row for each instruction in the emitted
object code.  The matrix would have columns for:
.DL
.LI
the source file name
.LI
the source line number
.LI
the source column number
.LI
whether this instruction is the beginning of a source statement
.LI
whether this instruction is the beginning of a basic block.
.LE
.P
Such a matrix, however, would be impractically large.  We shrink it with
two techniques.  First, we delete from the matrix each row whose file,
line and source column information is identical with that of its predecessors.
Second, we design a byte-coded language for a state machine and store a stream
of bytes in the object file instead of the matrix.  This language can be
much more compact than the matrix.  When a consumer of the statement
information executes, it must ``run'' the state machine to generate
the matrix for each compilation unit it is interested in.  The concept
of an encoded matrix also leaves room for expansion.  In the future,
columns can be added to the matrix to encode other things that are
related to individual instruction addresses.
.R
.H 3 "Definitions"
.IX line number information, definitions
The following terms are used in the description of the line number information
format:
.VL 20
.LI "state machine"
The hypothetical machine used by a consumer of the line number information 
to expand the byte-coded instruction stream into a 
matrix of line number information.
.LI "statement program"
A series of byte-coded line number information instructions representing one
compilation unit.
.LI "basic block"
A sequence of instructions that is entered only at the first instruction
and exited only at the last instruction.  We define a procedure invocation
to be an exit from a basic block.
.LI "sequence"
A series of contiguous target machine instructions.  One compilation
unit may emit multiple sequences (that is, not all instructions within
a compilation unit are assumed to be contiguous).
.LI "sbyte"
Small signed integer.
.LI "ubyte"
Small unsigned integer.
.LI "uhalf"
Medium unsigned integer.
.LI "sword"
Large signed integer.
.LI "uword"
Large unsigned integer.
.LI "LEB128"
.IX LEB128
Variable length signed and unsigned data.  See section 7.6.
.LE
.H 3 "State Machine Registers"
.IX line number information, state machine registers
The statement information state machine has the following registers:
.VL 20
.LI "\f(CWaddress\fP"
The program-counter value corresponding to a machine instruction generated
by the compiler.
.LI "\f(CWfile\fP"
An unsigned integer indicating the identity of the source file corresponding
to a machine instruction.
.IX source, files
.LI "\f(CWline\fP"
.IX source, lines
An unsigned integer indicating a source line number.  Lines are numbered
beginning at 1.  The compiler may emit the value 0 in cases where an
instruction cannot be attributed to any source line.
.LI "\f(CWcolumn\fP"
.IX source, columns
An unsigned integer indicating a column number within a source line.
Columns are numbered beginning at 1.  The value 0 is reserved to indicate
that a statement begins at the ``left edge'' of the line.
.LI "\f(CWis_stmt\fP"
A boolean indicating that the current instruction is the beginning of a
statement.
.LI "\f(CWbasic_block\fP"
A boolean indicating that the current instruction is the beginning of
a basic block.
.LI "\f(CWend_sequence\fP"
A boolean indicating that the current address is that of the first
byte after the end of a sequence of target machine instructions.
.LE
.P
At the beginning of each sequence within a statement program, the
state of the registers is:
.DS
.TS
;
lf(CW) l.
address	0
file	1
line	1
column	0
is_stmt	determined by \f(CWdefault_is_stmt\fP in the statement program prologue
basic_block	``false''
end_sequence	``false''
.TE
.DE
.H 3 "Statement Program Instructions"
The state machine instructions in a statement program belong to one
of three categories:
.VL 20
.LI "special opcodes"
.IX line number information, special opcodes
These have a ubyte opcode field and no arguments. 
Most of the instructions in a statement program are special opcodes.
.LI "standard opcodes"
.IX line number information, standard opcodes
These have a ubyte opcode field which may be followed by zero or more
LEB128 arguments (except for 
.Cf DW_LNS_fixed_advance_pc ,
see below).
The opcode implies the number of arguments and their 
meanings, but the statement program prologue also specifies the number 
of arguments for each standard opcode.
.LI "extended opcodes"
.IX line number information, extended opcodes
These have a multiple byte format.  The first byte is zero;
the next bytes are an unsigned LEB128 integer giving the number of bytes
in the instruction itself (does not include the first zero byte or the size).
The remaining bytes are the instruction itself.
.LE
.H 3 "The Statement Program Prologue"
.IX line number information, prologue
The optimal encoding of line number information depends to a certain
degree upon the architecture of the target machine.  The statement program
prologue provides information used by consumers in decoding the statement
program instructions for a particular compilation unit and also provides
information used throughout the rest of the statement program.  The statement
program for each compilation unit begins with a prologue containing the
following fields in order:
.AL
.LI
.Cf total_length 
(uword)
.br
The size in bytes of the statement information for this compilation unit
(not including the 
.Cf total_length
field itself).
.LI
.Cf version
(uhalf)
.br
Version identifier for the statement information format.
.LI
.Cf prologue_length 
(uword)
.br
The number of bytes following the 
.Cf prologue_length
field to the beginning of the first byte of the statement program itself.
.LI
.Cf minimum_instruction_length
(ubyte)
.br
The size in bytes of the smallest target machine instruction.  Statement
program opcodes that alter the 
.Cf address 
register first multiply their operands by this value.
.LI
.Cf default_is_stmt
(ubyte)
.br
The initial value of the 
.Cf is_stmt
register.  
.P
.I
A simple code generator
that emits machine instructions in the order implied by the source program
would set this to ``true,'' and every entry in the matrix would represent
a statement boundary.  A pipeline scheduling code generator would set
this to ``false'' and emit a specific statement program opcode for each
instruction that represented a statement boundary.
.R
.LI
.Cf line_base 
(sbyte)
.br
This parameter affects the meaning of the special opcodes.  See below.
.LI
.Cf line_range 
(ubyte)
.br
This parameter affects the meaning of the special opcodes.  See below.
.LI
.Cf opcode_base
(ubyte)
.br
The number assigned to the first special opcode.
.LI
.Cf standard_opcode_lengths
(array of ubyte)
.br
This array specifies the number of LEB128 operands for each of
the standard opcodes.  The first element of the array corresponds
to the opcode whose value is 1, and the last element corresponds
to the opcode whose value is 
.Cf "opcode_base - 1" . 
By increasing
.Cf opcode_base , 
and adding elements to this array, new standard opcodes
can be added, while allowing consumers who do not know about these
new opcodes to be able to skip them.
.LI
.Cf include_directories
(sequence of path names)
.br
The sequence contains an entry for each path that was searched
for included source files in this compilation.  (The paths include
those directories specified explicitly by the user for the compiler
to search and those the compiler searches without explicit direction).
Each path entry is either a full
path name or is relative to the current directory of the compilation.
The current directory of the compilation is understood to be the first entry
and is not explicitly represented.  
Each entry is a null-terminated
string containing a full path name.  The last entry is followed by
a single null byte.
.LI
.Cf file_names
(sequence of file entries)
.br
.IX source, files
The sequence contains an entry for each source file that contributed
to the statement information for this compilation unit or is
used in other contexts, such as in a declaration coordinate
or a macro file inclusion.  Each entry
has a null-terminated string containing the file name, 
an unsigned LEB128 number representing the directory index of the
directory in which the file was found, 
an unsigned LEB128 number representing the time of last modification for
the file and an unsigned LEB128 number representing the length in
bytes of the file.  A compiler may choose to emit LEB128(0) for the
time and length fields to indicate that this information is not
available.  The last entry is followed by a single null byte.
.P
The directory index represents an entry in the
.Cf include_directories
section.  The index is LEB128(0) if the file was found in
the current directory of the compilation, LEB128(1) if it was
found in the first directory in the
.Cf include_directories
section, and so on.  The directory index is ignored for file names
that represent full path names.
.P
The statement program assigns numbers to each of the file entries
in order, beginning with 1, and uses those numbers instead of file
names in the 
.Cf file 
register.
.P
A compiler may generate a single null byte for the file names field
and define file names using the extended opcode
.Cf DEFINE_FILE .
.LE
.H 3 "The Statement Program"
As stated before, the goal of a statement program is to build a 
matrix representing
one compilation unit, which may have produced multiple sequences of
target-machine instructions. Within a sequence, addresses may only increase.
(Line numbers may decrease in cases of pipeline scheduling.)
.H 4 "Special Opcodes"
.IX line number information, special opcodes
Each 1-byte special opcode has the following effect on the state machine:
.AL
.LI
Add a signed integer to the 
.Cf line 
register.
.LI
Multiply an unsigned integer by the 
.Cf minimum_instruction_length
field of the statement program prologue and 
add the result to the 
.Cf address 
register.
.LI
Append a row to the matrix using the current values of the state machine
registers.
.LI
Set the 
.Cf basic_block 
register to ``false.''
.LE
.P
All of the special opcodes do those same four things; 
they differ from one another
only in what values they add to the 
.Cf line 
and 
.Cf address 
registers.
.P
.I
Instead of assigning a fixed meaning to each special opcode, the statement
program uses several
parameters in the prologue to configure the instruction set. There are two
reasons for this.
First, although the opcode space available for special opcodes now
ranges from 10 through 255, the lower bound may increase if one adds new
standard opcodes. Thus, the 
.Cf opcode_base
field of the statement program
prologue gives the value of the first special opcode.
Second, the best choice of special-opcode meanings depends on the target
architecture.  For example, for a RISC machine where the compiler-generated code
interleaves instructions from different lines to schedule the pipeline,
it is important to be able to add a negative value to the 
.Cf line 
register
to express the fact that a later instruction may have been emitted for an
earlier source line.  For a machine where pipeline scheduling never occurs,
it is advantageous to trade away the ability to decrease the 
.Cf line 
register
(a standard opcode provides an alternate way to decrease the line number) in
return for the ability to add larger positive values to the 
.Cf address
register.  To permit this variety of strategies, the statement program prologue 
defines a 
.Cf line_base
field that specifies the minimum value which a special opcode can add
to the 
.Cf line
register and a 
.Cf line_range
field that defines the range of
values it can add to the 
.Cf line 
register.
.R
.P
A special opcode value is chosen based on the amount that needs to
be added to the 
.Cf line
and 
.Cf address
registers.  The maximum line increment
for a special opcode is the value of the 
.Cf line_base
field in the
prologue, plus the value of the 
.Cf line_range 
field, minus 1 
(\f(CWline base + line range - 1\fP).  If the desired line increment
is greater than the maximum line increment, a standard opcode
must be used instead of a special opcode.
The ``address advance'' is calculated by dividing the desired address
increment by the 
.Cf minimum_instruction_length
field from the
prologue.  The special opcode is then calculated using the following
formula:
.br
        \f(CWopcode = (desired line increment - line_base) +
.br
                (line_range * address advance) + opcode_base\fP
.br
If the resulting opcode is greater than 255, a standard opcode
must be used instead.
.P
To decode a special opcode, subtract the 
.Cf opcode_base
from
the opcode itself.  The amount to increment the 
.Cf address 
register is
the adjusted opcode divided by the 
.Cf line_range .
The amount to
increment the 
.Cf line 
register is the 
.Cf line_base
plus the result
of the adjusted opcode modulo the 
.Cf line_range .
That is,
.br
	\f(CWline increment = line_base + (adjusted opcode % line_range)\fP
.br
.P
.I
As an example, suppose that the 
.Cf opcode_base 
is 16, 
.Cf line_base
is -1 and 
.Cf line_range
is 4.
This means that we can use a special opcode whenever two successive
rows in the matrix have source line numbers differing by any value within
the range [-1, 2] (and, because of the limited number of opcodes available,
when the difference between addresses is within the range [0, 59]).
.P
The opcode mapping would be:
.R
.DS
.TS
box center;
l l l
nf(CW) nf(CW) nf(CW)
.
Opcode	Line advance	Address advance
_
16	-1	0
17	0	0
18	1	0
19	2	0
20	-1	1
21	0	1
22	1	1
23	2	1
...	...	...
253	0	59
254	1	59
255	2	59
.TE
.DE
.P
There is no requirement that the expression \f(CW255 - line_base + 1\fP be an
integral multiple of 
.Cf line_range .
.H 4 "Standard Opcodes"
.IX line number information, standard opcodes
There are currently 9 standard ubyte opcodes.  In the future 
additional ubyte opcodes may be defined by setting the 
.Cf opcode_base
field in the statement program
prologue to a value greater than 10.
.AL
.LI
.Cf DW_LNS_copy 
.br
Takes no arguments.  Append a row to the matrix using the current values of
the state-machine registers.  Then set the 
.Cf basic_block
register to ``false.''
.LI
.Cf DW_LNS_advance_pc
.br
Takes a single unsigned LEB128 operand,
multiplies it by the
.Cf minimum_instruction_length
field of the prologue, and adds the result to the
.Cf address
register of the state machine.
.LI
.Cf DW_LNS_advance_line
.br
Takes a single signed LEB128 operand and adds
that value to the 
.Cf line
register of the state machine.
.LI
.Cf DW_LNS_set_file 
.br
Takes a single unsigned LEB128 operand and stores
it in the 
.Cf file
register of the state machine.
.LI
.Cf DW_LNS_set_column
.br
Takes a single unsigned LEB128 operand and stores
it in the 
.Cf column 
register of the state machine.
.LI
.Cf DW_LNS_negate_stmt
.br
Takes no arguments.
Set the 
.Cf is_stmt
register of the state machine to the
logical negation of its current value.
.LI
.Cf DW_LNS_set_basic_block
.br
Takes no arguments.  Set the 
.Cf basic_block 
register of the state machine to ``true.''
.LI
.Cf DW_LNS_const_add_pc
.br
Takes no arguments.
Add to the 
.Cf address 
register of the state machine the
address increment value corresponding to special
opcode 255.
.P
.I
The motivation for 
.Cf DW_LNS_const_add_pc 
is this:  when the statement program needs
to advance the address by a small amount, it can use a single special
opcode, which occupies a single byte.  When it needs to advance the
address by up to twice the range of the last special opcode, it can use
.Cf DW_LNS_const_add_pc 
followed by a special opcode, for a total of two bytes.
Only if it needs to advance the address by more than twice that range
will it need to use both
.Cf DW_LNS_advance_pc
and a special opcode, requiring three or more bytes.
.R
.LI
.Cf DW_LNS_fixed_advance_pc
.br
Takes a single uhalf operand.  Add to the 
.Cf address 
register of the state machine the value of the (unencoded) operand.
This is the only extended opcode that takes an argument that is not
a variable length number.
.P
.I
The motivation for 
.Cf DW_LNS_fixed_advance_pc
is this:  existing assemblers cannot emit 
.Cf DW_LNS_advance_pc
or special opcodes because they cannot encode LEB128 numbers
or judge when the computation of a special opcode overflows and requires
the use of 
.Cf DW_LNS_advance_pc .
Such assemblers, however,  can use
.Cf DW_LNS_fixed_advance_pc
instead, sacrificing compression. 
.R
.LE
.H 4 "Extended Opcodes"
.IX line number information, extended opcodes
There are three extended opcodes currently defined.  The first byte
following the length field of the encoding for each contains a sub-opcode.
.AL
.LI
\f(CWDW_LNE_end_sequence\fP 
.br
Set the 
.Cf end_sequence 
register of the state machine
to ``true'' and append a row to the matrix using the
current values of the state-machine registers.  Then
reset the registers to the initial values specified
above.
.P
Every statement program sequence must end with a
.Cf DW_LNE_end_sequence
instruction which creates a
row whose address is that of the byte after the last target machine instruction
of the sequence.
.LI
\f(CWDW_LNE_set_address\fP
.br
Takes a single relocatable address as an operand.  The size of the
operand is the size appropriate to hold an address on the target machine.
Set the 
.Cf address 
register to the value given by the
relocatable address. 
.P
.I
All of the other statement program opcodes that affect the 
.Cf address 
register add a delta to it. 
This instruction stores a relocatable value into it instead.
.R
.LI
\f(CWDW_LNE_define_file\fP
.br
.IX source, files
Takes 4 arguments.  The first is a null terminated string containing a
source file name.  The second is an
unsigned LEB128 number representing the directory index of the
directory in which the file was found.
The third is an unsigned LEB128 number representing
the time of last modification of the file.  The fourth is an unsigned
LEB128 number representing the length in bytes of the file.
The time and length fields may contain LEB128(0) if the information is
not available.
.P
The directory index represents an entry in the
.Cf include_directories
section of the statement program prologue.  
The index is LEB128(0) if the file was found in
the current directory of the compilation, LEB128(1) if it was
found in the first directory in the
.Cf include_directories
section, and so on.  The directory index is ignored for file names
that represent full path names.
.P
The files are numbered, starting at 1,
in the order in which they appear; the names in the prologue
come before names defined by the
.Cf DW_LNE_define_file
instruction.
These numbers are used in the the 
.Cf file 
register of the state machine.
.LE
.P
.I
Appendix 3 gives some sample statement programs.
.R
.H 2 "Macro Information"
.I
.IX macro information
.IX pre-processor
.IX C %c
.IX C++ %caa
Some languages, such as C and C++, provide a way to replace text
in the source program with macros defined either in the source
file itself, or in another file included by the source file.
Because these macros are not themselves defined in the target
language, it is difficult to represent their definitions
using the standard language constructs of DWARF.  The debugging
information therefore reflects the state of the source after
the macro definition has been expanded, rather than as the
programmer wrote it.
The macro information table provides a way of preserving the original
source in the debugging information.
.R
.P
As described in section 3.1, the macro information for a given
compilation unit is represented in the
.Cf .debug_macinfo
.IX \f(CW.debug_macinfo\fP %debugam
section of an object file.  The macro information for each compilation
unit is represented as a series of ``macinfo'' entries.  Each
macinfo entry consists of a ``type code'' and up to two additional
operands.  The series of entries for a given compilation unit
ends with an entry containing a type code of 0.
.H 3 "Macinfo Types"
The valid macinfo types are as follows:
.VL 30
.LI \f(CWDW_MACINFO_define\fP
A macro definition.
.LI \f(CWDW_MACINFO_undef\fP
A macro un-definition.
.LI \f(CWDW_MACINFO_start_file\fP
The start of a new source file inclusion.
.LI \f(CWDW_MACINFO_end_file\fP
The end of the current source file inclusion.
.LI \f(CWDW_MACINFO_vendor_ext\fP
Vendor specific macro information directives that do not fit
into one of the standard categories.
.LE
.H 4 "Define and Undefine Entries"
.IX macro information, define and undefine entries
All 
.Cf DW_MACINFO_define
and 
.Cf DW_MACINFO_undef
entries have two operands.
The first operand encodes the line number of the source line
.IX source, lines
on which the relevant defining or undefining
pre-processor directives appeared.
.P 
The second operand consists of a null-terminated character string.
In the case of a 
.Cf DW_MACINFO_undef 
entry, the value of this
string will be simply the name of the pre-processor
symbol which was undefined at the indicated source line.
.P
In the case of a 
.Cf DW_MACINFO_define
entry, the value of this
string will be the name of the pre-processor symbol
that was defined at the indicated source line,
followed immediately by the macro formal parameter
list including the surrounding parentheses (in the
case of a function-like macro) followed by the
definition string for the macro.  If there is no
formal parameter list, then the name of the defined
macro is followed directly by its definition string.
.P
In the case of a function-like macro definition, no
whitespace characters should appear between the
name of the defined macro and the following left
parenthesis.  Also, no whitespace characters should
appear between successive formal parameters in the
formal parameter list.  (Successive formal parameters
should, however, be separated by commas.)  Also, exactly
one space character
should separate the right parenthesis which terminates
the formal parameter list and the following definition
string.
.P
In the case of a ``normal'' (i.e. non-function-like)
macro definition, exactly one space character
should separate the name of the defined macro from the following definition
text.
.H 4 "Start File Entries"
.IX macro information, start file entries
Each 
.Cf DW_MACINFO_start_file
entry also has two operands.  The first operand
encodes the line number of the
source line on which the inclusion pre-processor
directive occurred.
.P
.IX source, files
The second operand encodes a
source file name index.  This index corresponds to a file
number in the statement information table for the relevant
compilation unit.  This index
indicates (indirectly) the name of the file
which is being included by the inclusion directive on
the indicated source line.
.H 4 "End File Entries"
.IX macro information, end file entries
A 
.Cf DW_MACINFO_end_file 
entry has no operands.  The presence of the entry marks the end of
the current source file inclusion.
.H 4 "Vendor Extension Entries"
.IX macro information, vendor extensions
.IX vendor extensions
A
.Cf DW_MACINFO_vendor_ext
entry has two operands.
The first is a constant.  The second is a null-terminated
character string.
The meaning and/or significance of these operands is
intentionally left undefined by this specification.
.P
A consumer must be able to totally ignore all
.Cf DW_MACINFO_vendor_ext
entries that it does not understand.
.H 3 "Base Source Entries"
.IX macro information, base source entries
In addition to producing a matched pair of
.Cf DW_MACINFO_start_file 
and 
.Cf DW_MACINFO_end_file
entries for 
each inclusion directive actually processed during
compilation, a producer should generate such a matched
pair also for the ``base'' source file submitted to the
compiler for compilation.  If the base source file
.IX source, files
for a compilation is submitted to the compiler via
some means other than via a named disk file (e.g. via
the standard input \fIstream\fP on a UNIX system) then the
compiler should still produce this matched pair of
.Cf DW_MACINFO_start_file
and 
.Cf DW_MACINFO_end_file 
entries for
the base source file, however, the file name indicated
(indirectly) by the 
.Cf DW_MACINFO_start_file
entry of the
pair should reference a statement information file name entry consisting
of a null string.
.H 3 "Macinfo Entries for Command Line Options"
.IX macro information, command line options
In addition to producing
.Cf DW_MACINFO_define 
and
.Cf DW_MACINFO_undef
entries for each of the define and
undefine directives processed during compilation, the
DWARF producer should generate a 
.Cf DW_MACINFO_define 
or
.Cf DW_MACINFO_undef
entry for each pre-processor symbol
which is defined or undefined by some
means other than via a define or undefine directive
within the compiled source text.  In particular,
pre-processor symbol definitions and un-definitions
which occur as a result of command line options
(when invoking the compiler) should be represented by
their own 
.Cf DW_MACINFO_define
and 
.Cf DW_MACINFO_undef
entries.
.P
All such 
.Cf DW_MACINFO_define
and 
.Cf DW_MACINFO_undef
entries representing compilation options should appear 
before the first 
.Cf DW_MACINFO_start_file
entry for that compilation unit and should encode the value
0 in their line number operands.
.H 3 " General Rules and Restrictions"
.IX line number information, general rules
All macinfo entries within a 
.Cf .debug_macinfo 
section for a given compilation unit should appear in the same order
in which the directives were processed by the compiler.
.P
All macinfo entries representing command line options
should appear in the same order as the relevant command
line options were given to the compiler.  In the case
where the compiler itself implicitly supplies one or
more macro definitions or un-definitions in addition
to those which may be specified on the command line,
macinfo entries should also be produced for these
implicit definitions and un-definitions, and
these entries should also appear in the proper order
relative to each other and to any definitions or
undefinitions given explicitly by the user on the
command line.
.H 2 "Call Frame Information"
.IX call frame information
.IX activations
.I
Debuggers often need to be able to view and modify the state of any
subroutine activation that is on the call stack.  An activation
consists of:
.BL
.LI
A code location that is within the subroutine.  This location is
either the place where the program stopped when the debugger got
control (e.g. a breakpoint), or is a place where a subroutine
made a call or was interrupted by an asynchronous event (e.g. a
signal).
.LI
An area of memory that is allocated on a stack called a ``call
frame.''  The call frame is identified by an address on the
stack.  We refer to this address as the Canonical Frame Address or CFA.
.LI
A set of registers that are in use by the subroutine at the code
location.
.LE
.P
Typically, a set of registers are designated to be preserved across a
call.  If a callee wishes to use such a register, it saves the value
that the register had at entry time in its call frame and restores it
on exit.  The code that allocates space on the call frame stack and
performs the save operation is called the subroutine's prologue, and the
code that performs the restore operation and deallocates the frame is
called its epilogue.  Typically, the prologue code is physically at the
beginning of a subroutine and the epilogue code is at the end.
.P
To be able to view or modify an activation that is not on the top of
the call frame stack, the debugger must ``virtually unwind'' the stack of
activations until it finds the activation of interest. 
A debugger unwinds a
stack in steps.  Starting with the current activation it restores any
registers that were preserved by the current activation and computes the
predecessor's CFA and code location.  This has the logical effect of
returning from the current subroutine to its predecessor.  We say that
the debugger virtually unwinds the stack because it preserves enough
information to be able to ``rewind'' the stack back to the state it was
in before it attempted to unwind it.
.P
The unwinding operation needs to know where registers are saved and how
to compute the predecessor's CFA and code location.  When considering
an architecture-independent way of encoding this information one has to
consider a number of special things.
.BL
.LI
Prologue and epilogue code is not always in distinct blocks at the
beginning and end of a subroutine.  It is common to duplicate the
epilogue code at the site of each return from the code.  Sometimes
a compiler breaks up the register save/unsave operations and moves
them into the body of the subroutine to just where they are needed.
.LI
Compilers use different ways to manage the call frame.  Sometimes
they use a frame pointer register, sometimes not.
.LI
The algorithm to compute the CFA changes as you progress through
the prologue and epilogue code.  (By definition, the CFA value
does not change.)
.LI
Some subroutines have no call frame.
.LI
Sometimes a register is saved in another register that by
convention does not need to be saved.
.LI
Some architectures have special instructions that
perform some or all of the register management in one instruction,
leaving special information on the stack that indicates how
registers are saved.
.LI
Some architectures treat return address values
specially.  For example, in one architecture, 
the call instruction guarantees that the low order two
bits will be zero and the return instruction ignores those bits.
This leaves two bits of storage that are available to other uses
that must be treated specially.
.LE
.R
.H 3 "Structure of Call Frame Information"
.IX call frame information, structure
DWARF supports virtual unwinding by defining an architecture independent
basis for recording how procedures save and restore registers throughout
their lifetimes.  This basis must be augmented on some machines with
specific information that is defined by either an architecture specific
ABI authoring committee, a hardware vendor, or a compiler producer.
.IX ABI
.IX vendor extensions
The body defining a specific augmentation is referred to
below as the ``augmenter.''
.P
Abstractly, this mechanism describes a very large table that has the
following structure:
.TS
center;
l l l l l l
l s s s s s.
LOC  CFA   R0   R1   ...  RN
L0
L1
\...
LN
.TE
.P
The first column indicates an address for every location that contains
code in a program.  (In shared objects, this is an object-relative
offset.)  The remaining columns contain virtual unwinding rules that are
associated with the indicated location.  The first column of the rules
defines the CFA rule which is a register and a signed offset that are
added together to compute the CFA value.
.P
The remaining columns are labeled by register number.  This includes
some registers that have special designation on some architectures such
as the PC and the stack pointer register.  (The actual mapping of
registers for a particular architecture is performed by the augmenter.)
The register columns contain rules that describe
whether a given register has been saved and the rule to find 
the value for the register in the previous frame.
.P
The register rules are:
.IX call frame information, register rules
.VL 20
.LI "undefined"
A register that has this rule has no value in the
previous frame.  (By convention, it is not preserved by a callee.)
.LI "same value"
This register has not been modified from the
previous frame.  (By convention, it is preserved by the callee,
but the callee has not modified it.)
.LI "offset(N)"
The previous value of this register is saved at the address CFA+N where
CFA is the current CFA value and N is a signed offset.
.LI "register(R)"
The previous value of this register is stored in
another register numbered R.
.LI "architectural"
The rule is defined externally to this specification by the augmenter.
.LE
.P
.I
This table would be extremely large if actually constructed as
described.  Most of the entries at any point in the table are identical
to the ones above them.  The whole table can be represented quite
compactly by recording just the differences starting at the beginning
address of each subroutine in the program.
.R
.P
The virtual unwind information is encoded in a self-contained section
called 
.Cf .debug_frame . 
.IX \f(CW.debug_frame\fP %debugaf
Entries in a 
.Cf .debug_frame
section are aligned on
.IX call frame information, Common Information Entry
an addressing unit boundary and come in two forms: A Common Information
Entry (CIE) and a Frame Description Entry (FDE).
Sizes of data objects used in the encoding of the 
.Cf .debug_frame
section are described in terms of the same data definitions
used for the line number information (see section 6.2.1).
.P
A Common Information Entry holds information that is shared among many
Frame Descriptors.  There is at least one CIE in every non-empty
.Cf .debug_frame 
section.  A CIE contains the following fields, in order:
.AL
.LI 
\f(CWlength\fP
.br
A uword constant that gives the number of bytes of the CIE
structure, not including the length field, itself 
(length mod <addressing unit size> == 0).
.LI 
\f(CWCIE_id\fP
.br
A uword constant that is used to distinguish CIEs
from FDEs.
.LI
\f(CWversion\fP
.br
A ubyte version number.  This number is specific to the call frame
information and is independent of the DWARF version number.
.LI 
\f(CWaugmentation\fP
.br
A null terminated string that identifies the
augmentation to this CIE or to the FDEs that use
it.  If a reader encounters an augmentation string that is
unexpected, then only the following fields can be read:
CIE: 
.Cf length , 
.Cf CIE_id , 
.Cf version , 
.Cf augmentation ;
FDE:
.Cf length , 
.Cf CIE_pointer , 
.Cf initial_location , 
.Cf address_range .
If there is no augmentation, this value is a zero byte.
.LI 
\f(CWcode_alignment_factor\fP
.br
An unsigned LEB128 constant that is factored out
of all advance location instructions (see below).
.LI 
\f(CWdata_alignment_factor\fP
.br
A signed LEB128 constant that is factored out
of all offset instructions (see below.)
.LI 
\f(CWreturn_address_register\fP
.br
A ubyte constant that indicates
which column in the rule table represents the return address
of the function.  Note that this column might not correspond
to an actual machine register.
.LI 
\f(CWinitial_instructions\fP
.br
A sequence of rules that are interpreted to
create the initial setting of each column in the table.
.LI 
\f(CWpadding\fP
.br
Enough 
.Cf DW_CFA_nop
instructions to make the size of this entry
match the 
.Cf length
value above.
.LE
.P
An FDE contains the following fields, in order:
.IX call frame information, Frame Description Entry
.AL
.LI 
\f(CWlength\fP
.br
A uword constant that gives the number of bytes of the header
and instruction stream for this function (not including the length
field itself) (length mod <addressing unit size> == 0).
.LI 
\f(CWCIE_pointer\fP
.br
A uword constant offset into the
.Cf .debug_frame 
section that denotes the CIE that is associated with this FDE.
.LI 
\f(CWinitial_location\fP
An addressing-unit sized constant indicating
the address of the first location associated with this table entry.
.LI 
\f(CWaddress_range\fP
.br
An addressing unit sized constant indicating the
number of bytes of program instructions described by this entry.
.LI 
\f(CWinstructions\fP
.br
A sequence of table defining instructions that are
described below.
.LE
.H 3 "Call Frame Instructions"
.IX call frame information, instructions
Each call frame instruction is defined to
take 0 or more operands.  Some of the operands may be
encoded as part of the opcode (see section 7.23).
The instructions are as follows:
.AL
.LI 
.Cf DW_CFA_advance_loc
takes a single argument that represents a constant delta.
The required action is to
create a new table row with a location value that
is computed by taking the current entry's location value and
adding (delta * \f(CWcode_alignment_factor\fP).  All other values in the
new row are initially identical to the current row.
.LI 
.Cf DW_CFA_offset
takes two arguments: 
an unsigned LEB128 constant representing a factored offset
and a register number.  The required action is
to change the rule for the register indicated by the register
number to be an offset(N) rule with a value of
(N = factored offset * \f(CWdata_alignment_factor\fP).
.LI 
.Cf DW_CFA_restore
takes a single argument that represents a register number.
The required action is
to change the rule for the indicated register 
to the rule assigned it by the \f(CWinitial_instructions\fP in the CIE.
.LI 
.Cf DW_CFA_set_loc
takes a single argument that represents an address.
The required action is to create a new table row
using the specified address as the location. 
All other values in the
new row are initially identical to the current row.
The new location value should always be greater than the current
one.
.LI 
.Cf DW_CFA_advance_loc1
takes a single ubyte argument that represents a constant delta.
This instruction is identical to 
.Cf DW_CFA_advance_loc
except for the encoding and size of the delta argument.
.LI 
.Cf DW_CFA_advance_loc2
takes a single uhalf argument that represents a constant delta.
This instruction is identical to 
.Cf DW_CFA_advance_loc
except for the encoding and size of the delta argument.
.LI 
.Cf DW_CFA_advance_loc4
takes a single uword argument that represents a constant delta.
This instruction is identical to 
.Cf DW_CFA_advance_loc
except for the encoding and size of the delta argument.
.LI 
.Cf DW_CFA_offset_extended
takes two unsigned LEB128 arguments representing a register number
and a factored offset.
This instruction is identical to 
.Cf DW_CFA_offset
except for the encoding and size of the register argument.
.LI 
.Cf DW_CFA_restore_extended
takes a single unsigned LEB128 argument that represents a register number.
This instruction is identical to 
.Cf DW_CFA_restore
except for the encoding and size of the register argument.
.LI 
.Cf DW_CFA_undefined
takes a single unsigned LEB128 argument that represents a register number.
The required action is to set the rule for the specified register
to ``undefined.''
.LI 
.Cf DW_CFA_same_value
takes a single unsigned LEB128 argument that represents a register number.
The required action is to set the rule for the specified register
to ``same value.''
.LI 
.Cf DW_CFA_register
takes two unsigned LEB128 arguments representing register numbers.
The required action is to set the rule for the first register
to be the second register.
.LI 
\f(CWDW_CFA_remember_state\fP
.LI 
\f(CWDW_CFA_restore_state\fP
.br
These instructions define a stack of information.  Encountering the 
.Cf DW_CFA_remember_state
instruction means to save the rules for every register
on the current row on the stack.  Encountering the
.Cf DW_CFA_restore_state
instruction means to pop the set of rules
off the stack and place them in the current row.  
.I
(This
operation is useful for compilers that move epilogue
code into the body of a function.)
.R
.LI
.Cf DW_CFA_def_cfa
takes two unsigned LEB128 arguments representing a
register number and an offset.
The required action is to define the current CFA rule
to use the provided register and offset.
.LI 
.Cf DW_CFA_def_cfa_register
takes a single unsigned LEB128 argument representing a register
number.  The required action is to define the current CFA
rule to use the provided register (but to keep the old offset).
.LI 
.Cf DW_CFA_def_cfa_offset
takes a single unsigned LEB128 argument representing an offset.
The required action is to define the current CFA
rule to use the provided offset (but to keep the old register).
.LI 
.Cf DW_CFA_nop
has no arguments and no required actions.  It is used as padding
to make the FDE an appropriate size.
.LE
.H 3 "Call Frame Instruction Usage"
.IX call frame information, usage
.I
To determine the virtual unwind rule set for a given location (L1), one
searches through the FDE headers looking at the 
.Cf initial_location
and
.Cf address_range
values to see if L1 is contained in the FDE.  If so, then:
.AL
.LI
Initialize a register set by reading the 
.Cf initial_instructions
field of the associated CIE.
.LI
Read and process the FDE's instruction sequence until a
.Cf DW_CFA_advance_loc ,
.Cf DW_CFA_set_loc ,
or the end of the instruction stream is
encountered.
.LI
If a 
.Cf DW_CFA_advance_loc
or 
.Cf DW_CFA_set_loc
instruction was encountered, then
compute a new location value (L2).  If L1 >= L2 then process the
instruction and go back to step 2.
.LI
The end of the instruction stream can be thought of as a
.br
\f(CWDW_CFA_set_loc( initial_location + address_range )\fP 
.br
instruction.  
Unless the FDE is ill-formed, L1 should be less than L2 at this point.
.LE
.P
The rules in the register set now apply to location L1.
.P
For an example, see Appendix 5.
.R
.OP
.H 1 "DATA REPRESENTATION"
This section describes the binary representation of the debugging
information entry itself, of the
attribute types and of other fundamental elements described above.
.H 2 "Vendor Extensibility"
.IX vendor extensions
To reserve a portion of the DWARF name space and ranges of
enumeration values for use for vendor specific extensions,
.IX tags
.IX types, base
.IX base types
.IX locations, expressions
.IX calling conventions
.IX call frame information
special labels are reserved for tag names, attribute names,
base type encodings, location operations, language names,
calling conventions and call frame instructions.
The labels denoting the beginning and end of the reserved value
range for vendor specific extensions consist of the appropriate prefix (
.Cf DW_TAG ,
.Cf DW_AT ,
.Cf DW_ATE ,
.Cf DW_OP ,
.Cf DW_LANG ,
.CF DW_CC
or
.Cf DW_CFA
respectively) followed by 
.Cf _lo_user
or 
.Cf _hi_user .
For example, for entry tags, the special labels are
.Cf DW_TAG_lo_user
and
.Cf DW_TAG_hi_user .
Values in the range between \fIprefix\fP\f(CW_lo_user\fP and
\fIprefix\fP\f(CW_hi_user\fP
inclusive, are reserved for vendor specific extensions.
Vendors may use values in this range without
conflicting with current or future system-defined values.
All other values are reserved for use by the system.
.P
Vendor defined tags, attributes, base type encodings, location atoms, 
language names, calling conventions and call frame instructions, 
conventionally use the form
\fIprefix\f(CW_\fIvendor_id\f(CW_\fIname\fR, where \fIvendor_id\fP is some 
identifying character sequence chosen so as to avoid conflicts with other
vendors.
.P
.IX compatibility
To ensure that extensions added by one vendor may be safely ignored
by consumers that do not understand those extensions, 
the following rules should be followed:
.AL
.LI
New attributes should be added in such a way that a debugger may recognize
the format of a new attribute value without knowing the content of that
attribute value.
.LI
The semantics of any new attributes should not alter the semantics of
previously existing attributes.
.LI
The semantics of any new tags
should not conflict with the semantics of previously existing tags.
.LE
.H 2 "Reserved Error Values"
.IX error values
As a convenience for consumers of DWARF information,
the value 0 is reserved in the encodings for attribute names, attribute
forms, base type encodings, location operations, languages,
statement program opcodes, macro information entries and tag names
to represent an error condition or unknown value.  DWARF does
not specify names for these reserved values, since they do not
represent valid encodings for the given type and should not appear
in DWARF debugging information.
.H 2 "Executable Objects and Shared Objects"
The relocated addresses in the debugging information for an executable
object are virtual addresses and the relocated addresses in the
debugging information for a shared object are offsets relative to
the start of the lowest segment used by that shared object.
.P
.I
This requirement makes the debugging information for shared objects
position independent.
Virtual addresses in a shared object may be calculated by adding the
offset to the base address at which the object was attached.
This offset is available in the run-time linker's data structures.
.H 2 "File Constraints"
All debugging information entries in a relocatable object file, 
executable object or shared
object are required to be physically contiguous.
.H 2 "Format of Debugging Information"
.IX Version 2
For each compilation unit compiled with a DWARF Version 2 producer,
.IX compilation units
.IX compilation units, header
a contribution is made to the
.Cf .debug_info
.IX \f(CW.debug_info\fP %debugai
section of the object file.  Each such contribution consists of
a compilation unit header followed by a series of debugging information
entries.  Unlike the information encoding for DWARF Version 1, Version 2
.IX Version 1
debugging information entries do not themselves contain the debugging
information entry tag or the attribute name and form encodings for
each attribute.  Instead, each debugging information entry begins with
a code that represents an entry in a separate abbreviations table.
This code is followed directly by a series of attribute values.
The appropriate entry in the abbreviations table guides the interpretation
of the information contained directly in the 
.Cf .debug_info
section.  Each compilation unit is associated with a particular
abbreviation table, but multiple compilation units may share
the same table.  
.IX abbreviations table
.I
.P
This encoding was based on the observation that typical DWARF producers
produce a very limited number of different types of debugging information
entries.  By extracting the common information from those entries
into a separate table, we are able to compress the generated information.
.R
.H 3 "Compilation Unit Header"
.IX compilation units, header
The header for the series of debugging information entries contributed
by a single compilation unit consists of the following information:
.AL
.LI
A 4-byte unsigned integer representing the length of the
.Cf .debug_info
contribution for that compilation unit, not including the length field itself.
.LI
A 2-byte unsigned integer representing the version of the DWARF information
for that compilation unit.  For DWARF Version 2, the value in this field is 2.
.IX Version 2
.LI
A 4-byte unsigned offset into the 
.Cf .debug_abbrev
.IX \f(CW.debug_abbrev\fP %debugaab
section.  This offset associates the compilation unit with a particular
set of debugging information entry abbreviations.
.LI
.IX segmented address space
.IX address space, segmented
.IX addresses, size of
A 1-byte unsigned integer representing the size in bytes of an address
on the target architecture.  If the system uses segmented addressing,
this value represents the size of the offset portion of an address.
.IX addresses, offset portion
.LE
.P
.I
The compilation unit header does not replace the
.Cf DW_TAG_compile_unit
debugging information entry.  It is additional information that
is represented outside the standard DWARF tag/attributes format.
.R
.H 3 "Debugging Information Entry"
Each debugging information entry begins with an unsigned LEB128
.IX debugging information entries
number containing the abbreviation code for the entry.
This code represents an entry within the abbreviation table associated
with the compilation unit containing this entry.  The abbreviation
.IX abbreviations table
code is followed by a series of attribute values.
.IX attributes, values
.P
On some architectures, there are alignment constraints on section boundaries.
To make it easier to pad debugging information sections to satisfy
such constraints, the abbreviation code 0 is reserved.  Debugging
information entries consisting of only the 0 abbreviation code are considered
null entries.
.IX debugging information entries, null entries
.H 3 "Abbreviation Tables"
.IX abbreviations table
The abbreviation tables for all compilation units are contained in
a separate object file section called
.Cf .debug_abbrev .
.IX \f(CW.debug_abbrev\fP %debugaab
As mentioned before, multiple compilation units may share the same
abbreviation table.  
.P
The abbreviation table for a single compilation
unit consists of a series of abbreviation declarations.
Each declaration specifies the tag and attributes for a particular
.IX tags
.IX attributes
form of debugging information entry.  Each declaration begins with
an unsigned LEB128 number representing the abbreviation code itself.
It is this code that appears at the beginning of a debugging information
entry in the
.Cf .debug_info
section.  As described above, the abbreviation code 0 is reserved for null
debugging information entries.
The abbreviation code is followed by another unsigned LEB128
number that encodes the entry's tag.
.IX tags
.nr aX \n(Fg+1
.nr bX \n(Fg+2
The encodings for the tag names are given in Figures \n(aX
and \n(bX.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Tag name	Value
_
DW_TAG_array_type	0x01
DW_TAG_class_type	0x02
DW_TAG_entry_point	0x03
DW_TAG_enumeration_type	0x04
DW_TAG_formal_parameter	0x05
DW_TAG_imported_declaration	0x08
DW_TAG_label	0x0a
DW_TAG_lexical_block	0x0b          
DW_TAG_member	0x0d
DW_TAG_pointer_type	0x0f
DW_TAG_reference_type	0x10
DW_TAG_compile_unit	0x11
DW_TAG_string_type	0x12
DW_TAG_structure_type	0x13
DW_TAG_subroutine_type	0x15
DW_TAG_typedef	0x16
DW_TAG_union_type	0x17
DW_TAG_unspecified_parameters	0x18
DW_TAG_variant	0x19
DW_TAG_common_block	0x1a
DW_TAG_common_inclusion	0x1b
DW_TAG_inheritance	0x1c
DW_TAG_inlined_subroutine	0x1d
DW_TAG_module	0x1e
DW_TAG_ptr_to_member_type	0x1f
DW_TAG_set_type	0x20
DW_TAG_subrange_type	0x21
DW_TAG_with_stmt	0x22
DW_TAG_access_declaration	0x23
DW_TAG_base_type	0x24
DW_TAG_catch_block	0x25
DW_TAG_const_type	0x26
DW_TAG_constant	0x27
DW_TAG_enumerator	0x28
DW_TAG_file_type	0x29
.TE
.FG "Tag encodings (part 1)"
.DE
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Tag name	Value
_
DW_TAG_friend	0x2a
DW_TAG_namelist	0x2b
DW_TAG_namelist_item	0x2c
DW_TAG_packed_type	0x2d
DW_TAG_subprogram	0x2e
DW_TAG_template_type_param	0x2f
DW_TAG_template_value_param	0x30
DW_TAG_thrown_type	0x31
DW_TAG_try_block	0x32
DW_TAG_variant_part	0x33
DW_TAG_variable	0x34
DW_TAG_volatile_type	0x35
DW_TAG_lo_user	0x4080
DW_TAG_hi_user	0xffff
.TE
.FG "Tag encodings (part 2)"
.DE
.P
Following the tag encoding is a 1-byte value that determines
whether a debugging information entry using this abbreviation
has child entries or not.  If the value is
.Cf DW_CHILDREN_yes ,
the next physically succeeding entry of any debugging information
entry using this abbreviation is the first child of the prior entry.
If the 1-byte value following the abbreviation's tag encoding
is
.Cf DW_CHILDREN_no ,
the next physically succeeding entry of any debugging information entry
using this abbreviation is a sibling of the prior entry.  (Either
the first child or sibling entries may be null entries).
.IX debugging information entries, siblings
.IX debugging information entries, child entries
.IX debugging information entries, null entries
.nr aX \n(Fg+1
The encodings for the child determination byte are given in Figure \n(aX.
(As mentioned in section 2.3, each chain of sibling entries is
terminated by a null entry).
.IX debugging information entries, null entries
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Child determination name	Value
_
DW_CHILDREN_no	0
DW_CHILDREN_yes	1
.TE
.FG "Child determination encodings"
.DE
.P
Finally, the child encoding is followed by a series of attribute specifications.
.IX attributes
Each attribute specification consists of two parts.  The first part
is an unsigned LEB128 number representing the attribute's name.
.IX attributes, names
The second part is an unsigned LEB128 number representing the
attribute's form.  The series of attribute specifications ends
.IX attributes, forms
with an entry containing 0 for the name and 0 for the form.
.P
The attribute form 
.Cf DW_FORM_indirect
is a special case.  For attributes with this form, the attribute value
itself in the
.Cf .debug_info
section begins with an unsigned LEB128 number that represents its form.
This allows producers to choose forms for particular attributes dynamically,
without having to add a new entry to the abbreviation table.
.P
The abbreviations for a given compilation unit end with an entry
consisting of a 0 byte for the abbreviation code.
.I
.P
See Appendix 2 for a depiction of the organization
of the debugging information.
.R
.H 3 "Attribute Encodings"
.nr aX \n(Fg+1
.nr bX \n(Fg+2
The encodings for the attribute names are given in Figures \n(aX
and \n(bX.
.DF
.TS
box center;
l l l
lf(CW) lf(CW) l
. 
Attribute name	Value	Classes
_
DW_AT_sibling	0x01	reference
DW_AT_location	0x02	block, constant
DW_AT_name	0x03	string
DW_AT_ordering	0x09	constant
DW_AT_byte_size	0x0b	constant
DW_AT_bit_offset	0x0c	constant
DW_AT_bit_size	0x0d	constant
DW_AT_stmt_list	0x10	constant
DW_AT_low_pc	0x11	address
DW_AT_high_pc	0x12	address
DW_AT_language	0x13	constant
DW_AT_discr	0x15	reference
DW_AT_discr_value	0x16	constant
DW_AT_visibility	0x17	constant
DW_AT_import	0x18	reference
DW_AT_string_length	0x19	block, constant
DW_AT_common_reference	0x1a	reference
DW_AT_comp_dir	0x1b	string
DW_AT_const_value	0x1c	string, constant, block
DW_AT_containing_type	0x1d	reference
DW_AT_default_value	0x1e	reference
DW_AT_inline	0x20	constant
DW_AT_is_optional	0x21	flag
DW_AT_lower_bound	0x22	constant, reference
DW_AT_producer	0x25	string
DW_AT_prototyped	0x27	flag
DW_AT_return_addr	0x2a	block, constant
DW_AT_start_scope	0x2c	constant
DW_AT_stride_size	0x2e	constant
DW_AT_upper_bound	0x2f	constant, reference
.TE
.FG "Attribute encodings, part 1"
.DE
.DF
.TS
box center;
l l l
lf(CW) lf(CW) l
. 
Attribute name	Value	Classes
_
DW_AT_abstract_origin	0x31	reference
DW_AT_accessibility	0x32  	constant
DW_AT_address_class	0x33	constant
DW_AT_artificial	0x34	flag
DW_AT_base_types	0x35	reference
DW_AT_calling_convention	0x36	constant
DW_AT_count	0x37	constant, reference
DW_AT_data_member_location	0x38	block, reference
DW_AT_decl_column	0x39	constant
DW_AT_decl_file	0x3a	constant
DW_AT_decl_line	0x3b	constant
DW_AT_declaration	0x3c 	flag
DW_AT_discr_list	0x3d 	block
DW_AT_encoding	0x3e	constant
DW_AT_external	0x3f	flag
DW_AT_frame_base	0x40	block, constant
DW_AT_friend	0x41	reference
DW_AT_identifier_case	0x42	constant
DW_AT_macro_info	0x43	constant
DW_AT_namelist_item	0x44	block
DW_AT_priority	0x45 	reference
DW_AT_segment	0x46	block, constant
DW_AT_specification	0x47	reference
DW_AT_static_link	0x48	block, constant
DW_AT_type	0x49	reference
DW_AT_use_location	0x4a	block, constant
DW_AT_variable_parameter	0x4b 	flag
DW_AT_virtuality	0x4c 	constant
DW_AT_vtable_elem_location	0x4d	block, reference
DW_AT_lo_user	0x2000	\(em
DW_AT_hi_user	0x3fff	\(em
.TE
.FG "Attribute encodings, part 2"
.DE
.P
.IX attributes, forms
The attribute form governs how the value of the attribute is encoded.
The possible forms may belong to one of the following
form classes:
.VL 18
.LI address
.IX attributes, addresses
Represented as an object of appropriate size to hold an 
address on the target machine (\f(CWDW_FORM_addr\fP).
This address is relocatable in
a relocatable object file and is relocated in an 
executable file or shared object.
.LI "block"
.IX attributes, blocks
Blocks come in four forms.  The first consists of a 1-byte length
followed by 0 to 255 contiguous information bytes  (\f(CWDW_FORM_block1\fP). 
The second consists of a 2-byte length
followed by 0 to 65,535 contiguous information bytes  (\f(CWDW_FORM_block2\fP). 
The third consists of a 4-byte length
followed by 0 to 4,294,967,295 contiguous information bytes  (\f(CWDW_FORM_block4\fP). 
The fourth consists of an unsigned LEB128 length followed by the number
of bytes specified by the length (\f(CWDW_FORM_block\fP).
In all forms, the length is the number of information bytes that follow.
The information bytes may contain any mixture of relocated (or
relocatable) addresses, references to other debugging information entries or
data bytes.
.LI "constant"
.IX attributes, constants
There are six forms of constants:
one, two, four and eight byte values (respectively,
.Cf DW_FORM_data1 ,
.Cf DW_FORM_data2 ,
.Cf DW_FORM_data4 ,
and
.Cf DW_FORM_data8 ).
.IX variable length data
.IX LEB128
There are also variable length constant data forms encoded
using LEB128 numbers (see below).  Both signed (\f(CWDW_FORM_sdata\fP)
and unsigned (\f(CWDW_FORM_udata\fP) variable length constants are available.
.LI flag
.IX attributes, flags
A flag is represented as a single byte of data (\f(CWDW_FORM_flag\fP). 
If the flag has value zero, it indicates the absence of the attribute.
If the flag has a non-zero value, it indicates the presence of
the attribute.
.LI reference
.IX attributes, references
There are two types of reference.  The first is an
offset relative to the first byte of the compilation unit header
for the compilation unit containing the reference.
The offset must refer to an entry within
that same compilation unit.  There are five forms for this
type of reference:
one, two, four and eight byte offsets (respectively,
.Cf DW_FORM_ref1 ,
.Cf DW_FORM_ref2 ,
.Cf DW_FORM_ref4 ,
and
.Cf DW_FORM_ref8 ).
There are is also an unsigned variable length offset encoded
using LEB128 numbers (\f(CWDW_FORM_ref_udata\fP).
.P
The second type of reference
is the address of any debugging information entry within
the same executable or shared object; it may refer to an entry
in a different compilation unit from the unit containing the
reference.  This type of reference (\f(CWDW_FORM_ref_addr\fP) is the
size of an address on the target architecture; it is relocatable
in a relocatable object file and relocated in an executable file
or shared object.
.P
.I
The use of compilation unit relative references will reduce
the number of link-time relocations and so speed up linking.
.P
The use of address-type references allows for the commonization
of information, such as types, across compilation units.
.R
.LI string
.IX attributes, strings
A string is a sequence of contiguous non-null bytes followed by one null
byte.  A string may be represented immediately in the debugging information
entry itself (\f(CWDW_FORM_string\fP), or may be represented as a 4-byte offset
into a string table contained in the 
.Cf .debug_str
.IX \f(CW.debug_str\fP %debugas
.IX string table
section of the object file (\f(CWDW_FORM_strp\fP).
.LE
.P
.nr aX \n(Fg+1
The form encodings are listed in Figure \n(aX.
.DF
.TS
box center;
l l l
lf(CW) lf(CW) l
. 
Form name	Value	Class
_
DW_FORM_addr	0x01	address
DW_FORM_block2	0x03	block
DW_FORM_block4	0x04	block
DW_FORM_data2	0x05	constant
DW_FORM_data4	0x06	constant
DW_FORM_data8	0x07	constant
DW_FORM_string	0x08	string
DW_FORM_block	0x09	block
DW_FORM_block1	0x0a	block
DW_FORM_data1	0x0b	constant
DW_FORM_flag	0x0c	flag
DW_FORM_sdata	0x0d	constant
DW_FORM_strp	0x0e	string
DW_FORM_udata	0x0f	constant
DW_FORM_ref_addr	0x10	reference
DW_FORM_ref1	0x11	reference
DW_FORM_ref2	0x12	reference
DW_FORM_ref4	0x13	reference
DW_FORM_ref8	0x14	reference
DW_FORM_ref_udata	0x15	reference
DW_FORM_indirect	0x16	(see section 7.5.3)
.TE
.FG "Attribute form encodings"
.DE
.H 2 "Variable Length Data"
.IX variable length data
.IX LEB128
The special constant data forms
.Cf DW_FORM_sdata
and
.Cf DW_FORM_udata
are encoded using ``Little Endian Base 128'' (LEB128)
numbers. LEB128 is a scheme for encoding integers densely that
exploits the assumption that most integers are small in magnitude.
(This encoding is equally suitable whether the target machine
architecture represents data in big-endian or little-endian order.
It is ``little endian'' only in the sense that it avoids using space
to represent the ``big'' end of an unsigned integer, when the big
end is all zeroes or sign extension bits).
.P
.Cf DW_FORM_udata
(unsigned LEB128) numbers are encoded as follows:
start at the
low order end of an unsigned integer and chop it into 7-bit chunks.
Place each chunk into the low order 7 bits of a byte.  Typically,
several of the high order bytes will be zero; discard them.  Emit the
remaining bytes in a stream, starting with the low order byte;
set the high order bit on each byte except the last emitted byte.
The high bit of zero on the last byte indicates to the decoder
that it has encountered the last byte.
.P
The integer zero is a special case, consisting of a single zero byte.
.P
.I
.nr aX \n(Fg+1
Figure \n(aX gives some examples of
.Cf DW_FORM_udata 
numbers.  The 
.Cf 0x80
in each case is the high order bit of the byte, indicating that
an additional byte follows:
.R
.DF
.TS
box center;
l l l
nf(CW) lf(CW) lf(CW)
.
Number	First byte	Second byte
_
2	2	\(em
127	127	\(em
128	0+0x80	1  
129	1+0x80	1
130	2+0x80	1
12857	57+0x80	100
.TE
.FG "Examples of unsigned LEB128 encodings"
.DE
.P
The encoding for 
.Cf DW_FORM_sdata
(signed, 2s complement LEB128) numbers is similar, except that the
criterion for discarding high order bytes is not whether they are
zero, but whether they consist entirely of sign extension bits.
Consider the 32-bit integer 
.Cf -2 .
The three high level bytes of the number are sign extension, thus LEB128
would represent it as a single byte containing the low order 7 bits,
with the high order bit cleared to indicate the end of the byte
stream.  Note that there is nothing within the LEB128 representation
that indicates whether an encoded number is signed or unsigned.
The decoder must know what type of number to expect.
.P
.I
.nr aX \n(Fg+1
Figure \n(aX gives some examples of 
.Cf DW_FORM_sdata 
numbers.
.R
.P
.I
Appendix 4 gives algorithms for encoding and decoding these forms.
.R
.DF
.TS
box center;
l l l
nf(CW) lf(CW) lf(CW)
.
Number	First byte	Second byte
_
2	2	\(em
-2	0x7e	\(em
127	127+0x80	0
-127	1+0x80	0x7f
128	0+0x80	1
-128	0+0x80	0x7f
129	1+0x80	1
-129	0x7f+0x80	0x7e
.TE
.FG "Examples of signed LEB128 encodings"
.DE
.H 2 "Location Descriptions"
.H 3 "Location Expressions"
.IX locations, descriptions
.IX locations, expressions
A location expression is stored in a block of contiguous bytes.
The bytes form a set of operations.
Each location operation has a 1-byte code
that identifies that operation.  Operations can be followed
by one or more bytes of additional data.  All operations in a
location expression are concatenated from left to right.
The encodings for the operations in a location expression
.IX locations, expressions
.nr aX \n(Fg+1
.nr bX \n(Fg+2
are described in Figures \n(aX and \n(bX.
.DS
.TS
center box;
l l l l
lf(CW) lf(CW) l l
.
Operation	Code	No. of Operands	Notes
_
DW_OP_addr	0x03	1	constant address (size target specific)
DW_OP_deref	0x06	0
DW_OP_const1u	0x08	1	1-byte constant
DW_OP_const1s	0x09	1	1-byte constant
DW_OP_const2u	0x0a	1	2-byte constant
DW_OP_const2s	0x0b	1	2-byte constant
DW_OP_const4u	0x0c	1	4-byte constant
DW_OP_const4s	0x0d	1	4-byte constant
DW_OP_const8u	0x0e	1	8-byte constant
DW_OP_const8s	0x0f	1	8-byte constant
DW_OP_constu	0x10	1	ULEB128 constant
DW_OP_consts	0x11	1	SLEB128 constant
DW_OP_dup	0x12	0	
DW_OP_drop	0x13	0	
DW_OP_over	0x14	0	
DW_OP_pick	0x15	1	1-byte stack index	
DW_OP_swap	0x16	0	
DW_OP_rot	0x17	0	
DW_OP_xderef	0x18	0
DW_OP_abs	0x19	0
DW_OP_and	0x1a	0
DW_OP_div	0x1b	0
DW_OP_minus	0x1c	0
DW_OP_mod	0x1d	0
DW_OP_mul	0x1e	0
DW_OP_neg	0x1f	0
DW_OP_not	0x20	0
DW_OP_or	0x21	0
DW_OP_plus	0x22	0
DW_OP_plus_uconst	0x23	1	ULEB128 addend
DW_OP_shl	0x24	0
DW_OP_shr	0x25	0
DW_OP_shra	0x26	0
.TE
.FG "Location operation encodings, part 1"
.DE
.DS
.TS
center box;
l l l l
lf(CW) lf(CW) l l
.
Operation	Code	No. of Operands	Notes
_
DW_OP_xor	0x27	0
DW_OP_skip	0x2f	1	signed 2-byte constant
DW_OP_bra	0x28	1	signed 2-byte constant
DW_OP_eq	0x29	0	
DW_OP_ge	0x2a	0	
DW_OP_gt	0x2b	0	
DW_OP_le	0x2c	0	
DW_OP_lt	0x2d	0	
DW_OP_ne	0x2e	0	
DW_OP_lit0	0x30	0	literals 0..31 = (DW_OP_LIT0|literal)
DW_OP_lit1	0x31	0	
\.\.\.				
DW_OP_lit31	0x4f	0	
DW_OP_reg0	0x50	0	reg 0..31 = (DW_OP_REG0|regnum)
DW_OP_reg1	0x51	0
\.\.\.				
DW_OP_reg31	0x6f	0
DW_OP_breg0	0x70	1	SLEB128 offset
DW_OP_breg1	0x71	1	base reg 0..31 = (DW_OP_BREG0|regnum)
\.\.\.				
DW_OP_breg31	0x8f	1
DW_OP_regx	0x90	1	ULEB128 register
DW_OP_fbreg	0x91	1	SLEB128 offset
DW_OP_bregx	0x92	2	ULEB128 register followed by SLEB128 offset
DW_OP_piece	0x93	1	ULEB128 size of piece addressed
DW_OP_deref_size	0x94	1	1-byte size of data retrieved
DW_OP_xderef_size	0x95	1	1-byte size of data retrieved
DW_OP_nop	0x96	0
DW_OP_lo_user	0xe0		
DW_OP_hi_user	0xff		
.TE
.FG "Location operation encodings, part 2"
.DE
.H 3 "Location Lists"
.IX locations, lists
Each entry in a location list consists of two relative addresses
followed by a 2-byte length, followed by a block of contiguous
bytes.  The length specifies the number of bytes in the block
that follows.  The two addresses are the same size as used by
.Cf DW_FORM_addr
on the target machine.
.H 2 "Base Type Encodings"
.nr aX \n(Fg+1
.IX base types
.IX types, base
The values of the constants used in the
.Cf DW_AT_encoding
attribute are given in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Base type encoding name	Value
_
DW_ATE_address	0x1
DW_ATE_boolean	0x2
DW_ATE_complex_float	0x3
DW_ATE_float	0x4
DW_ATE_signed	0x5
DW_ATE_signed_char	0x6
DW_ATE_unsigned	0x7
DW_ATE_unsigned_char	0x8
DW_ATE_lo_user	0x80
DW_ATE_hi_user	0xff
.TE
.FG "Base type encoding values"
.DE
.H 2 "Accessibility Codes"
.nr aX \n(Fg+1
.IX accessibility
.IX declarations, accessibility
The encodings of the constants used in the 
.Cf DW_AT_accessibility
attribute are given in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Accessibility code name	Value
_
DW_ACCESS_public	1
DW_ACCESS_protected	2
DW_ACCESS_private	3
.TE
.FG "Accessibility encodings"
.DE
.H 2 "Visibility Codes"
.nr aX \n(Fg+1
The encodings of the constants used in the 
.Cf DW_AT_visibility
.IX visibility
.IX declarations, visibility
attribute are given in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Visibility code name	Value
_
DW_VIS_local	1
DW_VIS_exported	2
DW_VIS_qualified	3
.TE
.FG "Visibility encodings"
.DE
.H 2 "Virtuality Codes"
.nr aX \n(Fg+1
.IX virtuality
The encodings of the constants used in the 
.Cf DW_AT_virtuality
attribute are given in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Virtuality code name	Value
_
DW_VIRTUALITY_none	0
DW_VIRTUALITY_virtual	1
DW_VIRTUALITY_pure_virtual	2
.TE
.FG "Virtuality encodings"
.DE
.H 2 "Source Languages"
.nr aX \n(Fg+1
.IX languages
The encodings for source languages are given in Figure \n(aX.
Names marked with \(dg and their associated
values are reserved, but the languages
they represent are not supported in DWARF Version 2.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Language name	Value
_
DW_LANG_C89	0x0001
DW_LANG_C	0x0002
DW_LANG_Ada83\(dg	0x0003
DW_LANG_C_plus_plus	0x0004
DW_LANG_Cobol74\(dg	0x0005
DW_LANG_Cobol85\(dg	0x0006
DW_LANG_Fortran77	0x0007
DW_LANG_Fortran90	0x0008
DW_LANG_Pascal83	0x0009
DW_LANG_Modula2	0x000a
DW_LANG_lo_user	0x8000
DW_LANG_hi_user	0xffff
.TE
.FG "Language encodings"
.DE
.H 2 "Address Class Encodings"
.IX addresses, class
The value of the common address class encoding 
.Cf DW_ADDR_none
is 0.
.H 2 "Identifier Case"
.IX identifiers, case
The encodings of the constants used in the 
.Cf DW_AT_identifier_case
.nr aX \n(Fg+1
attribute are given in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Identifier Case Name	Value
_
DW_ID_case_sensitive	0
DW_ID_up_case	1
DW_ID_down_case	2
DW_ID_case_insensitive	3
.TE
.FG "Identifier case encodings"
.DE
.H 2 "Calling Convention Encodings"
.IX calling conventions
The encodings for the values of the 
.Cf DW_AT_calling_convention 
.nr aX \n(Fg+1
attribute are given in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Calling Convention Name	Value
_
DW_CC_normal	0x1
DW_CC_program	0x2
DW_CC_nocall	0x3
DW_CC_lo_user	0x40
DW_CC_hi_user	0xff
.TE
.FG "Calling convention encodings"
.DE
.H 2 "Inline Codes"
.IX subroutines, inline
The encodings of the constants used in the 
.Cf DW_AT_inline
.nr aX \n(Fg+1
attribute are given in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Inline Code Name	Value
_
DW_INL_not_inlined	0
DW_INL_inlined	1
DW_INL_declared_not_inlined	2
DW_INL_declared_inlined	3
.TE
.FG "Inline encodings"
.DE
.H 2 "Array Ordering"
.IX arrays, ordering
The encodings for the values of the order attributes of arrays
.nr aX \n(Fg+1
is given in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Ordering name	Value
_
DW_ORD_row_major	0
DW_ORD_col_major	1
.TE
.FG "Ordering encodings"
.DE
.H 2 "Discriminant Lists"
.IX variants
.IX discriminated unions
.IX discriminants
The descriptors used in the
.Cf DW_AT_dicsr_list
attribute are encoded as 1-byte constants.
.nr aX \n(Fg+1
The defined values are presented in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
. 
Descriptor Name	Value
_
DW_DSC_label	0
DW_DSC_range	1
.TE
.FG "Discriminant descriptor encodings"
.DE
.H 2 "Name Lookup Table"
.IX lookup, by name
Each set of entries in the table of global names contained in the
.Cf .debug_pubnames
.IX \f(CW.debug_pubnames\fP %debugap
section begins with a header consisting of: a 4-byte length containing
the length of the set of entries for this compilation unit, not including
the length field itself; a 2-byte version identifier containing
the value 2 for DWARF Version 2; a 4-byte offset into the 
.Cf .debug_info 
section; and a 4-byte length containing the size in bytes
of the contents of the 
.Cf .debug_info
section generated to represent this compilation unit.
This header is followed by a series of tuples.  
Each tuple consists of a 4-byte offset
followed by a string of non-null bytes terminated by one null byte.
Each set is terminated by a 4-byte word containing the value 0.
.H 2 "Address Range Table"
.IX lookup, by address
Each set of entries in the table of address ranges contained in the
.Cf .debug_aranges
.IX \f(CW.debug_aranges\fP %debugaar
section begins with a header consisting of: a 4-byte length containing
the length of the set of entries for this compilation unit, not including
the length field itself; a 2-byte version identifier containing
the value 2 for DWARF Version 2; a 4-byte offset into the 
.Cf .debug_info
section;  a 1-byte unsigned integer containing the size in bytes of an 
address (or the offset portion of an address for segmented addressing)
.IX addresses, offset portion
.IX addresses, size of
on the target system; and a 1-byte unsigned integer containing the 
size in bytes of a segment descriptor on the target system.
This header is followed by a series of tuples.  
Each tuple consists of an address and a length, each
in the size appropriate for an address on the target architecture.
The first tuple following the header in each set begins at
an offset that is a multiple of the size of a single tuple
(that is, twice the size of an address).  The header is
padded, if necessary, to the appropriate boundary.
Each set of tuples is terminated by a 0 for the address and 0 for the length.
.H 2 "Line Number Information"
.IX line number information
.IX line number information, definitions
The sizes of the integers used in the line number and
call frame information sections are as follows:
.VL 15
.LI "sbyte"
Signed 1-byte value.
.LI "ubyte"
Unsigned 1-byte value.
.LI "uhalf"
Unsigned 2-byte value.
.LI "sword"
Signed 4-byte value.
.LI "uword"
Unsigned 4-byte value.
.LI
.LE
.P
.IX Version 2
The version number in the statement program prologue is 2 for
DWARF Version 2.
The boolean values ``true'' and ``false'' used by the statement
information program are encoded as a single byte containing the
value 0 for ``false,'' and a non-zero value for ``true.''
The encodings for the pre-defined standard opcodes are given
.IX line number information, standard opcodes
.nr aX \n(Fg+1
in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW)	lf(CW)
.
Opcode Name	Value
_
DW_LNS_copy	1
DW_LNS_advance_pc	2
DW_LNS_advance_line	3
DW_LNS_set_file	4
DW_LNS_set_column	5
DW_LNS_negate_stmt	6
DW_LNS_set_basic_block	7
DW_LNS_const_add_pc	8
DW_LNS_fixed_advance_pc	9
.TE
.FG "Standard Opcode Encodings"
.DE
The encodings for the pre-defined extended opcodes are given
.IX line number information, extended opcodes
.nr aX \n(Fg+1
in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW) lf(CW)
.
Opcode Name	Value
_
DW_LNE_end_sequence	1
DW_LNE_set_address	2
DW_LNE_define_file	3
.TE
.FG "Extended Opcode Encodings"
.DE
.H 2 "Macro Information"
.IX macro information
.IX source, files
The source line numbers and source file indices encoded in the
macro information section are represented as unsigned LEB128 numbers
as are the constants in an
.Cf DW_MACINFO_vendor_ext
entry.
The macinfo type is encoded as a single byte.  The encodings are given
.nr aX \n(Fg+1
in Figure \n(aX.
.DF
.TS
box center;
l l
lf(CW)	lf(CW)
.
Macinfo Type Name	Value
_
DW_MACINFO_define	1
DW_MACINFO_undef	2
DW_MACINFO_start_file	3
DW_MACINFO_end_file	4
DW_MACINFO_vendor_ext	255
.TE
.FG "Macinfo Type Encodings"
.DE
.H 2 "Call Frame Information"
.IX call frame information
The value of the CIE id in the CIE header is
.Cf 0xffffffff .
The initial value of the CIE version number is 1.
.P
Call frame instructions are encoded in one or more bytes.
.IX call frame information, instructions
The primary opcode is encoded in the high order two bits of 
the first byte (that is, opcode = byte >> 6).
An operand or extended opcode may be encoded in the low order
6 bits.   Additional operands are encoded in subsequent bytes.
The instructions and their encodings are presented
.nr aX \n(Fg+1
in Figure \n(aX.
.DS
.TS
center box;
l l l l l
lf(CW) lf(CW) l l
lf(CW) lf(CW) l l
lf(CW) lf(CW) l l
lf(CW) lf(CW) lf(CW) l.
Instruction	High 2 Bits	Low 6 Bits	Operand 1	Operand 2
_
DW_CFA_advance_loc	0x1	delta		
DW_CFA_offset	0x2	register	ULEB128 offset
DW_CFA_restore	0x3	register
DW_CFA_set_loc	0	0x01	address
DW_CFA_advance_loc1	0	0x02	1-byte delta
DW_CFA_advance_loc2	0	0x03	2-byte delta
DW_CFA_advance_loc4	0	0x04	4-byte delta
DW_CFA_offset_extended	0	0x05	ULEB128 register	ULEB128 offset
DW_CFA_restore_extended	0	0x06	ULEB128 register
DW_CFA_undefined	0	0x07	ULEB128 register
DW_CFA_same_value	0	0x08	ULEB128 register
DW_CFA_register	0	0x09	ULEB128 register	ULEB128 register
DW_CFA_remember_state	0	0x0a
DW_CFA_restore_state	0	0x0b
DW_CFA_def_cfa	0	0x0c	ULEB128 register	ULEB128 offset
DW_CFA_def_cfa_register	0	0x0d	ULEB128 register
DW_CFA_def_cfa_offset	0	0x0e	ULEB128 offset
DW_CFA_nop	0	0
DW_CFA_lo_user	0	0x1c
DW_CFA_hi_user	0	0x3f
.TE
.FG "Call frame instruction encodings"
.DE
.H 2 "Dependencies"
The debugging information in this format is intended to exist in the
.Cf .debug_abbrev ,
.Cf .debug_aranges ,
.Cf .debug_frame ,
.Cf .debug_info ,
.Cf .debug_line ,
.Cf .debug_loc ,
.Cf .debug_macinfo ,
.Cf .debug_pubnames 
and
.Cf .debug_str
.IX \f(CW.debug_abbrev\fP %debugaab
.IX \f(CW.debug_aranges\fP %debugaar
.IX \f(CW.debug_frame\fP %debugaf
.IX \f(CW.debug_info\fP %debugai
.IX \f(CW.debug_line\fP %debugali
.IX \f(CW.debug_loc\fP %debugalo
.IX \f(CW.debug_macinfo\fP %debugam
.IX \f(CW.debug_pubnames\fP %debugap
.IX \f(CW.debug_str\fP %debugas
sections of an object file.
The information is not word-aligned, so the assembler must provide a
way for the compiler to produce 2-byte and 4-byte quantities without
alignment restrictions, and the linker must be able to
relocate a 4-byte reference at an arbitrary alignment.
In target architectures with 64-bit addresses, the assembler and linker
must similarly handle 8-byte references at arbitrary alignments.
.OP
.H 1 "FUTURE DIRECTIONS"
The \*(iX \*(tE is working on a specification for a set of interfaces
for reading DWARF information, that will hide changes in the
representation of that information from its consumers.  It is
hoped that using these interfaces will make the transition from
DWARF Version 1 to Version 2 much simpler and will make it
easier for a single consumer to support objects using either
Version 1 or Version 2 DWARF.
.P
A draft of this specification is available for review from
\*(iX. The \*(tE wishes to stress, however, that the specification
is still in flux.
.OP
.HU "Appendix 1 -- Current Attributes by Tag Value"
.P
The list below enumerates the attributes that are most applicable to each type
of debugging information entry.
DWARF does not in general require that a given debugging information
entry contain a particular attribute or set of attributes.  Instead, a
DWARF producer is free to generate any, all, or none of the attributes
described in the text as being applicable to a given entry.  Other
attributes (both those defined within this document but not explicitly
associated with the entry in question, and new, vendor-defined ones)
may also appear in a given debugging entry.
Therefore, the list may be
taken as instructive, but cannot be considered definitive.
.sp
.sp
.DS
.TS
box, tab(:) ;
lfB lfB
lf(CW) lf(CW) .
TAG NAME:APPLICABLE ATTRIBUTES
_
DW_TAG_access_declaration:DECL\(dg
:DW_AT_accessibility
:DW_AT_name
:DW_AT_sibling
_
DW_TAG_array_type:DECL
:DW_AT_abstract_origin
:DW_AT_accessibility
:DW_AT_byte_size
:DW_AT_declaration
:DW_AT_name
:DW_AT_ordering
:DW_AT_sibling
:DW_AT_start_scope
:DW_AT_stride_size
:DW_AT_type
:DW_AT_visibility
_
DW_TAG_base_type:DW_AT_bit_offset
:DW_AT_bit_size
:DW_AT_byte_size
:DW_AT_encoding
:DW_AT_name
:DW_AT_sibling
_
DW_TAG_catch_block:DW_AT_abstract_origin
:DW_AT_high_pc
:DW_AT_low_pc
:DW_AT_segment
:DW_AT_sibling
.TE
.DE
.br
\(dg
.Cf DW_AT_decl_column ,
.Cf DW_AT_decl_file ,
.Cf DW_AT_decl_line .
.SK
.DS
.B "Appendix 1 (cont'd) -- Current Attributes by Tag Value"



.TS
box, tab(:) ;
lfB lfB
lf(CW) lf(CW) .
TAG NAME:APPLICABLE ATTRIBUTES
_
DW_TAG_class_type:DECL
:DW_AT_abstract_origin
:DW_AT_accessibility
:DW_AT_byte_size
:DW_AT_declaration
:DW_AT_name
:DW_AT_sibling
:DW_AT_start_scope
:DW_AT_visibility
_
DW_TAG_common_block:DECL
:DW_AT_declaration
:DW_AT_location
:DW_AT_name
:DW_AT_sibling
:DW_AT_visibility
_
DW_TAG_common_inclusion:DECL
:DW_AT_common_reference
:DW_AT_declaration
:DW_AT_sibling
:DW_AT_visibility
_
DW_TAG_compile_unit:DW_AT_base_types
:DW_AT_comp_dir
:DW_AT_identifier_case
:DW_AT_high_pc
:DW_AT_language
:DW_AT_low_pc
:DW_AT_macro_info
:DW_AT_name
:DW_AT_producer
:DW_AT_sibling
:DW_AT_stmt_list
_
DW_TAG_const_type:DW_AT_sibling
:DW_AT_type
.TE
.DE
.br
.SK
.DS
.B "Appendix 1 (cont'd) -- Current Attributes by Tag Value"



.TS
box, tab(:) ;
lfB lfB
lf(CW) lf(CW) .
TAG NAME:APPLICABLE ATTRIBUTES
_
DW_TAG_constant:DECL
:DW_AT_accessibility
:DW_AT_constant_value
:DW_AT_declaration
:DW_AT_external
:DW_AT_name
:DW_AT_sibling
:DW_AT_start_scope
:DW_AT_type
:DW_AT_visibility
_
DW_TAG_entry_point:DW_AT_address_class
:DW_AT_low_pc
:DW_AT_name
:DW_AT_return_addr
:DW_AT_segment
:DW_AT_sibling
:DW_AT_static_link
:DW_AT_type
_
DW_TAG_enumeration_type:DECL
:DW_AT_abstract_origin
:DW_AT_accessibility
:DW_AT_byte_size
:DW_AT_declaration
:DW_AT_name
:DW_AT_sibling
:DW_AT_start_scope
:DW_AT_visibility
_
DW_TAG_enumerator:DECL
:DW_AT_const_value
:DW_AT_name
:DW_AT_sibling
_
DW_TAG_file_type:DECL
:DW_AT_abstract_origin
:DW_AT_byte_size
:DW_AT_name
:DW_AT_sibling
:DW_AT_start_scope
:DW_AT_type
:DW_AT_visibility
.TE
.DE
.br
.SK
.DS
.B "Appendix 1 (cont'd) -- Current Attributes by Tag Value"



.TS
box, tab(:) ;
lfB lfB
lf(CW) lf(CW) .
TAG NAME:APPLICABLE ATTRIBUTES
_
DW_TAG_formal_parameter:DECL
:DW_AT_abstract_origin
:DW_AT_artificial
:DW_AT_default_value
:DW_AT_is_optional
:DW_AT_location
:DW_AT_name
:DW_AT_segment
:DW_AT_sibling
:DW_AT_type
:DW_AT_variable_parameter
_
DW_TAG_friend:DECL
:DW_AT_abstract_origin
:DW_AT_friend
:DW_AT_sibling
_
DW_TAG_imported_declaration:DECL
:DW_AT_accessibility
:DW_AT_import
:DW_AT_name
:DW_AT_sibling
:DW_AT_start_scope
_
DW_TAG_inheritance:DECL
:DW_AT_accessibility
:DW_AT_data_member_location
:DW_AT_sibling
:DW_AT_type
:DW_AT_virtuality
_
DW_TAG_inlined_subroutine:DECL
:DW_AT_abstract_origin
:DW_AT_high_pc
:DW_AT_low_pc
:DW_AT_segment
:DW_AT_sibling
:DW_AT_return_addr
:DW_AT_start_scope
_
DW_TAG_label:DW_AT_abstract_origin
:DW_AT_low_pc
:DW_AT_name
:DW_AT_segment
:DW_AT_start_scope
:DW_AT_sibling
.TE
.DE
.br
.SK
.DS
.B "Appendix 1 (cont'd) -- Current Attributes by Tag Value"



.TS
box, tab(:) ;
lfB lfB
lf(CW) lf(CW) .
TAG NAME:APPLICABLE ATTRIBUTES
_
DW_TAG_lexical_block:DW_AT_abstract_origin
:DW_AT_high_pc
:DW_AT_low_pc
:DW_AT_name
:DW_AT_segment
:DW_AT_sibling
_
DW_TAG_member:DECL
:DW_AT_accessibility
:DW_AT_byte_size
:DW_AT_bit_offset
:DW_AT_bit_size
:DW_AT_data_member_location
:DW_AT_declaration
:DW_AT_name
:DW_AT_sibling
:DW_AT_type
:DW_AT_visibility
_
DW_TAG_module:DECL
:DW_AT_accessibility
:DW_AT_declaration
:DW_AT_high_pc
:DW_AT_low_pc
:DW_AT_name
:DW_AT_priority
:DW_AT_segment
:DW_AT_sibling
:DW_AT_visibility
_
DW_TAG_namelist:DECL
:DW_AT_accessibility
:DW_AT_abstract_origin
:DW_AT_declaration
:DW_AT_sibling
:DW_AT_visibility
_
DW_TAG_namelist_item:DECL
:DW_AT_namelist_item
:DW_AT_sibling
_
DW_TAG_packed_type:DW_AT_sibling
:DW_AT_type
.TE
.DE
.br
.SK
.DS
.B "Appendix 1 (cont'd) -- Current Attributes by Tag Value"



.TS
box, tab(:) ;
lfB lfB
lf(CW) lf(CW) .
TAG NAME:APPLICABLE ATTRIBUTES
_
DW_TAG_pointer_type:DW_AT_address_class
:DW_AT_sibling
:DW_AT_type
_
DW_TAG_ptr_to_member_type:DECL
:DW_AT_abstract_origin
:DW_AT_address_class
:DW_AT_containing_type
:DW_AT_declaration
:DW_AT_name
:DW_AT_sibling
:DW_AT_type
:DW_AT_use_location
:DW_AT_visibility
_
DW_TAG_reference_type:DW_AT_address_class
:DW_AT_sibling
:DW_AT_type
_
DW_TAG_set_type:DECL
:DW_AT_abstract_origin
:DW_AT_accessibility
:DW_AT_byte_size
:DW_AT_declaration
:DW_AT_name
:DW_AT_start_scope
:DW_AT_sibling
:DW_AT_type
:DW_AT_visibility
_
DW_TAG_string_type:DECL
:DW_AT_accessibility
:DW_AT_abstract_origin
:DW_AT_byte_size
:DW_AT_declaration
:DW_AT_name
:DW_AT_segment
:DW_AT_sibling
:DW_AT_start_scope
:DW_AT_string_length
:DW_AT_visibility
.TE
.DE
.SK
.DS
.B "Appendix 1 (cont'd) -- Current Attributes by Tag Value"



.TS
box, tab(:) ;
lfB lfB
lf(CW) lf(CW) .
TAG NAME:APPLICABLE ATTRIBUTES
_
DW_TAG_structure_type:DECL
:DW_AT_abstract_origin
:DW_AT_accessibility
:DW_AT_byte_size
:DW_AT_declaration
:DW_AT_name
:DW_AT_sibling
:DW_AT_start_scope
:DW_AT_visibility
_
DW_TAG_subprogram:DECL
:DW_AT_abstract_origin
:DW_AT_accessibility
:DW_AT_address_class
:DW_AT_artificial
:DW_AT_calling_convention
:DW_AT_declaration
:DW_AT_external
:DW_AT_frame_base
:DW_AT_high_pc
:DW_AT_inline
:DW_AT_low_pc
:DW_AT_name
:DW_AT_prototyped
:DW_AT_return_addr
:DW_AT_segment
:DW_AT_sibling
:DW_AT_specification
:DW_AT_start_scope
:DW_AT_static_link
:DW_AT_type
:DW_AT_visibility
:DW_AT_virtuality
:DW_AT_vtable_elem_location
.TE
.DE
.SK
.DS
.B "Appendix 1 (cont'd) -- Current Attributes by Tag Value"



.TS
box, tab(:) ;
lfB lfB
lf(CW) lf(CW) .
TAG NAME:APPLICABLE ATTRIBUTES
_
DW_TAG_subrange_type:DECL
:DW_AT_abstract_origin
:DW_AT_accessibility
:DW_AT_byte_size
:DW_AT_count
:DW_AT_declaration
:DW_AT_lower_bound
:DW_AT_name
:DW_AT_sibling
:DW_AT_type
:DW_AT_upper_bound
:DW_AT_visibility
_
DW_TAG_subroutine_type:DECL
:DW_AT_abstract_origin
:DW_AT_accessibility
:DW_AT_address_class
:DW_AT_declaration
:DW_AT_name
:DW_AT_prototyped
:DW_AT_sibling
:DW_AT_start_scope
:DW_AT_type
:DW_AT_visibility
_
DW_TAG_template_type_param:DECL
:DW_AT_name
:DW_AT_sibling
:DW_AT_type
_
DW_TAG_template_value_param:DECL
:DW_AT_name
:DW_AT_const_value
:DW_AT_sibling
:DW_AT_type
_
DW_TAG_thrown_type:DECL
:DW_AT_sibling
:DW_AT_type
_
DW_TAG_try_block:DW_AT_abstract_origin
:DW_AT_high_pc
:DW_AT_low_pc
:DW_AT_segment
:DW_AT_sibling
.TE
.DE
.br
.SK
.DS
.B "Appendix 1 (cont'd) -- Current Attributes by Tag Value"



.TS
box, tab(:) ;
lfB lfB
lf(CW) lf(CW) .
TAG NAME:APPLICABLE ATTRIBUTES
_
DW_TAG_typedef:DECL
:DW_AT_abstract_origin
:DW_AT_accessibility
:DW_AT_declaration
:DW_AT_name
:DW_AT_sibling
:DW_AT_start_scope
:DW_AT_type
:DW_AT_visibility
_
DW_TAG_union_type:DECL
:DW_AT_abstract_origin
:DW_AT_accessibility
:DW_AT_byte_size
:DW_AT_declaration
:DW_AT_friends
:DW_AT_name
:DW_AT_sibling
:DW_AT_start_scope
:DW_AT_visibility
_
DW_TAG_unspecified_parameters:DECL
:DW_AT_abstract_origin
:DW_AT_artificial
:DW_AT_sibling
_
DW_TAG_variable:DECL
:DW_AT_accessibility
:DW_AT_constant_value
:DW_AT_declaration
:DW_AT_external
:DW_AT_location
:DW_AT_name
:DW_AT_segment
:DW_AT_sibling
:DW_AT_specification
:DW_AT_start_scope
:DW_AT_type
:DW_AT_visibility
.TE
.DE
.br
.SK
.DS
.B "Appendix 1 (cont'd) -- Current Attributes by Tag Value"



.TS
box, tab(:) ;
lfB lfB
lf(CW) lf(CW) .
TAG NAME:APPLICABLE ATTRIBUTES
_
DW_TAG_variant:DECL
:DW_AT_accessibility
:DW_AT_abstract_origin
:DW_AT_declaration
:DW_AT_discr_list
:DW_AT_discr_value
:DW_AT_sibling
_
DW_TAG_variant_part:DECL
:DW_AT_accessibility
:DW_AT_abstract_origin
:DW_AT_declaration
:DW_AT_discr
:DW_AT_sibling
:DW_AT_type
_
DW_TAG_volatile_type:DW_AT_sibling
:DW_AT_type
_
DW_TAG_with_statement:DW_AT_accessibility
:DW_AT_address_class
:DW_AT_declaration
:DW_AT_high_pc
:DW_AT_location
:DW_AT_low_pc
:DW_AT_segment
:DW_AT_sibling
:DW_AT_type
:DW_AT_visibility
.TE
.DE
.SK
.OP
.HU "Appendix 2 -- Organization of Debugging Information"
The following diagram depicts the relationship of the abbreviation
tables contained in the 
.Cf .debug_abbrev
section to the information contained in the 
.Cf .debug_info
section.  Values are given in symbolic form, where possible.
.DF
.nf
.PS
scale=100
define t201 |
[ box invis ht 154 wid 295 with .sw at 0,0
"\f(CW\s9\&1\f1\s0" at 0,147 ljust
"\f(CW\s9\&DW_TAG_compile_unit\f1\s0" at 0,133 ljust
"\f(CW\s9\&DW_CHILDREN_yes\f1\s0" at 0,119 ljust
"\f(CW\s9\&DW_AT_name          DW_FORM_string\f1\s0" at 0,105 ljust
"\f(CW\s9\&DW_AT_producer      DW_FORM_string\f1\s0" at 0,91 ljust
"\f(CW\s9\&DW_AT_compdir       DW_FORM_string\f1\s0" at 0,77 ljust
"\f(CW\s9\&DW_AT_language      DW_FORM_data1\f1\s0" at 0,63 ljust
"\f(CW\s9\&DW_AT_low_poc       DW_FORM_addr\f1\s0" at 0,49 ljust
"\f(CW\s9\&DW_AT_high_pc       DW_FORM_addr\f1\s0" at 0,35 ljust
"\f(CW\s9\&DW_AT_stmt_list     DW_FORM_indirect\f1\s0" at 0,21 ljust
"\f(CW\s9\&0                   0\f1\s0" at 0,7 ljust
] |

define t103 |
[ box invis ht 42 wid 74 with .sw at 0,0
"\f(CW\s9\&4\f1\s0" at 0,35 ljust
"\f(CW\s9\&\"POINTER\"\f1\s0" at 0,21 ljust
"\f(CW\s9\&\f1\s0" at 0,7 ljust
] |

define t177 |
[ box invis ht 28 wid 13 with .sw at 0,0
"\f(CW\s9\&3\f1\s0" at 0,21 ljust
"\f(CW\s9\&\f1\s0" at 0,7 ljust
] |

define t224 |
[ box invis ht 84 wid 280 with .sw at 0,0
"\f(CW\s9\&4\f1\s0" at 0,77 ljust
"\f(CW\s9\&DW_TAG_typedef\f1\s0" at 0,63 ljust
"\f(CW\s9\&DW_CHILDREN_no\f1\s0" at 0,49 ljust
"\f(CW\s9\&DW_AT_name          DW_FORM_string\f1\s0" at 0,35 ljust
"\f(CW\s9\&DW_AT_type          DW_FORM_ref4 \f1\s0" at 0,21 ljust
"\f(CW\s9\&0                   0            \f1\s0" at 0,7 ljust
] |

define t149 |
[ box invis ht 28 wid 51 with .sw at 0,0
"\f(CW\s9\&4\f1\s0" at 0,21 ljust
"\f(CW\s9\&\"strp\"\f1\s0" at 0,7 ljust
] |

define t205 |
[ box invis ht 98 wid 280 with .sw at 0,0
"\f(CW\s9\&2\f1\s0" at 0,91 ljust
"\f(CW\s9\&DW_TAG_base_type\f1\s0" at 0,77 ljust
"\f(CW\s9\&DW_CHILDREN_no\f1\s0" at 0,63 ljust
"\f(CW\s9\&DW_AT_name          DW_FORM_string\f1\s0" at 0,49 ljust
"\f(CW\s9\&DW_AT_encoding      DW_FORM_data1\f1\s0" at 0,35 ljust
"\f(CW\s9\&DW_AT_byte_size     DW_FORM_data1\f1\s0" at 0,21 ljust
"\f(CW\s9\&0                   0\f1\s0" at 0,7 ljust
] |

define t126 |
[ box invis ht 126 wid 257 with .sw at 0,0
"\f(CW\s9\&\"myfile.c\"\f1\s0" at 0,119 ljust
"\f(CW\s9\&\"Best Compiler Corp: Version 1.3\"\f1\s0" at 0,105 ljust
"\f(CW\s9\&\"mymachine:/home/mydir/src:\"\f1\s0" at 0,91 ljust
"\f(CW\s9\&DW_LANG_C89\f1\s0" at 0,77 ljust
"\f(CW\s9\&0x0\f1\s0" at 0,63 ljust
"\f(CW\s9\&0x55\f1\s0" at 0,49 ljust
"\f(CW\s9\&DW_FORM_data4\f1\s0" at 0,35 ljust
"\f(CW\s9\&0x0\f1\s0" at 0,21 ljust
"\f(CW\s9\&\f1\s0" at 0,7 ljust
] |

define t219 |
[ box invis ht 70 wid 260 with .sw at 0,0
"\f(CW\s9\&3\f1\s0" at 0,63 ljust
"\f(CW\s9\&DW_TAG_pointer_type\f1\s0" at 0,49 ljust
"\f(CW\s9\&DW_CHILDREN_no\f1\s0" at 0,35 ljust
"\f(CW\s9\&DW_AT_type          DW_FORM_ref4\f1\s0" at 0,21 ljust
"\f(CW\s9\&0                   0\f1\s0" at 0,7 ljust
] |

define t109 |
[ box invis ht 42 wid 165 with .sw at 0,0
"\f(CW\s9\&\"char\"\f1\s0" at 0,35 ljust
"\f(CW\s9\&DW_ATE_unsigned_char\f1\s0" at 0,21 ljust
"\f(CW\s9\&1\f1\s0" at 0,7 ljust
] |

box invis ht 704 wid 680 with .sw at 0,0
t201 with .nw at 376,657
box ht 520 wid 320 with .nw at 360,672 
box ht 208 wid 280 with .nw at 24,208 
t103 with .nw at 40,353
t177 with .nw at 40,398
line  from 360,176 to 680,176 
line  from 360,280 to 680,280 
line  from 360,368 to 680,368 
line  from 360,488 to 680,488 
t224 with .nw at 376,270
"\f(CW\s9\&0\f1\s0" at 376,164 ljust
"\f(CW\s9\&0\f1\s0" at 40,289 ljust
"\fI\s9\&e2\f1\s0" at 40,317 ljust
"\fI\s9\&e2:\f1\s0" at 0,389 ljust
"\f(CW\s9\&2\f1\s0" at 44,176 ljust
line  from 24,128 to 304,128 
"\f(CW\s9\&...\f1\s0" at 44,113 ljust
t149 with .nw at 44,88
"\fI\s9\&e2\f1\s0" at 44,49 ljust
"\f(CW\s9\&...\f1\s0" at 44,17 ljust
box ht 416 wid 280 with .nw at 24,688 
"\fI\s9\&length\f1\s0" at 44,192 ljust
"\f(CW\s9\&4\f1\s0" at 48,140
"\fI\s9\&a1  (abbreviation table offset)\f1\s0" at 44,160 ljust
"\f(CW\s9\&4\f1\s0" at 44,624
"\fI\s9\&a1  (abbreviation table offset)\f1\s0" at 40,640 ljust
t205 with .nw at 376,477
"\fI\s9\&a1:\f1\s0" at 348,657 rjust
"\fI\s9\&length\f1\s0" at 40,672 ljust
"\fR\s10\&Abbreviation Table - .debug_abbrev\f1\s0" at 384,678 ljust
"\fR\s10\&Compilation Unit 1 - .debug_info\f1\s0" at 68,694 ljust
"\fR\s10\&Compilation Unit 2 - .debug_info\f1\s0" at 64,218 ljust
"\f(CW\s9\&2\f1\s0" at 44,656
"\f(CW\s9\&1\f1\s0" at 44,605
t126 with .nw at 36,599
line  from 24,616 to 304,616 
"\f(CW\s9\&2\f1\s0" at 40,461 ljust
t219 with .nw at 376,359
line  from 24,96 to 304,96 
line  from 24,32 to 304,32 
t109 with .nw at 40,449
"\fI\s9\&e1\f1\s0" at 40,373 ljust
"\fI\s9\&e1:\f1\s0" at 0,461 ljust
line  from 24,480 to 304,480 
line  from 24,400 to 304,400 
line  from 24,360 to 304,360 
line  from 24,304 to 304,304 
.PE
.fi
.DE
.SK
.OP
.HU "Appendix 3 -- Statement Program Examples"
.P
Consider this simple source file and the resulting machine code for
the Intel 8086 processor:
.DS
.S -2
.TS
;
lf(CW) lf(CW) s
lf(CW) lf(CW) s
lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) s
lf(CW) lf(CW) s
lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) s
lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) s
lf(CW) lf(CW) lf(CW) 
lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) s
lf(CW) lf(CW) lf(CW).
1:	int
2:	main()
	0x239:	push pb
	0x23a:	mov bp,sp
3:	{
4:	printf("Omit needless words\en");
	0x23c:	mov ax,0xaa
	0x23f:	push ax
	0x240:	call _printf
	0x243:	pop cx
5:	exit(0);
	0x244:	xor ax,ax
	0x246:	push ax
	0x247:	call _exit
	0x24a:	pop cx
6:	}
	0x24b:	pop bp
	0x24c:	ret
7:
	0x24d:
.TE
.S +2
.DE
.P
If the statement program prologue specifies the following:
.DS
.S -2
.TS
;
lf(CW) lf(CW).
minimum_instruction_length	1
opcode_base	10
line_base	1
line_range	15
.TE
.S +2
.DE
.P
Then one encoding of the statement program would occupy 12 bytes
(the opcode \f(CWSPECIAL(\fIm\fP, \fIn\fP)\fR indicates the special
opcode generated for a line increment of \fIm\fP and an address increment
of \fIn\fP):
.DS
.S -2
.TS
;
l l l
lf(CW) lf(CW) lf(CW).
Opcode	Operand	Byte Stream
_
DW_LNS_advance_pc	LEB128(0x239)	0x2, 0xb9, 0x04
SPECIAL(2, 0)		0xb
SPECIAL(2, 3)		0x38
SPECIAL(1, 8)		0x82
SPECIAL(1, 7)		0x73
DW_LNS_advance_pc	LEB128(2)	0x2, 0x2
DW_LNE_end_sequence		0x0, 0x1, 0x1
.TE
.S +2
.DE
.P
An alternate encoding of the same program using standard opcodes to
advance the program counter would occupy 22 bytes:
.DS
.S -2
.TS
;
l l l
lf(CW) lf(CW) lf(CW).
Opcode	Operand	Byte Stream
_
DW_LNS_fixed_advance_pc	0x239	0x9, 0x39, 0x2
SPECIAL(2, 0)		0xb
DW_LNS_fixed_advance_pc	0x3	0x9, 0x3, 0x0
SPECIAL(2, 0)		0xb
DW_LNS_fixed_advance_pc	0x8	0x9, 0x8, 0x0
SPECIAL(1, 0)		0xa
DW_LNS_fixed_advance_pc	0x7	0x9, 0x7, 0x0
SPECIAL(1, 0)		0xa
DW_LNS_fixed_advance_pc	0x2	0x9, 0x2, 0x0
DW_LNE_end_sequence		0x0, 0x1, 0x1
.TE
.S +2
.DE
.SK
.OP
.HU "Appendix 4 -- Encoding and decoding variable length data"
.ta .5i +.5i +.5i +.5i +.5i +.5i +.5i +.5i
.P
Here are algorithms expressed in a C-like pseudo-code to encode and decode
signed and unsigned numbers in LEB128:
.P
\fBEncode an unsigned integer:\fP
.br
.DS
.S -2
\f(CWdo
{		
	byte = low order 7 bits of value;
	value >>= 7;
	if (value != 0)	/* more bytes to come */
		set high order bit of byte;
	emit byte;
} while (value != 0);\fP
.S +2
.DE
.P
\fBEncode a signed integer:\fP
.br
.DS
.S -2
\f(CWmore = 1;
negative = (value < 0);
size = no. of bits in signed integer;
while(more)
{
	byte = low order 7 bits of value;
	value >>= 7;
	/* the following is unnecessary if the implementation of >>=
	 * uses an arithmetic rather than logical shift for a signed
	 * left operand
	 */
	if (negative)
		/* sign extend */
		value |= - (1 << (size - 7));
	/* sign bit of byte is 2nd high order bit (0x40) */
	if ((value == 0 && sign bit of byte is clear) ||
		(value == -1 && sign bit of byte is set))
		more = 0;
	else
		set high order bit of byte;
	emit byte;
}\fP
.S +2
.DE
.SK
.ta .5i +.5i +.5i +.5i +.5i +.5i +.5i +.5i
.P
\fBDecode unsigned LEB128 number:\fP
.br
.DS
.S -2
\f(CWresult = 0;
shift = 0;
while(true)
{
	byte = next byte in input;
	result |= (low order 7 bits of byte << shift);
	if (high order bit of byte == 0)
		break;
	shift += 7;
}\fP
.S +2
.DE
.P
\fBDecode signed LEB128 number:\fP
.br
.DS
.S -2
\f(CWresult = 0;
shift = 0;
size = no. of bits in signed integer;
while(true)
{
	byte = next byte in input;
	result |= (low order 7 bits of byte << shift);
	shift += 7;
	/* sign bit of byte is 2nd high order bit (0x40) */
	if (high order bit of byte == 0)
		break;
}
if ((shift < size) && (sign bit of byte is set))
	/* sign extend */
	result |= - (1 << shift);\fP
.S +2
.DE
.SK
.OP
.HU "Appendix 5 -- Call Frame Information Examples"
The following example uses a hypothetical RISC machine in the style of
the Motorola 88000.
.BL
.LI
Memory is byte addressed.
.LI
Instructions are all 4-bytes each and word aligned.
.LI
Instruction operands are typically of the form:
.br
.DS
	<destination reg> <source reg> <constant>
.DE
.LI
The address for the load and store instructions is computed by
adding the contents of the source register with the constant.
.LI
There are 8 4-byte registers:
.br
.DS
	R0 always 0
	R1 holds return address on call
	R2-R3 temp registers (not preserved on call)
	R4-R6 preserved on call
	R7 stack pointer.
.DE
.LI
The stack grows in the negative direction.
.LE
.P
The following are two code fragments from a subroutine 
called \f(CWfoo\fP that
uses a frame pointer (in addition to the stack pointer.)  The first
column values are byte addresses.
.DS
.S -2
.TS
;
lf(CW) lf(CW) s  s
lf(CW) lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) lf(CW) lf(CW)
lf(CW) lf(CW) s s
lf(CW) lf(CW) s s
lf(CW) lf(CW) s s
lf(CW) lf(CW) lf(CW) lf(CW).
	;; start prologue
foo	sub	R7, R7, <fsize>         ; Allocate frame
foo+4	store	R1, R7, (<fsize>-4)     ; Save the return address
foo+8	store	R6, R7, (<fsize>-8)     ; Save R6
foo+12	add	R6, R7, 0               ; R6 is now the Frame ptr
foo+16	store	R4, R6, (<fsize>-12)    ; Save a preserve reg.
	;; This subroutine does not change R5
	...
	;; Start epilogue (R7 has been returned to entry value)
foo+64	load	R4, R6, (<fsize>-12)    ; Restore R4
foo+68	load	R6, R7, (<fsize>-8)     ; Restore R6
foo+72	load	R1, R7, (<fsize>-4)     ; Restore return address
foo+76	add	R7, R7, <fsize>         ; Deallocate frame
foo+80	jump	R	; Return
foo+84
.TE
.S +2
.DE
.SK
The table for the \f(CWfoo\fP subroutine is as follows.  
It is followed by the
corresponding fragments from the 
.Cf .debug_frame 
section.
.DS
.S -2
.TS
tab(|);
lf(CW) lf(CW) lf(CW) lf(CW) lf(CW) lf(CW) lf(CW) lf(CW) lf(CW) lf(CW) lf(CW).
Loc|CFA|R0|R1|R2|R3|R4|R5|R6|R7|R8
foo|[R7]+0|s|u|u|u|s|s|s|s|r1
foo+4|[R7]+fsize|s|u|u|u|s|s|s|s|r1
foo+8|[R7]+fsize|s|u|u|u|s|s|s|s|c4
foo+12|[R7]+fsize|s|u|u|u|s|s|c8|s|c4
foo+16|[R6]+fsize|s|u|u|u|s|s|c8|s|c4
foo+20|[R6]+fsize|s|u|u|u|c12|s|c8|s|c4
...
foo+64|[R6]+fsize|s|u|u|u|c12|s|c8|s|c4
foo+68|[R6]+fsize|s|u|u|u|s|s|c8|s|c4
foo+72|[R7]+fsize|s|u|u|u|s|s|s|s|c4
foo+76|[R7]+fsize|s|u|u|u|s|s|s|s|r1
foo+80|[R7]+0|s|u|u|u|s|s|s|s|r1
.TE
.TS
;
l s
l l.
notes:
1.	R8 is the return address
2.	s = same_value rule
3.	u = undefined rule
4.	rN = register(N) rule
5.	cN = offset(N) rule
.sp
.sp
.TE
.S +2
.DE
.P
Common Information Entry (CIE):
.DS
.S -2
.TS
;
lf(CW) lf(CW) lf(CW).
cie	32	; length
cie+4	0xffffffff	; CIE_id
cie+8	1 	; version
cie+9	0	; augmentation
cie+10	4	; code_alignment_factor
cie+11	4	; data_alignment_factor
cie+12	8	; R8 is the return addr.
cie+13	DW_CFA_def_cfa (7, 0)	; CFA = [R7]+0
cie+16	DW_CFA_same_value (0)	; R0 not modified (=0)
cie+18	DW_CFA_undefined (1)	; R1 scratch
cie+20	DW_CFA_undefined (2)	; R2 scratch
cie+22	DW_CFA_undefined (3)	; R3 scratch
cie+24	DW_CFA_same_value (4)	; R4 preserve
cie+26	DW_CFA_same_value (5)	; R5 preserve
cie+28	DW_CFA_same_value (6)	; R6 preserve
cie+30	DW_CFA_same_value (7)	; R7 preserve
cie+32	DW_CFA_register (8, 1)	; R8 is in R1
cie+35	DW_CFA_nop	; padding
cie+36
.TE
.S +2
.DE
.SK
.P
Frame Description Entry (FDE):
.DS
.S -2
.TS
;
lf(CW) lf(CW) lf(CW).
fde	40	; length
fde+4	cie	; CIE_ptr
fde+8	foo	; initial_location
fde+12	84	; address_range
fde+16	DW_CFA_advance_loc(1)	; instructions
fde+17	DW_CFA_def_cfa_offset(<fsize>/4)	; assuming <fsize> < 512
fde+19	DW_CFA_advance_loc(1)
fde+20	DW_CFA_offset(8,1)
fde+22	DW_CFA_advance_loc(1)
fde+23	DW_CFA_offset(6,2)
fde+25	DW_CFA_advance_loc(1)
fde+26	DW_CFA_def_cfa_register(6)
fde+28	DW_CFA_advance_loc(1)
fde+29	DW_CFA_offset(4,3)
fde+31	DW_CFA_advance_loc(11)
fde+32	DW_CFA_restore(4)
fde+33	DW_CFA_advance_loc(1)
fde+34	DW_CFA_restore(6)
fde+35	DW_CFA_def_cfa_register(7)
fde+37	DW_CFA_advance_loc(1)
fde+38	DW_CFA_restore(8)
fde+39	DW_CFA_advance_loc(1)
fde+40	DW_CFA_def_cfa_offset(0)
fde+42	DW_CFA_nop	; padding
fde+43	DW_CFA_nop	; padding
fde+44
.TE
.S +2
.DE
.S +1

'\"
'\"  Table of Contents stuff
'\"
.de TP
.sp 4
..
.VM
.de TY
.ce 1
Table of Contents
.sp
..
.nr Lf 1
.ds Lf List of Figures
.SK
.TC 1 1 7 0
