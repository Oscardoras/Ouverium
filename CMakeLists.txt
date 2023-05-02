cmake_minimum_required(VERSION 3.0.0)
project(Ouver)

set(CMAKE_CXX_STANDARD 17)

FILE(GLOB_RECURSE app_sources
    src/main.cpp
    src/Utils.cpp src/Utils.hpp
    src/parser/*
    src/interpreter/*
    #src/compiler/Analyzer.cpp src/compiler/Analyzer.hpp
    #src/compiler/Functions.cpp src/compiler/Functions.hpp
)
add_executable(interpreter ${app_sources})
