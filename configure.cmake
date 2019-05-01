include(AutoconfHelper)

set(PACKAGE_NAME "libdwarf" )
set(PACKAGE_VERSION "20190430"  )
set(VERSION ${PACKAGE_VERSION} )
set(PACKAGE_STRING "\"${PACKAGE_NAME} ${VERSION}\"")
string(REGEX REPLACE "[\"]" "" tarname1 "${PACKAGE_STRING}" )
string(REGEX REPLACE "[^a-zA-Z0-9_]" "-" tarname "${tarname1}" )
set(PACKAGE_TARNAME "\"${tarname}\"" )

include(TestBigEndian)
test_big_endian(isBigEndian)
if (isBigEndian)
  set ( WORDS_BIGENDIAN 1 )
endif()

include(CheckIncludeFile)
check_include_file( "sys/types.h" HAVE_SYS_TYPES_H ) 
check_include_file( "sys/stat.h" HAVE_SYS_STAT_H ) 
check_include_file( "stdlib.h" HAVE_STDLIB_H     ) 
check_include_file( "string.h" HAVE_STRING_H     ) 
check_include_file( "memory.h" HAVE_MEMORY_H     ) 
check_include_file( "strings.h" HAVE_STRINGS_H   ) 
check_include_file( "inttypes.h" HAVE_INTTYPES_H ) 
check_include_file( "stdint.h" HAVE_STDINT_T     )
check_include_file( "unistd.h" HAVE_UNISTD_H     )
check_include_file( "sgidefs.h" HAVE_SGIDEFS_H )
check_include_file( "stdafx.h" HAVE_STDAFX_H )
check_include_file( "Windows.h" HAVE_WINDOWS_H )
check_include_file( "elf.h" HAVE_ELF_H ) 
check_include_file( "libelf.h" HAVE_LIBELF_H ) 
check_include_file( "alloca.h" HAVE_ALLOCA_H )
check_include_file( "elfaccess.h" HAVE_ELFACCESS_H )
check_include_file( "sys/elf_386.h" HAVE_ELF_386_H )
check_include_file( "sys/elf_amd64.h" HAVE_SYS_ELF_AMD64_H )
check_include_file( "sys/elf_sparc.h" HAVE_SYS_ELF_SPARC_H )
check_include_file( "sys/ia64/elf.h" HAVE_SYS_IA64_ELF_H)


set (CMAKE_REQUIRED_LIBRARIES elf)
check_function_exists( elf64_getehdr HAVE_ELF64_GETEHDR)
check_function_exists( elf64_getshdr HAVE_ELF64_GETSHDR)
set (CMAKE_REQUIRED_LIBRARIES)

if(HAVE_ELF_H)
    set(HAVE_LOCATION_OF_LIBELFHEADER "<elf.h>")
elseif(HAVE_LIBELF_H)
    set(HAVE_LOCATION_OF_LIBELFHEADER "<libelf.h>")
elseif(HAVE_LIBELF_LIBELF_H)
    set(HAVE_LOCATION_OF_LIBELFHEADER "<libelf/libelf.h>")
endif()

option(libelf "Use libelf (default is YES)" TRUE)
set(DWARF_WITH_LIBELF ${libelf} )
message(STATUS "Building using libelf... ${DWARF_WITH_LIBELF}")
if (DWARF_WITH_LIBELF AND 
    NOT HAVE_LIBELF_H AND 
    NOT HAVE_LIBELF_LIBELF_H)
    set(DWARF_WITH_LIBELF OFF)
endif ()

# libdwarf default-disabled shared
option(shared "build shared library libdwarf.so and use it if present" FALSE)

option(nonshared "build archive library libdwarf.a" TRUE)

#  This adds compiler option -Wall (gcc compiler warnings)
#  'set' here uses $< introducing what is called 
#  a 'generator expression' in cmake documentation.
option(wall "Add -Wall" FALSE)
set(dwfwall $<$<BOOL:${wall}>:"-Wall -O0 -Wpointer-arith -Wmissing-declarations -Wmissing-prototypes -Wdeclaration-after-statement -Wextra -Wcomment -Wformat -Wpedantic -Wuninitialized -Wno-long-long -Wshadow">)

option(nonstandardprintf "Use a special printf format for 64bit (default is NO)" FALSE)
set(HAVE_NONSTANDARD_PRINTF_64_FORMAT ${nonstandardprintf} )
message(STATUS "Checking enable nonstandardprintf... ${HAVE_NONSTANDARD_PRINTF_64_FORMAT}")


option(dodwarfgen "Build dwarfgen (default is NO)" FALSE)
set(BUILD_DWARFGEN ${dodwarfgen} )
message(STATUS "Building dwarfgen    ... ${BUILD_DWARFGEN}")

option(dodwarfexample "Build dwarfexample (default is NO)" FALSE)
set(BUILD_DWARFEXAMPLE ${dodwarfexample} )
message(STATUS "Building dwarfexample... ${BUILD_DWARFEXAMPLE}")

option(test "Do certain api tests (default is NO)" FALSE)
set(DO_TESTING ${test} )
message(STATUS "Building api tests   ... ${DOTESTS}")
