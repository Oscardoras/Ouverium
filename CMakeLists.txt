cmake_minimum_required(VERSION 3.21)
project(Ouverium VERSION 0.1.0 LANGUAGES C CXX)

include(CheckIPOSupported)
include(FetchContent)
include(CTest)
include(CPack)


# Configuration

if(CMAKE_BUILD_TYPE MATCHES Release)
    check_ipo_supported()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()


# Dependencies

# Boost
set(BOOST_INCLUDE_LIBRARIES asio dll)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z
)
FetchContent_MakeAvailable(Boost)

#WxWidgets
if(NOT DEFINED OUVERIUM_NO_GUI)
    find_package(wxWidgets COMPONENTS core base webview)
    if(wxWidgets_FOUND)
        include(${wxWidgets_USE_FILE})
        set(wxWidgets ${wxWidgets_LIBRARIES})
    else()
        set(wxBUILD_SHARED OFF)
        set(wxUSE_WEBVIEW ON)
        FetchContent_Declare(
            wxWidgets
            URL https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.4/wxWidgets-3.2.4.7z
        )
        FetchContent_MakeAvailable(wxWidgets)
        set(wxWidgets wx::core wx::base wx::webview)
    endif()
    add_compile_definitions(OUVERIUM_WXWIDGETS)
else()
    set(wxWidgets "")
endif()

# Readline
if (UNIX)
    if (EXISTS /usr/include/readline)
        add_compile_definitions(READLINE)
        set(Readline "-lreadline")
    else()
        message(WARNING "Readline is not installed.")
        set(Readline "")
    endif()
else()
    set(Readline "")
endif()


# Creating target

# Ouverium
FILE(GLOB_RECURSE ouverium_sources
    src/main.cpp
    src/Types.cpp src/Types.hpp
    src/GUIApp.cpp src/GUIApp.hpp
    src/parser/*
    src/interpreter/*
    src/compiler/*
)
add_executable(ouverium ${ouverium_sources})
target_include_directories(ouverium PRIVATE include)
target_compile_features(ouverium PRIVATE cxx_std_20)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(ouverium PRIVATE -Wall -Wextra)
endif()
if (WIN32)
    target_link_libraries(ouverium PRIVATE -lws2_32)
endif()
target_link_libraries(ouverium PRIVATE Boost::asio Boost::dll)
target_link_libraries(ouverium PRIVATE ${Readline})
target_link_libraries(ouverium PRIVATE ${wxWidgets})


# Testing

enable_testing()
add_test(NAME ouverium_test_hello_world COMMAND $<TARGET_FILE:ouverium> ${CMAKE_SOURCE_DIR}/tests/hello_world.fl)
add_test(NAME ouverium_test_string COMMAND $<TARGET_FILE:ouverium> ${CMAKE_SOURCE_DIR}/tests/string.fl)


# Installation

install(DIRECTORY libraries DESTINATION .)
install(DIRECTORY include DESTINATION .)

set(CPACK_PACKAGE_NAME "Ouverium")
set(CPACK_PACKAGE_VENDOR "Oscar Buon")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Programming language")
