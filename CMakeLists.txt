cmake_minimum_required(VERSION 3.0.0)
project(Ouver)

set (CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

FILE(GLOB_RECURSE app_sources
    src/main.cpp
    src/Utils.cpp src/Utils.hpp
    src/Expressions.cpp src/Expressions.hpp
    src/parser/*
    src/interpreter/*
    #src/compiler/Analyzer.cpp src/compiler/Analyzer.hpp
    #src/compiler/Functions.cpp src/compiler/Functions.hpp
)
add_executable(interpreter ${app_sources})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
