
set(SELFTEST "-DSELFTEST")
if (DO_TESTING)
    set_source_group(TESTLEB "Source Files" 
        ${CMAKE_SOURCE_DIR}/test/dwarf_leb_test.c 
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_leb.c 
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/pro_encode_nm.c )
    add_executable(selfleb ${TESTLEB})
    target_compile_options(selfleb PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selfleb PRIVATE ${DW_FWALL})
    add_test(NAME selfleb COMMAND selfleb)
endif()
if (DO_TESTING)
    set_source_group(TESTTIED "Source Files" 
        ${CMAKE_SOURCE_DIR}/test/dwarf_tied_test.c 
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_tied.c 
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_tsearchhash.c )
    add_executable(selftied ${TESTTIED})
    target_compile_options(selftied PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selftied PRIVATE ${DW_FWALL})
    add_test(NAME selftied COMMAND selftied)
endif()

if (DO_TESTING)
    set_source_group(TESTSTRING "Source Files" 
        ${CMAKE_SOURCE_DIR}/test/test_dwarfstring.c 
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarfstring.c 
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarfstring.h)
    add_executable(teststring ${TESTSTRING})
    target_compile_options(teststring PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selftied PRIVATE ${DW_FWALL})
    add_test(NAME teststring COMMAND teststring)
endif()

if (DO_TESTING) 
    set_source_group(GETOPTEST_SOURCES "Source Files" 
      ${CMAKE_SOURCE_DIR}/test/getopttest.c 
      ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwgetopt.c)
    add_executable(selfgetopttest ${GETOPTEST_SOURCES})
    target_compile_options(selfgetopttest PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selfgetopttest PRIVATE ${DW_FWALL})
    add_test(NAME selfgetopttest COMMAND selfgetopttest)
endif()    
    
if (DO_TESTING) 
    set_source_group(TESTESB_SOURCES "Source Files" 
       ${CMAKE_SOURCE_DIR}/test/testesb.c
       ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/esb.c 
       ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dwarf_tsearchbal.c)
    add_executable(selftestesb ${TESTESB_SOURCES})
    target_compile_options(selftestesb PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selftestesb PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    target_compile_options(selftestesb PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    target_compile_options(selftestesb PRIVATE ${SELFTEST} )
    target_compile_options(selftestesb PRIVATE ${DW_FWALL})
    add_test(NAME selftestesb COMMAND selftestesb)
endif()

if (DO_TESTING) 
    set_source_group(MAKENAME_SOURCES "Source Files"
      ${CMAKE_SOURCE_DIR}/test/makename_test.c 
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/esb.c 
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dwarf_tsearchbal.c 
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/makename.c)
    add_executable(selfmakename ${MAKENAME_SOURCES})
    target_compile_options(selfmakename PRIVATE ${DW_FWALL})
    target_compile_options(selfmakename PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selfmakename PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    add_test(NAME selfmakename COMMAND selfmakename)
endif()


if (DO_TESTING) 
    set_source_group(HELPERTREE_SOURCES "Source Files"
       ${CMAKE_SOURCE_DIR}/test/helpertree_test.c 
       ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/helpertree.c 
       ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dwarf_tsearchbal.c)
    add_executable(selfhelpertree ${HELPERTREE_SOURCES})
    target_compile_options(selfhelpertree PRIVATE ${DW_FWALL})
    target_compile_options(selfhelpertree PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/dwarfdump")
    target_compile_options(selfhelpertree PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selfhelpertree PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    add_test(NAME selfhelpertree COMMAND selfhelpertree)
endif()

if (DO_TESTING) 
    set_source_group(SELFMC_SOURCES "Source Files" 
        ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/macrocheck.c
        ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/esb.c 
        ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dwarf_tsearchbal.c)
    add_executable(selfmc ${SELFMC_SOURCES} )
    target_compile_options(selfmc PRIVATE ${SELFTEST} )
    target_compile_options(selfmc PRIVATE ${DW_FWALL})
    target_compile_options(selfmc PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selfmc PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    add_test(NAME selfmc COMMAND selfmc)
endif()

if (DO_TESTING) 
    set_source_group(SELFESB_SOURCES "Source Files"
        ${CMAKE_SOURCE_DIR}/test/testesb.c 
        ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/esb.c)
    add_executable(selfesb ${SELFESB_SOURCES})
    target_compile_options(selfesb PRIVATE ${SELFTEST} )
    target_compile_options(selfesb PRIVATE ${DW_FWALL})
    target_compile_options(selfesb PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selfesb PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    add_test(NAME selfesb COMMAND selfesb)
endif()

if (DO_TESTING) 
    set_source_group(SECTIONBITMAPS_SOURCES "Source Files" 
        ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/section_bitmaps.c 
        ${CMAKE_SOURCE_DIR}/test/section_bitmaps_test.c )
    add_executable(selfsectionbitmaps ${SECTIONBITMAPS_SOURCES})
    target_compile_options(selfsectionbitmaps PRIVATE 
        ${SELFTEST} )
    target_compile_options(selfsectionbitmaps PRIVATE ${DW_FWALL})
    target_compile_options(selfsectionbitmaps PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selfsectionbitmaps PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    add_test(NAME selfsectionbitmaps COMMAND selfsectionbitmaps)
endif()

if (DO_TESTING) 
    set(peobj "${CMAKE_SOURCE_DIR}/test/testobjLE32PE.exe") 
    set(pebase "${CMAKE_SOURCE_DIR}/test//testobjLE32PE.base") 
    set(execpedd "${CMAKE_BINARY_DIR}/src/bin/dwarfdump/dwarfdump")
    add_test(NAME selfdwarfdumppe COMMAND sh -c "${execpedd} -vvv -a ${peobj} |  head -700 | sed 's/last time 0x.*/last time 0x0/' >junk.testoutpe ; diff ${pebase} junk.testoutpe" )
endif()

if (DO_TESTING) 
    set(elfobj "${CMAKE_SOURCE_DIR}/test/testuriLE64ELf.obj") 
    set(elfbase "${CMAKE_SOURCE_DIR}/test/testuriLE64ELf.base")
    set(execelfdd "${CMAKE_BINARY_DIR}/src/bin/dwarfdump/dwarfdump")
    add_test(NAME selfdwarfdumpelf COMMAND sh -c "${execelfdd} -vvv -a ${elfobj} | head -700 > junk.testoutelf ; diff ${elfbase} junk.testoutelf" )
endif()

if (DO_TESTING) 
    set(elfobj "${CMAKE_SOURCE_DIR}/test/test-mach-o-32.dSYM") 
    set(elfbase "${CMAKE_SOURCE_DIR}/test/test-mach-o-32.base")
    set(execelfdd "${CMAKE_BINARY_DIR}/src/bin/dwarfdump/dwarfdump")
    add_test(NAME selfdwarfdumpmacho COMMAND sh -c "${execelfdd} -vvv -a ${elfobj} | head -700 > junk.testoutelf ; diff ${elfbase} junk.testoutelf" )
endif()
