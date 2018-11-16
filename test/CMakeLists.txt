cmake_minimum_required(VERSION 3.2)

# Setup testing
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

add_subdirectory(googletest EXCLUDE_FROM_ALL)


add_executable(tests test_main.cpp
                     ${CMAKE_SOURCE_DIR}/src/Bulk.cpp
                     ${CMAKE_SOURCE_DIR}/src/CmdInterpreter.cpp
                     ${CMAKE_SOURCE_DIR}/src/CmdProcessor.cpp
                     ${CMAKE_SOURCE_DIR}/src/Metrics.cpp
                     ${CMAKE_SOURCE_DIR}/src/ver.cpp
                     ${CMAKE_SOURCE_DIR}/inc/Bulk.h
                     ${CMAKE_SOURCE_DIR}/inc/CmdInterpreter.h
                     ${CMAKE_SOURCE_DIR}/inc/CmdProcessor.h
                     ${CMAKE_SOURCE_DIR}/inc/IObservable.h
                     ${CMAKE_SOURCE_DIR}/inc/IStreamWriter.h
                     ${CMAKE_SOURCE_DIR}/inc/Metrics.h
                     ${CMAKE_SOURCE_DIR}/inc/ver.h)

target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/inc)

if (MSVC)
    set_target_properties(tests PROPERTIES
                                CXX_STANDARD 14
                                CXX_STANDARD_REQUIRED ON
    )
else ()
    set_target_properties(tests PROPERTIES
                                CXX_STANDARD 14
                                CXX_STANDARD_REQUIRED ON
                                COMPILE_OPTIONS -Wpedantic -Wall -Wextra
    )
endif ()

target_link_libraries(tests gtest)

enable_testing()

add_test(ver_test_case tests)
add_test(cmd_processor_test_case tests)
add_test(bulk_test_case tests)
