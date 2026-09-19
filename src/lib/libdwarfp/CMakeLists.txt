set_source_group(SOURCES "Source Files"
"${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_memcpy_swap.c"
"${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.c"
"${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_tsearchhash.c"
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
dwarf_pro_weaks.c)

set_source_group(HEADERS "Header Files"
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
dwarf_pro_util.h)

add_library(dwarfp-shared SHARED ${SOURCES} ${HEADERS})
target_link_libraries(dwarfp-shared PRIVATE dwarf-shared)
set_target_properties(dwarfp-shared PROPERTIES
                                    C_VISIBILITY_PRESET hidden
                                    VISIBILITY_INLINES_HIDDEN TRUE
                                    OUTPUT_NAME "dwarfp"
                                    EXPORT_NAME "dwarfp"
                                    VERSION "${PROJECT_VERSION}"
                                    SOVERSION "${PROJECT_VERSION_MAJOR}"
                                    PUBLIC_HEADER "libdwarfp.h"
                                    )
target_compile_definitions(dwarfp-shared PRIVATE LIBDWARF_BUILD PUBLIC PIC)
target_include_directories(dwarfp-shared PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/lib/libdwarfp>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
set_folder(dwarfp-shared src/lib/libdwarfp)
dwarf_common_options(dwarfp-shared)
add_library(libdwarf::dwarfp ALIAS dwarfp-shared)


add_library(dwarfp-static STATIC ${SOURCES} ${HEADERS} ${CONFIGURATION_FILES})
target_link_libraries(dwarfp-static PRIVATE dwarf-static)
set_target_properties(dwarfp-static PROPERTIES
                                    POSITION_INDEPENDENT_CODE $<$<BOOL:${PIC_ALWAYS}>:ON>
                                    OUTPUT_NAME "$<$<PLATFORM_ID:Windows>:dwarfp-static>$<$<NOT:$<PLATFORM_ID:Windows>>:dwarfp>"
                                    EXPORT_NAME "dwarfp-static"
                                    PUBLIC_HEADER "libdwarfp.h"
                                    )
target_compile_definitions(dwarfp-static PUBLIC LIBDWARF_STATIC $<$<BOOL:${PIC_ALWAYS}>:PIC>)
target_include_directories(dwarfp-static PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/lib/libdwarfp>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
set_folder(dwarfp-static src/lib/libdwarfp)
dwarf_common_options(dwarfp-static)
add_library(libdwarf::dwarfp-static ALIAS dwarfp-static)

configure_file(libdwarfp.pc.in libdwarfp.pc @ONLY)

if(INSTALL_STATIC_LIBRARIES)
  set(DWARFP_STATIC_LIB "dwarfp-static")
endif()

install(TARGETS dwarfp-shared ${DWARFP_STATIC_LIB}
        EXPORT libdwarfpTargets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        )
export(
  TARGETS dwarfp-shared ${DWARFP_STATIC_LIB}
  NAMESPACE libdwarfp::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/libdwarfp-targets.cmake"
)
install(EXPORT libdwarfpTargets FILE libdwarfp-targets.cmake
        NAMESPACE libdwarfp::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/libdwarfp")
install(FILES cmake/libdwarfp-config.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/libdwarfp")
install(FILES "${PROJECT_BINARY_DIR}/src/lib/libdwarfp/libdwarfp.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
