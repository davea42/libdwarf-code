# cmake macros
include(TestBigEndian)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckSymbolExists)

set(PACKAGE_NAME "libdwarf" )
set(PACKAGE_VERSION "20190505"  )
set(VERSION ${PACKAGE_VERSION} )
set(PACKAGE_STRING "\"${PACKAGE_NAME} ${VERSION}\"")
string(REGEX REPLACE "[\"]" "" tarname1 "${PACKAGE_STRING}" )
string(REGEX REPLACE "[^a-zA-Z0-9_]" "-" tarname "${tarname1}" )
set(PACKAGE_TARNAME "\"${tarname}\"" )

test_big_endian(isBigEndian)
if (isBigEndian)
  set ( WORDS_BIGENDIAN 1 )
endif()

check_include_file( "sys/types.h"     HAVE_SYS_TYPES_H) 
check_include_file( "sys/stat.h"      HAVE_SYS_STAT_H ) 
check_include_file( "stdlib.h"        HAVE_STDLIB_H   ) 
check_include_file( "string.h"        HAVE_STRING_H   ) 
check_include_file( "memory.h"        HAVE_MEMORY_H   ) 
check_include_file( "strings.h"       HAVE_STRINGS_H  ) 
check_include_file( "inttypes.h"      HAVE_INTTYPES_H ) 
check_include_file( "stdint.h"        HAVE_STDINT_T   )
check_include_file( "unistd.h"        HAVE_UNISTD_H   )
check_include_file( "sgidefs.h"       HAVE_SGIDEFS_H  )
check_include_file( "stdafx.h"        HAVE_STDAFX_H   )
check_include_file( "Windows.h"       HAVE_WINDOWS_H  )
check_include_file( "elf.h"           HAVE_ELF_H      ) 
check_include_file( "libelf.h"        HAVE_LIBELF_H   ) 
check_include_file( "libelf/libelf.h" HAVE_LIBELF_LIBELF_H) 
check_include_file( "alloca.h"        HAVE_ALLOCA_H   )
check_include_file( "elfaccess.h"     HAVE_ELFACCESS_H)
check_include_file( "sys/elf_386.h"   HAVE_ELF_386_H  )
check_include_file( "sys/elf_amd64.h" HAVE_SYS_ELF_AMD64_H)
check_include_file( "sys/elf_sparc.h" HAVE_SYS_ELF_SPARC_H)
check_include_file( "sys/ia64/elf.h"  HAVE_SYS_IA64_ELF_H)


#  It's not really setting the location of libelfheader, 
#  it is really #  either elf.h, or if that is missing 
#  it is assuming  elf.h data is in the supplied libelf.
if(HAVE_ELF_H)
    set(HAVE_LOCATION_OF_LIBELFHEADER "<elf.h>")
elseif(HAVE_LIBELF_H)
    set(HAVE_LOCATION_OF_LIBELFHEADER "<libelf.h>")
elseif(HAVE_LIBELF_LIBELF_H)
    set(HAVE_LOCATION_OF_LIBELFHEADER "<libelf/libelf.h>")
endif()

if(HAVE_LIBELF_H)
    set(JUST_LIBELF "<libelf.h>")
elseif(HAVE_LIBELF_LIBELF_H)
    set(JUST_LIBELF "<libelf/libelf.h>")
endif()

if (HAVE_LIBELF_H OR HAVE_LIBELF_LIBELF_H)
  set (CMAKE_REQUIRED_LIBRARIES elf)
  check_symbol_exists( elf64_getehdr ${JUST_LIBELF} HAVE_ELF64_GETEHDR)
  check_symbol_exists( elf64_getshdr ${JUST_LIBELF} HAVE_ELF64_GETSHDR)
  set (CMAKE_REQUIRED_LIBRARIES)
endif()

option(libelf "Use libelf (default is YES)" TRUE)
set(DWARF_WITH_LIBELF ${libelf} )
message(STATUS "Building using libelf... ${DWARF_WITH_LIBELF}")
if (DWARF_WITH_LIBELF AND 
    NOT HAVE_LIBELF_H AND 
    NOT HAVE_LIBELF_LIBELF_H)
    set(DWARF_WITH_LIBELF OFF)
endif ()

if (DWARF_WITH_LIBELF)
  message(STATUS "checking using HAVE_ELF_H ... ${HAVE_ELF_H}")
  message(STATUS "checking using elf header ... ${HAVE_LOCATION_OF_LIBELFHEADER}")
  message(STATUS "checking using libelf header ... ${JUST_LIBELF}")
  check_c_source_compiles("
  #include ${HAVE_LOCATION_OF_LIBELFHEADER}
  int main()      
  {
      Elf64_Rel *p; int i; i = p->r_info;
      return 0;
  }" HAVE_ELF64_R_INFO)

  check_c_source_compiles("
  #include  ${HAVE_LOCATION_OF_LIBELFHEADER}
  int main()
  {
      Elf64_Rela p; p.r_offset = 1;
      return 0;
  }" HAVE_ELF64_RELA)
    
  check_c_source_compiles("
  #include ${HAVE_LOCATION_OF_LIBELFHEADER}
  int main()
  {
      Elf64_Sym p; p.st_info = 1;
      return 0;
  }" HAVE_ELF64_SYM)
  check_c_source_compiles("
  #include ${HAVE_LOCATION_OF_LIBELFHEADER}
  int main()
  {
      int p; p = 0;
      return 0;
  }" HAVE_RAW_LIBELF_OK)

  # This is attempting do determine that with GNU_SOURCE
  # we have off64_t. The autoconf version is not attemping
  # to set HAVE_LIBELF_OFF64_OK at present. 
  check_c_source_compiles("
  #define _GNU_SOURCE 1
  #include ${JUST_LIBELF}
  int main()
  { 
      off64_t  p; p = 0;
      return 0;
  }"  HAVE_LIBELF_OFF64_OK)

  check_c_source_compiles("
  #include ${JUST_LIBELF}
  /* This must be at global scope */
  struct _Elf;
  typedef struct _Elf Elf;
  struct _Elf *a = 0;
  int main()
  {
   int i = 12;
   return 0;
  }" HAVE_STRUCT_UNDERSCORE_ELF)
endif()
message(STATUS "Assuming struct Elf for the default libdwarf.h")
# Because cmake treats ; in an interesting way attempting
# to read/update/write Elf to _Elf fails badly: semicolons vanish.
# So for _Elf use a pre-prepared version.
if(HAVE_STRUCT_UNDERSCORE_ELF AND DWARF_WITH_LIBELF)
   message(STATUS "Found struct _Elf in ${JUST_LIBELF}")
   message(STATUS "Using struct _Elf in libdwarf.h")
   configure_file(libdwarf/generated_libdwarf.h.in 
       libdwarf/libdwarf.h COPYONLY)
else()
   configure_file(libdwarf/libdwarf.h.in 
       libdwarf/libdwarf.h COPYONLY)
   message(STATUS "${JUST_LIBELF} does not have struct _Elf")
   message(STATUS "Using struct Elf in libdwarf.h")
endif()

check_c_source_runs("
  static unsigned foo( unsigned x, 
      __attribute__ ((unused)) int y)
  {  
      unsigned x2 = x + 1;
      return x2;
  } 
  
  int main(void) {
      unsigned y = 0;
      y = foo(12,y);
      return 0;
  }"    HAVE_UNUSED_ATTRIBUTE)
message(STATUS "Checking compiler supports __attribute__ unused... ${HAVE_UNUSED_ATTRIBUTE}")

#  checking for ia 64 types, which might be enums, 
#  using HAVE_R_IA_64_DIR32LSB
#  to stand in for a small set.
check_c_source_compiles("
  #include ${HAVE_LOCATION_OF_LIBELFHEADER}
  int main()
  {  
      int p; p = R_IA_64_DIR32LSB;
      return 0;
  }" HAVE_R_IA_64_DIR32LSB)

check_c_source_compiles([=[
  #include "stdafx.h"
  int main() 
  { 
      int p; p = 27;
      return 0;
  }]=] HAVE_STDAFX_H)
#message(STATUS "Checking have windows stdafx.h... ${HAVE_STDAFX_H}")

check_c_source_compiles([=[
  #include <sys/types.h>
  #include <regex.h> 
  int main()
  {
      int i; 
      regex_t r;
      int cflags = REG_EXTENDED;
      const char *s = "abc";
      i = regcomp(&r,s,cflags);
      regfree(&r);
      return 0;
  } ]=]  HAVE_REGEX)

set(CMAKE_REQUIRED_LIBRARIES z)
check_c_source_compiles( [=[
  #include "zlib.h"
  int main()
  {
      Bytef dest[100];
      uLongf destlen = 100;
      Bytef *src = 0;
      uLong srclen = 3;
      int res = uncompress(dest,&destlen,src,srclen);
      if (res == Z_OK) {
           /* ALL IS WELL */
      }
      return 0;
  } ]=]  HAVE_ZLIB )
#message(STATUS "dadebug Checking zlib.h usability... ${HAVE_ZLIB}")
set(CMAKE_REQUIRED_LIBRARIES)
if (HAVE_ZLIB) 
  # For linking in libz
  set(dwfzlib "z")
endif()

check_c_source_compiles([=[
#include <stdint.h>
int main()
{
    intptr_t p; 
    p = 27;
    return 0;
}]=] HAVE_INTPTR_T)




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
