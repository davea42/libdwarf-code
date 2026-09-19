set_source_group(SOURCES "Source Files"
createirepformfrombinary.cc
createirepfrombinary.cc
dwarfgen.cc
irepattrtodbg.cc
ireptodbg.cc
dg_getopt.cc)

set_source_group(HEADERS "Header Files"
createirepfrombinary.h
general.h
irepattrtodbg.h
irepdie.h
irepform.h
irepframe.h
irepline.h
irepmacro.h
ireppubnames.h
irepresentation.h
ireptodbg.h
strtabdata.h
dg_getopt.h
)

add_executable(dwarfgen ${SOURCES} ${HEADERS})

set_folder(dwarfgen src/bin/dwarfgen)
dwarf_common_options(dwarfgen)

target_link_libraries(dwarfgen PRIVATE dwarfp-static dwarf-static)

install(TARGETS dwarfgen DESTINATION
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

#install(FILES dwarfgen.1 DESTINATION share/man/man1)
