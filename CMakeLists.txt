cmake_minimum_required(VERSION 3.26)
project(CProj C)

set(CMAKE_C_STANDARD 23)

add_compile_definitions(DEBUG=0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
endif()

file(GLOB_RECURSE SOURCES src/*.c)

add_executable(CProj ${SOURCES})
