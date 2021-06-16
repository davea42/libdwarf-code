set_source_group(SOURCES "Source Files" 
pro_alloc.c pro_arange.c 
pro_debug_sup.c
pro_die.c pro_dnames.c 
pro_error.c pro_expr.c 
pro_finish.c pro_forms.c pro_funcs.c pro_frame.c 
pro_init.c pro_line.c pro_reloc.c 
../../lib/libdwarf/dwarf_encode_nm.c
pro_reloc_symbolic.c pro_pubnames.c pro_section.c 
pro_types.c pro_vars.c pro_macinfo.c pro_weaks.c)

set_source_group(HEADERS "Header Files"
libdwarfp.h
pro_alloc.h pro_arange.h pro_die.h 
../../lib/libdwarf/dwarf_encode_nm.h
pro_error.h pro_expr.h pro_frame.h 
pro_incl.h pro_line.h 
pro_log_extra_flag_strings.c
pro_macinfo.h pro_opaque.h 
pro_reloc.h pro_reloc_stream.h pro_reloc_symbolic.h 
pro_section.h pro_types.h pro_util.h)

set_source_group(CONFIGURATION_FILES "Configuration Files" 
    ${CMAKE_SOURCE_DIR}/cmake/config.h.cmake 
    ${CMAKE_BINARY_DIR}/config.h)

	
list(LENGTH DWARFP_TARGETS targetCount)
message(" dadebug targetCount $(targetCount)" )
message(" dadebug cmake sourcedir ${CMAKE_SOURCE_DIR}")
math(EXPR targetCount "${targetCount} - 1")
list(APPEND DWARFP_LIBS ${LIBELF_LIBRARIES})
foreach(i RANGE ${targetCount})
	list(GET DWARFP_TARGETS ${i} target)
	list(GET DWARFP_TYPES ${i} type)
        message(" dadebug p range ${i} ${target} ${type}" )
        message(" dadebug targ ${DWARFP_TARGETS}" )
        message(" dadebug src ${CMAKE_CURRENT_SOURCE_DIR}" )
        message(" dadebug inc ${LIBDWARF_INCLUDE_DIR}" )
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
