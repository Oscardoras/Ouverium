cmake_minimum_required(VERSION 3.9.0)
project(Ouver)

include(CheckIPOSupported)
check_ipo_supported()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

FILE(GLOB_RECURSE capi_sources
    resources/library/src/*
)
add_library(capi SHARED ${capi_sources})
target_include_directories(capi PUBLIC resources/library/include)
set_property(TARGET capi PROPERTY C_STANDARD 99)

FILE(GLOB_RECURSE interpreter_sources
    src/main.cpp
    src/Utils.cpp src/Utils.hpp
    src/parser/*
    src/interpreter/*
)
add_executable(interpreter ${interpreter_sources})
target_link_libraries(interpreter PUBLIC capi)


FILE(GLOB_RECURSE compiler_sources
    src/Utils.cpp src/Utils.hpp
    src/parser/*
    src/compiler/*
)
add_executable(compiler ${compiler_sources})
target_link_libraries(compiler PUBLIC capi)
