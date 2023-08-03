set_source_group(SOURCES "Source Files"
        dwarf_abbrev.c
        dwarf_alloc.c
        dwarf_crc.c
        dwarf_crc32.c
        dwarf_arange.c
        dwarf_debug_sup.c
        dwarf_debugaddr.c
        dwarf_debuglink.c
        dwarf_die_deliv.c
        dwarf_debugnames.c
        dwarf_dsc.c
        dwarf_elf_load_headers.c
        dwarf_elfread.c
        dwarf_elf_rel_detector.c
        dwarf_error.c
        dwarf_fill_in_attr_form.c
        dwarf_find_sigref.c
        dwarf_fission_to_cu.c
        dwarf_form.c
        dwarf_form_class_names.c
        dwarf_frame.c
        dwarf_frame2.c
        dwarf_gdbindex.c
        dwarf_global.c
        dwarf_gnu_index.c
        dwarf_groups.c
        dwarf_harmless.c
        dwarf_generic_init.c
        dwarf_init_finish.c
        dwarf_leb.c
        dwarf_line.c
        dwarf_loc.c
        dwarf_loclists.c
        dwarf_locationop_read.c
        dwarf_machoread.c
        dwarf_macro.c
        dwarf_macro5.c
        dwarf_memcpy_swap.c
        dwarf_names.c
        dwarf_object_read_common.c
        dwarf_object_detector.c
        dwarf_peread.c
dwarf_query.c dwarf_ranges.c 
dwarf_rnglists.c
dwarf_safe_arithmetic.c
dwarf_safe_strcpy.c
dwarf_secname_ck.c
dwarf_setup_sections.c
dwarf_string.h dwarf_string.c
dwarf_stringsection.c
dwarf_tied.c 
dwarf_str_offsets.c
dwarf_tsearchhash.c dwarf_util.c 
dwarf_xu_index.c
dwarf_print_lines.c )

set_source_group(HEADERS "Header Files" dwarf.h dwarf_abbrev.h
dwarf_alloc.h dwarf_arange.h dwarf_base_types.h 
dwarf_debugaddr.h
dwarf_debuglink.h dwarf_die_deliv.h 
dwarf_debugnames.h dwarf_dsc.h 
dwarf_elf_access.h dwarf_elf_defines.h dwarf_elfread.h 
dwarf_elf_rel_detector.h 
dwarf_elfstructs.h 
dwarf_error.h dwarf_frame.h 
dwarf_gdbindex.h dwarf_global.h dwarf_harmless.h 
dwarf_gnu_index.h 
dwarf_line.h dwarf_loc.h 
dwarf_machoread.h dwarf_macro.h dwarf_macro5.h 
dwarf_object_detector.h dwarf_opaque.h 
dwarf_pe_descr.h dwarf_peread.h
dwarf_reading.h
dwarf_rnglists.h
dwarf_safe_arithmetic.h
dwarf_safe_strcpy.h
dwarf_tied_decls.h 
dwarf_tsearch.h 
dwarf_setup_sections.h
dwarf_str_offsets.h
dwarf_util.h 
dwarf_xu_index.h libdwarf_private.h
dwarf_macho_loader.h
dwarf_memcpy_swap.h)

set_source_group(CONFIGURATION_FILES "Configuration Files" ${PROJECT_SOURCE_DIR}/cmake/config.h.cmake ${PROJECT_BINARY_DIR}/config.h)
configure_file(libdwarf.pc.cmake libdwarf.pc @ONLY)

# Dwarfgen requires all symbols in .so to be visible, so
# do not say hidden on libdwarf in that case.
add_library(dwarf ${SOURCES})
if (BUILD_SHARED)
    if (NOT BUILD_DWARFGEN)
        set(CMAKE_C_VISIBILITY_PRESET hidden)
    endif ()
    set(COMPILER_FLAGS -DLIBDWARF_BUILD -DPIC)
    set_property(TARGET dwarf PROPERTY POSITION_INDEPENDENT_CODE 1)
else()
    set(COMPILER_FLAGS -DLIBDWARF_STATIC)
endif()
target_include_directories(dwarf PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include/libdwarf>)
if (ZLIB_FOUND)
    target_link_libraries(dwarf PUBLIC ZLIB::ZLIB)
endif()
if (ZSTD_FOUND)
    target_link_libraries(dwarf PUBLIC ZSTD::ZSTD)
endif()
target_compile_options(dwarf PUBLIC ${COMPILER_FLAGS} ${DW_FWALL})
msvc_posix(dwarf)
add_library(libdwarf::libdwarf ALIAS dwarf)


install(TARGETS dwarf)
install(TARGETS dwarf EXPORT libdwarfTargets ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
install(EXPORT libdwarfTargets FILE libdwarf-targets.cmake NAMESPACE libdwarf:: DESTINATION lib/cmake/libdwarf)
install(FILES cmake/libdwarf-config.cmake DESTINATION lib/cmake/libdwarf)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/libdwarf.h DESTINATION include/libdwarf)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dwarf.h DESTINATION include/libdwarf)
install(FILES ${PROJECT_CURRENT_BINARY_DIR}/libdwarf.pc DESTINATION lib/pkgconfig)
