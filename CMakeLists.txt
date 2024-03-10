cmake_minimum_required(VERSION 3.21)
project(Ouverium LANGUAGES C CXX)

include(CheckIPOSupported)
include(FetchContent)
include(CPack)


# Configuration

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
else()
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
    find_package(wxWidgets COMPONENTS core base)
    if(wxWidgets_FOUND)
        include(${wxWidgets_USE_FILE})
        set(wxWidgets ${wxWidgets_LIBRARIES})
    else()
        set(wxBUILD_SHARED OFF)
        FetchContent_Declare(
            wxWidgets
            URL https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.4/wxWidgets-3.2.4.7z
        )
        FetchContent_MakeAvailable(wxWidgets)
        set(wxWidgets wx::core wx::base)
    endif()
    add_compile_definitions(WXWIDGETS)
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

# CAPI
FILE(GLOB_RECURSE capi_sources
    src/capi/*
)
add_library(capi SHARED ${capi_sources})
target_include_directories(capi PUBLIC include)
set_property(TARGET capi PROPERTY C_STANDARD 99)

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
target_link_libraries(ouverium PRIVATE Boost::asio Boost::dll)
target_link_libraries(ouverium PRIVATE ${Readline})
target_link_libraries(ouverium PRIVATE ${wxWidgets})
target_link_libraries(ouverium INTERFACE capi)


# Installation

install(TARGETS ouverium capi)
install(DIRECTORY libraries DESTINATION .)
install(DIRECTORY include DESTINATION .)

set(CPACK_PACKAGE_NAME "Ouverium")
set(CPACK_PACKAGE_VENDOR "Oscar Buon")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Programming language")
