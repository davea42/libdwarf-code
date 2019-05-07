cmake_minimum_required(VERSION 3.2)

project(libdwarf C)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# used to compile on MSVC upto 2013 where snprintf is not available
macro(msvc_posix target)
    if(MSVC AND ("${MSVC_VERSION}" LESS 1900))
        # under VS 2015
	target_compile_definitions(${target} PUBLIC 
	    snprintf=_snprintf)
    endif()
endmacro()

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

# used when finding libelf
set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)

enable_testing()

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
            message(STATUS "Not building dwarfgen because libelf is not available" )
        endif ()
endif()
if ( BUILD_DWARFEXAMPLE )
	add_subdirectory(dwarfexample)
endif()

message(STATUS "Install prefix ... ${CMAKE_INSTALL_PREFIX}" )
add_custom_target(dd DEPENDS ${DWARF_TARGETS} dwarfdump)
