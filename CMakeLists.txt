cmake_minimum_required(VERSION 3.21)
project(Ouverium)

include(CheckIPOSupported)
include(FetchContent)


# Configuration

check_ipo_supported()
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")


# Dependencies

# Boost
set(BOOST_INCLUDE_LIBRARIES dll)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z
)
FetchContent_MakeAvailable(Boost)

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
    resources/library/src/*
)
add_library(capi SHARED ${capi_sources})
target_include_directories(capi PUBLIC resources/library/include)
set_property(TARGET capi PROPERTY C_STANDARD 99)

# Ouverium
FILE(GLOB_RECURSE ouverium_sources
    src/main.cpp
    src/Types.cpp src/Types.hpp
    src/parser/*
    src/interpreter/*
    src/compiler/*
)
add_executable(ouverium ${ouverium_sources})
target_link_libraries(ouverium PRIVATE Boost::dll)
target_link_libraries(ouverium PRIVATE ${Readline})
target_link_libraries(ouverium PUBLIC capi)


# Installation

file(CREATE_LINK ../libraries libraries SYMBOLIC)
file(CREATE_LINK ../resources/library/include capi_include SYMBOLIC)
