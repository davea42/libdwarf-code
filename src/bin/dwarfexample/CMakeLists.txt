set_source_group(SIMPLE_READER_SOURCES "Source Files" simplereader.c)

set_source_group(CONFIGURATION_FILES "Configuration Files" ${PROJECT_SOURCE_DIR}/cmake/config.h.cmake ${PROJECT_BINARY_DIR}/config.h)

add_executable(simplereader ${SIMPLE_READER_SOURCES} ${SIMPLE_READER_HEADERS} ${CONFIGURATION_FILES})
set_folder(simplereader src/bin/dwarfexample)
target_compile_definitions(simplereader PRIVATE CONFPREFIX={CMAKE_INSTALL_PREFIX}/lib LIBDWARF_BUILD)
target_link_libraries(simplereader PRIVATE libdwarf::libdwarf)

set_source_group(FRAME1_SOURCES "Source Files" frame1.c)
add_executable(frame1 ${FRAME1_SOURCES} ${FRAME1_HEADERS} ${CONFIGURATION_FILES})
set_folder(frame1 src/bin/dwarfexample)
target_compile_definitions(frame1 PRIVATE CONFPREFIX={CMAKE_INSTALL_PREFIX}/lib LIBDWARF_BUILD)
target_link_libraries(frame1 PRIVATE libdwarf::libdwarf)

set_source_group(FINDFUNCBYPC_SOURCES "Source Files" findfuncbypc.c)
add_executable(findfuncbypc ${FINDFUNCBYPC_SOURCES} ${FINDFUNCBYPC_HEADERS} ${CONFIGURATION_FILES})
set_folder(findfuncbypc src/bin/dwarfexample)
target_compile_definitions(findfuncbypc PRIVATE CONFPREFIX={CMAKE_INSTALL_PREFIX}/lib LIBDWARF_BUILD)
target_link_libraries(findfuncbypc PRIVATE libdwarf::libdwarf)

set_source_group(GETDEBUGLINK_SOURCES "Source Files" dwdebuglink.c)
add_executable(dwdebuglink ${GETDEBUGLINK_SOURCES} ${GETDEBUGLINK_HEADERS} ${CONFIGURATION_FILES})
set_folder(dwdebuglink src/bin/dwarfexample)
target_compile_definitions(dwdebuglink PRIVATE CONFPREFIX={CMAKE_INSTALL_PREFIX}/lib LIBDWARF_BUILD)
target_link_libraries(dwdebuglink PRIVATE libdwarf::libdwarf)


set_source_group(JITREADER_SOURCES "Source Files" jitreader.c)
add_executable(jitreader ${JITREADER_SOURCES} ${CONFIGURATION_FILES})
set_folder(jitreader src/bin/dwarfexample)
target_compile_definitions(jitreader PRIVATE CONFPREFIX={CMAKE_INSTALL_PREFIX}/lib LIBDWARF_BUILD)
target_link_libraries(jitreader PRIVATE libdwarf::libdwarf)

set_source_group(SHOWSECTIONGROUPS_SOURCES "Source Files" showsectiongroups.c)
add_executable(showsectiongroups ${SHOWSECTIONGROUPS_SOURCES} ${SHOWSECTIONGROUPS_HEADERS} ${CONFIGURATION_FILES})
set_folder(jitreader src/bin/dwarfexample)
target_compile_definitions(showsectiongroups PRIVATE CONFPREFIX={CMAKE_INSTALL_PREFIX}/lib LIBDWARF_BUILD)
target_link_libraries(showsectiongroups PRIVATE libdwarf::libdwarf)
