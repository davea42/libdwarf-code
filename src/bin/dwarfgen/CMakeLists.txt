set_source_group(SOURCES "Source Files" createirepformfrombinary.cc
                 createirepfrombinary.cc dwarfgen.cc irepattrtodbg.cc
                 ireptodbg.cc dg_getopt.c
                 )

set_source_group(
    HEADERS "Header Files" createirepfrombinary.h general.h irepattrtodbg.h
    irepdie.h irepform.h irepframe.h irepline.h irepmacro.h ireppubnames.h
    irepresentation.h ireptodbg.h strtabdata.h dg_getopt.h
    )

set_source_group(CONFIGURATION_FILES "Configuration Files"
                 ${PROJECT_SOURCE_DIR}/cmake/config.h.cmake
                 ${PROJECT_BINARY_DIR}/config.h
                 )
add_executable(dwarfgen ${SOURCES} ${HEADERS} ${CONFIGURATION_FILES})
set_folder(dwarfgen src/bin/dwarfgen)
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE CXX)
target_compile_options(dwarfgen PRIVATE -fpermissive)
target_include_directories(
    dwarfgen PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                     $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    )
target_link_libraries(dwarfgen PRIVATE libdwarf::libdwarf libdwarfp::libdwarfp)
install(TARGETS dwarfgen)
