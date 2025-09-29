cmake_minimum_required(VERSION 3.13)

option(BUILD_FOR_M "Build for M-class" ON)

if(BUILD_FOR_M)
    include(pico_sdk_import.cmake)
    project(onx C CXX ASM)
    include(cmake/M.cmake)
else()
    project(onx C ASM)
    include(cmake/A.cmake)
endif()

