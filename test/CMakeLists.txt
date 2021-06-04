

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
