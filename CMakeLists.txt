cmake_minimum_required(VERSION 3.9.0)
project(Ouver)

include(CheckIPOSupported)
check_ipo_supported()
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")


find_package(Boost REQUIRED COMPONENTS filesystem)


FILE(GLOB_RECURSE capi_sources
    resources/library/src/*
)
add_library(capi SHARED ${capi_sources})
target_include_directories(capi PUBLIC resources/library/include)
set_property(TARGET capi PROPERTY C_STANDARD 99)

FILE(GLOB_RECURSE ouverium_sources
    src/main.cpp
    src/Utils.cpp src/Utils.hpp
    src/parser/*
    src/interpreter/*
    src/compiler/*
)
add_executable(ouverium ${ouverium_sources})
target_include_directories(ouverium PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(ouverium PRIVATE ${Boost_LIBRARIES})
target_link_libraries(ouverium PUBLIC capi)

file(CREATE_LINK ../libraries libraries SYMBOLIC)
