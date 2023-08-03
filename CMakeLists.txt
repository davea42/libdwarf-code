cmake_minimum_required(VERSION 3.3)

project(libdwarf VERSION "0.7.1" DESCRIPTION "Library for reading DWARF2 and later DWARF" HOMEPAGE_URL "https://github.com/davea42/libdwarf-code" LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)

# used to compile on MSVC upto 2013 where snprintf is not available
macro(msvc_posix target)
    if (MSVC AND ("${MSVC_VERSION}" LESS 1900))
        # under VS 2015
        target_compile_definitions(${target} PUBLIC
                snprintf=_snprintf)
    endif ()
endmacro()

set(LIBDWARF_CRT "MD" CACHE STRING "Either MT or MD, specifies whether to use the static or dynamic MSVCRT.")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Use CMAKE_MSVC_RUNTIME in versions 3.15 and up
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
        cmake_policy(SET CMP0091 NEW)
        if (LIBDWARF_CRT STREQUAL "MT")
            message(STATUS "Using MT runtime by CMAKE_MSVC_RUNTIME_LIBRARY")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        elseif (LIBDWARF_CRT STREQUAL "MD")
            message(STATUS "Using MD runtime by CMAKE_MSVC_RUNTIME_LIBRARY")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else ()
            message(FATAL_ERROR "LIBDWARF_CRT must be MT or MD")
        endif ()
        # Use regexes in versions before 3.15
    else ()
        if (LIBDWARF_CRT STREQUAL "MT")
            message(STATUS "Using MT runtime by compile flag replacement")
            # taken from the CMake FAQ
            foreach (flag_v
                    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG
                    CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL
                    CMAKE_C_FLAGS_RELWITHDEBINFO
                    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG
                    CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL
                    CMAKE_CXX_FLAGS_RELWITHDEBINFO)
                if (${flag_v} MATCHES "([\\/\\-]M)D")
                    string(REGEX REPLACE "([\\/\\-]M)D" "\\1T"
                            ${flag_v} "${${flag_v}}")
                endif ()
            endforeach ()
        elseif (LIBDWARF_CRT STREQUAL "MD")
            message(STATUS "Using MD runtime by cmake default")
        else ()
            message(FATAL_ERROR "LIBDWARF_CRT must be MT or MD")
        endif ()
    endif ()
endif ()

# message("CMake flags are:")
# message("  CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
# message("  CMAKE_C_FLAGS_DEBUG: ${CMAKE_C_FLAGS_DEBUG}")
# message("  CMAKE_C_FLAGS_RELEASE: ${CMAKE_C_FLAGS_RELEASE}")
# message("  CMAKE_C_FLAGS_MINSIZEREL: ${CMAKE_C_FLAGS_MINSIZEREL}")
# message("  CMAKE_C_FLAGS_RELWITHDEBINFO: ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
# message("  CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
# message("  CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
# message("  CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
# message("  CMAKE_CXX_FLAGS_MINSIZEREL: ${CMAKE_CXX_FLAGS_MINSIZEREL}")
# message("  CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")


# set target folder on IDE
macro(set_folder target folder)
    set_target_properties(${target} PROPERTIES FOLDER ${folder})
endmacro()

# set source groups on IDE
macro(set_source_group list_name group_name)
    set(${list_name} ${ARGN})
    source_group(${group_name} FILES ${ARGN})
endmacro()

# view folders on supported IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# tells cmake to look in 64bit first (cmake
# would probably do this anyway).
set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)

### begin what was configure.cmake
# cmake macros
include(TestBigEndian)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckSymbolExists)
### Version also appears in configure.ac
set(PACKAGE_VERSION "\"${PROJECT_VERSION}\"")
set(PACKAGE_NAME "${PROJECT_NAME}")
set(PACKAGE_STRING "\"${PACKAGE_NAME} ${VERSION}\"")
string(REGEX REPLACE "[\"]" "" tarname1 "${PACKAGE_STRING}")
string(REGEX REPLACE "[^a-zA-Z0-9_]" "-" tarname "${tarname1}")
set(PACKAGE_TARNAME "\"${tarname}\"")

test_big_endian(isBigEndian)
if (${isBigEndian})
    set(WORDS_BIGENDIAN 1)
endif ()

check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file("sys/stat.h" HAVE_SYS_STAT_H)
check_include_file("inttypes.h" HAVE_INTTYPES_H)
check_include_file("memory.h" HAVE_MEMORY_H)
check_include_file("strings.h" HAVE_STRINGS_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_include_file("sgidefs.h" HAVE_SGIDEFS_H)
check_include_file("stdafx.h" HAVE_STDAFX_H)
check_include_file("fcntl.h" HAVE_FCNTL_H)

### cmake provides no way to guarantee uint32_t present.
### configure does guarantee that.
if (HAVE_STDINT_H)
    check_c_source_compiles("
  #include <stdint.h>
  int main()
  {
      uintptr_t i; i = 12;
      return (int)i;
  }" HAVE_UINTPTR_T)
    check_c_source_compiles("
  #include <stdint.h>
  int main()
  {
      intptr_t i; i = 12;
      return (int)i;
  }" HAVE_INTPTR_T)
endif ()
if (HAVE_UINTPTR_T)
    message(STATUS "HAVE_UINTPTR_T 1: uintptr_t defined in stdint.h... YES")
else ()
    message(STATUS "uintptr_t defined in stdint.h... NO")
    set(uintptr_t "unsigned long long int")
    message(STATUS "Setting #define uintptr_t " ${uintptr_t})
endif ()
if (uintptr_t)
    message(STATUS "uintptr_t value considered YES ")
else ()
    message(STATUS "uintptr_t value considered NO ")
endif ()
if (HAVE_INTPTR_T)
    message(STATUS "HAVE_INTPTR_T 1: intptr_t defined in stdint.h... YES")
else ()
    message(STATUS "intptr_t defined in stdint.h... NO")
    set(uintptr_t "long long int")
    message(STATUS "Setting #define intptr_t " ${intptr_t})
endif ()
if (intptr_t)
    message(STATUS "intptr_t value considered YES ")
else ()
    message(STATUS "intptr_t value considered NO ")
endif ()


check_c_source_compiles("
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
  }" HAVE_UNUSED_ATTRIBUTE)
message(STATUS "Checking compiler supports __attribute__ unused... ${HAVE_UNUSED_ATTRIBUTE}")

check_c_source_compiles([=[
  #include "stdafx.h"
  int main() 
  { 
      int p; p = 27;
      return 0;
  }]=] HAVE_STDAFX_H)
#message(STATUS "Checking have windows stdafx.h... ${HAVE_STDAFX_H}")

find_package(ZLIB)
find_package(ZSTD)

check_c_source_compiles([=[
#include <stdint.h>
int main()
{
    intptr_t p; 
    p = 27;
    return 0;
}]=] HAVE_INTPTR_T)

message(STATUS "CMAKE_SIZEOF_VOID_P ... " ${CMAKE_SIZEOF_VOID_P})

# libdwarf
option(BUILD_NON_SHARED "build archive library libdwarf[p].a" TRUE)
option(BUILD_SHARED "build shared library libdwarf[p].so and use it" FALSE)
if (BUILD_NON_SHARED)
    set(BUILD_SHARED FALSE)
    set(LIBDWARF_BUILD_TYPE "SHARED")
else ()
    set(BUILD_SHARED TRUE)
    set(LIBDWARF_BUILD_TYPE "STATIC")
endif ()

#  This adds compiler option -Wall (gcc compiler warnings)
option(WALL "Add -Wall" FALSE)

#  DW_FWALLXX are gnu C++ options.
if (WALL)
    set(DW_FWALLXX -Wall -Wextra -Wno-unused-private-field -Wpointer-arith -Wmissing-declarations -Wcomment -Wformat -Wpedantic -Wuninitialized -Wshadow -Wno-long-long -Werror)
    set(DW_FWALL ${DW_FWALLXX} -Wpointer-arith -Wmissing-declarations -Wmissing-prototypes -Wdeclaration-after-statement -Wextra -Wcomment -Wformat -Wpedantic -Wuninitialized -Wno-long-long -Wshadow -Werror)
    message(STATUS "Compiler warning options... YES ${DW_FWALL}")
else ()
    unset(DW_FWALL)
    message(STATUS "Compiler warning options... NO")
endif ()

option(BUILD_DWARFGEN "Build dwarfgen (default is NO)" FALSE)
message(STATUS "Building dwarfgen    ... ${BUILD_DWARFGEN}")

option(BUILD_DWARFEXAMPLE "Build dwarfexample (default is NO)" FALSE)
message(STATUS "Building dwarfexample... ${BUILD_DWARFEXAMPLE}")

option(DO_TESTING "Do certain api tests (default is NO)" FALSE)
message(STATUS "Building api tests   ... ${DOTESTS}")
### end what was configure.cmake

configure_file(cmake/config.h.cmake config.h)

add_subdirectory(src/lib/libdwarf)
add_subdirectory(src/bin/dwarfdump)

if (BUILD_DWARFEXAMPLE)
    add_subdirectory(src/bin/dwarfexample)
endif ()
if (DO_TESTING)
    include(CTest)
    add_subdirectory(src/lib/libdwarfp)
    add_subdirectory(test)
endif ()

if (BUILD_DWARFGEN)
    add_subdirectory(src/lib/libdwarfp)
    add_subdirectory(src/bin/dwarfgen)
endif ()

message(STATUS "Install prefix ... ${CMAKE_INSTALL_PREFIX}")
# installation of libdwarf[p] has to be in
# src/lib/libdwarf[p]/CMakeLists.txt
# so that it works properly for cmake before cmake 3.13.
