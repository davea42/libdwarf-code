# Copyright (C) 1995-2019, Rene Brun and Fons Rademakers.
# All rights reserved.
#
# This is licensed as LGPL 2.1 by the authors.
# For the licensing terms see COPYING and
# libdwarf/LIBDWARFCOPYRIGHT
# or https://github.com/root-project/root/blob/master/LICENSE
# For the list of contributors in the project that
# created this cmake file see 
# https://github.com/root-project/root/blob/master/README/CREDITS

#.rst:
# FindZSTD
# -----------
#
# Find the ZSTD library header and define variables.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines :prop_tgt:`IMPORTED` target ``ZSTD::ZSTD``,
# if ZSTD has been found
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables:
#
# ::
#
#   ZSTD_FOUND          - True if ZSTD is found.
#   ZSTD_INCLUDE_DIRS   - Where to find zstd.h
#
# Finds the Zstandard library. This module defines:
#   - ZSTD_INCLUDE_DIR, directory containing headers
#   - ZSTD_LIBRARIES, the Zstandard library path
#   - ZSTD_FOUND, whether Zstandard has been found

# Find header files
find_path(ZSTD_INCLUDE_DIR zstd.h)

# Find a ZSTD version
if (ZSTD_INCLUDE_DIR AND EXISTS "${ZSTD_INCLUDE_DIR}/zstd.h")
    file(READ "${ZSTD_INCLUDE_DIR}/zstd.h" CONTENT)
    string(REGEX MATCH ".*define ZSTD_VERSION_MAJOR *([0-9]+).*define ZSTD_VERSION_MINOR *([0-9]+).*define ZSTD_VERSION_RELEASE *([0-9]+)" VERSION_REGEX "${CONTENT}")
    set(ZSTD_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(ZSTD_VERSION_MINOR ${CMAKE_MATCH_2})
    set(ZSTD_VERSION_RELEASE ${CMAKE_MATCH_3})
    set(ZSTD_VERSION "${ZSTD_VERSION_MAJOR}.${ZSTD_VERSION_MINOR}.${ZSTD_VERSION_RELEASE}")
endif ()

# Find library
find_library(ZSTD_LIBRARIES NAMES zstd)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZSTD REQUIRED_VARS ZSTD_LIBRARIES ZSTD_INCLUDE_DIR ZSTD_VERSION VERSION_VAR ZSTD_VERSION)

if (ZSTD_FOUND)
    if (NOT TARGET ZSTD::ZSTD)
        add_library(ZSTD::ZSTD UNKNOWN IMPORTED)
        set_target_properties(ZSTD::ZSTD PROPERTIES IMPORTED_LOCATION "${ZSTD_LIBRARIES}" INTERFACE_INCLUDE_DIRECTORIES "${ZSTD_INCLUDE_DIR}")
    endif ()
endif ()
