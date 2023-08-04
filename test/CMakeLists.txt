# Library to have access to the dwarfdump *.h
add_library(dwarfdump_h INTERFACE)
target_include_directories(dwarfdump_h
                           INTERFACE "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump"
                           )

set_source_group(TESTSTRING "Source Files" test_dwarfstring.c
                 "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.c"
                 "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.h"
                 )
add_executable(selfteststring ${TESTSTRING})
target_link_libraries(selfteststring PRIVATE libdwarf::libdwarf
                                             libdwarfWarnings
                      )
add_test(NAME selfteststring COMMAND selfteststring)

set_source_group(
    TESTEXTRASTRING
    "Source Files"
    test_extra_flag_strings.c
    "${PROJECT_SOURCE_DIR}/src/lib/libdwarfp/dwarf_pro_log_extra_flag_strings.c"
    "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.c"
    "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.h"
    )
add_executable(selftestextrastring ${TESTEXTRASTRING})
target_link_libraries(selftestextrastring PRIVATE libdwarf::libdwarf dwarfp
                                                  libdwarfWarnings
                      )
add_test(NAME selftestextrastring COMMAND selftestextrastring)

set_source_group(
    TESTLINKEDTOPATH
    "Source Files"
    test_linkedtopath.c
    "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.c"
    "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.h"
    "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.c"
    "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_string.h"
    "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_debuglink.h"
    "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_debuglink.c"
    "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_error.h"
    )
add_executable(selftest_linkedtopath ${TESTEXTRASTRING})
target_link_libraries(selftest_linkedtopath PRIVATE libdwarf::libdwarf dwarfp
                                                    libdwarfWarnings
                      )
add_test(NAME selftest_linkedtopath COMMAND selftest_linkedtopath)

set_source_group(GETOPTEST_SOURCES "Source Files" test_getopt.c
                 "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_getopt.h"
                 "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_getopt.c"
                 )
add_executable(selfgetopttest ${GETOPTEST_SOURCES})
target_link_libraries(selfgetopttest PRIVATE libdwarf::libdwarf dwarfp
                                             dwarfdump_h libdwarfWarnings
                      )
add_test(NAME selfgetopttest COMMAND selfgetopttest)

set_source_group(
    MAKENAME_SOURCES
    "Source Files"
    test_makename.c
    "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c"
    "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_makename.c"
    "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c"
    )
add_executable(selfmakename ${MAKENAME_SOURCES})
target_link_libraries(selfmakename PRIVATE libdwarf::libdwarf dwarfp
                                           dwarfdump_h libdwarfWarnings
                      )
add_test(NAME selfmakename COMMAND selfmakename)

set_source_group(HELPERTREE_SOURCES "Source Files" test_helpertree.c
                 "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_helpertree.c"
                 "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c"
                 )
add_executable(selfhelpertree ${HELPERTREE_SOURCES})
target_link_libraries(selfhelpertree PRIVATE libdwarf::libdwarf dwarfp
                                             dwarfdump_h libdwarfWarnings
                      )
add_test(NAME selfhelpertree COMMAND selfhelpertree)

set_source_group(IGNORESEC_SOURCES "Source Files" test_ignoresec.c
                 "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_secname_ck.c"
                 )
add_executable(selfignoresec ${IGNORESEC_SOURCES})
target_link_libraries(selfignoresec PRIVATE libdwarf::libdwarf libdwarfWarnings)
add_test(NAME selfignoresec COMMAND selfignoresec)

set_source_group(SELFMC_SOURCES "Source Files" test_macrocheck.c
                 "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c"
                 "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c"
                 )
add_executable(selfmacrocheck ${SELFMC_SOURCES})
target_compile_options(selfmacrocheck PRIVATE "-DTESTING")
target_link_libraries(selfmacrocheck PRIVATE libdwarf::libdwarf dwarfp
                                             dwarfdump_h libdwarfWarnings
                      )
add_test(NAME selfmacrocheck COMMAND selfmacrocheck)

set_source_group(TESTESB_SOURCES "Source Files" test_esb.c
                 "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c"
                 "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_tsearchbal.c"
                 )
add_executable(selftestesb ${TESTESB_SOURCES})
target_link_libraries(selftestesb PRIVATE libdwarf::libdwarf dwarfdump_h
                                          libdwarfWarnings
                      )
add_test(NAME selftestesb COMMAND selftestesb)

set_source_group(TESTSANITIZED_SOURCES "Source Files" test_sanitized.c
                 "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c"
                 "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_sanitized.c"
                 )
add_executable(selftestsanitized ${TESTESB_SOURCES})
target_link_libraries(selftestsanitized PRIVATE libdwarf::libdwarf dwarfdump_h
                                                libdwarfWarnings
                      )
add_test(NAME selftestsanitized COMMAND selftestesb)

set_source_group(TESTLEB "Source Files" test_dwarf_leb.c
                 "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_leb.c"
                 )
add_executable(selfleb ${TESTLEB})
target_link_libraries(selfleb PRIVATE libdwarf::libdwarf libdwarfWarnings)
add_test(NAME selfleb COMMAND selfleb)

set_source_group(TESTTIED "Source Files" test_dwarf_tied.c
                 "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_tied.c"
                 "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_tsearchhash.c"
                 )
add_executable(selftied ${TESTTIED})
target_link_libraries(selftied PRIVATE libdwarf::libdwarf libdwarfWarnings)
add_test(NAME selftied COMMAND selftied)

set_source_group(TESTNAMES "Source Files" test_getname.c
                 "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_names.c"
                 )
add_executable(selftestnames ${TESTNAMES})
target_link_libraries(selftestnames PRIVATE libdwarf::libdwarf libdwarfWarnings)
add_test(NAME selftestnames COMMAND selftestnames)

if (BUILD_DWARFEXAMPLE AND NOT WIN32)
    set(execdl "${PROJECT_BINARY_DIR}/src/bin/dwarfexample/jitreader")
    add_test(
        NAME selfjitreader
        COMMAND
            sh -c
            "${PROJECT_SOURCE_DIR}/test/test_jitreaderdiff.sh ${PROJECT_SOURCE_DIR}"
        )
endif ()

set_source_group(OBJINT64TEST "Source Files" test_int64_test.c)
add_executable(selfintobj64test ${OBJINT64TEST})
target_link_libraries(selfintobj64test PRIVATE libdwarf::libdwarf
                                               libdwarfWarnings
                      )
add_test(NAME selfintobj64test COMMAND selfintobj64test -f
                                       "${PROJECT_SOURCE_DIR}"
         )

set_source_group(OBJERRMSGLIST "Source Files" test_errmsglist.c
                 "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.c"
                 )
add_executable(selferrmsglist ${OBJERRMSGLIST})
target_link_libraries(selferrmsglist PRIVATE libdwarf::libdwarf
                                             libdwarfWarnings
                      )
add_test(NAME selferrmsglist COMMAND selferrmsglist -f "${PROJECT_SOURCE_DIR}")

set_source_group(
    CANONICALLIST
    "Source Files"
    test_canonical.c
    "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_safe_strcpy.c"
    "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_canonical_append.c"
    "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_esb.c"
    )
add_executable(selfcanonical ${CANONICALLIST})
target_link_libraries(selfcanonical PRIVATE libdwarf::libdwarf dwarfdump_h
                                            libdwarfWarnings
                      )
add_test(NAME selfcanonical COMMAND selfcanonical)

set_source_group(SETUPSECLIST "Source Files" test_setupsections.c
                 "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_setup_sections.c"
                 )
add_executable(selfsetupsections ${SETUPSECLIST})
target_link_libraries(selfsetupsections PRIVATE libdwarf::libdwarf
                                                libdwarfWarnings
                      )
add_test(NAME selfsetupsections COMMAND selfsetupsections)

set_source_group(SAFESTRCPYLIST "Source Files" test_safe_strcpy.c
                 "${PROJECT_SOURCE_DIR}/src/lib/libdwarf/dwarf_safe_strcpy.c"
                 )
add_executable(selfteststrcpy ${SAFESTRCPYLIST})
target_link_libraries(selfteststrcpy PRIVATE libdwarf::libdwarf dwarfdump_h
                                             libdwarfWarnings
                      )
add_test(NAME selfteststrcpy COMMAND selfteststrcpy)

set_source_group(REGEXLIST "Source Files" test_regex.c
                 "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dd_regex.c"
                 )
add_executable(selfregex ${REGEXLIST})
target_link_libraries(selfregex PRIVATE libdwarf::libdwarf dwarfdump_h
                                        libdwarfWarnings
                      )
add_test(NAME selfregex COMMAND selfregex)

if (NOT WIN32)
    add_custom_target(
        copyconf ALL
        COMMAND "${CMAKE_COMMAND}" -E copy
                "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dwarfdump.conf"
                "${PROJECT_BINARY_DIR}/test/dwarfdump.conf"
        )
    add_custom_target(
        copyconf2 ALL
        COMMAND "${CMAKE_COMMAND}" -E copy
                "${PROJECT_SOURCE_DIR}/src/bin/dwarfdump/dwarfdump.conf"
                "${PROJECT_BINARY_DIR}/dwarfdump.conf"
        )
    set(pebasedir "${PROJECT_SOURCE_DIR}")
    set(peshdir "${PROJECT_SOURCE_DIR}/test")
    set(pebindir "${PROJECT_BINARY_DIR}")
    add_test(NAME selfdwarfdumppe
             COMMAND python3 "${peshdir}/test_dwarfdump.py" PE cmake
                     ${pebasedir} ${pebindir}
             )
endif ()

if (NOT WIN32)
    set(elfbasedir "${PROJECT_SOURCE_DIR}")
    set(elfshdir "${PROJECT_SOURCE_DIR}/test")
    set(elfbindir "${PROJECT_BINARY_DIR}")
    add_test(NAME selfdwarfdumpelf
             COMMAND python3 "${elfshdir}/test_dwarfdump.py" Elf cmake
                     ${elfbasedir} ${elfbindir}
             )
endif ()

if (NOT WIN32)
    set(macbasedir "${PROJECT_SOURCE_DIR}")
    set(macshdir "${PROJECT_SOURCE_DIR}/test")
    set(macbindir "${PROJECT_BINARY_DIR}")
    add_test(NAME selfdwarfdumpmacho
             COMMAND python3 "${macshdir}/test_dwarfdump.py" Macos cmake
                     ${macbasedir} ${macbindir}
             )
endif ()

if (BUILD_DWARFEXAMPLE AND NOT WIN32)
    set(dlbasedir "${PROJECT_SOURCE_DIR}")
    set(dlshdir "${PROJECT_SOURCE_DIR}/test")
    add_test(NAME selfdebuglinka
             COMMAND sh -c "${dlshdir}/test_debuglink-a.sh ${dlbasedir}"
             )
endif ()

if (BUILD_DWARFEXAMPLE AND NOT WIN32)
    set(dlbasedir "${PROJECT_SOURCE_DIR}")
    set(dlshdir "${PROJECT_SOURCE_DIR}/test")
    add_test(NAME selfdebuglinkb
             COMMAND sh -c "${dlshdir}/test_debuglink-b.sh ${dlbasedir}"
             )
endif ()
