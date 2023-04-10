cmake_minimum_required(VERSION 3.0.0)
project(Ouver_C)

FILE(GLOB_RECURSE app_sources library/*)
add_executable(prog ${app_sources})
