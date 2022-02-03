set_source_group(SOURCES "Source Files" 
dwarf_pro_alloc.c dwarf_pro_arange.c 
dwarf_pro_debug_sup.c
dwarf_pro_die.c dwarf_pro_dnames.c 
dwarf_pro_error.c dwarf_pro_expr.c 
dwarf_pro_finish.c dwarf_pro_forms.c
dwarf_pro_funcs.c dwarf_pro_frame.c 
dwarf_pro_init.c dwarf_pro_line.c dwarf_pro_reloc.c 
dwarf_pro_reloc_stream.c 
dwarf_pro_reloc_symbolic.c dwarf_pro_pubnames.c 
dwarf_pro_section.c 
dwarf_pro_types.c dwarf_pro_vars.c dwarf_pro_macinfo.c
dwarf_pro_weaks.c)

set_source_group(HEADERS "Header Files"
libdwarfp.h
dwarf_pro_alloc.h dwarf_pro_arange.h dwarf_pro_die.h 
dwarf_pro_error.h dwarf_pro_expr.h dwarf_pro_frame.h 
dwarf_pro_incl.h dwarf_pro_line.h 
dwarf_pro_log_extra_flag_strings.c
dwarf_pro_macinfo.h dwarf_pro_opaque.h 
dwarf_pro_reloc.h
dwarf_pro_reloc_stream.h
dwarf_pro_reloc_symbolic.h 
dwarf_pro_section.h dwarf_pro_types.h dwarf_pro_util.h)

set_source_group(CONFIGURATION_FILES "Configuration Files" 
    ${CMAKE_SOURCE_DIR}/cmake/config.h.cmake 
    ${CMAKE_BINARY_DIR}/config.h)

	
list(LENGTH DWARFP_TARGETS targetCount)
math(EXPR targetCount "${targetCount} - 1")
list(APPEND DWARFP_LIBS ${LIBELF_LIBRARIES})
foreach(i RANGE ${targetCount})
	list(GET DWARFP_TARGETS ${i} target)
	list(GET DWARFP_TYPES ${i} type)
	add_library(${target} ${type} ${SOURCES} ${HEADERS}
            ${GENNAMES_OUTPUT} ${CONFIGURATION_FILES})

	set_folder(${target} src/lib/libdwarfp)
	target_include_directories(${target} PUBLIC 
            ${LIBELF_INCLUDE_DIRS} )
	target_include_directories(${target} PRIVATE 
            ${LIBDWARF_INCLUDE_DIR} )
        target_compile_options(${target} PRIVATE ${DW_FWALL})
	msvc_posix(${target})

	target_link_libraries(${target} PUBLIC ${LIBELF_LIBRARIES})
	
	set_target_properties(${target} PROPERTIES OUTPUT_NAME dwarfp)

	set(SUFFIX $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:64>)
	set(LIBDIR lib${SUFFIX})
	set(BINDIR bin${SUFFIX})

	install(TARGETS ${target}
		RUNTIME DESTINATION ${BINDIR}		
		LIBRARY DESTINATION ${LIBDIR}
		ARCHIVE DESTINATION ${LIBDIR})
endforeach()
# The install has to be here, not in
# ../CMakeLists.txt  to make install work properly
# for cmake before cmake 3.13. This also works
# for newer cmake.
add_custom_target(dg DEPENDS ${DWARFP_TARGETS} dwarfgen)
install(TARGETS ${DWARFP_TARGETS} EXPORT libdwarfpTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
install(EXPORT libdwarfpTargets
    FILE libdwarfp-targets.cmake

    NAMESPACE libdwarfp::
    DESTINATION lib/cmake/libdwarfp
)
install(
    FILES cmake/libdwarfp-config.cmake
    DESTINATION lib/cmake/libdwarfp
)
install(DIRECTORY libdwarfp DESTINATION include/
    FILES_MATCHING PATTERN "*.h")
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/libdwarfp.h
    DESTINATION include/libdwarfp)
