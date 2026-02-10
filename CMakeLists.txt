cmake_minimum_required(VERSION 3.15)

project(UAC_Bypass VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_WARNINGS "Enable compiler warnings" ON)
option(ENABLE_TESTS "Build tests" ON)
set(FINAL_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/out)

add_library(UAC_Bypass SHARED
    UAC-Bypass-lib/src/example.cpp
    UAC-Bypass-lib/src/dllmain.cpp
    UAC-Bypass-lib/src/exports.cpp
)

target_include_directories(UAC_Bypass
    PUBLIC ${PROJECT_SOURCE_DIR}/UAC-Bypass-lib/include
    PRIVATE ${PROJECT_SOURCE_DIR}/UAC-Bypass-lib/src
)

if(ENABLE_WARNINGS)
    include(cmake/warnings.cmake)
    set_project_warnings(UAC_Bypass)
endif()

set_target_properties(UAC_Bypass PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${FINAL_OUTPUT_DIR}   
    LIBRARY_OUTPUT_DIRECTORY ${FINAL_OUTPUT_DIR}   
    ARCHIVE_OUTPUT_DIRECTORY ${FINAL_OUTPUT_DIR}  
)

if(ENABLE_TESTS)
    enable_testing()

    add_executable(test_api "tests/Test1/main.cpp" 
    )

    target_link_libraries(test_api PRIVATE UAC_Bypass)

    if(ENABLE_WARNINGS)
        set_project_warnings(test_api)
    endif()

    if(WIN32)
        add_custom_command(TARGET test_api POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:UAC_Bypass>
                $<TARGET_FILE_DIR:test_api>
        )
    endif()

    set_target_properties(test_api PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${FINAL_OUTPUT_DIR}
    )

    add_test(NAME UAC_Bypass COMMAND test_api)



    add_executable(test_api2 "tests/Test2/main.cpp")

    target_link_libraries(test_api2 PRIVATE UAC_Bypass)

    if(ENABLE_WARNINGS)
        set_project_warnings(test_api2)
    endif()

    if(WIN32)
        add_custom_command(TARGET test_api2 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:UAC_Bypass>
                $<TARGET_FILE_DIR:test_api2>
        )
    endif()

    set_target_properties(test_api2 PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${FINAL_OUTPUT_DIR}
    )

    add_test(NAME UAC_Bypass_Test2 COMMAND test_api2)


endif()
