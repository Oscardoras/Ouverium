cmake_minimum_required(VERSION 3.0.0)
project(Ouver)

set(CMAKE_CXX_STANDARD 17)

FILE(GLOB_RECURSE interpreter_sources
    src/main.cpp
    src/Utils.cpp src/Utils.hpp
    src/parser/*
    src/interpreter/*
    #src/compiler/Analyzer.cpp src/compiler/Analyzer.hpp
    #src/compiler/Functions.cpp src/compiler/Functions.hpp
)
add_executable(interpreter ${interpreter_sources})

FILE(GLOB_RECURSE capi_sources
    resources/library/*
)
add_library(capi SHARED ${capi_sources})
set_property(TARGET capi PROPERTY C_STANDARD 90)
link_libraries(interpreter PUBLIC capi)


FILE(GLOB_RECURSE compiler_sources
    src/compiler/c/*
)
#add_executable(compiler ${compiler_sources})
