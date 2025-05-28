cmake_minimum_required(VERSION 3.26)
project(CEngine C)

set(CMAKE_C_STANDARD 23)

add_compile_definitions(DEBUG=0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
endif()

file(GLOB_RECURSE SOURCES src/*.c)

set(SDL_VULKAN OFF)

add_subdirectory(SDL EXCLUDE_FROM_ALL)
add_subdirectory(SDL_Image EXCLUDE_FROM_ALL)
add_subdirectory(SDL_mixer EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_mixer::SDL3_mixer m)

add_compile_options(-Wno-format-truncation)

if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${GTK_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GTK3_LIBRARIES})
    target_compile_options(${PROJECT_NAME} PRIVATE ${GTK_CFLAGS_OTHER})
endif(UNIX)
