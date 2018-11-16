cmake_minimum_required(VERSION 3.2)

if (DEFINED ENV{TRAVIS_BUILD_NUMBER})
    project(yamr VERSION 1.1.$ENV{TRAVIS_BUILD_NUMBER})
else ()
    project(yamr VERSION 1.1.1)
endif ()

find_package (Threads REQUIRED)

file(GLOB_RECURSE SOURCES *.cpp *.h)

# Setup application
add_executable(${PROJECT_NAME} ${SOURCES})

configure_file(version.h.in ${PROJECT_SOURCE_DIR}/version.h)

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
                                          CXX_STANDARD 14
                                          CXX_STANDARD_REQUIRED ON
    )
else ()
    set_target_properties(${PROJECT_NAME} PROPERTIES
                                          CXX_STANDARD 14
                                          CXX_STANDARD_REQUIRED ON
                                          COMPILE_OPTIONS -Wpedantic -Wall -Wextra
    )
endif ()

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT petr_lutik@mail.ru)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

include (CPack)
