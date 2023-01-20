set_source_group(SOURCES "Source Files" createirepformfrombinary.cc
    createirepfrombinary.cc
    dwarfgen.cc irepattrtodbg.cc ireptodbg.cc
    dg_getopt.c)
    
set_source_group(HEADERS "Header Files" createirepfrombinary.h
    general.h irepattrtodbg.h 
    irepdie.h irepform.h irepframe.h 
    irepline.h irepmacro.h ireppubnames.h 
    irepresentation.h ireptodbg.h strtabdata.h
    ../../lib/libdwarf/libdwarf_private.h
    dg_getopt.h
    ../../lib/libdwarf/dwarf.h
    ../../lib/libdwarfp/libdwarfp.h)

set_source_group(CONFIGURATION_FILES "Configuration Files" 
    ${PROJECT_SOURCE_DIR}/cmake/config.h.cmake
    ${PROJECT_BINARY_DIR}/config.h)
    
add_executable(dwarfgen ${SOURCES} ${HEADERS} ${CONFIGURATION_FILES})

set_folder(dwarfgen src/bin/dwarfgen)

target_compile_options(dwarfgen PRIVATE ${DW_FWALLXX})
message("Current cmake src dir ${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(dwarfgen PRIVATE 
   ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/libdwarf
   ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/libdwarfp)

target_link_libraries(dwarfgen PRIVATE ${dwarfp-target}
   ${dwarf-target} ${DW_FZLIB})

set(SUFFIX $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:64>)
set(LIBDIR lib${SUFFIX})
set(BINDIR bin${SUFFIX})

install(TARGETS dwarfgen DESTINATION
	RUNTIME DESTINATION ${BINDIR}		
	LIBRARY DESTINATION ${LIBDIR}
	ARCHIVE DESTINATION ${LIBDIR})

#install(FILES dwarfgen.1 DESTINATION share/man/man1)
