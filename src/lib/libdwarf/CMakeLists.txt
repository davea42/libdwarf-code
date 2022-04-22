set_source_group(SOURCES "Source Files" dwarf_abbrev.c 
dwarf_alloc.c dwarf_crc.c dwarf_crc32.c dwarf_arange.c 
dwarf_debug_sup.c
dwarf_debuglink.c dwarf_die_deliv.c 
dwarf_debugnames.c dwarf_dsc.c
dwarf_elf_load_headers.c 
dwarf_elfread.c 
dwarf_elf_rel_detector.c 
dwarf_error.c 
dwarf_find_sigref.c dwarf_fission_to_cu.c
dwarf_form.c dwarf_form_class_names.c
dwarf_frame.c dwarf_frame2.c dwarf_funcs.c 
dwarf_gdbindex.c dwarf_global.c 
dwarf_gnu_index.c dwarf_groups.c 
dwarf_harmless.c dwarf_generic_init.c dwarf_init_finish.c 
dwarf_leb.c 
dwarf_line.c dwarf_loc.c 
dwarf_loclists.c
dwarf_locationop_read.c
dwarf_machoread.c dwarf_macro.c dwarf_macro5.c
dwarf_memcpy_swap.c
dwarf_names.c
dwarf_object_read_common.c dwarf_object_detector.c
dwarf_peread.c 
dwarf_pubtypes.c dwarf_query.c dwarf_ranges.c 
dwarf_rnglists.c
dwarf_safe_strcpy.c
dwarf_string.h dwarf_string.c
dwarf_stringsection.c
dwarf_tied.c 
dwarf_str_offsets.c
dwarf_tsearchhash.c dwarf_types.c dwarf_util.c 
dwarf_vars.c dwarf_weaks.c dwarf_xu_index.c
dwarf_print_lines.c )

set_source_group(HEADERS "Header Files" dwarf.h dwarf_abbrev.h
dwarf_alloc.h dwarf_arange.h dwarf_base_types.h 
dwarf_debuglink.h dwarf_die_deliv.h 
dwarf_debugnames.h dwarf_dsc.h 
dwarf_elf_access.h dwarf_elf_defines.h dwarf_elfread.h 
dwarf_elf_rel_detector.h 
dwarf_elfstructs.h 
dwarf_error.h dwarf_frame.h 
dwarf_funcs.h dwarf_gdbindex.h dwarf_global.h dwarf_harmless.h 
dwarf_gnu_index.h 
dwarf_line.h dwarf_loc.h 
dwarf_machoread.h dwarf_macro.h dwarf_macro5.h 
dwarf_object_detector.h dwarf_opaque.h 
dwarf_pe_descr.h dwarf_peread.h
dwarf_reading.h
dwarf_rnglists.h
dwarf_safe_strcpy.h
dwarf_tied_decls.h 
dwarf_tsearch.h 
dwarf_str_offsets.h
dwarf_types.h dwarf_util.h dwarf_vars.h dwarf_weaks.h 
dwarf_xu_index.h libdwarf_private.h
dwarf_macho_loader.h
dwarf_memcpy_swap.h)

set_source_group(CONFIGURATION_FILES "Configuration Files" 
    ${CMAKE_SOURCE_DIR}/cmake/config.h.cmake 
    ${CMAKE_BINARY_DIR}/config.h)
	
list(LENGTH DWARF_TARGETS targetCount)
math(EXPR targetCount "${targetCount} - 1")
list(APPEND DWARF_LIBS ${LIBELF_LIBRARIES})
if (DW_FZLIB)
    list(APPEND DWARF_LIBS z)
endif()
foreach(i RANGE ${targetCount})
	list(GET DWARF_TARGETS ${i} target)
	list(GET DWARF_TYPES ${i} type)
	add_library(${target} ${type} ${SOURCES} ${HEADERS}
            ${GENNAMES_OUTPUT} ${CONFIGURATION_FILES})

	set_folder(${target} src/lib/libdwarf)
	target_include_directories(${target} PUBLIC 
            ${LIBELF_INCLUDE_DIRS})
    target_compile_options(${target} PRIVATE -DLIBDWARF_BUILD
            ${DW_FWALL})
	msvc_posix(${target})

	target_link_libraries(${target} PUBLIC ${LIBELF_LIBRARIES} ${DW_FZLIB})
	
	set_target_properties(${target} PROPERTIES OUTPUT_NAME dwarf)

	set(SUFFIX $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:64>)
	set(LIBDIR lib${SUFFIX})
	set(BINDIR bin${SUFFIX})

	install(TARGETS ${target}
		RUNTIME DESTINATION ${BINDIR}		
		LIBRARY DESTINATION ${LIBDIR}
		ARCHIVE DESTINATION ${LIBDIR})
endforeach()
# The install has to be here, not in
# another CMakeLists.txt  to make install work properly
# for cmake before cmake 3.13. This also works
# for newer cmake.
add_custom_target(dd DEPENDS ${DWARF_TARGETS} dwarfdump)
install(TARGETS ${DWARF_TARGETS} EXPORT libdwarfTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
install(EXPORT libdwarfTargets
    FILE libdwarf-targets.cmake
    NAMESPACE libdwarf::
    DESTINATION lib/cmake/libdwarf
)
install(
    FILES cmake/libdwarf-config.cmake
    DESTINATION lib/cmake/libdwarf
)
install(DIRECTORY libdwarf DESTINATION include/
    FILES_MATCHING PATTERN "*.h")
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/libdwarf.h
    DESTINATION include/libdwarf)
