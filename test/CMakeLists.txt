

if (DO_TESTING)
    set_source_group(TESTLEB "Source Files" dwarf_leb_test.c 
        dwarf_leb.c pro_encode_nm.c )
    add_executable(selfleb ${TESTLEB})
    target_compile_options(selfleb PRIVATE ${DW_FWALL})
    add_test(NAME selfleb COMMAND selfleb)
endif()
if (DO_TESTING)
    set_source_group(TESTTIED "Source Files" 
        dwarf_tied_test.c dwarf_tied.c dwarf_tsearchhash.c )
    add_executable(selftied ${TESTTIED})
    target_compile_options(selftied PRIVATE ${DW_FWALL})
    add_test(NAME selftied COMMAND selftied)

    set_source_group(TESTSTRING "Source Files" 
        test_dwarfstring.c dwarfstring.c dwarfstring.h)
    add_executable(teststring ${TESTSTRING})
    target_compile_options(selftied PRIVATE ${DW_FWALL})
    add_test(NAME teststring COMMAND teststring)
endif()

set(SELFTEST "-DSELFTEST")
if (DO_TESTING) 
    set_source_group(GETOPTEST_SOURCES "Source Files" 
      getopttest.c dwgetopt.c)
    add_executable(selfgetopttest ${GETOPTEST_SOURCES})
    target_compile_options(selfgetopttest PRIVATE ${DW_FWALL})
    add_test(NAME selfgetopttest COMMAND selfgetopttest)
endif()    
    
if (DO_TESTING) 
    set_source_group(TESTESB_SOURCES "Source Files" esb.c testesb.c
       dwarf_tsearchbal.c)
    add_executable(selftestesb ${TESTESB_SOURCES})
    target_compile_options(selftestesb PRIVATE ${SELFTEST} )
    target_compile_options(selftestesb PRIVATE ${DW_FWALL})
    add_test(NAME selftestesb COMMAND selftestesb)
endif()

if (DO_TESTING) 
    set_source_group(TESTESB_SOURCES "Source Files" getopttest.c
        dwgetopt.c)
    add_executable(selfgetopttestnat ${GETOPTEST_SOURCES})
    target_compile_options(selfgetopttestnat PRIVATE ${DW_FWALL})
    if(UNIX)
	    target_compile_definitions(selfgetopttestnat 
            PRIVATE GETOPT_FROM_SYSTEM)
    endif()
    foreach(i 1 2 3 5 6 7 8 9 10)
	    add_test(NAME selfgetopttestnat${i} 
                COMMAND selfgetopttestnat -c ${i})
    endforeach()
endif()

if (DO_TESTING) 
    set_source_group(MAKENAME_SOURCES "Source Files"
       makename_test.c esb.c dwarf_tsearchbal.c makename.c)
    add_executable(selfmakename ${MAKENAME_SOURCES})
    target_compile_options(selfmakename PRIVATE ${DW_FWALL})
    target_compile_options(selfmakename PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/libdwarf")
    target_compile_options(selfmakename PRIVATE 
        "-I${CMAKE_BINARY_DIR}/libdwarf")
    add_test(NAME selfmakename COMMAND selfmakename)
endif()


if (DO_TESTING) 
    set_source_group(HELPERTREE_SOURCES "Source Files"
       helpertree_test.c helpertree.c dwarf_tsearchbal.c)
    add_executable(selfhelpertree ${HELPERTREE_SOURCES})
    target_compile_options(selfhelpertree PRIVATE ${DW_FWALL})
    target_compile_options(selfhelpertree PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/dwarfdump")
    target_compile_options(selfhelpertree PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/libdwarf")
    target_compile_options(selfhelpertree PRIVATE 
        "-I${CMAKE_BINARY_DIR}/libdwarf")
    add_test(NAME selfhelpertree COMMAND selfhelpertree)
endif()

if (DO_TESTING) 
    set_source_group(SELFMC_SOURCES "Source Files" macrocheck.c
        esb.c dwarf_tsearchbal.c)
    add_executable(selfmc ${SELFMC_SOURCES} )
    target_compile_options(selfmc PRIVATE ${SELFTEST} )
    target_compile_options(selfmc PRIVATE ${DW_FWALL})
    target_compile_options(selfmc PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/libdwarf")
    target_compile_options(selfmc PRIVATE 
        "-I${CMAKE_BINARY_DIR}/libdwarf")
    add_test(NAME selfmc COMMAND selfmc)
endif()

if (DO_TESTING) 
    set_source_group(SELFESB_SOURCES "Source Files"
        testesb.c esb.c)
    add_executable(selfesb ${SELFESB_SOURCES})
    target_compile_options(selfesb PRIVATE ${SELFTEST} )
    target_compile_options(selfesb PRIVATE ${DW_FWALL})
    target_compile_options(selfmakename PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/libdwarf")
    target_compile_options(selfmakename PRIVATE 
        "-I${CMAKE_BINARY_DIR}/libdwarf")
    add_test(NAME selfesb COMMAND selfesb)
endif()

if (DO_TESTING) 
    set_source_group(SECTIONBITMAPS_SOURCES "Source Files" 
        section_bitmaps.c section_bitmaps_test.c )
    add_executable(selfsectionbitmaps ${SECTIONBITMAPS_SOURCES})
    target_compile_options(selfsectionbitmaps PRIVATE 
        ${SELFTEST} )
    target_compile_options(selfsectionbitmaps PRIVATE ${DW_FWALL})
    target_compile_options(selfsectionbitmaps PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/libdwarf")
    target_compile_options(selfsectionbitmaps PRIVATE 
        "-I${CMAKE_BINARY_DIR}/libdwarf")
    add_test(NAME selfsectionbitmaps COMMAND selfsectionbitmaps)
endif()

if (DO_TESTING) 
    set_source_group(SELFPRINTRELOC_SOURCES "Source Files" 
        esb.c print_reloc_test.c)
    add_executable(selfprintreloc ${SELFPRINTRELOC_SOURCES})
    # We need access to a few relocation-data libdwarf headers
    target_compile_options(selfprintreloc PRIVATE ${SELFTEST} )
    target_compile_options(selfprintreloc PRIVATE ${DW_FWALL})
    target_compile_options(selfprintreloc PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/libdwarf")
    target_compile_options(selfprintreloc PRIVATE 
        "-I${CMAKE_BINARY_DIR}/libdwarf")
    add_test(NAME selfprintreloc COMMAND selfprintreloc)
endif()
if (DO_TESTING) 
    set(peobj "${CMAKE_SOURCE_DIR}/dwarfdump/testobjLE32PE.exe") 
    set(pebase "${CMAKE_SOURCE_DIR}/dwarfdump/testobjLE32PE.base") 
    set(execpedd "${CMAKE_BINARY_DIR}/dwarfdump/dwarfdump")
    add_test(NAME selfdwarfdumppe COMMAND sh -c "${execpedd} -vvv -a ${peobj} |  head -700 | sed 's/last time 0x.*/last time 0x0/' >junk.testoutpe ; diff ${pebase} junk.testoutpe" )
endif()

if (DO_TESTING) 
    set(elfobj "${CMAKE_SOURCE_DIR}/dwarfdump/testuriLE64ELf.obj") 
    set(elfbase "${CMAKE_SOURCE_DIR}/dwarfdump/testuriLE64ELf.base")
    set(execelfdd "${CMAKE_BINARY_DIR}/dwarfdump/dwarfdump")
    add_test(NAME selfdwarfdumpelf COMMAND sh -c "${execelfdd} -vvv -a ${elfobj} | head -700 > junk.testoutelf ; diff ${elfbase} junk.testoutelf" )
endif()
