cmake_minimum_required(VERSION 3.2)

project(libdwarf)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(LibdwarfMacros)

# view folders on supported IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# used when finding libelf
set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)

# this project has tests, but not essential here.
# enable_testing()

# always include project's folder to includes
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

include(configure.cmake)
find_package(LibElf REQUIRED)

list(APPEND CMAKE_REQUIRED_INCLUDES ${LIBELF_INCLUDE_DIRS})

include(configure.cmake)

if(nonshared)
	set(DWARF_TARGETS dwarf-static)
	set(DWARF_TYPES STATIC)
	set(dwarf-target dwarf-static)
endif()
if(shared)
	list(APPEND DWARF_TARGETS dwarf-shared)
	list(APPEND DWARF_TYPES SHARED)
	set(dwarf-target dwarf-shared)
endif()

add_subdirectory(libdwarf)
add_subdirectory(dwarfdump)
if ( BUILD_DWARFGEN )
	if ( DWARF_WITH_LIBELF )
	    add_subdirectory(dwarfgen)
        else ()
            message( "-- Not building dwarfgen because libelf is not available" )
        endif ()
endif()
if ( BUILD_DWARFEXAMPLE )
	add_subdirectory(dwarfexample)
endif()

add_custom_target(dd
	DEPENDS ${DWARF_TARGETS} dwarfdump)
