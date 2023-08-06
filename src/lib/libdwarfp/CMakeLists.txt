set_source_group(
    SOURCES
    "Source Files"
    dwarf_pro_alloc.c
    dwarf_pro_arange.c
    dwarf_pro_debug_sup.c
    dwarf_pro_die.c
    dwarf_pro_dnames.c
    dwarf_pro_error.c
    dwarf_pro_expr.c
    dwarf_pro_finish.c
    dwarf_pro_forms.c
    dwarf_pro_funcs.c
    dwarf_pro_frame.c
    dwarf_pro_init.c
    dwarf_pro_line.c
    dwarf_pro_reloc.c
    dwarf_pro_reloc_stream.c
    dwarf_pro_reloc_symbolic.c
    dwarf_pro_pubnames.c
    dwarf_pro_section.c
    dwarf_pro_types.c
    dwarf_pro_vars.c
    dwarf_pro_macinfo.c
    dwarf_pro_weaks.c
    )

set_source_group(
    HEADERS
    "Header Files"
    libdwarfp.h
    dwarf_pro_alloc.h
    dwarf_pro_arange.h
    dwarf_pro_die.h
    dwarf_pro_error.h
    dwarf_pro_expr.h
    dwarf_pro_frame.h
    dwarf_pro_incl.h
    dwarf_pro_line.h
    dwarf_pro_log_extra_flag_strings.c
    dwarf_pro_macinfo.h
    dwarf_pro_opaque.h
    dwarf_pro_reloc.h
    dwarf_pro_reloc_stream.h
    dwarf_pro_reloc_symbolic.h
    dwarf_pro_section.h
    dwarf_pro_types.h
    dwarf_pro_util.h
    )

set_source_group(CONFIGURATION_FILES "Configuration Files"
                 ${PROJECT_SOURCE_DIR}/cmake/config.h.cmake
                 ${PROJECT_BINARY_DIR}/config.h
                 )
if (BUILD_SHARED)
    add_library(dwarfp SHARED ${SOURCES} ${HEADERS}
            ${GENNAMES_OUTPUT} ${CONFIGURATION_FILES}
    )
else ()
    add_library(dwarfp STATIC ${SOURCES} ${HEADERS}
            ${GENNAMES_OUTPUT} ${CONFIGURATION_FILES}
    )
endif ()
target_include_directories(dwarfp
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                  $<INSTALL_INTERFACE:include/libdwarfp>
                           )
target_link_libraries(dwarfp PUBLIC libdwarf::libdwarf PRIVATE libdwarfWarningsC)
set_folder(dwarfp src/lib/libdwarfp)
msvc_posix(dwarfp)
add_library(libdwarfp::libdwarfp ALIAS dwarfp)

install(TARGETS dwarfp RUNTIME DESTINATION ${BINDIR}
        LIBRARY DESTINATION ${LIBDIR} ARCHIVE DESTINATION ${LIBDIR}
        )
# The install has to be here, not in ../CMakeLists.txt  to make install work
# properly for cmake before cmake 3.13. This also works for newer cmake.
configure_file(libdwarfp.pc.cmake libdwarfp.pc @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/src/lib/libdwarfp/libdwarfp.pc"
        DESTINATION lib/pkgconfig
        )
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/libdwarfp.h"
        DESTINATION include/libdwarfp
        )
install(TARGETS dwarfp EXPORT libdwarfpTargets)
install(EXPORT libdwarfpTargets FILE libdwarfp-targets.cmake
        NAMESPACE libdwarfp:: DESTINATION lib/cmake/libdwarfp
        )

include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/libdwarfp-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/libdwarfp-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/libdwarfp"
    )
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/libdwarfp-config-version.cmake"
    COMPATIBILITY SameMinorVersion
    )
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libdwarfp-config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/libdwarfp-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/libdwarfp"
        )
