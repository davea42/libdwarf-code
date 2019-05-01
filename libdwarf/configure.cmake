#   This changes the gennames option from -s  to -t
option(namestable "string functions implemented as binary search (default is with C switch)" TRUE)
if(namestable)
    set(dwarf_namestable "-s")
else()
    set(dwarf_namestable "-t")
endif()

option(windowspath "Detect certain Windows paths as full paths (default is NO)" FALSE)
set(HAVE_WINDOWS_PATH ${windowspath})
message(STATUS "Checking enable  windowspath... ${HAVE_WINDOWS_PATH}")

option(oldframecol "Use HAVE_OLD_FRAME_CFA_COL (default is to use new DW_FRAME_CFA_COL3)" FALSE)
set(HAVE_OLD_FRAME_CFA_COL ${oldframecol})
message(STATUS "Checking enable old frame columns... ${HAVE_OLD_FRAME_CFA_COL}")

#  See pro_init(), HAVE_DWARF2_99_EXTENSION also generates
#  32bit offset dwarf unless DW_DLC_OFFSET_SIZE_64 flag passed to
#  pro_init.
option(dwarf_format_sgi_irix "Force producer to SGI IRIX offset dwarf" FALSE)
set(HAVE_SGI_IRIX_OFFSETS ${dwarf_format_sgi_irix})
message(STATUS "Checking  producer generates SGI IRIX output... ${HAVE_SGI_IRIX_OFFSETS}")

option(dwarf_format_strict_32bit "Force producer to generate only DWARF format 32bit" FALSE)
set(HAVE_STRICT_DWARF2_32BIT_OFFSET ${dwarf_format_strict_32bit})
set(HAVE_DWARF2_99_EXTENSION NOT ${dwarf_format_strict_32bit})
message(STATUS "Checking producer generates only 32bit... ${HAVE_STRICT_DWARF2_32BIT_OFFSET}")

configure_file(config.h.in.cmake config.h)
