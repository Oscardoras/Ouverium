cmake_minimum_required(VERSION 3.25)
project(Ouverium VERSION 0.2.0 LANGUAGES C CXX)

include(CMakeDependentOption)
include(CheckIPOSupported)
include(FetchContent)
include(CTest)
include(CPack)


# Options

option(OUVERIUM_BUILD_BOOST "Download and build Boost" ON)
option(OUVERIUM_WITH_WXWIDGET "GUI library support (using wxWidgets)" ON)
cmake_dependent_option(OUVERIUM_BUILD_WXWIDGET "Download and build wxWidgets" ON OUVERIUM_WITH_WXWIDGET OFF)


# Configuration

if(CMAKE_BUILD_TYPE MATCHES Release)
    check_ipo_supported()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

enable_testing()


# Creating target

add_library(ouverium_options INTERFACE)
target_compile_features(ouverium_options INTERFACE cxx_std_20)
target_compile_options(ouverium_options INTERFACE $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>)

add_subdirectory(app)
add_subdirectory(external)
add_subdirectory(libs)
add_subdirectory(tests)

install(DIRECTORY libraries DESTINATION .)


# Installation

set(CPACK_PACKAGE_NAME "Ouverium")
set(CPACK_PACKAGE_VENDOR "Oscar Buon")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Programming language")
