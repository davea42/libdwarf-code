# Helper functions for translating autoconf projects. Several functions
# are lifted from the Mono sources

include (CheckFunctionExists)
include (CheckCSourceRuns)

# check if source compiles without linking
function(ac_try_compile SOURCE VAR)
    set(CMAKE_TMP_DIR ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp)
    if(NOT DEFINED "${VAR}")
        file(WRITE 
            "${CMAKE_TMP_DIR}/src.c" 
            "${SOURCE}\n"
        )
        
        if(NOT CMAKE_REQUIRED_QUIET)
            message(STATUS "Performing Test ${VAR}")
        endif()
        # Set up CMakeLists.txt for static library:
        file(WRITE 
            ${CMAKE_TMP_DIR}/CMakeLists.txt
            "add_library(compile STATIC src.c)"
        )
        
        # Configure:
        execute_process(
            COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . 
            WORKING_DIRECTORY ${CMAKE_TMP_DIR}
        )
        
        # Build:
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_TMP_DIR}
            RESULT_VARIABLE RESVAR 
            OUTPUT_VARIABLE OUTPUT
            ERROR_VARIABLE OUTPUT
        )
        
        # Set up result:
        if(RESVAR EQUAL 0)
            set(${VAR} 1 CACHE INTERNAL "Test ${VAR}")
            if(NOT CMAKE_REQUIRED_QUIET)
                message(STATUS "Performing Test ${VAR} - Success")
            endif()
            
            file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
                "Performing C SOURCE FILE Test ${VAR} succeded with the following output:\n"
                "${OUTPUT}\n"
                "Source file was:\n${SOURCE}\n")
        else()
            if(NOT CMAKE_REQUIRED_QUIET)
                message(STATUS "Performing Test ${VAR} - Failed")
            endif()
            set(${VAR} "" CACHE INTERNAL "Test ${VAR}")
            file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
            "Performing C SOURCE FILE Test ${VAR} failed with the following output:\n"
            "${OUTPUT}\n"
            "Source file was:\n${SOURCE}\n")
        endif()
    endif()
endfunction()
