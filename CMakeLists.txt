cmake_minimum_required(VERSION 3.0.0)
project(Ouver)

set (CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

FILE(GLOB_RECURSE app_sources src/*)
add_executable(interpreter ${app_sources})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
