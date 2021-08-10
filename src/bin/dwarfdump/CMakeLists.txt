
set_source_group(SOURCES "Source Files" addrmap.c 
    checkutil.c dwarfdump.c dwconf.c helpertree.c 
	glflags.c command_options.c compiler_info.c
    dwarf_names.c
	macrocheck.c 
    opscounttab.c
    print_abbrevs.c print_aranges.c
    attr_form.c
    print_debugfission.c print_die.c 
    print_debug_gnu.c
	print_debug_names.c  print_debug_sup.c
    print_frames.c  print_gdbindex.c
    print_hipc_lopc_attr.c
	print_lines.c 
    print_llex_codes.c print_origloclist_codes.c 
    print_loclists_codes.c
    print_loclists.c
    print_macro.c print_macinfo.c 
    print_pubnames.c print_ranges.c 
    print_rnglists.c
	print_str_offsets.c
	print_sections.c  print_section_groups.c print_static_funcs.c
    print_static_vars.c print_strings.c 
    print_tag_attributes_usage.c print_types.c 
    print_weaknames.c  
	sanitized.c section_bitmaps.c strstrnocase.c 
    true_section_name.c uri.c dwgetopt.c makename.c 
    naming.c common.c esb.c dwarf_tsearchbal.c)
	
set_source_group(HEADERS "Header Files" 
  addrmap.h attr_form.h checkutil.h common.h dwconf.h
  command_options.h compiler_info.h
  opscounttab.h 
  print_debug_gnu.h
  dwconf_using_functions.h esb_using_functions.h
  dwarfdump-af-table.h
  dwarfdump-ta-ext-table.h dwarfdump-ta-table.h 
  dwarfdump-tt-ext-table.h dwarfdump-tt-table.h
  dwgetopt.h esb.h glflags.h globals.h macrocheck.h defined_types.h
  makename.h dwarf_tsearch.h print_frames.h 
  section_bitmaps.h uri.h
  ../../lib/libdwarf/libdwarf_private.h)

set_source_group(CONFIGURATION_FILES "Configuration Files"
  ${CMAKE_SOURCE_DIR}/cmake/config.h.cmake
  ${CMAKE_BINARY_DIR}/config.h)
	
add_executable(dwarfdump ${SOURCES} ${HEADERS} ${CONFIGURATION_FILES})

set_folder(dwarfdump src/bin/dwarfdump)


target_compile_definitions(dwarfdump PRIVATE 
    "CONFPREFIX=${CMAKE_INSTALL_PREFIX}/lib")
target_compile_options(dwarfdump PRIVATE ${DW_FWALL})

target_link_libraries(dwarfdump PRIVATE ${dwarf-target} ${DW_FZLIB}) 

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
