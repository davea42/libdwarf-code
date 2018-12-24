include(configure.cmake)

add_library(esb_and_tsearchbal OBJECT esb.c dwarf_tsearchbal.c)

set_folder(esb_and_tsearchbal dwarfdump)

target_include_directories(esb_and_tsearchbal PRIVATE
    $<TARGET_PROPERTY:${dwarf-target},INTERFACE_INCLUDE_DIRECTORIES>)

msvc_posix(esb_and_tsearchbal)

set_source_group(SOURCES "Source Files" addrmap.c checkutil.c dwarfdump.c dwconf.c helpertree.c 
	glflags.c command_options.c compiler_info.c
        dwarf_names.c
	macrocheck.c print_abbrevs.c print_aranges.c print_debugfission.c print_die.c 
	print_dnames.c print_frames.c  print_gdbindex.c
	print_lines.c print_locs.c print_macro.c print_macros.c print_pubnames.c print_ranges.c print_reloc.c 
	print_str_offsets.c
	print_sections.c  print_section_groups.c print_static_funcs.c  print_static_vars.c print_strings.c print_types.c print_weaknames.c  
	sanitized.c section_bitmaps.c strstrnocase.c true_section_name.c uri.c dwgetopt.c makename.c naming.c common.c $<TARGET_OBJECTS:esb_and_tsearchbal>)
	
set_source_group(HEADERS "Header Files" checkutil.h common.h dwconf.h
  command_options.h compiler_info.h
  dwarf_names.h
  dwconf_using_functions.h esb_using_functions.h
  dwarfdump-ta-ext-table.h dwarfdump-ta-table.h dwarfdump-tt-ext-table.h dwarfdump-tt-table.h
  dwgetopt.h esb.h glflags.h globals.h macrocheck.h defined_types.h
  makename.h dwarf_tsearch.h print_frames.h section_bitmaps.h uri.h)

set_source_group(CONFIGURATION_FILES "Configuration Files" configure.cmake config.h.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
	
add_executable(dwarfdump ${SOURCES} ${HEADERS} ${CONFIGURATION_FILES})

set_folder(dwarfdump dwarfdump)

target_compile_options(dwarfdump PRIVATE ${dwfwall})

target_compile_definitions(dwarfdump PRIVATE "CONFPREFIX=${CMAKE_INSTALL_PREFIX}/lib")

target_link_libraries(dwarfdump PRIVATE ${dwarf-target} ${dwfzlib}) 

# Plain GNU C dash E does not work on a .list, 
# so copy to a .c name to run
# the following four table creations.
	
set_source_group(TESTESB_SOURCES "Source Files" esb.c testesb.c)

add_executable(testesb ${TESTESB_SOURCES})

set_folder(testesb dwarfdump/tests)

target_include_directories(testesb PRIVATE
    $<TARGET_PROPERTY:${dwarf-target},INTERFACE_INCLUDE_DIRECTORIES>)

add_test(NAME test COMMAND testesb)

set_source_group(GETOPTEST_SOURCES "Source Files" getopttest.c dwgetopt.c)

add_executable(getopttest ${GETOPTEST_SOURCES})

set_folder(getopttest dwarfdump/tests)

add_test(NAME getopttest COMMAND getopttest)

add_executable(getopttestnat ${GETOPTEST_SOURCES})

set_folder(getopttestnat dwarfdump/tests)

if(UNIX)
	target_compile_definitions(getopttestnat PRIVATE GETOPT_FROM_SYSTEM)
endif()

foreach(i 1 2 3 5 6 7 8)
	add_test(NAME getopttestnat${i} COMMAND getopttestnat -c ${i})
endforeach()
	
set_source_group(SELFHELPERTREE_SOURCES "Source Files" helpertree.c)

add_executable(selfhelpertree ${SELFHELPERTREE_SOURCES} $<TARGET_OBJECTS:esb_and_tsearchbal>)

set_folder(selfhelpertree dwarfdump/tests)

target_compile_definitions(selfhelpertree PRIVATE SELFTEST)

target_include_directories(selfhelpertree PRIVATE
    $<TARGET_PROPERTY:${dwarf-target},INTERFACE_INCLUDE_DIRECTORIES>)

add_test(NAME selfhelpertree COMMAND selfhelpertree)

#doesn't compile	
#set_source_group(SELFMC_SOURCES "Source Files" macrocheck.c $<TARGET_OBJECTS:esb_and_tsearchbal>)
#
#add_executable(selfmc ${SELFMC_SOURCES})
#
# set_folder(selfmc dwarfdump/tests)
#
#target_compile_definitions(selfmc PRIVATE SELFTEST)
#
#target_include_directories(selfmc PRIVATE
#    $<TARGET_PROPERTY:${dwarf-target},INTERFACE_INCLUDE_DIRECTORIES>)
#
#add_test(NAME selfmc COMMAND selfmc)
	
#set_source_group(SELFESB_SOURCES "Source Files" esb.c)
#
#add_executable(selfesb ${SELFESB_SOURCES})
#
#set_folder(selfesb dwarfdump/tests)
#
#target_compile_definitions(selfesb PRIVATE SELFTEST)

#target_include_directories(selfesb PRIVATE
#    $<TARGET_PROPERTY:${dwarf-target},INTERFACE_INCLUDE_DIRECTORIES>)

#add_test(NAME selfesb COMMAND selfesb)

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(SUFFIX 64)
endif()
set(LIBDIR lib${SUFFIX})
set(BINDIR bin${SUFFIX})

install(TARGETS dwarfdump DESTINATION
	RUNTIME DESTINATION ${BINDIR}		
	LIBRARY DESTINATION ${LIBDIR}
	ARCHIVE DESTINATION ${LIBDIR})

install(FILES dwarfdump.conf DESTINATION ${LIBDIR})

install(FILES dwarfdump.1 DESTINATION share/man/man1)
