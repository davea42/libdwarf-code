
if (DO_TESTING)
    set_source_group(TESTSTRING "Source Files"
        ${PROJECT_SOURCE_DIR}/test/test_dwarfstring.c
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.c
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.h)
    add_executable(selfteststring ${TESTSTRING})
    target_compile_definitions(selfteststring PRIVATE ${DW_LIBDWARF_STATIC})
    target_compile_options(selfteststring PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selfteststring PRIVATE ${DW_FWALL})
    add_test(NAME selfteststring COMMAND selfteststring)
endif()
if (DO_TESTING)
    set_source_group(TESTEXTRASTRING "Source Files"
        ${PROJECT_SOURCE_DIR}/test/test_extra_flag_strings.c
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarfp/dwarf_pro_log_extra_flag_strings.c
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.c
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.h)
    add_executable(selftestextrastring ${TESTEXTRASTRING})
    target_compile_definitions(selftestextrastring PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selftestextrastring PRIVATE 
         "-DTESTING -DLIBDWARF_BUILD" )
    target_compile_options(selftestextrastring PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf"
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarfp")
    target_compile_options(selftestextrastring PRIVATE ${DW_FWALL})
    add_test(NAME selftestextrastring COMMAND selftestextrastring)
endif()

if (DO_TESTING)
    set_source_group(TESTLINKEDTOPATH "Source Files"
        ${PROJECT_SOURCE_DIR}/test/test_linkedtopath.c
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.c
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.h
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.c
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.h
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_debuglink.h
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_debuglink.c
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_error.h)
    add_executable(selftest_linkedtopath ${TESTEXTRASTRING})
    target_compile_definitions(selftest_linkedtopath PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selftest_linkedtopath PRIVATE 
        "-DTESTING -DLIBDWARF_BUILD" )
    target_compile_options(selftest_linkedtopath PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf"
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarfp")
    target_compile_options(selftest_linkedtopath PRIVATE ${DW_FWALL})
    add_test(NAME selftest_linkedtopath COMMAND selftest_linkedtopath)
endif()

if (DO_TESTING)
    set_source_group(GETOPTEST_SOURCES "Source Files"
      ${PROJECT_SOURCE_DIR}/test/test_getopt.c
      ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_getopt.h
      ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_getopt.c)
    add_executable(selfgetopttest ${GETOPTEST_SOURCES})
    target_compile_definitions(selfgetopttest PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selfgetopttest PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf"
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarfp")
    target_compile_options(selfgetopttest PRIVATE ${DW_FWALL})
    add_test(NAME selfgetopttest COMMAND selfgetopttest)
endif()   

if (DO_TESTING)
    set_source_group(MAKENAME_SOURCES "Source Files"
      ${PROJECT_SOURCE_DIR}/test/test_makename.c
      ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c
      ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_makename.c
      ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c)
    add_executable(selfmakename ${MAKENAME_SOURCES})
    target_compile_definitions(selfmakename PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selfmakename PRIVATE ${DW_FWALL})
    target_compile_options(selfmakename PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf"
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarfp")
    target_compile_options(selfmakename PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump")
    add_test(NAME selfmakename COMMAND selfmakename)
endif()
if (DO_TESTING)
    set_source_group(HELPERTREE_SOURCES "Source Files"
      ${PROJECT_SOURCE_DIR}/test/test_helpertree.c
      ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_helpertree.c
      ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c)
    add_executable(selfhelpertree ${HELPERTREE_SOURCES})
    target_compile_definitions(selfhelpertree PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selfhelpertree PRIVATE ${DW_FWALL})
    target_compile_options(selfhelpertree PRIVATE
        "-I${PROJECT_SOURCE_DIR}/dwarfdump")
    target_compile_options(selfhelpertree PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selfhelpertree PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump")
    add_test(NAME selfhelpertree COMMAND selfhelpertree)
endif()
if (DO_TESTING)
    set_source_group(IGNORESEC_SOURCES "Source Files"
      ${PROJECT_SOURCE_DIR}/test/test_ignoresec.c
      ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_secname_ck.c)
    add_executable(selfignoresec ${IGNORESEC_SOURCES})
    target_compile_definitions(selfignoresec PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selfignoresec PRIVATE ${DW_FWALL})
    target_compile_options(selfignoresec PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf")
    add_test(NAME selfignoresec COMMAND selfignoresec)
endif()

if (DO_TESTING)
    set_source_group(SELFMC_SOURCES "Source Files"
      ${PROJECT_SOURCE_DIR}/test/test_macrocheck.c
      ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c
      ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c)
    add_executable(selfmacrocheck ${SELFMC_SOURCES} )
    target_compile_definitions(selfmacrocheck PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selfmacrocheck PRIVATE "-DTESTING" )
    target_compile_options(selfmacrocheck PRIVATE ${DW_FWALL})
    target_compile_options(selfmacrocheck PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf"
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarfp")
    target_compile_options(selfmacrocheck PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump")
    add_test(NAME selfmacrocheck COMMAND selfmacrocheck)
endif()

if (DO_TESTING)
    set_source_group(TESTESB_SOURCES "Source Files"
       ${PROJECT_SOURCE_DIR}/test/test_esb.c
       ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c
       ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c)
    add_executable(selftestesb ${TESTESB_SOURCES})
    target_compile_definitions(selftestesb PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selftestesb PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selftestesb PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump")
    target_compile_options(selftestesb PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump")
    target_compile_options(selftestesb PRIVATE ${DW_FWALL})
    add_test(NAME selftestesb COMMAND selftestesb)
endif()
if (DO_TESTING)
    set_source_group(TESTSANITIZED_SOURCES "Source Files"
       ${PROJECT_SOURCE_DIR}/test/test_sanitized.c
       ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c
       ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_sanitized.c
       ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_utf8.c
    )
    add_executable(selftestsanitized ${TESTESB_SOURCES})
    target_compile_definitions(selftestsanitized PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selftestsanitized PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf")
    target_compile_options(selftestsanitized PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump")
    target_compile_options(selftestsanitized PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump")
    target_compile_options(selftestsanitized PRIVATE ${DW_FWALL})
    add_test(NAME selftestsanitized COMMAND selftestesb)
endif()


if (DO_TESTING)
    set_source_group(TESTLEB "Source Files" 
        ${PROJECT_SOURCE_DIR}/test/test_dwarf_leb.c 
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_leb.c )
    add_executable(selfleb ${TESTLEB})
    target_compile_definitions(selfleb PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selfleb PRIVATE 
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf" "-DLIBDWARF_BUILD")
    target_compile_options(selfleb PRIVATE ${DW_FWALL})
    add_test(NAME selfleb COMMAND selfleb)
endif()

if (DO_TESTING)
    set_source_group(TESTTIED "Source Files" 
        ${PROJECT_SOURCE_DIR}/test/test_dwarf_tied.c 
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_tied.c 
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_tsearchhash.c )
    add_executable(selftied ${TESTTIED})
    target_compile_definitions(selftied PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selftied PRIVATE 
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf"
        "-DTESTING" "-DLIBDWARF_BUILD")
    target_compile_options(selftied PRIVATE ${DW_FWALL})
    add_test(NAME selftied COMMAND selftied)
endif()
if (DO_TESTING) 
    set_source_group(TESTNAMES "Source Files"
        ${PROJECT_SOURCE_DIR}/test/test_getname.c
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_names.c)
    add_executable(selftestnames ${TESTNAMES})
    target_compile_definitions(selftestnames PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selftestnames PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf" "-DLIBDWARF_BUILD")
    target_compile_options(selftestnames PRIVATE ${DW_FWALL})
    add_test(NAME selftestnames COMMAND selftestnames)
endif()

if (DO_TESTING AND BUILD_DWARFEXAMPLE AND NOT WIN32)
    set(execdl "${PROJECT_BINARY_DIR}/src/bin/dwarfexample/jitreader")
    add_test(NAME selfjitreader COMMAND sh -c "${PROJECT_SOURCE_DIR}/test/test_jitreaderdiff.sh ${PROJECT_SOURCE_DIR}")
endif()

if (DO_TESTING) 
    set_source_group(OBJINT64TEST "Source Files"
        ${PROJECT_SOURCE_DIR}/test/test_int64_test.c )
    add_executable(selfintobj64test ${OBJINT64TEST})
    target_compile_definitions(selfintobj64test PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selfintobj64test PRIVATE
        "-I${PROJECT_SOURCEDIR}/src/lib/libdwarf" )
    target_compile_options(selfintobj64test PRIVATE ${DW_FWALL})
    add_test(NAME selfintobj64test COMMAND 
        selfintobj64test -f "${PROJECT_SOURCE_DIR}")
endif()

if (DO_TESTING) 
    set_source_group(OBJERRMSGLIST "Source Files"
        ${PROJECT_SOURCE_DIR}/test/test_errmsglist.c 
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.c)
    add_executable(selferrmsglist ${OBJERRMSGLIST})
    target_compile_definitions(selferrmsglist PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selferrmsglist PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf" )
    target_compile_options(selferrmsglist PRIVATE ${DW_FWALL})
    add_test(NAME selferrmsglist COMMAND 
        selferrmsglist -f "${PROJECT_SOURCE_DIR}")
endif()

if (DO_TESTING)
    set_source_group(CANONICALLIST "Source Files"
        ${PROJECT_SOURCE_DIR}/test/test_canonical.c 
        ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_safe_strcpy.c
        ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_canonical_append.c
        ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c)
    add_executable(selfcanonical ${CANONICALLIST})
    target_compile_definitions(selfcanonical PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selfcanonical PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf"
        "-DLIBDWARF_BUILD")
    target_compile_options(selfcanonical PRIVATE ${DW_FWALL})
    add_test(NAME selfcanonical COMMAND selfcanonical)
endif()

if (DO_TESTING)
    set_source_group(SETUPSECLIST "Source Files"
        ${PROJECT_SOURCE_DIR}/test/test_setupsections.c 
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_setup_sections.c
        )
    add_executable(selfsetupsections ${SETUPSECLIST})
    target_compile_definitions(selfsetupsections PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selfsetupsections PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf"
        "-DLIBDWARF_BUILD")
    target_compile_options(selfsetupsections PRIVATE ${DW_FWALL})
    add_test(NAME selfsetupsections COMMAND selfsetupsections)
endif()

if (DO_TESTING)
    set_source_group(SAFESTRCPYLIST "Source Files"
        ${PROJECT_SOURCE_DIR}/test/test_safe_strcpy.c 
        ${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.c)
    add_executable(selfteststrcpy ${SAFESTRCPYLIST})
    target_compile_definitions(selfteststrcpy PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selfteststrcpy PRIVATE
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf" )
    target_compile_options(selfteststrcpy PRIVATE ${DW_FWALL})
    add_test(NAME selfteststrcpy COMMAND selfteststrcpy)
endif()

if (DO_TESTING)
    set_source_group(REGEXLIST "Source Files"
        ${PROJECT_SOURCE_DIR}/test/test_regex.c 
        ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_regex.c)
    add_executable(selfregex ${REGEXLIST})
    target_compile_definitions(selfregex PRIVATE 
        ${DW_LIBDWARF_STATIC})
    target_compile_options(selfregex PRIVATE
        "-I$(PROJECT_BINARY_DIR)"
        "-I${PROJECT_SOURCE_DIR}/src/bin/dwarfdump"
        "-I${PROJECT_SOURCE_DIR}/src/lib/libdwarf" )
    target_compile_options(selfregex PRIVATE ${DW_FWALL})
    add_test(NAME selfregex COMMAND selfregex)
endif()

if (DO_TESTING AND NOT WIN32) 
    add_custom_target (copyconf ALL
       COMMAND ${CMAKE_COMMAND} -E
       copy ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dwarfdump.conf
          ${PROJECT_BINARY_DIR}/test/dwarfdump.conf )
    add_custom_target (copyconf2 ALL
       COMMAND ${CMAKE_COMMAND} -E
       copy ${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dwarfdump.conf
          ${PROJECT_BINARY_DIR}/dwarfdump.conf )
    set(pebasedir "${PROJECT_SOURCE_DIR}")
    set(peshdir   "${PROJECT_SOURCE_DIR}/test")
    set(pebindir  "${PROJECT_BINARY_DIR}")
    add_test(NAME selfdwarfdumppe COMMAND python3 ${peshdir}/test_dwarfdump.py PE cmake ${pebasedir} ${pebindir})
endif()

if (DO_TESTING AND NOT WIN32) 
    set(elfbasedir "${PROJECT_SOURCE_DIR}")
    set(elfshdir   "${PROJECT_SOURCE_DIR}/test")
    set(elfbindir  "${PROJECT_BINARY_DIR}")
    add_test(NAME selfdwarfdumpelf COMMAND python3 ${elfshdir}/test_dwarfdump.py Elf cmake ${elfbasedir} ${elfbindir})
endif()

if (DO_TESTING AND NOT WIN32) 
    set(macbasedir "${PROJECT_SOURCE_DIR}")
    set(macshdir   "${PROJECT_SOURCE_DIR}/test")
    set(macbindir  "${PROJECT_BINARY_DIR}")
    add_test(NAME selfdwarfdumpmacho COMMAND python3 ${macshdir}/test_dwarfdump.py Macos cmake ${macbasedir} ${macbindir})
endif()

if (DO_TESTING AND BUILD_DWARFEXAMPLE AND NOT WIN32) 
    set(dlbasedir "${PROJECT_SOURCE_DIR}")
    set(dlshdir   "${PROJECT_SOURCE_DIR}/test")
    add_test(NAME selfdebuglinka COMMAND sh -c "${dlshdir}/test_debuglink-a.sh ${dlbasedir}")
endif()

if (DO_TESTING AND BUILD_DWARFEXAMPLE AND NOT WIN32) 
    set(dlbasedir "${PROJECT_SOURCE_DIR}")
    set(dlshdir   "${PROJECT_SOURCE_DIR}/test")
    add_test(NAME selfdebuglinkb COMMAND sh -c "${dlshdir}/test_debuglink-b.sh ${dlbasedir}")
endif()
