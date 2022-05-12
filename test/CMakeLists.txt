
if (DO_TESTING)
    set_source_group(TESTSTRING "Source Files"
        ${CMAKE_SOURCE_DIR}/test/test_dwarfstring.c
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.c
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.h)
    add_executable(selfteststring ${TESTSTRING})
    target_compile_options(selfteststring PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selfteststring PRIVATE ${DW_FWALL})
    add_test(NAME selfteststring COMMAND selfteststring)
endif()
if (DO_TESTING)
    set_source_group(TESTEXTRASTRING "Source Files"
        ${CMAKE_SOURCE_DIR}/test/test_extra_flag_strings.c
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarfp/dwarf_pro_log_extra_flag_strings.c
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.c
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.h)
    add_executable(selftestextrastring ${TESTEXTRASTRING})
    target_compile_options(selftestextrastring PRIVATE 
         "-DTESTING -DLIBDWARF_BUILD" )
    target_compile_options(selftestextrastring PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf"
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarfp")
    target_compile_options(selftestextrastring PRIVATE ${DW_FWALL})
    add_test(NAME selftestextrastring COMMAND selftestextrastring)
endif()

if (DO_TESTING)
    set_source_group(TESTLINKEDTOPATH "Source Files"
        ${CMAKE_SOURCE_DIR}/test/test_linkedtopath.c
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.c
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.h
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.c
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.h
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_debuglink.h
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_debuglink.c
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_error.h)
    add_executable(selftest_linkedtopath ${TESTEXTRASTRING})
    target_compile_options(selftest_linkedtopath PRIVATE 
        "-DTESTING -DLIBDWARF_BUILD" )
    target_compile_options(selftest_linkedtopath PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf"
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarfp")
    target_compile_options(selftest_linkedtopath PRIVATE ${DW_FWALL})
    add_test(NAME selftest_linkedtopath COMMAND selftest_linkedtopath)
endif()

if (DO_TESTING)
    set_source_group(GETOPTEST_SOURCES "Source Files"
      ${CMAKE_SOURCE_DIR}/test/getopttest.c
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_getopt.h
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_getopt.c)
    add_executable(selfgetopttest ${GETOPTEST_SOURCES})
    target_compile_options(selfgetopttest PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf"
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarfp")
    target_compile_options(selfgetopttest PRIVATE ${DW_FWALL})
    add_test(NAME selfgetopttest COMMAND selfgetopttest)
endif()   

if (DO_TESTING)
    set_source_group(MAKENAME_SOURCES "Source Files"
      ${CMAKE_SOURCE_DIR}/test/makename_test.c
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_makename.c
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c)
    add_executable(selfmakename ${MAKENAME_SOURCES})
    target_compile_options(selfmakename PRIVATE ${DW_FWALL})
    target_compile_options(selfmakename PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf"
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarfp")
    target_compile_options(selfmakename PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    add_test(NAME selfmakename COMMAND selfmakename)
endif()
if (DO_TESTING)
    set_source_group(HELPERTREE_SOURCES "Source Files"
      ${CMAKE_SOURCE_DIR}/test/helpertree_test.c
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_helpertree.c
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c)
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
      ${CMAKE_SOURCE_DIR}/test/test_macrocheck.c
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c
      ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c)
    add_executable(selfmacrocheck ${SELFMC_SOURCES} )
    target_compile_options(selfmacrocheck PRIVATE "-DTESTING" )
    target_compile_options(selfmacrocheck PRIVATE ${DW_FWALL})
    target_compile_options(selfmacrocheck PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf"
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarfp")
    target_compile_options(selfmacrocheck PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    add_test(NAME selfmacrocheck COMMAND selfmacrocheck)
endif()

if (DO_TESTING)
    set_source_group(TESTESB_SOURCES "Source Files"
       ${CMAKE_SOURCE_DIR}/test/testesb.c
       ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c
       ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c)
    add_executable(selftestesb ${TESTESB_SOURCES})
    target_compile_options(selftestesb PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selftestesb PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    target_compile_options(selftestesb PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    target_compile_options(selftestesb PRIVATE ${DW_FWALL})
    add_test(NAME selftestesb COMMAND selftestesb)
endif()
if (DO_TESTING)
    set_source_group(SECTIONBITMAPS_SOURCES "Source Files"
        ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_section_bitmaps.c
        ${CMAKE_SOURCE_DIR}/test/section_bitmaps_test.c )
    add_executable(selfsectionbitmaps ${SECTIONBITMAPS_SOURCES})
    target_compile_options(selfsectionbitmaps PRIVATE ${DW_FWALL})
    target_compile_options(selfsectionbitmaps PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selfsectionbitmaps PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump")
    add_test(NAME selfsectionbitmaps COMMAND selfsectionbitmaps)
endif()

if (DO_TESTING)
    set_source_group(TESTLEB "Source Files" 
        ${CMAKE_SOURCE_DIR}/test/dwarf_leb_test.c 
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_leb.c )
    add_executable(selfleb ${TESTLEB})
    target_compile_options(selfleb PRIVATE 
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf" "-DLIBDWARF_BUILD")
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
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf" "-DLIBDWARF_BUILD")
    target_compile_options(selftied PRIVATE ${DW_FWALL})
    add_test(NAME selftied COMMAND selftied)
endif()
if (DO_TESTING) 
    set_source_group(TESTNAMES "Source Files"
        ${CMAKE_SOURCE_DIR}/test/getnametest.c
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_names.c)
    add_executable(selftestnames ${TESTNAMES})
    target_compile_options(selftestnames PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf" "-DLIBDWARF_BUILD")
    target_compile_options(selftestnames PRIVATE ${DW_FWALL})
    add_test(NAME selftestnames COMMAND selftestnames)
endif()

if (DO_TESTING AND BUILD_DWARFEXAMPLE)
    set(execdl "${CMAKE_BINARY_DIR}/src/bin/dwarfexample/jitreader")
    add_test(NAME selfjitreader COMMAND sh -c "${CMAKE_SOURCE_DIR}/test/jitreaderdiff.sh ${CMAKE_SOURCE_DIR}")
endif()

if (DO_TESTING) 
    set_source_group(OBJERRMSGLIST "Source Files"
        ${CMAKE_SOURCE_DIR}/test/test_errmsglist.c 
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.c)
    add_executable(selferrmsglist ${OBJERRMSGLIST})
    target_compile_options(selferrmsglist PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf" )
    target_compile_options(selferrmsglist PRIVATE ${DW_FWALL})
    add_test(NAME selferrmsglist COMMAND 
        selferrmsglist -f "${CMAKE_SOURCE_DIR}")
endif()

if (DO_TESTING)
    set_source_group(CANONICALLIST "Source Files"
        ${CMAKE_SOURCE_DIR}/test/test_canonical.c 
        ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_safe_strcpy.c
        ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_canonical_append.c
        ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c)
    add_executable(selfcanonical ${CANONICALLIST})
    target_compile_options(selfcanonical PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf"
        "-DLIBDWARF_BUILD")
    target_compile_options(selfcanonical PRIVATE ${DW_FWALL})
    add_test(NAME selfcanonical COMMAND selfcanonical)
endif()

if (DO_TESTING)
    set_source_group(SAFESTRCPYLIST "Source Files"
        ${CMAKE_SOURCE_DIR}/test/test_safe_strcpy.c 
        ${CMAKE_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.c)
    add_executable(selfteststrcpy ${SAFESTRCPYLIST})
    target_compile_options(selfteststrcpy PRIVATE
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf" )
    target_compile_options(selfteststrcpy PRIVATE ${DW_FWALL})
    add_test(NAME selfteststrcpy COMMAND selfteststrcpy)
endif()

if (DO_TESTING)
    set_source_group(REGEXLIST "Source Files"
        ${CMAKE_SOURCE_DIR}/test/test_regex.c 
        ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dd_regex.c)
    add_executable(selfregex ${REGEXLIST})
    target_compile_options(selfregex PRIVATE
        "-I$(CMAKE_BINARY_DIR)"
        "-I${CMAKE_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${CMAKE_SOURCE_DIR}/src/lib/libdwarf" )
    target_compile_options(selfregex PRIVATE ${DW_FWALL})
    add_test(NAME selfregex COMMAND selfregex)
endif()

if (DO_TESTING) 
    add_custom_target (copyconf ALL
       COMMAND ${CMAKE_COMMAND} -E
       copy ${CMAKE_SOURCE_DIR}/src/bin/dwarfdump/dwarfdump.conf
          ${CMAKE_BINARY_DIR}/test/dwarfdump.conf )
    set(pebasedir "${CMAKE_SOURCE_DIR}")
    set(peshdir   "${CMAKE_SOURCE_DIR}/test")
    set(pebindir  "${CMAKE_BINARY_DIR}")
    add_test(NAME selfdwarfdumppe COMMAND ${peshdir}/dwarfdumptest.py PE cmake ${pebasedir} ${pebindir})
endif()

if (DO_TESTING) 
    set(elfbasedir "${CMAKE_SOURCE_DIR}")
    set(elfshdir   "${CMAKE_SOURCE_DIR}/test")
    set(elfbindir  "${CMAKE_BINARY_DIR}")
    add_test(NAME selfdwarfdumpelf COMMAND ${elfshdir}/dwarfdumptest.py Elf cmake ${elfbasedir} ${elfbindir})
endif()

if (DO_TESTING) 
    set(macbasedir "${CMAKE_SOURCE_DIR}")
    set(macshdir   "${CMAKE_SOURCE_DIR}/test")
    set(macbindir  "${CMAKE_BINARY_DIR}")
    add_test(NAME selfdwarfdumpmacho COMMAND ${macshdir}/dwarfdumptest.py Macos cmake ${macbasedir} ${macbindir})
endif()

if (DO_TESTING AND BUILD_DWARFEXAMPLE) 
    set(dlbasedir "${CMAKE_SOURCE_DIR}")
    set(dlshdir   "${CMAKE_SOURCE_DIR}/test")
    add_test(NAME selfdebuglinka COMMAND sh -c "${dlshdir}/debuglinktest-a.sh ${dlbasedir}")
endif()

if (DO_TESTING AND BUILD_DWARFEXAMPLE) 
    set(dlbasedir "${CMAKE_SOURCE_DIR}")
    set(dlshdir   "${CMAKE_SOURCE_DIR}/test")
    add_test(NAME selfdebuglinkb COMMAND sh -c "${dlshdir}/debuglinktest-b.sh ${dlbasedir}")
endif()


