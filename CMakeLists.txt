cmake_minimum_required(VERSION 3.25)
project(Ouverium VERSION 0.2.0 LANGUAGES C CXX)

include(CheckIPOSupported)
include(FetchContent)
include(CTest)
include(CPack)


# Options

option(OUVERIUM_BUILD_BOOST "Download and build Boost" ON)
option(OUVERIUM_WITH_WXWIDGET "GUI library support (using wxWidgets)" ON)
option(OUVERIUM_BUILD_WXWIDGET "Download and build wxWidgets" ON)


# Configuration

# add_compile_definitions($<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>)

if(CMAKE_BUILD_TYPE MATCHES Release)
    check_ipo_supported()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

enable_testing()


# Dependencies

# Boost
if (OUVERIUM_BUILD_BOOST)
    set(BOOST_INCLUDE_LIBRARIES asio dll)
    FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0-cmake.tar.gz
        SYSTEM
    )
    FetchContent_MakeAvailable(Boost)
endif()

# Readline
if (UNIX)
    find_library(READLINE_LIB readline)
    if (NOT READLINE_LIB_FOUND)
        message(WARNING "Readline is not installed.")
    endif()
endif()

# WxWidgets
if (OUVERIUM_BUILD_WXWIDGET)
    set(wxBUILD_SHARED OFF)
    set(wxUSE_WEBVIEW ON)
    set(wxBUILD_INSTALL OFF)
    FetchContent_Declare(
        wxWidgets
        URL https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.4/wxWidgets-3.2.4.zip
        SYSTEM
    )
    FetchContent_MakeAvailable(wxWidgets)
elseif(OUVERIUM_WITH_WXWIDGET)
    find_package(wxWidgets COMPONENTS core base webview)
endif()


# Creating target

add_subdirectory(app)
add_subdirectory(libs)
add_subdirectory(tests)

file(COPY ${CMAKE_SOURCE_DIR}/libraries DESTINATION ${CMAKE_BINARY_DIR})
install(DIRECTORY libraries DESTINATION .)


# Installation

set(CPACK_PACKAGE_NAME "Ouverium")
set(CPACK_PACKAGE_VENDOR "Oscar Buon")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Programming language")
